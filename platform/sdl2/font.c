// Rufe.org LLC 2022-2024: ISC License

#include "asset/font_zlib.c"

enum { FWIDTH = 16 };
enum { FHEIGHT = 32 };
enum { FALPHA = 225 };
// texture width/height
enum { FTEX_W = 16 };
enum { FTEX_H = 8 };

struct fontS {
} fontD;
DATA struct SDL_Texture* font_textureD;
DATA struct SDL_Texture* pixel_textureD;

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
  SDL_Texture* texture = 0;
  struct SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(
      SDL_SWSURFACE, FWIDTH * FTEX_W, FHEIGHT * FTEX_H, 0,
      SDL_PIXELFORMAT_INDEX8);

  if (surface) {
    {
      SDL_Palette* palette = surface->format->palette;
      for (int it = 0; it < 256; ++it) {
        int alpha = it > 16 ? 255 : 0;
        palette->colors[it] = (SDL_Color){it, it, it, alpha};
      }
    }

    int rc;
    unsigned long size = FWIDTH * FTEX_W * FHEIGHT * FTEX_H;
    unsigned long zsize = font_zlib_len;
    rc = puff(surface->pixels, &size, font_zlib, &zsize);
    Log("glyph_init puff rc %d\n", rc);

    if (rc == 0) texture = SDL_CreateTextureFromSurface(rendererD, surface);

    if (rc == 0) {
      SDL_Palette* palette = surface->format->palette;
      for (int it = 0; it < 256; ++it) {
        int alpha = it > 16 ? 255 : 0;
        int c = it < 128 ? 0 : 255;
        palette->colors[it] = (SDL_Color){c, c, c, alpha};
      }
      pixel_textureD = SDL_CreateTextureFromSurface(rendererD, surface);
      if (pixel_textureD) SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }

    SDL_FreeSurface(surface);
  }

  if (texture) SDL_SetTextureScaleMode(texture, SDL_ScaleModeLinear);
  if (texture) SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

  font_textureD = texture;

  return texture != 0;
}

int
font_init()
{
  return glyph_init();
}

STATIC void
render_monofont_string(struct SDL_Renderer* renderer, struct fontS* font,
                       const char* string, int len, SDL_Point origin)
{
  USE(font_texture);
  rect_t target_rect = {
      .x = origin.x,
      .y = origin.y,
      .w = FWIDTH,
      .h = FHEIGHT,
  };

  for (int it = 0; it < len; ++it) {
    char c = string[it];
    if (char_visible(c)) {
      rect_t src = (rect_t){XY(point_by_glyph(c)), FWIDTH, FHEIGHT};
      SDL_RenderCopy(renderer, font_texture, &src, &target_rect);
    }
    target_rect.x += FWIDTH;
  }
}

STATIC void
font_color(color)
{
  SDL_SetTextureColorMod(font_textureD, V3b(&color));
}

STATIC void
font_alpha(alpha)
{
  SDL_SetTextureAlphaMod(font_textureD, alpha);
}

STATIC void
font_reset()
{
  USE(font_texture);
  SDL_SetTextureColorMod(font_texture, 255, 255, 255);
  SDL_SetTextureAlphaMod(font_texture, FALPHA);
}

STATIC rect_t
font_rect_by_char(char c)
{
  return (rect_t){XY(point_by_glyph(c)), FWIDTH, FHEIGHT};
}

#define FONT 1
