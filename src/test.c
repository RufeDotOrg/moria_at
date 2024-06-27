
#include "game.c"

#include "platform/platform.c"

int
readansi()
{
  SDL_Event event;
  int ret = 0;

  while (ret == 0 && SDL_PollEvent(&event)) {
    if (event.type == SDL_KEYDOWN) {
      if (char_alpha(event.key.keysym.sym)) {
        ret = event.key.keysym.sym;
      }
    } else if (event.type == SDL_QUIT) {
      ret = CTRL('c');
    }
    // } else if (event.type == SDL_WINDOWEVENT) {
    //   ret = sdl_window_event(event);
    // }
  }

  if (ret == 0) {
    nanosleep(&(struct timespec){0, 8e6}, 0);
    if (PC) ret = CTRL('d');
  }

  return ret;
}

int
play()
{
  char c;
  do {
    c = readansi();
    if (c > ' ') printf("%d (%c) ", c, c);
    switch (c) {
    }
    platform_draw();
  } while (c != CTRL('c'));
  return 0;
}

// Maybe replace platform_init instead of having prior global_init?
int
global_init(int argc, char** argv)
{
}

#ifdef __FATCOSMOCC__
#include "platform/cosmo-init.h"
#endif

int
main(int argc, char** argv)
{
  global_init(argc, argv);
  platform_init();
  platform_pregame();

  play();

  return platform_postgame(0);
}
