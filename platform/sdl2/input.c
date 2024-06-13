

enum {
  TOUCH_NONE,
  TOUCH_HISTORY,
  TOUCH_LOCK,
  TOUCH_STAT,
  TOUCH_MAP,
  TOUCH_VERSION,
  TOUCH_GAMEPLAY,
  TOUCH_LB,
  TOUCH_RB,
  TOUCH_PAD
};
enum {
  GR_VERSION,
  GR_PAD,
  GR_BUTTON1,
  GR_BUTTON2,
  GR_GAMEPLAY,
  GR_MINIMAP,
  GR_HISTORY,
  GR_LOCK,
  GR_STAT,
  GR_OVERLAY,
  GR_WIDESCREEN,  // show_history() in landscape orientation
  GR_COUNT
};
enum { MAX_BUTTON = 2 };

DATA SDL_Point ppD[9];
DATA int pp_keyD[9] = {5, 6, 3, 2, 1, 4, 7, 8, 9};
DATA rect_t grectD[GR_COUNT];
DATA uint8_t finger_countD;
DATA int last_pressD;
DATA int quitD;

char
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
static char
char_by_dir(dir)
{
  switch (dir) {
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
    // As a platform choice, prefer ' ' over '.'
    // The former does not yield the turn to creatures
    default:
      return ' ';
  }
}
static char
gamesym_by_scancode(code, shiftbit)
{
  USE(mode);
  if (mode == 0) {
    switch (code) {
      case SDL_SCANCODE_KP_1 ... SDL_SCANCODE_KP_9: {
        int dir = 1 + (code - SDL_SCANCODE_KP_1);
        return char_by_dir(dir) ^ shiftbit;
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
    }
  }

  if (mode > 0) {
    switch (code) {
      case SDL_SCANCODE_KP_MINUS:
        return '-';
      case SDL_SCANCODE_KP_MULTIPLY:
        return '*';
      case SDL_SCANCODE_KP_DIVIDE:
        return '/';
    }
  }
  return 0;
}

static int
nearest_pp(y, x, po_dsq)
int* po_dsq;
{
  int r = -1;
  int64_t min_dsq = INT64_MAX;
  for (int it = 0; it < AL(ppD); ++it) {
    int ppx = ppD[it].x;
    int ppy = ppD[it].y;
    int dx = ppx - x;
    int dy = ppy - y;
    int dsq = dx * dx + dy * dy;
#define TOUCH_CARDINAL_WEIGHT .43f
    if (it % 2 == 1) dsq *= TOUCH_CARDINAL_WEIGHT;
    if (dsq < min_dsq) {
      min_dsq = dsq;
      r = it;
    }
  }
  if (po_dsq) *po_dsq = min_dsq;
  return r;
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

int
gameplay_tapxy(relx, rely)
{
  rect_t zr;
  zoom_rect(&zr);

  int try = (float)rely * zr.h;
  try /= SYMMAP_HEIGHT;
  try /= ART_H;

  int trx = (float)relx * zr.w;
  trx /= SYMMAP_WIDTH;
  trx /= ART_W;

  ylookD = zr.y + CLAMP(try, 0, zr.h - 1);
  xlookD = zr.x + CLAMP(trx, 0, zr.w - 1);

  return 0;
}

int
overlay_begin()
{
  return 0;
}
int
overlay_end()
{
  for (int it = AL(overlay_usedD) - 1; it > 0; --it) {
    if (overlay_usedD[it] > 2) return it;
  }
  return AL(overlay_usedD) - 1;
}
int
overlay_bisect(dir)
{
  int sample[AL(overlay_usedD)];
  int sample_used;
  int row = finger_rowD;

  sample[0] = CLAMP(row + dir, 0, AL(overlay_usedD) - 1);
  sample_used = 0;
  for (int it = row; it >= 0 && it < AL(overlay_usedD); it += dir) {
    if (overlay_usedD[it] > 2) {
      sample[sample_used] = it;
      sample_used += 1;
    }
  }

  return sample[sample_used / 2];
}
int
overlay_input(input)
{
  int row = finger_rowD;
  for (int it = row + input; it >= 0 && it < AL(overlay_usedD); it += input) {
    if (overlay_usedD[it] > 1) return it;
  }
  return row;
}
static void
overlay_autoselect()
{
  int row = finger_rowD;
  if (overlay_usedD[row] <= 1) {
    for (int it = row + 1; it < AL(overlay_usedD); it += 1) {
      if (overlay_usedD[it] > 1) {
        finger_rowD = it;
        return;
      }
    }

    for (int it = row - 1; it > 0; --it) {
      if (overlay_usedD[it] > 1) {
        finger_rowD = it;
        return;
      }
    }

    finger_rowD = 0;
  }
}

static int
touch_by_xy(x, y)
{
  SDL_Point tpp = {x, y};
  {
    AUSE(grect, GR_STAT);
    if (SDL_PointInRect(&tpp, &grect)) {
      return TOUCH_STAT;
    }
  }
  {
    AUSE(grect, GR_HISTORY);
    if (SDL_PointInRect(&tpp, &grect)) {
      return TOUCH_HISTORY;
    }
  }
  {
    AUSE(grect, GR_LOCK);
    if (SDL_PointInRect(&tpp, &grect)) {
      return TOUCH_LOCK;
    }
  }
  {
    AUSE(grect, GR_MINIMAP);
    if (SDL_PointInRect(&tpp, &grect)) {
      return TOUCH_MAP;
    }
  }

  enum { OPT_VERSION = 0 };
  if (OPT_VERSION) {
    AUSE(grect, GR_VERSION);
    if (SDL_PointInRect(&tpp, &grect)) {
      return TOUCH_VERSION;
    }
  }

  {
    AUSE(grect, GR_GAMEPLAY);
    if (SDL_PointInRect(&tpp, &grect)) {
      gameplay_tapxy(x - grect.x, y - grect.y);
      return TOUCH_GAMEPLAY;
    }
  }

  for (int it = 0; it < MAX_BUTTON; ++it) {
    AUSE(grect, GR_BUTTON1 + it);
    if (SDL_PointInRect(&tpp, &grect)) return TOUCH_LB + it;
  }

  {
    AUSE(grect, GR_PAD);
    int r = 0;
    if (SDL_PointInRect(&tpp, &grect)) {
      SDL_Point rel = {tpp.x - grect.x, tpp.y - grect.y};

      int n = nearest_pp(rel.y, rel.x, 0);
      r = TOUCH_PAD + pp_keyD[n];
    }

    last_pressD = r;

    return r;
  }
}
static int
orientation_lock_toggle()
{
  MUSE(global, orientation_lock);
  SDL_SetWindowResizable(windowD, orientation_lock);
  globalD.orientation_lock = ~orientation_lock;
  return 0;
}
static int
column_transition(dx)
{
  USE(finger_col);

  finger_col = CLAMP(finger_col + dx, 0, 1);
  finger_colD = finger_col;

  return (finger_col == 0) ? '*' : '/';
}
static int
fingerdown_xy_mode(x, y, mode)
{
  int finger = finger_countD - 1;
  if (KEYBOARD) {
    finger = ((KMOD_SHIFT & SDL_GetModState()) != 0);
  }

  int touch = touch_by_xy(x, y);
  if (mode == 0) {
    if (touch > TOUCH_PAD) {
      char c = char_by_dir(touch - TOUCH_PAD);
      switch (finger) {
        case 0:
          return c;
        case 1:
          if (c == ' ') return '=';
          return c & ~0x20;
        default:
          break;
      }
    } else if (touch) {
      switch (touch) {
        case TOUCH_HISTORY:
          return CTRL('p');
        case TOUCH_LOCK:
          orientation_lock_toggle();
          return CTRL('d');
        case TOUCH_STAT:
          return 'c';
        case TOUCH_MAP:
          return 'm';
        case TOUCH_VERSION:
          return 'v';
        case TOUCH_GAMEPLAY:
          return finger ? '-' : 'O';
        case TOUCH_LB:
          return finger ? 'd' : 'a';
        case TOUCH_RB:
          return finger ? '!' : '.';
        default:
          break;
      }
    }
  }
  if (mode == 1) {
    if (touch > TOUCH_PAD) {
      int dir = touch - TOUCH_PAD;
      int dx = dir_x(dir);
      int dy = dir_y(dir);

      if (!dx && !dy) {
        return 'A' + finger_rowD;
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
    if (touch == TOUCH_LB) {
      return ESCAPE;
    }
    if (touch == TOUCH_RB) {
      return 'a' + finger_rowD;
    }
    if (touch == TOUCH_GAMEPLAY && finger) return '-';
  }
  if (mode == 2) {
    if (touch == TOUCH_LB) return 'o';
    if (touch == TOUCH_RB) return ESCAPE;
    if (touch == TOUCH_STAT) return 'c';
    if (touch == TOUCH_HISTORY) return CTRL('p');
    if (touch) return ' ';
  }

  return 0;
}
char
sdl_pump()
{
  USE(view_rect);
  USE(layout_rect);
  USE(mode);
  SDL_Event event;
  int ret = 0;

  while (ret == 0 && SDL_PollEvent(&event)) {
    if ((MOUSE || TOUCH) && event.type == SDL_FINGERDOWN) {
      finger_countD += 1;
      SDL_FPoint tp = {event.tfinger.x, event.tfinger.y};
      if (SDL_PointInFRect(&tp, &view_rect)) {
        int x = (tp.x - view_rect.x) / view_rect.w * layout_rect.w;
        int y = (tp.y - view_rect.y) / view_rect.h * layout_rect.h;
        ret = fingerdown_xy_mode(x, y, mode);
      }
    } else if ((MOUSE || TOUCH) && event.type == SDL_FINGERUP) {
      finger_countD -= 1;
      if (!PC && blipD) ret = ' ';
    } else if (KEYBOARD && (event.type == SDL_KEYDOWN)) {
      ret = sdl_keyboard_event(event);
    } else if (event.type == SDL_QUIT) {
      quitD = TRUE;
    } else if (event.type == SDL_WINDOWEVENT) {
      ret = sdl_window_event(event);
    }
  }

  if (ret == 0) {
    nanosleep(&(struct timespec){0, 8e6}, 0);
    if (PC) ret = CTRL('d');
  } else {
    USE(msg_more);
    // TBD: game option to allow soft ack of -more-?
    if (!PC && mode == 0 && msg_more && ret != CTRL('d')) ret = ' ';
  }

  return ret;
}
// mode_change is triggered by interactive UI navigation
// may edit row/col selection to make the interface feel "smart"
// not utilized by the replay system
static int
mode_change(mnext)
{
  int subprev = submodeD;
  int subnext = overlay_submodeD;
  int mprev = modeD;

  if (mprev != mnext || subprev != subnext) {
    if (mprev == 1) ui_stateD[subprev] = finger_rowD;

    if (mnext == 1) {
      finger_rowD = (subnext > 0) ? ui_stateD[subnext] : 0;
      finger_colD = (subnext == 'e') ? 1 : 0;

      overlay_autoselect();
    }
  }

  modeD = mnext;
  submodeD = subnext;

  return mnext;
}
int
platform_readansi()
{
  char c = sdl_pump();
  if (quitD) return CTRL('c');
  return c;
}

// direct access to selection is not deterministic simulation
int
platform_selection(int* yptr, int* xptr)
{
  *yptr = finger_colD;
  *xptr = finger_rowD;
  return 0;
}

int
input_init()
{
  platformD.readansi = platform_readansi;
  if (TOUCH) platformD.selection = platform_selection;
  return 1;
}

#define INPUT 1
