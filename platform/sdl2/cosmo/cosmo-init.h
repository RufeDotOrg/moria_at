// Rufe.org LLC 2022-2024: ISC License

#define FAIL_LOG "fail.txt"
#define VERBOSE_LOG "log.txt"

char*
cosmo_libname()
{
  if (IsWindows()) return "SDL2.dll";
  if (IsXnu()) return "libSDL2-2.0.0.dylib";
  return "libSDL2-2.0.so";
}
#include <libc/calls/struct/stat.h>
int
verify_info(char* path, int pathlen)
{
  char* cwd = getcwd(path, pathlen);
  if (cwd) printf("env cwd: %s\n", cwd);

  printf("env TMPDIR: %s\n", getenv("TMPDIR"));
  printf("env HOME: %s\n", getenv("HOME"));
  printf("env KPRINTF_LOG: %s\n", getenv("KPRINTF_LOG"));
  printf("env SDL_OPENGL_LIBRARY: %s\n", getenv("SDL_OPENGL_LIBRARY"));
  printf("pid %d tid %d\n", getpid(), gettid());

  struct stat statbuf;
  if (stat(cosmo_libname(), &statbuf) == 0) {
    printf("  %s: %jd\n", cosmo_libname(), statbuf.st_size);
  }
  return 0;
}
STATIC const char*
get_tmp_dir(void)
{
  const char* tmpdir;
  if (!(tmpdir = getenv("TMPDIR")) || !*tmpdir) {
    if (!(tmpdir = getenv("HOME")) || !*tmpdir) {
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
steam_helper(char* exe, int exelen, int errcode)
{
  int fdin, fdout;
  // Verify source before proceeding to truncate destination
  fdin = open("dlopen-helper", O_RDONLY);
  if (fdin == -1) return errcode;

  fdout = creat(exe, 0744);
  if (fdout == -1) return errcode + 1;

  if (copyfd(fdin, fdout, -1) == -1) return errcode + 2;

  close(fdout);
  close(fdin);
  return 0;
}
STATIC int
steam_runtime()
{
  char* appvar_list[] = {"SteamAppId", "STEAM_APPID"};
  for (int it = 0; it < AL(appvar_list); ++it) {
    if (getenv(appvar_list[it])) return 1;
  }
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

// This is enough for cosmocc to enable kNtImageSubsystemWindowsGui
#include <libc/nt/events.h>
STATIC int
enable_windows_gui()
{
  return (fn)GetMessage != 0;
}
STATIC void
enable_windows_console()
{
  if (COSMO_WINDOWAPP && IsWindows()) {
    if (AllocConsole()) freopen("/dev/tty", "wb", stdout);
  }
}
static int
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

  if (!IsAarch64()) rv = strlcat(pathmem, "dlopen-helper", PATH_MAX);
  if (IsAarch64()) rv = strlcat(pathmem, "aarch64-dlopen-helper", PATH_MAX);
  if (rv >= pathlen) return 1;

  return steam_helper(AP(pathmem), 3);
}

int
enable_local_library(char* pathmem, int pathlen, int errcode)
{
  char* sys_ld = getenv("LD_LIBRARY_PATH");
  int rv = strlcpy(pathmem, sys_ld, pathlen);
  if (rv >= pathlen) return errcode;

  rv = strlcat(pathmem, ":.:", pathlen);
  if (rv >= pathlen) return errcode;

  setenv("LD_LIBRARY_PATH", pathmem, 1);
  return 0;
}

int
verify_init(char* path, int pathlen, int status)
{
  if (status) {
    if (IsLinux() && steam_runtime()) wb_print_log(FAIL_LOG);
    if (IsWindows()) enable_windows_console();
    printf("E-mail support@rufe.org: init status %d\n", status);
    verify_info(path, pathlen);
    if (status == 20) printf("libSDL2 is not found\n");
    if (IsWindows()) Sleep(10);

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
  int debug = 0;
  int keep_console = 0;
  int keep_log = 0;
  while (opt != -1) {
    opt = getopt(argc, argv, "chv?");
    switch (opt) {
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

  if (COSMO_WINDOWAPP) enable_windows_gui();

  if (RELEASE && !IsWindows()) {
    if (steam_runtime()) init_status = dlopen_patch(AP(pathmem));
    if (!init_status) init_status = enable_local_library(AP(pathmem), 10);
    verify_init(AP(pathmem), init_status);
  }

  // Override steam's environment
  setenv("SDL_GAMECONTROLLER_IGNORE_DEVICES", "", 1);
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

  if (IsWindows()) {
    SDL_LogSetOutputFunction(NT2SYSV(gamelog), 0);
  } else {
    SDL_LogSetOutputFunction(gamelog, 0);
  }

  global_init(argc, argv);
  platformD.seed = vptr(rdseed);

  if (COSMO_CRASH && RELEASE) cosmo_crashinit(custom_gamecrash_handler);

  if (COSMO_CRASH && !RELEASE) {
    printf("setenv KPRINTF_LOG (does not override)\n");
    setenv("KPRINTF_LOG", FAIL_LOG, 0);
    ShowCrashReports();
  }
}
#define global_init cosmo_init
