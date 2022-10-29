#include "game_common.h"
#include "game_const.h"
#include "game_type.h"

#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

static struct termios save_termD;
// Common terminal commands
static char tc_crlfD[] = "\r\n";
static char tc_clearD[] = "\x1b[2J";
static char tc_clear_lineD[] = "\x1b[K";
static char tc_move_cursorD[] = "\x1b[H";
static char tc_hide_cursorD[] = "\x1b[?25l";
static char tc_show_cursorD[] = "\x1b[?25h";

void
cave_init()
{
  for (int row = 0; row < MAX_HEIGHT; ++row) {
    for (int col = 0; col < MAX_WIDTH; ++col) {
      bool wall = (row == 0 || row + 1 == MAX_HEIGHT) ||
                  (col == 0 || col + 1 == MAX_WIDTH);
      if (!wall) caveD[row][col].fval = FLOOR_LIGHT;
    }
  }
}

char
get_sym(int row, int col)
{
  if (row == uD.y && col == uD.x) return '@';
  switch (caveD[row][col].fval) {
    case FLOOR_LIGHT:
      return '.';
  }
  return '#';
}

void
panel_bounds(struct panelS* panel)
{
  int panel_row = panel->panel_row;
  int panel_col = panel->panel_col;
  panel->panel_row_min = panel_row * (SCREEN_HEIGHT / 2);
  panel->panel_row_max = panel->panel_row_min + SCREEN_HEIGHT;
  panel->panel_col_min = panel_col * (SCREEN_WIDTH / 2);
  panel->panel_col_max = panel->panel_col_min + SCREEN_WIDTH;
}
void
panel_update(struct panelS* panel, int x, int y, bool force)
{
  bool yd = (y < panel->panel_row_min + 1 || y > panel->panel_row_max - 2);
  if (force || yd) {
    int prow = (y - SCREEN_HEIGHT / 4) / (SCREEN_HEIGHT / 2);
    panel->panel_row = CLAMP(prow, 0, (MAX_HEIGHT / SCREEN_HEIGHT) * 2 - 2);
  }

  bool xd = (x < panel->panel_col_min + 1 || x > panel->panel_col_max - 2);
  if (force || xd) {
    int pcol = (x - SCREEN_WIDTH / 4) / (SCREEN_WIDTH / 2);
    panel->panel_col = CLAMP(pcol, 0, (MAX_WIDTH / SCREEN_WIDTH) * 2 - 2);
  }

  panel_bounds(panel);
}

static char bufferD[4 * 1024];
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
main()
{
  ioctl(0, TCGETA, &save_termD);

  struct termios tbuf;
  tcgetattr(STDIN_FILENO, &tbuf);
  tbuf.c_iflag &= ~(ICRNL | IXON);
  tbuf.c_oflag &= ~(OPOST);
  tbuf.c_lflag &= ~(ECHO | ICANON | ISIG);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &tbuf);

  write(1, tc_hide_cursorD, sizeof(tc_hide_cursorD));

  uD.x = uD.y = 1;
  cave_init();
  panel_bounds(&panelD);

  char c;
  while (1) {
    buffer_usedD = 0;
    buffer_append(AP(tc_clearD));
    buffer_append(AP(tc_move_cursorD));
    int panel_row_min = panelD.panel_row_min;
    int panel_row_max = panelD.panel_row_max;
    int panel_col_min = panelD.panel_col_min;
    int panel_col_max = panelD.panel_col_max;
    for (int row = panel_row_min; row < panel_row_max; ++row) {
      buffer_append(AP(tc_clear_lineD));
      for (int col = panel_col_min; col < panel_col_max; ++col) {
        char c = get_sym(row, col);
        buffer_append(&c, 1);
      }
      buffer_append(AP(tc_crlfD));
    }
    char line[80];
    {
      int print_len = snprintf(AP(line), "(%d,%d) xy (%d,%d) p\r\n", uD.x, uD.y,
                               panelD.panel_col, panelD.panel_row);
      if (print_len < AL(line)) buffer_append(line, print_len);
    }
    buffer_append(AP(tc_move_cursorD));
    write(STDOUT_FILENO, bufferD, buffer_usedD);

    int read_count = read(0, &c, 1);
    if (read_count == -1) {
      if (errno != EAGAIN) break;
      continue;
    }

    if (c == CTRL('c')) break;
    int x = uD.x;
    int y = uD.y;
    switch (c) {
      case 'k':
        y -= (y > 0);
        break;
      case 'j':
        y += (y + 1 < MAX_HEIGHT);
        break;
      case 'l':
        x += (x + 1 < MAX_WIDTH);
        break;
      case 'h':
        x -= (x > 0);
        break;
      case 'n':
        x += (x + 1 < MAX_WIDTH);
        y += (y + 1 < MAX_HEIGHT);
        break;
      case 'b':
        x -= (x > 0);
        y += (y + 1 < MAX_HEIGHT);
        break;
      case 'y':
        x -= (x > 0);
        y -= (y > 0);
        break;
      case 'u':
        x += (x + 1 < MAX_WIDTH);
        y -= (y > 0);
        break;
    }
    if (caveD[y][x].fval != FLOOR_WALL) {
      uD.x = x;
      uD.y = y;
      panel_update(&panelD, uD.x, uD.y, false);
    }
  }

  write(1, tc_clearD, sizeof(tc_clearD));
  write(1, tc_show_cursorD, sizeof(tc_show_cursorD));

  ioctl(0, TCSETA, &save_termD);
  return 0;
}
