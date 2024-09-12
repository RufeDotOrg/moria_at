// Rufe.org LLC 2022-2024: ISC License with local modifications
// ISC License
//
// Copyright 2020 Justine Alexandra Roberts Tunney
//
// Permission to use, copy, modify, and/or distribute this software for
// any purpose with or without fee is hereby granted, provided that the
// above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
// WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
// AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
// PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
// TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <libc/nt/enum/processaccess.h>
#include "libc/calls/struct/sigaction.h"
#include "libc/sysv/consts/sa.h"
#include "libc/sysv/consts/sig.h"

#include "libc/calls/struct/ucontext.internal.h"
#include "libc/calls/ucontext.h"

enum { MAX_MODULE = 64 };
struct NtModuleInfo {
  void* lpBaseOfDll;
  uint32_t SizeOfImage;
  void* EntryPoint;
};

typedef uint32_t
    __attribute__((__ms_abi__)) (*GetModuleInformationT)(int64_t, int64_t,
                                                         void*, uint32_t);

static void
cosmo_moduleinfo(GetModuleInformationT info_fn)
{
  int64_t mlist[MAX_MODULE];
  uint32_t byte_count;

  if (info_fn) {
    int pid = GetCurrentProcessId();
    int64_t handle =
        OpenProcess(kNtProcessVmRead | kNtProcessQueryInformation, 0, pid);
    if (EnumProcessModules(handle, mlist, sizeof(mlist), &byte_count)) {
      int module_count = MIN(byte_count / sizeof(int64_t), MAX_MODULE);
      kprintf("module_count %d\n", module_count);
      for (int it = 0; it < module_count; ++it) {
        struct NtModuleInfo modinfo;
        if (info_fn(handle, mlist[it], &modinfo, sizeof(modinfo))) {
          kprintf(" Address Range %p - %p", modinfo.lpBaseOfDll,
                  modinfo.lpBaseOfDll + modinfo.SizeOfImage);
        }
        uint16_t wide_name[PATH_MAX];
        GetModuleBaseName(handle, mlist[it], wide_name, PATH_MAX);
        kprintf(" %hs ", wide_name);
        kprintf("\n");
      }
      if (byte_count > sizeof(mlist))
        kprintf("... MAX_MODULE exceeded: some modules are not listed!\n");
    }

    if (!COSMO_WINDOWAPP) sleep(3);
  }
}

void
__game_crash(int sig, struct siginfo* si, void* arg)
{
  USE(phase);
  // Crash during initialization; switch to "software" renderer
  if (DISK && phase == PHASE_PREGAME) {
    memcpy(globalD.pc_renderer, AP("software"));
    disk_cache_write();
  }

  // Crash during play; attempt flush to disk
  if (DISK && phase == PHASE_GAME) disk_postgame();

  // Crash during postgame; noop

  // Cosmo crash handler
  __oncrash(sig, si, arg);

  if (IsWindows()) {
    kprintf("Rufe.org WindowsCrashReport enabled\n");
    void* psapi = cosmo_dlopen("psapi.dll", RTLD_LAZY);
    if (psapi) cosmo_moduleinfo(cosmo_dlsym(psapi, "GetModuleInformation"));
  }
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
cosmo_crashinit()
{
  struct sigaltstack ss;
  static char crashstack[65536];
  FindDebugBinary();
  ss.ss_flags = 0;
  ss.ss_size = sizeof(crashstack);
  ss.ss_sp = crashstack;
  !sigaltstack(&ss, 0);

  InstallCrashHandler(SIGQUIT, 0);
  InstallCrashHandler(SIGTRAP, 0);
  InstallCrashHandler(SIGFPE, 0);
  InstallCrashHandler(SIGILL, 0);
  InstallCrashHandler(SIGBUS, 0);
  InstallCrashHandler(SIGABRT, 0);
  InstallCrashHandler(SIGSEGV, SA_ONSTACK);
}
