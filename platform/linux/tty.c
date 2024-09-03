
// Global application memory
DATA char symmapD[SYMMAP_HEIGHT][SYMMAP_WIDTH];
DATA char save_termD[128];
#define SAVENAME "savetty"

enum { SHOW_MINERAL = 0 };
static char
get_sym(int row, int col)
{
  struct caveS* cave_ptr;

  if (row == uD.y && col == uD.x) return '@';

  cave_ptr = &caveD[row][col];
  if (cave_ptr->midx) {
    struct monS* mon = &entity_monD[cave_ptr->midx];
    struct creatureS* creature = &creatureD[mon->cidx];
    if (mon_lit(cave_ptr->midx)) return creature->cchar;
  }
  if (maD[MA_BLIND] || (CF_VIZ & cave_ptr->cflag) == 0) return ' ';
  if (cave_ptr->oidx) {
    struct objS* obj = &entity_objD[cave_ptr->oidx];
    if (obj->tval != TV_INVIS_TRAP) return obj->tchar;
  }
  switch (cave_ptr->fval) {
    case FLOOR_LIGHT:
    case FLOOR_DARK:
    case FLOOR_CORR:
    case FLOOR_THRESHOLD:
      return '.';
    case FLOOR_OBST:
      return ';';
    case MAGMA_WALL:
    case QUARTZ_WALL:
      if (SHOW_MINERAL) return '%';
      break;
  }
  return '#';
}
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
enum { MINIMAP_WIDTH = SYMMAP_WIDTH };
enum { RATIO = (MAX_WIDTH / MINIMAP_WIDTH) };
#define TL 0 /* top left */
#define TR 1
#define BL 2
#define BR 3
#define HE 4 /* horizontal edge */
#define VE 5
#define CH(x) (screen_border[0][x])
static void
py_map()
{
  int i, j;
  static uint8_t screen_border[][6] = {
      {'+', '+', '+', '+', '-', '|'}, /* normal chars */
  };
  uint8_t map[MAX_WIDTH / RATIO + 1];
  uint8_t tmp;
  int priority[256];
  int row, orow, col;
  char* iter;

  for (i = 0; i < 256; i++) priority[i] = 0;
  priority['<'] = 5;
  priority['>'] = 5;
  priority['@'] = 10;
  priority['+'] = 3;
  priority['#'] = -5;
  priority['%'] = -5;
  priority['.'] = -10;
  priority['\''] = -3;
  priority[' '] = -15;
  priority['B'] = 9;

  iter = screenD[0];
  *iter++ = CH(TL);
  for (int it = 0; it < MINIMAP_WIDTH; ++it) *iter++ = CH(HE);
  *iter++ = CH(TR);
  screen_usedD[0] = (MINIMAP_WIDTH + 2);

  orow = -1;
  map[MAX_WIDTH / RATIO] = '\0';
  for (i = 0; i < MAX_HEIGHT; i++) {
    row = i / RATIO;
    if (row != orow) {
      if (orow >= 0) {
        screen_usedD[orow + 1] =
            snprintf(screenD[orow + 1], AL(screenD[orow + 1]), "%c%s%c", CH(VE),
                     map, CH(VE));
      }
      for (j = 0; j < MAX_WIDTH / RATIO; j++) map[j] = ' ';
      orow = row;
    }
    for (j = 0; j < MAX_WIDTH; j++) {
      col = j / RATIO;
      tmp = get_sym(i, j);
      if (priority[map[col]] < priority[tmp]) map[col] = tmp;
    }
  }
  if (orow >= 0) {
    screen_usedD[orow + 1] = snprintf(screenD[orow + 1], AL(screenD[orow + 1]),
                                      "%c%s%c", CH(VE), map, CH(VE));
  }
  iter = screenD[orow + 2];
  *iter++ = CH(BL);
  for (int it = 0; it < MINIMAP_WIDTH; ++it) *iter++ = CH(HE);
  *iter++ = CH(BR);
  screen_usedD[orow + 2] = (MINIMAP_WIDTH + 2);
}
int
platform_predraw()
{
  if (minimap_enlargeD) {
    py_map();
  } else {
    symmap_update();
  }
  return 1;
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
  int used = buffer_usedD;
  if (used + str_len > sizeof(bufferD)) return 0;
  for (int it = 0; it < str_len; ++it) {
    char c = str[it];
    bufferD[used + it] = c ? c : ' ';
  }
  buffer_usedD = used + str_len;
  return 1;
}
int
platform_draw()
{
  buffer_usedD = 0;
  buffer_append(SP(tc_clearD));
  buffer_append(SP(tc_move_cursorD));
  char* msg = AS(msg_cqD, msg_writeD);
  int msg_used = AS(msglen_cqD, msg_writeD);

  if (!msg_used) {
    msg = AS(msg_cqD, msg_writeD - 1);
    msg_used = AS(msglen_cqD, msg_writeD - 1);
  }

  if (msg_used) {
    buffer_append(msg, msg_used);
  }
  if (msg_moreD) {
    buffer_append(AP(" -more-"));
  }
  buffer_append(SP(tc_crlfD));
  if (screen_usedD[0]) {
    for (int row = 0; row < AL(screenD); ++row) {
      buffer_append(SP(tc_clear_lineD));
      buffer_append(screenD[row], screen_usedD[row]);
      buffer_append(SP(tc_crlfD));
    }
  } else if (overlay_usedD[0]) {
    for (int row = 0; row < AL(overlayD); ++row) {
      buffer_append(SP(tc_clear_lineD));
      buffer_append(overlayD[row], overlay_usedD[row]);
      buffer_append(SP(tc_crlfD));
    }
  } else {
    char vital[SYMMAP_HEIGHT][13];
    int it, row;

    memset(vital, 0x20202020, sizeof(vital));
    row = 0;
    for (it = 0; it < MAX_A; ++it, ++row) {
      snprintf(vital[row], AL(vital[0]), "%-4.04s: %6d", stat_abbrD[it],
               vital_statD[it]);
    }
    row += 1;
    it = 0;
    snprintf(vital[row++], AL(vital[0]), "LEV : %6d", vitalD[it++]);
    snprintf(vital[row++], AL(vital[0]), "EXP :%7d", vitalD[it++]);
    snprintf(vital[row++], AL(vital[0]), "MANA: %6d", vitalD[it++]);
    snprintf(vital[row++], AL(vital[0]), "MHP : %6d", vitalD[it++]);
    snprintf(vital[row++], AL(vital[0]), "CHP : %6d", vitalD[it++]);
    row += 1;
    snprintf(vital[row++], AL(vital[0]), "AC  : %6d", vitalD[it++]);
    snprintf(vital[row++], AL(vital[0]), "GOLD: %6d", vitalD[it++]);

    for (it = 0; it < SYMMAP_HEIGHT; ++it) {
      buffer_append(SP(tc_clear_lineD));
      buffer_append(vital[it], AL(vital[0]));
      buffer_append(AP(symmapD[it]));
      if (it < AL(active_affectD)) {
        if (active_affectD[it]) {
          char* aff = affectD[it][active_affectD[it] - 1];
          buffer_append(aff, strlen(aff));
        }
      }
      buffer_append(SP(tc_crlfD));
    }
  }
  buffer_append(SP(tc_move_cursorD));
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

int
platform_save()
{
  int fd, byte_count;
  byte_count = 0;
  for (int it = 0; it < AL(save_bufD); ++it) {
    struct bufS buf = save_bufD[it];
    byte_count += buf.mem_size;
  }
  fd = open(SAVENAME, O_CREAT | O_TRUNC | O_WRONLY, 0600);
  if (fd) {
    write(fd, &byte_count, sizeof(byte_count));
    for (int it = 0; it < AL(save_bufD); ++it) {
      struct bufS buf = save_bufD[it];
      write(fd, buf.mem, buf.mem_size);
    }
    close(fd);
    Log("platform_save %d byte_count", byte_count);
    return byte_count;
  }
  return 0;
}
int
platform_load()
{
  int fd, byte_count, save_size;

  byte_count = 0;
  for (int it = 0; it < AL(save_bufD); ++it) {
    struct bufS buf = save_bufD[it];
    byte_count += buf.mem_size;
  }

  fd = open(SAVENAME, O_RDONLY);
  if (fd != -1) {
    read(fd, &save_size, sizeof(save_size));
    if (save_size == byte_count) {
      for (int it = 0; it < AL(save_bufD); ++it) {
        struct bufS buf = save_bufD[it];
        read(fd, buf.mem, buf.mem_size);
      }
    }
    close(fd);
    Log("platform_load %d save_size %d byte_count", save_size, byte_count);
    return save_size == byte_count;
  }

  return fd != -1;
}
int
platform_erase()
{
  int fd;
  fd = open(SAVENAME, O_CREAT | O_TRUNC | O_WRONLY, 0600);
  close(fd);
  return fd != -1;
}
int
platform_random()
{
  int seed, fd;
  fd = open("/dev/urandom", O_RDONLY);
  if (fd != -1) {
    read(fd, &seed, sizeof(seed));
    close(fd);
  }
  return seed;
}
