// Rufe.org LLC 2022-2024: ISC License

enum { JOYSTICK_VERBOSE = 0 };

// game controllers & joysticks
DATA SDL_Joystick* joystick_ptrD;
DATA float jxD;
DATA float jyD;
DATA int joystick_productD;
DATA int triggerD;
#define STEAM_VIRTUAL_GAMEPAD 0x11ff
#define STEAM_DECK_RAW 0x1205

// Sony Dualsense button order is default
enum {
  JS_SOUTH,
  JS_EAST,
  JS_NORTH,
  JS_WEST,
  JS_LBUMPER,
  JS_RBUMPER,
  JS_LTRIGGER,
  JS_RTRIGGER,
  JS_LTINY,
  JS_RTINY,
  JS_SYSTEM,
  JS_LSTICK,
  JS_RSTICK,
  JS_COUNT,
};
DATA char rawdeckD[] = {
    JS_LSTICK,  // Ltouchpad
    JS_RSTICK,  // Rtouchpad
    -1,         // Ellipsis
    JS_SOUTH,    JS_EAST,     JS_WEST,  JS_NORTH, JS_LBUMPER, JS_RBUMPER,
    JS_LTRIGGER, JS_RTRIGGER, JS_LTINY, JS_RTINY, JS_SYSTEM,
};
DATA char steam_virtualD[] = {
    JS_SOUTH, JS_EAST,  JS_WEST, JS_NORTH,  JS_LBUMPER, JS_RBUMPER,
    JS_LTINY, JS_RTINY, -1,      JS_LSTICK, JS_RSTICK,
};

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
    // TBD: hack for better play experience on big screens
    if (globalD.zoom_factor == 0) globalD.zoom_factor = 1;
    // Center input
    jxD = jyD = .5;
  }
  joystick_productD = product;
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
  if (JOYSTICK_VERBOSE)
    Log("axis %d value %d", event.jaxis.axis, event.jaxis.value);

  if (JOYSTICK) {
    int axis = event.jaxis.axis;

    int ok = event.jaxis.value + 32768;
    float norm = (float)ok / (32767 * 2 + 1);
    if (JOYSTICK_VERBOSE) Log("norm %.03f", norm);
    if (axis == 0) {
      jxD = norm;
    }
    if (axis == 1) {
      jyD = norm;
    }
    if (axis == 2 && joystick_productD == STEAM_VIRTUAL_GAMEPAD) {
      // Log("ltrigger %d", event.jaxis.value);
      int trigger = (event.jaxis.value > 10000);
      if (trigger && !triggerD) ret = '#';
      triggerD = trigger;
    }
    if (axis == 5 && joystick_productD == STEAM_VIRTUAL_GAMEPAD) {
      // Log("rtrigger %d", event.jaxis.value);
      int trigger = (event.jaxis.value > 10000);
      if (trigger && !triggerD) ret = '!';
      triggerD = trigger;
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
      return '.';
    case JS_WEST:
      return CTRL('w');  // show advanced menu
    case JS_LBUMPER:
      return 'c';
    case JS_RBUMPER:
      return 'm';
    case JS_LTRIGGER:
      return '#';
    case JS_RTRIGGER:
      return '!';
    case JS_LTINY:
      return CTRL('z');
    case JS_RTINY:
      return 'p';
    case JS_SYSTEM:
      return '-';
    case JS_LSTICK:
      return 'i';
    case JS_RSTICK:
      return 'e';
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
    case JS_LSTICK:
    case JS_RSTICK:
      return ESCAPE;
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
    case JS_LBUMPER:
    case JS_RBUMPER:
    case JS_RTINY:
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

    switch (joystick_productD) {
      case STEAM_VIRTUAL_GAMEPAD:
        if (button < AL(steam_virtualD))
          button = steam_virtualD[button];
        else
          button = -1;
        break;
      case STEAM_DECK_RAW:
        if (button < AL(rawdeckD))
          button = rawdeckD[button];
        else
          button = -1;
        break;
    }

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
