
static struct termios save_termD;
// Common terminal commands
static char tc_crlfD[] = "\r\n";
static char tc_clearD[] = "\x1b[2J";
static char tc_clear_lineD[] = "\x1b[K";
static char tc_move_cursorD[] = "\x1b[H";
static char tc_hide_cursorD[] = "\x1b[?25l";
static char tc_show_cursorD[] = "\x1b[?25h";

static char statusD[STATUS_HEIGHT][STATUS_WIDTH];
static char symmapD[SCREEN_HEIGHT][SCREEN_WIDTH];
static char screenD[19][80];
static int screen_usedD[AL(screenD)];
static char overlayD[STATUS_HEIGHT][80 - STATUS_WIDTH];
static int overlay_usedD[STATUS_HEIGHT];
static char logD[80];
static int log_usedD;
static char debugD[80];
static int debug_usedD;

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
static void
platform_draw()
{
  buffer_usedD = 0;
  buffer_append(AP(tc_clearD));
  buffer_append(AP(tc_move_cursorD));
  if (log_usedD) {
    buffer_append(logD, log_usedD);
    log_usedD = 0;
  }
  buffer_append(AP(tc_crlfD));
  if (screen_usedD[0]) {
    for (int row = 0; row < AL(screenD); ++row) {
      buffer_append(AP(tc_clear_lineD));
      buffer_append(screenD[row], screen_usedD[row]);
      buffer_append(AP(tc_crlfD));
    }
    AC(screenD);
    AC(screen_usedD);
  } else if (overlay_usedD[0]) {
    for (int row = 0; row < STATUS_HEIGHT; ++row) {
      buffer_append(AP(tc_clear_lineD));
      buffer_append(AP(statusD[row]));
      buffer_append(overlayD[row], overlay_usedD[row]);
      buffer_append(AP(tc_crlfD));
    }
    AC(overlayD);
    AC(overlay_usedD);
  } else {
    for (int row = 0; row < STATUS_HEIGHT - 1; ++row) {
      buffer_append(AP(tc_clear_lineD));
      buffer_append(AP(statusD[row]));
      if (row < SCREEN_HEIGHT) buffer_append(AP(symmapD[row]));
      buffer_append(AP(tc_crlfD));
    }
    buffer_append(debugD, debug_usedD);
    buffer_append(AP(tc_crlfD));
  }
  if (cD.poison) buffer_append(AP("POISONED "));
  if (uD.food < PLAYER_FOOD_FAINT)
    buffer_append(AP("FAINTING"));
  else if (uD.food < PLAYER_FOOD_WEAK)
    buffer_append(AP("WEAK"));
  else if (uD.food < PLAYER_FOOD_ALERT)
    buffer_append(AP("HUNGRY"));
  buffer_append(AP(tc_move_cursorD));
  write(STDOUT_FILENO, bufferD, buffer_usedD);
}
static void
im_print()
{
  buffer_usedD = 0;
  buffer_append(AP(tc_move_cursorD));
  buffer_append(AP(tc_clear_lineD));
  buffer_append(logD, log_usedD);
  buffer_append(AP(tc_move_cursorD));
  write(STDOUT_FILENO, bufferD, buffer_usedD);
}
void symmap_patch(y, x, c) char c;
{
  int ay = y - panelD.panel_row_min;
  int ax = x - panelD.panel_col_min;
  symmapD[ay][ax] = c;

  int ty = ay + 2;
  int tx = ax + STATUS_WIDTH + 1;

  buffer_usedD = snprintf(AP(bufferD), "\x1b[%d;%dH", ty, tx);
  buffer_append(&c, 1);
  write(STDOUT_FILENO, bufferD, buffer_usedD);
}
#define SYMMAP_PATCH(y, x) \
  if (panel_contains(&panelD, y, x)) symmap_patch(y, x, get_sym(y, x))
