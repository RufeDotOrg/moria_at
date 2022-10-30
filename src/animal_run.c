#include "game_common.h"
#include "game_const.h"
#include "game_type.h"

#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

static int death;
static int dun_level;
static int free_turn_flag;
static int new_level_flag;
static int modeD;
static char logD[80];
static int log_usedD;
ARR_REUSE(obj, 256);

static struct termios save_termD;
// Common terminal commands
static char tc_crlfD[] = "\r\n";
static char tc_clearD[] = "\x1b[2J";
static char tc_clear_lineD[] = "\x1b[K";
static char tc_move_cursorD[] = "\x1b[H";
static char tc_hide_cursorD[] = "\x1b[?25l";
static char tc_show_cursorD[] = "\x1b[?25h";

static void
msg_print(char* text)
{
  // TBD: drops text, does not halt game for -more-
  log_usedD = snprintf(AP(logD), "%s", text);
}
static void
go_up()
{
  register struct caveS* c_ptr;
  register int no_stairs = FALSE;

  c_ptr = &caveD[uD.y][uD.x];
  if (c_ptr->oidx != 0)
    if (entity_objD[c_ptr->oidx].tval == TV_UP_STAIR) {
      dun_level -= 1;
      new_level_flag = TRUE;
      msg_print("You enter a maze of up staircases.");
      msg_print("You pass through a one-way door.");
    } else
      no_stairs = TRUE;
  else
    no_stairs = TRUE;

  if (no_stairs) {
    msg_print("I see no up staircase here.");
    free_turn_flag = TRUE;
  }
}
static void
go_down()
{
  register struct caveS* c_ptr;
  register int no_stairs = FALSE;

  c_ptr = &caveD[uD.y][uD.x];
  if (c_ptr->oidx != 0)
    if (entity_objD[c_ptr->oidx].tval == TV_DOWN_STAIR) {
      dun_level += 1;
      new_level_flag = TRUE;
      msg_print("You enter a maze of down staircases.");
      msg_print("You pass through a one-way door.");
    } else
      no_stairs = TRUE;
  else
    no_stairs = TRUE;

  if (no_stairs) {
    msg_print("I see no down staircase here.");
    free_turn_flag = TRUE;
  }
}
void
cave_init()
{
  for (int row = 0; row < MAX_HEIGHT; ++row) {
    for (int col = 0; col < MAX_WIDTH; ++col) {
      bool wall = (row == 0 || row + 1 == MAX_HEIGHT) ||
                  (col == 0 || col + 1 == MAX_WIDTH);
      if (wall)
        caveD[row][col].fval = GRANITE_WALL;
      else
        caveD[row][col].fval = FLOOR_LIGHT;
    }
  }
  uD.x = uD.y = 1;
}

#define RNG_M 2147483647L /* m = 2^31 - 1 */
#define RNG_A 16807L
#define RNG_Q 127773L /* m div a */
#define RNG_R 2836L   /* m mod a */
static uint32_t rnd_seed = 3123;
int
rnd()
{
  register long low, high, test;

  high = rnd_seed / RNG_Q;
  low = rnd_seed % RNG_Q;
  test = RNG_A * low - RNG_R * high;
  if (test > 0)
    rnd_seed = test;
  else
    rnd_seed = test + RNG_M;
  return rnd_seed;
}

int
randint(maxval)
int maxval;
{
  register long randval;

  randval = rnd();
  return ((int)(randval % maxval) + 1);
}

static void build_room(yval, xval) int yval, xval;
{
  register int i, j, y_depth, x_right;
  int y_height, x_left;
  uint8_t floor;
  register struct caveS *c_ptr, *d_ptr;

  if (dun_level <= randint(25))
    floor = FLOOR_LIGHT;
  else
    floor = FLOOR_DARK;

  y_height = yval - randint(1);
  y_depth = yval + randint(2);
  x_left = xval - randint(2);
  x_right = xval + randint(3);

  for (i = y_height; i <= y_depth; i++) {
    c_ptr = &caveD[i][x_left];
    for (j = x_left; j <= x_right; j++) {
      c_ptr->fval = floor;
      c_ptr->lr = TRUE;
      c_ptr++;
    }
  }

  for (i = (y_height - 1); i <= (y_depth + 1); i++) {
    c_ptr = &caveD[i][x_left - 1];
    c_ptr->fval = GRANITE_WALL;
    c_ptr->lr = TRUE;
    c_ptr = &caveD[i][x_right + 1];
    c_ptr->fval = GRANITE_WALL;
    c_ptr->lr = TRUE;
  }

  c_ptr = &caveD[y_height - 1][x_left];
  d_ptr = &caveD[y_depth + 1][x_left];
  for (i = x_left; i <= x_right; i++) {
    c_ptr->fval = GRANITE_WALL;
    c_ptr->lr = TRUE;
    c_ptr++;
    d_ptr->fval = GRANITE_WALL;
    d_ptr->lr = TRUE;
    d_ptr++;
  }
}
typedef struct {
  int x;
  int y;
} coords;
bool
in_bounds(int row, int col)
{
  bool rc = (row > 0 && row < MAX_HEIGHT - 1);
  bool cc = (col > 0 && col < MAX_WIDTH - 1);
  return cc && rc;
}
static void rand_dir(rdir, cdir) int *rdir, *cdir;
{
  register int tmp;

  tmp = randint(4);
  if (tmp < 3) {
    *cdir = 0;
    *rdir = -3 + (tmp << 1); /* tmp=1 -> *rdir=-1; tmp=2 -> *rdir=1 */
  } else {
    *rdir = 0;
    *cdir = -7 + (tmp << 1); /* tmp=3 -> *cdir=-1; tmp=4 -> *cdir=1 */
  }
}
static void correct_dir(rdir, cdir, y1, x1, y2, x2) int *rdir, *cdir;
register int y1, x1, y2, x2;
{
  if (y1 < y2)
    *rdir = 1;
  else if (y1 == y2)
    *rdir = 0;
  else
    *rdir = -1;
  if (x1 < x2)
    *cdir = 1;
  else if (x1 == x2)
    *cdir = 0;
  else
    *cdir = -1;
  if ((*rdir != 0) && (*cdir != 0)) {
    if (randint(2) == 1)
      *rdir = 0;
    else
      *cdir = 0;
  }
}
static void place_broken_door(broken, y, x) int broken, y, x;
{
  register struct objS* obj;
  register struct caveS* cave_ptr;

  // invcopy(&t_list[cur_pos], OBJ_OPEN_DOOR);
  obj = obj_use();
  obj->tval = TV_OPEN_DOOR;
  obj->tchar = '\'';
  obj->subval = 1;
  obj->number = 1;
  obj->p1 = broken;

  cave_ptr = &caveD[y][x];
  cave_ptr->oidx = obj_index(obj);
  cave_ptr->fval = FLOOR_CORR;
}
static void place_closed_door(locked, y, x) int locked, y, x;
{
  register struct objS* obj;
  register struct caveS* cave_ptr;

  // invcopy(&t_list[cur_pos], OBJ_CLOSED_DOOR);
  obj = obj_use();
  obj->tval = TV_CLOSED_DOOR;
  obj->tchar = '+';
  obj->subval = 1;
  obj->number = 1;
  obj->p1 = locked;

  cave_ptr = &caveD[y][x];
  cave_ptr->oidx = obj_index(obj);
  cave_ptr->fval = FLOOR_OBST;
}
static void place_secret_door(y, x) int y, x;
{
  register struct objS* obj;
  register struct caveS* cave_ptr;

  // invcopy(&t_list[cur_pos], OBJ_SECRET_DOOR);
  obj = obj_use();
  obj->tval = TV_SECRET_DOOR;
  obj->tchar = '8';
  obj->subval = 1;
  obj->number = 1;

  cave_ptr = &caveD[y][x];
  cave_ptr->oidx = obj_index(obj);
  cave_ptr->fval = FLOOR_OBST;
}
static void place_door(y, x) int y, x;
{
  register int tmp;
  register int lock;

  tmp = randint(3);
  if (tmp == 1) {
    place_broken_door(randint(4) == 1, y, x);
  } else if (tmp == 2) {
    tmp = randint(12);
    lock = randint(10);
    if (tmp > 3)
      place_closed_door(0, y, x);
    else if (tmp == 3)
      place_closed_door(-lock, y, x);
    else
      place_closed_door(lock, y, x);
  } else
    place_secret_door(y, x);
}
static coords doorstk[100];
static int doorindex;
static void build_tunnel(row1, col1, row2, col2) int row1, col1, row2, col2;
{
  register int tmp_row, tmp_col, i, j;
  register struct caveS* c_ptr;
  struct caveS* d_ptr;
  coords tunstk[1000], wallstk[1000];
  coords* tun_ptr;
  int row_dir, col_dir, tunindex, wallindex;
  int stop_flag, door_flag, main_loop_count;
  int start_row, start_col;

  /* Main procedure for Tunnel  		*/
  /* Note: 9 is a temporary value  	*/
  stop_flag = FALSE;
  door_flag = FALSE;
  tunindex = 0;
  wallindex = 0;
  main_loop_count = 0;
  start_row = row1;
  start_col = col1;
  correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

  do {
    /* prevent infinite loops, just in case */
    main_loop_count++;
    if (main_loop_count > 2000) stop_flag = TRUE;

    if (randint(100) > DUN_TUN_CHG) {
      if (randint(DUN_TUN_RND) == 1)
        rand_dir(&row_dir, &col_dir);
      else
        correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);
    }
    tmp_row = row1 + row_dir;
    tmp_col = col1 + col_dir;
    while (!in_bounds(tmp_row, tmp_col)) {
      if (randint(DUN_TUN_RND) == 1)
        rand_dir(&row_dir, &col_dir);
      else
        correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);
      tmp_row = row1 + row_dir;
      tmp_col = col1 + col_dir;
    }
    c_ptr = &caveD[tmp_row][tmp_col];
    if (c_ptr->fval == FLOOR_NULL) {
      row1 = tmp_row;
      col1 = tmp_col;
      if (tunindex < 1000) {
        tunstk[tunindex].y = row1;
        tunstk[tunindex].x = col1;
        tunindex++;
      }
      door_flag = FALSE;
    } else if (c_ptr->fval == TMP2_WALL)
      /* do nothing */
      ;
    else if (c_ptr->fval == GRANITE_WALL) {
      row1 = tmp_row;
      col1 = tmp_col;
      if (wallindex < 1000) {
        wallstk[wallindex].y = row1;
        wallstk[wallindex].x = col1;
        wallindex++;
      }
      for (i = row1 - 1; i <= row1 + 1; i++)
        for (j = col1 - 1; j <= col1 + 1; j++)
          if (in_bounds(i, j)) {
            d_ptr = &caveD[i][j];
            /* values 11 and 12 are impossible here, place_streamer
               is never run before build_tunnel */
            if (d_ptr->fval == GRANITE_WALL) d_ptr->fval = TMP2_WALL;
          }
    } else if (c_ptr->fval == FLOOR_CORR || c_ptr->fval == FLOOR_OBST) {
      row1 = tmp_row;
      col1 = tmp_col;
      if (!door_flag) {
        if (doorindex < 100) {
          doorstk[doorindex].y = row1;
          doorstk[doorindex].x = col1;
          doorindex++;
        }
        door_flag = TRUE;
      }
      if (randint(100) > DUN_TUN_CON) {
        /* make sure that tunnel has gone a reasonable distance
           before stopping it, this helps prevent isolated rooms */
        tmp_row = row1 - start_row;
        if (tmp_row < 0) tmp_row = -tmp_row;
        tmp_col = col1 - start_col;
        if (tmp_col < 0) tmp_col = -tmp_col;
        if (tmp_row > 10 || tmp_col > 10) stop_flag = TRUE;
      }
    } else /* c_ptr->fval != NULL, TMP2, GRANITE, CORR */
    {
      row1 = tmp_row;
      col1 = tmp_col;
    }
  } while (((row1 != row2) || (col1 != col2)) && (!stop_flag));

  tun_ptr = &tunstk[0];
  for (i = 0; i < tunindex; i++) {
    d_ptr = &caveD[tun_ptr->y][tun_ptr->x];
    d_ptr->fval = FLOOR_CORR;
    tun_ptr++;
  }
  for (i = 0; i < wallindex; i++) {
    c_ptr = &caveD[wallstk[i].y][wallstk[i].x];
    if (c_ptr->fval == TMP2_WALL) {
      if (randint(100) < DUN_TUN_PEN)
        place_door(wallstk[i].y, wallstk[i].x);
      else {
        /* these have to be doorways to rooms */
        c_ptr->fval = FLOOR_CORR;
      }
    }
  }
}
static void fill_cave(fval) register int fval;
{
  register int i, j;
  register struct caveS* c_ptr;

  /* no need to check the border of the cave */

  for (i = MAX_HEIGHT - 2; i > 0; i--) {
    c_ptr = &caveD[i][1];
    for (j = MAX_WIDTH - 2; j > 0; j--) {
      if ((c_ptr->fval == FLOOR_NULL) || (c_ptr->fval == TMP1_WALL) ||
          (c_ptr->fval == TMP2_WALL))
        c_ptr->fval = fval;
      c_ptr++;
    }
  }
}
static void delete_object(y, x) int y, x;
{
  register struct caveS* cave_ptr;
  cave_ptr = &caveD[y][x];
  obj_unuse(&entity_objD[cave_ptr->oidx]);
  cave_ptr->oidx = 0;
}
static void place_stair_tval_tchar(y, x, tval, tchar) int y, x, tval, tchar;
{
  register struct objS* obj;
  register struct caveS* cave_ptr;

  cave_ptr = &caveD[y][x];
  if (cave_ptr->oidx != 0) delete_object(y, x);
  obj = obj_use();
  // invcopy(&t_list[cur_pos], obj_up_stair);
  obj->tval = tval;
  obj->tchar = tchar;
  obj->subval = 1;
  obj->number = 1;

  cave_ptr->oidx = obj_index(obj);
}
static void place_stairs(typ, num, walls) int typ, num, walls;
{
  register struct caveS* cave_ptr;
  int i, j, flag;
  register int y1, x1, y2, x2;

  for (i = 0; i < num; i++) {
    flag = FALSE;
    do {
      j = 0;
      do {
        /* Note: don't let y1/x1 be zero, and don't let y2/x2 be equal
           to cur_height-1/cur_width-1, these values are always
           BOUNDARY_ROCK. */
        y1 = randint(MAX_HEIGHT - 14);
        x1 = randint(MAX_WIDTH - 14);
        y2 = y1 + 12;
        x2 = x1 + 12;
        do {
          do {
            cave_ptr = &caveD[y1][x1];
            // if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->oidx == 0) &&
            //     (next_to_walls(y1, x1) >= walls)) {
            if (cave_ptr->fval <= MAX_OPEN_SPACE) {
              flag = TRUE;
              if (typ == 1)
                place_stair_tval_tchar(y1, x1, TV_UP_STAIR, '<');
              else {
                place_stair_tval_tchar(y1, x1, TV_DOWN_STAIR, '>');
                log_usedD = snprintf(AP(logD), "%d,%d stairs", x1, y1);
                uD.x = x1;
                uD.y = y1;
              }
            }
            x1++;
          } while ((x1 != x2) && (!flag));
          x1 = x2 - 12;
          y1++;
        } while ((y1 != y2) && (!flag));
        j++;
      } while ((!flag) && (j <= 30));
      walls--;
    } while (!flag);
  }
}
int
next_to_corr(y, x)
register int y, x;
{
  register int k, j, i;
  register struct caveS* c_ptr;

  i = 0;
  for (j = y - 1; j <= (y + 1); j++)
    for (k = x - 1; k <= (x + 1); k++) {
      c_ptr = &caveD[j][k];
      /* should fail if there is already a door present */
      if (c_ptr->fval == FLOOR_CORR &&
          (c_ptr->oidx == 0 || entity_objD[c_ptr->oidx].tval < TV_MIN_DOORS))
        i++;
    }
  return (i);
}
static int
next_to(y, x)
register int y, x;
{
  register int next;

  if (next_to_corr(y, x) > 2)
    if ((caveD[y - 1][x].fval >= MIN_WALL) &&
        (caveD[y + 1][x].fval >= MIN_WALL))
      next = TRUE;
    else if ((caveD[y][x - 1].fval >= MIN_WALL) &&
             (caveD[y][x + 1].fval >= MIN_WALL))
      next = TRUE;
    else
      next = FALSE;
  else
    next = FALSE;
  return (next);
}
static void try_door(y, x) register int y, x;
{
  if ((caveD[y][x].fval == FLOOR_CORR) && (randint(100) > DUN_TUN_JCT) &&
      next_to(y, x))
    place_door(y, x);
}
void
cave_gen()
{
  int room_map[MAX_COL][MAX_ROW] = {0};
  register int i, j, k;
  int y1, x1, y2, x2, pick1, pick2, tmp;
  int16_t yloc[400], xloc[400];

  k = DUN_ROOM_MEAN;
  for (i = 0; i < k; i++)
    room_map[randint(MAX_ROW) - 1][randint(MAX_COL) - 1] = 1;
  k = 0;
  for (i = 0; i < MAX_ROW; i++)
    for (j = 0; j < MAX_COL; j++)
      if (room_map[i][j]) {
        yloc[k] = i * (SCREEN_HEIGHT >> 1) + (SCREEN_HEIGHT >> 2);
        xloc[k] = j * (SCREEN_WIDTH >> 1) + (SCREEN_WIDTH >> 2);
        build_room(yloc[k], xloc[k]);
        k++;
      }

  uD.x = xloc[0];
  uD.y = yloc[0];

  for (i = 0; i < k; i++) {
    pick1 = randint(k) - 1;
    pick2 = randint(k) - 1;
    y1 = yloc[pick1];
    x1 = xloc[pick1];
    yloc[pick1] = yloc[pick2];
    xloc[pick1] = xloc[pick2];
    yloc[pick2] = y1;
    xloc[pick2] = x1;
  }
  doorindex = 0;
  /* move zero entry to k, so that can call build_tunnel all k times */
  yloc[k] = yloc[0];
  xloc[k] = xloc[0];
  for (i = 0; i < k; i++) {
    y1 = yloc[i];
    x1 = xloc[i];
    y2 = yloc[i + 1];
    x2 = xloc[i + 1];
    build_tunnel(y2, x2, y1, x1);
  }

  fill_cave(GRANITE_WALL);
  // for (i = 0; i < DUN_STR_MAG; i++) place_streamer(MAGMA_WALL, DUN_STR_MC);
  // for (i = 0; i < DUN_STR_QUA; i++) place_streamer(QUARTZ_WALL, DUN_STR_QC);
  // place_boundary();
  /* Place intersection doors  */
  for (i = 0; i < doorindex; i++) {
    try_door(doorstk[i].y, doorstk[i].x - 1);
    try_door(doorstk[i].y, doorstk[i].x + 1);
    try_door(doorstk[i].y - 1, doorstk[i].x);
    try_door(doorstk[i].y + 1, doorstk[i].x);
  }
  place_stairs(2, randint(2) + 2, 3);
  place_stairs(1, randint(2), 3);
  /* Set up the character co-ords, used by alloc_monster, place_win_monster
   */
  // new_spot(&char_row, &char_col);

  // int alloc_level = (dun_level / 3);
  // if (alloc_level < 2)
  //   alloc_level = 2;
  // else if (alloc_level > 10)
  //   alloc_level = 10;
  // alloc_monster((randint(8) + MIN_MALLOC_LEVEL + alloc_level), 0, TRUE);
  // alloc_object(set_corr, 3, randint(alloc_level));
  // alloc_object(set_room, 5, randnor(TREAS_ROOM_ALLOC, 3));
  // alloc_object(set_floor, 5, randnor(TREAS_ANY_ALLOC, 3));
  // alloc_object(set_floor, 4, randnor(TREAS_GOLD_ALLOC, 3));
  // alloc_object(set_floor, 1, randint(alloc_level));
  // if (dun_level >= WIN_MON_APPEAR) place_win_monster();
}

char
get_sym(int row, int col)
{
  register struct caveS* cave_ptr;

  if (row == uD.y && col == uD.x) return '@';
  cave_ptr = &caveD[row][col];
  if (cave_ptr->oidx) {
    struct objS* obj = &entity_objD[cave_ptr->oidx];
    return obj->tchar;
  }
  switch (cave_ptr->fval) {
    case FLOOR_LIGHT:
      return '.';
    case FLOOR_DARK:
    case FLOOR_CORR:
      return ' ';
    case FLOOR_OBST:
      return ';';
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
    panel->panel_row = CLAMP(prow, 0, MAX_ROW);
  }

  bool xd = (x < panel->panel_col_min + 1 || x > panel->panel_col_max - 2);
  if (force || xd) {
    int pcol = (x - SCREEN_WIDTH / 4) / (SCREEN_WIDTH / 2);
    panel->panel_col = CLAMP(pcol, 0, MAX_COL);
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

void
dungeon()
{
  new_level_flag = FALSE;
  char c;
  do {
    buffer_usedD = 0;
    buffer_append(AP(tc_clearD));
    buffer_append(AP(tc_move_cursorD));
    if (log_usedD) {
      buffer_append(logD, log_usedD);
      log_usedD = 0;
    }
    buffer_append(AP(tc_crlfD));
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
      int print_len =
          snprintf(AP(line), "(%d,%d) xy (%d,%d) p (%d) fval %d feet\r\n", uD.x,
                   uD.y, panelD.panel_col, panelD.panel_row,
                   caveD[uD.y][uD.x].fval, dun_level * 50);
      if (print_len < AL(line)) buffer_append(line, print_len);
    }
    buffer_append(AP(tc_move_cursorD));
    write(STDOUT_FILENO, bufferD, buffer_usedD);

    int read_count = read(0, &c, 1);
    if (read_count == -1) {
      if (errno != EAGAIN) break;
      continue;
    }

    if (c == CTRL('c')) {
      death = 1;
      break;
    }
    if (c == CTRL('w')) {
      modeD = !modeD ? MODE_MAP : MODE_DFLT;
    }

    int x, y, x_max, y_max;
    if (modeD == MODE_DFLT) {
      x = uD.x;
      y = uD.y;
      x_max = MAX_WIDTH;
      y_max = MAX_HEIGHT;
    }
    if (modeD == MODE_MAP) {
      x = panelD.panel_col;
      y = panelD.panel_row;
      x_max = MAX_COL;
      y_max = MAX_ROW;
    }
    switch (c) {
      case 'k':
        y -= (y > 0);
        break;
      case 'j':
        y += (y + 1 < y_max);
        break;
      case 'l':
        x += (x + 1 < x_max);
        break;
      case 'h':
        x -= (x > 0);
        break;
      case 'n':
        x += (x + 1 < x_max);
        y += (y + 1 < y_max);
        break;
      case 'b':
        x -= (x > 0);
        y += (y + 1 < y_max);
        break;
      case 'y':
        x -= (x > 0);
        y -= (y > 0);
        break;
      case 'u':
        x += (x + 1 < x_max);
        y -= (y > 0);
        break;
      case '<':
        go_up();
        break;
      case '>':
        go_down();
        break;
    }
    if (modeD == MODE_DFLT) {
      if (caveD[y][x].fval < MIN_WALL) {
        uD.x = x;
        uD.y = y;
        panel_update(&panelD, uD.x, uD.y, false);
      }
    }
    if (modeD == MODE_MAP) {
      panelD.panel_row = y;
      panelD.panel_col = x;
      panel_bounds(&panelD);
    }
  } while (!new_level_flag);
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

  dun_level = 1;
  cave_gen();

  while (!death) {
    panel_update(&panelD, uD.x, uD.y, true);
    dungeon();

    if (!death) cave_gen();
  }

  write(1, tc_clearD, sizeof(tc_clearD));
  write(1, tc_show_cursorD, sizeof(tc_show_cursorD));

  ioctl(0, TCSETA, &save_termD);
  return 0;
}
