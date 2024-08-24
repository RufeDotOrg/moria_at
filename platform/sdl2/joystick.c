
enum { JOYSTICK_VERBOSE = 0 };

// game controllers & joysticks
DATA SDL_Joystick* joystick_ptrD;
DATA float jxD;
DATA float jyD;

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
// Xbox remap
enum {
  XB_LTOUCHPAD,
  XB_RTOUCHPAD,
  XB_ELLIPSIS,
  XB_SOUTH,
  XB_EAST,
  XB_NORTH,
  XB_WEST,
  XB_LBUMPER,
  XB_RBUMPER,
  XB_LTRIGGER,
  XB_RTRIGGER,
  XB_LTINY,
  XB_RTINY,
  XB_SYSTEM,
  XB_COUNT,
};
DATA char xboxD[] = {
    JS_LSTICK,  // Ltouchpad
    JS_RSTICK,  // Rtouchpad
    -1,         // Ellipsis
    JS_SOUTH,    JS_EAST,     JS_NORTH, JS_WEST,  JS_LBUMPER, JS_RBUMPER,
    JS_LTRIGGER, JS_RTRIGGER, JS_LTINY, JS_RTINY, JS_SYSTEM,
};
// TBD: Dynamic
DATA char* button_mapD; // = xboxD;

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
  if (jsidx >= 0) joystick_ptrD = SDL_JoystickOpen(jsidx);
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
// Enough to walk, independent of touch.c
// TBD: possibly want selection behaviors in touch.c
STATIC int
joystick_button(button)
{
  char c = key_dir(joystick_dir());
  if (button) {
    if (c == ' ')
      c = 'a';
    else
      c &= ~0x20;  // run
  }
  Log("joystick button %c (%d)", c, c);
  return c;
}

STATIC int
joystick_init()
{
  jxD = jyD = .5;

  int count = 0;
  for (int it = 0; it < SDL_NumJoysticks(); ++it) {
    const char* name;
    const char* path;

    count++;
    name = SDL_JoystickNameForIndex(it);
    // !path may be a unique identifier!
    path = SDL_JoystickPathForIndex(it);
    Log("%s %s", name, path);
    joystick_assign(it);
  }
  Log("joystick count %d", count);
}

int
sdl_axis_motion(SDL_Event event)
{
  if (JOYSTICK_VERBOSE)
    Log("axis %d value %d", event.jaxis.axis, event.jaxis.value);

  if (JOYSTICK) {
    int ok = event.jaxis.value + 32768;
    float norm = (float)ok / (32767 * 2 + 1);
    if (JOYSTICK_VERBOSE) Log("norm %.03f", norm);
    if (event.jaxis.axis == 0) {
      jxD = norm;
    }
    if (event.jaxis.axis == 1) {
      jyD = norm;
    }
  }
  return 0;
}

int
overlay_dir(dir, finger)
{
  int dx = dir_x(dir);
  int dy = dir_y(dir);

  // Controller uses center tap as confirm? (or change buttons.. ?)
  if (!dx && !dy) {
    char c = (finger ? 'A' : 'a') + finger_rowD;
    Log("overlay trigger: char %c (%d)", c, c);
    return c;
  }

  if (dx && !dy) {
    if (finger)
      finger_rowD = dx < 0 ? overlay_begin() : overlay_end();
    else
      return column_transition(dx);
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
sdl_joystick_event(SDL_Event event)
{
  USE(mode);
  if (1) {
    char* statename[] = {"release", "press"};
    Log("button %d %s mode %d", event.jbutton.button,
        statename[event.jbutton.state], mode);
  }

  if (JOYSTICK) {
    int button = event.jbutton.button;

    if (button_mapD) {
      button = -1;
      if (button < JS_COUNT) button = button_mapD[button];
    }

    if (mode == 0) {
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
          return 'd';
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
    } else if (mode == 1) {
      switch (button) {
        case JS_SOUTH:
        case JS_EAST:  // movement
          return overlay_dir(joystick_dir(), button == JS_SOUTH);
        case JS_WEST:
        case JS_LSTICK:  // Press Lstick
        case JS_RSTICK:  // Press Rstick
          return ESCAPE;
        case JS_NORTH:
          return '-';  // sort shop/inven
      }
    } else if (mode == 2) {
      switch (button) {
        case JS_SOUTH:
        case JS_EAST:
        case JS_WEST:
        case JS_LBUMPER:
        case JS_RBUMPER:
        case JS_RTINY:
          return ESCAPE;
        case JS_NORTH:
          return 'o';  // from death screen, go back to last game frame
      }
    }
  }
  return 0;
}

int
sdl_joystick_device(SDL_Event event)
{
  if (JOYSTICK) {
    int type = event.type;
    if (event.type == SDL_JOYDEVICEADDED && !joystick_ptrD) {
      joystick_assign(event.jdevice.which);
    }
    if (event.type == SDL_JOYDEVICEREMOVED && joystick_ptrD) {
      SDL_JoystickClose(joystick_ptrD);
      joystick_ptrD = 0;
    }
  }
}
