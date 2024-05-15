#include "cosmo-abi.h"

#define SDL_AllocFormat abi_SDL_AllocFormat
#define SDL_CreateRenderer abi_SDL_CreateRenderer
#define SDL_CreateRGBSurfaceWithFormat abi_SDL_CreateRGBSurfaceWithFormat
#define SDL_CreateTexture abi_SDL_CreateTexture
#define SDL_DestroyTexture abi_SDL_DestroyTexture
#define SDL_CreateTextureFromSurface abi_SDL_CreateTextureFromSurface
#define SDL_CreateWindow abi_SDL_CreateWindow
#define SDL_FreeSurface abi_SDL_FreeSurface
#define SDL_GetCurrentDisplayMode abi_SDL_GetCurrentDisplayMode
#define SDL_GetDisplayBounds abi_SDL_GetDisplayBounds
#define SDL_GetModState abi_SDL_GetModState
#define SDL_GetNumRenderDrivers abi_SDL_GetNumRenderDrivers
#define SDL_GetNumVideoDisplays abi_SDL_GetNumVideoDisplays
#define SDL_GetRenderDriverInfo abi_SDL_GetRenderDriverInfo
#define SDL_GetRendererInfo abi_SDL_GetRendererInfo
#define SDL_GetRendererOutputSize abi_SDL_GetRendererOutputSize
#define SDL_GetWindowDisplayIndex abi_SDL_GetWindowDisplayIndex
#define SDL_GetWindowSize abi_SDL_GetWindowSize
#define SDL_Init abi_SDL_Init
#define SDL_LogSetAllPriority abi_SDL_LogSetAllPriority
#define SDL_LogSetOutputFunction abi_SDL_LogSetOutputFunction
#define SDL_MapRGBA abi_SDL_MapRGBA
#define SDL_PollEvent abi_SDL_PollEvent
#define SDL_RenderClear abi_SDL_RenderClear
#define SDL_RenderCopy abi_SDL_RenderCopy
#define SDL_RenderDrawRect abi_SDL_RenderDrawRect
#define SDL_RenderFillRect abi_SDL_RenderFillRect
#define SDL_RenderFlush abi_SDL_RenderFlush
#define SDL_RenderPresent abi_SDL_RenderPresent
#define SDL_RenderSetViewport abi_SDL_RenderSetViewport
#define SDL_RWclose abi_SDL_RWclose
#define SDL_RWFromFile abi_SDL_RWFromFile
#define SDL_RWread abi_SDL_RWread
#define SDL_RWseek abi_SDL_RWseek
#define SDL_RWwrite abi_SDL_RWwrite
#define SDL_SetHint abi_SDL_SetHint
#define SDL_GetHint abi_SDL_GetHint
#define SDL_SetRenderDrawBlendMode abi_SDL_SetRenderDrawBlendMode
#define SDL_SetRenderDrawColor abi_SDL_SetRenderDrawColor
#define SDL_SetRenderTarget abi_SDL_SetRenderTarget
#define SDL_SetTextureAlphaMod abi_SDL_SetTextureAlphaMod
#define SDL_SetTextureBlendMode abi_SDL_SetTextureBlendMode
#define SDL_SetTextureColorMod abi_SDL_SetTextureColorMod
#define SDL_SetWindowIcon abi_SDL_SetWindowIcon
#define SDL_SetWindowResizable abi_SDL_SetWindowResizable
#define SDL_ShowCursor abi_SDL_ShowCursor
#define SDL_UpdateTexture abi_SDL_UpdateTexture
#define SDL_WasInit abi_SDL_WasInit

#define SDL_GetError abi_SDL_GetError
#define SDL_GL_LoadLibrary abi_SDL_GL_LoadLibrary
#define SDL_GL_UnloadLibrary abi_SDL_GL_UnloadLibrary

typedef void (*elipsis)(SDL_PRINTF_FORMAT_STRING const char* a, ...);
typedef void __attribute__((__ms_abi__)) (*win_elipsis)(
    SDL_PRINTF_FORMAT_STRING const char* a, ...);
#define SDL_Log(x, ...)                                            \
  if (IsWindows())                                                 \
    ((win_elipsis)cosmo_dlsym(libD, "SDL_Log"))(x, ##__VA_ARGS__); \
  else                                                             \
    ((elipsis)cosmo_dlsym(libD, "SDL_Log"))(x, ##__VA_ARGS__);
#define SDL_SetError(x, ...)                                            \
  if (IsWindows())                                                      \
    ((win_elipsis)cosmo_dlsym(libD, "SDL_SetError"))(x, ##__VA_ARGS__); \
  else                                                                  \
    ((elipsis)cosmo_dlsym(libD, "SDL_SetError"))(x, ##__VA_ARGS__);
