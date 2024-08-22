
// game controllers & joysticks
DATA SDL_Joystick* joystick_ptrD;
DATA float jxD;
DATA float jyD;

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
  if (JOYSTICK) {
    Log("axis %d value %d", event.jaxis.axis, event.jaxis.value);
    int ok = event.jaxis.value + 32768;
    float norm = (float)ok / (32767 * 2 + 1);
    Log("norm %.03f", norm);
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
  if (JOYSTICK) {
    char* statename[] = {"release", "press"};
    Log("button %d %s", event.jbutton.button, statename[event.jbutton.state]);
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
