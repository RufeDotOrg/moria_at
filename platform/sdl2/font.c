// Rufe.org LLC 2022-2025: ISC License

#include "asset/font_zlib.c"

enum { FHEIGHT = 32 };
enum { FWIDTH = 16 };
enum { FALPHA = 225 };
// texture width/height
enum { FTEX_W = 16 };
enum { FTEX_H = 8 };

struct fontS {
} fontD;
DATA struct SDL_Texture* font_textureD;
DATA struct SDL_Texture* pixel_textureD;
DATA int font_colorD;
DATA point_t font_scaleD = {FWIDTH, FHEIGHT};

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
int
font_default(int color)
{
  font_colorD = color;
}

STATIC void
render_monofont_string(struct SDL_Renderer* renderer, struct fontS* font,
                       const char* string, int len, SDL_Point origin)
{
  USE(font_texture);
  USE(font_scale);
  rect_t target_rect = {
      .x = origin.x,
      .y = origin.y,
      .w = font_scale.x,
      .h = font_scale.y,
  };

  for (int it = 0; it < len; ++it) {
    char c = string[it];
    if (char_visible(c)) {
      rect_t src = (rect_t){XY(point_by_glyph(c)), FWIDTH, FHEIGHT};
      SDL_RenderCopy(renderer, font_texture, &src, &target_rect);
    }
    target_rect.x += font_scale.x;
  }
}

STATIC int
render_monofont_block_text(struct SDL_Renderer* renderer, struct fontS* font,
                           SDL_Rect* block, void* text, int pitch)
{
  USE(font_texture);
  USE(font_scale);
  rect_t target_rect;
  point_t limit = {block->x + block->w, block->y + block->h};
  int count = 0;
  char* line = text;

  target_rect.w = font_scale.x;
  target_rect.h = font_scale.y;
  target_rect.y = block->y;
  for (int row = 0; target_rect.y < limit.y; ++row) {
    target_rect.x = block->x;
    for (int col = 0; col < pitch; ++col) {
      if (target_rect.x < limit.x) {
        char c = line[col];
        if (char_visible(c)) {
          count += 1;
          rect_t src = (rect_t){XY(point_by_glyph(c)), FWIDTH, FHEIGHT};
          SDL_RenderCopy(renderer, font_texture, &src, &target_rect);
        }
        target_rect.x += target_rect.w;
      }
    }
    target_rect.y += target_rect.h;
    line += pitch;
  }
  return count;
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
  SDL_SetTextureColorMod(font_texture, V3b(&font_colorD));
  SDL_SetTextureAlphaMod(font_texture, FALPHA);
}

STATIC rect_t
font_rect_by_char(char c)
{
  return (rect_t){XY(point_by_glyph(c)), FWIDTH, FHEIGHT};
}

#define FONT 1
