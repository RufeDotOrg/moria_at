#include <setjmp.h>
#include <time.h>

#include "SDL.h"

#define Log SDL_Log

#include "third_party/zlib/puff.c"

#define ORGNAME "org.rufe"
#define APPNAME "moria.app"
#define CACHENAME "moria.cache"
#define SAVENAME "savechar"  // TBD custom
#ifndef __APPLE__
enum { __APPLE__ };
char* SDL_AppleGetDocumentPath(const char*, const char*);
#endif

#ifndef ANDROID
enum { ANDROID };
int SDL_AndroidGetExternalStorageState();
char* SDL_AndroidGetExternalStoragePath();
#endif

enum { FONT = 1 };
#if defined(ANDROID) || defined(__APPLE__)
enum { TOUCH = 1 };
enum { KEYBOARD = 0 };
enum { MOUSE = 0 };
#else
enum { TOUCH = 0 };
enum { KEYBOARD = 1 };
enum { MOUSE = TOUCH };
#endif

enum { SDL_EVLOG = 0 };
enum { BATCHING = 1 };
enum { REORIENTATION = 1 };
#define ORIENTATION_LIST \
  "Portrait LandscapeRight PortraitUpsideDown LandscapeLeft"
enum { VSYNC = 1 };
enum { QUALITY = 0 };

enum { WINDOW = 0 };
#define WINDOW_X 1920  // 1440, 1334
#define WINDOW_Y 1080  // 720, 750

// optional: layout restriction
enum { PORTRAIT = 0 };
#define PORTRAIT_X 1080
#define PORTRAIT_Y 1920
enum { LANDSCAPE = 0 };
#define LANDSCAPE_X 1920
#define LANDSCAPE_Y 1080

// The game directly includes platform for iterative dev
// Custom platform code is may be included after the game based on this define
// Custom code may depend on game logic AND platform specifics
#define CUSTOM_SETUP 1
// Color
#define U4(i) \
  (i & 0xff), ((i >> 8) & 0xff), ((i >> 16) & 0xff), ((i >> 24) & 0xff)

enum { ART_W = 32 };
enum { ART_H = 64 };
enum { SPRITE_SQ = 32 };
enum { MAP_W = SYMMAP_WIDTH * ART_W };
enum { MAP_H = SYMMAP_HEIGHT * ART_H };
enum { MMSCALE = 2 };

// render.c
DATA struct SDL_Window* windowD;
DATA int refresh_rateD;
DATA SDL_Rect display_rectD;
DATA SDL_Rect safe_rectD;
DATA struct SDL_Renderer* rendererD;
DATA uint32_t texture_formatD;
DATA SDL_PixelFormat* pixel_formatD;
DATA int orientation_lockD;

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
DATA SDL_Texture* portraitD;
DATA SDL_Texture* landscapeD;
DATA SDL_Texture* layoutD;
DATA SDL_Rect layout_rectD;
DATA SDL_FRect view_rectD;
DATA uint32_t max_texture_widthD;
DATA uint32_t max_texture_heightD;

enum {
  GR_VERSION,
  GR_PAD,
  GR_BUTTON1,
  GR_BUTTON2,
  GR_GAMEPLAY,
  GR_MINIMAP,
  GR_HISTORY,
  GR_LOCK,
  GR_STAT,
  GR_OVERLAY,
  GR_WIDESCREEN,  // show_history() in landscape orientation
  GR_COUNT
};
enum { MAX_BUTTON = 2 };
SDL_Rect grectD[GR_COUNT];
DATA fn text_fnD;

DATA char savepathD[1024];
DATA int savepath_usedD;
DATA char exportpathD[1024];
DATA int exportpath_usedD;
DATA char cachepathD[1024];
DATA int cachepath_usedD;

GAME int overlay_copyD[AL(overlay_usedD)];
GAME int modeD;
GAME int submodeD;
GAME uint8_t finger_rowD;
GAME uint8_t finger_colD;

DATA SDL_Color whiteD = {255, 255, 255, 255};
DATA int xD;
DATA uint8_t finger_countD;
DATA int quitD;
DATA int last_pressD;
DATA float retina_scaleD;
DATA char moreD[] = "-more-";
enum { STRLEN_MORE = AL(moreD) - 1 };

int
render_init()
{
  int winflag = WINDOW ? SDL_WINDOW_BORDERLESS : SDL_WINDOW_FULLSCREEN;
  if (__APPLE__) winflag |= SDL_WINDOW_ALLOW_HIGHDPI;
  if (REORIENTATION) winflag |= SDL_WINDOW_RESIZABLE;
  windowD = SDL_CreateWindow("", 0, 0, WINDOW_X, WINDOW_Y, winflag);
  if (!windowD) return 0;

  int use_display = SDL_GetWindowDisplayIndex(windowD);
  int num_display = SDL_GetNumVideoDisplays();
  for (int it = 0; it < num_display; ++it) {
    SDL_Rect r;
    SDL_GetDisplayBounds(it, &r);
    Log("%d Display) %d %d %d %d\n", it, r.x, r.y, r.w, r.h);
    if (it == use_display) {
      SDL_DisplayMode mode;
      SDL_GetCurrentDisplayMode(it, &mode);
      Log(" -> Refresh Rate %d\n", mode.refresh_rate);
      refresh_rateD = mode.refresh_rate;
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
  // ANDROID fix for SDL Error: BLASTBufferQueue
  if (ANDROID) SDL_RenderPresent(rendererD);
  // APPLE fix for visual artifacts on first frame
  if (__APPLE__) SDL_RenderClear(rendererD);

  if (SDL_GetRendererInfo(rendererD, &rinfo) != 0) return 0;

  Log("SDL RendererInfo "
      "[ rinfo.name %s ] "
      "[ rinfo.flags 0x%08x ] "
      "[ max texture %d %d ] "
      "\n",
      rinfo.name, rinfo.flags, rinfo.max_texture_width,
      rinfo.max_texture_height);
  Log("vsync %d", (rinfo.flags & SDL_RENDERER_PRESENTVSYNC) != 0);

  max_texture_widthD = rinfo.max_texture_width;
  max_texture_heightD = rinfo.max_texture_height;
  texture_formatD = rinfo.texture_formats[0];
  pixel_formatD = SDL_AllocFormat(rinfo.texture_formats[0]);

  {
    int rw, rh;
    if (SDL_GetRendererOutputSize(rendererD, &rw, &rh) != 0) return 1;
    Log("Renderer output size %d %d\n", rw, rh);

    int ww, wh;
    SDL_GetWindowSize(windowD, &ww, &wh);
    retina_scaleD = MAX((float)rw / ww, (float)rh / wh);
  }

  return 1;
}
void
render_update()
{
  USE(renderer);
  USE(layout);

  if (layout) {
    SDL_SetRenderTarget(renderer, 0);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    USE(display_rect);
    USE(view_rect);
    SDL_Rect target = {
        view_rect.x * display_rect.w,
        view_rect.y * display_rect.h,
        view_rect.w * display_rect.w,
        view_rect.h * display_rect.h,
    };
    SDL_RenderCopy(renderer, layout, NULL, &target);
  }

  SDL_RenderPresent(renderer);
}

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

#include "font.c"

// Texture
enum {
  TOUCH_NONE,
  TOUCH_HISTORY,
  TOUCH_LOCK,
  TOUCH_STAT,
  TOUCH_MAP,
  TOUCH_VERSION,
  TOUCH_GAMEPLAY,
  TOUCH_LB,
  TOUCH_RB,
  TOUCH_PAD
};
DATA SDL_Point ppD[9];
DATA int pp_keyD[9] = {5, 6, 3, 2, 1, 4, 7, 8, 9};

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

static void
display_resize(int dw, int dh)
{
  Log("display_resize %dx%d", dw, dh);
  display_rectD.w = dw;
  display_rectD.h = dh;

  // TBD: Review game utilization of viewport
  // Disabled the push event in SDL that occurs on another thread
  SDL_RenderSetViewport(rendererD, &(SDL_Rect){0, 0, dw, dh});
}

static int
orientation_update()
{
  USE(display_rect);
  int orientation = display_rect.w > display_rect.h ? SDL_ORIENTATION_LANDSCAPE
                                                    : SDL_ORIENTATION_PORTRAIT;
  if (LANDSCAPE) orientation = SDL_ORIENTATION_LANDSCAPE;
  if (PORTRAIT) orientation = SDL_ORIENTATION_PORTRAIT;

  USE(layout_rect);
  USE(safe_rect);
  float scale = 1.f;
  if (orientation == SDL_ORIENTATION_LANDSCAPE) {
    layoutD = landscapeD;
    layout_rect = (SDL_Rect){0, 0, LANDSCAPE_X, LANDSCAPE_Y};

    {
      // safe_rect is respected on the orientation axis
      float fw = safe_rect.w ? safe_rect.w : display_rect.w;
      float fh = display_rect.h;
      float xscale = fw / layout_rect.w;
      float yscale = fh / layout_rect.h;
      scale = MIN(xscale, yscale);
      Log("orientation %d %.03f %.03f %d %d sw dh", orientation, xscale, yscale,
          safe_rect.w, display_rect.h);
    }
  } else if (orientation == SDL_ORIENTATION_PORTRAIT) {
    layoutD = portraitD;
    layout_rect = (SDL_Rect){0, 0, PORTRAIT_X, PORTRAIT_Y};

    {
      // safe_rect is respected on the orientation axis
      float fw = display_rect.w;
      float fh = safe_rect.h ? safe_rect.h : display_rect.h;
      float xscale = fw / layout_rect.w;
      float yscale = fh / layout_rect.h;
      scale = MIN(xscale, yscale);
      Log("orientation %d %.03f %.03f %d %d dw sh", orientation, xscale, yscale,
          display_rect.w, safe_rect.h);
    }
  } else {
    layoutD = 0;
    text_fnD = 0;
    layout_rect = display_rectD;
  }
  layout_rectD = layout_rect;

  // Note tension: center of display vs. center of safe area
  //   affects visual aesthetic
  //   affects input positioning for touch
  SDL_Rect ar_rect = {0, 0, layout_rect.w * scale, layout_rect.h * scale};
  ar_rect.x = (display_rect.w - ar_rect.w) / 2;
  ar_rect.y = MAX(safe_rect.y, (display_rect.h - ar_rect.h) / 2);

  float xuse = (float)ar_rect.w / display_rect.w;
  float yuse = (float)ar_rect.h / display_rect.h;
  float xpad = (float)ar_rect.x / display_rect.w;
  float ypad = (float)ar_rect.y / display_rect.h;

  Log("orientation %d scale %f: %.03f %.03f xuse yuse %.03f %.03f xpad ypad",
      orientation, scale, xuse, yuse, xpad, ypad);
  SDL_FRect view = {xpad, ypad, xuse, yuse};
  view_rectD = view;

  return customD.orientation(orientation);
}

char*
path_append_filename(char* path, int path_len, char* filename)
{
  int wridx = path_len;
  char* write = &path[wridx];

  if (wridx) *write++ = '/';
  for (char* iter = filename; *iter != 0; ++iter) {
    *write++ = *iter;
  }
  *write = 0;

  return path;
}
SDL_RWops*
file_access(char* filename, char* access)
{
  SDL_RWops* ret = SDL_RWFromFile(filename, access);
  if (ret != 0) Log("%s file_access %s", access, filename);
  return ret;
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
  if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
      event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
    int drw = display_rectD.w;
    int drh = display_rectD.h;
    int dw = event.window.data1;
    int dh = event.window.data2;

    if (__APPLE__) {
      SDL_GetWindowSafeRect(windowD, &safe_rectD);
      safe_rectD.x *= retina_scaleD;
      safe_rectD.y *= retina_scaleD;
      safe_rectD.w *= retina_scaleD;
      safe_rectD.h *= retina_scaleD;
      dw *= retina_scaleD;
      dh *= retina_scaleD;
    } else {
      safe_rectD = (SDL_Rect){0, 0, dw, dh};
    }

    if (dw != drw || dh != drh) {
      display_resize(dw, dh);
      orientation_update();

      // android 11 devices don't render the first frame (e.g. samsung A20)
      if (ANDROID) SDL_RenderPresent(rendererD);

      if (mode) {
        return (finger_colD == 0) ? '*' : '/';
      } else if (drw) {
        customD.draw();
      }
    }
  } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
    if (display_rectD.w != 0) {
      // android 11 devices don't render the first frame (e.g. samsung A20)
      if (ANDROID) SDL_RenderPresent(rendererD);

      if (mode)
        return (finger_colD == 0) ? '*' : '/';
      else
        customD.draw();
    }
  } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
    if (ANDROID || __APPLE__) platformD.postgame(0);
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

int
gameplay_tapxy(relx, rely)
{
  MUSE(global, zoom_factor);
  float gsy = rely;
  float gsx = relx;
  int cellw = SYMMAP_WIDTH;
  int cellh = SYMMAP_HEIGHT;

  if (zoom_factor) {
    cellh >>= zoom_factor;
    cellw >>= zoom_factor;

    cellh += 1;
    cellw += 1;

    gsy *= cellh;
    gsx *= cellw;

    gsy /= SYMMAP_HEIGHT;
    gsx /= SYMMAP_WIDTH;
  }

  int ry = (int)gsy / ART_H;
  int rx = (int)gsx / ART_W;
  ylookD = MIN(ry, cellh - 1);
  xlookD = MIN(rx, cellw - 1);
  return 0;
}

int
overlay_end()
{
  for (int it = AL(overlay_copyD) - 1; it > 0; --it) {
    if (overlay_copyD[it] > 2) return it;
  }
  return AL(overlay_copyD) - 1;
}
int
overlay_bisect(dir)
{
  int sample[AL(overlay_copyD)];
  int sample_used;
  int row = finger_rowD;

  sample[0] = CLAMP(row + dir, 0, AL(overlay_copyD) - 1);
  sample_used = 0;
  for (int it = row; it >= 0 && it < AL(overlay_copyD); it += dir) {
    if (overlay_copyD[it] > 2) {
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
touch_by_xy(x, y)
{
  SDL_Point tpp = {x, y};
  {
    AUSE(grect, GR_STAT);
    if (SDL_PointInRect(&tpp, &grect)) {
      return TOUCH_STAT;
    }
  }
  {
    AUSE(grect, GR_HISTORY);
    if (SDL_PointInRect(&tpp, &grect)) {
      return TOUCH_HISTORY;
    }
  }
  {
    AUSE(grect, GR_LOCK);
    if (SDL_PointInRect(&tpp, &grect)) {
      return TOUCH_LOCK;
    }
  }
  {
    AUSE(grect, GR_MINIMAP);
    if (SDL_PointInRect(&tpp, &grect)) {
      return TOUCH_MAP;
    }
  }

  {
    AUSE(grect, GR_VERSION);
    if (SDL_PointInRect(&tpp, &grect)) {
      return TOUCH_VERSION;
    }
  }

  {
    AUSE(grect, GR_GAMEPLAY);
    if (SDL_PointInRect(&tpp, &grect)) {
      gameplay_tapxy(x - grect.x, y - grect.y);
      return TOUCH_GAMEPLAY;
    }
  }

  for (int it = 0; it < MAX_BUTTON; ++it) {
    AUSE(grect, GR_BUTTON1 + it);
    if (SDL_PointInRect(&tpp, &grect)) return TOUCH_LB + it;
  }

  {
    AUSE(grect, GR_PAD);
    int r = 0;
    if (SDL_PointInRect(&tpp, &grect)) {
      SDL_Point rel = {tpp.x - grect.x, tpp.y - grect.y};

      int n = nearest_pp(rel.y, rel.x);
      r = TOUCH_PAD + pp_keyD[n];
    }

    last_pressD = r;

    return r;
  }
}
static int
orientation_lock_toggle()
{
  // This works; TBD persistent global configuration
  SDL_SetWindowResizable(windowD, orientation_lockD);
  orientation_lockD = ~orientation_lockD;
  return 0;
}
static int
fingerdown_xy_mode(x, y, mode)
{
  int finger = finger_countD - 1;
  if (KEYBOARD) {
    finger = ((KMOD_SHIFT & SDL_GetModState()) != 0);
  }

  int touch = touch_by_xy(x, y);
  if (mode == 0) {
    if (touch > TOUCH_PAD) {
      char c = char_by_dir(touch - TOUCH_PAD);
      switch (finger) {
        case 0:
          return c;
        case 1:
          if (c == ' ') return '=';
          return c & ~0x20;
        default:
          break;
      }
    } else if (touch) {
      switch (touch) {
        case TOUCH_HISTORY:
          return CTRL('p');
        case TOUCH_LOCK:
          orientation_lock_toggle();
          return ' ';
        case TOUCH_STAT:
          return 'C';
        case TOUCH_MAP:
          return 'M';
        case TOUCH_VERSION:
          return 'v';
        case TOUCH_GAMEPLAY:
          return finger ? '-' : 'O';
        case TOUCH_LB:
          return finger ? 'd' : 'A';
        case TOUCH_RB:
          return finger ? CTRL('a') : '.';
        default:
          break;
      }
    }
  }
  if (mode == 1) {
    if (touch > TOUCH_PAD) {
      int dir = touch - TOUCH_PAD;
      int dx = dir_x(dir);
      int dy = dir_y(dir);

      if (!dx && !dy) {
        return 'A' + finger_rowD;
      }
      if (dx && !dy) {
        if (finger)
          finger_rowD = dx > 0 ? overlay_end() : 0;
        else
          finger_colD = CLAMP(finger_colD + dx, 0, 1);
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
    if (touch == TOUCH_GAMEPLAY && finger) return '-';
  }
  if (mode == 2) {
    if (touch == TOUCH_LB) return 'o';
    if (touch == TOUCH_RB) return ESCAPE;
    if (touch == TOUCH_STAT) return 'C';
    if (touch == TOUCH_HISTORY) return CTRL('p');
    if (touch) return ' ';
  }

  return 0;
}

// Game interface
char
sdl_pump()
{
  USE(view_rect);
  USE(layout_rect);
  USE(mode);
  SDL_Event event;
  int ret = 0;
  // TBD: fastplay in portrait mode for now
  int fastplay = (MOUSE || TOUCH) ? (layout_rectD.h > layout_rectD.w) : 0;

  while (ret == 0 && SDL_PollEvent(&event)) {
    if ((MOUSE || TOUCH) && event.type == SDL_FINGERDOWN) {
      finger_countD += 1;
      SDL_FPoint tp = {event.tfinger.x, event.tfinger.y};
      if (SDL_PointInFRect(&tp, &view_rect)) {
        int x = (tp.x - view_rect.x) / view_rect.w * layout_rect.w;
        int y = (tp.y - view_rect.y) / view_rect.h * layout_rect.h;
        ret = fingerdown_xy_mode(x, y, mode);
      }
    } else if ((MOUSE || TOUCH) && event.type == SDL_FINGERUP) {
      finger_countD -= 1;
      if (fastplay) ret = ' ';
    } else if (KEYBOARD && (event.type == SDL_KEYDOWN)) {
      ret = sdl_kb_event(event);
    } else if (event.type == SDL_QUIT) {
      quitD = TRUE;
    } else if (event.type == SDL_WINDOWEVENT) {
      ret = sdl_window_event(event);
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
  SDL_RWops* readf = SDL_RWFromFile("/dev/urandom", "rb");
  if (readf) {
    SDL_RWread(readf, &ret, sizeof(ret), 1);
    SDL_RWclose(readf);
  }
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

  return 0;
}
int checksum(blob, len) void* blob;
{
  int* iter = blob;
  int count = len / sizeof(int);
  int* end = iter + count;
  int ret = 0;
  for (; iter < end; ++iter) {
    ret ^= *iter;
  }
  return ret;
}
// filename unchanged unless a valid classidx is specified
char*
filename_by_class(char* filename, int classidx)
{
  if (classidx >= 0 && classidx < AL(classD)) {
    char* dst = &filename[4];
    for (char* src = classD[classidx].name; *src != 0; ++src) {
      *dst++ = *src | 0x20;
    }
    *dst = 0;
  }
  Log("filename_by_class %s", filename);
  return filename;
}

int
path_exists(char* path)
{
  SDL_RWops* readf = file_access(path, "rb");
  uint32_t save_size = 0;
  if (readf) {
    SDL_RWread(readf, &save_size, sizeof(save_size), 1);
    SDL_RWclose(readf);
  }
  return save_size != 0;
}
int
path_delete(char* path)
{
  SDL_RWops* writef = file_access(path, "wb");
  if (writef) SDL_RWclose(writef);
  return 1;
}
int
path_save(char* path)
{
  int version = AL(savesumD) - 1;
  int sum = savesumD[version];
  int* savefield = savefieldD[version];

  SDL_RWops* writef = file_access(path, "wb");
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
    Log("path_save %s: version %d save checksum %x", path, version, checksumD);
    return sum;
  }
  return 0;
}
int
path_load(char* path)
{
  int save_size = 0;
  clear_savebuf();

  SDL_RWops* readf = file_access(path, "rb");
  if (readf) {
    checksumD = 0;
    SDL_RWread(readf, &save_size, sizeof(save_size), 1);
    int version = version_by_savesum(save_size);
    if (version >= 0) {
      int* savefield = savefieldD[version];
      for (int it = 0; it < AL(save_bufD); ++it) {
        struct bufS buf = save_bufD[it];
        SDL_RWread(readf, buf.mem, savefield[it], 1);
        int ck = checksum(buf.mem, savefield[it]);
        checksumD ^= ck;
      }
    } else if (save_size == savesum()) {
      for (int it = 0; it < AL(save_bufD); ++it) {
        struct bufS buf = save_bufD[it];
        SDL_RWread(readf, buf.mem, buf.mem_size, 1);
        int ck = checksum(buf.mem, buf.mem_size);
        checksumD ^= ck;
      }
    } else {
      save_size = 0;
    }

    if (input_resumeD == 0) {
      if (save_size) {
        char gh[AL(git_hashD)];
        if (SDL_RWread(readf, gh, sizeof(gh), 1)) {
          int sum = 0;
          if (memcmp(gh, git_hashD, sizeof(gh)) == 0) {
            for (int it = 0; it < AL(midpoint_bufD); ++it) {
              sum += midpoint_bufD[it].mem_size;
            }
            int64_t offset = SDL_RWseek(readf, 0, RW_SEEK_CUR);
            int64_t end = SDL_RWseek(readf, sum, RW_SEEK_CUR);
            if (end > 0) {
              SDL_RWseek(readf, offset, RW_SEEK_SET);
              for (int it = 0; it < AL(midpoint_bufD); ++it) {
                struct bufS buf = midpoint_bufD[it];
                if (!SDL_RWread(readf, buf.mem, buf.mem_size, 1)) sum = 0;
              }
            }
          }

          if (sum) {
            input_resumeD = input_action_usedD;
          } else {
            input_resumeD = 0;
            uD.new_level_flag = NL_MIDPOINT_LOST;
          }
        }
      }
    }

    SDL_RWclose(readf);
  }

  return save_size != 0;
}
int
path_savemidpoint(char* path)
{
  int save_size = 0;
  int write_ok = 0;
  int memory_ok;

  memory_ok = (input_record_writeD <= AL(input_recordD) - 1 &&
               input_action_usedD <= AL(input_actionD) - 1);

  if (memory_ok) {
    SDL_RWops* rwfile = file_access(path, "rb+");
    if (rwfile) {
      SDL_RWread(rwfile, &save_size, sizeof(save_size), 1);

      int64_t offset = SDL_RWseek(rwfile, save_size, RW_SEEK_CUR);
      if (offset > 0) {
        write_ok = SDL_RWwrite(rwfile, &git_hashD, sizeof(git_hashD), 1);
        for (int it = 0; it < AL(midpoint_bufD); ++it) {
          struct bufS buf = midpoint_bufD[it];
          if (!SDL_RWwrite(rwfile, buf.mem, buf.mem_size, 1)) write_ok = 0;
        }
      }

      SDL_RWclose(rwfile);
    }
  }
  return write_ok;
}
int
platform_load(saveslot, external)
{
  char filename[16] = SAVENAME;
  filename_by_class(filename, saveslot);
  char* path;
  if (external) {
    path = path_append_filename(exportpathD, exportpath_usedD, filename);
  } else {
    path = path_append_filename(savepathD, savepath_usedD, filename);
  }

  return path_load(path);
}
int
platform_save(saveslot)
{
  char filename[16] = SAVENAME;
  filename_by_class(filename, saveslot);
  char* path = path_append_filename(savepathD, savepath_usedD, filename);
  return path_save(path);
}
int
platform_erase(saveslot, external)
{
  char filename[16] = SAVENAME;
  filename_by_class(filename, saveslot);
  char* path;
  if (external) {
    path = path_append_filename(exportpathD, exportpath_usedD, filename);
  } else {
    path = path_append_filename(savepathD, savepath_usedD, filename);
  }
  return path_delete(path);
}
int
platform_savemidpoint()
{
  MUSE(global, saveslot_class);
  if (saveslot_class >= 0 && saveslot_class < AL(classD)) {
    char filename[16] = SAVENAME;
    filename_by_class(filename, saveslot_class);
    char* path = path_append_filename(savepathD, savepath_usedD, filename);
    return path_savemidpoint(path);
  }
  return 0;
}
int
platform_saveex()
{
  char filename[16] = SAVENAME;
  int count = 0;

  for (int it = 0; it < AL(classD); ++it) {
    filename_by_class(filename, it);
    char *in_path, *ex_path;
    in_path = path_append_filename(savepathD, savepath_usedD, filename);
    ex_path = path_append_filename(exportpathD, exportpath_usedD, filename);
    if (path_load(in_path)) count += (path_save(ex_path) != 0);
  }
  return count;
}
int
cache_write()
{
  SDL_RWops* writef = file_access(cachepathD, "wb");
  if (writef) {
    int ret = SDL_RWwrite(writef, &globalD, sizeof(globalD), 1);
    Log("cache_write: %d", ret);
    SDL_RWclose(writef);
  }
  return writef != 0;
}

int
path_copy_to(char* srcpath, char* dstpath)
{
  SDL_RWops *readf, *writef;
  readf = file_access(srcpath, "rb");
  if (readf) {
    writef = file_access(dstpath, "wb");
    if (writef) {
      char chunk[4 * 1024];
      int read_count;
      do {
        read_count = SDL_RWread(readf, chunk, 1, AL(chunk));
        if (read_count) {
          int write_count = SDL_RWwrite(writef, chunk, 1, read_count);
          if (write_count != read_count) return 1;
        }
      } while (read_count);
      SDL_RWclose(writef);
    }
    SDL_RWclose(readf);
  }

  return readf == 0 || writef == 0;
}

int
platform_selection(int* yptr, int* xptr)
{
  *yptr = finger_colD;
  *xptr = finger_rowD;
  return modeD == 1;
}
int
cache_read()
{
  uint32_t success = 0;
  if (cachepath_usedD) {
    SDL_RWops* readf = file_access(cachepathD, "rb");
    if (readf) {
      if (SDL_RWread(readf, &globalD, sizeof(globalD), 1))
        success = sizeof(globalD);
      SDL_RWclose(readf);
    }
  }

  return success;
}
int
cache_default()
{
  // try to be FUN on the platform with defaults
  if (ANDROID || __APPLE__) globalD.zoom_factor = 2;
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

// Initialization
#define SDL_SCOPE (SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS)
int
platform_pregame()
{
  int init;

  init = !SDL_WasInit(SDL_SCOPE);
  if (init) {
    if (!RELEASE) Log("Initializing development build");
    if (!RELEASE) SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

    // SDL config
    if (SDL_EVLOG) SDL_SetHint(SDL_HINT_EVENT_LOGGING, "1");
    if (BATCHING) SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
    if (VSYNC) SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    if (QUALITY) SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    if (__APPLE__) SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
    // SDL_SetHint(SDL_HINT_RENDER_METAL_PREFER_LOW_POWER_DEVICE, "1");

    // iOS/Android orientation
    SDL_SetHint(SDL_HINT_ORIENTATIONS, ORIENTATION_LIST);

    // Platform Screensaver
    if (__APPLE__ || ANDROID) SDL_DisableScreenSaver();
    // Platform Input isolation
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI, "0");
    // Mouse->Touch
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, MOUSE ? "1" : "0");
    // Touch->Mouse
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");

    SDL_Init(SDL_SCOPE);

    if (__APPLE__) {
      char* prefpath = SDL_GetPrefPath(ORGNAME, APPNAME);
      if (prefpath) {
        int len = snprintf(savepathD, AL(savepathD), "%s", prefpath);
        if (len < 0 || len >= AL(savepathD))
          savepathD[0] = 0;
        else
          savepath_usedD = len;
        SDL_free(prefpath);
      }

      char* external = SDL_AppleGetDocumentPath(ORGNAME, APPNAME);
      if (external) {
        int len = snprintf(exportpathD, AL(exportpathD), "%s", external);
        if (len < 0 || len >= AL(exportpathD)) {
          exportpathD[0] = 0;
        } else {
          exportpath_usedD = len;
        }
        SDL_free(external);
      }

      if (SL(CACHENAME)) {
        // Apple allows user interactions with the external path files
        memcpy(cachepathD, exportpathD, exportpath_usedD);
        cachepath_usedD = exportpath_usedD;
      }
    }

    if (ANDROID) {
      int state = SDL_AndroidGetExternalStorageState();
      if (state & 0x3) {
        int len = 0;
        char* external = (char*)SDL_AndroidGetExternalStoragePath();
        if (external) {
          len = snprintf(exportpathD, AL(exportpathD), "%s", external);
          Log("GetExternalStoragePath: %s", external);
          SDL_free(external);
        }
        if (len <= 0 || len >= AL(exportpathD)) {
          exportpathD[0] = 0;
        } else {
          exportpath_usedD = len;
        }
        Log("storage: [state %d] exportpath: %s", state, exportpathD);
      }

      if (SL(CACHENAME)) {
        char* cache = SDL_GetCachePath(ORGNAME, APPNAME);
        if (cache) {
          Log("SDL_GetCachePath: %s", cache);
          int len = snprintf(cachepathD, AL(cachepathD), "%s", cache);
          if (len <= 0 || len >= AL(cachepathD)) {
            cachepathD[0] = 0;
            len = 0;
          }

          cachepath_usedD = len;
          SDL_free(cache);
        }
      }
    }

    if (SL(CACHENAME) && cachepath_usedD) {
      path_append_filename(cachepathD, cachepath_usedD, CACHENAME);
      cachepath_usedD += SL(CACHENAME);
      Log("Game cache enabled: %s", cachepathD);
    }

    if (!render_init()) return 1;
  }

  if (!cache_read()) cache_default();
  Log("SDL global\n"
      " %d saveslot_class\n"
      " %u zoom_factor\n",
      globalD.saveslot_class, globalD.zoom_factor);

  for (int it = 0; it < AL(paletteD); ++it) {
    rgbaD[it] = SDL_MapRGBA(pixel_formatD, U4(paletteD[it]));
  }

  if (FONT && init) {
    if (!font_load() || !font_init()) return 2;
  }

  if (init) {
    USE(renderer);
    USE(texture_format);
    portraitD =
        SDL_CreateTexture(renderer, texture_format, SDL_TEXTUREACCESS_TARGET,
                          PORTRAIT_X, PORTRAIT_Y);
    landscapeD =
        SDL_CreateTexture(renderer, texture_format, SDL_TEXTUREACCESS_TARGET,
                          LANDSCAPE_X, LANDSCAPE_Y);
  }

#ifdef CUSTOM_SETUP
  custom_setup();
#endif
  platformD.seed = platform_random;
  platformD.load = platform_load;
  platformD.save = platform_save;
  platformD.erase = platform_erase;
  platformD.readansi = platform_readansi;
  platformD.predraw = customD.predraw;
  platformD.draw = customD.draw;
  if (TOUCH) platformD.selection = platform_selection;
  platformD.savemidpoint = platform_savemidpoint;

  if (exportpath_usedD) {
    platformD.saveex = platform_saveex;
  }

  if (WINDOW) {
    SDL_Event event;
    event.window.event = SDL_WINDOWEVENT_RESIZED;
    event.window.data1 = WINDOW_X;
    event.window.data2 = WINDOW_Y;
    sdl_window_event(event);
  }

  while (display_rectD.w == 0) {
    sdl_pump();
  }

  customD.pregame();

  // Migration code
  if (platformD.load(-1, 0)) fs_upgrade();

  return init;
}
int
platform_postgame(may_exit)
{
  platform_savemidpoint();
  if (cachepath_usedD) cache_write();

  // Android closure does not require the process to end.
  // exit(...) ensures the process terminates.
  // otherwise main() should handle resume with previous memory contents
  if (ANDROID && may_exit) exit(0);

  return 0;
}
