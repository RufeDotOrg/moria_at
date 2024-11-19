// Rufe.org LLC 2022-2024: ISC License
#pragma once

enum { DISK = 0 };
enum { FONT = 0 };
enum { COLOR = 0 };
enum { PUFF_STREAM = 0 };

// Override COLOR/DISK/FONT/PUFF_STREAM when included
#include "color.c"
#include "disk.c"
#include "font.c"
#include "puff_stream.c"

#include "asset/art.c"
#include "asset/icon.c"
#include "asset/player.c"
#include "asset/treasure.c"
#include "asset/wall.c"

#include "asset/scancode.c"

// Game specific inclusion
#include "dpad.c"

#include "touch.c"

#include "joystick.c"

enum { TEST_UI = 0 };

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
DATA SDL_Texture* mmtextureD;
DATA SDL_Texture* ui_textureD;
DATA SDL_Texture* map_textureD;
DATA SDL_Texture* text_textureD;
DATA uint16_t mon_drawD[AL(monD)];

enum { PHASE_PREGAME = 1, PHASE_GAME = 2, PHASE_POSTGAME = 3 };
DATA int phaseD;

enum { STRLEN_MORE = AL(moreD) - 1 };

#define RF(r, framing)                                                    \
  (rect_t)                                                                \
  {                                                                       \
    .x = r.x - (framing), .y = r.y - (framing), .w = r.w + 2 * (framing), \
    .h = r.h + 2 * (framing),                                             \
  }

void
bitmap_yx_into_surface(void* bitmap, int64_t ph, int64_t pw, SDL_Point into,
                       struct SDL_Surface* surface)
{
  uint8_t bpp = surface->format->BytesPerPixel;
  uint8_t* pixels = surface->pixels;
  int64_t pitch = surface->pitch;
  int color = -1;
  if (pixel_formatD) pixel_convert(&color);
  uint8_t* src = bitmap;
  for (int64_t row = 0; row < ph; ++row) {
    uint8_t* dst = pixels + (pitch * (into.y + row)) + (bpp * into.x);
    for (int64_t col = 0; col < pw; ++col) {
      if (*src) memcpy(dst, &color, bpp);
      src += 1;
      dst += bpp;
    }
  }
}

static SDL_Point
point_by_spriteid(uint32_t id)
{
  int col = id % SPRITE_SQ;
  int row = id / SPRITE_SQ;
  return (SDL_Point){
      col * ART_W,
      row * ART_H,
  };
}
DATA int art_textureD;
DATA int tart_textureD;
DATA int wart_textureD;
DATA int part_textureD;

enum { DECODE = ART_W * ART_H / 8 };
_Static_assert(ART_W / 8 == sizeof(int), "use memcpy below instead");
int art_decode(buf, len) uint8_t* buf;
{
  int offset = 0;
  int id = 0;
  USE(sprite);
  USE(sprite_id);
  uint8_t* pixels = sprite->pixels;
  int pitch = sprite->pitch;

  while (len >= DECODE) {
    rect_t drect = {XY(point_by_spriteid(sprite_id + id)), ART_W, ART_H};
    // bitfield adjustment
    drect.x /= 8;
    // copy to sprite (no conversion)
    for (int it = 0; it < ART_H; ++it) {
      int* write = iptr(pixels + (drect.y + it) * pitch + drect.x);
      *write = *iptr(&buf[offset + 4 * it]);
    }

    id += 1;
    len -= DECODE;
    offset += DECODE;
  }

  Log("art_decode [%d->%d sprite_id] %d %d offset len", sprite_id,
      sprite_id + id, offset, len);
  sprite_idD = sprite_id + id;
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
  if (COSMO) phaseD = PHASE_PREGAME;
  if (DISK && !disk_pregame()) return 1;
  if (DISK && KEYBOARD)
    disk_read_keys(gameplay_inputD, sizeof(gameplay_inputD));

  platform_pregame();

  if (FONT && !font_init()) return 2;

  SDL_Surface* sprite = SDL_CreateRGBSurfaceWithFormat(
      SDL_SWSURFACE, ART_W * SPRITE_SQ, ART_H * SPRITE_SQ, 0,
      SDL_PIXELFORMAT_INDEX1LSB);
  if (sprite) {
    {
      SDL_Palette* palette = sprite->format->palette;
      *(int*)&palette->colors[0] = 0;
      *(int*)&palette->colors[1] = -1;
    }

    spriteD = sprite;
    art_textureD = sprite_idD - 1;
    if (puffex_stream_len(art_decode, AP(artZ)) != 0) return 4;
    tart_textureD = sprite_idD - 1;
    if (puffex_stream_len(art_decode, AP(treasureZ)) != 0) return 4;
    wart_textureD = sprite_idD - 1;
    if (puffex_stream_len(art_decode, AP(wallZ)) != 0) return 4;
    part_textureD = sprite_idD;  // no offset
    if (puffex_stream_len(art_decode, AP(playerZ)) != 0) return 4;

    if (sprite_idD < SPRITE_SQ * SPRITE_SQ) {
      // SDL2 determines texture format
      sprite_textureD = SDL_CreateTextureFromSurface(rendererD, sprite);
      if (sprite_textureD) {
        SDL_SetTextureBlendMode(sprite_textureD, SDL_BLENDMODE_BLEND);
        uint32_t fmt;
        SDL_QueryTexture(sprite_textureD, &fmt, 0, 0, 0);
        Log("sprite_textureD format 0x%x", fmt);
        if (!RELEASE)
          Log("sprite_textureD format: %s", SDL_GetPixelFormatName(fmt));
      } else {
        Log("WARNING: Unable to CreateTextureFromSurface for sprites");
      }
    } else {
      Log("WARNING: Assets exceed available sprite memory");
    }
    SDL_FreeSurface(sprite);
  }
  spriteD = 0;

  // Software renderer will skip this; limiting risk on troubled systems
  if (PC && globalD.pc_renderer[0] != 's') {
    enum { ICO_SZ = 128 };
    SDL_Surface* s = SDL_CreateRGBSurfaceWithFormat(
        SDL_SWSURFACE, ICO_SZ, ICO_SZ, 0, SDL_PIXELFORMAT_ARGB8888);
    if (s) {
      if (puff_io(s->pixels, s->h * s->pitch, AP(rgb_icoZ))) {
        Log("puff_io for icon OK\n");
        SDL_SetWindowIcon(windowD, s);
        Log("SetWindowIcon OK");
      }
      SDL_FreeSurface(s);
    }
  }
  if (PC) SDL_SetWindowTitle(windowD, "moria_at");
  if (PC && !MOUSE) {
    Log("ShowCursor -> disable");
    SDL_ShowCursor(SDL_DISABLE);
  }

  if (TOUCH) ui_init();
  if (TOUCH) platformD.selection = touch_selection;

  if (DPAD) dpad_init();
  if (DPAD) platformD.dpad = dpad_init;

  // !!texture_formatD override!! minimapD is streaming abgr8888
  mmtextureD =
      SDL_CreateTexture(rendererD, SDL_PIXELFORMAT_ABGR8888,
                        SDL_TEXTUREACCESS_STREAMING, MAX_WIDTH, MAX_HEIGHT);

  SDL_SetTextureBlendMode(mmtextureD, SDL_BLENDMODE_NONE);

  map_textureD = SDL_CreateTexture(rendererD, texture_formatD,
                                   SDL_TEXTUREACCESS_TARGET, MAP_W, MAP_H);
  SDL_SetTextureBlendMode(map_textureD, SDL_BLENDMODE_NONE);

  text_textureD = SDL_CreateTexture(
      rendererD, texture_formatD, SDL_TEXTUREACCESS_TARGET, 2 * 1024, 2 * 1024);
  if (text_textureD)
    SDL_SetTextureScaleMode(text_textureD, SDL_ScaleModeLinear);

  Log("texture creation complete: %d OK",
      (mmtextureD != 0) + (map_textureD != 0) + (text_textureD != 0));

  if (globalD.orientation_lock) SDL_SetWindowResizable(windowD, 0);
  Log("SetWindowResizable");

  font_reset();

  if (JOYSTICK) SDL_Init(SDL_INIT_JOYSTICK);

  // Hardware dependent "risky" initialization complete!
  if (COSMO) phaseD = PHASE_GAME;
  Log("initialization complete");

  // Migration code
  if (platformD.load(-1, 0)) fs_upgrade();

  return 0;
}

int
custom_postgame(may_exit)
{
  // Postgame activities should not be called from the crash handler
  if (COSMO) phaseD = PHASE_POSTGAME;

  if (DISK) disk_postgame();
  if (JOYSTICK) joystick_assign(-1);
  return platform_postgame(may_exit);
}

static int
portrait_layout()
{
  USE(layout_rect);
  int margin = (layout_rect.w - MAP_W) / 2;

  grectD[GR_VERSION] = (rect_t){
      layout_rect.w - 8 * FWIDTH,
      layout_rect.h - 3 * FHEIGHT,
      FWIDTH * 8,
      FHEIGHT * 3,
  };

  int lift = FHEIGHT / 2;
  grectD[GR_PAD] = (rect_t){
      margin,
      layout_rect.h - PADSIZE - lift,
      PADSIZE,
      PADSIZE,
  };

  int size = PADSIZE / 2;
  grectD[GR_BUTTON1] = (rect_t){layout_rect.w - margin - size * 2,
                                grectD[GR_PAD].y + size, size, size};
  grectD[GR_BUTTON2] =
      (rect_t){layout_rect.w - margin - size, grectD[GR_PAD].y, size, size};

  grectD[GR_GAMEPLAY] = (rect_t){
      margin,
      layout_rect.h - PADSIZE - MAP_H - FHEIGHT + FHEIGHT / 4,
      MAP_W,
      MAP_H,
  };
  grectD[GR_MINIMAP] = (rect_t){
      layout_rect.w - margin - MMSCALE * MAX_WIDTH - 4 * FWIDTH,
      grectD[GR_GAMEPLAY].y - 2 * FHEIGHT - MMSCALE * MAX_HEIGHT,
      MMSCALE * MAX_WIDTH,
      MMSCALE * MAX_HEIGHT,
  };
  grectD[GR_HISTORY] = (rect_t){
      layout_rect.w - 128 - margin - 4 * FWIDTH,
      FHEIGHT / 2,
      128,
      128,
  };
  grectD[GR_LOCK] = grectD[GR_HISTORY];
  grectD[GR_LOCK].x -= 2 * grectD[GR_HISTORY].w;

  grectD[GR_STAT] = (rect_t){
      margin,
      0,
      PADSIZE + 3,
      (8 + 5) * FHEIGHT,
  };

  int olimit = overlay_widthD * FWIDTH;
  grectD[GR_OVERLAY] = (rect_t){
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

  grectD[GR_VERSION] = (rect_t){
      layout_rect.w - 8 * FWIDTH,
      layout_rect.h - 3 * FHEIGHT,
      FWIDTH * 8,
      FHEIGHT * 3,
  };

  int lift = (layout_rect.h - (8 + 5) * FHEIGHT - PADSIZE - ymargin) / 2;
  grectD[GR_PAD] = (rect_t){
      0,
      layout_rect.h - PADSIZE - lift,
      PADSIZE,
      PADSIZE,
  };

  int size = PADSIZE / 2;
  grectD[GR_BUTTON1] = (rect_t){
      layout_rect.w - size * 2,
      grectD[GR_PAD].y + size,
      size,
      size,
  };
  grectD[GR_BUTTON2] = (rect_t){
      layout_rect.w - size,
      grectD[GR_PAD].y,
      size,
      size,
  };

  grectD[GR_GAMEPLAY] = (rect_t){
      xmargin,
      ymargin,
      MAP_W,
      MAP_H,
  };
  grectD[GR_MINIMAP] = (rect_t){
      (layout_rect.w - PADSIZE) + (PADSIZE - MMSCALE * MAX_WIDTH) / 2,
      FHEIGHT * 5,
      MMSCALE * MAX_WIDTH,
      MMSCALE * MAX_HEIGHT,
  };
  grectD[GR_HISTORY] = (rect_t){
      layout_rect.w - size / 2 - 96 / 2,
      layout_rect.h / 2 - 128 - FHEIGHT,
      96,
      128,
  };
  grectD[GR_LOCK] = grectD[GR_HISTORY];
  grectD[GR_LOCK].x -= grectD[GR_HISTORY].w * 2;

  grectD[GR_STAT] = (rect_t){
      0,
      ymargin,
      PADSIZE,
      (8 + 5) * FHEIGHT,
  };

  grectD[GR_OVERLAY] = (rect_t){
      xmargin,
      0,
      overlay_widthD * FWIDTH,
      overlay_heightD * FHEIGHT,
  };
  grectD[GR_WIDESCREEN] = (rect_t){
      xmargin,
      0,
      layout_rect.w - xmargin,
      overlay_heightD * FHEIGHT,
  };

  return 0;
}

STATIC int
swap_layout(SDL_Rect* rect, SDL_Rect* layout_rect)
{
  rect->x = layout_rect->w - rect->x - rect->w;
  return 0;
}

// Rectangle

// Shrinks w/h of subrect to fit alignrect
// Fits subrect within the xrange yrange of alignrect
void
align_subrect(rect_t* pi_alignrect, rect_t* pio_subrect)
{
  rect_t* align = pi_alignrect;
  rect_t* subrect = pio_subrect;

  if (subrect->w > align->w) subrect->w = align->w;
  if (subrect->h > align->h) subrect->h = align->h;

  if (subrect->x < align->x) subrect->x = align->x;
  if (subrect->x + subrect->w > align->x + align->w)
    subrect->x = align->x + align->w - subrect->w;

  if (subrect->y < align->y) subrect->y = align->y;
  if (subrect->y + subrect->h > align->y + align->h)
    subrect->y = align->y + align->h - subrect->h;
}
int
view_rect(rect_t* po_rect)
{
  po_rect->x = panelD.panel_col_min;
  po_rect->y = panelD.panel_row_min;
  po_rect->w = SYMMAP_WIDTH;
  po_rect->h = SYMMAP_HEIGHT;
}
int
zoom_rect(rect_t* po_rect)
{
  int zoom_factor = globalD.zoom_factor;
  int cellh = SYMMAP_HEIGHT >> zoom_factor;
  int cellw = SYMMAP_WIDTH >> zoom_factor;

  rect_t view;
  view_rect(&view);
  rect_t rect = {
      uD.x - cellw / 2,
      uD.y - cellh / 2,
      cellw + (zoom_factor != 0),
      cellh + (zoom_factor != 0),
  };
  align_subrect(&view, &rect);
  memcpy(po_rect, &rect, sizeof(rect));
}

// Text Drawing
static int
rect_altfill(r)
rect_t r;
{
  int rx = r.x;
  int ry = r.y;
  int rh = r.h;
  int rw = r.w;
  uint32_t even = lightingD[1];
  uint32_t odd = lightingD[2];
  for (int row = 0; row * FHEIGHT < rh; ++row) {
    int color = row % 2 ? odd : even;
    SDL_SetRenderDrawColor(rendererD, V4b(&color));
    rect_t target = {
        rx,
        ry + row * FHEIGHT,
        rw,
        FHEIGHT,
    };
    SDL_RenderFillRect(rendererD, &target);
  }

  return 0;
}
void rect_innerframe(r) rect_t r;
{
  SDL_SetRenderDrawColor(rendererD, V4b(&whiteD));
  SDL_RenderDrawRect(rendererD, &r);
  SDL_RenderDrawRect(rendererD, &RF(r, -1));
  SDL_RenderDrawRect(rendererD, &RF(r, -3));
}
void rect_frame(r, scale) rect_t r;
{
  int i = scale * 3;
  SDL_SetRenderDrawColor(rendererD, V4b(&whiteD));
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
        if (TEST_UI) {
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
    AUSE(grect, GR_MINIMAP);
    {
      SDL_Point p = {grect.x, grect.y - FHEIGHT - 24};
      len = snprintf(tmp, AL(tmp), "turn:%7d", turnD);
      render_monofont_string(renderer, &fontD, tmp, len, p);

      int delta = turnD - last_turnD;
      if (TEST_UI) delta = 1234567;
      if (delta) {
        p.x += len * FWIDTH + 1;
        if (delta > 0)
          font_color(font_rgba(BRIGHT + GREEN));
        else
          font_color(font_rgba(BRIGHT + RED));

        len = snprintf(tmp, AL(tmp), "%+d", delta);
        render_monofont_string(renderer, &fontD, tmp, len, p);
        font_reset();
      }
    }

    {
      SDL_Point p = {grect.x + grect.w / 2, grect.y + grect.h + 24};
      len = snprintf(tmp, AL(tmp), "%s", dun_descD);
      p.x -= (len * FWIDTH) / 2;
      render_monofont_string(renderer, &fontD, tmp, len, p);

      if (PC && TEST_CAVEGEN) {
        p.y += FHEIGHT;
        len = snprintf(tmp, AL(tmp), "%d %d x/y", uD.x, uD.y);
        render_monofont_string(renderer, &fontD, tmp, len, p);
      }

      if (PC && TEST_REPLAY) {
        p.x = grect.x + FWIDTH / 2;
        p.y += FHEIGHT;
        len = snprintf(tmp, AL(tmp), "Input Actions: %d", input_action_usedD);
        render_monofont_string(renderer, &fontD, tmp, len, p);

        p.y += FHEIGHT;
        len = snprintf(tmp, AL(tmp), "Input Record: %d", input_record_readD);
        render_monofont_string(renderer, &fontD, tmp, len, p);

        p.y += FHEIGHT;
        if (replay_desync) {
          font_color(font_rgba(BRIGHT + RED));
          render_monofont_string(renderer, &fontD, AP("REPLAY DESYNC!!!"), p);
          font_reset();
        }
      }
    }
  }

  if (TOUCH) {
    if (msg_moreD || TEST_UI) {
      static int tapD;
      tapD = (tapD + 1) % 4;
      switch (tapD) {
        case 0:
          font_color(font_rgba(BRIGHT + RED));
          break;
        case 1:
          font_color(font_rgba(BRIGHT + GREEN));
          break;
        case 2:
          font_color(font_rgba(BRIGHT + BLUE));
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
  if (PC) {
    if (msg_moreD || TEST_UI) {
      DATA char spacebar[] = "-press spacebar-";
      SDL_Point p = {0, layout_rect.h};
      p.x += AL(spacebar);
      p.y -= FHEIGHT;
      render_monofont_string(renderer, &fontD, AP(spacebar), p);
    }
    int numlock = SDL_GetModState() & KMOD_NUM;
    if (numlock) {
      DATA char numlock_warning[] = "-please disable numlock-";
      SDL_Point p = {0, layout_rect.h};
      p.x += AL(numlock_warning);
      p.y -= 2 * FHEIGHT;
      render_monofont_string(renderer, &fontD, AP(numlock_warning), p);
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

    if (!msg_used) {
      msg = AS(msg_cqD, msg_writeD - 1);
      msg_used = AS(msglen_cqD, msg_writeD - 1);
      alpha = msg_turnD == turnD ? FALPHA : 150;
    }

    SDL_Point p = {
        grect.x + FWIDTH / 2,
        grect.y + FHEIGHT / 2,
    };

    if (TEST_UI) {
      rect_t msg_target = {p.x, p.y, msg_widthD * FWIDTH,
                           FHEIGHT + FHEIGHT / 8};
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
      SDL_RenderFillRect(renderer, &msg_target);
    }

    if (msg_used) {
      rect_t fill = {p.x, p.y, msg_used * FWIDTH, FHEIGHT + FHEIGHT / 8};
      SDL_SetRenderDrawColor(rendererD, 0, 0, 0, 0);
      SDL_RenderFillRect(renderer, &fill);

      font_texture_alphamod(alpha);
      render_monofont_string(renderer, &fontD, msg, msg_used, p);
      font_reset();
    }

    if (msg_more || TEST_UI) {
      rect_t r = {
          grect.x + grect.w - STRLEN_MORE * FWIDTH - FWIDTH / 2,
          grect.y + grect.h - FHEIGHT - FHEIGHT / 2,
          STRLEN_MORE * FWIDTH,
          FHEIGHT + FHEIGHT / 8,
      };
      rect_t r2 = {
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
#define FHEIGHT (FHEIGHT * font_scaleD)
#define FWIDTH (FWIDTH * font_scaleD)
int
landscape_text(mode)
{
  USE(msg_more);
  USE(renderer);
  USE(layout_rect);
  MUSE(global, small_text);
  if (!PC) small_text = 1;

  if (mode == 0) {
    font_scaleD = small_text ? 1.0f : 1.25f;
    char* msg = AS(msg_cqD, msg_writeD);
    int msg_used = AS(msglen_cqD, msg_writeD);
    int alpha = FALPHA;

    if (!msg_used) {
      msg = AS(msg_cqD, msg_writeD - 1);
      msg_used = AS(msglen_cqD, msg_writeD - 1);
      alpha = msg_turnD == turnD ? FALPHA : 128;
    }

    SDL_Point p = {layout_rect.w / 2 - msg_used * FWIDTH / 2, 0};
    rect_t rect = {
        p.x - FWIDTH / 2,
        p.y,
        (1 + msg_used) * FWIDTH,
        FHEIGHT,
    };
    if (TEST_UI) {
      rect_t text_target = {
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
      if (small_text) rect_frame(rect, 1);
    }

    if (msg_more || TEST_UI) {
      int wlimit = msg_widthD * FWIDTH;
      int mlimit = STRLEN_MORE * FWIDTH;
      int margin = small_text ? (layout_rect.w - wlimit - mlimit) / 4 : FWIDTH;

      rect_t rect2 = {
          margin,
          0,
          mlimit,
          FHEIGHT,
      };
      rect_t rect3 = {
          layout_rect.w - margin - mlimit,
          0,
          mlimit,
          FHEIGHT,
      };

      SDL_Point p2 = {rect2.x, rect2.y};
      render_monofont_string(renderer, &fontD, AP(moreD), p2);
      SDL_Point p3 = {rect3.x, rect3.y};
      render_monofont_string(renderer, &fontD, AP(moreD), p3);
      if (small_text) rect_frame(rect2, 1);
      if (small_text) rect_frame(rect3, 1);
    }

    font_scaleD = 1.0f;
    common_text();
  } else {
    vitalstat_text();
  }

  return 0;
}
#undef FONT_HEIGHT
#undef FONT_WIDTH

// Drawing

// Modifies RenderTarget!
int
map_draw()
{
  rect_t dest_rect;
  rect_t src_rect;
  SDL_Point rp;
  USE(sprite_texture);
  USE(font_texture);
  USE(renderer);

  SDL_SetRenderTarget(renderer, map_textureD);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  // Ascii renderer only
  if (countD.imagine || !globalD.sprite) sprite_texture = 0;

  dest_rect.w = ART_W;
  dest_rect.h = ART_H;
  for (int row = 0; row < SYMMAP_HEIGHT; ++row) {
    dest_rect.y = row * ART_H;
    for (int col = 0; col < SYMMAP_WIDTH; ++col) {
      dest_rect.x = col * ART_W;

      struct vizS* viz = &vizD[row][col];
      char sym = viz->sym;
      int fidx = viz->floor;
      int light = viz->light;
      int dim = viz->dim;
      int cridx = viz->cr;
      int tridx = viz->tr;

      // Art priority creature, wall, treasure, fallback to symmap ASCII
      SDL_Texture* srct = 0;
      rect_t* srcr = 0;

      if (sym == '@') {
        rp = (SDL_Point){col, row};
      }

      if (sprite_texture) {
        srct = sprite_texture;
        srcr = &src_rect;
        if (cridx) {
          src_rect = (rect_t){
              XY(point_by_spriteid(art_textureD + cridx)),
              ART_W,
              ART_H,
          };
        } else if (fidx) {
          src_rect = (rect_t){
              XY(point_by_spriteid(wart_textureD + fidx)),
              ART_W,
              ART_H,
          };
        } else if (tridx) {
          src_rect = (rect_t){
              XY(point_by_spriteid(tart_textureD + tridx)),
              ART_W,
              ART_H,
          };
        } else if (sym == '@') {
          src_rect = (rect_t){
              XY(point_by_spriteid(part_textureD + turnD % 2)),
              ART_W,
              ART_H,
          };
        } else {
          srct = 0;
        }
      }

      if (!srct && sym > ' ') {
        src_rect = font_rect_by_char(sym);
        srcr = &src_rect;
        srct = font_texture;
      }

      SDL_SetRenderDrawColor(renderer, V4b(&lightingD[light]));
      SDL_RenderFillRect(renderer, &dest_rect);

      if (srct) {
        if (dim) SDL_SetTextureColorMod(srct, 192, 192, 192);
        SDL_RenderCopy(renderer, srct, srcr, &dest_rect);
        if (dim) SDL_SetTextureColorMod(srct, 255, 255, 255);
      }
      switch (viz->fade) {
        case 0:
          // TBD: configuration on default dimming?
          SDL_SetRenderDrawColor(renderer, 0, 0, 0, 16);
          SDL_RenderFillRect(renderer, &dest_rect);
          break;
        case 1:
          SDL_SetRenderDrawColor(renderer, 0, 0, 0, 32);
          SDL_RenderFillRect(renderer, &dest_rect);
          break;
        case 2:
          SDL_SetRenderDrawColor(renderer, 0, 0, 0, 64);
          SDL_RenderFillRect(renderer, &dest_rect);
          break;
        case 3:
          SDL_SetRenderDrawColor(renderer, 0, 0, 0, 98);
          SDL_RenderFillRect(renderer, &dest_rect);
          break;
      }
      if (viz->vflag & VF_LOOK) {
        SDL_SetRenderDrawColor(renderer, V4b(&whiteD));
        SDL_RenderDrawRect(renderer, &dest_rect);
      }
      if (viz->vflag & VF_MAGICK) {
        if (sprite_texture) {
          srct = sprite_texture;
          src_rect = (rect_t){
              XY(point_by_spriteid(tart_textureD + 42)),
              ART_W,
              ART_H,
          };
        } else {
          srct = font_texture;
          src_rect = font_rect_by_char('*');
        }
        SDL_RenderCopy(renderer, srct, &src_rect, &dest_rect);
      }
    }
  }

  if (sprite_texture) {
    uint32_t oidx = caveD[uD.y][uD.x].oidx;
    struct objS* obj = &entity_objD[oidx];
    uint32_t tval = obj->tval;
    dest_rect.y = rp.y * ART_H;
    dest_rect.x = rp.x * ART_W;

    if (tval - 1 < TV_MAX_PICK_UP || tval == TV_CHEST) {
      src_rect = (rect_t){
          XY(point_by_spriteid(part_textureD + 2)),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(renderer, sprite_texture, &src_rect, &dest_rect);
    } else if (tval == TV_GLYPH) {
      src_rect = (rect_t){
          XY(point_by_spriteid(part_textureD + 3)),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(renderer, sprite_texture, &src_rect, &dest_rect);
    } else if (tval == TV_VIS_TRAP) {
      src_rect = (rect_t){
          XY(point_by_spriteid(part_textureD + 4)),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(renderer, sprite_texture, &src_rect, &dest_rect);
    }

    if (countD.paralysis) {
      src_rect = (rect_t){
          XY(point_by_spriteid(part_textureD + 5)),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(renderer, sprite_texture, &src_rect, &dest_rect);
    }
    if (countD.poison) {
      src_rect = (rect_t){
          XY(point_by_spriteid(part_textureD + 6)),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(renderer, sprite_texture, &src_rect, &dest_rect);
    }
    if (maD[MA_SLOW]) {
      src_rect = (rect_t){
          XY(point_by_spriteid(part_textureD + 7)),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(renderer, sprite_texture, &src_rect, &dest_rect);
    }
    if (maD[MA_BLIND]) {
      src_rect = (rect_t){
          XY(point_by_spriteid(part_textureD + 8)),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(renderer, sprite_texture, &src_rect, &dest_rect);
    }
    if (countD.confusion) {
      src_rect = (rect_t){
          XY(point_by_spriteid(part_textureD + 9)),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(renderer, sprite_texture, &src_rect, &dest_rect);
    }
    if (maD[MA_FEAR]) {
      src_rect = (rect_t){
          XY(point_by_spriteid(part_textureD + 10)),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(renderer, sprite_texture, &src_rect, &dest_rect);
    }
    if (uD.food < PLAYER_FOOD_FAINT) {
      src_rect = (rect_t){
          XY(point_by_spriteid(part_textureD + 12)),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(renderer, sprite_texture, &src_rect, &dest_rect);
    } else if (uD.food <= PLAYER_FOOD_ALERT) {
      src_rect = (rect_t){
          XY(point_by_spriteid(part_textureD + 11)),
          ART_W,
          ART_H,
      };
      SDL_RenderCopy(renderer, sprite_texture, &src_rect, &dest_rect);
    }
  }
  return 0;
}
int
draw_game()
{
  USE(minimap_enlarge);

  int show_minimap = (maD[MA_BLIND] == 0);
  int show_game = 1;

  vitalstat_text();

  {
    USE(mmtexture);

    if (show_minimap) {
      AUSE(grect, GR_MINIMAP);

      SDL_Rect panel = {
          panelD.panel_col_min,
          panelD.panel_row_min,
          SYMMAP_WIDTH,
          SYMMAP_HEIGHT,
      };
      SDL_RenderCopy(rendererD, mmtexture, &panel, &grect);
      rect_frame(grect, 3);
    }

    if (show_game) {
      AUSE(grect, GR_GAMEPLAY);
      if (minimap_enlarge) {
        SDL_Rect scale = {
            0,
            0,
            dun_level ? MAX_WIDTH : SYMMAP_WIDTH,
            dun_level ? MAX_HEIGHT : SYMMAP_HEIGHT,
        };
        SDL_RenderCopy(rendererD, mmtexture, &scale, &grect);
      } else {
        map_draw();

        rect_t zr;
        zoom_rect(&zr);

        rect_t vr;
        view_rect(&vr);

        rect_t source = {zr.x - vr.x, zr.y - vr.y, zr.w, zr.h};
        source.x *= ART_W;
        source.w *= ART_W;
        source.y *= ART_H;
        source.h *= ART_H;

        SDL_SetRenderDrawColor(rendererD, V4b(&whiteD));
        SDL_RenderDrawRect(rendererD, &source);

        SDL_SetRenderDrawBlendMode(rendererD, SDL_BLENDMODE_NONE);
        SDL_SetRenderTarget(rendererD, layoutD);

        SDL_RenderCopy(rendererD, map_textureD, &source, &grect);
      }
    }
  }
}
int
draw_menu(mode, using_selection)
{
  USE(renderer);
  USE(overlay_width);
  USE(overlay_height);
  char* msg = AS(msg_cqD, msg_writeD);
  int msg_used = AS(msglen_cqD, msg_writeD);
  int is_text;
  int is_death = 0;
  AUSE(grect, GR_OVERLAY);

  if (mode == 1) {
    is_text = 1;
  } else {
    is_death = (screen_submodeD == 0);
    is_text = (screen_submodeD != 0);
    if (screen_submodeD == 2) {
      grect = grectD[GR_WIDESCREEN];
      overlay_width = msg_widthD;
    }
  }

  rect_t src_rect = {
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
        if (using_selection && row == finger_rowD) {
          font_color(font_rgba(BRIGHT + RED));
          if (tlen <= 1) {
            text = "-";
            tlen = 1;
          }
        }
        render_monofont_string(renderer, &fontD, text, tlen, p);
        if (using_selection && row == finger_rowD) {
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

  if (TOUCH) {
    if (is_death) {
      AUSE(grect, GR_HISTORY);
      if (ui_textureD) SDL_RenderCopy(renderer, ui_textureD, NULL, &grect);
      rect_frame(grect, 1);
    }
  }
}
// mode_change is triggered by interactive UI navigation
// may edit row/col selection to make the interface feel "smart"
// not utilized by the replay system
STATIC int
mode_change(mnext)
{
  int subprev = submodeD;
  int subnext = overlay_submodeD;
  int mprev = modeD;

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

  return mnext;
}

int
custom_draw()
{
  USE(renderer);
  int using_selection = (TOUCH || (JOYSTICK && joystick_count() > 0));

  // Dynamic assignment of menu selection mode
  // Controller hotplugging can toggle this feature
  platformD.selection = using_selection ? touch_selection : noop;

  SDL_SetRenderTarget(renderer, layoutD);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  int mode = 0;
  if (screen_usedD[0])
    mode = 2;
  else if (overlay_usedD[0])
    mode = 1;

  // numpad directions are gameplay only
  if (KEYBOARD) keyboard_map(mode == 0 ? gameplay_inputD : 0);

  if (using_selection) {
    mode_change(mode);

    if (tptextureD) {
      {
        AUSE(grect, GR_PAD);
        SDL_RenderCopy(rendererD, tptextureD, 0, &grect);
        rect_frame(grect, 0);
      }

      if (JOYSTICK) {  // dpad joystick
        SDL_Rect r = {0, 0, 64, 64};
        float jx, jy;
        joystick_2f(&jx, &jy);
        r.x = jx * (PADSIZE - r.w);
        r.y = jy * (PADSIZE - r.h);
        // offset into the touchpad zone
        r.x += grectD[GR_PAD].x;
        r.y += grectD[GR_PAD].y;
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        SDL_RenderFillRect(renderer, &r);
      }

      // TBD: Are we showing buttons with controllers?
      if (TOUCH) {
        for (int it = 0; it < MAX_BUTTON; ++it) {
          AUSE(grect, GR_BUTTON1 + it);
          int color = rgba_by_palette(it == 0 ? RED : GREEN);
          SDL_SetRenderDrawColor(rendererD, V4b(&color));
          SDL_RenderFillRect(rendererD, &grect);
        }

        if (ui_textureD && mode == 0) {
          AUSE(grect, GR_HISTORY);
          SDL_RenderCopy(renderer, ui_textureD, NULL, &grect);
          rect_frame(grect, 1);
        }
        if (mode == 0) {
          AUSE(grect, GR_LOCK);
          if (sprite_textureD) {
            MUSE(global, orientation_lock);
            // TBD: UI Icon?
            int tridx = 1;
            if (orientation_lock) {
              tridx = 48;
            } else {
              tridx = 45;
            }
            rect_t sprite_rect = {
                XY(point_by_spriteid(tart_textureD + tridx)),
                ART_W,
                ART_H,
            };
            rect_t dest = {
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
    }
  }

  if (mode == 0)
    draw_game();
  else
    draw_menu(mode, using_selection);

  if (text_fnD) text_fnD(mode);

  // Render version stamp on all screens
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
  struct objS* obj;
  int color = 0;
  int grey = lightingD[2];
  USE(white);

  c_ptr = &caveD[row][col];
  if (mon_drawD[c_ptr->midx]) {
    color = rgba_by_palette(BRIGHT + PINK);
  } else if (c_ptr->fval == BOUNDARY_WALL) {
    color = white;
  } else if (CF_LIT & c_ptr->cflag && c_ptr->fval >= MIN_WALL) {
    color = white;
  } else if (CF_LIT & c_ptr->cflag) {
    color = grey;
  }

  if ((!color || color == grey) && c_ptr->oidx) {
    obj = &entity_objD[c_ptr->oidx];
    if (CF_VIZ & c_ptr->cflag) {
      if (obj->tval == TV_UP_STAIR) {
        color = rgba_by_palette(BLUE);
      } else if (obj->tval == TV_DOWN_STAIR) {
        color = rgba_by_palette(RED);
      } else if (obj->tval == TV_VIS_TRAP || obj->tval == TV_RUBBLE) {
        color = rgba_by_palette(YELLOW);
      } else if (obj->tval == TV_SECRET_DOOR) {
        color = white;
      } else if (obj->tval == TV_CLOSED_DOOR) {
        color = grey;
      } else if (obj->tval != 0 && obj->tval <= TV_MAX_PICK_UP) {
        color = rgba_by_palette(BRIGHT + BLUE);
      } else if (obj->tval == TV_STORE_DOOR || obj->tval == TV_PAWN_DOOR) {
        color = rgba_by_palette(BRIGHT + PURPLE);
      }
    }
  }

  if ((!color || color == grey) && (CF_TEMP_LIGHT & c_ptr->cflag)) {
    color = rgba_by_palette(BRIGHT + GREEN);
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
      struct objS* obj = &entity_objD[c_ptr->oidx];
      struct vizS viz = {0};
      if (row != py || col != px) {
        viz.light = (c_ptr->cflag & CF_SEEN) != 0;
        viz.fade = fade_by_distance(py, px, row, col) - 1;
        if (mon_drawD[c_ptr->midx]) {
          int cr = mon_drawD[c_ptr->midx];
          viz.cr = cr;
          viz.sym = creatureD[cr].cchar;
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

  uint32_t vy = ylookD - rmin;
  uint32_t vx = xlookD - cmin;
  if (vy < SYMMAP_HEIGHT && vx < SYMMAP_WIDTH) {
    vizD[vy][vx].vflag |= VF_LOOK;
  }
}
void
viz_minimap_stair(row, col, rgba)
{
  minimapD[row][col] = rgba;
  minimapD[row][col - 1] = rgba;
  minimapD[row - 1][col] = rgba;
  minimapD[row - 1][col + 1] = rgba;
  minimapD[row - 2][col + 1] = rgba;
  minimapD[row - 2][col + 2] = rgba;
}
void
viz_minimap()
{
  int full_map = minimap_enlargeD && dun_level;
  MUSE(panel, panel_row_min);
  MUSE(panel, panel_col_min);
  int rmin = full_map ? 0 : panel_row_min;
  int rmax = rmin + (full_map ? MAX_HEIGHT : SYMMAP_HEIGHT);
  int cmin = full_map ? 0 : panel_col_min;
  int cmax = cmin + (full_map ? MAX_WIDTH : SYMMAP_WIDTH);
  int color;

  for (int row = rmin; row < rmax; ++row) {
    for (int col = cmin; col < cmax; ++col) {
      color = cave_color(row, col);

      uint32_t drow = row - panel_row_min;
      uint32_t dcol = col - panel_col_min;
      if (!color && (drow < SYMMAP_HEIGHT && dcol < SYMMAP_WIDTH)) {
        color = lightingD[0];
      }

      minimapD[row][col] = color;
      if (color == rgba_by_palette(BLUE) || color == rgba_by_palette(RED))
        viz_minimap_stair(row, col, color);
    }
  }

  SDL_UpdateTexture(mmtextureD, NULL, &minimapD[0][0],
                    MAX_WIDTH * sizeof(minimapD[0][0]));
}
int
custom_predraw()
{
  FOR_EACH(mon, {
    int draw = mon_lit(it_index);

    int cidx = 0;
    if (draw) cidx = mon->cidx;
    mon_drawD[it_index] = cidx;
  });

  viz_update();
  viz_minimap();
  return 1;
}

int
custom_orientation(orientation)
{
  orientation = platform_orientation(orientation);

  fn text_fn = 0;
  if (orientation == SDL_ORIENTATION_PORTRAIT) {
    text_fn = portrait_text;
    overlay_widthD = 67;
    overlay_heightD = AL(overlayD) + 1;
    msg_widthD = 63;
  } else if (orientation == SDL_ORIENTATION_LANDSCAPE) {
    text_fn = landscape_text;
    overlay_widthD = 78;
    overlay_heightD = AL(overlayD) + 2;
    msg_widthD = (!PC || globalD.small_text) ? 92 : 80;
  }
  text_fnD = text_fn;

  if (orientation == SDL_ORIENTATION_PORTRAIT) portrait_layout();
  if (orientation == SDL_ORIENTATION_LANDSCAPE) landscape_layout();
  if (globalD.hand_swap) {
    for (int it = 0; it < GR_COUNT; ++it) {
      swap_layout(&grectD[it], &layout_rectD);
    }
  }
  return 0;
}

int
feature_menu()
{
  char c;
  int line;
  char opt[2][4] = {"off", "on"};
  char* default_renderer = PC ? "opengl" : "opengles2";
  int using_selection = platformD.selection != noop;

  while (1) {
    overlay_submodeD = 'f';
    line = 0;
    BufMsg(overlay, "a) ascii gameplay renderer (%s)",
           opt[globalD.sprite == 0]);
    if (using_selection) {
      line = 'c' - 'a';
      BufMsg(overlay, "c) colorize dpad (%s)", opt[globalD.dpad_color != 0]);
      line = 'd' - 'a';
      BufMsg(overlay, "d) dpad sensitivity (%d)", globalD.dpad_sensitivity);
    }
    line = 'g' - 'a';
    BufMsg(overlay, "g) gpu interface (%s)",
           globalD.pc_renderer[0] ? globalD.pc_renderer : default_renderer);
    line = 'h' - 'a';
    BufMsg(overlay, "h) hand-swap user interface (%s)",
           opt[globalD.hand_swap != 0]);
    line = 'm' - 'a';
    BufMsg(overlay, "m) magnification scale (%d x)", 1 << globalD.zoom_factor);
    line = 'r' - 'a';
    if (!vsync_rateD) {
      BufMsg(overlay, "r) refresh / video sync (%s)", opt[globalD.vsync != 0]);
    } else {
      BufMsg(overlay,
             "r) refresh / video sync (%s) | %d fps of %d display claimed",
             opt[globalD.vsync != 0], vsync_rateD, refresh_rateD);
    }
    if (PC) {
      line = 't' - 'a';
      BufMsg(overlay, "t) landscape text size (%s)",
             globalD.small_text ? "small" : "large");
    }
    line = 'o' - 'a';
    BufMsg(overlay, "o) orientation lock (%s)",
           opt[globalD.orientation_lock != 0]);
    line = 'v' - 'a';
    BufMsg(overlay, "v) version info");

    c = CLOBBER_MSG("feature menu");
    if (is_ctrl(c)) break;

    switch (c) {
      case 'a':
        INVERT(globalD.sprite);
        break;
      case 'c':
        INVERT(globalD.dpad_color);
        platformD.dpad();
        break;
      case 'd':
        if (globalD.dpad_sensitivity >= 99)
          globalD.dpad_sensitivity = 55;
        else
          globalD.dpad_sensitivity += 10;
        platformD.dpad();
        break;
      case 'h':
        INVERT(globalD.hand_swap);
        platformD.orientation(0);
        break;
      case 'm':
        globalD.zoom_factor = (globalD.zoom_factor - 1) % MAX_ZOOM;
        break;
      case 'r':
        platformD.vsync(INVERT(globalD.vsync));
        break;
      case 't':
        INVERT(globalD.small_text);
        platformD.orientation(0);
        break;
      case 'o':
        INVERT(globalD.orientation_lock);
        break;
      case 'v':
        show_version();
        break;
    }
  }
  return 0;
}
