
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define REPLAYNAME "replaychar"
#define REPLAYSIZE (16 * 1024)

STATIC int mmap_replay(ptr) void** ptr;
{
  int size = REPLAYSIZE;
  if (*ptr) munmap(*ptr, size);

  int classidx = globalD.saveslot_class;
  char filename[32] = REPLAYNAME;
  if (classidx >= 0 && classidx < AL(classD)) {
    char* dst = &filename[6];
    for (char* src = classD[classidx].name; *src != 0; ++src) {
      *dst++ = *src | 0x20;
    }
    *dst = 0;
  }

  int flag = O_RDWR;
  struct stat sv;
  if (stat(filename, &sv) == -1 || sv.st_size != size) flag |= O_CREAT;
  int fd = open(filename, flag, 0644);
  if (flag & O_CREAT) {
    if (fd > 0 && ftruncate(fd, size) < 0) close(fd), fd = -1;
  }

  void* buf = 0;
  if (fd > 1) buf = mmap(0, size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
  if (fd > 1) close(fd);
  showx(buf);
  if (buf == (void*)-1) buf = 0;
  *ptr = buf;
  return buf != 0;
}

int
mmap_init()
{
  platformD.mmap_replay = mmap_replay;
}

#define MMAP 1
