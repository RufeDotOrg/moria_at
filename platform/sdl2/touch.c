// Rufe.org LLC 2022-2024: ISC License
enum {
  TOUCH_NONE,
  TOUCH_MENU,
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
  GR_MENU,
  GR_LOCK,
  GR_STAT,
  GR_OVERLAY,
  GR_WIDESCREEN,  // show_history() in landscape orientation
  GR_COUNT
};
enum { MAX_BUTTON = 2 };

DATA rect_t grectD[GR_COUNT];
DATA uint8_t finger_countD;
DATA int last_pressD;

STATIC int
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

STATIC int
overlay_begin()
{
  return 0;
}
STATIC int
overlay_end()
{
  for (int it = AL(overlay_usedD) - 1; it > 0; --it) {
    if (overlay_usedD[it] > 2) return it;
  }
  return AL(overlay_usedD) - 1;
}
STATIC int
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
STATIC int
overlay_input(input)
{
  int row = finger_rowD;
  for (int it = row + input; it >= 0 && it < AL(overlay_usedD); it += input) {
    if (overlay_usedD[it] > 1) return it;
  }
  return row;
}
STATIC void
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

STATIC int
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
    AUSE(grect, GR_MENU);
    if (SDL_PointInRect(&tpp, &grect)) {
      return TOUCH_MENU;
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
    if (SDL_PointInRect(&tpp, &grect)) {
      return TOUCH_LB + it;
    }
  }

  {
    AUSE(grect, GR_PAD);
    int r = 0;
    if (SDL_PointInRect(&tpp, &grect)) {
      SDL_Point rel = {tpp.x - grect.x, tpp.y - grect.y};

      int dsq;
      int n = dpad_nearest_pp(rel.y, rel.x, &dsq);
      if (dsq > limit_dsqD) return 99;

      r = TOUCH_PAD + pp_keyD[n];
    }

    last_pressD = r;

    return r;
  }

  return TOUCH_NONE;
}
STATIC int
orientation_lock_toggle()
{
  MUSE(global, orientation_lock);
  SDL_SetWindowResizable(windowD, orientation_lock);
  globalD.orientation_lock = ~orientation_lock;
  return 0;
}
STATIC int
column_transition(column, dx)
{
  uint32_t ret = column + dx;
  // drop mode
  if (ret > 1) return '0';

  finger_colD = ret;
  return (ret == 0) ? '*' : '/';
}
STATIC int
fingerdown_xy_mode(x, y, mode)
{
  int finger = finger_countD - 1;
  if (KEYBOARD) {
    finger = ((KMOD_SHIFT & SDL_GetModState()) != 0);
  }

  int touch = touch_by_xy(x, y);
  if (mode == 0) {
    if (touch > TOUCH_PAD) {
      char c = key_dir(touch - TOUCH_PAD);
      switch (finger) {
        case 0:
          return c;
        case 1:
          if (c == ' ') return CTRL('w');  // menu
          return c & ~0x20;
      }
    } else if (touch) {
      switch (touch) {
        case TOUCH_MENU:
          return CTRL('w');
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
          return finger ? 'p' : 'a';
        case TOUCH_RB:
          return finger ? '!' : '.';
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
        if (finger) finger_rowD = dx < 0 ? overlay_begin() : overlay_end();
        if (!finger) {
          return column_transition(finger_colD, dx);
        }
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
    if (touch == TOUCH_LB) return finger ? 'p' : 'o';
    if (touch == TOUCH_RB) return ESCAPE;
    if (touch == TOUCH_STAT) return 'c';
    if (touch < 99) return ' ';
  }

  return 0;
}

int
sdl_touch_event(SDL_Event event)
{
  int ret = 0;
  if (TOUCH) {
    USE(view_rect);
    USE(layout_rect);
    USE(mode);

    if (event.type == SDL_FINGERDOWN) {
      finger_countD += 1;
      SDL_FPoint tp = {event.tfinger.x, event.tfinger.y};
      if (SDL_PointInFRect(&tp, &view_rect)) {
        int x = (tp.x - view_rect.x) / view_rect.w * layout_rect.w;
        int y = (tp.y - view_rect.y) / view_rect.h * layout_rect.h;
        ret = fingerdown_xy_mode(x, y, mode);
      }
    } else if (event.type == SDL_FINGERUP) {
      finger_countD -= 1;
      if (!PC && blipD) ret = ' ';
    }

    if (!PC && ret > ' ' && mode == 0 && msg_moreD) ret = ' ';
  }

  return ret;
}

// direct access to selection is not deterministic simulation
STATIC int
touch_selection(int* yptr, int* xptr)
{
  *yptr = finger_colD;
  *xptr = finger_rowD;
  return 0;
}
