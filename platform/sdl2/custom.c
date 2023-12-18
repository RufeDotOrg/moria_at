
#define SAVENAME "savechar"

int
ui_init()
{
  enum { UI_W = 16 };
  enum { UI_H = 16 };
  USE(renderer);
  uint8_t bitmap[UI_H][UI_W];
  SDL_Surface *icon;

  icon = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, UI_W, UI_H, 0,
                                        texture_formatD);
  if (icon) {
    for (int row = 0; row < UI_H; ++row) {
      for (int col = 0; col < UI_W; ++col) {
        int rmod = row % 4;
        if (rmod == 3) {
          bitmap[row][col] = 0;
        } else {
          bitmap[row][col] = 7;
        }
      }
    }
    for (int it = 0; it < 4; ++it) {
      bitmap[it * 4 + 1][1] = 0;
    }

    bitmap_yx_into_surface(&bitmap[0][0], UI_H, UI_W, (SDL_Point){0, 0}, icon);
    ui_textureD = SDL_CreateTextureFromSurface(renderer, icon);
    SDL_FreeSurface(icon);
  }
  return icon != 0;
}

static int
input_init()
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
  tptextureD = SDL_CreateTexture(rendererD, 0, SDL_TEXTUREACCESS_STREAMING,
                                 PADSIZE, PADSIZE);
  SDL_SetTextureBlendMode(tptextureD, SDL_BLENDMODE_NONE);
  surface_ppfill(tpsurfaceD);
  SDL_UpdateTexture(tptextureD, NULL, tpsurfaceD->pixels, tpsurfaceD->pitch);

  return 0;
}

int
custom_pregame()
{
  if (ART_H * SPRITE_SQ <= max_texture_heightD &&
      ART_W * SPRITE_SQ <= max_texture_widthD) {
    spriteD =
        SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, ART_W * SPRITE_SQ,
                                       ART_H * SPRITE_SQ, 0, texture_formatD);
    if (spriteD) {
      if (!art_io() || !art_init()) return 3;
      if (!tart_io() || !tart_init()) return 3;
      if (!wart_io() || !wart_init()) return 3;
      if (!part_io() || !part_init()) return 3;

      if (sprite_idD < SPRITE_SQ * SPRITE_SQ) {
        sprite_textureD = SDL_CreateTextureFromSurface(rendererD, spriteD);
        if (sprite_textureD)
          SDL_SetTextureBlendMode(sprite_textureD, SDL_BLENDMODE_BLEND);
      } else {
        Log("WARNING: Assets exceed available sprite memory");
      }
      SDL_FreeSurface(spriteD);
      spriteD = 0;
    }
  }

  if (TOUCH) ui_init();
  if (TOUCH) input_init();

  mmsurfaceD = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, MAX_WIDTH,
                                              MAX_HEIGHT, 0, texture_formatD);
  mmtextureD = SDL_CreateTexture(rendererD, 0, SDL_TEXTUREACCESS_STREAMING,
                                 MAX_WIDTH, MAX_HEIGHT);
  SDL_SetTextureBlendMode(mmtextureD, SDL_BLENDMODE_NONE);
  map_textureD = SDL_CreateTexture(rendererD, texture_formatD,
                                   SDL_TEXTUREACCESS_TARGET, MAP_W, MAP_H);
  SDL_SetTextureBlendMode(map_textureD, SDL_BLENDMODE_NONE);
  text_textureD = SDL_CreateTexture(
      rendererD, texture_formatD, SDL_TEXTUREACCESS_TARGET, 2 * 1024, 2 * 1024);

  layout_rectD = (SDL_Rect){0, 0, PORTRAIT_X, PORTRAIT_Y};
}

static int
portrait_layout()
{
  USE(layout_rect);
  int margin = (layout_rect.w - MAP_W) / 2;

  grectD[GR_VERSION] = (SDL_Rect){
      layout_rect.w - 8 * FWIDTH,
      layout_rect.h - 3 * FHEIGHT,
      FWIDTH * 8,
      FHEIGHT * 3,
  };

  int lift = FHEIGHT / 2;
  grectD[GR_PAD] = (SDL_Rect){
      margin,
      layout_rect.h - PADSIZE - lift,
      PADSIZE,
      PADSIZE,
  };

  int size = PADSIZE / 2;
  grectD[GR_BUTTON1] = (SDL_Rect){layout_rect.w - margin - size * 2,
                                  grectD[GR_PAD].y + size, size, size};
  grectD[GR_BUTTON2] =
      (SDL_Rect){layout_rect.w - margin - size, grectD[GR_PAD].y, size, size};

  grectD[GR_GAMEPLAY] = (SDL_Rect){
      margin,
      layout_rect.h - PADSIZE - MAP_H - FHEIGHT + FHEIGHT / 4,
      MAP_W,
      MAP_H,
  };
  grectD[GR_MINIMAP] = (SDL_Rect){
      layout_rect.w - margin - MMSCALE * MAX_WIDTH - 4 * FWIDTH,
      grectD[GR_GAMEPLAY].y - 2 * FHEIGHT - MMSCALE * MAX_HEIGHT,
      MMSCALE * MAX_WIDTH,
      MMSCALE * MAX_HEIGHT,
  };
  grectD[GR_HISTORY] = (SDL_Rect){
      layout_rect.w - 128 - margin - 4 * FWIDTH,
      FHEIGHT / 2,
      128,
      128,
  };
  grectD[GR_LOCK] = grectD[GR_HISTORY];
  grectD[GR_LOCK].x -= 2 * grectD[GR_HISTORY].w;

  grectD[GR_STAT] = (SDL_Rect){
      margin,
      0,
      PADSIZE + 3,
      (8 + 5) * FHEIGHT,
  };

  int olimit = overlay_widthD * FWIDTH;
  grectD[GR_OVERLAY] = (SDL_Rect){
      (layout_rect.w - olimit) / 2,
      0,  // grectD[GR_GAMEPLAY].y,
      olimit,
      layout_rect.h - PADSIZE - FHEIGHT,  // overlay_heightD*FHEIGHT
  };
  grectD[GR_WIDESCREEN] = grectD[GR_OVERLAY];

  return 0;
}
static int
landscape_layout()
{
  USE(layout_rect);
  int xmargin = (layout_rect.w - MAP_W) / 2;
  int ymargin = FHEIGHT + 8;

  grectD[GR_VERSION] = (SDL_Rect){
      layout_rect.w - 8 * FWIDTH,
      layout_rect.h - 3 * FHEIGHT,
      FWIDTH * 8,
      FHEIGHT * 3,
  };

  int lift = (layout_rect.h - (8 + 5) * FHEIGHT - PADSIZE - ymargin) / 2;
  grectD[GR_PAD] = (SDL_Rect){
      0,
      layout_rect.h - PADSIZE - lift,
      PADSIZE,
      PADSIZE,
  };

  int size = PADSIZE / 2;
  grectD[GR_BUTTON1] = (SDL_Rect){
      layout_rect.w - size * 2,
      grectD[GR_PAD].y + size,
      size,
      size,
  };
  grectD[GR_BUTTON2] = (SDL_Rect){
      layout_rect.w - size,
      grectD[GR_PAD].y,
      size,
      size,
  };

  grectD[GR_GAMEPLAY] = (SDL_Rect){
      xmargin,
      ymargin,
      MAP_W,
      MAP_H,
  };
  grectD[GR_MINIMAP] = (SDL_Rect){
      (layout_rect.w - PADSIZE) + (PADSIZE - MMSCALE * MAX_WIDTH) / 2,
      FHEIGHT * 5,
      MMSCALE * MAX_WIDTH,
      MMSCALE * MAX_HEIGHT,
  };
  grectD[GR_HISTORY] = (SDL_Rect){
      layout_rect.w - size / 2 - 96 / 2,
      layout_rect.h / 2 - 128 - FHEIGHT,
      96,
      128,
  };
  grectD[GR_LOCK] = grectD[GR_HISTORY];
  grectD[GR_LOCK].x -= grectD[GR_HISTORY].w * 2;

  grectD[GR_STAT] = (SDL_Rect){
      0,
      ymargin,
      PADSIZE,
      (8 + 5) * FHEIGHT,
  };

  grectD[GR_OVERLAY] = (SDL_Rect){
      xmargin,
      0,
      overlay_widthD * FWIDTH,
      overlay_heightD * FHEIGHT,
  };
  grectD[GR_WIDESCREEN] = (SDL_Rect){
      xmargin,
      0,
      layout_rect.w - xmargin,
      overlay_heightD * FHEIGHT,
  };

  return 0;
}

int
custom_orientation(orientation)
{
  if (orientation == SDL_ORIENTATION_PORTRAIT) {
    overlay_widthD = 67;
    overlay_heightD = AL(overlayD) + 1;
    msg_widthD = 63;
    portrait_layout();
  } else if (orientation == SDL_ORIENTATION_LANDSCAPE) {
    overlay_widthD = 78;
    overlay_heightD = AL(overlayD) + 2;
    msg_widthD = 92;
    landscape_layout();
  }
  return 0;
}

int
custom_setup()
{
  customD.orientation = custom_orientation;
  customD.pregame = custom_pregame;
}
