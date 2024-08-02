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
    case SDL_SCANCODE_RETURN:
      return '\n';
    case SDL_SCANCODE_KP_1 ... SDL_SCANCODE_KP_9:
      return '1' + (code - SDL_SCANCODE_KP_1);
    case SDL_SCANCODE_KP_0:
      return '0';
    case SDL_SCANCODE_KP_ENTER:
      return '\n';
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
  int sym;
  int mod = event.key.keysym.mod;

  // Scancode override:
  // Gameplay mode raw inputs
  // Player customization
  if (scancode_mapD) {
    uint8_t scancode = MIN(event.key.keysym.scancode, 255);
    sym = scancode_mapD[scancode];
    // require numlock off (due to Windows Shift+Numlock complexity)
    if (mod & KMOD_NUM) sym = 0;
  } else {
    // Common text input
    sym = event.key.keysym.sym;
    if (sym >= SDLK_SCANCODE_MASK) {
      sym = gamesym_by_scancode(event.key.keysym.scancode);
    }
  }

  // Compression (TBD customization or mode toggle)
  if (char_alpha(sym)) {
    if (mod & KMOD_CTRL)
      sym = CTRL(sym);
    else if (mod & KMOD_SHIFT)
      sym ^= 0x20;
  } else {
    if (mod & KMOD_SHIFT) sym = sym_shift(sym);
  }

  return sym;
}

#define KEYBOARD 1
