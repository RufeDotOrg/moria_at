
#include <libc/limits.h>
#include <libc/log/internal.h>
#include <libc/macros.internal.h>
#include <libc/nt/enum/processaccess.h>
#include "libc/calls/struct/sigaction.h"
#include "libc/calls/struct/sigaltstack.h"
#include "libc/calls/struct/sigset.h"
#include "libc/sysv/consts/sa.h"
#include "libc/sysv/consts/sig.h"

struct NtModuleInfo {
  void* lpBaseOfDll;
  uint32_t SizeOfImage;
  void* EntryPoint;
};

uint32_t __attribute__((__ms_abi__)) (*GetModuleInformation)(int64_t, int64_t,
                                                             void*, uint32_t);

static void
CustomCrashReport()
{
  kprintf("Rufe.org crash augmentation enabled\n");

  if (IsWindows()) {
    enum { MODULE_MAX = 32 };
    int64_t mod_list[MODULE_MAX];
    uint32_t byte_count;
    int pid = GetCurrentProcessId();
    int64_t handle =
        OpenProcess(kNtProcessVmRead | kNtProcessQueryInformation, 0, pid);
    if (EnumProcessModules(handle, mod_list, sizeof(mod_list), &byte_count)) {
      int module_count = MIN(byte_count / sizeof(int64_t), MODULE_MAX);
      kprintf("module_count %d\n", module_count);
      for (int it = 0; it < module_count; ++it) {
        struct NtModuleInfo modinfo;
        if (GetModuleInformation &&
            GetModuleInformation(handle, mod_list[it], &modinfo,
                                 sizeof(modinfo))) {
          kprintf(" Address Range %p - %p", modinfo.lpBaseOfDll,
                  modinfo.lpBaseOfDll + modinfo.SizeOfImage);
        }
        uint16_t wide_name[PATH_MAX];
        GetModuleBaseName(handle, mod_list[it], wide_name, PATH_MAX);
        kprintf(" %hs ", wide_name);
        kprintf("\n");
      }
      if (byte_count > sizeof(mod_list))
        kprintf("... MODULE_MAX exceeded: some modules are not listed!\n");
    }
    sleep(3);
  }

  switch (platform_phaseD) {
    case PLATFORM_PREGAME:
      // Crash during initialization switch to "software" renderer
      globalD.pc_renderer = 1;
      // Fall-thru; save-to-disk
    case PLATFORM_GAME:
      platformD.postgame();
      break;
  }
}

void
__game_crash(int sig, struct siginfo* si, void* arg)
{
  __oncrash(sig, si, arg);
  CustomCrashReport();
}

static void
InstallCrashHandler(int sig, int flags)
{
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGQUIT);
  sigaddset(&sa.sa_mask, SIGFPE);
  sigaddset(&sa.sa_mask, SIGILL);
  sigaddset(&sa.sa_mask, SIGSEGV);
  sigaddset(&sa.sa_mask, SIGTRAP);
  sigaddset(&sa.sa_mask, SIGBUS);
  sigaddset(&sa.sa_mask, SIGABRT);
  sa.sa_flags = SA_SIGINFO | flags;
  GetSymbolTable();
  sa.sa_sigaction = __game_crash;
  sigaction(sig, &sa, 0);
}

static void
crash_init()
{
  struct sigaltstack ss;
  static char crashstack[65536];
  FindDebugBinary();
  ss.ss_flags = 0;
  ss.ss_size = sizeof(crashstack);
  ss.ss_sp = crashstack;
  !sigaltstack(&ss, 0);

  if (IsWindows()) {
    void* psapi = cosmo_dlopen("psapi.dll", RTLD_LAZY);
    GetModuleInformation = cosmo_dlsym(psapi, "GetModuleInformation");
  }

  InstallCrashHandler(SIGQUIT, 0);
#ifdef __x86_64__
  InstallCrashHandler(SIGTRAP, 0);
#else
  InstallCrashHandler(SIGTRAP, SA_RESETHAND);
#endif
  InstallCrashHandler(SIGFPE, SA_RESETHAND);
  InstallCrashHandler(SIGILL, SA_RESETHAND);
  InstallCrashHandler(SIGBUS, SA_RESETHAND);
  InstallCrashHandler(SIGABRT, SA_RESETHAND);
  InstallCrashHandler(SIGSEGV, SA_RESETHAND | SA_ONSTACK);
}
#define MOD_CRASH 1
