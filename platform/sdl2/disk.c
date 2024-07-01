
#define SAVENAME "savechar"
#define CACHENAME "moria.cache"

DATA char cachepathD[1024];
DATA int cachepath_usedD;
DATA int checksumD;

char*
path_append_filename(char* path, int path_len, char* filename)
{
  int wridx = path_len;
  char* write = &path[wridx];

  if (wridx) *write++ = '/';
  for (char* iter = filename; *iter != 0; ++iter) {
    *write++ = *iter;
  }
  *write = 0;

  return path;
}
SDL_RWops*
file_access(char* filename, char* access)
{
  SDL_RWops* ret = SDL_RWFromFile(filename, access);
  if (ret != 0) Log("%s file_access %s", access, filename);
  return ret;
}

// Disk I/O
int
version_by_savesum(sum)
{
  for (int it = 0; it < AL(savesumD); ++it)
    if (savesumD[it] == sum) return it;
  return -1;
}
int
savesum()
{
  int sum = 0;
  for (int it = 0; it < AL(save_bufD); ++it) {
    struct bufS buf = save_bufD[it];
    sum += buf.mem_size;
  }
  return sum;
}
int
clear_savebuf()
{
  for (int it = 0; it < AL(save_bufD); ++it) {
    struct bufS buf = save_bufD[it];
    memset(buf.mem, 0, buf.mem_size);
  }

  return 0;
}
int checksum(blob, len) void* blob;
{
  int* iter = blob;
  int count = len / sizeof(int);
  int* end = iter + count;
  int ret = 0;
  for (; iter < end; ++iter) {
    ret ^= *iter;
  }
  return ret;
}
// filename unchanged unless a valid classidx is specified
char*
filename_by_class(char* filename, int classidx)
{
  if (classidx >= 0 && classidx < AL(classD)) {
    char* dst = &filename[4];
    for (char* src = classD[classidx].name; *src != 0; ++src) {
      *dst++ = *src | 0x20;
    }
    *dst = 0;
  }
  Log("filename_by_class %s", filename);
  return filename;
}

int
path_exists(char* path)
{
  SDL_RWops* readf = file_access(path, "rb");
  uint32_t save_size = 0;
  if (readf) {
    SDL_RWread(readf, &save_size, sizeof(save_size), 1);
    SDL_RWclose(readf);
  }
  return save_size != 0;
}
int
path_delete(char* path)
{
  SDL_RWops* writef = file_access(path, "wb");
  if (writef) SDL_RWclose(writef);
  return (writef != 0);
}
int
path_save(char* path)
{
  int version = AL(savesumD) - 1;
  int sum = savesumD[version];
  int* savefield = savefieldD[version];

  SDL_RWops* writef = file_access(path, "wb");
  if (writef) {
    checksumD = 0;
    SDL_RWwrite(writef, &sum, sizeof(sum), 1);
    for (int it = 0; it < AL(save_bufD); ++it) {
      struct bufS buf = save_bufD[it];
      SDL_RWwrite(writef, buf.mem, savefield[it], 1);
      int ck = checksum(buf.mem, savefield[it]);
      checksumD ^= ck;
    }
    SDL_RWclose(writef);
    Log("path_save %s: version %d save checksum %x", path, version, checksumD);
    return sum;
  }
  return 0;
}
int
path_load(char* path)
{
  int save_size = 0;
  clear_savebuf();

  SDL_RWops* readf = file_access(path, "rb");
  if (readf) {
    checksumD = 0;
    SDL_RWread(readf, &save_size, sizeof(save_size), 1);
    int version = version_by_savesum(save_size);
    if (version >= 0) {
      int* savefield = savefieldD[version];
      for (int it = 0; it < AL(save_bufD); ++it) {
        struct bufS buf = save_bufD[it];
        SDL_RWread(readf, buf.mem, savefield[it], 1);
        int ck = checksum(buf.mem, savefield[it]);
        checksumD ^= ck;
      }
    } else if (save_size == savesum()) {
      for (int it = 0; it < AL(save_bufD); ++it) {
        struct bufS buf = save_bufD[it];
        SDL_RWread(readf, buf.mem, buf.mem_size, 1);
        int ck = checksum(buf.mem, buf.mem_size);
        checksumD ^= ck;
      }
    } else {
      save_size = 0;
    }

    if (input_resumeD == 0) {
      if (save_size) {
        char gh[AL(git_hashD)];
        if (SDL_RWread(readf, gh, sizeof(gh), 1)) {
          int sum = 0;
          if (memcmp(gh, git_hashD, sizeof(gh)) == 0) {
            for (int it = 0; it < AL(midpoint_bufD); ++it) {
              sum += midpoint_bufD[it].mem_size;
            }
            int64_t offset = SDL_RWseek(readf, 0, RW_SEEK_CUR);
            int64_t end = SDL_RWseek(readf, sum, RW_SEEK_CUR);
            if (end > 0) {
              SDL_RWseek(readf, offset, RW_SEEK_SET);
              for (int it = 0; it < AL(midpoint_bufD); ++it) {
                struct bufS buf = midpoint_bufD[it];
                if (!SDL_RWread(readf, buf.mem, buf.mem_size, 1)) sum = 0;
              }
            }
          }

          if (sum) {
            input_resumeD = input_action_usedD;
          } else {
            input_resumeD = 0;
            uD.new_level_flag = NL_MIDPOINT_LOST;
          }
        }
      }
    }

    SDL_RWclose(readf);
  }

  return save_size != 0;
}
int
path_savemidpoint(char* path)
{
  int save_size = 0;
  int write_ok = 0;
  int memory_ok;

  memory_ok = (input_record_writeD <= AL(input_recordD) - 1 &&
               input_action_usedD <= AL(input_actionD) - 1);

  if (memory_ok) {
    SDL_RWops* rwfile = file_access(path, "rb+");
    if (rwfile) {
      SDL_RWread(rwfile, &save_size, sizeof(save_size), 1);

      int64_t offset = SDL_RWseek(rwfile, save_size, RW_SEEK_CUR);
      if (offset > 0) {
        write_ok = SDL_RWwrite(rwfile, &git_hashD, sizeof(git_hashD), 1);
        for (int it = 0; it < AL(midpoint_bufD); ++it) {
          struct bufS buf = midpoint_bufD[it];
          if (!SDL_RWwrite(rwfile, buf.mem, buf.mem_size, 1)) write_ok = 0;
        }
      }

      SDL_RWclose(rwfile);
    }
  }
  return write_ok;
}
int
platform_load(saveslot, external)
{
  char filename[16] = SAVENAME;
  filename_by_class(filename, saveslot);
  char* path;
  if (external) {
    path = path_append_filename(exportpathD, exportpath_usedD, filename);
  } else {
    path = path_append_filename(savepathD, savepath_usedD, filename);
  }

  return path_load(path);
}
int
platform_save(saveslot)
{
  char filename[16] = SAVENAME;
  filename_by_class(filename, saveslot);
  char* path = path_append_filename(savepathD, savepath_usedD, filename);
  return path_save(path);
}
int
platform_erase(saveslot, external)
{
  char filename[16] = SAVENAME;
  filename_by_class(filename, saveslot);
  char* path;
  if (external) {
    path = path_append_filename(exportpathD, exportpath_usedD, filename);
  } else {
    path = path_append_filename(savepathD, savepath_usedD, filename);
  }
  return path_delete(path);
}
int
disk_savemidpoint()
{
  MUSE(global, saveslot_class);
  if (saveslot_class >= 0 && saveslot_class < AL(classD)) {
    char filename[16] = SAVENAME;
    filename_by_class(filename, saveslot_class);
    char* path = path_append_filename(savepathD, savepath_usedD, filename);
    return path_savemidpoint(path);
  }
  return 0;
}
int
platform_saveex()
{
  char filename[16] = SAVENAME;
  int count = 0;

  for (int it = 0; it < AL(classD); ++it) {
    filename_by_class(filename, it);
    char *in_path, *ex_path;
    in_path = path_append_filename(savepathD, savepath_usedD, filename);
    ex_path = path_append_filename(exportpathD, exportpath_usedD, filename);
    if (path_load(in_path)) count += (path_save(ex_path) != 0);
  }
  return count;
}
int
platform_testex()
{
  int ret = 0;
  char* filename = "tempfile";
  char* ex_path = path_append_filename(exportpathD, exportpath_usedD, filename);

  ret += (path_save(ex_path) != 0);

  ret += (path_load(ex_path) != 0);

  ret += (path_delete(ex_path) != 0);

  return ret;
}
int
cache_write()
{
  SDL_RWops* writef = file_access(cachepathD, "wb");
  if (writef) {
    int ret = SDL_RWwrite(writef, &globalD, sizeof(globalD), 1);
    Log("cache_write: %d", ret);
    SDL_RWclose(writef);
  }
  return writef != 0;
}

int
path_copy_to(char* srcpath, char* dstpath)
{
  SDL_RWops *readf, *writef;
  readf = file_access(srcpath, "rb");
  if (readf) {
    writef = file_access(dstpath, "wb");
    if (writef) {
      char chunk[4 * 1024];
      int read_count;
      do {
        read_count = SDL_RWread(readf, chunk, 1, AL(chunk));
        if (read_count) {
          int write_count = SDL_RWwrite(writef, chunk, 1, read_count);
          if (write_count != read_count) return 1;
        }
      } while (read_count);
      SDL_RWclose(writef);
    }
    SDL_RWclose(readf);
  }

  return readf == 0 || writef == 0;
}
int
cache_read()
{
  uint32_t success = 0;
  if (cachepath_usedD) {
    SDL_RWops* readf = file_access(cachepathD, "rb");
    if (readf) {
      if (SDL_RWread(readf, &globalD, sizeof(globalD), 1))
        success = sizeof(globalD);
      SDL_RWclose(readf);
    }
  }

  return success;
}

int
disk_postgame(may_exit)
{
  if (uD.new_level_flag != NL_DEATH) disk_savemidpoint();
  if (cachepath_usedD) cache_write();
}

int
disk_init()
{
  if (__APPLE__) {
    char* prefpath = SDL_GetPrefPath(ORGNAME, APPNAME);
    if (prefpath) {
      int len = snprintf(savepathD, AL(savepathD), "%s", prefpath);
      if (len < 0 || len >= AL(savepathD))
        savepathD[0] = 0;
      else
        savepath_usedD = len;
      SDL_free(prefpath);
    }

    char* external = SDL_AppleGetDocumentPath(ORGNAME, APPNAME);
    if (external) {
      int len = snprintf(exportpathD, AL(exportpathD), "%s", external);
      if (len < 0 || len >= AL(exportpathD)) {
        exportpathD[0] = 0;
      } else {
        exportpath_usedD = len;
      }
      SDL_free(external);
    }

    if (SL(CACHENAME)) {
      // Apple allows user interactions with the external path files
      memcpy(cachepathD, exportpathD, exportpath_usedD);
      cachepath_usedD = exportpath_usedD;
    }
  }

  if (ANDROID) {
    int state = SDL_AndroidGetExternalStorageState();
    if (state & 0x3) {
      int len = 0;
      char* external = (char*)SDL_AndroidGetExternalStoragePath();
      if (external) {
        len = snprintf(exportpathD, AL(exportpathD), "%s", external);
        Log("GetExternalStoragePath: %s", external);
        SDL_free(external);
      }
      if (len <= 0 || len >= AL(exportpathD)) {
        exportpathD[0] = 0;
      } else {
        exportpath_usedD = len;
      }
      Log("storage: [state %d] exportpath: %s", state, exportpathD);
    }

    if (!PC && SL(CACHENAME) != 0) {
      char* cache = SDL_GetCachePath(ORGNAME, APPNAME);
      if (cache) {
        Log("SDL_GetCachePath: %s", cache);
        int len = snprintf(cachepathD, AL(cachepathD), "%s", cache);
        if (len <= 0 || len >= AL(cachepathD)) {
          cachepathD[0] = 0;
          len = 0;
        }

        cachepath_usedD = len;
        SDL_free(cache);
      }
    }
  }

  if (SL(CACHENAME)) {
    path_append_filename(cachepathD, cachepath_usedD, CACHENAME);
    cachepath_usedD += SL(CACHENAME);
    Log("Game cache enabled: %s", cachepathD);
  }

  int cache_valid = cache_read();
  Log("global cache_valid (%d)\n"
      " %d saveslot_class\n"
      " %u zoom_factor\n"
      " %u orientation_lock\n",
      cache_valid, globalD.saveslot_class, globalD.zoom_factor,
      globalD.orientation_lock);

  platformD.load = platform_load;
  platformD.save = platform_save;
  platformD.erase = platform_erase;
  platformD.savemidpoint = disk_savemidpoint;
  if (exportpath_usedD) platformD.saveex = platform_saveex;
  if (exportpath_usedD) platformD.testex = platform_testex;
  return 1;
}

#define DISK 1
