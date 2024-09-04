// #include cosmo_sdl.c twice
// First overrides names to rewrite code written against SDL2
// Second provides bootstrapping within steam's runtime
// Second is AFTER all custom game & platform interaction.
//
#ifndef COSMO_SDL
#define COSMO_SDL
GLOBAL void* libD;

#include <cosmo.h>
#include <dlfcn.h>
#include <libc/nt/dll.h>  // SDL OverrideSO
#include <math.h>

#include <libc/errno.h>

#include "cosmo/cosmo-abi.h"
#include "cosmo/cosmo-sdl.h"
#define COSMO 1
#else  // COSMO_SDL
// 0: nt console app
// 1: nt window app
enum { COSMO_WINDOWAPP = 1 };
#include "cosmo/cosmo-crash.c"
#include "cosmo/cosmo-init.h"
#endif  // COSMO_SDL
