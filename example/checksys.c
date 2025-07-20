#include "src/game.c"
enum { MOD_SAVECHAR = 0 };
enum { TEST_REPLAY = 0 };
enum { TEST_CAVEGEN = 0 };
#include "src/mod/savechar.c"

int
global_init(int argc, char** argv)
{
  globalD.ghash = -1;
}
int
custom_gamecrash_handler(int sig)
{
}
#include "platform/sdl2/platform.c"

#define Log(x, ...) printf(x "\n", ##__VA_ARGS__)
// #include "platform/sdl2/platform.c"
// 0: nt console app
// 1: nt window app
enum { COSMO_WINDOWAPP = RELEASE };
enum { COSMO_CRASH = 0 };
enum { COSMO_LOG = 1 };
enum { STEAM = 0 };
#include "platform/sdl2/asset/icon.c"
#include "platform/sdl2/cosmo/cosmo-crash.c"
#include "platform/sdl2/cosmo/cosmo-init.h"
enum { DISK = 0 };
#include "platform/sdl2/disk.c"
#define noop(x) int x() __attribute__((alias("noop")))
noop(dpad_nearest_pp);
noop(touch_selection);
noop(overlay_begin);
noop(overlay_end);
noop(column_transition);
noop(overlay_bisect);
noop(overlay_input);
enum { PADSIZE = (26 + 2) * 16 };
DATA int pp_keyD[9] = {5, 6, 3, 2, 1, 4, 7, 8, 9};
#include "platform/sdl2/joystick.c"
#include "platform/sdl2/puff_stream.c"

#include <libc/nt/dll.h>

int puff_io(out, outmax, in, insize) void* out;
void* in;
{
  unsigned long size = outmax;
  if (puff(out, &size, in, &(unsigned long){insize}) == 0) return size;
  return 0;
}
int
main(int argc, char** argv)
{
  global_init(argc, argv);
  platform_init();
  if (platformD.pregame() == 0) {
    printf("polling test\n");
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      printf("  %x\n", event.type);
    }

    printf("testing puff+longjmp ");
    char pixel[128 * 128 * 4];
    printf(puff_io(AP(pixel), AP(rgb_icoZ)) ? "ok\n" : "fail\n");

    // TBD: texture tests:
    //   SDL_PIXELFORMAT_INDEX1LSB
    //   SDL_PIXELFORMAT_INDEX8
    //   SDL_PIXELFORMAT_ABGR8888
    //   texture_formatD

    // SDL i/o test
    printf("SDL i/o test\n");
    if (DISK) {
      uint64_t bufsz = 2 * 1024 * 1024;
      uint8_t* buf = malloc(bufsz);
      int disk = 0;
      uint64_t sdl2_hash = 0;
      if (buf) {
        SDL_RWops* readf = file_access("SDL2.dll", "rb");
        if (readf) {
          uint64_t sz = SDL_RWsize(readf);
          if (sz < bufsz) disk = SDL_RWread(readf, &buf, sz, 1);
          if (disk) sdl2_hash = djb2(DJB2, buf, sz);
          SDL_RWclose(readf);
        }
      }
      printf("buf 0xjx disk %d sdl2_hash 0x%jx\n", buf, disk, sdl2_hash);
      free(buf);
    }

    // review locale && disk path buffer length checks
    if (DISK) {
      printf("testing disk cache for global settings ");
      printf(disk_pregame() ? "ok\n" : "fail\n");
    }

    if (JOYSTICK) {
      globalD.use_joystick = 1;
      printf("testing joystick_init ");
      printf(joystick_init() ? "ok\n" : "fail\n");

      printf("polling w/ joystick test\n");
      SDL_Event event[8];
      int used = 0;
      while (SDL_PollEvent(&event[used])) {
        printf("  %x\n", event[used].type);
        used += (event[used].type == SDL_JOYDEVICEADDED);
      }

      while (used) {
        used -= 1;
        printf("assign joystick %d\n", used);
        sdl_joystick_device(event[used]);
      }

      printf("polling w/ joystick assignment\n");
      for (int it = 0; it < 8; ++it) {
        if (SDL_PollEvent(&event[0])) printf("  %x\n", event[0].type);
      }

      printf("joystick unassign, release\n");
      globalD.use_joystick = 0;
      joystick_update();
    }
  }

  int postgame = platformD.postgame();
  show(postgame);

  return 0;
}
