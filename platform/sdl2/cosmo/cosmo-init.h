// Rufe.org LLC 2022-2025: ISC License

#define FAIL_LOG "fail.txt"
#define VERBOSE_LOG "log.txt"

#define ENV_STEAMAPP1 "STEAMAPPID"
#define ENV_STEAMAPP2 "STEAM_APPID"
#define ENV_TMPDIR "TMPDIR"
#define ENV_HOME "HOME"
#define ENV_KPRINTF_LOG "KPRINTF_LOG"
#define ENV_OPENGL "SDL_OPENGL_LIBRARY"
#define ENV_LD_LIBRARY_PATH "LD_LIBRARY_PATH"
#define ENV_SDL_GAMECONTROLLER_IGNORE_DEVICES \
  "SDL_GAMECONTROLLER_IGNORE_DEVICES"

extern custom_gamecrash_handler();

char*
cosmo_libname()
{
  if (IsWindows()) return "SDL2.dll";
  if (IsXnu()) return "libSDL2-2.0.0.dylib";
  return "libSDL2-2.0.so";
}
#include <libc/calls/calls.h>
#include <libc/calls/struct/stat.h>
#include <libc/sysv/consts/ok.h>
int
verify_info(char* path, int pathlen)
{
  char* appvar_list[] = {
      ENV_STEAMAPP1,       ENV_STEAMAPP2,
      ENV_TMPDIR,          ENV_HOME,
      ENV_KPRINTF_LOG,     ENV_OPENGL,
      ENV_LD_LIBRARY_PATH, ENV_SDL_GAMECONTROLLER_IGNORE_DEVICES,
  };
  char* cwd = getcwd(path, pathlen);

  printf("%s\n", cwd);
  printf("env\n");
  for (int it = 0; it < AL(appvar_list); ++it) {
    char* name = appvar_list[it];
    printf("  %s: %s\n", name, getenv(name));
  }

  struct stat statbuf;
  if (stat(cosmo_libname(), &statbuf) == 0) {
    printf("  %s: %jd\n", cosmo_libname(), statbuf.st_size);
  }
  return 0;
}
STATIC const char*
get_tmp_dir()
{
  const char* tmpdir;
  if (!(tmpdir = getenv(ENV_TMPDIR)) || !*tmpdir) {
    if (!(tmpdir = getenv(ENV_HOME)) || !*tmpdir) {
      tmpdir = ".";
    }
  }
  return tmpdir;
}
STATIC int
is_file_newer_than(const char* path, const char* other)
{
  struct stat st1, st2;
  if (stat(path, &st1)) {
    return -1;
  }
  if (stat(other, &st2)) {
    if (errno == ENOENT) {
      return 2;
    } else {
      return -1;
    }
  }
  return timespec_cmp(st1.st_mtim, st2.st_mtim) > 0;
}
#include <libc/sysv/consts/o.h>
STATIC int
steam_helper(char* exe, int errcode)
{
  char* source = IsAarch64() ? "/zip/dlopen_aarch64" : "/zip/dlopen_x86";
  printf("%s->%s\n", source, exe);
  int fdin = open(source, O_RDONLY);
  if (fdin == -1) return errcode;

  int fdout = open(exe, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fdout == -1) return errcode + 1;

  char buf[4 * 1024];
  int count;
  while ((count = read(fdin, AP(buf))) > 0) {
    int wr_count = write(fdout, buf, count);
    if (wr_count != count) return errcode + 2;
  }
  if (count == -1) return errcode + 3;

  close(fdout);
  close(fdin);
  return 0;
}

// Logging fix-up
// Avoid default SDL behavior of accessing parent console window
#include <libc/nexgen32e/nt2sysv.h>
DATA const char* SDL_priority_prefixes[SDL_NUM_LOG_PRIORITIES] = {
    NULL, "VERBOSE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL"};
void
gamelog(void* nulldata, int category, SDL_LogPriority p, const char* message)
{
  printf("R_ %s: %s\r\n", SDL_priority_prefixes[p], message);
}

#include <libc/nt/events.h>
// Not static (which would be optimized away)
// This is enough for cosmocc to enable kNtImageSubsystemWindowsGui
GLOBAL fn win_messageD;
STATIC int
enable_windows_gui()
{
  if (COSMO_WINDOWAPP) win_messageD = vptr(GetMessage);
}
STATIC void
enable_windows_console()
{
  if (COSMO_WINDOWAPP && IsWindows()) {
    if (AllocConsole()) freopen("/dev/tty", "wb", stdout);
  }
}
STATIC int
wb_print_log(char* name)
{
  freopen(name, "wb", stdout);
}

int
dlopen_patch(char* pathmem, int pathlen)
{
  int rv = strlcpy(pathmem, get_tmp_dir(), pathlen);
  if (rv >= pathlen) return 1;

  rv = strlcat(pathmem, "/.cosmo/", PATH_MAX);
  if (rv >= pathlen) return 1;

  if (mkdir(pathmem, 0755) && errno != EEXIST) return 2;
  rv = strlcat(pathmem, "dlopen-helper", PATH_MAX);

  return steam_helper(pathmem, 3);
}

int
enable_local_library(char* pathmem, int pathlen, int errcode)
{
  char* sys_ld = getenv(ENV_LD_LIBRARY_PATH);
  int rv = 0;
  *pathmem = 0;
  if (sys_ld) strlcpy(pathmem, sys_ld, pathlen);
  if (rv >= pathlen) return errcode;

  rv = strlcat(pathmem, ":.:", pathlen);
  if (rv >= pathlen) return errcode;

  setenv(ENV_LD_LIBRARY_PATH, pathmem, 1);
  return 0;
}

int
verify_init(char* path, int pathlen, int status)
{
  if (status) {
    if (IsLinux() && STEAM) wb_print_log(FAIL_LOG);
    if (IsWindows()) enable_windows_console();
    printf("E-mail support@rufe.org: init status %d\n", status);
    verify_info(path, pathlen);
    if (status == 20) printf("\nlibSDL2 is not found\n");
    if (IsWindows() && GetConsoleWindow()) Sleep(10 * 1000);

    exit(status);
  }

  return 0;
}

int
cosmo_init(int argc, char** argv)
{
  char pathmem[4 * 1024];
  int init_status = 0;
  int opt = 0;
  int keep_console = 0;
  int keep_log = COSMO_LOG;
  while (opt != -1) {
    opt = getopt(argc, argv, "chv?");
    switch (opt) {
      case '-':
        break;
      case 'C':
      case 'c':
        if (IsWindows()) keep_console = 1;
        break;
      case 'V':
      case 'v':
        keep_log = 1;
        break;
      case '?':
      case 'H':
      case 'h':
        printf(
            "%s [-CDLH]\n"
            "C/c: console enabled on Windows\n"
            "H/h: help\n"
            "V/v: write stdout to " VERBOSE_LOG "\n",
            GetProgramExecutableName());
        exit(1);
    }
  }

  pathmem[0] = 0;
  if (RELEASE && !IsWindows()) {
    if (STEAM) init_status = dlopen_patch(AP(pathmem));
    if (!init_status) init_status = enable_local_library(AP(pathmem), 10);
    verify_init(AP(pathmem), init_status);
  }

  // Override steam's environment
  setenv(ENV_SDL_GAMECONTROLLER_IGNORE_DEVICES, "", 1);
  // ^ (update this if steaminput is dynamically loaded in the future)

  // Cosmo does not re-init dlopen-helper on an environment change
  void* sdl_lib = cosmo_dlopen(cosmo_libname(), RTLD_LAZY);
  if (!sdl_lib) init_status = 20;
  libD = sdl_lib;

  verify_init(AP(pathmem), init_status);

  if (keep_console)
    enable_windows_console();
  else if (keep_log)
    wb_print_log(VERBOSE_LOG);

  if (COSMO_WINDOWAPP) enable_windows_gui();

  if (IsWindows()) {
    SDL_LogSetOutputFunction(NT2SYSV(gamelog), 0);
  } else {
    SDL_LogSetOutputFunction(gamelog, 0);
  }

  global_init(argc, argv);
  platformD.seed = vptr(rdseed);

  if (COSMO_CRASH && RELEASE) cosmo_crashinit(custom_gamecrash_handler);

  if (COSMO_CRASH && !RELEASE) {
    setenv(ENV_KPRINTF_LOG, FAIL_LOG, 0);
    ShowCrashReports();
  }
}
#define global_init cosmo_init
