#pragma once

enum { DISK = 0 };
enum { FONT = 0 };
enum { INPUT = 0 };
enum { COLOR = 0 };

// Override DISK/FONT/INPUT when included
#include "color.c"
#include "disk.c"
#include "font.c"
#include "input.c"

#include "art.c"
#include "icon.c"
#include "player.c"
#include "treasure.c"
#include "wall.c"

enum { UITEST = 0 };

enum { PADSIZE = (26 + 2) * 16 };
enum { AFF_X = 3 };
enum { AFF_Y = AL(active_affectD) / AFF_X };
enum { SPRITE_SQ = 32 };
enum { MAP_W = SYMMAP_WIDTH * ART_W };
enum { MAP_H = SYMMAP_HEIGHT * ART_H };
enum { MMSCALE = 2 };

DATA char moreD[] = "-more-";

DATA fn text_fnD;
DATA uint32_t sprite_idD;
DATA SDL_Surface* spriteD;
DATA SDL_Texture* sprite_textureD;
DATA SDL_Surface* mmsurfaceD;
DATA SDL_Texture* mmtextureD;
DATA SDL_Texture* ui_textureD;
DATA SDL_Surface* tpsurfaceD;
DATA SDL_Texture* tptextureD;
DATA SDL_Texture* map_textureD;
DATA SDL_Texture* text_textureD;
enum { STRLEN_MORE = AL(moreD) - 1 };

#define P(p) p.x, p.y
#define RF(r, framing)                                                    \
  (SDL_Rect)                                                              \
  {                                                                       \
    .x = r.x - (framing), .y = r.y - (framing), .w = r.w + 2 * (framing), \
    .h = r.h + 2 * (framing),                                             \
  }
#define U4(i) \
  (i & 0xff), ((i >> 8) & 0xff), ((i >> 16) & 0xff), ((i >> 24) & 0xff)

// hex RGBA to little endian
#define CHEX(x) __builtin_bswap32(x)
DATA uint32_t paletteD[] = {
    CHEX(0x00000000), CHEX(0xcc0000ff), CHEX(0x4e9a06ff), CHEX(0xc4a000ff),
    CHEX(0x3465a4ff), CHEX(0x75507bff), CHEX(0x06989aff), CHEX(0xd3d7cfff),
    CHEX(0x555753ff), CHEX(0xef2929ff), CHEX(0x8ae234ff), CHEX(0xfce94fff),
    CHEX(0x729fcfff), CHEX(0xad7fa8ff), CHEX(0x34e2e2ff), CHEX(0xeeeeecff),
};
DATA uint32_t rgbaD[AL(paletteD)];
DATA uint32_t lightingD[] = {
    CHEX(0x161616ff),
    CHEX(0x282828ff),
    CHEX(0x3c3c3cff),
    CHEX(0x505050ff),
};
static SDL_Color*
color_by_palette(c)
{
  return (SDL_Color*)&paletteD[c];
}

void
bitmap_yx_into_surface(void* bitmap, int64_t ph, int64_t pw, SDL_Point into,
                       struct SDL_Surface* surface)
{
  uint8_t bpp = surface->format->BytesPerPixel;
  uint8_t* pixels = surface->pixels;
  int64_t pitch = surface->pitch;
  uint8_t* src = bitmap;
  for (int64_t row = 0; row < ph; ++row) {
    uint8_t* dst = pixels + (pitch * (into.y + row)) + (bpp * into.x);
    for (int64_t col = 0; col < pw; ++col) {
      memcpy(dst, &rgbaD[*src & 0xff], bpp);
      src += 1;
      dst += bpp;
    }
  }
}
void
bitfield_to_bitmap(uint8_t* bitfield, uint8_t* bitmap, int64_t bitmap_size)
{
  int byte_count = bitmap_size / 8;
  for (int it = 0; it < byte_count; ++it) {
    for (int jt = 0; jt < 8; ++jt) {
      bitmap[it * 8 + jt] = ((bitfield[it] & (1 << jt)) != 0) * 15;
    }
  }
}

// art.c
#define MAX_ART 279
DATA uint8_t artD[96 * 1024];
DATA uint64_t art_usedD;
DATA uint32_t art_textureD[MAX_ART];
int
art_io()
{
  int rc;
  void* bytes = &artD;
  unsigned long size = AL(artD);
  unsigned long zsize = sizeof(artZ);
  rc = puff(bytes, &size, artZ, &zsize);
  art_usedD = size;
  return rc == 0;
}

static SDL_Point
point_by_spriteid(sprite_idD)
{
  int col = sprite_idD % SPRITE_SQ;
  int row = sprite_idD / SPRITE_SQ;
  return (SDL_Point){
      col * ART_W,
      row * ART_H,
  };
}

int
art_init()
{
  uint8_t bitmap[ART_H][ART_W];
  uint64_t art_size = (ART_W * ART_H / 8);
  int byte_count = art_usedD;
  uint64_t byte_used = 0;
  for (int it = 0; it < AL(art_textureD); ++it) {
    int offset = byte_used;
    byte_used += art_size;
    if (byte_used > byte_count) break;

    bitfield_to_bitmap(&artD[offset], &bitmap[0][0], ART_W * ART_H);
    bitmap_yx_into_surface(&bitmap[0][0], ART_H, ART_W,
                           point_by_spriteid(sprite_idD), spriteD);
    art_textureD[it] = sprite_idD++;
  }

  Log("art_init result %d", byte_used <= byte_count);

  return byte_used <= byte_count;
}

// treasure
#define MAX_TART 53
DATA uint8_t tartD[16 * 1024];
DATA uint64_t tart_usedD;
DATA uint32_t tart_textureD[MAX_TART];
int
tart_io()
{
  int rc;
  void* bytes = &tartD;
  unsigned long size = AL(tartD);
  unsigned long zsize = sizeof(treasureZ);
  rc = puff(bytes, &size, treasureZ, &zsize);
  tart_usedD = size;
  return rc == 0;
}

int
tart_init()
{
  uint8_t bitmap[ART_H][ART_W];
  uint64_t art_size = (ART_W * ART_H / 8);
  int byte_count = tart_usedD;
  uint64_t byte_used = 0;
  for (int it = 0; it < AL(tart_textureD); ++it) {
    int offset = byte_used;
    byte_used += art_size;
    if (byte_used > byte_count) break;

    bitfield_to_bitmap(&tartD[offset], &bitmap[0][0], ART_W * ART_H);
    bitmap_yx_into_surface(&bitmap[0][0], ART_H, ART_W,
                           point_by_spriteid(sprite_idD), spriteD);
    tart_textureD[it] = sprite_idD++;
  }

  Log("tart_init result %d", byte_used <= byte_count);

  return byte_used <= byte_count;
}

// wall
#define MAX_WART 6
DATA uint8_t wartD[4 * 1024];
DATA uint64_t wart_usedD;
DATA uint32_t wart_textureD[MAX_WART];
int
wart_io()
{
  int rc;
  void* bytes = &wartD;
  unsigned long size = AL(wartD);
  unsigned long zsize = sizeof(wallZ);
  rc = puff(bytes, &size, wallZ, &zsize);
  wart_usedD = size;
  return rc == 0;
}

int
wart_init()
{
  uint8_t bitmap[ART_H][ART_W];
  uint64_t art_size = (ART_W * ART_H / 8);
  int byte_count = wart_usedD;
  uint64_t byte_used = 0;
  for (int it = 0; it < AL(wart_textureD); ++it) {
    int offset = byte_used;
    byte_used += art_size;
    if (byte_used > byte_count) break;

    bitfield_to_bitmap(&wartD[offset], &bitmap[0][0], ART_W * ART_H);
    bitmap_yx_into_surface(&bitmap[0][0], ART_H, ART_W,
                           point_by_spriteid(sprite_idD), spriteD);
    wart_textureD[it] = sprite_idD++;
  }

  Log("wart_init result %d", byte_used <= byte_count);

  return byte_used <= byte_count;
}

// player
#define MAX_PART 13
DATA uint8_t partD[4 * 1024];
DATA uint64_t part_usedD;
DATA uint32_t part_textureD[MAX_PART];
int
part_io()
{
  int rc;
  void* bytes = &partD;
  unsigned long size = AL(partD);
  unsigned long zsize = sizeof(playerZ);
  rc = puff(bytes, &size, playerZ, &zsize);
  part_usedD = size;
  return rc == 0;
}

int
part_init()
{
  uint8_t bitmap[ART_H][ART_W];
  uint64_t art_size = (ART_W * ART_H / 8);
  int byte_count = part_usedD;
  uint64_t byte_used = 0;
  for (int it = 0; it < AL(part_textureD); ++it) {
    int offset = byte_used;
    byte_used += art_size;
    if (byte_used > byte_count) break;

    bitfield_to_bitmap(&partD[offset], &bitmap[0][0], ART_W * ART_H);
    bitmap_yx_into_surface(&bitmap[0][0], ART_H, ART_W,
                           point_by_spriteid(sprite_idD), spriteD);
    part_textureD[it] = sprite_idD++;
  }

  Log("part_init result %d", byte_used <= byte_count);

  return byte_used <= byte_count;
}

int
ui_init()
{
  enum { UI_W = 16 };
  enum { UI_H = 16 };
  USE(renderer);
  uint8_t bitmap[UI_H][UI_W];
  SDL_Surface* icon;

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

//  cos of (it * M_PI * 2 / 8);
DATA float cos_table[] = {1.000,  0.707,  0.000,  -0.707,
                          -1.000, -0.707, -0.000, 0.707};
static float
cos_lookup(idx)
{
  return cos_table[idx];
}
static float
sin_lookup(idx)
{
  idx += 6;
  idx %= AL(cos_table);
  return cos_table[idx];
}
static void surface_ppfill(surface) SDL_Surface* surface;
{
  uint8_t bpp = surface->format->BytesPerPixel;
  uint8_t* pixels = surface->pixels;
  for (int64_t row = 0; row < surface->h; ++row) {
    uint8_t* dst = pixels + (surface->pitch * row);
    for (int64_t col = 0; col < surface->w; ++col) {
      memcpy(dst, &rgbaD[nearest_pp(row, col)], bpp);
      dst += bpp;
    }
  }
}
static int
tp_init()
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
fs_upgrade()
{
  // Make a copy of all characters to external storage
  platformD.saveex();

  // Move default "savechar" into the class slot
  if (platformD.load(-1, 0)) {
    if (platformD.save(uD.clidx)) {
      platformD.erase(-1, 0);
    }
  }
}
int puff_io(out, outmax, in, insize) void* out;
void* in;
{
  unsigned long size = outmax;
  if (puff(out, &size, in, &(unsigned long){insize}) == 0) return size;
  return 0;
}
int
custom_pregame()
{
  platform_pregame();

  if (DISK && !disk_init()) return 1;
  if (FONT && !font_init()) return 2;
  if (INPUT && !input_init()) return 3;

  for (int it = 0; it < AL(paletteD); ++it) {
    rgbaD[it] = SDL_MapRGBA(pixel_formatD, U4(paletteD[it]));
  }

  if (ART_H * SPRITE_SQ <= max_texture_heightD &&
      ART_W * SPRITE_SQ <= max_texture_widthD) {
    spriteD =
        SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, ART_W * SPRITE_SQ,
                                       ART_H * SPRITE_SQ, 0, texture_formatD);
    if (spriteD) {
      if (!art_io() || !art_init()) return 4;
      if (!tart_io() || !tart_init()) return 4;
      if (!wart_io() || !wart_init()) return 4;
      if (!part_io() || !part_init()) return 4;

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

  if (PC) {
    enum { ICO_SZ = 128 };
    SDL_Surface* s = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, ICO_SZ,
                                                    ICO_SZ, 0, texture_formatD);
    if (s) {
      if (puff_io(s->pixels, s->h * s->pitch, AP(icoZ))) {
        Log("puff_io for icon OK\n");
        int pitch = s->pitch;
        uint8_t* pixels = s->pixels;
        for (int row = 0; row < ICO_SZ; ++row) {
          int* pptr = vptr(pixels + (row * pitch));
          for (int col = 0; col < ICO_SZ; ++col) {
            int rgb = rgb_by_labr(*pptr);
            if (rgb == -1) rgb = 0;
            *pptr = rgb;
            pptr += 1;
          }
        }

        SDL_SetWindowIcon(windowD, s);
        SDL_FreeSurface(s);
      }
    }
  }

  if (TOUCH) ui_init();
  if (TOUCH) tp_init();

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

  // Migration code
  if (platformD.load(-1, 0)) fs_upgrade();

  font_reset();

  if (globalD.orientation_lock) SDL_SetWindowResizable(windowD, 0);
  return 0;
}

int
custom_postgame(may_exit)
{
  if (DISK) disk_postgame(may_exit);
  return platform_postgame(may_exit);
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

// Text Drawing
static int
rect_altfill(r)
SDL_Rect r;
{
  int rx = r.x;
  int ry = r.y;
  int rh = r.h;
  int rw = r.w;
  uint32_t rc[] = {
      lightingD[1],
      lightingD[2],
  };
  for (int row = 0; row * FHEIGHT < rh; ++row) {
    int color = rc[row % 2];
    SDL_SetRenderDrawColor(rendererD, U4(color));
    SDL_Rect target = {
        rx,
        ry + row * FHEIGHT,
        rw,
        FHEIGHT,
    };
    SDL_RenderFillRect(rendererD, &target);
  }

  return 0;
}
void rect_innerframe(r) SDL_Rect r;
{
  SDL_SetRenderDrawColor(rendererD, U4(paletteD[BRIGHT + WHITE]));
  SDL_RenderDrawRect(rendererD, &r);
  SDL_RenderDrawRect(rendererD, &RF(r, -1));
  SDL_RenderDrawRect(rendererD, &RF(r, -3));
}
void rect_frame(r, scale) SDL_Rect r;
{
  int i = scale * 3;
  SDL_SetRenderDrawColor(rendererD, U4(paletteD[BRIGHT + WHITE]));
  SDL_RenderDrawRect(rendererD, &RF(r, i));
  SDL_RenderDrawRect(rendererD, &RF(r, i + 1));
  SDL_RenderDrawRect(rendererD, &RF(r, i + 3));
}
int
vitalstat_text()
{
  char tmp[80];
  int len = 0;
  AUSE(grect, GR_STAT);

  if (uD.lev) {
    rect_altfill(grect);
    if (uD.ridx < AL(raceD) && uD.clidx < AL(classD)) {
      int it = 0;
      len = snprintf(tmp, AL(tmp), "%s %s", raceD[uD.ridx].name,
                     classD[uD.clidx].name);
      SDL_Point p = {grect.x + grect.w / 2 - (len * FWIDTH) / 2,
                     grect.y + it * FHEIGHT + 1};
      if (len > 0) render_monofont_string(rendererD, &fontD, tmp, len, p);
    }
    for (int it = 0; it < MAX_A; ++it) {
      len = snprintf(tmp, AL(tmp), "%-4.04s: %7d %-4.04s: %6d", vital_nameD[it],
                     vitalD[it], stat_abbrD[it], vital_statD[it]);
      SDL_Point p = {grect.x + FWIDTH / 2, grect.y + it * FHEIGHT + FHEIGHT};
      if (len > 0) render_monofont_string(rendererD, &fontD, tmp, len, p);
    }
    {
      int it = MAX_A;
      len = snprintf(tmp, AL(tmp), "%-4.04s: %7d", vital_nameD[it], vitalD[it]);
      SDL_Point p = {grect.x + FWIDTH / 2, grect.y + it * FHEIGHT + FHEIGHT};
      if (len > 0) render_monofont_string(rendererD, &fontD, tmp, len, p);
    }

    char* affstr[AFF_X];
    for (int it = 0; it < AFF_Y; ++it) {
      for (int jt = 0; jt < AL(affstr); ++jt) {
        int idx = AL(affstr) * it + jt;
        if (UITEST) {
          affstr[jt] = affectD[idx][0];
        } else if (active_affectD[idx])
          affstr[jt] = affectD[idx][active_affectD[idx] - 1];
        else
          affstr[jt] = "";
      }

      len = snprintf(tmp, AL(tmp), "%-8.08s %-8.08s %-8.08s", affstr[0],
                     affstr[1], affstr[2]);
      SDL_Point p = {
          grect.x + FWIDTH / 2,
          grect.y + AL(vital_nameD) * FHEIGHT + it * FHEIGHT,
      };
      if (len > 0) render_monofont_string(rendererD, &fontD, tmp, len, p);
    }

    rect_innerframe(grect);
  }
  return 0;
}
int
common_text()
{
  USE(layout_rect);
  USE(renderer);
  char tmp[80];
  int len;
  {
    AUSE(grect, GR_VERSION);
    SDL_Point p = {grect.x, grect.y};
    render_monofont_string(renderer, &fontD, "moria", AL("moria"), p);
    p.y += FHEIGHT;
    render_monofont_string(renderer, &fontD, "version", AL("version"), p);
    p.y += FHEIGHT;
    render_monofont_string(renderer, &fontD, versionD + 10, AL(versionD) - 11,
                           p);
  }

  {
    AUSE(grect, GR_MINIMAP);
    {
      SDL_Point p = {grect.x + grect.w / 2, grect.y - FHEIGHT - 24};
      len = snprintf(tmp, AL(tmp), "turn:%7d", turnD);
      p.x -= (len * FWIDTH) / 2;
      render_monofont_string(renderer, &fontD, tmp, len, p);
    }

    {
      SDL_Point p = {grect.x + grect.w / 2, grect.y + grect.h + 24};
      len = snprintf(tmp, AL(tmp), "%s", dun_descD);
      p.x -= (len * FWIDTH) / 2;
      render_monofont_string(renderer, &fontD, tmp, len, p);
    }
  }

  if (TOUCH) {
    if (msg_moreD || UITEST) {
      static int tapD;
      tapD = (tapD + 1) % 4;
      switch (tapD) {
        case 0:
          font_color(*color_by_palette(RED));
          break;
        case 1:
          font_color(*color_by_palette(GREEN));
          break;
        case 2:
          font_color(*color_by_palette(BLUE));
          break;
        case 3:
          font_reset();
          break;
      }

      AUSE(grect, GR_PAD);
      SDL_Point p = {grect.x + grect.w / 2, grect.y + grect.h / 2};
      p.x -= STRLEN_MORE * FWIDTH / 2;
      p.y -= FHEIGHT / 2;
      render_monofont_string(renderer, &fontD, AP(moreD), p);
      font_reset();
    }
  }
  return 0;
}
int
portrait_text(mode)
{
  USE(renderer);
  USE(msg_more);

  if (mode == 0) {
    AUSE(grect, GR_GAMEPLAY);
    char* msg = AS(msg_cqD, msg_writeD);
    int msg_used = AS(msglen_cqD, msg_writeD);
    int alpha = FALPHA;

    SDL_Point p = {
        grect.x + FWIDTH / 2,
        grect.y + FHEIGHT / 2,
    };

    // Gameplay shows previous message to help the player out
    if (!msg_used) {
      msg = AS(msg_cqD, msg_writeD - 1);
      msg_used = AS(msglen_cqD, msg_writeD - 1);
      alpha = 150;
    }

    if (UITEST) {
      SDL_Rect msg_target = {p.x, p.y, msg_widthD * FWIDTH,
                             FHEIGHT + FHEIGHT / 8};
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
      SDL_RenderFillRect(renderer, &msg_target);
    }

    if (msg_used) {
      SDL_Rect fill = {p.x, p.y, msg_used * FWIDTH, FHEIGHT + FHEIGHT / 8};
      SDL_SetRenderDrawColor(rendererD, 0, 0, 0, 0);
      SDL_RenderFillRect(renderer, &fill);

      font_texture_alphamod(alpha);
      render_monofont_string(renderer, &fontD, msg, msg_used, p);
      font_reset();
    }

    if (msg_more || UITEST) {
      SDL_Rect r = {
          grect.x + grect.w - STRLEN_MORE * FWIDTH - FWIDTH / 2,
          grect.y + grect.h - FHEIGHT - FHEIGHT / 2,
          STRLEN_MORE * FWIDTH,
          FHEIGHT + FHEIGHT / 8,
      };
      SDL_Rect r2 = {
          grect.x + FWIDTH / 2,
          grect.y + grect.h - FHEIGHT - FHEIGHT / 2,
          STRLEN_MORE * FWIDTH,
          FHEIGHT + FHEIGHT / 8,
      };
      SDL_SetRenderDrawColor(rendererD, 0, 0, 0, 0);
      SDL_RenderFillRect(renderer, &r);
      SDL_RenderFillRect(renderer, &r2);
      render_monofont_string(rendererD, &fontD, AP(moreD),
                             (SDL_Point){r.x, r.y});
      render_monofont_string(rendererD, &fontD, AP(moreD),
                             (SDL_Point){r2.x, r2.y});
    }

    common_text();
  }

  return 0;
}
int
landscape_text(mode)
{
  USE(msg_more);
  USE(renderer);
  USE(layout_rect);

  if (mode == 0) {
    char* msg = AS(msg_cqD, msg_writeD);
    int msg_used = AS(msglen_cqD, msg_writeD);
    int alpha = FALPHA;

    // Show previous message to help the player out
    if (!msg_used) {
      msg = AS(msg_cqD, msg_writeD - 1);
      msg_used = AS(msglen_cqD, msg_writeD - 1);
      alpha = 128;
    }

    SDL_Point p = {layout_rect.w / 2 - msg_used * FWIDTH / 2, 0};
    SDL_Rect rect = {
        p.x - FWIDTH / 2,
        p.y,
        (1 + msg_used) * FWIDTH,
        FHEIGHT,
    };
    if (UITEST) {
      SDL_Rect text_target = {
          layout_rect.w / 2 - (msg_widthD * FWIDTH / 2) - FWIDTH / 2,
          p.y,
          (1 + msg_widthD) * FWIDTH,
          FHEIGHT,
      };
      SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
      SDL_RenderFillRect(renderer, &text_target);
    }

    if (msg_used) {
      font_texture_alphamod(alpha);
      render_monofont_string(renderer, &fontD, msg, msg_used, p);
      font_reset();

      SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
      rect_frame(rect, 1);
    }

    if (msg_more || UITEST) {
      int wlimit = msg_widthD * FWIDTH;
      int mlimit = STRLEN_MORE * FWIDTH;
      int margin = (layout_rect.w - wlimit - mlimit) / 4;

      SDL_Rect rect2 = {
          margin,
          0,
          mlimit,
          FHEIGHT,
      };
      SDL_Rect rect3 = {
          layout_rect.w - margin - mlimit,
          0,
          mlimit,
          FHEIGHT,
      };

      SDL_Point p2 = {rect2.x, rect2.y};
      render_monofont_string(renderer, &fontD, AP(moreD), p2);
      SDL_Point p3 = {rect3.x, rect3.y};
      render_monofont_string(renderer, &fontD, AP(moreD), p3);
      rect_frame(rect2, 1);
      rect_frame(rect3, 1);
    }

    common_text();
  } else {
    vitalstat_text();
  }

  return 0;
}

// Drawing
int
map_draw(zoom_prect)
SDL_Rect* zoom_prect;
{
  SDL_Rect dest_rect;
  SDL_Rect sprite_src;
  SDL_Point rp;
  dest_rect.w = ART_W;
  dest_rect.h = ART_H;
  SDL_SetRenderTarget(rendererD, map_textureD);
  SDL_SetRenderDrawColor(rendererD, 0, 0, 0, 0);
  SDL_SetRenderDrawBlendMode(rendererD, SDL_BLENDMODE_BLEND);
  int imagine = countD.imagine;
  for (int row = 0; row < SYMMAP_HEIGHT; ++row) {
    dest_rect.y = row * ART_H;
    for (int col = 0; col < SYMMAP_WIDTH; ++col) {
      dest_rect.x = col * ART_W;

      struct vizS* viz = &vizD[row][col];
      char sym = viz->sym;
      uint64_t fidx = viz->floor;
      uint64_t light = viz->light;
      uint64_t dim = viz->dim;
      uint64_t cridx = viz->cr;
      uint64_t tridx = viz->tr;

      // Art priority creature, wall, treasure, fallback to symmap ASCII
      SDL_Texture* srct = 0;
      SDL_Rect* srcr = 0;

      if (sym == '@') {
        rp = (SDL_Point){col, row};
      }

      if (!imagine) {
        if (cridx && cridx <= AL(art_textureD)) {
          sprite_src = (SDL_Rect){
              P(point_by_spriteid(art_textureD[cridx - 1])),
              ART_W,
              ART_H,
          };
          srct = sprite_textureD;
        } else if (fidx && fidx <= AL(wart_textureD)) {
          sprite_src = (SDL_Rect){
              P(point_by_spriteid(wart_textureD[fidx - 1])),
              ART_W,
              ART_H,
          };

          srct = sprite_textureD;
        } else if (tridx && tridx <= AL(tart_textureD)) {
          sprite_src = (SDL_Rect){
              P(point_by_spriteid(tart_textureD[tridx - 1])),
              ART_W,
              ART_H,
          };

          srct = sprite_textureD;
        } else if (sym == '@') {
          sprite_src = (SDL_Rect){
              P(point_by_spriteid(part_textureD[0 + (turnD) % 2])),
              ART_W,
              ART_H,
          };

          srct = sprite_textureD;
        }
      }

      if (srct) {
        srcr = &sprite_src;
      } else {
        srct = font_texture_by_char(sym);
      }

      SDL_SetRenderDrawColor(rendererD, U4(lightingD[light]));
      SDL_RenderFillRect(rendererD, &dest_rect);

      if (srct) {
        if (dim) SDL_SetTextureColorMod(srct, 192, 192, 192);
        SDL_RenderCopy(rendererD, srct, srcr, &dest_rect);
        if (dim) SDL_SetTextureColorMod(srct, 255, 255, 255);
      }
      switch (viz->fade) {
        case 1:
          SDL_SetRenderDrawColor(rendererD, 0, 0, 0, 32);
          SDL_RenderFillRect(rendererD, &dest_rect);
          break;
        case 2:
          SDL_SetRenderDrawColor(rendererD, 0, 0, 0, 64);
          SDL_RenderFillRect(rendererD, &dest_rect);
          break;
        case 3:
          SDL_SetRenderDrawColor(rendererD, 0, 0, 0, 98);
          SDL_RenderFillRect(rendererD, &dest_rect);
          break;
      }
    }
  }

  if (sprite_textureD) {
    uint32_t oidx = caveD[uD.y][uD.x].oidx;
    struct objS* obj = &entity_objD[oidx];
    uint32_t tval = obj->tval;
    dest_rect.y = rp.y * ART_H;
    dest_rect.x = rp.x * ART_W;

    if (tval - 1 < TV_MAX_PICK_UP || tval == TV_CHEST) {
      sprite_src = (SDL_Rect){
          P(point_by_spriteid(part_textureD[2])),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(rendererD, sprite_textureD, &sprite_src, &dest_rect);
    } else if (tval == TV_GLYPH) {
      sprite_src = (SDL_Rect){
          P(point_by_spriteid(part_textureD[3])),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(rendererD, sprite_textureD, &sprite_src, &dest_rect);
    } else if (tval == TV_VIS_TRAP) {
      sprite_src = (SDL_Rect){
          P(point_by_spriteid(part_textureD[4])),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(rendererD, sprite_textureD, &sprite_src, &dest_rect);
    }

    if (countD.paralysis) {
      sprite_src = (SDL_Rect){
          P(point_by_spriteid(part_textureD[5])),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(rendererD, sprite_textureD, &sprite_src, &dest_rect);
    }
    if (countD.poison) {
      sprite_src = (SDL_Rect){
          P(point_by_spriteid(part_textureD[6])),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(rendererD, sprite_textureD, &sprite_src, &dest_rect);
    }
    if (maD[MA_SLOW]) {
      sprite_src = (SDL_Rect){
          P(point_by_spriteid(part_textureD[7])),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(rendererD, sprite_textureD, &sprite_src, &dest_rect);
    }
    if (maD[MA_BLIND]) {
      sprite_src = (SDL_Rect){
          P(point_by_spriteid(part_textureD[8])),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(rendererD, sprite_textureD, &sprite_src, &dest_rect);
    }
    if (countD.confusion) {
      sprite_src = (SDL_Rect){
          P(point_by_spriteid(part_textureD[9])),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(rendererD, sprite_textureD, &sprite_src, &dest_rect);
    }
    if (maD[MA_FEAR]) {
      sprite_src = (SDL_Rect){
          P(point_by_spriteid(part_textureD[10])),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(rendererD, sprite_textureD, &sprite_src, &dest_rect);
    }
    if (uD.food < PLAYER_FOOD_FAINT) {
      sprite_src = (SDL_Rect){
          P(point_by_spriteid(part_textureD[12])),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(rendererD, sprite_textureD, &sprite_src, &dest_rect);
    } else if (uD.food <= PLAYER_FOOD_ALERT) {
      sprite_src = (SDL_Rect){
          P(point_by_spriteid(part_textureD[11])),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(rendererD, sprite_textureD, &sprite_src, &dest_rect);
    }
  }

  int zf, zh, zw;
  zf = globalD.zoom_factor;
  zh = SYMMAP_HEIGHT >> zf;
  zw = SYMMAP_WIDTH >> zf;
  // give equal vision on each side of the player during zoom
  int zsymmetry = (zf != 0);

  int zy, zx;
  zy = CLAMP(rp.y - zh / 2, 0, SYMMAP_HEIGHT - zh - zsymmetry);
  zx = CLAMP(rp.x - zw / 2, 0, SYMMAP_WIDTH - zw - zsymmetry);
  zoom_prect->x = zx * ART_W;
  zoom_prect->y = zy * ART_H;
  zoom_prect->w = (zw + zsymmetry) * ART_W;
  zoom_prect->h = (zh + zsymmetry) * ART_H;

  SDL_SetRenderDrawColor(rendererD, U4(paletteD[BRIGHT + WHITE]));
  SDL_RenderDrawRect(rendererD, zoom_prect);

  SDL_SetRenderDrawBlendMode(rendererD, SDL_BLENDMODE_NONE);
  SDL_SetRenderTarget(rendererD, layoutD);
  return 0;
}
int
draw_mode0()
{
  USE(renderer);
  USE(msg_more);
  USE(minimap_enlarge);

  int show_minimap = (maD[MA_BLIND] == 0);
  int show_game = 1;

  vitalstat_text();

  {
    USE(mmtexture);

    if (show_minimap) {
      AUSE(grect, GR_MINIMAP);
      USE(mmsurface);
      bitmap_yx_into_surface(&minimapD[0][0], MAX_HEIGHT, MAX_WIDTH,
                             (SDL_Point){0, 0}, mmsurface);
      SDL_UpdateTexture(mmtexture, NULL, mmsurface->pixels, mmsurface->pitch);

      SDL_RenderCopy(rendererD, mmtexture, NULL, &grect);
      rect_frame(grect, 3);
    }

    if (show_game) {
      AUSE(grect, GR_GAMEPLAY);
      if (minimap_enlarge) {
        SDL_RenderCopy(rendererD, mmtexture, NULL, &grect);
      } else {
        SDL_Rect zoom_rect;
        map_draw(&zoom_rect);
        SDL_RenderCopy(rendererD, map_textureD, &zoom_rect, &grect);
      }
    }
  }
}
int
draw_mode2()
{
  USE(mode);
  USE(renderer);
  USE(overlay_width);
  USE(overlay_height);
  char* msg = AS(msg_cqD, msg_writeD);
  int msg_used = AS(msglen_cqD, msg_writeD);
  int is_text;
  AUSE(grect, GR_OVERLAY);

  if (mode == 1) {
    is_text = 1;
  } else {
    is_text = (screen_submodeD != 0);
    if (screen_submodeD == 2) {
      grect = grectD[GR_WIDESCREEN];
      overlay_width = msg_widthD;
    }
  }

  SDL_Rect src_rect = {
      0,
      0,
      overlay_width * FWIDTH,
      overlay_height * FHEIGHT,
  };
  SDL_SetRenderTarget(renderer, text_textureD);
  if (is_text) {
    rect_altfill(src_rect);
  } else {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderFillRect(renderer, &src_rect);
  }

  int left = FWIDTH / 2;
  if (msg_used)
    render_monofont_string(renderer, &fontD, msg, msg_used,
                           (SDL_Point){left, 0});

  switch (mode) {
    case 1: {
      for (int row = 0; row < AL(overlayD); ++row) {
        SDL_Point p = {
            left,
            row * FHEIGHT + FHEIGHT,
        };
        char* text = overlayD[row];
        int tlen = overlay_usedD[row];
        if (TOUCH && row == finger_rowD) {
          font_color(*color_by_palette(RED));
          if (tlen <= 1) {
            text = "-";
            tlen = 1;
          }
        }
        render_monofont_string(renderer, &fontD, text, tlen, p);
        if (TOUCH && row == finger_rowD) {
          font_reset();
        }
      }
    } break;
    case 2: {
      for (int row = 0; row < AL(screenD); ++row) {
        SDL_Point p = {left, row * FHEIGHT + FHEIGHT};
        render_monofont_string(renderer, &fontD, screenD[row],
                               screen_usedD[row], p);
      }
    } break;
  }

  SDL_SetRenderTarget(renderer, layoutD);
  SDL_RenderCopy(renderer, text_textureD, &src_rect, &grect);
  if (is_text) rect_innerframe(grect);
}
int
custom_draw()
{
  USE(mode);
  USE(renderer);
  USE(layout);

  if (layout) SDL_SetRenderTarget(renderer, layout);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  if (TOUCH && tpsurfaceD) {
    {
      AUSE(grect, GR_PAD);
      SDL_RenderCopy(rendererD, tptextureD, 0, &grect);
    }

    int bc[] = {RED, GREEN};
    for (int it = 0; it < MAX_BUTTON; ++it) {
      AUSE(grect, GR_BUTTON1 + it);
      SDL_SetRenderDrawColor(rendererD, U4(paletteD[bc[it]]));
      SDL_RenderFillRect(rendererD, &grect);
    }

    if (ui_textureD && mode != 1) {
      AUSE(grect, GR_HISTORY);
      SDL_RenderCopy(renderer, ui_textureD, NULL, &grect);
      rect_frame(grect, 1);
    }
    if (mode == 0) {
      AUSE(grect, GR_LOCK);
      MUSE(global, orientation_lock);
      int tridx;
      if (orientation_lock) {
        tridx = 48;
      } else {
        tridx = 45;
      }
      if (tridx <= AL(tart_textureD)) {
        SDL_Rect sprite_rect = {
            P(point_by_spriteid(tart_textureD[tridx - 1])),
            ART_W,
            ART_H,
        };
        SDL_Rect dest = {
            grect.x,
            grect.y,
            ART_W * 2,
            ART_H * 2,
        };
        dest.x += (grect.w - dest.w) / 2;
        SDL_RenderCopy(renderer, sprite_textureD, &sprite_rect, &dest);
      }
      rect_frame(grect, 1);
    }
  }

  if (mode == 0)
    draw_mode0();
  else
    draw_mode2();

  if (text_fnD) text_fnD(mode);

  SDL_RenderFlush(renderer);
  return platform_draw();
}
int
obj_viz(obj, viz)
struct objS* obj;
struct vizS* viz;
{
  if (obj->tval != TV_INVIS_TRAP) viz->sym = obj->tchar;
  switch (obj->tval) {
      // Misc
    case TV_MISC:
      return 25;
    case TV_CHEST:
      if (obj->idflag & ID_REVEAL && obj->flags & CH_TRAPPED) return 33;
      if (obj->flags & CH_LOCKED) return 34;
      if (obj->sn == SN_EMPTY) return 35;
      return 32;
    case TV_SPIKE:
      return 50;
    case TV_PROJECTILE:
      return 13;
    case TV_LIGHT:
      return 21;
    case TV_LAUNCHER:
      return 15;
      // Worn
    case TV_HAFTED:
      return 24;
    case TV_POLEARM:
      return 6;
    case TV_SWORD:
      return 14;
    case TV_DIGGING:
      return 10;
    case TV_BOOTS:
      return 22;
    case TV_GLOVES:
      return 23;
    case TV_CLOAK:
      return 20;
    case TV_HELM:
      if (obj->subval <= 5)
        return 11;  // helm
      else
        return 39;  // crown
    case TV_SHIELD:
      return 2;
    case TV_HARD_ARMOR:
      return 9;
    case TV_SOFT_ARMOR:
      return 1;
    case TV_AMULET:
      return 3;
    case TV_RING:
      return 7;
      // Activate
    case TV_STAFF:
      return 12;
    case TV_WAND:
      return 5;
    case TV_SCROLL1:
    case TV_SCROLL2:
      return 8;
    case TV_POTION1:
    case TV_POTION2:
    case TV_FLASK:
      return 4;
    case TV_FOOD:
      if ((obj->subval & 0x3f) <= 20)
        return 40;  // mushroom/mold
      else
        return 19;  // food
    case TV_MAGIC_BOOK:
      return 18;
    case TV_PRAYER_BOOK:
      return 17;
      // Gold
      // TBD: copper/silver/gold/mithril/gems by subval
    case TV_GOLD:
      return 16;
    /* Dungeon Fixtures */
    case TV_VIS_TRAP:
      if (obj->tchar == ' ')
        viz->light = 0;
      else
        return 26;
      break;
    case TV_RUBBLE:
      return 27;
    case TV_OPEN_DOOR:
      return 28;
    case TV_CLOSED_DOOR:
      if (obj->p1 == 0 || (obj->idflag & ID_REVEAL) == 0)
        return 29;
      else if (obj->p1 > 0)
        return 36;  // locked
      else if (obj->p1 < 0)
        return 37;  // stuck
    case TV_UP_STAIR:
      return 30;
    case TV_DOWN_STAIR:
      return 31;
    case TV_SECRET_DOOR:
      viz->floor = 2;
      break;
    case TV_GLYPH:
      return 38;
  }
  return 0;
}
static int
cave_color(row, col)
{
  struct caveS* c_ptr;
  struct monS* mon;
  struct objS* obj;
  int color = 0;

  c_ptr = &caveD[row][col];
  mon = &entity_monD[c_ptr->midx];
  if (mon->mlit) {
    color = BRIGHT + MAGENTA;
  } else if (c_ptr->fval == BOUNDARY_WALL) {
    color = BRIGHT + WHITE;
  } else if (CF_LIT & c_ptr->cflag && c_ptr->fval >= MIN_WALL) {
    color = BRIGHT + WHITE;
  } else if (CF_LIT & c_ptr->cflag) {
    color = BRIGHT + BLACK;
  }

  if (color <= BRIGHT + BLACK && c_ptr->oidx) {
    obj = &entity_objD[c_ptr->oidx];
    if (CF_VIZ & c_ptr->cflag) {
      if (obj->tval == TV_UP_STAIR) {
        color = BRIGHT + GREEN;
      } else if (obj->tval == TV_DOWN_STAIR) {
        color = BRIGHT + RED;
      } else if (obj->tval == TV_VIS_TRAP || obj->tval == TV_RUBBLE) {
        color = BRIGHT + YELLOW;
      } else if (obj->tval == TV_SECRET_DOOR) {
        color = BRIGHT + WHITE;
      } else if (obj->tval == TV_CLOSED_DOOR) {
        color = BRIGHT + BLACK;
      } else if (obj->tval != 0 && obj->tval <= TV_MAX_PICK_UP) {
        color = BRIGHT + BLUE;
      } else if (obj->tval == TV_STORE_DOOR || obj->tval == TV_PAWN_DOOR) {
        color = BRIGHT + GREEN;
      }
    }
  }

  if (color <= BRIGHT + BLACK && (CF_TEMP_LIGHT & c_ptr->cflag)) {
    color = BRIGHT + CYAN;
  }

  return color;
}
static int
fade_by_distance(y1, x1, y2, x2)
{
  int dy = y1 - y2;
  int dx = x1 - x2;
  int sq = dx * dx + dy * dy;

  if (sq <= 1) return 1;
  if (sq <= 4) return 2;
  if (sq <= 9) return 3;
  return 4;
}
static void
viz_update()
{
  int blind, py, px;
  int rmin = panelD.panel_row_min;
  int rmax = panelD.panel_row_max;
  int cmin = panelD.panel_col_min;
  int cmax = panelD.panel_col_max;

  struct vizS* vptr = &vizD[0][0];
  blind = maD[MA_BLIND];
  py = uD.y;
  px = uD.x;
  for (int row = rmin; row < rmax; ++row) {
    for (int col = cmin; col < cmax; ++col) {
      struct caveS* c_ptr = &caveD[row][col];
      struct monS* mon = &entity_monD[c_ptr->midx];
      struct objS* obj = &entity_objD[c_ptr->oidx];
      struct vizS viz = {0};
      if (row != py || col != px) {
        viz.light = (c_ptr->cflag & CF_SEEN) != 0;
        viz.fade = fade_by_distance(py, px, row, col) - 1;
        if (mon->mlit) {
          viz.cr = mon->cidx;
          viz.sym = creatureD[mon->cidx].cchar;
        } else if (blind) {
          // May have MA_DETECT resulting in lit monsters above
          // No walls, objects, or lighting
          viz.light = 0;
          viz.fade = 3;
        } else if (CF_VIZ & c_ptr->cflag) {
          switch (c_ptr->fval) {
            case GRANITE_WALL:
            case BOUNDARY_WALL:
              viz.sym = '#';
              viz.floor = 1;
              break;
            case QUARTZ_WALL:
              viz.sym = '#';
              viz.floor = 3 + (c_ptr->oidx != 0);
              break;
            case MAGMA_WALL:
              viz.sym = '#';
              viz.floor = 5 + (c_ptr->oidx != 0);
              break;
            case FLOOR_OBST:
              viz.tr = obj_viz(obj, &viz);
              break;
            default:
              viz.light += ((CF_LIT & c_ptr->cflag) != 0);
              viz.dim = obj->tval && los(py, px, row, col) == 0;
              viz.tr = obj_viz(obj, &viz);
              break;
          }
        }
      } else {
        viz.sym = '@';
        viz.light = 3;
      }

      *vptr++ = viz;
    }
  }
}
void
viz_minimap_stair(row, col, color)
{
  row = CLAMP(row, 2, MAX_HEIGHT - 1);
  col = CLAMP(col, 1, MAX_WIDTH - 2 - 1);
  minimapD[row][col] = color;
  minimapD[row][col - 1] = color;
  minimapD[row - 1][col] = color;
  minimapD[row - 1][col + 1] = color;
  minimapD[row - 2][col + 1] = color;
  minimapD[row - 2][col + 2] = color;
}
void
viz_minimap()
{
  int rmin = panelD.panel_row_min;
  int rmax = panelD.panel_row_max;
  int cmin = panelD.panel_col_min;
  int cmax = panelD.panel_col_max;
  int color;

  if (minimap_enlargeD && dun_level) {
    for (int row = 0; row < MAX_HEIGHT; ++row) {
      for (int col = 0; col < MAX_WIDTH; ++col) {
        color = cave_color(row, col);

        if ((row >= rmin && row <= rmax) && (col >= cmin && col <= cmax) &&
            color == 0) {
          color = BRIGHT + BLACK;
        }

        minimapD[row][col] = color;
        if (color == BRIGHT + GREEN || color == BRIGHT + RED)
          viz_minimap_stair(row, col, color);
      }
    }
  } else {
    enum { RATIO = MAX_WIDTH / SYMMAP_WIDTH };
    for (int row = 0; row < MAX_HEIGHT / RATIO; ++row) {
      for (int col = 0; col < MAX_WIDTH / RATIO; ++col) {
        color = cave_color(row + rmin, col + cmin);
        for (int i = 0; i < RATIO; ++i) {
          for (int j = 0; j < RATIO; ++j) {
            minimapD[row * RATIO + i][col * RATIO + j] = color;
          }
        }
      }
    }
  }
}
static void
overlay_autoselect()
{
  int row = finger_rowD;
  if (overlay_usedD[row] <= 1) {
    for (int it = row + 1; it < AL(overlay_usedD); it += 1) {
      if (overlay_usedD[it] > 1) {
        finger_rowD = it;
        return;
      }
    }

    for (int it = row - 1; it > 0; --it) {
      if (overlay_usedD[it] > 1) {
        finger_rowD = it;
        return;
      }
    }

    finger_rowD = 0;
  }
}

static int
mode_change()
{
  int subprev = submodeD;
  int subnext = overlay_submodeD;
  int mprev = modeD;
  int mnext;

  if (screen_usedD[0])
    mnext = 2;
  else if (overlay_usedD[0])
    mnext = 1;
  else
    mnext = 0;

  if (mprev != mnext || subprev != subnext) {
    if (mprev == 1) ui_stateD[subprev] = finger_rowD;

    if (mnext == 1) {
      finger_rowD = (subnext > 0) ? ui_stateD[subnext] : 0;
      finger_colD = (subnext == 'e') ? 1 : 0;

      overlay_autoselect();
    }
  }

  modeD = mnext;
  submodeD = subnext;
  memcpy(overlay_copyD, overlay_usedD, sizeof(overlay_copyD));

  return mnext;
}
int
custom_predraw()
{
  mode_change();
  viz_update();
  viz_minimap();
  return 1;
}

int
custom_orientation(orientation)
{
  platform_orientation(orientation);

  if (orientation == SDL_ORIENTATION_PORTRAIT) {
    layout_rectD = (SDL_Rect){0, 0, PORTRAIT_X, PORTRAIT_Y};
    text_fnD = portrait_text;
    overlay_widthD = 67;
    overlay_heightD = AL(overlayD) + 1;
    msg_widthD = 63;
    portrait_layout();
  } else if (orientation == SDL_ORIENTATION_LANDSCAPE) {
    layout_rectD = (SDL_Rect){0, 0, LANDSCAPE_X, LANDSCAPE_Y};
    text_fnD = landscape_text;
    overlay_widthD = 78;
    overlay_heightD = AL(overlayD) + 2;
    msg_widthD = 92;
    landscape_layout();
  } else {
    layout_rectD = display_rectD;
    text_fnD = 0;
  }
  return 0;
}
