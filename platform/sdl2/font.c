
#include "font_zip.c"

enum { FHEIGHT = 32 };
enum { FWIDTH = 16 };
enum { FALPHA = 225 };
// texture width/height
enum { FTEX_W = 16 };
enum { FTEX_H = 8 };

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
DATA struct SDL_Texture* font_textureD;
DATA int font_scaleD = 1;

STATIC int
font_load()
{
  int rc;
  void* bytes = &fontD;
  unsigned long size = sizeof(fontD);
  unsigned long zsize = sizeof(font_zip);
  rc = puff(bytes, &size, font_zip, &zsize);
  return rc == 0;
}

STATIC point_t
point_by_glyph(uint32_t index)
{
  int col = index % FTEX_W;
  int row = index / FTEX_W;
  return (SDL_Point){
      col * FWIDTH,
      row * FHEIGHT,
  };
}

STATIC int
glyph_init()
{
  USE(texture_format);

  struct SDL_Surface* surface;
  struct SDL_Texture* texture;
  surface = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, FWIDTH * FTEX_W,
                                           FHEIGHT * FTEX_H, 0, texture_format);
  texture = 0;
  if (surface) {
    MUSE(font, left_adjustment);

    uint8_t bpp = surface->format->BytesPerPixel;
    uint8_t* pixels = surface->pixels;
    int pitch = surface->pitch;
    int color = -1;
    if (pixel_formatD) pixel_convert(&color);

    for (int i = START_GLYPH; i < END_GLYPH; ++i) {
      int glyph_index = i - START_GLYPH;
      if (glyph_index < AL(fontD.glyph)) {
        point_t into = point_by_glyph(glyph_index);
        struct glyphS* glyph = &fontD.glyph[glyph_index];
        int ph, pw, ox, oy;
        ph = glyph->pixel_height;
        pw = glyph->pixel_width;
        ox = into.x + glyph->offset_x + left_adjustment;
        oy = into.y + glyph->offset_y;

        {
          uint8_t* src = &fontD.bitmap[glyph->bitmap_offset];
          for (int64_t row = 0; row < ph; ++row) {
            uint8_t* dst = pixels + (pitch * (oy + row)) + (bpp * ox);
            for (int64_t col = 0; col < pw; ++col) {
              if (*src) memset(dst, color, bpp);
              src += 1;
              dst += bpp;
            }
          }
        }
      }
    }

    texture = SDL_CreateTextureFromSurface(rendererD, surface);
    if (texture) SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
    SDL_FreeSurface(surface);
  }

  font_textureD = texture;

  return texture != 0;
}

int
font_init()
{
  return font_load() && glyph_init();
}

STATIC void
font_color(color)
{
  SDL_SetTextureColorMod(font_textureD, V3b(&color));
}

STATIC void
render_monofont_string(struct SDL_Renderer* renderer, struct fontS* font,
                       const char* string, int len, SDL_Point origin)
{
  USE(font_scale);
  USE(font_texture);
  rect_t target_rect = {
      .x = origin.x,
      .y = origin.y,
      .w = FWIDTH * font_scale,
      .h = FHEIGHT * font_scale,
  };

  for (int it = 0; it < len; ++it) {
    char c = string[it];
    if (char_visible(c)) {
      uint64_t glyph_index = c - START_GLYPH;
      rect_t src = (rect_t){XY(point_by_glyph(glyph_index)), FWIDTH, FHEIGHT};
      SDL_RenderCopy(renderer, font_texture, &src, &target_rect);
    }
    target_rect.x += FWIDTH * font_scale;
  }
}

STATIC void
font_texture_alphamod(alpha)
{
  SDL_SetTextureAlphaMod(font_textureD, alpha);
}

STATIC void
font_reset()
{
  SDL_SetTextureColorMod(font_textureD, 255, 255, 255);
  SDL_SetTextureAlphaMod(font_textureD, FALPHA);
}

// TBD: maybe '.' should show?
STATIC SDL_Texture*
font_texture_by_char(char c)
{
  if (char_visible(c) && c != '.') return font_textureD;
  return 0;
}

STATIC rect_t
font_rect_by_char(char c)
{
  if (char_visible(c)) {
    return (rect_t){XY(point_by_glyph(c - START_GLYPH)), FWIDTH, FHEIGHT};
  }
  return (rect_t){0};
}

#define FONT 1
