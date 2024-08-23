
enum { JOYSTICK_VERBOSE = 0 };

// game controllers & joysticks
DATA SDL_Joystick* joystick_ptrD;
DATA float jxD;
DATA float jyD;

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
      c = CTRL('w');  // menu
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
    if (mode == 0) {
      switch (event.jbutton.button) {
        case 0:
        case 1:  // movement
          return joystick_button(!event.jbutton.button);
        case 2:
          return '.';
        case 3:
          return 'a';
        case 4:  // Lbumper
          return 'c';
        case 5:  // Rbumper
          return 'm';
        case 6:  // Ltrigger
          return 'd';
        case 7:  // Rtrigger
          return '!';
        case 11:  // Press Lstick
          return 'i';
        case 12:  // Press Rstick
          return 'e';
      }
    } else if (mode == 1) {
      switch (event.jbutton.button) {
        case 0:
        case 1:  // movement
          return overlay_dir(joystick_dir(), !event.jbutton.button);
        case 2:
        case 3:
        case 11:  // Press Lstick
        case 12:  // Press Rstick
          return ESCAPE;
      }
    } else if (mode == 2) {
      switch (event.jbutton.button) {
        case 1:
        case 2:
        case 4:  // Lbumper
        case 5:  // Rbumper
          return ESCAPE;
        case 0:
        case 3:
          return 'o';
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
