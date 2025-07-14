// Rufe.org LLC 2022-2025: ISC License
#include "libc/calls/struct/sigaction.h"
#include "libc/sysv/consts/sa.h"
#include "libc/sysv/consts/sig.h"

int siglistD[] = {
    SIGQUIT, SIGTRAP, SIGFPE, SIGILL, SIGBUS, SIGABRT, SIGSEGV,
};

static void
cosmo_sig_flags(fn handler, int sig, int flags)
{
  struct sigaction sa = {0};
  for (int it = 0; it < AL(siglistD); ++it)
    sigaddset(&sa.sa_mask, siglistD[it]);
  sa.sa_flags = flags;
  sa.sa_handler = vptr(handler);
  sigaction(sig, &sa, 0);
}

static void
cosmo_crashinit(fn handler)
{
  for (int it = 0; it < AL(siglistD); ++it) {
    cosmo_sig_flags(handler, siglistD[it], 0);
  }
}
#define COSMO_CRASH 1
