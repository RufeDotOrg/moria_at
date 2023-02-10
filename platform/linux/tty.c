
// Global application memory
EXTERN char symmapD[SYMMAP_HEIGHT][SYMMAP_WIDTH];
EXTERN char save_termD[128];

static void
symmap_update()
{
  int rmin = panelD.panel_row_min;
  int rmax = panelD.panel_row_max;
  int cmin = panelD.panel_col_min;
  int cmax = panelD.panel_col_max;
  char* sym = &symmapD[0][0];
  for (int row = rmin; row < rmax; ++row) {
    for (int col = cmin; col < cmax; ++col) {
      *sym++ = get_sym(row, col);
    }
  }
}

// Common terminal commands
static char tc_crlfD[] = "\r\n";
static char tc_clearD[] = "\x1b[2J";
static char tc_clear_lineD[] = "\x1b[K";
static char tc_move_cursorD[] = "\x1b[H";
static char tc_hide_cursorD[] = "\x1b[?25l";
static char tc_show_cursorD[] = "\x1b[?25h";

static char bufferD[2 * 1024];
static int buffer_usedD;
int
buffer_append(char* str, int str_len)
{
  if (buffer_usedD + str_len > sizeof(bufferD)) return 0;
  memcpy(&bufferD[buffer_usedD], str, str_len);
  buffer_usedD += str_len;
  return 1;
}
int
platform_draw()
{
  buffer_usedD = 0;
  buffer_append(AP(tc_clearD));
  buffer_append(AP(tc_move_cursorD));
  char* msg = AS(msg_cqD, msg_writeD);
  int msg_used = AS(msglen_cqD, msg_writeD);
  if (msg_used) {
    buffer_append(msg, msg_used);
  }
  buffer_append(AP(tc_crlfD));
  if (screen_usedD[0]) {
    for (int row = 0; row < AL(screenD); ++row) {
      buffer_append(AP(tc_clear_lineD));
      buffer_append(screenD[row], screen_usedD[row]);
      buffer_append(AP(tc_crlfD));
    }
  } else if (overlay_usedD[0]) {
    for (int row = 0; row < STATUS_HEIGHT; ++row) {
      buffer_append(AP(tc_clear_lineD));
      buffer_append(AP(vitalinfoD[row]));
      buffer_append(overlayD[row], overlay_usedD[row]);
      buffer_append(AP(tc_crlfD));
    }
  } else {
    for (int row = 0; row < STATUS_HEIGHT - 1; ++row) {
      buffer_append(AP(tc_clear_lineD));
      buffer_append(AP(vitalinfoD[row]));
      if (row < AL(symmapD)) buffer_append(AP(symmapD[row]));
      buffer_append(AP(tc_crlfD));
    }
    buffer_append(affectinfoD, affectinfo_usedD);
  }
  buffer_append(AP(tc_move_cursorD));
  write(STDOUT_FILENO, bufferD, buffer_usedD);
  return 1;
}
static char
_from_vt100(char c)
{
  switch (c) {
    case 'F':
      return 'b';
    case 'B':
      return 'j';
    case '6':
      return 'n';
    case 'D':
      return 'h';
    case 'E':
      return '.';
    case 'C':
      return 'l';
    case 'H':
      return 'y';
    case 'A':
      return 'k';
    case '5':
      return 'u';
  }
  return 0;
}
char
tty_translate(char* str, int len)
{
  char c = 0;
  char mask = -1;
  if (len > 5) {
    c = str[5];
    if (c == '~') c = str[2];
    mask = ~0x20;
  } else if (len > 2) {
    c = str[2];
  }

  return (_from_vt100(c) & mask);
}
