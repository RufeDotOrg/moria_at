// Rufe.org LLC 2022-2024: ISC License
//
// #include platform.c twice for custom code:
// Custom code comes last, using game and platform details in depth.
//
// Platform and Game code coexist peacefully:
//  1) cc game.c
//  2) cc platform.c
//  3) cc $(cat game.c platform.c) OR cc $(cat platform.c game.c)
//
// Two workflows exist:
// 1) Iterative development (access all APIs until deps are known)
// 2) Release build (isolate platform DLL/SO + game code)
//
#define Log SDL_Log
#define rect_t SDL_Rect
#define point_t SDL_Point

#ifndef PLATFORM
#define PLATFORM
#include <setjmp.h>
#include <time.h>

// Skip cpuinfo inclusion
#define SDL_cpuinfo_h_

#include "SDL.h"

enum { COSMO = 0 };
#ifdef __FATCOSMOCC__
#include "cosmo_sdl.c"
#else
STATIC int
IsWindows()
{
  return 0;
}
#endif

#define ORGNAME "org.rufe"
#define APPNAME "moria.app"

enum { SDL_EVLOG = 0 };
enum { SDL_VERBOSE = 0 };
enum { BATCHING = 1 };
enum { REORIENTATION = 1 };
#define ORIENTATION_LIST \
  "Portrait LandscapeRight PortraitUpsideDown LandscapeLeft"
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

#if __APPLE__
#undef snprintf
#endif

enum { KEYBOARD = 0 };
#if defined(ANDROID) || (TARGET_OS_IPHONE)
enum { TOUCH = 1 };
enum { MOUSE = 0 };
enum { JOYSTICK = 0 };
enum { PC = 0 };
#else
enum { TOUCH = 0 };
enum { MOUSE = TOUCH };
enum { JOYSTICK = 1 };
enum { PC = 1 };
#include "keyboard.c"
#endif
enum { MOTION = 0 };  //(MOUSE || TOUCH) };
enum { DPAD = (TOUCH || JOYSTICK) };
enum { GFX = 1 };
enum { SCREENSHOT = 0 };

// render.c
DATA struct SDL_Window* windowD;
DATA rect_t display_rectD;
DATA rect_t safe_rectD;
DATA struct SDL_Renderer* rendererD;
DATA uint32_t texture_formatD;

// Claimed vs. actualized refresh rate
DATA int refresh_rateD;
DATA int vsync_rateD;

DATA SDL_Texture* portraitD;
DATA SDL_Texture* landscapeD;
DATA SDL_Texture* layoutD;
DATA SDL_Rect layout_rectD;
DATA SDL_FRect view_rectD;

DATA float retina_scaleD;
DATA int quitD;

int
check_gl()
{
  const char* hint = SDL_GetHint(SDL_HINT_RENDER_DRIVER);
  Log("SDL_RENDER_DRIVER hint: %s", hint);
  if (!hint) return 0;

  char gl[] = {'o', 'p', 'e', 'n'};
  for (int it = 0; it < AL(gl); ++it) {
    if (hint[it] != gl[it]) return 0;
  }
  return 1;
}

int
sdl_window_event(event)
SDL_Event event;
{
  if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
      event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
    int drw = display_rectD.w;
    int drh = display_rectD.h;
    int dw = event.window.data1;
    int dh = event.window.data2;

    if (TARGET_OS_IPHONE) {
      SDL_GetWindowSafeRect(windowD, &safe_rectD);
      safe_rectD.x *= retina_scaleD;
      safe_rectD.y *= retina_scaleD;
      safe_rectD.w *= retina_scaleD;
      safe_rectD.h *= retina_scaleD;
      dw *= retina_scaleD;
      dh *= retina_scaleD;
    } else {
      safe_rectD = (rect_t){0, 0, dw, dh};
    }

    if (dw != drw || dh != drh) {
      Log("display_resize %dx%d", dw, dh);
      display_rectD.w = dw;
      display_rectD.h = dh;

      // TBD: Review game utilization of viewport
      // Disabled the push event in SDL that occurs on another thread
      if (!PC) SDL_RenderSetViewport(rendererD, &display_rectD);

      // auto-detect orientation
      platformD.orientation(0);

      // orientation may be set before renderer creation
      // android 11 devices don't render the first frame (e.g. samsung A20)
      if (ANDROID && rendererD) SDL_RenderPresent(rendererD);

      return CTRL('d');
    }
  } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
    if (display_rectD.w != 0) {
      // android 11 devices don't render the first frame (e.g. samsung A20)
      if (ANDROID && rendererD) SDL_RenderPresent(rendererD);

      return CTRL('d');
    }
  } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
    if (ANDROID || __APPLE__) platformD.postgame(0);
  }
  return 0;
}

int
render_init()
{
  int winflag = WINDOW ? SDL_WINDOW_BORDERLESS : SDL_WINDOW_FULLSCREEN;
  if (check_gl()) {
    winflag |= SDL_WINDOW_OPENGL;
  }
  if (__APPLE__) winflag |= SDL_WINDOW_ALLOW_HIGHDPI;
  if (REORIENTATION) winflag |= SDL_WINDOW_RESIZABLE;
  windowD = SDL_CreateWindow("", 0, 0, WINDOW_X, WINDOW_Y, winflag);
  if (!windowD) return 0;

  if (!RELEASE) {
    uint32_t format = SDL_GetWindowPixelFormat(windowD);
    Log("Window pixel format (%d) %s", format, SDL_GetPixelFormatName(format));
  }

  int use_display = SDL_GetWindowDisplayIndex(windowD);
  int num_display = SDL_GetNumVideoDisplays();
  rect_t bounds_rect = {0};
  for (int it = 0; it < num_display; ++it) {
    rect_t r;
    SDL_GetDisplayBounds(it, &r);
    Log("%d Display) %d %d %d %d", it, r.x, r.y, r.w, r.h);
    if (it == use_display) {
      bounds_rect = r;
      SDL_DisplayMode mode;
      SDL_GetCurrentDisplayMode(it, &mode);
      Log(" -> Refresh Rate %d", mode.refresh_rate);
      refresh_rateD = mode.refresh_rate;
    }
  }

  int num_driver = SDL_GetNumRenderDrivers();
  Log("%d NumRenderDrivers", num_driver);

  SDL_RendererInfo rinfo;
  for (int it = 0; it < num_driver; ++it) {
    if (SDL_GetRenderDriverInfo(it, &rinfo) == 0) {
      Log("%d) SDL RendererInfo: "
          "rinfo.name %s "
          "rinfo.flags 0x%08x ",
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

  Log("SDL RendererInfo: "
      "rinfo.name %s "
      "rinfo.flags 0x%08x "
      "max texture %d %d ",
      rinfo.name, rinfo.flags, rinfo.max_texture_width,
      rinfo.max_texture_height);
  Log("vsync %d", (rinfo.flags & SDL_RENDERER_PRESENTVSYNC) != 0);

  {
    int rw, rh;
    if (SDL_GetRendererOutputSize(rendererD, &rw, &rh) != 0) return 1;
    Log("Renderer output size %d %d", rw, rh);

    int ww, wh;
    SDL_GetWindowSize(windowD, &ww, &wh);
    retina_scaleD = MAX((float)rw / ww, (float)rh / wh);
  }

  if (!texture_formatD) {
    texture_formatD = rinfo.texture_formats[0];
    for (int it = 0; it < rinfo.num_texture_formats; ++it) {
      if (rinfo.texture_formats[it] == SDL_PIXELFORMAT_ABGR8888) {
        texture_formatD = SDL_PIXELFORMAT_ABGR8888;
      }
    }
    Log("Texture pixel format 0x%x", texture_formatD);
    if (!RELEASE) {
      Log("Texture pixel format %s", SDL_GetPixelFormatName(texture_formatD));
    }
  }

  portraitD =
      SDL_CreateTexture(rendererD, texture_formatD, SDL_TEXTUREACCESS_TARGET,
                        PORTRAIT_X, PORTRAIT_Y);
  landscapeD =
      SDL_CreateTexture(rendererD, texture_formatD, SDL_TEXTUREACCESS_TARGET,
                        LANDSCAPE_X, LANDSCAPE_Y);

  if (PC && bounds_rect.w > 0 && bounds_rect.h > 0) {
    SDL_Event event;
    event.window.event = SDL_WINDOWEVENT_RESIZED;
    event.window.data1 = bounds_rect.w;
    event.window.data2 = bounds_rect.h;
    sdl_window_event(event);
  }

  return 1;
}
STATIC int
platform_screenshot()
{
  int olist[] = {SDL_ORIENTATION_LANDSCAPE, SDL_ORIENTATION_PORTRAIT};
  char* fname[] = {"landscape.nmg", "portrait.nmg"};
  USE(renderer);

  for (int it = 0; it < AL(olist); ++it) {
    platformD.orientation(olist[it]);
    platformD.draw();
    SDL_SetRenderTarget(renderer, layoutD);
    int sz = layout_rectD.w * layout_rectD.h * 1.5f;
    void* pixels = SDL_malloc(sz);
    int pitch = layout_rectD.w;

    memset(pixels, 0, sz);
    Log("layoutD %dx%d", layout_rectD.w, layout_rectD.h);
    Log("sz %d pixels %p pitch %d", sz, pixels, pitch);
    if (pixels) {
      SDL_RenderReadPixels(renderer, 0, SDL_PIXELFORMAT_NV12, pixels, pitch);
      struct padS header = {"nmg", layout_rectD.w, layout_rectD.h};
      SDL_RWops* f = SDL_RWFromFile(fname[it], "wb");
      if (f) {
        SDL_RWwrite(f, &header, sizeof(header), 1);
        SDL_RWwrite(f, pixels, sz, 1);
        SDL_RWclose(f);
      }
      SDL_free(pixels);
    }
  }

  platformD.orientation(0);
}
int
platform_draw()
{
  USE(renderer);
  USE(layout);

  if (layout) {
    SDL_SetRenderTarget(renderer, 0);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    USE(display_rect);
    USE(view_rect);
    rect_t target = {
        view_rect.x * display_rect.w,
        view_rect.y * display_rect.h,
        view_rect.w * display_rect.w,
        view_rect.h * display_rect.h,
    };
    SDL_RenderCopy(renderer, layout, NULL, &target);
  }

  SDL_RenderPresent(renderer);
  return 0;
}

STATIC int
orientation_default()
{
  if (LANDSCAPE) return SDL_ORIENTATION_LANDSCAPE;

  if (PORTRAIT) return SDL_ORIENTATION_PORTRAIT;

  int dw = display_rectD.w;
  int dh = display_rectD.h;
  return dw > dh ? SDL_ORIENTATION_LANDSCAPE : SDL_ORIENTATION_PORTRAIT;
}

STATIC int
platform_orientation(orientation)
{
  USE(display_rect);
  if (!orientation) orientation = orientation_default();

  if (REORIENTATION)
    SDL_SetWindowResizable(windowD, globalD.orientation_lock == 0);

  USE(safe_rect);
  float scale = 1.f;
  void* layout = 0;
  SDL_Rect layout_rect = display_rectD;
  if (orientation == SDL_ORIENTATION_LANDSCAPE) {
    layout = landscapeD;
    layout_rect = (rect_t){0, 0, LANDSCAPE_X, LANDSCAPE_Y};

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
    layout = portraitD;
    layout_rect = (rect_t){0, 0, PORTRAIT_X, PORTRAIT_Y};

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
  }
  layoutD = layout;
  layout_rectD = layout_rect;

  if (layout && scale != 1.f)
    SDL_SetTextureScaleMode(layout, SDL_ScaleModeLinear);
  if (layout && scale != 1.f) Log("layout using ScaleModeLinear");

  // Note tension: center of display vs. center of safe area
  //   affects visual aesthetic
  //   affects input positioning for touch
  rect_t ar_rect = {0, 0, layout_rect.w * scale, layout_rect.h * scale};
  ar_rect.x = (display_rect.w - ar_rect.w) / 2;
  ar_rect.y = MAX(safe_rect.y, (display_rect.h - ar_rect.h) / 2);

  float xuse = (float)ar_rect.w / display_rect.w;
  float yuse = (float)ar_rect.h / display_rect.h;
  float xpad = (float)ar_rect.x / display_rect.w;
  float ypad = (float)ar_rect.y / display_rect.h;

  Log("orientation %d scale %f use_layout %d: %.03f %.03f xuse yuse %.03f "
      "%.03f "
      "xpad ypad",
      orientation, scale, layoutD != 0, xuse, yuse, xpad, ypad);
  SDL_FRect view = {xpad, ypad, xuse, yuse};
  view_rectD = view;

  return orientation;
}

// 0 on success
STATIC int
platform_vsync(vsync)
{
  int ret = SDL_RenderSetVSync(rendererD, vsync);
  uint64_t elapsed[8];
  uint64_t begin = SDL_GetTicks64();
  for (int it = 0; it < 8; ++it) {
    platform_draw();
    elapsed[it] = SDL_GetTicks64() - begin;
  }
  // for (int it = 0; it < 8; ++it) {
  //   printf("%ju elapsed ms\n", elapsed[it]);
  // }
  vsync_rateD = 8 * 1000.0 / CLAMP(elapsed[8 - 1], 8 * 1, 8 * 1000);
  // Log("real refresh rate: %d | stated refresh rate %d", vsync_rateD,
  //     refresh_rateD);
  return ret;
}

int
sdl_pump()
{
  SDL_Event event;
  int ret;

  ret = 0;
  while (ret == 0 && SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_WINDOWEVENT:
        ret = sdl_window_event(event);
        break;
      case SDL_QUIT:
        quitD = 1;
        break;
      case SDL_FINGERDOWN:
      case SDL_FINGERUP:
        if (MOUSE || TOUCH) ret = sdl_touch_event(event);
        break;
      case SDL_KEYDOWN:
        // case SDL_KEYUP: // (optional)
        if (KEYBOARD) ret = sdl_keyboard_event(event);
        if (SCREENSHOT && (event.key.keysym.mod & KMOD_CTRL) > 0 &&
            (event.key.keysym.mod & KMOD_ALT) > 0 &&
            event.key.keysym.sym == 's')
          platform_screenshot();
        break;
      case SDL_MOUSEMOTION:
      case SDL_FINGERMOTION:
        if (MOTION) ret = sdl_motion(event);
        break;
      case SDL_JOYAXISMOTION:
        if (JOYSTICK) ret = sdl_axis_motion(event);
        break;
      case SDL_JOYBUTTONDOWN:
      case SDL_JOYBUTTONUP:  // (optional)
        if (JOYSTICK) ret = sdl_joystick_event(event);
        break;
      case SDL_JOYDEVICEADDED:
      case SDL_JOYDEVICEREMOVED:
        if (JOYSTICK) sdl_joystick_device(event);
        break;
    }
  }

  if (ret == 0) {
    nanosleep(&(struct timespec){0, 8e6}, 0);
    if (PC) ret = CTRL('d');
  }

  return ret;
}

int
platform_input()
{
  int c = sdl_pump();
  if (quitD) return CTRL('c');
  return c;
}

STATIC int
rate_of_refresh()
{
  USE(refresh_rate);
  if (!refresh_rate) refresh_rate = 60;
  return refresh_rate;
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

STATIC void
log_hint(char* name)
{
  const char* val = SDL_GetHint(name);
  if (val) Log("Hint: %s=%s", name, val);
}

// Initialization
int
platform_pregame()
{
  int scope = (SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);

  if (!SDL_WasInit(scope)) {
    if (!RELEASE) Log("Initializing development build");
    if (SDL_VERBOSE) SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

    // SDL config
    if (SDL_EVLOG) SDL_SetHint(SDL_HINT_EVENT_LOGGING, "1");
    if (BATCHING) SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
    if (!SDL_GetHint(SDL_HINT_RENDER_VSYNC)) {
      SDL_SetHint(SDL_HINT_RENDER_VSYNC, globalD.vsync ? "1" : "0");
    }
    if (QUALITY) SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    if (PC) {
      if (!SDL_GetHint(SDL_HINT_RENDER_DRIVER)) {
        if (globalD.pc_renderer[0])
          SDL_SetHint(SDL_HINT_RENDER_DRIVER, globalD.pc_renderer);
        else
          SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
      }
      if (JOYSTICK) {
        log_hint(SDL_HINT_GAMECONTROLLERCONFIG);
        log_hint(SDL_HINT_GAMECONTROLLERCONFIG_FILE);
        log_hint(SDL_HINT_GAMECONTROLLERTYPE);
        log_hint(SDL_HINT_GAMECONTROLLER_IGNORE_DEVICES);
        log_hint(SDL_HINT_GAMECONTROLLER_IGNORE_DEVICES_EXCEPT);
        log_hint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS);
      }
    } else {
      SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
      // SDL_SetHint(SDL_HINT_RENDER_METAL_PREFER_LOW_POWER_DEVICE, "1");
    }

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

    SDL_Init(scope);

    SDL_version sv;
    SDL_GetVersion(&sv);
    Log("SDL version %d.%d.%d", V3b(&sv));
    if (!render_init()) return 1;
  }

  if (WINDOW) {
    SDL_Event event;
    event.window.event = SDL_WINDOWEVENT_RESIZED;
    event.window.data1 = WINDOW_X;
    event.window.data2 = WINDOW_Y;
    sdl_window_event(event);
  }

  while (!PC && display_rectD.w == 0) {
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_WINDOWEVENT) {
        sdl_window_event(event);
      }
    }
  }

  return 0;
}
int
platform_postgame(may_exit)
{
  // Android return from main does not require the process to exit.
  // exit(...) ensures the process terminates.
  // otherwise main() should handle resume with previous memory contents
  if (ANDROID && may_exit) {
    SDL_Quit();
    exit(0);
  }

  return 0;
}

static int
platform_init()
{
  platformD.pregame = platform_pregame;
  platformD.postgame = platform_postgame;
  platformD.draw = platform_draw;
  platformD.input = platform_input;
  platformD.orientation = platform_orientation;
  platformD.vsync = platform_vsync;
  if (platformD.seed == noop) platformD.seed = platform_random;

  return 0;
}
#else  // PLATFORM
#include "custom.c"
static int
custom_init()
{
  platform_init();

  platformD.pregame = custom_pregame;
  platformD.postgame = custom_postgame;
  platformD.predraw = custom_predraw;
  platformD.draw = custom_draw;
  platformD.orientation = custom_orientation;
  return 0;
}
#define platform_init custom_init

#ifdef __FATCOSMOCC__
#include "cosmo_sdl.c"
#endif
#endif

#undef Log
#undef rect_t
#undef point_t
