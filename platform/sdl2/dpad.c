// Rufe.org LLC 2022-2024: ISC License

enum { PADSIZE = (26 + 2) * 16 };

DATA SDL_Texture* tptextureD;
DATA SDL_Point ppD[9];
DATA int pp_keyD[9] = {5, 6, 3, 2, 1, 4, 7, 8, 9};
DATA float limit_dsqD;
DATA int dpad_colorD[] = {0xff1b0196, 0xff01559d, 0xff089ba4, 0xff109c02,
                          0xff8c9f0c, 0xffa55906, 0xff9e026d, 0xff62029b};
DATA int dpad_greyscaleD[] = {0xff303030, 0xff3f3f3f, 0xff525252, 0xff636363,
                              0xff767676, 0xff888888, 0xff9d9d9d, 0xffb0b0b0};

//  cos of (it * M_PI * 2 / 8);
DATA float cos_table[] = {1.000,  0.707,  0.000,  -0.707,
                          -1.000, -0.707, -0.000, 0.707};
STATIC float
cos_lookup(idx)
{
  return cos_table[idx];
}
STATIC float
sin_lookup(idx)
{
  idx += 6;
  idx %= AL(cos_table);
  return cos_table[idx];
}
STATIC int
dpad_nearest_pp(y, x, po_dsq)
int* po_dsq;
{
  int r = -1;
  int64_t min_dsq = INT64_MAX;
  for (int it = 0; it < AL(ppD); ++it) {
    int ppx = ppD[it].x;
    int ppy = ppD[it].y;
    int dx = ppx - x;
    int dy = ppy - y;
    int dsq = dx * dx + dy * dy;
#define TOUCH_CARDINAL_WEIGHT .43f
    if (it % 2 == 1) dsq *= TOUCH_CARDINAL_WEIGHT;
    if (dsq < min_dsq) {
      min_dsq = dsq;
      r = it;
    }
  }
  if (po_dsq) *po_dsq = min_dsq;
  return r;
}
STATIC int
dpad_init()
{
  int cx = PADSIZE / 2;
  int cy = PADSIZE / 2;
  ppD[0] = (SDL_Point){cx, cy};

  enum { PPDIST = 3 * PADSIZE / 8 };
  for (int it = 0; it < 8; ++it) {
    int ox = cos_lookup(it) * PPDIST;
    int oy = sin_lookup(it) * PPDIST;
    ppD[1 + it].x = cx + ox;
    ppD[1 + it].y = cy + oy;
  }

  return 0;
}
STATIC void dpadfill_pixels_pitch(pixels, pitch) uint8_t* pixels;
{
  MUSE(global, dpad_sensitivity);
  MUSE(global, dpad_color);

  int bgcolor = lightingD[1];
  int* color = dpad_color ? dpad_colorD : dpad_greyscaleD;

  int limit_dsq = dpad_sensitivity * dpad_sensitivity;
  if (dpad_sensitivity >= 99) limit_dsq = INT32_MAX;
  for (int64_t row = 0; row < PADSIZE; ++row) {
    int* dst = vptr(pixels + row * pitch);
    for (int64_t col = 0; col < PADSIZE; ++col) {
      int dsq;
      int n = dpad_nearest_pp(row, col, &dsq);
      int c = bgcolor;

      if (dsq > limit_dsq) n = 0;
      if (n > 0) {
        // flips the diagonals to provide contrast
        if (n % 2 == 1)
          c = color[n - 1];
        else
          c = color[(n - 1 + 4) % 8];
      }

      *dst++ = c;
    }
  }

  // Input limiter
  limit_dsqD = limit_dsq;
}
int
dpad_classic()
{
  SDL_Texture* tp = tptextureD;
  if (!tp)
    tp = SDL_CreateTexture(rendererD, SDL_PIXELFORMAT_ABGR8888,
                           SDL_TEXTUREACCESS_STREAMING, PADSIZE, PADSIZE);

  void* pix;
  int pitch;
  if (SDL_LockTexture(tp, 0, &pix, &pitch) == 0) {
    dpadfill_pixels_pitch(pix, pitch);
    SDL_UnlockTexture(tp);
  }
  SDL_SetTextureBlendMode(tp, SDL_BLENDMODE_NONE);
  tptextureD = tp;

  return 0;
}
