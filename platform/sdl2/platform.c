#include <time.h>

#include "SDL.h"

#include "art.c"
#include "font_zip.c"

#include "third_party/zlib/puff.c"

#define CTRL(x) (x & 037)
#define Log SDL_Log
#define R(r) r.x, r.y, r.w, r.h
#define C(c) c.r, c.g, c.b, c.a

BOOL
char_visible(char c)
{
  uint8_t vis = c - 0x21;
  return vis < 0x7f - 0x21;
}
// render.c
static struct SDL_Window *windowD;
static SDL_Rect display_rectD;
static float aspectD;
static struct SDL_Renderer *rendererD;
static uint32_t texture_formatD;
static SDL_PixelFormat *pixel_formatD;
static SDL_Color bg_colorD;

BOOL
render_init()
{
  windowD = SDL_CreateWindow("", 0, 0, 1920, 1080, SDL_WINDOW_FULLSCREEN);
  if (!windowD) return FALSE;

  int num_display = SDL_GetNumVideoDisplays();
  for (int it = 0; it < num_display; ++it) {
    SDL_Rect r;
    SDL_GetDisplayBounds(it, &r);
    Log("%d Display) %d %d %d %d\n", it, r.x, r.y, r.w, r.h);
    if (r.x == 0 && r.y == 0) {
      display_rectD = r;
      if (display_rectD.w > display_rectD.h) {
        aspectD = (float)display_rectD.h / display_rectD.w;
      } else {
        aspectD = (float)display_rectD.w / display_rectD.h;
      }
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
  if (!rendererD) return FALSE;

  if (SDL_GetRendererInfo(rendererD, &rinfo) != 0) return FALSE;

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
    if (SDL_GetRendererOutputSize(rendererD, &w, &h) != 0) return FALSE;
    Log("Renderer output size %d %d\n", w, h);
  }

  Log("display_rectD %d %d\n", display_rectD.w, display_rectD.h);

  return TRUE;
}

void
render_update()
{
  SDL_Renderer *r = rendererD;
  SDL_RenderPresent(r);
  SDL_SetRenderDrawColor(r, bg_colorD.r, bg_colorD.g, bg_colorD.b, bg_colorD.a);
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

void
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
static struct fontS fontD;
static const SDL_Color whiteD = {0xff, 0xff, 0xff, 0xff};
static struct SDL_Texture *font_textureD[MAX_GLYPH];
int rowD, colD;
float rfD, cfD;

void
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

BOOL
font_load()
{
  return puff((void *)&fontD, &(uint64_t){sizeof(fontD)}, font_zip,
              &(uint64_t){sizeof(font_zip)}) == 0;
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
      int val = (*src != 0) ? -1 : 0;
      memset(dst, val, bpp);
      src += 1;
      dst += bpp;
    }
  }
}

// art.c
#define ART_W 32
#define ART_H 64
#define MAX_ART 279
static uint8_t artD[96 * 1024];
static uint64_t art_usedD;
static struct SDL_Texture *art_textureD[MAX_ART];
BOOL
art_io()
{
  int rc = -1;
  art_usedD = AL(artD);
  rc = puff((void *)&artD, &art_usedD, artZ, &(uint64_t){sizeof(artZ)});
  Log("art_io() [ rc %d ] [ art_usedD %ju ]\n", rc, art_usedD);
  return rc == 0;
}

void
bitfield_to_bitmap(uint8_t *bitfield, uint8_t *bitmap, int64_t bitmap_size)
{
  int byte_count = bitmap_size / 8;
  for (int it = 0; it < byte_count; ++it) {
    for (int jt = 0; jt < 8; ++jt) {
      bitmap[it * 8 + jt] = ((bitfield[it] & (1 << jt)) != 0);
    }
  }
}
BOOL
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
    memset(surface->pixels, 0, surface->h * surface->pitch);
    bitmap_yx_into_surface(&bitmap[0][0], ART_H, ART_W, (SDL_Point){0, 0},
                           surface);
    art_textureD[it] = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(art_textureD[it], SDL_BLENDMODE_NONE);
  }
  SDL_FreeSurface(surface);

  for (int it = 0; it < AL(art_textureD); ++it) {
    if (!art_textureD[it]) return FALSE;
  }
  Log("Art textures available %ju", AL(art_textureD));

  return TRUE;
}

BOOL
font_init(struct fontS *font)
{
  struct SDL_Renderer *renderer = rendererD;
  uint32_t format = texture_formatD;

  if (font_textureD[0]) return FALSE;

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
    if (font_textureD[glyph_index] == 0) return FALSE;
  }

  return TRUE;
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
      SDL_RenderCopy(renderer, texture, NULL, &target_rect);
      target_rect.x += glyph->advance_x;
    } else {
      target_rect.x += whitespace;
    }
  }
}

// Texture
SDL_Texture *map_textureD;
SDL_Rect map_rectD;
SDL_Color mapbgD;
SDL_Rect scale_rectD;
float scaleD;
SDL_Texture *text_textureD;
SDL_Rect text_rectD;

SDL_Rect widgetD[3];
enum {
  CH_WALK,
  CH_RUN,
  CH_BASH,
  CH_ZAP,
  CH_WEAR,
  CH_TAKEOFF,
  CH_ACTUATE,
  CH_DROP
};
int inputD[AL(widgetD)];
int slide_rangeD;
int slide_incrementD;
int *ch_ptrD;

SDL_Rect
subrect_xy_wh(SDL_FPoint xy, SDL_FPoint wh)
{
  SDL_Rect r;
  r.w = wh.x * display_rectD.w;
  r.h = wh.y * display_rectD.h;
  r.x = xy.x * display_rectD.w - r.w / 2;
  r.y = xy.y * display_rectD.h - r.h / 2;
  return r;
}

void
texture_init()
{
  int w, h;
  w = SYMMAP_WIDTH * ART_W;
  h = SYMMAP_HEIGHT * ART_H;
  map_rectD = (SDL_Rect){.w = w, .h = h};
  map_textureD = SDL_CreateTexture(rendererD, texture_formatD,
                                   SDL_TEXTUREACCESS_TARGET, w, h);
  mapbgD = (SDL_Color){15, 15, 15, 0};

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
  if (char_visible(c)) {
    uint64_t glyph_index = c - START_GLYPH;
    t = font_textureD[glyph_index];
  }
  return t;
}

void
platform_draw()
{
  int show_map, height, width;
  struct SDL_Texture *texture;

  show_map = 1;
  height = fontD.max_pixel_height;
  width = fontD.max_pixel_width;

  SDL_SetRenderTarget(rendererD, text_textureD);
  SDL_RenderFillRect(rendererD, &text_rectD);

  if (screen_usedD[0]) {
    show_map = 0;
    for (int row = 0; row < AL(screenD); ++row) {
      SDL_Point p = {0, (row + 1) * height};
      render_font_string(rendererD, &fontD, screenD[row], screen_usedD[row], p);
    }
  } else if (overlay_usedD[0]) {
    show_map = 0;
    for (int row = 0; row < STATUS_HEIGHT; ++row) {
      SDL_Point p = {0, (row + 1) * height};
      render_font_string(rendererD, &fontD, overlayD[row], overlay_usedD[row],
                         p);
    }
  } else {
    for (int row = 0; row < STATUS_HEIGHT; ++row) {
      SDL_Point p = {0, (row + 1) * height};
      render_font_string(rendererD, &fontD, statusD[row], AL(statusD[0]), p);
    }
    SDL_Point p = (SDL_Point){0, display_rectD.h - height};
    render_font_string(rendererD, &fontD, debugD, debug_usedD, p);
  }
  SDL_SetRenderTarget(rendererD, 0);
  SDL_RenderCopy(rendererD, text_textureD, NULL, &text_rectD);

  if (show_map) {
    SDL_Rect sprite_rect;
    sprite_rect.w = ART_W;
    sprite_rect.h = ART_H;
    SDL_SetRenderTarget(rendererD, map_textureD);
    SDL_SetRenderDrawColor(rendererD, C(mapbgD));
    SDL_RenderFillRect(rendererD, &map_rectD);
    for (int row = 0; row < AL(symmapD); ++row) {
      sprite_rect.y = row * ART_H;
      for (int col = 0; col < SYMMAP_WIDTH; ++col) {
        uint64_t cridx = cremapD[row][col];
        char sym = symmapD[row][col];
        SDL_Texture *srct;
        sprite_rect.x = col * ART_W;
        // Creature art overlay
        if (cridx && cridx < AL(art_textureD)) {
          srct = art_textureD[cridx - 1];
        } else {
          srct = texture_by_sym(sym);
        }
        SDL_RenderCopy(rendererD, srct, NULL, &sprite_rect);
      }
    }
    SDL_SetRenderTarget(rendererD, 0);

    SDL_RenderCopy(rendererD, map_textureD, NULL, &scale_rectD);
  }

  if (slide_incrementD) {
    SDL_Color c = {0, 0, 78, 0};
    SDL_SetRenderDrawColor(rendererD, C(c));
    for (int it = 0; it < AL(widgetD); ++it) {
      SDL_RenderFillRect(rendererD, &widgetD[it]);
      SDL_Point p = {widgetD[it].x, widgetD[it].y};
      switch (inputD[it] / slide_incrementD + (it > 1) * CH_WEAR) {
        case 0:
          render_font_string(rendererD, &fontD, AP("Walk"), p);
          break;
        case 1:
          render_font_string(rendererD, &fontD, AP("Run"), p);
          break;
        case 2:
          render_font_string(rendererD, &fontD, AP("Bash"), p);
          break;
        case 3:
          render_font_string(rendererD, &fontD, AP("Zap"), p);
          break;
        case 4:
          render_font_string(rendererD, &fontD, AP("Wear"), p);
          break;
        case 5:
          render_font_string(rendererD, &fontD, AP("Takeoff"), p);
          break;
        case 6:
          render_font_string(rendererD, &fontD, AP("Activate"), p);
          break;
        case 7:
          render_font_string(rendererD, &fontD, AP("Drop"), p);
          break;
      };
    }
  }

  char *msg = AS(msg_cqD, msg_writeD);
  int msg_used = AS(msglen_cqD, msg_writeD);
  render_font_string(rendererD, &fontD, msg, msg_used, (SDL_Point){0, 0});

  render_update();
}
void
im_print()
{
  platform_draw();
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
      if (x || y) return c & ~0x20;
      return '.';
    case 2:
      return 'A';
  }
  return -1;
}

char
touch(finger, ty, tx)
float ty, tx;
{
  if (prev_cmdD == 'A') {
    float row = ty * rowD;
    int r = row;
    // -1 for line prompt
    if (r > 0) return 'a' + r - 1;
    return ESCAPE;
  }

  SDL_Point p = {tx * display_rectD.w, ty * display_rectD.h};

  if (SDL_PointInRect(&p, &scale_rectD)) {
    tx = (float)(p.x - scale_rectD.x) / scale_rectD.w;
    ty = (float)(p.y - scale_rectD.y) / scale_rectD.h;
    return map_touch(finger, ty, tx);
  } else if (p.x > (scale_rectD.x + scale_rectD.w)) {
    if (finger == 0) {
      int i = ty * AL(inputD);
      ch_ptrD = &inputD[i];
      Log("Finger on right margin %d widget %d\n", finger, i);
    }
    return ' ';
  }
  return -1;
}

char
motion(xrel, yrel)
{
  int delta = *ch_ptrD;
  int prev = delta / slide_incrementD;
  delta = CLAMP(delta + xrel, 1, slide_rangeD);
  *ch_ptrD = delta;
  if (prev != delta / slide_incrementD) return ' ';

  return 0;
}

// Game interface
char
platform_readansi()
{
  SDL_Event event;
  if (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      Log("SDL_QUIT");
      death = 1;
      new_level_flag = TRUE;
      return CTRL('c');
    }
    // Spacebar yields to the game without causing a turn to pass
    if (event.type == SDL_WINDOWEVENT) {
      Log("SDL_WindowEvent "
          "[ event %d ] "
          "[ data1 %d data2 %d ]"
          "",
          event.window.event, event.window.data1, event.window.data2);
      if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
          event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        display_rectD.w = event.window.data1;
        display_rectD.h = event.window.data2;
        if (display_rectD.w > display_rectD.h) {
          aspectD = (float)display_rectD.h / display_rectD.w;
        } else {
          aspectD = (float)display_rectD.w / display_rectD.h;
        }

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
        Log("font %dw %dh console %drow %dcol rf/cf %f %f\n", px, py, rowD,
            colD, rfD, cfD);

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
        for (int it = 0; it < AL(widgetD); ++it) {
          widgetD[it] = subrect_xy_wh((SDL_FPoint){.85, .3 + .2 * it},
                                      (SDL_FPoint){.1, .05});
        }

        slide_rangeD = (display_rectD.w - scale_rectD.w) / 2 - 2 * px;
        slide_incrementD = slide_rangeD / 4 + 1;
        for (int it = 0; it < AL(inputD); ++it) {
          inputD[it] = 1 + slide_incrementD * it;
        }
      }
      return ' ';
    }
    if (event.type == SDL_DISPLAYEVENT) {
      Log("SDL_DisplayEvent [ event %d ]", event.display.event);
      return ' ';
    }
    if (event.type == SDL_KEYDOWN) {
      if (event.key.keysym.sym < SDLK_SCANCODE_MASK) {
        SDL_Keymod km = SDL_GetModState();
        int shift = (km & KMOD_SHIFT) != 0 ? 0x20 : 0;
        if (isalpha(event.key.keysym.sym)) {
          if (km & KMOD_CTRL) return (event.key.keysym.sym & 037);
          return event.key.keysym.sym ^ shift;
        }

        return shift ? sym_shift(event.key.keysym.sym) : event.key.keysym.sym;
      }
    }
    // Prototyping choice menu
    if (event.type == SDL_MOUSEBUTTONDOWN) {
      float x, y, w, h;
      x = 1.f + event.button.x;
      y = 1.f + event.button.y;
      w = display_rectD.w + 1.f;
      h = display_rectD.h + 1.f;
      Log("MouseDown "
          "[ button %d ] "
          "[ %fy, %fx ]"
          "\n",
          event.button.button, y / h, x / w);
      return touch(event.button.button - 1, y / h, x / w);
    }
    if (event.type == SDL_FINGERDOWN) {
      return touch(event.tfinger.fingerId, event.tfinger.y, event.tfinger.x);
    }
    if (event.type == SDL_MOUSEMOTION) {
      if (ch_ptrD) {
        char m = motion(event.motion.xrel, event.motion.yrel);
        if (m) return m;
      }
    }
    if (event.type == SDL_FINGERMOTION) {
      if (ch_ptrD) {
        char m = motion(event.tfinger.dx * display_rectD.w,
                        event.tfinger.dy * display_rectD.h);
        if (m) return m;
      }
    }
    if (event.type == SDL_MOUSEBUTTONUP) {
      ch_ptrD = 0;
    }
    if (event.type == SDL_FINGERUP) {
      Log("finger up %jd\n", event.tfinger.fingerId);
      ch_ptrD = 0;
    }
  } else {
    nanosleep(&(struct timespec){0, 8e6}, 0);
  }

  return 0;
}

void
platform_init()
{
  SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
  SDL_SetHint(SDL_HINT_RENDER_VSYNC, "0");

  // IOS/Android orientation
  SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeRight");
  SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "1");

  // Ugh
  SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
  SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");

  SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

  if (!render_init()) return;

  if (!font_load() || !font_init(&fontD)) return;

  texture_init();

  if (!art_io() || !art_init()) return;
}
void
platform_reset()
{
  // Exit terminates the android activity
  // otherwise main() may resume with stale memory
  exit(0);
}
