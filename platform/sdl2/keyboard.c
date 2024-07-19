DATA char* numdir_remapD;

int
keyboard_numdir(remap)
char* remap;
{
  numdir_remapD = remap;
}

STATIC char
sym_shift(char c)
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
  int ret = 0;
  int mod = event.key.keysym.mod;
  int shift = (mod & KMOD_SHIFT) != 0 ? 0x20 : 0;

  if (event.key.keysym.sym < SDLK_SCANCODE_MASK) {
    if (char_alpha(event.key.keysym.sym)) {
      int ctrl = (mod & KMOD_CTRL);
      ret = ctrl ? CTRL(event.key.keysym.sym) : (event.key.keysym.sym ^ shift);
    } else {
      ret = shift ? sym_shift(event.key.keysym.sym) : event.key.keysym.sym;
    }
  } else {  // Scancode
    int numlock = (mod & KMOD_NUM);
    // require numlock off (due to Windows Shift+Numlock complexity)
    if (!numlock) {
      ret = gamesym_by_scancode(event.key.keysym.scancode);
      if (char_digit(ret) && numdir_remapD)
        ret = numdir_remapD[ret - '0'] ^ shift;
    }
  }
  return ret;
}

#define KEYBOARD 1
