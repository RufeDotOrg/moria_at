// Convert screenshots to iOS format
#define main _alt_main
#include "src/moria_at.c"
#undef main

#include <stdio.h>

float
calc(w, h, goalw, goalh)
{
  float xscale = (float)goalw / w;
  float yscale = (float)goalh / h;
  return MIN(xscale, yscale);
}

int
exit_with_usage()
{
  printf("screenshot <source .nmg>");
  exit(1);
}
int
reorient(point_t* arr, int len)
{
  for (int it = 0; it < len; ++it) {
    int tmp = arr[it].x ^ arr[it].y;
    arr[it].x ^= tmp;
    arr[it].y ^= tmp;
  }
}

int
main(int argc, char** argv)
{
  global_init(argc, argv);
  platform_init();

  if (argc < 2) exit_with_usage();

  for (int argit = 1; argit < argc; ++argit) {
    struct padS header;
    char* srcname = argv[argit];
    FILE* f = fopen(srcname, "rb");
    if (!f) exit_with_usage();
    fread(&header, sizeof(header), 1, f);

    if (strcmp("nmg", header.fourcc) != 0) exit_with_usage();

    int srcw = header.width;
    int srch = header.height;
    int sz = srch * srcw * 1.5f;
    if (platformD.pregame() == 0) {
      USE(renderer);

      SDL_Texture* srct =
          SDL_CreateTexture(renderer, SDL_PIXELFORMAT_NV12,
                            SDL_TEXTUREACCESS_STREAMING, srcw, srch);
      int srcpitch = srcw;
      void* srcpixels;
      if (srct && SDL_LockTexture(srct, 0, &srcpixels, &srcpitch) == 0) {
        printf("%p pixels %d srcpitch %d sz\n", srcpixels, srcpitch, sz);
        if (1 != fread(srcpixels, sz, 1, f)) return 30;
        SDL_UnlockTexture(srct);
      } else {
        printf("source texture lock failure\n");
        return 2;
      }
      printf("--->source texture OK\n");
      SDL_SetTextureScaleMode(srct, SDL_ScaleModeLinear);
      // for (int it = 0; it < 1e9; ++it) {
      //   SDL_SetRenderTarget(renderer, 0);
      //   SDL_RenderCopy(renderer, srct, 0, 0);
      //   SDL_RenderPresent(renderer);
      //   if (platformD.input() == CTRL('c')) return 0;
      // }

      point_t ios_saferect[] = {
          {2580, 1200},  // iPhone 16 Pro Max
          {2672, 2014},  // iPad Pro 13"
      };
      point_t ios_displayrect[] = {
          {2868, 1320},
          {2752, 2064},
      };
      char* fname[] = {"iphone", "ipad"};

      if (srch > srcw) reorient(AP(ios_saferect));
      if (srch > srcw) reorient(AP(ios_displayrect));

      for (int it = 0; it < AL(ios_saferect); ++it) {
        int drw = ios_displayrect[it].x;
        int drh = ios_displayrect[it].y;
        SDL_Texture* t = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
                                           SDL_TEXTUREACCESS_TARGET, drw, drh);

        SDL_SetRenderTarget(renderer, t);

        int sfw = ios_saferect[it].x;
        int sfh = ios_saferect[it].y;
        float scale = calc(srcw, srch, sfw, sfh);
        rect_t dst = {0, 0, srcw * scale, srch * scale};
        dst.x += (drw - dst.w) / 2;
        dst.y += (drh - dst.h) / 2;
        printf(
            "ios displayrect %dx%d ios saferect %dx%d scale %.03f offset "
            "%dx%d\n",
            drw, drh, sfw, sfh, scale, dst.x, dst.y);
        SDL_RenderCopy(renderer, srct, 0, &dst);
        SDL_RenderPresent(renderer);

        int dsz = drw * drh * 1.5f;
        void* pixels = malloc(dsz);
        int pitch = drw;
        if (pixels) {
          SDL_RenderReadPixels(renderer, 0, SDL_PIXELFORMAT_NV12, pixels,
                               pitch);

          struct padS header = {"nmg", drw, drh};
          // tbd: better io interface
          char filename[512];
          int len = snprintf(AP(filename), "%s_%s", fname[it], srcname);
          FILE* f = fopen(filename, "wb");
          if (f) {
            fwrite(&header, sizeof(header), 1, f);
            fwrite(pixels, dsz, 1, f);
            fclose(f);
          }
          printf("read complete\n");
          free(pixels);
        } else {
          printf("alloc fail\n");
        }
      }
    }
    fclose(f);
  }

  return platformD.postgame(1);
}
