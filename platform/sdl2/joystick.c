
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
// Enough to walk, independent of touch.c
// TBD: possibly want selection behaviors in touch.c
STATIC int
joystick_dir(button)
{
  int scale = 64;
  int x = jxD * (PADSIZE - scale) + scale / 2;
  int y = jyD * (PADSIZE - scale) + scale / 2;

  int n = dpad_nearest_pp(y, x, 0);
  char c = key_dir(pp_keyD[n]);
  if (button) {
    if (c == ' ')
      c = '@';  // menu
    else
      c &= ~0x20;  // run
  }
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
sdl_joystick_event(SDL_Event event)
{
  if (1) {
    char* statename[] = {"release", "press"};
    Log("button %d %s", event.jbutton.button, statename[event.jbutton.state]);
  }
  if (JOYSTICK) {
    switch (event.jbutton.button) {
      case 0:
      case 1:  // movement
        return joystick_dir(event.jbutton.button);
      case 2:
        return '.';
      case 3:
        return 'a';
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
