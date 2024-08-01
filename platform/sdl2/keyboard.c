DATA char* scancode_mapD;

int
keyboard_map(scancode)
char* scancode;
{
  scancode_mapD = scancode;
}

// TBD: table is smaller & faster
// 0x9d 157 bytes
int
sym_shift(c)
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
    case '\\':
      return '|';
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
// (0x1d 29 bytes) compact switch table
STATIC char
gamesym_by_scancode(code)
{
  switch (code) {
    case SDL_SCANCODE_KP_1 ... SDL_SCANCODE_KP_9:
      return '1' + (code - SDL_SCANCODE_KP_1);
    case SDL_SCANCODE_KP_0:
      return '0';
    case SDL_SCANCODE_KP_ENTER:
      return ' ';
    case SDL_SCANCODE_KP_PLUS:
      return '+';
    case SDL_SCANCODE_KP_MINUS:
      return '-';
    case SDL_SCANCODE_KP_PERIOD:
      return '.';
    case SDL_SCANCODE_KP_MULTIPLY:
      return '*';
    case SDL_SCANCODE_KP_DIVIDE:
      return '/';
  }

  return 0;
}

int
sdl_keyboard_event(event)
SDL_Event event;
{
  int ret = event.key.keysym.sym;
  int mod = event.key.keysym.mod;

  if (ret >= SDLK_SCANCODE_MASK) {
    uint8_t scancode = MIN(event.key.keysym.scancode, 255);
    if (scancode_mapD)
      ret = scancode_mapD[scancode];
    else
      ret = gamesym_by_scancode(scancode);

    // require numlock off (due to Windows Shift+Numlock complexity)
    if (mod & KMOD_NUM) ret = 0;
  } else {
    if (mod & KMOD_SHIFT) ret = sym_shift(ret);
  }

  if (char_alpha(ret)) {
    if (mod & KMOD_CTRL)
      ret = CTRL(ret);
    else if (mod & KMOD_SHIFT)
      ret ^= 0x20;
  }

  return ret;
}

#define KEYBOARD 1
