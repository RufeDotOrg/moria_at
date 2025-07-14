#include "src/game.c"
enum { MOD_SAVECHAR = 0 };
enum { TEST_REPLAY = 0 };
enum { TEST_CAVEGEN = 0 };
#include "src/mod/savechar.c"

int
global_init(int argc, char** argv)
{
}
int
custom_gamecrash_handler(int sig)
{
}
#include "platform/sdl2/platform.c"

// #include "platform/sdl2/platform.c"
// 0: nt console app
// 1: nt window app
enum { COSMO_WINDOWAPP = RELEASE };
enum { COSMO_CRASH = 0 };
enum { COSMO_LOG = 1 };
#include "platform/sdl2/cosmo/cosmo-crash.c"
#include "platform/sdl2/cosmo/cosmo-init.h"

#include <libc/nt/dll.h>

int
sdl_joystick_event()
{
}
int
sdl_axis_motion()
{
}
int
sdl_joystick_device()
{
}
int
main(int argc, char** argv)
{
  global_init(argc, argv);
  platform_init();
  if (platformD.pregame() == 0) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      printf("  %x\n", event.type);
    }
  }

  int postgame = platformD.postgame();
  show(postgame);

  return 0;
}
