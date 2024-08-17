
#include <libc/limits.h>
#include <libc/log/internal.h>
#include <libc/macros.internal.h>
#include <libc/nt/enum/processaccess.h>
#include "libc/calls/struct/sigaction.h"
#include "libc/calls/struct/sigaltstack.h"
#include "libc/calls/struct/sigset.h"
#include "libc/sysv/consts/sa.h"
#include "libc/sysv/consts/sig.h"

#include "libc/calls/struct/ucontext.internal.h"
#include "libc/calls/ucontext.h"

struct NtModuleInfo {
  void* lpBaseOfDll;
  uint32_t SizeOfImage;
  void* EntryPoint;
};

static uint32_t
    __attribute__((__ms_abi__)) (*GetModuleInformation)(int64_t, int64_t, void*,
                                                        uint32_t);
static uint16_t
    __attribute__((__ms_abi__)) (*RtlCaptureStackBackTrace)(uint32_t, uint32_t,
                                                            void*, uint32_t*);

GLOBAL int platform_phaseD;
STATIC int
platform_phase(p)
{
  platform_phaseD = p;
  return 0;
}

static void
WindowsCrashReport(ucontext_t* ctx)
{
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

  uint64_t pc = 0;
  if (ctx) {
    pc = ctx->uc_mcontext.PC;
    if (pc > UINT32_MAX) {
      // kprintf("Foreign code backtrace:\n");
      // enum { STACK_MAX = 64 };
      // void* stack[STACK_MAX];
      // uint32_t hash;
      // int stack_count = RtlCaptureStackBackTrace(0, STACK_MAX, stack, &hash);
      // for (int it = 0; it < stack_count; ++it) {
      //   kprintf(" %p\n", stack[it]);
      // }
      // kprintf("Hash: 0x%x stack_count %d\n", hash, stack_count);
    }
  }

  sleep(3);
}

static void
CommonCrashReport(ucontext_t* ctx)
{
  kprintf("Rufe.org crash augmentation enabled\n");

  switch (platform_phaseD) {
    case PLATFORM_PREGAME:
      // Crash during initialization switch to "software" renderer
      memcpy(globalD.pc_renderer, AP("software"));
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
  CommonCrashReport(arg);
  if (IsWindows()) WindowsCrashReport(arg);
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

    // void* ntdll = cosmo_dlopen("ntdll.dll", RTLD_LAZY);
    // RtlCaptureStackBackTrace = cosmo_dlsym(ntdll,
    // "RtlCaptureStackBackTrace");
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
#define COSMO_CRASH 1
