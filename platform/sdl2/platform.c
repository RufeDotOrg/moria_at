#include <math.h>
#include <stdio.h>
#include <time.h>

#include "SDL.h"

#define Log SDL_Log

#include "art.c"
#include "auxval.c"
#include "dlfcn.c"
#include "font_zip.c"
#include "player.c"
#include "random.c"
#include "treasure.c"
#include "wall.c"

#include "third_party/zlib/puff.c"

#ifndef ANDROID
enum { ANDROID };
enum { TOUCH };
#else
enum { TOUCH = 1 };
#endif
#define CTRL(x) (x & 037)
#define P(p) p.x, p.y
#define R(r) r.x, r.y, r.w, r.h
#define RS(r, s) (r.x * s.w), (r.y * s.h), (r.w * s.w), (r.h * s.h)
#define C(c) c.r, c.g, c.b, c.a
#define C3(c) c.r, c.g, c.b

int
char_visible(char c)
{
  uint8_t vis = c - 0x21;
  return vis < 0x7f - 0x21;
}
// render.c
EXTERN struct SDL_Window *windowD;
EXTERN SDL_Rect display_rectD;
EXTERN float aspectD;
EXTERN struct SDL_Renderer *rendererD;
EXTERN uint32_t texture_formatD;
EXTERN SDL_PixelFormat *pixel_formatD;
EXTERN SDL_Surface *mmsurfaceD;
EXTERN SDL_Rect scale_rectD;
EXTERN float scaleD;
EXTERN int rowD, colD;
EXTERN float rfD, cfD;
static int overlay_copyD[AL(overlay_usedD)];
static SDL_Color blackD;
static SDL_Color whiteD = {255, 255, 255, 255};
static SDL_Color font_colorD;
static int xD;
static int modeD;
static int prevD;
static int finger_stackD;
static int finger_rowD;
static int finger_colD;
static int quitD;

int
render_init()
{
  windowD = SDL_CreateWindow("", 0, 0, 1920, 1080, SDL_WINDOW_FULLSCREEN);
  if (!windowD) return 0;

  int num_display = SDL_GetNumVideoDisplays();
  for (int it = 0; it < num_display; ++it) {
    SDL_Rect r;
    SDL_GetDisplayBounds(it, &r);
    Log("%d Display) %d %d %d %d\n", it, r.x, r.y, r.w, r.h);
    if (r.x == 0 && r.y == 0) {
      display_rectD = r;
      aspectD = (float)display_rectD.w / display_rectD.h;
    }
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
  }

  Log("display_rectD %d %d\n", display_rectD.w, display_rectD.h);

  return 1;
}

void
render_update()
{
  SDL_Renderer *r = rendererD;
  SDL_RenderPresent(r);
  SDL_SetRenderDrawColor(r, blackD.r, blackD.g, blackD.b, blackD.a);
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
EXTERN struct fontS fontD;
EXTERN struct SDL_Texture *font_textureD[MAX_GLYPH];

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
    CHEX(0x00000000), CHEX(0xcc000000), CHEX(0x4e9a0600), CHEX(0xc4a00000),
    CHEX(0x3465a400), CHEX(0x75507b00), CHEX(0x06989a00), CHEX(0xd3d7cf00),
    CHEX(0x55575300), CHEX(0xef292900), CHEX(0x8ae23400), CHEX(0xfce94f00),
    CHEX(0x729fcf00), CHEX(0xad7fa800), CHEX(0x34e2e200), CHEX(0xeeeeec00),
};
static uint32_t rgbaD[AL(paletteD)];
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
EXTERN uint8_t artD[96 * 1024];
EXTERN uint64_t art_usedD;
EXTERN struct SDL_Texture *art_textureD[MAX_ART];
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
    SDL_SetTextureBlendMode(art_textureD[it], SDL_BLENDMODE_NONE);
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
EXTERN uint8_t tartD[8 * 1024];
EXTERN uint64_t tart_usedD;
EXTERN struct SDL_Texture *tart_textureD[MAX_TART];
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
EXTERN uint8_t wartD[4 * 1024];
EXTERN uint64_t wart_usedD;
EXTERN struct SDL_Texture *wart_textureD[MAX_WART];
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
EXTERN uint8_t partD[4 * 1024];
EXTERN uint64_t part_usedD;
EXTERN struct SDL_Texture *part_textureD[MAX_PART];
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
  rowD = display_rectD.h / height;
  colD = display_rectD.w / width;
  Log("font %dw %dh row/col %d %d\n", width, height, rowD, colD);

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
  const char *iter = string;
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
static SDL_Color mapbgD;
static SDL_Color lightbgD;
static SDL_Color shroudbgD;

enum { TOUCH_LB = 1, TOUCH_RB, TOUCH_PAD };
EXTERN SDL_FRect buttonD[2];
EXTERN SDL_FRect padD;
EXTERN SDL_FPoint ppD[9];
static int pp_keyD[9] = {5, 6, 3, 2, 1, 4, 7, 8, 9};

EXTERN SDL_Rect map_rectD;
EXTERN SDL_Texture *map_textureD;
EXTERN SDL_Rect text_rectD;
EXTERN SDL_Texture *text_textureD;
void
texture_init()
{
  int w, h;
  w = SYMMAP_WIDTH * ART_W;
  h = SYMMAP_HEIGHT * ART_H;
  map_rectD = (SDL_Rect){.w = w, .h = h};
  map_textureD = SDL_CreateTexture(rendererD, texture_formatD,
                                   SDL_TEXTUREACCESS_TARGET, w, h);
  w = 4 * 1024;
  h = 4 * 1024;
  text_rectD = (SDL_Rect){.w = w, .h = h};
  text_textureD = SDL_CreateTexture(rendererD, texture_formatD,
                                    SDL_TEXTUREACCESS_TARGET, w, h);
}

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

SDL_FPoint
center_from_frect(r)
SDL_FRect r;
{
  return (SDL_FPoint){r.x + r.w / 2, r.y + r.h / 2};
}

#define TOUCH_DIAMETER .04f
#define TOUCH_GAP (TOUCH_DIAMETER * 1.12f)
SDL_FRect
rect_from_pp(idx)
{
  float sx = TOUCH_DIAMETER;
  float sy = TOUCH_DIAMETER * aspectD;

  SDL_FRect r = {
      ppD[idx].x - sx / 2,
      ppD[idx].y - sy / 2,
      sx,
      sy,
  };
  return r;
}

int
platform_draw()
{
  int show_map, mode, height, width, left;
  struct SDL_Texture *texture;

  show_map = 1;
  height = fontD.max_pixel_height;
  width = fontD.max_pixel_width;
  left = fontD.left_adjustment + scale_rectD.x;

  SDL_SetRenderTarget(rendererD, text_textureD);
  SDL_RenderFillRect(rendererD, &text_rectD);

  if (screen_usedD[0]) {
    mode = 2;
    show_map = 0;
    for (int row = 0; row < AL(screenD); ++row) {
      SDL_Point p = {left, (row + 1) * height};
      render_font_string(rendererD, &fontD, screenD[row], screen_usedD[row], p);
    }
  } else if (overlay_usedD[0]) {
    memcpy(overlay_copyD, overlay_usedD, sizeof(overlay_copyD));
    mode = 1;
    show_map = 0;
    for (int row = 0; row < STATUS_HEIGHT; ++row) {
      font_colorD =
          (TOUCH && row == finger_rowD) ? (SDL_Color){255, 0, 0, 255} : whiteD;
      SDL_Point p = {
          left,
          (row + 1) * height,
      };
      render_font_string(rendererD, &fontD, overlayD[row], overlay_usedD[row],
                         p);
    }
    font_colorD = whiteD;
  } else {
    mode = 0;
  }
  if (modeD != mode) {
    finger_rowD = 0;
    finger_colD = 0;
    modeD = mode;
  }

  for (int row = 0; row < STATUS_HEIGHT; ++row) {
    SDL_Point p = {0, (row + 1) * height};
    render_font_string(rendererD, &fontD, vitalinfoD[row], AL(vitalinfoD[0]),
                       p);
  }

  SDL_SetRenderTarget(rendererD, 0);
  SDL_RenderCopy(rendererD, text_textureD, NULL, &text_rectD);

  if (show_map) {
    SDL_Rect sprite_rect;
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
        uint64_t cridx = viz->cr;
        uint64_t tridx = viz->tr;

        // Art priority creature, treasure, fallback to symmap ASCII
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
        }

        switch (light) {
          case 0:
            SDL_SetRenderDrawColor(rendererD, C(mapbgD));
            break;
          case 1:
            SDL_SetRenderDrawColor(rendererD, C(shroudbgD));
            break;
          case 2:
            SDL_SetRenderDrawColor(rendererD, C(lightbgD));
            break;
        }
        SDL_RenderFillRect(rendererD, &sprite_rect);
        SDL_RenderCopy(rendererD, srct, NULL, &sprite_rect);
      }
    }
    SDL_SetRenderDrawBlendMode(rendererD, SDL_BLENDMODE_NONE);
    SDL_SetRenderTarget(rendererD, 0);

    SDL_RenderCopy(rendererD, map_textureD, NULL, &scale_rectD);
  }

  if (TOUCH) {
    {
      SDL_Color c = {0, 0, 78, 0};
      SDL_SetRenderDrawColor(rendererD, C(c));

      SDL_Rect pr = {RS(padD, display_rectD)};
      SDL_RenderFillRect(rendererD, &pr);
    }
    {
      SDL_Color c = {50, 0, 0, 0};
      SDL_SetRenderDrawColor(rendererD, C(c));

      for (int it = 0; it < AL(ppD); ++it) {
        if (ppD[it].x || ppD[it].y) {
          SDL_FRect r = rect_from_pp(it);
          SDL_Rect ppr = {RS(r, display_rectD)};
          SDL_RenderFillRect(rendererD, &ppr);
        }
      }
    }
  }

  {
    render_font_string(rendererD, &fontD, versionD, sizeof(versionD) - 1,
                       (SDL_Point){
                           display_rectD.w - 2 * MAX_WIDTH,
                           height,
                       });
    render_font_string(rendererD, &fontD, git_hashD, sizeof(git_hashD) - 1,
                       (SDL_Point){
                           display_rectD.w - width / 2 - 2 * MAX_WIDTH,
                           2 * height,
                       });
  }

  if (dun_level != 0) {
    bitmap_yx_into_surface(&minimapD[0][0], MAX_HEIGHT, MAX_WIDTH,
                           (SDL_Point){0, 0}, mmsurfaceD);
    SDL_Texture *t = SDL_CreateTextureFromSurface(rendererD, mmsurfaceD);
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_NONE);
    SDL_Rect r = {
        display_rectD.w - 2 * MAX_WIDTH - width,
        3 * height,
        2 * MAX_WIDTH,
        2 * MAX_HEIGHT,
    };
    if (minimap_enlargeD) r = scale_rectD;
    SDL_RenderCopy(rendererD, t, NULL, &r);
    SDL_DestroyTexture(t);
  }

  if (TOUCH) {
    SDL_Color c = {0, 0, 78, 0};
    SDL_SetRenderDrawColor(rendererD, C(c));

    for (int it = 0; it < AL(buttonD); ++it) {
      SDL_Rect r = {RS(buttonD[it], display_rectD)};
      SDL_RenderFillRect(rendererD, &r);
    }

    if (mode == 1) {
      SDL_Rect r = {RS(buttonD[1], display_rectD)};
      SDL_Point p = {r.x + r.w / 2, r.y + r.h / 2};
      p.x -= width / 2;
      p.y -= height / 2;
      char text = 'a' + finger_rowD;
      render_font_string(rendererD, &fontD, &text, 1, p);
    }
  }

  char *msg = AS(msg_cqD, msg_writeD);
  int msg_used = AS(msglen_cqD, msg_writeD);
  if (msg_used) {
    render_font_string(rendererD, &fontD, msg, msg_used, (SDL_Point){0, 0});
  } else if (show_map) {
    msg = AS(msg_cqD, msg_writeD - 1);
    msg_used = AS(msglen_cqD, msg_writeD - 1);
    font_texture_alphamod(128);
    render_font_string(rendererD, &fontD, msg, msg_used, (SDL_Point){0, 0});
    font_texture_alphamod(255);
  }

  SDL_Point p = {0, display_rectD.h - height};
  render_font_string(rendererD, &fontD, affectinfoD, affectinfo_usedD, p);

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
char
dir_by_scancode(sym)
{
  // Disable directional movement during fullscreen
  if (modeD) return -1;

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
int
dir_by_yx(y, x)
{
  int dir = 5;
  if (y > 0)
    dir -= 3;
  else if (y < 0)
    dir += 3;
  if (x > 0)
    dir += 1;
  else if (x < 0)
    dir -= 1;
  return dir;
}

#define xrange 0.33f
#define yrange 0.28f
char
map_touch(finger, ty, tx)
float ty, tx;
{
  int y, x;
  char c;

  x = y = 0;
  if (ty <= yrange)
    y = -1;
  else if (ty >= (1.0 - yrange))
    y = 1;
  if (tx <= xrange)
    x = -1;
  else if (tx >= (1.0 - xrange))
    x = 1;
  c = char_by_dir(dir_by_yx(y, x));
  Log("touch "
      "[ finger %d ] "
      "[ ty tx %f %f ] "
      "[ dir_by_yx %d,%d ] "
      "[ char %d ] "
      "",
      finger, ty, tx, y, x, c);

  switch (finger) {
    case 0:
      return c;
    case 1:
      if (x || y) c &= ~0x20;
      return c;
  }
  return -1;
}

int
sdl_window_event(event)
SDL_Event event;
{
  Log("SDL_WindowEvent "
      "[ event %d ] "
      "[ data1 %d data2 %d ]"
      "",
      event.window.event, event.window.data1, event.window.data2);
  if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
      (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED &&
       (scale_rectD.x == 0 || display_rectD.w != event.window.data1 ||
        display_rectD.h != event.window.data2))) {
    display_rectD.w = event.window.data1;
    display_rectD.h = event.window.data2;
    aspectD = (float)display_rectD.w / display_rectD.h;

    int dx, dy;
    dx = event.window.data1;
    dy = event.window.data2;
    int px, py;
    px = fontD.max_pixel_width;
    py = fontD.max_pixel_height;
    int mx, my;
    mx = map_rectD.w;
    my = map_rectD.h;

    // Console row/col
    rowD = display_rectD.h / py;
    colD = display_rectD.w / px;
    rfD = 1.0f / rowD;
    cfD = 1.0f / colD;
    Log("font %dw %dh console %drow %dcol rf/cf %f %f\n", px, py, rowD, colD,
        rfD, cfD);

    float xscale, yscale, scale;
    // reserve space for status width (left)
    if (px * 13 + mx <= dx) {
      xscale = 1.0f;
    } else {
      xscale = (float)(dx - px * 13) / mx;
    }
    // reserve space for top bar, bottom bar
    // hack: -8 to fit common 1080 res
    if (py * 2 - 8 + my <= dy) {
      yscale = 1.0f;
    } else {
      yscale = (float)(dy - py * 2 - 8) / my;
    }

    scale = SDL_min(xscale, yscale);
    mx = map_rectD.w * scale;
    my = map_rectD.h * scale;
    scale_rectD.x = dx / 2 - mx / 2;
    scale_rectD.y = dy / 2 - my / 2;
    scale_rectD.w = my;
    scale_rectD.h = mx;
    scaleD = scale;
    Log("Scale %.3f (%.3fx %.3fy) %dx %dy %dw %dh", scale, xscale, yscale,
        R(scale_rectD));

    // Input constraints
    padD = (SDL_FRect){0, .5, .25 - (2 * cfD), .5 - (rfD)};

    SDL_FPoint center = center_from_frect(padD);
    ppD[0] = center;
    float dist = TOUCH_GAP * 2;
    for (int it = 0; it < 8; ++it) {
      float cf = cos(it * M_PI / 4);
      float sf = sin(it * M_PI / 4);
      ppD[1 + it].x = center.x + cf * dist;
      ppD[1 + it].y = center.y + sf * dist * aspectD;
    }

    for (int it = 0; it < AL(buttonD); ++it) {
      buttonD[it] = (SDL_FRect){.77 + (.11 * it), .75 - (.22 * it), .11, .22};
    }
    return -1;
  } else if (event.window.event == SDL_WINDOWEVENT_RESTORED) {
    // Ask for a redraw
    return ' ';
  }
  return 0;
}

int
touch_from_event(event)
SDL_Event *event;
{
  int r;

  r = 0;
  SDL_FPoint tp = {event->tfinger.x, event->tfinger.y};
  for (int it = 0; it < AL(buttonD); ++it) {
    if (SDL_PointInFRect(&tp, &buttonD[it])) r = 1 + it;
  }

  for (int it = 0; it < AL(ppD); ++it) {
    SDL_FRect rect = rect_from_pp(it);
    if (SDL_PointInFRect(&tp, &rect)) {
      r = TOUCH_PAD + pp_keyD[it];
    }
  }

  if (SDL_PointInFRect(&tp, &padD)) {
    float min_dsq = FLT_MAX;
    for (int it = 0; it < AL(ppD); ++it) {
      SDL_FPoint center = ppD[it];
      float dx = tp.x - center.x;
      float dy = tp.y - center.y;
      float dsq = dx * dx + dy * dy;
      if (dsq < min_dsq) {
        min_dsq = dsq;
        r = TOUCH_PAD + pp_keyD[it];
      }
    }
  }

  return r;
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

// Game interface
char
sdl_pump()
{
  int mode, prev;
  SDL_Event event;

  mode = modeD;
  prev = prevD;
  if (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      Log("SDL_QUIT");
      quitD = TRUE;
      return 0;
    }
    if (event.type == SDL_WINDOWEVENT) {
      return sdl_window_event(event);
    }
    if (event.type == SDL_DISPLAYEVENT) {
      Log("SDL_DisplayEvent [ event %d ]", event.display.event);
      return 0;
    }
    if (event.type == SDL_KEYDOWN) {
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
      } else {
        int dir = dir_by_scancode(event.key.keysym.sym);
        if (dir > 0) return char_by_dir(dir) ^ shift;
        switch (event.key.keysym.sym) {
          case SDLK_KP_ENTER:
            return ' ';
          case SDLK_KP_PLUS:
          case SDLK_KP_PERIOD:
            return '.';
          case SDLK_KP_0:
            return 'M';
        }
      }
    }

    // Finger inputs
    SDL_FPoint motion = {0};
    int touch = 0;
    switch (event.type) {
      case SDL_FINGERDOWN:
      case SDL_FINGERUP:
        touch = touch_from_event(&event);
        break;
      case SDL_FINGERMOTION:
        motion = (SDL_FPoint){event.tfinger.x, event.tfinger.y};
        break;
      case SDL_MOUSEMOTION:
        if (TOUCH && !ANDROID) {
          motion = (SDL_FPoint){event.motion.x / (float)display_rectD.w,
                                event.motion.y / (float)display_rectD.h};
        }
        break;
    }
    switch (event.type) {
      case SDL_FINGERDOWN:
        finger_stackD += 1;
        break;
      case SDL_FINGERUP:
        finger_stackD -= 1;
        break;
    }

    int finger = event.tfinger.fingerId;
    if (TOUCH && !ANDROID) {
      finger = ((KMOD_SHIFT & SDL_GetModState()) != 0);
    }

    // Playing (Mode 0)
    if (mode == 0 && event.type == SDL_FINGERDOWN) {
      SDL_FPoint tp = {event.tfinger.x, event.tfinger.y};
      if (touch > TOUCH_PAD) {
        // SDL_FPoint rp = {(tp.x - padD.x) / padD.w, (tp.y - padD.y) /
        // padD.h}; char c = map_touch(finger, rp.y, rp.x);
        char c = char_by_dir(touch - TOUCH_PAD);
        switch (finger) {
          case 0:
            return c;
          case 1:
            if (c != ' ') return c & ~0x20;
            return c;
        }
      } else if (touch) {
        switch (touch) {
          case TOUCH_LB:
            if (finger) return 'S';
            return 'A';
          case TOUCH_RB:
            if (finger) return 'd';
            return '.';
        }
      } else {
        if (tp.x < .85 && tp.y < .09) return CTRL('p');
        if (tp.x < .25 && tp.y < .5) return 'C';
        if (tp.x > .85 && tp.y < .09) return 'v';
        if (tp.x > .85 && tp.y < .23) return 'M';
      }
    }

    // Overlay (Mode 1)
    if (mode == 1 && event.type == SDL_FINGERDOWN) {
      if (touch > TOUCH_PAD) {
        int dir = touch - TOUCH_PAD;
        int dx = dir_x(dir);
        int dy = dir_y(dir);

        if (dx && !dy) {
          finger_colD = CLAMP(finger_colD + dx, 0, 1);
          finger_rowD = 0;
        }
        if (dy && !dx) {
          finger_rowD =
              overlay_input(dy);  // CLAMP(finger_rowD + dy, 0, INVEN_EQUIP);
        }
        Log("touch %d dx %d dy finger_col %d finger_row %d", dx, dy,
            finger_colD, finger_rowD);
        return (finger_colD == 0) ? '*' : '/';
      }
      if (touch == TOUCH_LB) {
        return ESCAPE;
      }
      if (touch == TOUCH_RB) {
        return 'a' + finger_rowD;
      }
    }

    // Screen (mode 2)
    if (mode == 2) {
      if (event.type == SDL_FINGERUP) return ' ';
    }
  } else {
    nanosleep(&(struct timespec){0, 8e6}, 0);
  }

  return 0;
}

int
platform_readansi()
{
  char c = sdl_pump();
  if (isalpha(c)) prevD = c;
  if (quitD) return CTRL('c');
  return c;
}

// Disk I/O
static int checksumD;
void checksum(blob, len) void *blob;
{
  int *iter = blob;
  int count = len / sizeof(int);
  int *end = iter + count;
  for (; iter < end; ++iter) {
    checksumD ^= *iter;
  }
}
int
save()
{
  int byte_count = 0;
  for (int it = 0; it < save_field_countD; ++it) {
    Log("%p %s %d\n", save_addr_ptrD[it], save_name_ptrD[it],
        save_len_ptrD[it]);
    byte_count += save_len_ptrD[it];
  }
  SDL_RWops *writef = SDL_RWFromFile("savechar", "wb");
  if (writef) {
    checksumD = 0;
    SDL_RWwrite(writef, &byte_count, sizeof(byte_count), 1);
    for (int it = 0; it < save_field_countD; ++it) {
      SDL_RWwrite(writef, save_addr_ptrD[it], save_len_ptrD[it], 1);
      checksum(save_addr_ptrD[it], save_len_ptrD[it]);
    }
    SDL_RWclose(writef);
    return byte_count;
  }
  return 0;
}
int
load()
{
  int byte_count = 0;
  for (int it = 0; it < save_field_countD; ++it) {
    printf("%p %s %d\n", save_addr_ptrD[it], save_name_ptrD[it],
           save_len_ptrD[it]);
    byte_count += save_len_ptrD[it];
  }
  SDL_RWops *readf = SDL_RWFromFile("savechar", "rb");
  if (readf) {
    int save_size = 0;

    SDL_RWread(readf, &save_size, sizeof(save_size), 1);
    if (save_size == byte_count) {
      for (int it = 0; it < save_field_countD; ++it) {
        SDL_RWread(readf, save_addr_ptrD[it], save_len_ptrD[it], 1);
      }
    }
    SDL_RWclose(readf);
    checksumD = 0;
    for (int it = 0; it < save_field_countD; ++it) {
      checksum(save_addr_ptrD[it], save_len_ptrD[it]);
    }
    return save_size == byte_count;
  }

  return 0;
}

// Initialization
#define SDL_SCOPE (SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS)
void
platform_init()
{
  if (!SDL_WasInit(SDL_SCOPE)) {
    // SDL config
    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "0");

    // IOS/Android orientation
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeRight");
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "1");

    // Platform Input isolation
    if (ANDROID || !TOUCH) {
      SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
      SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
    }

    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
    SDL_Init(SDL_SCOPE);

    if (!render_init()) return;

    for (int it = 0; it < AL(paletteD); ++it) {
      SDL_Color c = (*(SDL_Color *)&paletteD[it]);
      rgbaD[it] = SDL_MapRGB(pixel_formatD, C3(c));
    }

    if (!font_load() || !font_init(&fontD)) return;

    texture_init();

    if (!art_io() || !art_init()) return;
    if (!tart_io() || !tart_init()) return;
    if (!wart_io() || !wart_init()) return;
    if (!part_io() || !part_init()) return;

    mmsurfaceD = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, MAX_WIDTH,
                                                MAX_HEIGHT, 0, texture_formatD);
  } else {
    for (int it = 0; it < AL(paletteD); ++it) {
      SDL_Color c = (*(SDL_Color *)&paletteD[it]);
      rgbaD[it] = SDL_MapRGB(pixel_formatD, C3(c));
    }
  }

  mapbgD = (SDL_Color){120, 120, 120, 15};
  lightbgD = (SDL_Color){220, 220, 220, 45};
  shroudbgD = (SDL_Color){170, 170, 170, 30};
  font_colorD = whiteD;

  if (ANDROID)
    platformD.seed = platform_random;
  else
    platformD.seed = platform_auxval_random;
  platformD.load = load;
  platformD.save = save;
  platformD.readansi = platform_readansi;
  platformD.draw = platform_draw;
}
void
platform_reset()
{
  // Exit terminates the android activity
  // otherwise main() may resume with stale memory
  if (ANDROID) exit(0);
}
