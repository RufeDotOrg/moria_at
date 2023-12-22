
#include "font_zip.c"

enum { FHEIGHT = 32 };
enum { FWIDTH = 16 };

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
DATA struct SDL_Texture* font_textureD[MAX_GLYPH];
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

STATIC int
glyph_init()
{
  USE(renderer);
  USE(texture_format);

  struct SDL_Surface* surface;
  surface = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, FWIDTH, FHEIGHT, 0,
                                           texture_format);
  if (surface) {
    uint8_t bpp = surface->format->BytesPerPixel;
    uint8_t* pixels = surface->pixels;
    int64_t pitch = surface->pitch;
    for (int i = START_GLYPH; i < END_GLYPH; ++i) {
      uint64_t glyph_index = i - START_GLYPH;
      memset(surface->pixels, 0, FHEIGHT * pitch);
      if (glyph_index < AL(fontD.glyph)) {
        struct glyphS* glyph = &fontD.glyph[glyph_index];
        int ph, pw, oy, ox;
        ph = glyph->pixel_height;
        pw = glyph->pixel_width;
        oy = glyph->offset_y;
        ox = glyph->offset_x + fontD.left_adjustment;

        {
          uint8_t* src = &fontD.bitmap[glyph->bitmap_offset];
          for (int64_t row = 0; row < ph; ++row) {
            uint8_t* dst = pixels + (pitch * (oy + row)) + (bpp * ox);
            for (int64_t col = 0; col < pw; ++col) {
              if (*src) memset(dst, -1, bpp);
              src += 1;
              dst += bpp;
            }
          }
        }

        // Glyph BlendMode is SDL_BLENDMODE_BLEND
        font_textureD[glyph_index] =
            SDL_CreateTextureFromSurface(renderer, surface);
      }
    }
    SDL_FreeSurface(surface);
  }

  for (int i = START_GLYPH; i < END_GLYPH; ++i) {
    uint64_t glyph_index = i - START_GLYPH;
    if (font_textureD[glyph_index] == 0) return 0;
  }

  return 1;
}

int
font_init()
{
  return font_load() && glyph_init();
}

STATIC void
font_color(SDL_Color c)
{
  for (int it = 0; it < AL(font_textureD); ++it) {
    SDL_SetTextureColorMod(font_textureD[it], c.r, c.g, c.b);
  }
}

STATIC void
render_monofont_string(struct SDL_Renderer* renderer, struct fontS* font,
                       const char* string, int len, SDL_Point origin)
{
  USE(font_scale);
  SDL_Rect target_rect = {
      .x = origin.x,
      .y = origin.y,
      .w = FWIDTH * font_scale,
      .h = FHEIGHT * font_scale,
  };

  for (int it = 0; it < len; ++it) {
    char c = string[it];
    if (char_visible(c)) {
      uint64_t glyph_index = c - START_GLYPH;
      struct SDL_Texture* texture = font_textureD[glyph_index];
      SDL_RenderCopy(renderer, texture, NULL, &target_rect);
    }
    target_rect.x += FWIDTH * font_scale;
  }
}

STATIC void
font_texture_alphamod(alpha)
{
  for (int it = 0; it < AL(font_textureD); ++it) {
    SDL_SetTextureAlphaMod(font_textureD[it], alpha);
  }
}

STATIC SDL_Texture*
font_texture_by_char(char c)
{
  SDL_Texture* t = 0;
  if (c == '.') return 0;
  if (char_visible(c)) {
    uint64_t glyph_index = c - START_GLYPH;
    t = font_textureD[glyph_index];
  }
  return t;
}
