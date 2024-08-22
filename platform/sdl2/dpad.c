
enum { PADSIZE = (26 + 2) * 16 };

DATA SDL_Surface* tpsurfaceD;
DATA SDL_Texture* tptextureD;

DATA SDL_Point ppD[9];
DATA int pp_keyD[9] = {5, 6, 3, 2, 1, 4, 7, 8, 9};
DATA float limit_dsqD;

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

STATIC void surface_ppfill(surface) SDL_Surface* surface;
{
  MUSE(global, dpad_sensitivity);
  if (dpad_sensitivity >= 99)
    limit_dsqD = (float)PADSIZE * PADSIZE;
  else
    limit_dsqD = (float)dpad_sensitivity * dpad_sensitivity;

  uint8_t bpp = surface->format->BytesPerPixel;
  uint8_t* pixels = surface->pixels;
  for (int64_t row = 0; row < surface->h; ++row) {
    uint8_t* dst = pixels + (surface->pitch * row);
    for (int64_t col = 0; col < surface->w; ++col) {
      int dsq;
      int n = dpad_nearest_pp(row, col, &dsq);
      int color = lightingD[1];

      int lum = CLAMP(dpad_sensitivity - sqrtf(dsq), 0, 99);

      if (globalD.dpad_color) {
        if (dpad_sensitivity >= 99) lum = MAX(1, lum);
        if (n > 0 && lum > 0) {
          int labr = 0;
          // flips the diagonals to provide contrast
          if (n % 2 == 1)
            labr = dark_labrD[n - 1];
          else
            labr = dark_labrD[(n - 1 + 4) % 8];

          bptr(&labr)[0] += (CLAMP(lum / 4, 0, 31) + 4);
          color = rgb_by_labr(labr);
        }
      } else {
        color = rgb_by_labr(lum);
      }
      if (pixel_formatD) pixel_convert(&color);

      memcpy(dst, &color, bpp);
      dst += bpp;
    }
  }
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

  if (tpsurfaceD) SDL_FreeSurface(tpsurfaceD);
  tpsurfaceD = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, PADSIZE, PADSIZE,
                                              0, texture_formatD);
  if (tptextureD) SDL_DestroyTexture(tptextureD);
  tptextureD = SDL_CreateTexture(rendererD, texture_formatD,
                                 SDL_TEXTUREACCESS_STREAMING, PADSIZE, PADSIZE);
  SDL_SetTextureBlendMode(tptextureD, SDL_BLENDMODE_NONE);
  surface_ppfill(tpsurfaceD);
  SDL_UpdateTexture(tptextureD, NULL, tpsurfaceD->pixels, tpsurfaceD->pitch);

  return 0;
}
