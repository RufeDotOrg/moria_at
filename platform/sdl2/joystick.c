// Rufe.org LLC 2022-2025: ISC License

enum { JOYSTICK_VERBOSE = 0 };
enum { BIND_VERBOSE = 0 };

// game controllers & joysticks
DATA SDL_Joystick* joystick_ptrD;
DATA float jxD;
DATA float jyD;
DATA int triggerD;
DATA int joystick_refcountD;

enum { CHAR_LTRIGGER = '0' };
enum { CHAR_RTRIGGER = '-' };  // gameplay: zoom, menu: sort
enum { MAX_MAPPING = 64 };
enum { MAX_AXIS = 16 };

enum {
  JS_SOUTH,
  JS_EAST,
  JS_WEST,
  JS_NORTH,
  JS_LSHOULDER,
  JS_RSHOULDER,
  JS_LTRIGGER,  // Disabled if AXIS trigger support exists
  JS_RTRIGGER,  // Disabled if AXIS trigger support exists
  JS_BACK,
  JS_START,
  JS_COUNT,
};
DATA char mappingD[MAX_MAPPING];

enum {
  JA_LX,
  JA_LY,
  JA_LTRIGGER,
  JA_RX,
  JA_RY,
  JA_RTRIGGER,
  JA_COUNT,
};
DATA char ja_mappingD[MAX_AXIS];

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

#define BUTTON(text, id)                                                   \
  {                                                                        \
    char* fa = strstr(mapping, "," text ":b");                             \
    if (BIND_VERBOSE) Log("%s", fa);                                       \
    if (fa) {                                                              \
      fa += AL(text) + 2;                                                  \
      int kv = parse_num(fa);                                              \
      if (BIND_VERBOSE) Log(text " is button %d (%c) -> %d", kv, *fa, id); \
      if (kv < AL(mappingD)) mappingD[kv] = id;                            \
    }                                                                      \
  }
#define AXIS(text, id)                                                     \
  {                                                                        \
    char* fa = strstr(mapping, "," text ":a");                             \
    if (BIND_VERBOSE) Log("%s", fa);                                       \
    if (fa) {                                                              \
      fa += AL(text) + 2;                                                  \
      int kv = parse_num(fa);                                              \
      if (BIND_VERBOSE) Log(text " is button %d (%c) -> %d", kv, *fa, id); \
      if (kv < AL(ja_mappingD)) ja_mappingD[kv] = id;                      \
    }                                                                      \
  }
STATIC int
find_axis(ja)
{
  for (int it = 0; it < AL(ja_mappingD); ++it)
    if (ja_mappingD[it] == ja) return 1;
  return 0;
}
STATIC int
joystick_assign(jsidx)
{
  if (joystick_ptrD) SDL_JoystickClose(joystick_ptrD);

  memset(mappingD, -1, sizeof(mappingD));
  memset(ja_mappingD, -1, sizeof(ja_mappingD));

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
      BUTTON("x", JS_WEST);
      BUTTON("y", JS_NORTH);
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

      if (!find_axis(JA_LTRIGGER)) BUTTON("lefttrigger", JS_LTRIGGER);
      if (!find_axis(JA_RTRIGGER)) BUTTON("righttrigger", JS_RTRIGGER);

      SDL_free(mapping);
    }
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
joystick_button(button)
{
  char c = key_dir(joystick_dir());
  if (c == ' ')
    c = (button == JS_EAST) ? 'a' : '.';
  else if (button == JS_EAST)
    c &= ~0x20;  // run
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
        if (trigger && !triggerD) ret = CHAR_LTRIGGER;
        triggerD = trigger;
        break;
      case JA_RX:
      case JA_RY:
        break;
      case JA_RTRIGGER:
        if (trigger && !triggerD) ret = CHAR_RTRIGGER;
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
      return joystick_button(button);
    case JS_WEST:
      return 'p';
    case JS_NORTH:
      return '!';
    case JS_LSHOULDER:
      return 'c';
    case JS_RSHOULDER:
      return 'm';
    case JS_LTRIGGER:
      return CHAR_LTRIGGER;
    case JS_RTRIGGER:
      return CHAR_RTRIGGER;
    case JS_BACK:
      return CTRL('z');
    case JS_START:
      return CTRL('w');  // show advanced menu
    default:
      return 0;
  }
}
int
joystick_menu_button(button)
{
  switch (button) {
    case JS_SOUTH:
    case JS_EAST:  // movement
      return overlay_dir(joystick_dir(), button == JS_EAST);
    case JS_NORTH:
    case JS_WEST:
      return ESCAPE;
    default:
      return 0;
  }
}
int
joystick_popup_button(button)
{
  switch (button) {
    case JS_SOUTH:
      return ESCAPE;
    case JS_WEST:
      return 'p';
    case JS_LSHOULDER:
      return 'c';
    case JS_EAST:
      return 'o';  // from death screen, go back to last game frame; reroll
    case JS_BACK:
      return CTRL('z');
    default:
      return 0;
  }
}
int
sdl_joystick_event(SDL_Event event)
{
  USE(mode);
  int button = event.jbutton.button;
  int state = event.jbutton.state;

  if (JOYSTICK_VERBOSE) {
    char* statename[] = {"release", "press"};
    Log("button %d %s mode %d", button, statename[state], mode);
  }

  int ret = 0;
  if (JOYSTICK) {
    if (state) {
      if (button >= 0 && button < AL(mappingD))
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
    } else {
      if (blipD) ret = ' ';
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

STATIC int
joystick_init()
{
  MUSE(global, label_button_order);

  SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS,
              label_button_order ? "1" : "0");

  int ok = (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == 0);
  joystick_refcountD += ok;

  int using_selection = (joystick_count() > 0);
  platformD.selection = using_selection ? fnptr(touch_selection) : noop;
  return ok;
}

STATIC int
joystick_update()
{
  while (joystick_refcountD--) SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
  if (globalD.use_joystick) joystick_init();
}
