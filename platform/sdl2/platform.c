#include <setjmp.h>
#include <time.h>

#include "SDL.h"

#define Log SDL_Log

#include "art.c"
#include "font_zip.c"
#include "player.c"
#include "treasure.c"
#include "wall.c"

#include "third_party/zlib/puff.c"

#ifndef __APPLE__
enum { __APPLE__ };
#endif

#ifndef ANDROID
enum { ANDROID };
#endif

#if defined(ANDROID) || defined(__APPLE__)
enum { TOUCH = 1 };
#else
enum { TOUCH };
#endif

enum { WINDOW };
#define WINDOW_X 1920  // 1440, 1334
#define WINDOW_Y 1080  // 720, 750
#define P(p) p.x, p.y
#define R(r) r.x, r.y, r.w, r.h
#define RS(r, scale) \
  (r.x * scale.w), (r.y * scale.h), (r.w * scale.w), (r.h * scale.h)
#define RF(r, framing)                                                    \
  (SDL_Rect)                                                              \
  {                                                                       \
    .x = r.x - (framing), .y = r.y - (framing), .w = r.w + 2 * (framing), \
    .h = r.h + 2 * (framing),                                             \
  }
// FRect center
#define F4CENTER(r) r.x + r.w * .5f, r.y + r.h * .5f
#define R4CENTER(r) r.x + r.w / 2, r.y + r.h / 2
// Color
#define C(c) c.r, c.g, c.b, c.a
#define C3(c) c.r, c.g, c.b
#define U4(i) \
  (i & 0xff), ((i >> 8) & 0xff), ((i >> 16) & 0xff), ((i >> 24) & 0xff)

// TBD: clean-up
int los();

int
char_visible(char c)
{
  uint8_t vis = c - 0x21;
  return vis < 0x7f - 0x21;
}
// render.c
DATA struct SDL_Window *windowD;
DATA SDL_Rect display_rectD;
DATA float aspectD;
DATA struct SDL_Renderer *rendererD;
DATA uint32_t texture_formatD;
DATA SDL_PixelFormat *pixel_formatD;

DATA SDL_Surface *mmsurfaceD;
DATA SDL_Texture *mmtextureD;
DATA SDL_Surface *tpsurfaceD;
DATA SDL_Texture *tptextureD;
DATA SDL_Rect map_rectD;
DATA SDL_Texture *map_textureD;
DATA SDL_Rect text_rectD;
DATA SDL_Rect textdst_rectD;
DATA SDL_Texture *text_textureD;
DATA SDL_Rect affectdst_rectD;

DATA SDL_Rect gameplay_rectD;
DATA int rowD, colD;
DATA float rfD, cfD;
DATA float columnD[4];

DATA char savepathD[1024];

static int overlay_copyD[AL(overlay_usedD)];
static SDL_Color whiteD = {255, 255, 255, 255};
static SDL_Color font_colorD;
static int xD;
static int modeD;
static int submodeD;
static uint8_t finger_rowD;
static uint8_t finger_colD;
static uint8_t finger_countD;
static int quitD;
static int last_pressD;
static SDL_Point hdpi_scaleD;

#define MMSCALE 2

int
render_init()
{
  int winflag = WINDOW ? SDL_WINDOW_BORDERLESS : SDL_WINDOW_FULLSCREEN;
  if (__APPLE__) winflag |= SDL_WINDOW_ALLOW_HIGHDPI;
  windowD = SDL_CreateWindow("", 0, 0, WINDOW_X, WINDOW_Y, winflag);
  if (!windowD) return 0;

  int num_display = SDL_GetNumVideoDisplays();
  for (int it = 0; it < num_display; ++it) {
    SDL_Rect r;
    SDL_GetDisplayBounds(it, &r);
    Log("%d Display) %d %d %d %d\n", it, r.x, r.y, r.w, r.h);
  }

  int num_driver = SDL_GetNumRenderDrivers();
  Log("%d NumRenderDrivers\n", num_driver);

  SDL_RendererInfo rinfo;
  for (int it = 0; it < num_driver; ++it) {
    if (SDL_GetRenderDriverInfo(it, &rinfo) == 0) {
      Log("%d) SDL RendererInfo "
          "[ rinfo.name %s ] "
          "[ rinfo.flags 0x%08x ] "
          "\n",
          it, rinfo.name, rinfo.flags);
    }
  }
  rendererD = SDL_CreateRenderer(windowD, -1, 0);
  if (!rendererD) return 0;

  if (SDL_GetRendererInfo(rendererD, &rinfo) != 0) return 0;

  Log("SDL RendererInfo "
      "[ rinfo.name %s ] "
      "[ rinfo.flags 0x%08x ] "
      "[ max texture %d %d ] "
      "\n",
      rinfo.name, rinfo.flags, rinfo.max_texture_width,
      rinfo.max_texture_height);
  texture_formatD = rinfo.texture_formats[0];
  pixel_formatD = SDL_AllocFormat(rinfo.texture_formats[0]);

  {
    int w, h;
    if (SDL_GetRendererOutputSize(rendererD, &w, &h) != 0) return 1;
    Log("Renderer output size %d %d\n", w, h);
    hdpi_scaleD = (SDL_Point){w, h};
  }

  return 1;
}

void
render_update()
{
  SDL_Renderer *r = rendererD;
  SDL_RenderPresent(r);
  SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
  SDL_RenderClear(r);
}

// font.c
#define MAX_FOOTPRINT (32 * 1024)
#define START_GLYPH 0x21
#define END_GLYPH 0x7f
#define MAX_GLYPH (END_GLYPH - START_GLYPH)
#define GLYPH_BYTE_COUNT (sizeof(struct glyphS) * MAX_GLYPH)
#define MAX_BITMAP                                                            \
  (MAX_FOOTPRINT - sizeof(struct glyphS) * MAX_GLYPH - sizeof(uint16_t) * 4 - \
   sizeof(uint64_t))

// Glyph
// offset_x, offset_y - offset for the bitmap
// advance_x - offset applied before next glyph
// pixel_width, pixel_height - bitmap dimensions
// bitmap_offset - offset into font bitmap
// kerning is an offset based on the (current,previous) glyph pair
//  applied before rendering [not supported at this time]
struct glyphS {
  int16_t offset_x;
  int16_t offset_y;
  int16_t advance_x;
  int16_t pixel_width;
  int16_t pixel_height;
  uint32_t bitmap_offset;
};

static void
glyph_debug(struct glyphS *g)
{
  Log("glyph "
      "[ offset_x %d ] "
      "[ offset_y %d ] "
      "[ advance_x %d ] "
      "[ pixel_width %d ] "
      "[ pixel_height %d ] "
      "[ bitmap_offset %u ] "
      "\n",
      g->offset_x, g->offset_y, g->advance_x, g->pixel_width, g->pixel_height,
      g->bitmap_offset);
}

struct fontS {
  struct glyphS glyph[MAX_GLYPH];
  uint8_t bitmap[MAX_BITMAP];
  uint16_t max_pixel_width;
  uint16_t max_pixel_height;
  uint16_t font_size;
  uint16_t left_adjustment;
  uint64_t bitmap_used;
};
DATA struct fontS fontD;
DATA struct SDL_Texture *font_textureD[MAX_GLYPH];

static void
font_debug(struct fontS *font)
{
  Log("font "
      "[ max_pixel_width %u ] "
      "[ max_pixel_height %u ] "
      "[ font_size %u "
      "[ left_adjustment %u ] "
      "[ bitmap_used %ju ] "
      "\n",
      font->max_pixel_width, font->max_pixel_height, font->font_size,
      font->left_adjustment, font->bitmap_used);
  Log("  [ sizeof(struct fontS) %ju ] "
      "[ MAX_BITMAP %jd ] "
      "[ GLYPH_BYTE_COUNT %ju ] "
      "[ other_byte_count %ju ] "
      "\n",
      sizeof(struct fontS), MAX_BITMAP, GLYPH_BYTE_COUNT,
      MAX_FOOTPRINT - MAX_BITMAP - GLYPH_BYTE_COUNT);
}

// hex RGBA to little endian
#define CHEX(x) __builtin_bswap32(x)
static uint32_t paletteD[] = {
    CHEX(0x00000000), CHEX(0xcc0000ff), CHEX(0x4e9a06ff), CHEX(0xc4a000ff),
    CHEX(0x3465a4ff), CHEX(0x75507bff), CHEX(0x06989aff), CHEX(0xd3d7cfff),
    CHEX(0x555753ff), CHEX(0xef2929ff), CHEX(0x8ae234ff), CHEX(0xfce94fff),
    CHEX(0x729fcfff), CHEX(0xad7fa8ff), CHEX(0x34e2e2ff), CHEX(0xeeeeecff),
};
static uint32_t rgbaD[AL(paletteD)];
static uint32_t lightingD[] = {
    CHEX(0x161616ff),
    CHEX(0x282828ff),
    CHEX(0x3c3c3cff),
    CHEX(0x505050ff),
};
static SDL_Color *
color_by_palette(c)
{
  return (SDL_Color *)&paletteD[c];
}

void
bitmap_yx_into_surface(uint8_t *src, int64_t ph, int64_t pw, SDL_Point into,
                       struct SDL_Surface *surface)
{
  uint8_t bpp = surface->format->BytesPerPixel;
  uint8_t *pixels = surface->pixels;
  for (int64_t row = 0; row < ph; ++row) {
    uint8_t *dst = pixels + (surface->pitch * (into.y + row)) + (bpp * into.x);
    for (int64_t col = 0; col < pw; ++col) {
      memcpy(dst, &rgbaD[*src & 0xff], bpp);
      src += 1;
      dst += bpp;
    }
  }
}

void
bitfield_to_bitmap(uint8_t *bitfield, uint8_t *bitmap, int64_t bitmap_size)
{
  int byte_count = bitmap_size / 8;
  for (int it = 0; it < byte_count; ++it) {
    for (int jt = 0; jt < 8; ++jt) {
      bitmap[it * 8 + jt] = ((bitfield[it] & (1 << jt)) != 0) * 15;
    }
  }
}

// art.c
#define ART_W 32
#define ART_H 64
#define MAX_ART 279
DATA uint8_t artD[96 * 1024];
DATA uint64_t art_usedD;
DATA struct SDL_Texture *art_textureD[MAX_ART];
int
art_io()
{
  int rc = -1;
  art_usedD = AL(artD);
  rc = puff((void *)&artD, &art_usedD, artZ, &(uint64_t){sizeof(artZ)});
  Log("art_io() [ rc %d ] [ art_usedD %ju ]\n", rc, art_usedD);
  return rc == 0;
}

int
art_init()
{
  struct SDL_Renderer *renderer = rendererD;
  int byte_count = art_usedD;
  uint8_t bitmap[ART_H][ART_W];

  struct SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(
      SDL_SWSURFACE, ART_W, ART_H, 0, texture_formatD);
  uint64_t byte_used = 0;
  for (int it = 0; it < AL(art_textureD);
       ++it, byte_used += (ART_W * ART_H / 8)) {
    if (byte_used >= byte_count) break;
    bitfield_to_bitmap(&artD[byte_used], &bitmap[0][0], ART_W * ART_H);
    bitmap_yx_into_surface(&bitmap[0][0], ART_H, ART_W, (SDL_Point){0, 0},
                           surface);
    art_textureD[it] = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(art_textureD[it], SDL_BLENDMODE_BLEND);
  }
  SDL_FreeSurface(surface);

  for (int it = 0; it < AL(art_textureD); ++it) {
    if (!art_textureD[it]) return 0;
  }
  Log("Art textures available %ju", AL(art_textureD));

  return 1;
}

// treasure
#define MAX_TART 32
DATA uint8_t tartD[8 * 1024];
DATA uint64_t tart_usedD;
DATA struct SDL_Texture *tart_textureD[MAX_TART];
int
tart_io()
{
  int rc = -1;
  tart_usedD = AL(tartD);
  rc = puff((void *)&tartD, &tart_usedD, treasureZ,
            &(uint64_t){sizeof(treasureZ)});
  Log("tart_io() [ rc %d ] [ tart_usedD %ju ]\n", rc, tart_usedD);
  return rc == 0;
}

int
tart_init()
{
  struct SDL_Renderer *renderer = rendererD;
  int byte_count = tart_usedD;
  uint8_t bitmap[ART_H][ART_W];

  struct SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(
      SDL_SWSURFACE, ART_W, ART_H, 0, texture_formatD);
  uint64_t byte_used = 0;
  for (int it = 0; it < AL(tart_textureD);
       ++it, byte_used += (ART_W * ART_H / 8)) {
    if (byte_used >= byte_count) break;
    bitfield_to_bitmap(&tartD[byte_used], &bitmap[0][0], ART_W * ART_H);
    bitmap_yx_into_surface(&bitmap[0][0], ART_H, ART_W, (SDL_Point){0, 0},
                           surface);
    tart_textureD[it] = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(tart_textureD[it], SDL_BLENDMODE_BLEND);
  }
  SDL_FreeSurface(surface);

  for (int it = 0; it < AL(tart_textureD); ++it) {
    if (!tart_textureD[it]) return 0;
  }
  Log("Treasure Art textures available %ju", AL(tart_textureD));

  return 1;
}

// wall
#define MAX_WART 6
DATA uint8_t wartD[4 * 1024];
DATA uint64_t wart_usedD;
DATA struct SDL_Texture *wart_textureD[MAX_WART];
int
wart_io()
{
  int rc = -1;
  wart_usedD = AL(wartD);
  rc = puff((void *)&wartD, &wart_usedD, wallZ, &(uint64_t){sizeof(wallZ)});
  Log("wart_io() [ rc %d ] [ wart_usedD %ju ]\n", rc, wart_usedD);
  return rc == 0;
}

int
wart_init()
{
  struct SDL_Renderer *renderer = rendererD;
  int byte_count = wart_usedD;
  uint8_t bitmap[ART_H][ART_W];

  struct SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(
      SDL_SWSURFACE, ART_W, ART_H, 0, texture_formatD);
  uint64_t byte_used = 0;
  for (int it = 0; it < AL(wart_textureD);
       ++it, byte_used += (ART_W * ART_H / 8)) {
    if (byte_used >= byte_count) break;
    bitfield_to_bitmap(&wartD[byte_used], &bitmap[0][0], ART_W * ART_H);
    bitmap_yx_into_surface(&bitmap[0][0], ART_H, ART_W, (SDL_Point){0, 0},
                           surface);
    wart_textureD[it] = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(wart_textureD[it], SDL_BLENDMODE_BLEND);
  }
  SDL_FreeSurface(surface);

  for (int it = 0; it < AL(wart_textureD); ++it) {
    if (!wart_textureD[it]) return 0;
  }
  Log("Wall Art textures available %ju", AL(wart_textureD));

  return 1;
}

// player
#define MAX_PART 13
DATA uint8_t partD[4 * 1024];
DATA uint64_t part_usedD;
DATA struct SDL_Texture *part_textureD[MAX_PART];
int
part_io()
{
  int rc = -1;
  part_usedD = AL(partD);
  rc = puff((void *)&partD, &part_usedD, playerZ, &(uint64_t){sizeof(playerZ)});
  Log("part_io() [ rc %d ] [ part_usedD %ju ]\n", rc, part_usedD);
  return rc == 0;
}

int
part_init()
{
  struct SDL_Renderer *renderer = rendererD;
  int byte_count = part_usedD;
  uint8_t bitmap[ART_H][ART_W];

  struct SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(
      SDL_SWSURFACE, ART_W, ART_H, 0, texture_formatD);
  uint64_t byte_used = 0;
  for (int it = 0; it < AL(part_textureD);
       ++it, byte_used += (ART_W * ART_H / 8)) {
    if (byte_used >= byte_count) break;
    bitfield_to_bitmap(&partD[byte_used], &bitmap[0][0], ART_W * ART_H);
    bitmap_yx_into_surface(&bitmap[0][0], ART_H, ART_W, (SDL_Point){0, 0},
                           surface);
    part_textureD[it] = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(part_textureD[it], SDL_BLENDMODE_BLEND);
  }
  SDL_FreeSurface(surface);

  for (int it = 0; it < AL(part_textureD); ++it) {
    if (!part_textureD[it]) return 0;
  }
  Log("Player Art textures available %ju", AL(part_textureD));

  return 1;
}

int
font_load()
{
  return puff((void *)&fontD, &(uint64_t){sizeof(fontD)}, font_zip,
              &(uint64_t){sizeof(font_zip)}) == 0;
}

int
font_init(struct fontS *font)
{
  struct SDL_Renderer *renderer = rendererD;
  uint32_t format = texture_formatD;

  if (font_textureD[0]) return 0;

  // TBD: adopt bitfield for font pixels
  for (int it = 0; it < MAX_BITMAP; ++it) {
    if (fontD.bitmap[it]) fontD.bitmap[it] = 15;
  }

  int16_t width = font->max_pixel_width;
  int16_t height = font->max_pixel_height;

  struct SDL_Surface *surface =
      SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, width, height, 0, format);
  for (int i = START_GLYPH; i < END_GLYPH; ++i) {
    uint64_t glyph_index = i - START_GLYPH;
    memset(surface->pixels, 0, surface->h * surface->pitch);
    if (glyph_index < AL(font->glyph)) {
      struct glyphS *glyph = &font->glyph[glyph_index];
      int ph, pw, oy, ox;
      ph = glyph->pixel_height;
      pw = glyph->pixel_width;
      oy = glyph->offset_y;
      ox = glyph->offset_x + font->left_adjustment;
      bitmap_yx_into_surface(&font->bitmap[glyph->bitmap_offset], ph, pw,
                             (SDL_Point){ox, oy}, surface);
      // Glyph BlendMode is SDL_BLENDMODE_BLEND
      font_textureD[glyph_index] =
          SDL_CreateTextureFromSurface(renderer, surface);
    }
  }
  SDL_FreeSurface(surface);

  for (int i = START_GLYPH; i < END_GLYPH; ++i) {
    uint64_t glyph_index = i - START_GLYPH;
    if (font_textureD[glyph_index] == 0) return 0;
  }

  return 1;
}

SDL_Rect
font_string_rect(struct fontS *font, const char *string)
{
  uint64_t width = 0;
  uint64_t whitespace = font->max_pixel_width;

  const char *iter = string;
  while (*iter) {
    char c = *iter;
    if (char_visible(c)) {
      struct glyphS *glyph = &font->glyph[c - START_GLYPH];
      width += glyph->advance_x;
    } else {
      width += whitespace;
    }
    iter += 1;
  }

  return (SDL_Rect){.x = 0,
                    .y = 0,
                    .w = width + font->left_adjustment + font->max_pixel_width,
                    .h = font->max_pixel_height};
}

void
render_font_string(struct SDL_Renderer *renderer, struct fontS *font,
                   const char *string, int len, SDL_Point origin)
{
  SDL_Rect target_rect = {.x = origin.x + font->left_adjustment,
                          .y = origin.y,
                          .w = font->max_pixel_width,
                          .h = font->max_pixel_height};
  uint64_t whitespace = font->max_pixel_width;

  for (int it = 0; it < len; ++it) {
    char c = string[it];
    if (char_visible(c)) {
      uint64_t glyph_index = c - START_GLYPH;
      struct glyphS *glyph = &font->glyph[glyph_index];
      struct SDL_Texture *texture = font_textureD[glyph_index];
      SDL_SetTextureColorMod(texture, C3(font_colorD));
      SDL_RenderCopy(renderer, texture, NULL, &target_rect);
      SDL_SetTextureColorMod(texture, C3(whiteD));
      target_rect.x += glyph->advance_x;
    } else {
      target_rect.x += whitespace;
    }
  }
}

static void
font_texture_alphamod(alpha)
{
  for (int it = 0; it < AL(font_textureD); ++it) {
    SDL_SetTextureAlphaMod(font_textureD[it], alpha);
  }
}

// Texture
enum { TOUCH_LB = 1, TOUCH_RB, TOUCH_PAD };
DATA SDL_Rect buttonD[2];
DATA SDL_FRect padD;
DATA SDL_Point ppD[9];
DATA SDL_Rect pp_rectD;
static int pp_keyD[9] = {5, 6, 3, 2, 1, 4, 7, 8, 9};

SDL_Texture *
texture_by_sym(char c)
{
  SDL_Texture *t = 0;
  if (c == '@') return part_textureD[0 + 4];
  if (c == '.') return 0;
  if (char_visible(c)) {
    uint64_t glyph_index = c - START_GLYPH;
    t = font_textureD[glyph_index];
  }
  return t;
}

//  cos of (it * M_PI / 4);
static float cos_table[] = {1.000,  0.707,  0.000,  -0.707,
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

// Draw
int
obj_viz(obj, viz)
struct objS *obj;
struct vizS *viz;
{
  if (obj->tval != TV_INVIS_TRAP) viz->sym = obj->tchar;
  switch (obj->tval) {
      // Misc
    case TV_MISC:
      if (obj->tchar == 's')
        return 25;
      else if (obj->tchar == '!')
        return 4;
      else  // '~'
        return 23;
    case TV_CHEST:
      return 32;
    case TV_PROJECTILE:
      return 13;
    case TV_LAUNCHER:
      return 15;
    case TV_LIGHT:
      return 21;
      // Worn
    case TV_HAFTED:
      return 10;
    case TV_POLEARM:
      return 6;
    case TV_SWORD:
      return 14;
    case TV_DIGGING:
      return 10;
    case TV_BOOTS:
    case TV_GLOVES:
    case TV_CLOAK:
    case TV_HELM:
      return 11;
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
      return 19;
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
      return 29;
    case TV_UP_STAIR:
      return 30;
    case TV_DOWN_STAIR:
      return 31;
    case TV_SECRET_DOOR:
      viz->floor = 2;
      break;
  }
  return 0;
}
static int
cave_color(row, col)
{
  struct caveS *c_ptr;
  struct monS *mon;
  struct objS *obj;
  int color = 0;

  c_ptr = &caveD[row][col];
  mon = &entity_monD[c_ptr->midx];
  if (mon->mlit) {
    color = BRIGHT + MAGENTA;
  } else if (c_ptr->fval == BOUNDARY_WALL) {
    color = BRIGHT + WHITE;
  } else if (CF_VIZ & c_ptr->cflag && c_ptr->fval >= MIN_WALL) {
    color = BRIGHT + WHITE;
  } else if (CF_TEMP_LIGHT & c_ptr->cflag) {
    color = BRIGHT + CYAN;
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
      } else if (obj->tval == TV_VIS_TRAP) {
        color = BRIGHT + YELLOW;
      } else if (obj->tval == TV_SECRET_DOOR) {
        color = BRIGHT + WHITE;
      } else if (obj->tval != 0 && obj->tval <= TV_MAX_PICK_UP) {
        color = BRIGHT + BLUE;
      } else if (obj->tval == TV_STORE_DOOR || obj->tval == TV_PAWN_DOOR) {
        color = BRIGHT + GREEN;
      }
    }
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

  struct vizS *vptr = &vizD[0][0];
  blind = maD[MA_BLIND];
  py = uD.y;
  px = uD.x;
  for (int row = rmin; row < rmax; ++row) {
    for (int col = cmin; col < cmax; ++col) {
      struct caveS *c_ptr = &caveD[row][col];
      struct monS *mon = &entity_monD[c_ptr->midx];
      struct objS *obj = &entity_objD[c_ptr->oidx];
      struct vizS viz = {0};
      if (row != py || col != px) {
        viz.light = (c_ptr->cflag & CF_SEEN) != 0;
        viz.fade = fade_by_distance(py, px, row, col) - 1;
        if (mon->mlit) {
          viz.cr = mon->cidx;
        } else if (!blind && (CF_VIZ & c_ptr->cflag)) {
          switch (c_ptr->fval) {
            case GRANITE_WALL:
            case BOUNDARY_WALL:
              viz.floor = 1;
              break;
            case QUARTZ_WALL:
              viz.floor = 3 + (c_ptr->oidx != 0);
              break;
            case MAGMA_WALL:
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

        if (color == 0 &&
            (row >= rmin && row <= rmax && (col == cmin || col == cmax))) {
          color = BRIGHT + BLACK;
        } else if (color == 0 && (col >= cmin && col <= cmax &&
                                  (row == rmin || row == rmax))) {
          color = BRIGHT + BLACK;
        }

        minimapD[row][col] = color;
      }
    }
  } else {
    enum { RATIO = MAX_WIDTH / SYMMAP_WIDTH };
    for (int row = 0; row < MAX_HEIGHT / RATIO; ++row) {
      for (int col = 0; col < MAX_WIDTH / RATIO; ++col) {
        color = cave_color(row + rmin, col + cmin);
        for (int i = 0; i < 4; ++i) {
          for (int j = 0; j < 4; ++j) {
            minimapD[row * RATIO + i][col * RATIO + j] = color;
          }
        }
      }
    }
  }
}
int
platform_predraw()
{
  viz_update();
  viz_minimap();
  return 1;
}

static void
pprect_index(idx)
{
  pp_rectD.x = ppD[idx].x - pp_rectD.w / 2;
  pp_rectD.y = ppD[idx].y - pp_rectD.h / 2;
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
int
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
      finger_rowD = (subnext != 0) ? ui_stateD[subnext] : 0;
      finger_colD = (subnext == 'e') ? 1 : 0;

      overlay_autoselect();
    }
  }

  modeD = mnext;
  submodeD = subnext;

  return mnext;
}

void
alt_fill(y, x, left, top, width, height)
{
  SDL_SetRenderDrawBlendMode(rendererD, SDL_BLENDMODE_BLEND);

  SDL_SetRenderDrawColor(rendererD, U4(lightingD[1]));
  for (int row = 0; row < y; row += 2) {
    SDL_Rect rect = {
        left,
        top + row * height,
        x * width,
        height,
    };
    SDL_RenderFillRect(rendererD, &rect);
  }

  SDL_SetRenderDrawColor(rendererD, U4(lightingD[2]));
  for (int row = 1; row < y; row += 2) {
    SDL_Rect rect = {
        left,
        top + row * height,
        x * width,
        height,
    };
    SDL_RenderFillRect(rendererD, &rect);
  }
  SDL_SetRenderDrawBlendMode(rendererD, SDL_BLENDMODE_NONE);
}

void rect_frame(r, scale) SDL_Rect r;
{
  int i = scale * 3;
  SDL_SetRenderDrawColor(rendererD, U4(paletteD[BRIGHT + WHITE]));
  SDL_RenderDrawRect(rendererD, &RF(r, i));
  SDL_RenderDrawRect(rendererD, &RF(r, i + 1));
  SDL_RenderDrawRect(rendererD, &RF(r, i + 3));
}

static void
overlay_draw(width, height)
{
  int left = 0;
  int top = 0;
  SDL_Rect src_rect = {
      0,
      0,
      AL(overlayD[0]) * width + left * 2,
      AL(overlayD) * height + top * 2,
  };

  SDL_SetRenderTarget(rendererD, text_textureD);
  SDL_SetRenderDrawColor(rendererD, 0, 0, 0, 0);
  SDL_RenderFillRect(rendererD, &src_rect);

  alt_fill(AL(overlayD), AL(overlayD[0]), left, top, width, height);
  memcpy(overlay_copyD, overlay_usedD, sizeof(overlay_copyD));
  for (int row = 0; row < AL(overlayD); ++row) {
    font_colorD = whiteD;
    SDL_Point p = {
        left,
        top + row * height,
    };
    if (TOUCH && row == finger_rowD) {
      font_colorD = (SDL_Color){255, 0, 0, 255};
      if (overlay_usedD[row] <= 1) overlayD[row][0] = '-';
    }
    render_font_string(rendererD, &fontD, overlayD[row], overlay_usedD[row], p);
  }
  font_colorD = whiteD;

  SDL_SetRenderTarget(rendererD, 0);
  SDL_RenderCopy(rendererD, text_textureD, &src_rect, &textdst_rectD);
  rect_frame(textdst_rectD, 1);
}

static void
screen_draw(width, height)
{
  int left = 0;
  int top = 0;
  int max_len = 80;
  int is_text = (screen_submodeD != 0);

  for (int it = 0; it < AL(screenD); ++it) {
    max_len = MAX(max_len, screen_usedD[it]);
  }

  SDL_Rect src_rect = {
      0,
      0,
      max_len * width + left * 2,
      AL(screenD) * height + top * 2,
  };

  SDL_SetRenderTarget(rendererD, text_textureD);
  SDL_SetRenderDrawColor(rendererD, 0, 0, 0, 0);
  SDL_RenderFillRect(rendererD, &src_rect);

  if (is_text) alt_fill(AL(screenD), max_len, left, top, width, height);
  for (int row = 0; row < AL(screenD); ++row) {
    SDL_Point p = {left, top + row * height};
    render_font_string(rendererD, &fontD, screenD[row], screen_usedD[row], p);
  }

  SDL_SetRenderTarget(rendererD, 0);
  SDL_RenderCopy(rendererD, text_textureD, &src_rect, &textdst_rectD);
  if (is_text) rect_frame(textdst_rectD, 1);
}

static void
affect_draw(width, height)
{
  char tmp[80];
  int len;
  char *affstr[3];
  enum { AFF_Y = AL(active_affectD) / AL(affstr) };
  SDL_Rect src_rect = {
      0,
      0,
      26 * width,
      AFF_Y * height,
  };

  SDL_SetRenderTarget(rendererD, text_textureD);
  SDL_SetRenderDrawColor(rendererD, 0, 0, 0, 0);
  SDL_RenderFillRect(rendererD, &src_rect);

  alt_fill(AFF_Y, 26, 0, 0, width, height);
  for (int it = 0; it < AFF_Y; ++it) {
    for (int jt = 0; jt < AL(affstr); ++jt) {
      int idx = AL(affstr) * it + jt;
      if (active_affectD[idx])
        affstr[jt] = affectD[idx][active_affectD[idx] - 1];
      else
        affstr[jt] = "";
    }

    len = snprintf(tmp, AL(tmp), "%-8.08s %-8.08s %-8.08s", affstr[0],
                   affstr[1], affstr[2]);
    SDL_Point p = {
        0,
        it * height,
    };
    if (len > 0) render_font_string(rendererD, &fontD, tmp, len, p);
  }

  SDL_SetRenderTarget(rendererD, 0);
  SDL_RenderCopy(rendererD, text_textureD, &src_rect, &affectdst_rectD);
  rect_frame(affectdst_rectD, 1);
}

int
platform_draw()
{
  int show_map, mode, more, height, width, left, top, len;
  char tmp[80];

  mode = mode_change();
  switch (mode) {
    case 0:
      show_map = 1;
      more = msg_moreD;
      break;
    case 1:
      show_map = 0;
      more = 0;
      overlay_draw(fontD.max_pixel_width, fontD.max_pixel_height);
      break;
    case 2:
      show_map = 0;
      more = 0;
      screen_draw(fontD.max_pixel_width, fontD.max_pixel_height);
      break;
  }

  height = fontD.max_pixel_height;
  width = fontD.max_pixel_width;
  top = gameplay_rectD.y + 6;
  left = columnD[0] * display_rectD.w / 2;

  {
    alt_fill(AL(vitalD), 26 + 2, left, top, width, height);
    for (int it = 0; it < MAX_A; ++it) {
      len = snprintf(tmp, AL(tmp), "%-4.04s: %7d %-4.04s: %6d", vital_nameD[it],
                     vitalD[it], stat_abbrD[it], vital_statD[it]);
      SDL_Point p = {left + width / 2, top + it * height};
      if (len > 0) render_font_string(rendererD, &fontD, tmp, len, p);
    }
    {
      int it = MAX_A;
      len = snprintf(tmp, AL(tmp), "%-4.04s: %7d", vital_nameD[it], vitalD[it]);
      SDL_Point p = {left + width / 2, top + it * height};
      if (len > 0) render_font_string(rendererD, &fontD, tmp, len, p);
    }
    SDL_Rect r = {left + width / 2, top, (26 + 1) * width, AL(vitalD) * height};
    rect_frame(r, 1);
  }

  if (mode == 0) {
    affect_draw(width, height);
  }

  {
    SDL_Surface *surface = mmsurfaceD;
    SDL_Texture *texture = mmtextureD;
    bitmap_yx_into_surface(&minimapD[0][0], MAX_HEIGHT, MAX_WIDTH,
                           (SDL_Point){0, 0}, surface);
    SDL_UpdateTexture(texture, NULL, surface->pixels, surface->pitch);
    left = columnD[2] * display_rectD.w;
    int ax = display_rectD.w - left;
    int pad = (ax - MMSCALE * MAX_WIDTH) / 2;
    SDL_Rect r = {
        left + pad,
        top + height + height / 2,
        MMSCALE * MAX_WIDTH,
        MMSCALE * MAX_HEIGHT,
    };
    if (mode == 0) {
      SDL_RenderCopy(rendererD, texture, NULL, &r);
      rect_frame(r, 3);
    }

    if (minimap_enlargeD) {
      SDL_RenderCopy(rendererD, texture, NULL, &gameplay_rectD);
    }
  }

  if (show_map) {
    SDL_Rect zoom_rect;
    SDL_Rect sprite_rect;
    SDL_Point rp;
    sprite_rect.w = ART_W;
    sprite_rect.h = ART_H;
    SDL_SetRenderTarget(rendererD, map_textureD);
    SDL_SetRenderDrawColor(rendererD, 0, 0, 0, 0);
    SDL_RenderFillRect(rendererD, &map_rectD);
    SDL_SetRenderDrawBlendMode(rendererD, SDL_BLENDMODE_BLEND);
    for (int row = 0; row < SYMMAP_HEIGHT; ++row) {
      sprite_rect.y = row * ART_H;
      for (int col = 0; col < SYMMAP_WIDTH; ++col) {
        sprite_rect.x = col * ART_W;

        struct vizS *viz = &vizD[row][col];
        char sym = viz->sym;
        uint64_t fidx = viz->floor;
        uint64_t light = viz->light;
        uint64_t dim = viz->dim;
        uint64_t cridx = viz->cr;
        uint64_t tridx = viz->tr;

        // Art priority creature, wall, treasure, fallback to symmap ASCII
        SDL_Texture *srct = 0;
        if (cridx && cridx <= AL(art_textureD)) {
          srct = art_textureD[cridx - 1];
        } else if (fidx && fidx <= AL(wart_textureD)) {
          srct = wart_textureD[fidx - 1];
        } else if (tridx && tridx <= AL(tart_textureD)) {
          srct = tart_textureD[tridx - 1];
        }

        if (!srct) {
          srct = texture_by_sym(sym);
          if (sym == '@') {
            zoom_rect = sprite_rect;
            rp = (SDL_Point){col, row};
          }
        }

        SDL_SetRenderDrawColor(rendererD, U4(lightingD[light]));
        SDL_RenderFillRect(rendererD, &sprite_rect);

        if (dim) SDL_SetTextureColorMod(srct, 192, 192, 192);
        SDL_RenderCopy(rendererD, srct, NULL, &sprite_rect);
        if (dim) SDL_SetTextureColorMod(srct, 255, 255, 255);
        switch (viz->fade) {
          case 1:
            SDL_SetRenderDrawColor(rendererD, 0, 0, 0, 32);
            SDL_RenderFillRect(rendererD, &sprite_rect);
            break;
          case 2:
            SDL_SetRenderDrawColor(rendererD, 0, 0, 0, 64);
            SDL_RenderFillRect(rendererD, &sprite_rect);
            break;
          case 3:
            SDL_SetRenderDrawColor(rendererD, 0, 0, 0, 98);
            SDL_RenderFillRect(rendererD, &sprite_rect);
            break;
        }
      }
    }

    int zw, zy, zf;
    zf = zoom_factorD;

    zw = SYMMAP_WIDTH / 2 >> zf;
    zy = SYMMAP_HEIGHT / 2 >> zf;
    if (rp.x + zw >= SYMMAP_WIDTH) rp.x = SYMMAP_WIDTH - zw - 1;
    if (rp.y + zy >= SYMMAP_HEIGHT) rp.y = SYMMAP_HEIGHT - zy - 1;
    rp.x = MAX(0, rp.x - zw);
    rp.y = MAX(0, rp.y - zy);
    zoom_rect.x = rp.x * ART_W;
    zoom_rect.y = rp.y * ART_H;
    zoom_rect.w = (zw * 2 + (zf != 0)) * ART_W;
    zoom_rect.h = (zy * 2 + (zf != 0)) * ART_H;

    SDL_SetRenderDrawColor(rendererD, U4(paletteD[BRIGHT + WHITE]));
    SDL_RenderDrawRect(rendererD, &zoom_rect);

    SDL_SetRenderDrawBlendMode(rendererD, SDL_BLENDMODE_NONE);
    SDL_SetRenderTarget(rendererD, 0);

    SDL_RenderCopy(rendererD, map_textureD, &zoom_rect, &gameplay_rectD);
  }

  if (TOUCH && tpsurfaceD) {
    {
      SDL_Rect pr = {RS(padD, display_rectD)};
      SDL_RenderCopy(rendererD, tptextureD, 0, &pr);
    }
    {
      for (int it = 1; it < AL(ppD); ++it) {
        if (ppD[it].x || ppD[it].y) {
          if (pp_keyD[it] + TOUCH_PAD == last_pressD) {
            pprect_index(it);
            SDL_SetRenderDrawColor(rendererD, 0x00, 0xd0, 0, 0xff);
            SDL_RenderFillRect(rendererD, &pp_rectD);
          }
        }
      }
    }
    if (more) {
      static int tapD;
      tapD = (tapD + 1) % 4;
      switch (tapD) {
        case 0:
          font_colorD = *color_by_palette(RED);
          break;
        case 1:
          font_colorD = *color_by_palette(GREEN);
          break;
        case 2:
          font_colorD = *color_by_palette(BLUE);
          break;
        case 3:
          font_colorD = *color_by_palette(WHITE);
          break;
      }

      static char moreD[] = "-more-";
      SDL_Point p = ppD[0];
      p.x -= AL(moreD) / 2 * width;
      p.y -= height / 2;
      render_font_string(rendererD, &fontD, AP(moreD), p);

      font_colorD = whiteD;
    }
  }

  if (mode == 0) {
    left = columnD[2] * display_rectD.w;
    int ax = display_rectD.w - left;
    int pad;

    SDL_Point p = {0, top};

    len = snprintf(tmp, AL(tmp), "turn:%7d", turnD);
    pad = (ax - len * width) / 2;
    p.x = left + pad;
    render_font_string(rendererD, &fontD, tmp, len, p);

    p.y += 2 * height + MMSCALE * MAX_HEIGHT;
    len = snprintf(tmp, AL(tmp), "%s", dun_descD);
    pad = (ax - len * width) / 2;
    p.x = left + pad;
    render_font_string(rendererD, &fontD, tmp, len, p);
  }

  if (mode != 1) {
    SDL_Point p = {
        textdst_rectD.x + textdst_rectD.w + width,
        display_rectD.h - height,
    };
    render_font_string(rendererD, &fontD, versionD + 9, sizeof(versionD) - 10,
                       p);
  }

  if (TOUCH) {
    int bc[] = {RED, GREEN};

    for (int it = 0; it < AL(buttonD); ++it) {
      SDL_SetRenderDrawColor(rendererD, U4(paletteD[bc[it]]));
      SDL_RenderFillRect(rendererD, &buttonD[it]);
    }

    if (mode == 1) {
      SDL_Point p = {R4CENTER(buttonD[1])};
      p.x -= width / 2;
      p.y -= height / 2;
      char text = 'a' + finger_rowD;
      render_font_string(rendererD, &fontD, &text, 1, p);
    }
  }

  {
    char *msg = AS(msg_cqD, msg_writeD);
    int msg_used = AS(msglen_cqD, msg_writeD);
    int more_used = 0;
    int inset = __APPLE__ ? columnD[0] * display_rectD.w : width / 2;

    if (more) {
      if (map_rectD.w != gameplay_rectD.w)
        more_used = snprintf(tmp, AL(tmp), "-");
      else
        more_used = snprintf(tmp, AL(tmp), "-more %d-", more);
    }

    // Gameplay shows previous message to help the player out
    if (!msg_used && show_map) {
      msg = AS(msg_cqD, msg_writeD - 1);
      msg_used = AS(msglen_cqD, msg_writeD - 1);
      font_texture_alphamod(128);  // faded
    }

    SDL_Rect rect = {
        display_rectD.w / 2 - msg_used * width / 2,
        0,
        msg_used * width,
        height,
    };
    SDL_Rect rect2 = {
        inset,
        0,
        more_used * width,
        height,
    };
    SDL_Rect rect3 = {
        display_rectD.w - more_used * width - inset,
        0,
        more_used * width,
        height,
    };
    SDL_Point p = {rect.x, 0};
    SDL_Point p2 = {rect2.x, 0};
    SDL_Point p3 = {rect3.x, 0};

    if (msg_used) {
      SDL_SetRenderDrawBlendMode(rendererD, SDL_BLENDMODE_NONE);
      rect_frame(rect, 1);
      if (more_used) {
        rect_frame(rect2, 1);
        rect_frame(rect3, 1);
      }

      render_font_string(rendererD, &fontD, msg, msg_used, p);
      if (more_used) {
        render_font_string(rendererD, &fontD, tmp, more_used, p2);
        render_font_string(rendererD, &fontD, tmp, more_used, p3);
      }
    }
    font_texture_alphamod(255);
  }

  render_update();

  return 1;
}

char
sym_shift(char c)
{
  switch (c) {
    case '`':
      return '~';
    case '1':
      return '!';
    case '2':
      return '@';
    case '3':
      return '#';
    case '4':
      return '$';
    case '5':
      return '%';
    case '6':
      return '^';
    case '7':
      return '&';
    case '8':
      return '*';
    case '9':
      return '(';
    case '0':
      return ')';
    case '-':
      return '_';
    case '=':
      return '+';
    case '[':
      return '{';
    case ']':
      return '}';
    case ';':
      return ':';
    case '\'':
      return '"';
    case ',':
      return '<';
    case '.':
      return '>';
    case '/':
      return '?';
  }

  return c;
}
static char
dir_by_scancode(sym)
{
  switch (sym) {
    case SDLK_KP_1 ... SDLK_KP_9:
      return 1 + (sym - SDLK_KP_1);
  }
  return -1;
}
static char
char_by_dir(dir)
{
  switch (dir) {
    case 4:
      return 'h';
    case 7:
      return 'y';
    case 8:
      return 'k';
    case 9:
      return 'u';
    case 6:
      return 'l';
    case 3:
      return 'n';
    case 2:
      return 'j';
    case 1:
      return 'b';
    // As a platform choice, prefer ' ' over '.'
    // The former does not yield the turn to creatures
    default:
      return ' ';
  }
}

static int
nearest_pp(y, x)
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
  return r;
}

static void surface_ppfill(surface) SDL_Surface *surface;
{
  uint8_t bpp = surface->format->BytesPerPixel;
  uint8_t *pixels = surface->pixels;
  int oy = padD.y * display_rectD.h;
  int ox = padD.x * display_rectD.w;
  for (int64_t row = 0; row < surface->h; ++row) {
    uint8_t *dst = pixels + (surface->pitch * row);
    for (int64_t col = 0; col < surface->w; ++col) {
      memcpy(dst, &rgbaD[nearest_pp(row + oy, col + ox)], bpp);
      dst += bpp;
    }
  }
}

int
sdl_window_event(event)
SDL_Event event;
{
  int mode = modeD;
  Log("SDL_WindowEvent "
      "[ event %d ] "
      "[ data1 %d data2 %d ]"
      "",
      event.window.event, event.window.data1, event.window.data2);
  if ((event.window.event == SDL_WINDOWEVENT_RESIZED ||
       event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) &&
      (display_rectD.w != event.window.data1 ||
       display_rectD.h != event.window.data2)) {
    int dw = event.window.data1;
    int dh = event.window.data2;

    if (__APPLE__) {
      dw *= hdpi_scaleD.x / dw;
      dh *= hdpi_scaleD.y / dh;
    }
    display_rectD.w = dw;
    display_rectD.h = dh;
    aspectD = (float)dw / dh;
    Log("Window %dw%d wXh; Aspect ratio %.03f", dw, dh, aspectD);

    int fwidth, fheight;
    fwidth = fontD.max_pixel_width;
    fheight = fontD.max_pixel_height;

    // Console row/col
    rowD = dh / fheight;
    colD = dw / fwidth;
    rfD = 1.0f / rowD;
    cfD = 1.0f / colD;
    Log("font %d width %d height console %drow %dcol rf/cf %f %f\n", fwidth,
        fheight, rowD, colD, rfD, cfD);

    // Map scaling due to vertical constraint
    float scale;
    int ymin = fheight + 6;                                // text, frame
    int xmin = fwidth * (26 + 2) + (MMSCALE * MAX_WIDTH);  // touchpad, minimap
    if (ymin + map_rectD.h <= dh) {
      scale = 1.0f;
    } else {
      float yscale = (dh - ymin) / (float)map_rectD.h;
      Log("YScale %.3f", yscale);
      scale = yscale;
    }
    // Horizontal constraint
    if (xmin + map_rectD.w > dw) {
      float xscale = (dw - xmin) / (float)map_rectD.w;
      Log("XScale %.3f", xscale);
      scale = MIN(scale, xscale);
    }
    Log("Scale %.3f", scale);
    gameplay_rectD =
        (SDL_Rect){.w = map_rectD.w * scale, .h = map_rectD.h * scale};

    // Column
    float c0, c1, c2, c3;
    if (dw > 2 * 1024) {
      c1 = .5 - (float)gameplay_rectD.w / dw * .5f;
      c2 = .5 + (float)gameplay_rectD.w / dw * .5f;
    } else {
      c1 = (26 + 2) * cfD;
      c2 = c1 + ((float)gameplay_rectD.w / dw);
    }
    c3 = c2 + (1.0 - c2) * .5;
    c0 = c1 - (26 + 2) * cfD;
    columnD[0] = c0;
    columnD[1] = c1;
    columnD[2] = c2;
    columnD[3] = c3;
    Log("Column %.03f %.03f %.03f %.03f", c0, c1, c2, c3);

    // Map position
    gameplay_rectD.x = columnD[1] * dw;
    gameplay_rectD.y =
        (scale != 1.0 ? ymin : ymin + (dh - gameplay_rectD.h - ymin) / 2);

    // Affect Text
    if (scale != 1.0) {
      affectdst_rectD = (SDL_Rect){
          c2 * dw,
          gameplay_rectD.y + (MMSCALE * MAX_HEIGHT) + (fheight * 4),
          (1.0 - c2) * dw,
          5 * fheight,
      };
    } else {
      affectdst_rectD = (SDL_Rect){
          0.5f * c0 * dw + fwidth / 2,
          gameplay_rectD.y + (AL(vitalD) + 1) * fheight,
          (26 + 1) * fwidth,
          5 * fheight,
      };
    }

    // Right hand controls
    float c3w, c3h, c3o, c3button;
    c3o = c1 + AL(overlayD[0]) * cfD;
    c3w = CLAMP(1.0 - c3o, 8 * cfD, 16 * cfD);
    c3h = c3w * aspectD;
    float bmin, bmax;
    bmin = MIN(1.0 - c3w, c3);
    bmax = MAX(1.0 - c3w, c3);
    c3button = CLAMP(c3o, bmin, bmax);
    textdst_rectD = (SDL_Rect){
        gameplay_rectD.x + 6,
        gameplay_rectD.y + 6,
        MIN(AL(overlayD[0]) * fwidth, (c3button - c1) * dw),
        MIN(AL(overlayD) * fheight, dh - gameplay_rectD.y - c3h * dh),
    };
    Log("textdst %dw %dh", textdst_rectD.w, textdst_rectD.h);
    Log("wanted %jdw %jdh", AL(overlayD[0]) * fwidth, AL(overlayD) * fheight);

    // Input constraints
    if (TOUCH) {
      float lift = (dh <= 768) ? 0.f : .1f;
      padD = (SDL_FRect){.w = c1 - c0, .h = (c1 - c0) * aspectD};
      padD.x = c0 * 0.5f;
      padD.y = 1.0 - padD.h - lift;

      SDL_FPoint center = {F4CENTER(padD)};
      int cx = center.x * display_rectD.w;
      int cy = center.y * display_rectD.h;
      ppD[0] = (SDL_Point){cx, cy};

#define TOUCH_RATIO .375f
      float dist = padD.w * TOUCH_RATIO;
      Log("dist %.03f pad %.03fx%.03f", dist, padD.w, padD.h);
#define TOUCH_VIZ_RATIO .15f
      pp_rectD = (SDL_Rect){
          .w = display_rectD.w * padD.w * TOUCH_VIZ_RATIO,
          .h = display_rectD.h * padD.h * TOUCH_VIZ_RATIO,
      };

      for (int it = 0; it < 8; ++it) {
        int ox = cos_lookup(it) * dist * display_rectD.w;
        int oy = sin_lookup(it) * dist * aspectD * display_rectD.h;
        ppD[1 + it].x = cx + ox;
        ppD[1 + it].y = cy + oy;
      }

      int bw = c3w * dw;
      int bh = c3h * dh;
      Log("button %dw %dh", bw, bh);
      for (int it = 0; it < AL(buttonD); ++it) {
        SDL_Rect r = {.w = bw, .h = bh};
        r.x = textdst_rectD.x + textdst_rectD.w + 6 - (1 - it) * r.w;
        r.y = textdst_rectD.y + textdst_rectD.h + 6 - (it)*r.h;
        buttonD[it] = r;
      }

      if (tpsurfaceD) SDL_FreeSurface(tpsurfaceD);
      tpsurfaceD = SDL_CreateRGBSurfaceWithFormat(
          SDL_SWSURFACE, padD.w * dw, padD.h * dh, 0, texture_formatD);
      if (tptextureD) SDL_DestroyTexture(tptextureD);
      tptextureD = SDL_CreateTexture(rendererD, 0, SDL_TEXTUREACCESS_STREAMING,
                                     padD.w * dw, padD.h * dh);
      SDL_SetTextureBlendMode(tptextureD, SDL_BLENDMODE_NONE);
      surface_ppfill(tpsurfaceD);
      SDL_UpdateTexture(tptextureD, NULL, tpsurfaceD->pixels,
                        tpsurfaceD->pitch);
    }

    if (mode) return ' ';
    platform_draw();
  } else if (event.window.event == SDL_WINDOWEVENT_RESTORED) {
    if (mode) return ' ';
    platform_draw();
  }
  return 0;
}

int
sdl_kb_event(event)
SDL_Event event;
{
  SDL_Keymod km = SDL_GetModState();
  int shift = (km & KMOD_SHIFT) != 0 ? 0x20 : 0;

  if (event.key.keysym.sym < SDLK_SCANCODE_MASK) {
    // if (event.key.keysym.sym == ' ') xD = (xD + 1) % 8;
    if (isalpha(event.key.keysym.sym)) {
      if (km & KMOD_CTRL)
        return CTRL(event.key.keysym.sym);
      else
        return event.key.keysym.sym ^ shift;
    } else {
      return shift ? sym_shift(event.key.keysym.sym) : event.key.keysym.sym;
    }
  } else if (modeD == 0) {
    int dir = dir_by_scancode(event.key.keysym.sym);
    if (dir > 0) return char_by_dir(dir) ^ shift;
    switch (event.key.keysym.sym) {
      case SDLK_KP_ENTER:
        return ' ';
      case SDLK_KP_PLUS:
        return '+';
      case SDLK_KP_MINUS:
        return '-';
      case SDLK_KP_PERIOD:
        return '.';
      case SDLK_KP_0:
        return 'M';
    }
  }
  return 0;
}

static int
touch_from_event(event)
SDL_Event *event;
{
  int r;

  r = 0;
  SDL_FPoint tp = {event->tfinger.x, event->tfinger.y};
  SDL_Point tpp = {tp.x * display_rectD.w, tp.y * display_rectD.h};
  for (int it = 0; it < AL(buttonD); ++it) {
    if (SDL_PointInRect(&tpp, &buttonD[it])) r = 1 + it;
  }

  if (SDL_PointInFRect(&tp, &padD)) {
    int n = nearest_pp(tpp.y - padD.y, tpp.x - padD.x);
    r = TOUCH_PAD + pp_keyD[n];
  }
  last_pressD = r;

  return r;
}

int
overlay_bisect(dir)
{
  int sample[AL(overlay_copyD)];
  int sample_used, row;

  sample_used = 0;
  row = finger_rowD;
  for (int it = row; it >= 0 && it < AL(overlay_copyD); it += dir) {
    if (overlay_copyD[it] > 1) {
      sample[sample_used] = it;
      sample_used += 1;
    }
  }

  return sample[sample_used / 2];
}
int
overlay_input(input)
{
  int row = finger_rowD;
  for (int it = row + input; it >= 0 && it < AL(overlay_copyD); it += input) {
    if (overlay_copyD[it] > 1) return it;
  }
  return row;
}

static int
sdl_touch_event(event)
SDL_Event event;
{
  // Finger inputs
  int mode = modeD;
  int touch = touch_from_event(&event);
  int finger = finger_countD - 1;
  if (TOUCH && !(ANDROID || __APPLE__)) {
    finger = ((KMOD_SHIFT & SDL_GetModState()) != 0);
  }
  SDL_FPoint tp = {event.tfinger.x, event.tfinger.y};

  // Playing (Mode 0)
  if (mode == 0 && event.type == SDL_FINGERDOWN) {
    if (touch > TOUCH_PAD) {
      char c = char_by_dir(touch - TOUCH_PAD);
      switch (finger) {
        case 0:
          return c;
        case 1:
          if (c == ' ') return '=';
          return c & ~0x20;
      }
    } else if (touch) {
      switch (touch) {
        case TOUCH_LB:
          return finger ? 'd' : 'A';
        case TOUCH_RB:
          return '.';
      }
    } else {
      if (tp.y < .09) return CTRL('p');
      if (tp.x < .23 && tp.y < .5) return 'C';
      if (tp.x > .775 && tp.y < .28) return 'M';
      if (tp.x > .23 && tp.x < .775 && tp.y > .90) return '-';
      if (tp.x >= .775 && tp.y > .90) return 'v';
    }
  }

  // Overlay (Mode 1)
  if (mode == 1 && event.type == SDL_FINGERDOWN) {
    if (touch > TOUCH_PAD) {
      int dir = touch - TOUCH_PAD;
      int dx = dir_x(dir);
      int dy = dir_y(dir);

      if (!dx && !dy) {
        return 'A' + finger_rowD;
      }
      if (dx && !dy) {
        if (finger) {
          return CTRL('d');
        } else {
          finger_colD = CLAMP(finger_colD + dx, 0, 1);
        }
      }
      if (dy && !dx) {
        if (finger)
          finger_rowD = overlay_bisect(dy);
        else
          finger_rowD = overlay_input(dy);
      }
      return (finger_colD == 0) ? '*' : '/';
    }
    if (touch == TOUCH_LB) {
      return ESCAPE;
    }
    if (touch == TOUCH_RB) {
      return 'a' + finger_rowD;
    }
    if (tp.x > .23 && tp.x < .775 && tp.y > .90) return '-';
    if (tp.x < .23 && tp.y < .5 && submodeD == 'c') return 'C';
  }

  // Screen (mode 2)
  if (mode == 2 && event.type == SDL_FINGERUP) return ' ';
  if (mode == 2 && event.type == SDL_FINGERDOWN) {
    SDL_FPoint tp = {event.tfinger.x, event.tfinger.y};
    if (touch == TOUCH_LB) return 'o';
    if (touch == TOUCH_RB) return ESCAPE;
    if (tp.y < .09) return CTRL('p');
    if (tp.x < .23 && tp.y < .5) return 'C';
    if (tp.x >= .775 && tp.y > .90) return 'v';
  }
  return 0;
}

// Game interface
char
sdl_pump()
{
  SDL_Event event;
  int ret = 0;

  while (ret == 0 && SDL_PollEvent(&event)) {
    if (TOUCH && (event.type == SDL_FINGERDOWN || event.type == SDL_FINGERUP)) {
      finger_countD += (event.type == SDL_FINGERDOWN);
      ret = sdl_touch_event(event);
      finger_countD -= (event.type == SDL_FINGERUP);
    } else if (!TOUCH && (event.type == SDL_KEYDOWN)) {
      ret = sdl_kb_event(event);
    } else if (event.type == SDL_QUIT) {
      quitD = TRUE;
    } else if (event.type == SDL_WINDOWEVENT) {
      ret = sdl_window_event(event);
    } else if (event.type == SDL_DISPLAYEVENT) {
      Log("SDL_DisplayEvent [ event %d ]", event.display.event);
    }
  }

  if (ret == 0) {
    nanosleep(&(struct timespec){0, 8e6}, 0);
  }

  return ret;
}

int
platform_readansi()
{
  char c = sdl_pump();
  if (quitD) return CTRL('c');
  return c;
}

int
platform_random()
{
  int ret = -1;
  SDL_RWops *readf = SDL_RWFromFile("/dev/urandom", "rb");
  if (readf) SDL_RWread(readf, &ret, sizeof(ret), 1);
  SDL_RWclose(readf);
  Log("seed %d", ret);
  return ret;
}

// Disk I/O
static int checksumD;
int
version_by_savesum(sum)
{
  for (int it = 0; it < AL(savesumD); ++it)
    if (savesumD[it] == sum) return it;
  return -1;
}
int
savesum()
{
  int sum = 0;
  for (int it = 0; it < AL(save_bufD); ++it) {
    struct bufS buf = save_bufD[it];
    sum += buf.mem_size;
  }
  return sum;
}
int
clear_savebuf()
{
  for (int it = 0; it < AL(save_bufD); ++it) {
    struct bufS buf = save_bufD[it];
    memset(buf.mem, 0, buf.mem_size);
  }
  memset(msglen_cqD, 0, sizeof(msglen_cqD));
  return 0;
}
int checksum(blob, len) void *blob;
{
  int *iter = blob;
  int count = len / sizeof(int);
  int *end = iter + count;
  int ret = 0;
  for (; iter < end; ++iter) {
    ret ^= *iter;
  }
  return ret;
}
int
platform_save()
{
  int version = AL(savesumD) - 1;
  int sum = savesumD[version];
  int *savefield = savefieldD[version];
  char *path = __APPLE__ ? savepathD : "savechar";

  SDL_RWops *writef = SDL_RWFromFile(path, "wb");
  if (writef) {
    checksumD = 0;
    SDL_RWwrite(writef, &sum, sizeof(sum), 1);
    for (int it = 0; it < AL(save_bufD); ++it) {
      struct bufS buf = save_bufD[it];
      SDL_RWwrite(writef, buf.mem, savefield[it], 1);
      int ck = checksum(buf.mem, savefield[it]);
      checksumD ^= ck;
    }
    SDL_RWclose(writef);
    Log("version %d save checksum %x", version, checksumD);
    return sum;
  }
  return 0;
}
int
devsave()
{
  int sum = savesum();
  char *path = __APPLE__ ? savepathD : "savechar";
  SDL_RWops *writef = SDL_RWFromFile(path, "wb");
  if (writef) {
    checksumD = 0;
    SDL_RWwrite(writef, &sum, sizeof(sum), 1);
    for (int it = 0; it < AL(save_bufD); ++it) {
      struct bufS buf = save_bufD[it];
      SDL_RWwrite(writef, buf.mem, buf.mem_size, 1);
      int ck = checksum(buf.mem, buf.mem_size);
      checksumD ^= ck;
    }
    SDL_RWclose(writef);
    Log("save checksum %x", checksumD);
    return sum;
  }
  return 0;
}
int
platform_load()
{
  int save_size = 0;
  char *path = __APPLE__ ? savepathD : "savechar";
  clear_savebuf();

  SDL_RWops *readf = SDL_RWFromFile(path, "rb");
  if (readf) {
    checksumD = 0;
    SDL_RWread(readf, &save_size, sizeof(save_size), 1);
    int version = version_by_savesum(save_size);
    if (version >= 0) {
      int *savefield = savefieldD[version];
      for (int it = 0; it < AL(save_bufD); ++it) {
        struct bufS buf = save_bufD[it];
        SDL_RWread(readf, buf.mem, savefield[it], 1);
        int ck = checksum(buf.mem, savefield[it]);
        checksumD ^= ck;
      }
      Log("version %d load char checksum %x", version, checksumD);
    } else if (save_size == savesum()) {
      for (int it = 0; it < AL(save_bufD); ++it) {
        struct bufS buf = save_bufD[it];
        SDL_RWread(readf, buf.mem, buf.mem_size, 1);
        int ck = checksum(buf.mem, buf.mem_size);
        checksumD ^= ck;
      }
      Log("load char checksum %x", checksumD);
    } else {
      Log("load char invalid size %d", save_size);
      save_size = 0;
    }

    SDL_RWclose(readf);
  }

  return save_size != 0;
}
int
platform_erase()
{
  char *path = __APPLE__ ? savepathD : "savechar";
  clear_savebuf();

  SDL_RWops *writef = SDL_RWFromFile(path, "w+b");
  if (writef) SDL_RWclose(writef);

  return 0;
}

int
platform_selection(int *yptr, int *xptr)
{
  *yptr = finger_colD;
  *xptr = finger_rowD;
  return modeD == 1;
}

// Initialization
#define SDL_SCOPE (SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS)
int
platform_pregame()
{
  int init;

  init = !SDL_WasInit(SDL_SCOPE);
  if (init) {
    // SDL config
    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "0");

    // __APPLE__/Android orientation
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeRight");

    // Platform Input isolation
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI, "0");
    // Mouse->Touch
    if (TOUCH)
      SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "1");
    else
      SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
    // Touch->Mouse
    if (ANDROID) SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");

    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
    SDL_Init(SDL_SCOPE);

    if (__APPLE__ || ANDROID) SDL_DisableScreenSaver();

    if (__APPLE__) {
      char *prefpath = SDL_GetPrefPath("org.rufe", "moria.app");
      if (prefpath) {
        int len = snprintf(savepathD, AL(savepathD), "%s/savechar", prefpath);
        if (len < 0 || len >= AL(savepathD)) savepathD[0] = 0;
        SDL_free(prefpath);
      }
    }

    if (!render_init()) return 1;
  }

  for (int it = 0; it < AL(paletteD); ++it) {
    rgbaD[it] = SDL_MapRGBA(pixel_formatD, U4(paletteD[it]));
  }

  if (init) {
    if (!font_load() || !font_init(&fontD)) return 2;

    if (!art_io() || !art_init()) return 3;
    if (!tart_io() || !tart_init()) return 3;
    if (!wart_io() || !wart_init()) return 3;
    if (!part_io() || !part_init()) return 3;

    mmsurfaceD = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, MAX_WIDTH,
                                                MAX_HEIGHT, 0, texture_formatD);
    mmtextureD = SDL_CreateTexture(rendererD, 0, SDL_TEXTUREACCESS_STREAMING,
                                   MAX_WIDTH, MAX_HEIGHT);
    SDL_SetTextureBlendMode(mmtextureD, SDL_BLENDMODE_NONE);
    map_rectD =
        (SDL_Rect){.w = SYMMAP_WIDTH * ART_W, .h = SYMMAP_HEIGHT * ART_H};
    map_textureD =
        SDL_CreateTexture(rendererD, texture_formatD, SDL_TEXTUREACCESS_TARGET,
                          map_rectD.w, map_rectD.h);
    SDL_SetTextureBlendMode(map_textureD, SDL_BLENDMODE_NONE);
    text_rectD = (SDL_Rect){.w = 2 * 1024, .h = 2 * 1024};
    text_textureD =
        SDL_CreateTexture(rendererD, texture_formatD, SDL_TEXTUREACCESS_TARGET,
                          text_rectD.w, text_rectD.h);
  }

  font_colorD = whiteD;

  if (ANDROID || __APPLE__) zoom_factorD = 2;

  if (WINDOW) {
    SDL_Event event;
    event.window.event = SDL_WINDOWEVENT_RESIZED;
    event.window.data1 = WINDOW_X;
    event.window.data2 = WINDOW_Y;
    sdl_window_event(event);
  }

  while (gameplay_rectD.x == 0) {
    sdl_pump();
  }

  platformD.selection = platform_selection;
  return 0;
}
int
platform_postgame()
{
  // Exit terminates the android activity
  // otherwise main() may resume with stale memory
  if (ANDROID) exit(0);
  return 0;
}
