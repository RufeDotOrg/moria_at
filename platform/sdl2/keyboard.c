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
char_by_dir(dir)
{
  switch (dir) {
    case 5:
    case 0:
      return ' ';
    case 4:
      return 'h';
    case 7:
      return 'y';
    case 8:
      return 'k';
    case 9:
      return 'u';
    case 6:
      return 'l';
    case 3:
      return 'n';
    case 2:
      return 'j';
    case 1:
      return 'b';
  }

  // Display refresh
  return CTRL('d');
}
STATIC char
gamesym_by_scancode(code, shiftbit)
{
  switch (code) {
    case SDL_SCANCODE_KP_1 ... SDL_SCANCODE_KP_9: {
      int dir = 1 + (code - SDL_SCANCODE_KP_1);
      char c = char_by_dir(dir);
      if (c <= ' ') return c;
      return c ^ shiftbit;
    }
    case SDL_SCANCODE_KP_0:
      return 'm';
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
  int mod = event.key.keysym.mod;
  int shift = (mod & KMOD_SHIFT) != 0 ? 0x20 : 0;

  if (event.key.keysym.sym < SDLK_SCANCODE_MASK) {
    if (char_alpha(event.key.keysym.sym)) {
      int ctrl = (mod & KMOD_CTRL);
      if (ctrl)
        return CTRL(event.key.keysym.sym);
      else
        return event.key.keysym.sym ^ shift;
    } else {
      return shift ? sym_shift(event.key.keysym.sym) : event.key.keysym.sym;
    }
  } else {
    if (mod & KMOD_NUM) return 0;

    return gamesym_by_scancode(event.key.keysym.scancode, shift);
  }
  return 0;
}

#define KEYBOARD 1
