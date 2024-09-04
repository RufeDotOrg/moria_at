
// 0: nt console app
// 1: nt window app
enum { COSMO_WINDOWAPP = 1 };

// TBD: dylib name
DATA char* libnameD[] = {"libSDL2-2.0.so", "SDL2.dll", "libSDL2-2.0.0.dylib"};
void*
load_lib()
{
  for (int it = 0; it < AL(libnameD); ++it) {
    printf("load_lib(): %s\n", libnameD[it]);
    void* lib = cosmo_dlopen(libnameD[it], RTLD_LAZY);
    if (lib) return lib;
  }
  return 0;
}
enum { MAX_PATH = 4 * 1024 };
#include <sys/stat.h>
#include <unistd.h>
int
steam_debug()
{
  char path[MAX_PATH];
  getcwd(path, MAX_PATH);
  printf("env cwd: %s\n", path);
  struct stat statbuf;
  for (int it = 0; it < AL(libnameD); ++it) {
    if (stat(libnameD[it], &statbuf) == 0) {
      printf("  %s %jd bytes\n", libnameD[it], statbuf.st_size);
    }
  }
  printf("env TMPDIR: %s\n", getenv("TMPDIR"));
  printf("env HOME: %s\n", getenv("HOME"));
  printf("env KPRINTF_LOG: %s\n", getenv("KPRINTF_LOG"));
  printf("env SDL_OPENGL_LIBRARY: %s\n", getenv("SDL_OPENGL_LIBRARY"));
  printf("pid %d tid %d\n", getpid(), gettid());
  return 0;
}
#include <errno.h>
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
// TBD: aarch64 dlopen-helper
STATIC int
steam_helper(char* exe)
{
  int fdin, fdout;
  // Verify source before proceeding to truncate destination
  if ((fdin = open("dlopen-helper", O_RDONLY)) == -1) {
    perror("dlopen-helper no source");
    return 0;
  }

  if ((fdout = creat(exe, 0755)) == -1) {
    perror(exe);
    return 0;
  }
  if (copyfd(fdin, fdout, -1) == -1) {
    perror("dlopen copy");
    return 0;
  }

  close(fdout);
  close(fdin);

  printf("steam_helper: copy complete\n");

  return 1;
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
STATIC void
enable_windows_gui()
{
  printf("%p GetMessage()\n", (fn)GetMessage);
}
STATIC void
enable_windows_console()
{
  if (IsWindows() && AllocConsole()) {
    freopen("/dev/tty", "wb", stdout);
  }
}

int
cosmo_init(int argc, char** argv)
{
  int opt = 0;
  while (opt != -1) {
    opt = getopt(argc, argv, "lch?");
    switch (opt) {
      case 'c':
        if (COSMO_WINDOWAPP) enable_windows_console();
        break;
      case 'l':
        freopen("log.txt", "wb", stdout);
        break;
      case '?':
      case 'h':
        printf(
            "%s [-clh]\n"
            "c: console enabled on Windows\n"
            "l: write stdout to log.txt\n"
            "h: help\n",
            GetProgramExecutableName());
        exit(1);
    }
  }

  // TBD: IsXnu() issues with custom handler (does default handler work?)
  if (IsLinux() || IsWindows()) crash_init();

  if (COSMO_WINDOWAPP) enable_windows_gui();

  steam_debug();

  if (!IsWindows()) {
    char exe[PATH_MAX];
    strlcpy(exe, get_tmp_dir(), PATH_MAX);
    if (exe[0] == '.') return 0;
    strlcat(exe, "/.cosmo/", PATH_MAX);
    if (mkdir(exe, 0755) && errno != EEXIST) {
      perror("mkdir");
      return 0;
    }
    if (!IsAarch64())
      strlcat(exe, "dlopen-helper", PATH_MAX);
    else
      strlcat(exe, "aarch64-dlopen-helper", PATH_MAX);

    steam_helper(exe);
    int newer = is_file_newer_than(GetProgramExecutableName(), exe);
    printf("dlopen newer? %d\n", newer);
  }

  char path[MAX_PATH] = ":.:";
  char* sys_ld = getenv("LD_LIBRARY_PATH");
  if (sys_ld) {
    int ldlen = strlen(sys_ld);
    if (ldlen > MAX_PATH - 4) {
      printf("LD_LIBRARY_PATH is enormous.\n");
      return 1;
    }
    memcpy(path, sys_ld, ldlen);
    memcpy(path + ldlen, ":.:", 4);
  }
  printf("setenv LD_LIBRARY_PATH: %s\n", path);
  setenv("LD_LIBRARY_PATH", path, 1);

  printf("setenv KPRINTF_LOG (does not override)\n");
  setenv("KPRINTF_LOG", "crash.txt", 0);

  // Cosmo does not re-init dlopen-helper on an environment change
  libD = load_lib();
  printf("%p libD\n", libD);

  if (!libD) exit(1);

  if (IsWindows()) {
    SDL_LogSetOutputFunction(NT2SYSV(gamelog), 0);
  } else if (IsXnu()) {
    SDL_LogSetOutputFunction(0, 0);
  } else {
    SDL_LogSetOutputFunction(gamelog, 0);
  }

  global_init(argc, argv);
  platformD.seed = vptr(rdseed);
}
#define global_init cosmo_init
