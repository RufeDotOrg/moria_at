// Rufe.org LLC 2022-2025: ISC License
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

#include "asset/icon.c"
#include "asset/lamp_run.c"
#include "asset/lamp_walk.c"
#include "asset/sprite.c"

#include "asset/scancode.c"

// Game specific inclusion
enum { MAP_W = SYMMAP_WIDTH * ART_W };
enum { MAP_H = SYMMAP_HEIGHT * ART_H };

#include "dpad.c"

#include "touch.c"

#include "joystick.c"

enum { TEST_UI = 0 };

enum { AFF_X = 3 };
enum { AFF_Y = AL(active_affectD) / AFF_X };
enum { SPRITE_SQ = 32 };
enum { MMSCALE = 2 };

DATA char moreD[] = "-more-";

DATA fn text_fnD;
DATA uint32_t sprite_idD;
DATA SDL_Surface* spriteD;
DATA SDL_Texture* sprite_textureD;
DATA SDL_Texture* lwtextureD;
DATA SDL_Texture* lrtextureD;
DATA SDL_Texture* mmtextureD;
DATA SDL_Texture* ui_textureD;
DATA SDL_Texture* map_textureD;
DATA uint16_t mon_drawD[AL(monD)];

enum { PHASE_PREGAME = 1, PHASE_GAME = 2, PHASE_POSTGAME = 3 };
DATA int phaseD;

enum { STRLEN_MORE = AL(moreD) - 1 };

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
int
art_decode(buf, len)
uint8_t* buf;
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
  enum { UI_CN = 4 };
  int bitmap[UI_H][UI_W];

  for (int row = 0; row < UI_H; ++row) {
    for (int col = 0; col < UI_W; ++col) {
      int rmod = row % 4;
      if (rmod == 3) {
        bitmap[row][col] = 0;
      } else {
        bitmap[row][col] = -1;
      }
    }
  }
  for (int it = 0; it < 4; ++it) {
    bitmap[it * 4 + 1][1] = 0;
  }

  SDL_Texture* t = SDL_CreateTexture(rendererD, SDL_PIXELFORMAT_ABGR8888,
                                     SDL_TEXTUREACCESS_STATIC, UI_W, UI_H);
  if (t) SDL_UpdateTexture(t, 0, bitmap, UI_W * 4);
  if (t) SDL_SetTextureColorMod(t, V3b(&greyD[4]));
  ui_textureD = t;

  return t != 0;
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
  if (FONT) font_default(greyD[5]);

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
    if (puffex_stream_len(art_decode, AP(sprite_mmgz)) != 0) return 4;
    // TBD: encoding
    art_textureD = 0 - 1;
    tart_textureD = 279 - 1;
    wart_textureD = 332 - 1;
    part_textureD = 338;  // no offset

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

  if (TOUCH) {
    SDL_Surface* sprite = SDL_CreateRGBSurfaceWithFormat(
        SDL_SWSURFACE, 64, 64, 0, SDL_PIXELFORMAT_INDEX1LSB);
    if (sprite) {
      {
        SDL_Palette* palette = sprite->format->palette;
        *(int*)&palette->colors[0] = 0;
        *(int*)&palette->colors[1] = -1;
      }
      memcpy(sprite->pixels, lamp_walk_mmg, 64 * 64 / 8);
      lwtextureD = SDL_CreateTextureFromSurface(rendererD, sprite);
      SDL_SetTextureColorMod(lwtextureD, V3b(&greyD[4]));
      SDL_FreeSurface(sprite);
    }
  }
  if (TOUCH) {
    SDL_Surface* sprite = SDL_CreateRGBSurfaceWithFormat(
        SDL_SWSURFACE, 64, 64, 0, SDL_PIXELFORMAT_INDEX1LSB);
    if (sprite) {
      {
        SDL_Palette* palette = sprite->format->palette;
        *(int*)&palette->colors[0] = 0;
        *(int*)&palette->colors[1] = -1;
      }
      memcpy(sprite->pixels, lamp_run_mmg, 64 * 64 / 8);
      lrtextureD = SDL_CreateTextureFromSurface(rendererD, sprite);
      SDL_SetTextureColorMod(lrtextureD, V3b(&greyD[4]));

      SDL_FreeSurface(sprite);
    }
  }

  // Software renderer will skip this; limiting risk on troubled systems
  if (PC && !IsWindows()) {
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
    SDL_ShowCursor(0);
  }

  if (TOUCH) ui_init();
  if (TOUCH) platformD.selection = touch_selection;

  if (DPAD) dpad_init();
  if (DPAD) dpad_classic();
  if (DPAD) platformD.dpad = dpad_classic;

  // !!texture_formatD override!! minimapD is streaming abgr8888
  mmtextureD =
      SDL_CreateTexture(rendererD, SDL_PIXELFORMAT_ABGR8888,
                        SDL_TEXTUREACCESS_STREAMING, MAX_WIDTH, MAX_HEIGHT);

  SDL_SetTextureBlendMode(mmtextureD, SDL_BLENDMODE_NONE);

  map_textureD = SDL_CreateTexture(rendererD, texture_formatD,
                                   SDL_TEXTUREACCESS_TARGET, MAP_W, MAP_H);
  SDL_SetTextureBlendMode(map_textureD, SDL_BLENDMODE_NONE);

  font_reset();

  if (JOYSTICK) joystick_init();

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
  grectD[GR_BUTTON3] =
      (rect_t){layout_rect.w - margin - size * 2 + size / 4,
               grectD[GR_PAD].y + size / 4, size / 2, size / 2};

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
  grectD[GR_MENU] = (rect_t){
      layout_rect.w - 128 - margin - 4 * FWIDTH,
      FHEIGHT / 2,
      128,
      128,
  };
  grectD[GR_LOCK] = grectD[GR_MENU];
  grectD[GR_LOCK].x -= 2 * grectD[GR_MENU].w;

  grectD[GR_STAT] = (rect_t){
      margin,
      0,
      PADSIZE + 3,
      (8 + 5) * FHEIGHT,
  };

  int olimit = overlay_widthD * FWIDTH;
  grectD[GR_OVERLAY] = (rect_t){
      (layout_rect.w - olimit) / 2,
      grectD[GR_GAMEPLAY].y,
      olimit,
      (2 + AL(overlayD)) * FHEIGHT,
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
  grectD[GR_BUTTON3] = (rect_t){
      layout_rect.w - size * 2 + size / 4,
      grectD[GR_PAD].y + size / 4,
      size / 2,
      size / 2,
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
  grectD[GR_MENU] = (rect_t){
      layout_rect.w - size / 2 - 96 / 2,
      layout_rect.h / 2 - 128 - FHEIGHT,
      96,
      128,
  };
  grectD[GR_LOCK] = grectD[GR_MENU];
  grectD[GR_LOCK].x -= grectD[GR_MENU].w * 2;

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
      (2 + AL(overlayD)) * FHEIGHT,
  };
  grectD[GR_WIDESCREEN] = (rect_t){
      xmargin,
      0,
      layout_rect.w - xmargin,
      (2 + AL(overlayD)) * FHEIGHT,
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
  if (globalD.sprite == 0) zoom_factor = 0;
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
  uint32_t even = greyD[1];
  uint32_t odd = greyD[2];
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
void
framing_base_step(rect_t frame, int base, int step)
{
  int list[] = {base, base + step, base + step * 3};
  SDL_SetRenderDrawColor(rendererD, V4b(&greyD[4]));
  for (int it = 0; it < AL(list); ++it) {
    int sz = list[it];
    rect_t rect = {
        .x = frame.x - (sz),
        .y = frame.y - (sz),
        .w = frame.w + 2 * (sz),
        .h = frame.h + 2 * (sz),
    };
    SDL_RenderDrawRect(rendererD, &rect);
  }
}
int
vitalstat_layout()
{
  apclear(AB(stattextD));
  if (uD.lev) {
    int line = 0;
    int pitch = AL(stattextD[0]);

    if (uD.ridx < AL(raceD) && uD.clidx < AL(classD)) {
      int len = strlen(raceD[uD.ridx].name) + strlen(classD[uD.clidx].name) + 1;
      int wscount = pitch - len;
      snprintf(&stattextD[line][wscount / 2], pitch, "%s %s",
               raceD[uD.ridx].name, classD[uD.clidx].name);
    }
    ++line;

    for (int it = 0; it < MAX_A; ++it) {
      snprintf(stattextD[line++], pitch, "%-4.04s: %7d %-4.04s: %6d",
               vital_nameD[it], vitalD[it], stat_abbrD[it], vital_statD[it]);
    }
    {
      snprintf(stattextD[line++], pitch, "%-4.04s: %7d", vital_nameD[MAX_A],
               vitalD[MAX_A]);
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

      snprintf(stattextD[line++], pitch, "%-8.08s %-8.08s %-8.08s", affstr[0],
               affstr[1], affstr[2]);
    }
  }
}
int
vitalstat_text()
{
  USE(renderer);
  AUSE(grect, GR_STAT);

  rect_altfill(grect);
  rect_t tr = {
      grect.x + FWIDTH / 2,
      grect.y,
      grect.w - FWIDTH,
      grect.h,
  };
  render_monofont_block_text(renderer, 0, &tr, AP(stattextD[0]));
  framing_base_step(grect, 0, -1);

  return 0;
}
int
game_text()
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
          font_color(font_rgba(GREEN));
        else
          font_color(font_rgba(RED));

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
          font_color(font_rgba(RED));
          render_monofont_string(renderer, &fontD, AP("REPLAY DESYNC!!!"), p);
          font_reset();
        }
      }
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
      alpha = (msg_turnD == turnD) ? FALPHA : PREV_ALPHA;
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

      font_alpha(alpha);
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

    game_text();
  }

  vitalstat_text();

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

    if (!msg_used) {
      msg = AS(msg_cqD, msg_writeD - 1);
      msg_used = AS(msglen_cqD, msg_writeD - 1);
      alpha = (msg_turnD == turnD) ? FALPHA : PREV_ALPHA;
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
      font_alpha(alpha);
      render_monofont_string(renderer, &fontD, msg, msg_used, p);
      font_reset();

      SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }

    if (msg_more || TEST_UI) {
      int mlimit = STRLEN_MORE * FWIDTH;
      int margin = FWIDTH;

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
    }

    game_text();
  }

  vitalstat_text();

  return 0;
}

// Drawing

// Modifies RenderTarget!
int
map_draw()
{
  rect_t dest_rect;
  rect_t src_rect;
  SDL_Point rp;
  USE(sprite_texture);
  USE(pixel_texture);
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
        srct = pixel_texture;
      }

      SDL_SetRenderDrawColor(renderer, V4b(&greyD[light]));
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
        SDL_SetRenderDrawColor(renderer, V4b(&greyD[4]));
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
          srct = pixel_texture;
          src_rect = font_rect_by_char('*');
        }
        SDL_RenderCopy(renderer, srct, &src_rect, &dest_rect);
      }
    }
  }

  if (!sprite_texture) font_reset();
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
      framing_base_step(grect, 9, 1);
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

        if (globalD.sprite) {
          rect_t zr;
          zoom_rect(&zr);

          rect_t vr;
          view_rect(&vr);

          rect_t source = {zr.x - vr.x, zr.y - vr.y, zr.w, zr.h};
          source.x *= ART_W;
          source.w *= ART_W;
          source.y *= ART_H;
          source.h *= ART_H;

          float arscale = (float)MAP_H / source.h;
          SDL_Rect target = {0, 0, source.w * arscale, source.h * arscale};
          target.x = grect.x + (grect.w - target.w) / 2;
          target.y = grect.y;

          SDL_SetRenderDrawColor(rendererD, V4b(&greyD[4]));
          SDL_RenderDrawRect(rendererD, &source);

          SDL_SetRenderDrawBlendMode(rendererD, SDL_BLENDMODE_NONE);
          SDL_SetRenderTarget(rendererD, layoutD);

          SDL_RenderCopy(rendererD, map_textureD, &source, &target);
        } else {
          SDL_SetRenderDrawBlendMode(rendererD, SDL_BLENDMODE_NONE);
          SDL_SetRenderTarget(rendererD, layoutD);

          SDL_RenderCopy(rendererD, map_textureD, 0, &grect);
        }
      }
    }
  }
}
int
draw_menu(mode, using_selection)
{
  USE(renderer);
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
    }
  }

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderFillRect(renderer, &grect);
  if (is_text) rect_altfill(grect);

  point_t anchor = {XY(grect)};
  anchor.x += FWIDTH / 2;
  if (msg_used) render_monofont_string(renderer, &fontD, msg, msg_used, anchor);
  anchor.y += FHEIGHT * 2;

  char* textlist = 0;
  int* lenlist = 0;
  int step = 0;
  USE(finger_row);
  switch (mode) {
    case 1:
      textlist = &overlayD[0][0];
      lenlist = overlay_usedD;
      step = AL(overlayD[0]);
      break;
    case 2:
      textlist = &screenD[0][0];
      lenlist = screen_usedD;
      step = AL(screenD[0]);
      finger_row = -1;
      break;
  }

  for (int row = 0; row < AL(screenD); ++row) {
    SDL_Point p = {anchor.x, anchor.y + row * FHEIGHT};
    render_monofont_string(renderer, &fontD, &textlist[row * step],
                           lenlist[row], p);
  }

  if (using_selection && finger_row >= 0) {
    SDL_Point p = {anchor.x, anchor.y + finger_row * FHEIGHT};
    font_color(font_rgba(RED));
    font_alpha(255);
    if (lenlist[finger_row] <= 1) {
      render_monofont_string(renderer, &fontD, "-", 1, p);
    } else {
      render_monofont_string(renderer, &fontD, &textlist[finger_row * step],
                             lenlist[finger_row], p);
    }
    font_reset();
  }

  if (is_text) framing_base_step(grect, -1, 1);
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
STATIC int*
button_colorptr(c)
{
  if (globalD.dpad_color) return &colorD[c];
  return &greyD[2];
}
int
custom_draw()
{
  USE(renderer);
  int using_selection = (TOUCH || (JOYSTICK && joystick_active()));

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
        framing_base_step(grect, 0, 1);
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
        SDL_SetRenderDrawColor(renderer, V4b(&greyD[4]));
        SDL_RenderFillRect(renderer, &r);
      }

      if (TOUCH) {
        {
          AUSE(grect, GR_BUTTON1);
          SDL_SetRenderDrawColor(rendererD, V4b(button_colorptr(0)));
          SDL_RenderFillRect(rendererD, &grect);
        }
        {
          AUSE(grect, GR_BUTTON2);
          SDL_SetRenderDrawColor(rendererD, V4b(button_colorptr(2)));
          SDL_RenderFillRect(rendererD, &grect);
        }

        if (ui_textureD && mode == 0) {
          AUSE(grect, GR_MENU);
          SDL_RenderCopy(renderer, ui_textureD, NULL, &grect);
          framing_base_step(grect, 1, 1);
        }
        if (mode == 0) {
          {
            AUSE(grect, GR_LOCK);
            SDL_Texture* tex = force_runD ? lrtextureD : lwtextureD;
            SDL_RenderCopy(renderer, tex, 0, &grect);
            framing_base_step(grect, 1, 1);
          }
          {
            AUSE(grect, GR_BUTTON3);
            fn draw_b3 = force_runD ? SDL_RenderFillRect : SDL_RenderDrawRect;
            SDL_SetRenderDrawColor(rendererD, V4b(button_colorptr(1)));
            draw_b3(rendererD, &grect);
          }
        }
      }
    }
  }

  int numlock = 0;
  if (PC) numlock = SDL_GetModState() & KMOD_NUM;

  if (PC && numlock) {
    DATA char numlock_warning[] = "-please disable numlock-";
    SDL_Point p = {layout_rectD.w / 2, layout_rectD.h / 2};
    p.x -= FWIDTH * AL(numlock_warning) / 2;
    p.y -= FHEIGHT / 2;
    render_monofont_string(renderer, &fontD, AP(numlock_warning), p);
  } else {
    if (mode == 0)
      draw_game();
    else
      draw_menu(mode, using_selection);

    if (text_fnD) text_fnD(mode);
  }

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
  int grey = greyD[1];
  int white = greyD[3];
  int you = greyD[5];

  c_ptr = &caveD[row][col];
  if (mon_drawD[c_ptr->midx]) {
    color = rgba_by_palette(ORANGE);
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
        color = rgba_by_palette(TEAL);
      } else if (obj->tval == TV_DOWN_STAIR) {
        color = rgba_by_palette(RED);
      } else if (obj->tval == TV_VIS_TRAP || obj->tval == TV_RUBBLE ||
                 obj->tval == TV_CLOSED_DOOR || obj->tval == TV_OPEN_DOOR ||
                 obj->tval == TV_GLYPH) {
        color = rgba_by_palette(YELLOW);
      } else if (obj->tval == TV_SECRET_DOOR) {
        color = white;
      } else if (obj->tval != 0 && obj->tval <= TV_MAX_PICK_UP ||
                 obj->tval == TV_CHEST) {
        color = rgba_by_palette(PINK);
      } else if (obj->tval == TV_STORE_DOOR || obj->tval == TV_PAWN_DOOR) {
        color = rgba_by_palette(YELLOW);
      }
    }
  }

  if ((!color || color == grey) && (CF_TEMP_LIGHT & c_ptr->cflag)) {
    color = you;
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
  // minimapD[row - 2][col + 1] = rgba;
  // minimapD[row - 2][col + 2] = rgba;
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
        color = greyD[0];
      }

      minimapD[row][col] = color;
      if (color == rgba_by_palette(TEAL) || color == rgba_by_palette(RED))
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
  vitalstat_layout();
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
    msg_widthD = 63;  // (MAP_W / FWIDTH) - 1;
  } else if (orientation == SDL_ORIENTATION_LANDSCAPE) {
    text_fn = landscape_text;
    overlay_widthD = 78;
    msg_widthD = 92;
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
feature_menutext(mflag)
{
  char opt[2][4] = {"off", "on"};
  char* default_renderer = PC ? "opengl" : "opengles2";
  char *text, *value;

  while (mflag) {
    char tmp[32];
    tmp[0] = 0;

    uint32_t flag = mflag & -mflag;
    int line = __builtin_ffs(flag) - 1;
    char c = 'a' + line;
    switch (c) {
      case 'a':
        text = "ascii gameplay renderer";
        value = opt[globalD.sprite == 0];
        break;
      case 'b':
        text = "balrog victory";
        value = opt[uD.total_winner != 0];
        break;
      case 'c':
        text = "color interface";
        value = opt[globalD.dpad_color != 0];
        break;
      case 'd':
        text = "dpad sensitivity";
        apcati(AP(tmp), globalD.dpad_sensitivity);
        break;
      case 'g':
        text = "gpu interface";
        value = globalD.pc_renderer[0] ? globalD.pc_renderer : default_renderer;
        break;
      case 'h':
        text = "hand-swap user interface";
        value = opt[globalD.hand_swap != 0];
        break;
      case 'j':
        text = "joystick";
        value = opt[globalD.use_joystick];
        break;
      case 'l':
        text = "label button order for controllers:";
        value = opt[globalD.label_button_order != 0];
        break;
      case 'm':
        text = "magnification scale";
        apcati(AP(tmp), 1 << globalD.zoom_factor);
        break;
      case 'r':
        text = "refresh / video sync";
        value = opt[globalD.vsync != 0];
        // vsync_rateD, refresh_rateD;
        break;
      case 'o':
        text = "orientation lock";
        value = opt[globalD.orientation_lock != 0];
        break;
      case 'v':
        text = "version detail";
        value = versionD;
        break;
      default:
        text = 0;
        value = 0;
        break;
    }
    if (tmp[0]) value = tmp;
    int used = snprintf(AP(overlayD[line]), "%c) %s (%s)", c, text, value);
    if (used < 1) break;
    overlay_usedD[line] = used;
    mflag ^= flag;
  }
  return CLOBBER_MSG("feature menu");
}
int
feature_menu()
{
  int using_selection = platformD.selection != noop;

  while (1) {
    overlay_submodeD = 'f';
    int flag = 0;

    flag |= char_bit('a');
    flag |= char_bit('b');
    if (TOUCH || using_selection) flag |= char_bit('c');
    if (using_selection) flag |= char_bit('d');
    flag |= char_bit('g');
    flag |= char_bit('h');
    if (JOYSTICK) flag |= char_bit('j');
    if (JOYSTICK) flag |= char_bit('l');
    flag |= char_bit('m');
    if (!PC) flag |= char_bit('o');
    flag |= char_bit('r');
    flag |= char_bit('v');

    char c = feature_menutext(flag);
    if (is_ctrl(c)) break;

    switch (c) {
      case 'a':
        INVERT(globalD.sprite);
        break;
      case 'b':
        if (uD.total_winner) uD.total_winner = 0;
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
      case 'j':
        INVERT(globalD.use_joystick);
        joystick_update();
        break;
      case 'l':
        INVERT(globalD.label_button_order);
        joystick_update();
        break;
      case 'm':
        globalD.zoom_factor = (globalD.zoom_factor - 1) % MAX_ZOOM;
        break;
      case 'r':
        platformD.vsync(INVERT(globalD.vsync));
        break;
      case 'o':
        INVERT(globalD.orientation_lock);
        platformD.orientation(0);
        break;
      case 'v':
        show_version();
        break;
    }
  }
  return 0;
}

static int
custom_gamecrash_handler(int sig)
{
  USE(phase);
  // Crash in global_init/cosmo_init; noop

  // Crash during pregame(); switch to "software" renderer
  if (DISK && phase == PHASE_PREGAME) {
    memcpy(globalD.pc_renderer, AP("software"));
    disk_cache_write();
  }

  // Crash during play; attempt flush to disk
  if (DISK && phase == PHASE_GAME) disk_postgame();

  // Crash during postgame; noop
  return 0;
}
