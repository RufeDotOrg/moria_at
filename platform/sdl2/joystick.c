// Rufe.org LLC 2022-2024: ISC License

enum { JOYSTICK_VERBOSE = 0 };

// game controllers & joysticks
DATA SDL_Joystick* joystick_ptrD;
DATA float jxD;
DATA float jyD;
DATA int triggerD;

enum {
  JS_SOUTH,
  JS_EAST,
  JS_NORTH,
  JS_WEST,
  JS_LSHOULDER,
  JS_RSHOULDER,
  JS_LTRIGGER,  // Disabled if AXIS trigger support exists
  JS_RTRIGGER,  // Disabled if AXIS trigger support exists
  JS_BACK,
  JS_START,
  JS_COUNT,
};
DATA char mappingD[JS_COUNT];

enum {
  JA_LX,
  JA_LY,
  JA_LTRIGGER,
  JA_RX,
  JA_RY,
  JA_RTRIGGER,
  JA_COUNT,
};
DATA char ja_mappingD[JA_COUNT];

STATIC int
joystick_enabled()
{
  return joystick_ptrD != 0;
}
// TBD: deadzone check?
STATIC int
joystick_2f(float* x, float* y)
{
  *x = jxD;
  *y = jyD;
  return 0;
}
STATIC int
joystick_count()
{
  return SDL_NumJoysticks();
}

enum { BIND_VERBOSE = 0 };
#define BUTTON(text, id)                                                   \
  {                                                                        \
    char* fa = strstr(mapping, "," text ":");                              \
    if (BIND_VERBOSE) Log("%s", fa);                                       \
    if (fa) {                                                              \
      fa += AL(text) + 2;                                                  \
      int kv = *fa - '0';                                                  \
      if (BIND_VERBOSE) Log(text " is button %d (%c) -> %d", kv, *fa, id); \
      if (kv < AL(mappingD))                                               \
        mappingD[kv] = id;                                                 \
      else                                                                 \
        mappingD[kv] = -1;                                                 \
    }                                                                      \
  }
#define AXIS(text, id)                                                     \
  {                                                                        \
    char* fa = strstr(mapping, "," text ":");                              \
    if (BIND_VERBOSE) Log("%s", fa);                                       \
    if (fa) {                                                              \
      fa += AL(text) + 2;                                                  \
      int kv = *fa - '0';                                                  \
      if (BIND_VERBOSE) Log(text " is button %d (%c) -> %d", kv, *fa, id); \
      if (kv < AL(ja_mappingD))                                            \
        ja_mappingD[kv] = id;                                              \
      else                                                                 \
        ja_mappingD[kv] = -1;                                              \
    }                                                                      \
  }
STATIC int
joystick_assign(jsidx)
{
  if (joystick_ptrD) SDL_JoystickClose(joystick_ptrD);

  void* joystick = 0;
  if (jsidx >= 0) joystick = SDL_JoystickOpen(jsidx);
  joystick_ptrD = joystick;

  int product = 0;
  if (joystick) {
    const char* name = SDL_JoystickNameForIndex(jsidx);
    product = SDL_JoystickGetDeviceProduct(jsidx);
    Log("joystick_assign (product 0x%x): %s", product, name);
    SDL_JoystickGUID guid = SDL_JoystickGetGUID(joystick);
    char* mapping = SDL_GameControllerMappingForGUID(guid);
    if (mapping) {
      Log("GUID mapping: %s", mapping);
      BUTTON("a", JS_SOUTH);
      BUTTON("b", JS_EAST);
      BUTTON("x", JS_NORTH);
      BUTTON("y", JS_WEST);
      BUTTON("lefttrigger", JS_LTRIGGER);
      BUTTON("righttrigger", JS_RTRIGGER);
      BUTTON("leftshoulder", JS_LSHOULDER);
      BUTTON("rightshoulder", JS_RSHOULDER);
      BUTTON("back", JS_BACK);
      BUTTON("start", JS_START);

      AXIS("leftx", JA_LX);
      AXIS("lefty", JA_LY);
      AXIS("lefttrigger", JA_LTRIGGER);
      AXIS("rightx", JA_RX);
      AXIS("righty", JA_RY);
      AXIS("righttrigger", JA_RTRIGGER);

      // Disable trigger as a button (prefer axis motion)
      if (ja_mappingD[JA_LTRIGGER] >= 0) mappingD[JS_LTRIGGER] = -1;
      if (ja_mappingD[JA_RTRIGGER] >= 0) mappingD[JS_RTRIGGER] = -1;

      SDL_free(mapping);
    }
    // TBD: hack for better play experience on big screens
    if (globalD.zoom_factor == 0) globalD.zoom_factor = 1;
    // Center input
    jxD = jyD = .5;
  }
}
STATIC int
joystick_dir()
{
  int scale = 64;
  int x = jxD * (PADSIZE - scale) + scale / 2;
  int y = jyD * (PADSIZE - scale) + scale / 2;

  int n = dpad_nearest_pp(y, x, 0);
  return (pp_keyD[n]);
}
STATIC int
joystick_button(alt)
{
  char c = key_dir(joystick_dir());
  if (alt) {
    if (c == ' ')
      c = 'a';
    else
      c &= ~0x20;  // run
  }
  return c;
}

int
sdl_axis_motion(SDL_Event event)
{
  int ret = 0;

  if (JOYSTICK) {
    int axis = event.jaxis.axis;

    if (JOYSTICK_VERBOSE)
      Log("axis raw %d value %d", event.jaxis.axis, event.jaxis.value);

    if (axis < AL(ja_mappingD))
      axis = ja_mappingD[axis];
    else
      axis = -1;

    int ok = event.jaxis.value + 32768;
    float norm = (float)ok / (32767 * 2 + 1);
    if (JOYSTICK_VERBOSE) Log("norm %.03f", norm);

    int trigger = (event.jaxis.value > 10000);
    switch (axis) {
      case JA_LX:
        jxD = norm;
        break;
      case JA_LY:
        jyD = norm;
        break;
      case JA_LTRIGGER:
        if (trigger && !triggerD)
          ret = '!';  // TBD: document change to repeat actuate
        triggerD = trigger;
        break;
      case JA_RX:
      case JA_RY:
        break;
      case JA_RTRIGGER:
        if (trigger && !triggerD) ret = '-';  // TBD: document change to zoom
        triggerD = trigger;
        break;
    }
  }
  return ret;
}

int
overlay_dir(dir, finger)
{
  int dx = dir_x(dir);
  int dy = dir_y(dir);

  if (!dx && !dy) {
    // Controller uses center tap as study & confirm
    return (finger ? 'A' : 'a') + finger_rowD;
  }

  if (dx && !dy) {
    if (finger)
      finger_rowD = dx < 0 ? overlay_begin() : overlay_end();
    else
      return column_transition(finger_colD, dx);
  }
  if (dy && !dx) {
    if (finger)
      finger_rowD = overlay_bisect(dy);
    else
      finger_rowD = overlay_input(dy);
  }
  return CTRL('d');
}
int
joystick_game_button(button)
{
  switch (button) {
    case JS_SOUTH:
    case JS_EAST:  // movement
      return joystick_button(button == JS_SOUTH);
    case JS_NORTH:
      return CTRL('w');  // show advanced menu
    case JS_WEST:
      return '.';
    case JS_LSHOULDER:
      return 'c';
    case JS_RSHOULDER:
      return 'm';
    case JS_LTRIGGER:
      return '!';
    case JS_RTRIGGER:
      return '-';
    case JS_BACK:
      return CTRL('z');
    case JS_START:
      return 'p';
  }
}
int
joystick_menu_button(button)
{
  switch (button) {
    case JS_SOUTH:
    case JS_EAST:  // movement
      return overlay_dir(joystick_dir(), button == JS_SOUTH);
    case JS_WEST:
    case JS_NORTH:
      return '-';  // sort shop/inven
  }
}
int
joystick_popup_button(button)
{
  switch (button) {
    case JS_NORTH:
    case JS_EAST:
    case JS_WEST:
    case JS_LSHOULDER:
    case JS_RSHOULDER:
    case JS_START:
      return ESCAPE;
    case JS_SOUTH:
      return 'o';  // from death screen, go back to last game frame; reroll
  }
}
int
sdl_joystick_event(SDL_Event event)
{
  USE(mode);
  if (JOYSTICK_VERBOSE) {
    char* statename[] = {"release", "press"};
    Log("button %d %s mode %d", event.jbutton.button,
        statename[event.jbutton.state], mode);
  }

  int ret = 0;
  if (JOYSTICK) {
    uint32_t button = event.jbutton.button;
    if (JOYSTICK_VERBOSE) Log("button event raw: %d", button);

    if (button < AL(mappingD))
      button = mappingD[button];
    else
      button = -1;

    if (mode == 0) {
      ret = joystick_game_button(button);
      if (ret > ' ' && msg_moreD) ret = ' ';
    } else if (mode == 1) {
      ret = joystick_menu_button(button);
    } else if (mode == 2) {
      ret = joystick_popup_button(button);
    }
  }
  return ret;
}

int
sdl_joystick_device(SDL_Event event)
{
  if (JOYSTICK) {
    int type = event.type;
    if (event.type == SDL_JOYDEVICEADDED) {
      joystick_assign(event.jdevice.which);
    }
    if (event.type == SDL_JOYDEVICEREMOVED) {
      joystick_assign(SDL_NumJoysticks() - 1);
    }
  }
}
