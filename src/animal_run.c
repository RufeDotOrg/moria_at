#include "game.c"

static int obj_teleport_idxD;
static int death;
static int dun_level;
static int free_turn_flag;
static int new_level_flag;
static char statusD[SCREEN_HEIGHT][STATUS_WIDTH];
static char symmapD[SCREEN_HEIGHT][SCREEN_WIDTH];
static char screenD[19][80];
static int screen_usedD[AL(screenD)];
static char overlayD[SCREEN_HEIGHT][80 - STATUS_WIDTH];
static int overlay_usedD[SCREEN_HEIGHT];
static char descD[160];
static char death_descD[160];
static char logD[80];
static int log_usedD;

ARR_REUSE(obj, 256);
ARR_REUSE(mon, 256);

#define MSG(x, ...)                                                   \
  {                                                                   \
    char vtype[80];                                                   \
    int len = snprintf(AP(vtype), "<%d>" x, __LINE__, ##__VA_ARGS__); \
    msg_print2(vtype, len);                                           \
  }

// Inventory of object IDs; obj_get(id)
// Zero is an available or empty slot
// [INVEN_WIELD, INVEN_AUX] are equipment
static int invenD[MAX_INVEN];

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
static void
draw()
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
    for (int row = 0; row < SCREEN_HEIGHT; ++row) {
      buffer_append(AP(tc_clear_lineD));
      buffer_append(AP(statusD[row]));
      buffer_append(overlayD[row], overlay_usedD[row]);
      buffer_append(AP(tc_crlfD));
    }
    AC(overlayD);
    AC(overlay_usedD);
  } else {
    for (int row = 0; row < SCREEN_HEIGHT; ++row) {
      buffer_append(AP(tc_clear_lineD));
      buffer_append(AP(statusD[row]));
      buffer_append(AP(symmapD[row]));
      buffer_append(AP(tc_crlfD));
    }
  }
  char line[80];
  int print_len =
      snprintf(AP(line), "(%d,%d) xy (%d,%d) quadrant (%d) fval %d feet", uD.x,
               uD.y, panelD.panel_col, panelD.panel_row, caveD[uD.y][uD.x].fval,
               dun_level * 50);
  if (print_len < AL(line)) buffer_append(line, print_len);
  buffer_append(AP(tc_move_cursorD));
  write(STDOUT_FILENO, bufferD, buffer_usedD);
}
static char
inkey()
{
  char c = platform_readansi();
  if (c == CTRL('c')) {
    death = 1;
    new_level_flag = TRUE;
    return -1;
  }
  return c;
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
void
in_wait()
{
  char c;
  do {
    c = inkey();
  } while (c != ' ');
}
char log_extD[] = " -more-";
static void msg_print2(msg, msglen) char* msg;
int msglen;
{
  int len = log_usedD;
  int maxlen = AL(logD) - AL(log_extD);

  // wait for user to acknowledge prior buffer -more-
  if (len + msglen >= maxlen) {
    log_usedD += snprintf(logD + len, AL(logD), "%s", log_extD);
    im_print();
    in_wait();
    len = 0;
  } else if (len) {
    logD[len] = ' ';
    len += 1;
  }

  len += snprintf(logD + len, AL(logD), "%s", msg);
  log_usedD = len;
}
static void
msg_print(char* text)
{
  msg_print2(text, strlen(text));
}
int in_subcommand(prompt, command) char* prompt;
char* command;
{
  if (log_usedD) im_print();
  log_usedD = snprintf(AP(logD), "%s", prompt);
  im_print();
  *command = inkey();
  log_usedD = 0;
  return (*command != ESCAPE);
}
static char map_roguedir(comval) register char comval;
{
  switch (comval) {
    case 'h':
      comval = '4';
      break;
    case 'y':
      comval = '7';
      break;
    case 'k':
      comval = '8';
      break;
    case 'u':
      comval = '9';
      break;
    case 'l':
      comval = '6';
      break;
    case 'n':
      comval = '3';
      break;
    case 'j':
      comval = '2';
      break;
    case 'b':
      comval = '1';
      break;
    case '.':
      comval = '5';
      break;
  }
  return (comval);
}
int get_dir(prompt, dir) char* prompt;
int* dir;
{
  char command;

  if (!prompt) prompt = "Which direction?";
  if (in_subcommand(prompt, &command)) {
    command = map_roguedir(command);
    if (command >= '1' && command <= '9' && command != '5') {
      *dir = command - '0';
      return TRUE;
    }
  }

  free_turn_flag = TRUE;
  return FALSE;
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
      BOOL wall = (row == 0 || row + 1 == MAX_HEIGHT) ||
                  (col == 0 || col + 1 == MAX_WIDTH);
      if (wall)
        caveD[row][col].fval = BOUNDARY_WALL;
      else
        caveD[row][col].fval = FLOOR_LIGHT;
    }
  }
  uD.x = uD.y = 1;
}
static void
place_boundary()
{
  for (int row = 0; row < MAX_HEIGHT; ++row)
    for (int col = 0; col < MAX_WIDTH; ++col)
      if ((row == 0 || row + 1 == MAX_HEIGHT) ||
          (col == 0 || col + 1 == MAX_WIDTH)) {
        caveD[row][col].fval = BOUNDARY_WALL;
      }
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

int randint(maxval) int maxval;
{
  register long randval;

  randval = rnd();
  return ((int)(randval % maxval) + 1);
}
#define MAX_SHORT 0xffff
int randnor(mean, stand) int mean, stand;
{
  register int tmp, offset, low, iindex, high;

  tmp = randint(MAX_SHORT);

  /* off scale, assign random value between 4 and 5 times SD */
  if (tmp == MAX_SHORT) {
    offset = 4 * stand + randint(stand);

    /* one half are negative */
    if (randint(2) == 1) offset = -offset;

    return mean + offset;
  }

  /* binary search normal normal_table to get index that matches tmp */
  /* this takes up to 8 iterations */
  low = 0;
  iindex = AL(normal_table) >> 1;
  high = AL(normal_table);
  while (TRUE) {
    if ((normal_table[iindex] == tmp) || (high == (low + 1))) break;
    if (normal_table[iindex] > tmp) {
      high = iindex;
      iindex = low + ((iindex - low) >> 1);
    } else {
      low = iindex;
      iindex = iindex + ((high - iindex) >> 1);
    }
  }

  /* might end up one below target, check that here */
  if (normal_table[iindex] < tmp) iindex = iindex + 1;

    /* normal_table is based on SD of 64, so adjust the index value here,
       round the half way case up */
#define NORMAL_TABLE_SD 64
  offset = ((stand * iindex) + (NORMAL_TABLE_SD >> 1)) / NORMAL_TABLE_SD;

  /* one half should be negative */
  if (randint(2) == 1) offset = -offset;

  return mean + offset;
}
int damroll(num, sides) int num, sides;
{
  register int i, sum = 0;

  for (i = 0; i < num; i++) sum += randint(sides);
  return (sum);
}
int pdamroll(array) uint8_t* array;
{
  return damroll(array[0], array[1]);
}
int critical_blow(weight, plus, dam) register int weight, plus, dam;
{
  register int critical;

  critical = dam;
  /* Weight of weapon, plusses to hit, and character level all      */
  /* contribute to the chance of a critical  		   */
  if (randint(5000) <= (int)(weight + 5 * plus)) {
    //  + (class_level_adj[py.misc.pclass][attack_type] * py.misc.lev)
    weight += randint(650);
    // 380 max itemization (you can wield iron chests apparently)
    // 280 for two-handed great flail (TV_HAFTED)
    // 300 for lance (TV_POLEARM)
    // 280 for zweihander (TV_SWORD)

    if (weight < 400) {
      critical = 2 * dam + 5;
      msg_print("It was a good hit! (x2 damage)");
    } else if (weight < 700) {
      critical = 3 * dam + 10;
      msg_print("It was an excellent hit! (x3 damage)");
    } else if (weight < 900) {
      critical = 4 * dam + 15;
      msg_print("It was a superb hit! (x4 damage)");
    } else {
      critical = 5 * dam + 20;
      msg_print("It was a *GREAT* hit! (x5 damage)");
    }
  }
  return (critical);
}
void disturb(search, light) int search, light;
{
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
BOOL
in_bounds(int row, int col)
{
  BOOL rc = (row > 0 && row < MAX_HEIGHT - 1);
  BOOL cc = (col > 0 && col < MAX_WIDTH - 1);
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
            //  if (next_to_walls(y1, x1) >= walls)
            if (cave_ptr->fval <= MAX_OPEN_SPACE && cave_ptr->oidx == 0) {
              flag = TRUE;
              if (typ == 1)
                place_stair_tval_tchar(y1, x1, TV_UP_STAIR, '<');
              else {
                place_stair_tval_tchar(y1, x1, TV_DOWN_STAIR, '>');
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
int next_to_corr(y, x) register int y, x;
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
static int next_to(y, x) register int y, x;
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
int distance(y1, x1, y2, x2) int y1, x1, y2, x2;
{
  register int dy, dx;

  dy = y1 - y2;
  if (dy < 0) dy = -dy;
  dx = x1 - x2;
  if (dx < 0) dx = -dx;

  return ((((dy + dx) << 1) - (dy > dx ? dx : dy)) >> 1);
}
static void try_door(y, x) register int y, x;
{
  if ((caveD[y][x].fval == FLOOR_CORR) && (randint(100) > DUN_TUN_JCT) &&
      next_to(y, x))
    place_door(y, x);
}
int set_room(element) register int element;
{
  return (element == FLOOR_DARK || element == FLOOR_LIGHT);
}
int set_corr(element) register int element;
{
  return (element == FLOOR_CORR || element == FLOOR_OBST);
}
int set_floor(element) int element;
{
  return (element <= MAX_FLOOR);
}
int set_large(item)         /* Items too large to fit in chests   -DJG- */
    struct treasureS* item; /* Use treasure_type since item not yet created */
{
  switch (item->tval) {
    case TV_CHEST:
    case TV_BOW:
    case TV_POLEARM:
    case TV_HARD_ARMOR:
    case TV_SOFT_ARMOR:
    case TV_STAFF:
      return TRUE;
    case TV_HAFTED:
    case TV_SWORD:
    case TV_DIGGING:
      if (item->weight > 150)
        return TRUE;
      else
        return FALSE;
  }
  return FALSE;
}
int slot_equip(tval) int tval;
{
  int slot = 0;
  switch (tval) {
    case TV_SLING_AMMO:
    case TV_BOLT:
    case TV_ARROW:
    case TV_BOW:
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
    case TV_DIGGING:
    case TV_SPIKE:
      slot = INVEN_WIELD;
      break;
    case TV_LIGHT:
      slot = INVEN_LIGHT;
      break;
    case TV_BOOTS:
      slot = INVEN_FEET;
      break;
    case TV_GLOVES:
      slot = INVEN_HANDS;
      break;
    case TV_CLOAK:
      slot = INVEN_OUTER;
      break;
    case TV_HELM:
      slot = INVEN_HEAD;
      break;
    case TV_SHIELD:
      slot = INVEN_ARM;
      break;
    case TV_HARD_ARMOR:
    case TV_SOFT_ARMOR:
      slot = INVEN_BODY;
      break;
    case TV_AMULET:
      slot = INVEN_NECK;
      break;
    case TV_RING:
      slot = INVEN_RING;
      break;
  }
  return slot;
}
int get_obj_num(level, must_be_small) int level, must_be_small;
{
  int i, j;

  if (level == 0)
    i = randint(o_level[0]);
  else {
    if (level >= MAX_OBJ_LEVEL)
      level = MAX_OBJ_LEVEL;
    else if (randint(OBJ_GREAT) == 1) {
      level = level * MAX_OBJ_LEVEL / randint(MAX_OBJ_LEVEL) + 1;
      if (level > MAX_OBJ_LEVEL) level = MAX_OBJ_LEVEL;
    }

    /* This code has been added to make it slightly more likely to get the
       higher level objects.  Originally a uniform distribution over all
       objects less than or equal to the dungeon level.  This distribution
       makes a level n objects occur approx 2/n% of the time on level n,
       and 1/2n are 0th level. */
    do {
      if (randint(2) == 1)
        i = randint(o_level[level]) - 1;
      else
      /* Choose three objects, pick the highest level. */
      {
        i = randint(o_level[level]) - 1;
        j = randint(o_level[level]) - 1;
        if (i < j) i = j;
        j = randint(o_level[level]) - 1;
        if (i < j) i = j;
        j = treasureD[sorted_objects[i]].level;
        if (j == 0)
          i = randint(o_level[0]) - 1;
        else
          i = randint(o_level[j] - o_level[j - 1]) - 1 + o_level[j - 1];
      }
    } while ((must_be_small) && (set_large(&treasureD[sorted_objects[i]])));
  }
  return (i);
}
int get_mon_num(level) int level;
{
  register int i, j, num;

  if (level <= 0)
    i = randint(m_level[0]);
  else {
    if (level > MAX_MON_LEVEL) level = MAX_MON_LEVEL;
    if (randint(MON_NASTY) == 1) {
      i = randnor(0, 4);
      level = level + ABS(i) + 1;
      if (level > MAX_MON_LEVEL) level = MAX_MON_LEVEL;
    } else {
      /* This code has been added to make it slightly more likely to
         get the higher level monsters. Originally a uniform
         distribution over all monsters of level less than or equal to the
         dungeon level. This distribution makes a level n monster occur
         approx 2/n% of the time on level n, and 1/n*n% are 1st level. */

      num = m_level[level] - m_level[0];
      i = randint(num);
      j = randint(num);
      if (j > i) i = j;
      level = creatureD[i + m_level[0]].level;
    }
    i = randint(m_level[level] - m_level[level - 1]) + m_level[level - 1];
  }
  return i;
}
int place_monster(y, x, z, slp) register int y, x, z;
int slp;
{
  struct monS* mon;
  struct creatureS* cre;

  cre = &creatureD[z];
  mon = mon_use();
  if (!mon) return FALSE;
  // TBD: duck type
  mon->fy = y;
  mon->fx = x;
  mon->cidx = z;
  if (cre->cdefense & CD_MAX_HP)
    mon->hp = cre->hd[0] * cre->hd[1];
  else
    mon->hp = pdamroll(cre->hd);
  mon->cdis = distance(uD.y, uD.x, y, x);

  caveD[y][x].midx = mon_index(mon);
  return TRUE;
}
void alloc_mon(num, dis, slp) int num, dis;
int slp;
{
  register int y, x, i;
  int z;

  for (i = 0; i < num; i++) {
    do {
      y = randint(MAX_HEIGHT - 2);
      x = randint(MAX_WIDTH - 2);
    } while (caveD[y][x].fval >= MIN_CLOSED_SPACE || (caveD[y][x].midx != 0) ||
             (distance(y, x, uD.y, uD.x) <= dis));

    z = get_mon_num(dun_level);
    /* Dragons are always created sleeping here, so as to give the player a
       sporting chance.  */
    // if (c_list[z].cchar == 'd' || c_list[z].cchar == 'D') slp = TRUE;
    /* Place_monster() should always return TRUE here.  It does not
       matter if it fails though.  */
    place_monster(y, x, z, slp);
  }
}

#define MAX_GOLD 18
static int goldD[MAX_GOLD] = {
    3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 24, 28, 32, 40, 80,
};
void place_gold(y, x) int y, x;
{
  register int i, cur_pos;
  struct objS* obj;

  obj = obj_use();
  caveD[y][x].oidx = obj_index(obj);

  i = ((randint(dun_level + 2) + 2) / 2) - 1;
  if (randint(OBJ_GREAT) == 1) i += randint(dun_level + 1);
  if (i >= MAX_GOLD) i = MAX_GOLD - 1;

  // invcopy(&t_list[cur_pos], OBJ_GOLD_LIST + i);
  obj->tval = TV_GOLD;
  obj->tchar = '$';
  obj->cost = goldD[i];
  obj->subval = i;
  obj->number = 1;
  obj->level = 1;

  obj->cost += (8 * randint(obj->cost)) + randint(8);
  if (uD.y == y && uD.x == x)
    msg_print("You feel something roll beneath your feet.");
}
void place_object(y, x, must_be_small) int y, x, must_be_small;
{
  struct objS* obj;

  obj = obj_use();
  caveD[y][x].oidx = obj_index(obj);

  int sn = get_obj_num(dun_level, must_be_small);
  int z = sorted_objects[sn];
  struct treasureS* treasure = &treasureD[z];

  // invcopy(&t_list[cur_pos], z);
  // TBD: duck type
  obj->flags = treasure->flags;
  obj->tidx = z;
  obj->tval = treasure->tval;
  obj->tchar = treasure->tchar;
  obj->subval = treasure->subval;
  obj->number = treasure->number;
  obj->weight = treasure->weight;
  obj->p1 = treasure->p1;
  obj->cost = treasure->cost;
  memcpy(obj->damage, treasure->damage, sizeof(obj->damage));
  obj->level = treasure->level;

  // TBD: the following is important for projectile count and special assignment
  // magic_treasure(cur_pos, dun_level);

  if (uD.y == y && uD.x == x)
    msg_print("You feel something roll beneath your feet.");
}
void place_trap(y, x, subval) int y, x, subval;
{
  struct objS* obj = obj_use();

  caveD[y][x].oidx = obj_index(obj);
  // invcopy(&t_list[cur_pos], OBJ_TRAP_LIST + subval);
  obj->tval = TV_INVIS_TRAP;
  obj->tchar = '%';
  obj->subval = subval;
  obj->number = 1;
  obj->p1 = 2;
  obj->damage[0] = 1;
  obj->damage[1] = 8;
  obj->level = 50;
}
void alloc_obj(alloc_set, typ, num) int (*alloc_set)();
int typ, num;
{
  for (int it = 0; it < num; it++) {
    int y, x;
    do {
      y = randint(MAX_HEIGHT) - 1;
      x = randint(MAX_WIDTH) - 1;
    }
    /* don't put an object beneath the player, this could cause problems
       if player is standing under rubble, or on a trap */
    while ((!(*alloc_set)(caveD[y][x].fval)) || (caveD[y][x].oidx != 0) ||
           (y == uD.y && x == uD.x));
    switch (typ) {
      case 1:
        place_trap(y, x, randint(MAX_TRAP));
        break;
      case 2:  // unused
      case 3:
        // place_rubble(y, x);
        break;
      case 4:
        place_gold(y, x);
        break;
      case 5:
        place_object(y, x, FALSE);
        break;
    }
  }
}
void
cave_gen()
{
  int room_map[MAX_COL - 1][MAX_ROW - 1] = {0};
  register int i, j, k;
  int y1, x1, y2, x2, pick1, pick2, tmp;
  int16_t yloc[400], xloc[400];

  k = randnor(DUN_ROOM_MEAN, 2);
  for (i = 0; i < k; i++)
    room_map[randint(AL(room_map)) - 1][randint(AL(room_map[0])) - 1] = 1;
  k = 0;
  for (i = 0; i < AL(room_map); i++)
    for (j = 0; j < AL(room_map[0]); j++)
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
  // for (i = 0; i < DUN_STR_MAG; i++) place_streamer(MAGMA_WALL,
  // DUN_STR_MC); for (i = 0; i < DUN_STR_QUA; i++)
  // place_streamer(QUARTZ_WALL, DUN_STR_QC);
  place_boundary();
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

  int alloc_level = CLAMP(dun_level / 3, 2, 10);
  alloc_mon((randint(8) + MIN_MALLOC_LEVEL + alloc_level), 0, TRUE);
  // alloc_obj(set_corr, 3, randint(alloc_level));
  alloc_obj(set_room, 5, randnor(TREAS_ROOM_MEAN, 3));
  // alloc_obj(set_floor, 5, randnor(TREAS_ANY_ALLOC, 3));
  alloc_obj(set_floor, 4, randnor(TREAS_GOLD_ALLOC, 3));
  alloc_obj(set_floor, 1, randint(alloc_level));
  // if (dun_level >= WIN_MON_APPEAR) place_win_monster();
}
void
generate_cave()
{
  memset(caveD, 0, sizeof(caveD));
  obj_usedD = 0;
  memset(entity_objD, 0, sizeof(entity_objD));
  mon_usedD = 0;
  memset(entity_monD, 0, sizeof(entity_monD));
  cave_gen();
}

char
get_sym(int row, int col)
{
  register struct caveS* cave_ptr;

  if (row == uD.y && col == uD.x) return '@';
  cave_ptr = &caveD[row][col];
  if (cave_ptr->midx) {
    struct monS* mon = &entity_monD[cave_ptr->midx];
    struct creatureS* creature = &creatureD[mon->cidx];
    return creature->cchar;
  }
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
panel_update(struct panelS* panel, int y, int x, BOOL force)
{
  BOOL yd = (y < panel->panel_row_min + 2 || y > panel->panel_row_max - 3);
  if (force || yd) {
    int prow = (y - SCREEN_HEIGHT / 4) / (SCREEN_HEIGHT / 2);
    panel->panel_row = CLAMP(prow, 0, MAX_ROW);
  }

  BOOL xd = (x < panel->panel_col_min + 2 || x > panel->panel_col_max - 3);
  if (force || xd) {
    int pcol = (x - SCREEN_WIDTH / 4) / (SCREEN_WIDTH / 2);
    panel->panel_col = CLAMP(pcol, 0, MAX_COL - 2);
  }

  panel_bounds(panel);
}

static void get_moves(monptr, mm) int monptr;
register int* mm;
{
  int y, ay, x, ax, move_val;

  y = entity_monD[monptr].fy - uD.y;
  x = entity_monD[monptr].fx - uD.x;
  if (y < 0) {
    move_val = 8;
    ay = -y;
  } else {
    move_val = 0;
    ay = y;
  }
  if (x > 0) {
    move_val += 4;
    ax = x;
  } else
    ax = -x;
  /* this has the advantage of preventing the diamond maneuvre, also faster
   */
  if (ay > (ax << 1))
    move_val += 2;
  else if (ax > (ay << 1))
    move_val++;
  switch (move_val) {
    case 0:
      mm[0] = 9;
      if (ay > ax) {
        mm[1] = 8;
        mm[2] = 6;
        mm[3] = 7;
        mm[4] = 3;
      } else {
        mm[1] = 6;
        mm[2] = 8;
        mm[3] = 3;
        mm[4] = 7;
      }
      break;
    case 1:
    case 9:
      mm[0] = 6;
      if (y < 0) {
        mm[1] = 3;
        mm[2] = 9;
        mm[3] = 2;
        mm[4] = 8;
      } else {
        mm[1] = 9;
        mm[2] = 3;
        mm[3] = 8;
        mm[4] = 2;
      }
      break;
    case 2:
    case 6:
      mm[0] = 8;
      if (x < 0) {
        mm[1] = 9;
        mm[2] = 7;
        mm[3] = 6;
        mm[4] = 4;
      } else {
        mm[1] = 7;
        mm[2] = 9;
        mm[3] = 4;
        mm[4] = 6;
      }
      break;
    case 4:
      mm[0] = 7;
      if (ay > ax) {
        mm[1] = 8;
        mm[2] = 4;
        mm[3] = 9;
        mm[4] = 1;
      } else {
        mm[1] = 4;
        mm[2] = 8;
        mm[3] = 1;
        mm[4] = 9;
      }
      break;
    case 5:
    case 13:
      mm[0] = 4;
      if (y < 0) {
        mm[1] = 1;
        mm[2] = 7;
        mm[3] = 2;
        mm[4] = 8;
      } else {
        mm[1] = 7;
        mm[2] = 1;
        mm[3] = 8;
        mm[4] = 2;
      }
      break;
    case 8:
      mm[0] = 3;
      if (ay > ax) {
        mm[1] = 2;
        mm[2] = 6;
        mm[3] = 1;
        mm[4] = 9;
      } else {
        mm[1] = 6;
        mm[2] = 2;
        mm[3] = 9;
        mm[4] = 1;
      }
      break;
    case 10:
    case 14:
      mm[0] = 2;
      if (x < 0) {
        mm[1] = 3;
        mm[2] = 1;
        mm[3] = 6;
        mm[4] = 4;
      } else {
        mm[1] = 1;
        mm[2] = 3;
        mm[3] = 4;
        mm[4] = 6;
      }
      break;
    case 12:
      mm[0] = 1;
      if (ay > ax) {
        mm[1] = 2;
        mm[2] = 4;
        mm[3] = 3;
        mm[4] = 7;
      } else {
        mm[1] = 4;
        mm[2] = 2;
        mm[3] = 7;
        mm[4] = 3;
      }
      break;
  }
}
int mmove(dir, y, x) int dir;
register int *y, *x;
{
  register int new_row, new_col;
  int b;

  switch (dir) {
    case 1:
      new_row = *y + 1;
      new_col = *x - 1;
      break;
    case 2:
      new_row = *y + 1;
      new_col = *x;
      break;
    case 3:
      new_row = *y + 1;
      new_col = *x + 1;
      break;
    case 4:
      new_row = *y;
      new_col = *x - 1;
      break;
    case 5:
      new_row = *y;
      new_col = *x;
      break;
    case 6:
      new_row = *y;
      new_col = *x + 1;
      break;
    case 7:
      new_row = *y - 1;
      new_col = *x - 1;
      break;
    case 8:
      new_row = *y - 1;
      new_col = *x;
      break;
    case 9:
      new_row = *y - 1;
      new_col = *x + 1;
      break;
  }
  b = FALSE;
  if ((new_row >= 0) && (new_row < MAX_HEIGHT) && (new_col >= 0) &&
      (new_col < MAX_WIDTH)) {
    *y = new_row;
    *x = new_col;
    b = TRUE;
  }
  return (b);
}
void move_rec(y1, x1, y2, x2) register int y1, x1, y2, x2;
{
  int tmp = caveD[y1][x1].midx;
  caveD[y1][x1].midx = 0;
  caveD[y2][x2].midx = tmp;
}
void update_mon(monptr) int monptr;
{
}
int test_hit(bth, level, pth, ac) int bth, level, pth, ac;
{
  register int i, die;

  disturb(1, 0);
  i = bth + pth * BTH_PLUS_ADJ;
  // TBD:  + (level * class_level_adj[py.misc.pclass][attack_type]);

  // pth could be less than 0 if player wielding weapon too heavy for him
  // always miss 1 out of 20, always hit 1 out of 20
  die = randint(20);
  if ((die != 1) && ((die == 20) || ((i > 0) && (randint(i) > ac))))
    return TRUE;
  else
    return FALSE;
}
static void mon_death(y, x) int y, x;
{
  caveD[y][x].midx = 0;
}
static int mon_take_hit(midx, dam) int midx, dam;
{
  struct monS* mon = &entity_monD[midx];
  mon->hp -= dam;
  if (mon->hp >= 0) return -1;

  mon_death(mon->fy, mon->fx);
  // new_exp = ((long)c_ptr->mexp * c_ptr->level) / p_ptr->lev;
  uD.exp += 1;
  int cidx = mon->cidx;
  mon_unuse(mon);
  return cidx;
}
static void obj_desc(oidx) int oidx;
{
  struct objS* obj = &entity_objD[oidx];
  struct treasureS* treasure = &treasureD[obj->tidx];

  snprintf(AP(descD), "%s", treasure->name);
}
BOOL is_a_vowel(c) char c;
{
  switch (c) {
    case 'a':
    case 'e':
    case 'i':
    case 'o':
    case 'u':
      return TRUE;
  }
  return FALSE;
}
static void mon_desc(midx) int midx;
{
  struct monS* mon = &entity_monD[midx];
  struct creatureS* cre = &creatureD[mon->cidx];

  // if (!mon->ml)
  //  strcpy(cdesc, "It ");
  // else
  snprintf(AP(descD), "The %s", cre->name);

  // if (CM_WIN & cre->cmove)
  //  snprintf(AP(death_descD), "The %s", cre->name);
  if (is_a_vowel(cre->name[0]))
    snprintf(AP(death_descD), "An %s", cre->name);
  else
    snprintf(AP(death_descD), "A %s", cre->name);
}
void
py_init()
{
  uD.chp = 100;
  uD.mhp = 100;
  uD.lev = 1;

  int8_t stat[MAX_A];
  for (int it = 0; it < MAX_A; ++it) {
    stat[it] = 15;
  }
  memcpy(statD.max_stat, AP(stat));
  memcpy(statD.cur_stat, AP(stat));
  AC(statD.mod_stat);
  memcpy(statD.use_stat, AP(stat));

  // Test delta cur/max
  for (int it = 0; it < MAX_A; ++it) {
    statD.max_stat[it] += 1;
  }
}
int8_t modify_stat(stat, amount) int stat, amount;
{
  int loop, i;
  int8_t tmp_stat;

  tmp_stat = statD.cur_stat[stat];
  loop = (amount < 0 ? -amount : amount);
  for (i = 0; i < loop; i++) {
    if (amount > 0) {
      if (tmp_stat < 18)
        tmp_stat++;
      else if (tmp_stat < 108)
        tmp_stat += 10;
      else
        tmp_stat = 118;
    } else {
      if (tmp_stat > 27)
        tmp_stat -= 10;
      else if (tmp_stat > 18)
        tmp_stat = 18;
      else if (tmp_stat > 3)
        tmp_stat--;
    }
  }
  return tmp_stat;
}
void set_use_stat(stat) int stat;
{
  statD.use_stat[stat] = modify_stat(stat, statD.mod_stat[stat]);

  // if (stat == A_STR) {
  //   py.flags.status |= PY_STR_WGT;
  //   calc_bonuses();
  // } else if (stat == A_DEX)
  //   calc_bonuses();
  // else if (stat == A_INT) {
  //   if (class[py.misc.pclass].spell == MAGE) calc_spells(A_INT);
  //   calc_mana(A_INT);
  // } else if (stat == A_WIS) {
  //   if (class[py.misc.pclass].spell == PRIEST) calc_spells(A_WIS);
  //   calc_mana(A_WIS);
  // } else if (stat == A_CON)
  //   calc_hitpoints();
}
void py_bonuses(obj, factor) struct objS* obj;
int factor;
{
  int i, amount;

  amount = obj->p1 * factor;
  if (obj->flags & TR_STATS) {
    for (i = 0; i < 6; i++)
      if ((1 << i) & obj->flags) {
        statD.mod_stat[i] += amount;
        set_use_stat(i);
      }
  }
  // if (TR_SEARCH & obj->flags) {
  //  py.misc.srh += amount;
  //  py.misc.fos -= amount;
  //}
  // if (TR_STEALTH & obj->flags) py.misc.stl += amount;
  // if (TR_SPEED & obj->flags) change_speed(-amount);
  // if ((TR_BLIND & obj->flags) && (factor > 0)) py.flags.blind += 1000;
  // if ((TR_TIMID & obj->flags) && (factor > 0)) py.flags.afraid += 50;
  // if (TR_INFRA & obj->flags) py.flags.see_infra += amount;
}
static void
py_map()
{
  int y, x, dir;
  while (get_dir("Map: Look which direction?", &dir)) {
    mmove(dir, &panelD.panel_row, &panelD.panel_col);
    if (panelD.panel_row > MAX_ROW - 2) panelD.panel_row = MAX_ROW - 2;
    if (panelD.panel_col > MAX_COL - 2) panelD.panel_col = MAX_COL - 2;
    panel_bounds(&panelD);
    symmap_update();
    draw();
  }
  panel_update(&panelD, uD.y, uD.x, TRUE);
  free_turn_flag = TRUE;
}
static int py_inven(begin, end) int begin, end;
{
  int line = 0;

  for (int it = begin; it < end; ++it) {
    int obj_id = invenD[it];
    if (obj_id) {
      struct objS* obj = obj_get(obj_id);
      obj_desc(obj_index(obj));
      overlay_usedD[line] =
          snprintf(AP(overlayD[line]), "%c) %s", 'a' + it - begin, descD);
      line += 1;
    }
  }
  free_turn_flag = TRUE;
  return line;
}
void
py_wear()
{
  char c;
  struct objS* obj;

  int count = py_inven(0, INVEN_EQUIP);
  draw();
  if (count) {
    if (in_subcommand("Wear/Wield which item?", &c)) {
      int iidx = c - 'a';
      if (iidx < INVEN_EQUIP) {
        obj = obj_get(invenD[iidx]);
        int slot = slot_equip(obj->tval);
        int slot_count = (slot == INVEN_RING) ? 2 : 1;
        // TBD: Replace equipped item?
        for (int it = 0; it < slot_count; ++it) {
          if (invenD[slot + it] == 0) {
            invenD[slot] = obj->id;
            invenD[iidx] = 0;
            py_bonuses(obj, 1);
            MSG("You are wearing %s.", descD);
          }
        }
      }
    }
  }
}
static void
py_drop()
{
  char c, y, x;
  struct caveS* c_ptr;
  struct objS* obj;

  y = uD.y;
  x = uD.x;
  c_ptr = &caveD[y][x];
  int count = py_inven(0, INVEN_EQUIP);
  draw();
  if (c_ptr->oidx == 0 && count) {
    if (in_subcommand("Drop which item?", &c)) {
      int iidx = c - 'a';
      if (iidx < INVEN_EQUIP) {
        obj = obj_get(invenD[iidx]);
        c_ptr->oidx = obj_index(obj);
        invenD[iidx] = 0;
      }
    }
  }
}
static int
py_carry_count()
{
  int count = 0;
  for (int it = 0; it < INVEN_EQUIP; ++it) {
    count += (invenD[it] == 0);
  }
  return count;
}
static int inven_carry(obj_id) int obj_id;
{
  for (int it = 0; it < INVEN_EQUIP; ++it) {
    if (!invenD[it]) {
      invenD[it] = obj_id;
      return it;
    }
  }
  return -1;
}
char stat_nameD[MAX_A][5] = {
    "STR ", "INT ", "WIS ", "DEX ", "CON ", "CHR ",
};
void
py_screen()
{
  int line, col, col_width;
  col_width = 20;
  line = 0;
  col = 0;
#define PY_STAT(FMT, ...)                                                     \
  screen_usedD[line] = snprintf(screenD[line] + (col_width * col), col_width, \
                                FMT, ##__VA_ARGS__);                          \
  line += 1;
  for (int it = 0; it < MAX_A; ++it) {
    if (statD.use_stat[it] == statD.max_stat[it]) {
      PY_STAT("%s: %6d", stat_nameD[it], statD.use_stat[it]);
    } else {
      PY_STAT("%s: %6d %6d", stat_nameD[it], statD.use_stat[it],
              statD.max_stat[it]);
    }
  }
  free_turn_flag = TRUE;
}
void
py_takeoff()
{
  char c;
  struct objS* obj;

  int carry_count = py_carry_count();
  int equip_count = py_inven(INVEN_EQUIP, MAX_INVEN);
  draw();
  if (carry_count && equip_count) {
    if (in_subcommand("Take off which item?", &c)) {
      int iidx = INVEN_EQUIP + (c - 'a');
      if (iidx < MAX_INVEN) {
        obj = obj_get(invenD[iidx]);
        inven_carry(obj->id);
        invenD[iidx] = 0;

        obj_desc(obj_index(obj));
        py_bonuses(obj, -1);
        MSG("You take off %s.", descD);
      }
    }
  }
}
static void py_carry(y, x, pickup) int y, x;
int pickup;
{
  struct caveS* c_ptr;
  struct objS* obj;
  int locn;

  c_ptr = &caveD[y][x];
  obj_desc(c_ptr->oidx);
  obj = &entity_objD[c_ptr->oidx];

  /* There's GOLD in them thar hills!      */
  if (obj->tval == TV_GOLD) {
    uD.gold += obj->cost;
    MSG("You have found %d gold pieces worth of %s", obj->cost, descD);
    delete_object(y, x);
  } else {
    // TBD: Merge items of the same type?

    if (pickup) {
      if (py_carry_count()) {
        locn = inven_carry(obj->id);
        MSG("You pickup %s (%c)", descD, locn + 'a');
        caveD[y][x].oidx = 0;
      } else {
        MSG("You can't carry %s", descD);
      }
    } else {
      MSG("You see %s here.", descD);
    }
  }
}
static void py_take_hit(damage) int damage;
{
  uD.chp -= damage;
  if (uD.chp < 0) {
    death = TRUE;
    new_level_flag = TRUE;
  }
}
void
py_experience()
{
  int exp = uD.exp;
  int lev = uD.lev;
  int expfact = 100;

  if (exp > MAX_EXP) exp = MAX_EXP;

  while ((lev < MAX_PLAYER_LEVEL) &&
         (player_exp[lev - 1] * expfact / 100) <= exp) {
    int dif_exp, need_exp;

    lev += 1;
    MSG("Welcome to level %d.", lev);
    // calc_hitpoints();

    need_exp = player_exp[lev - 1] * expfact / 100;
    if (exp > need_exp) {
      /* lose some of the 'extra' exp when gaining several levels at once */
      dif_exp = exp - need_exp;
      exp = need_exp + (dif_exp / 2);
    }

    // TBD: spell/mana
    // c_ptr = &class[p_ptr->pclass];
    // if (c_ptr->spell == MAGE) {
    //   calc_spells(A_INT);
    //   calc_mana(A_INT);
    // } else if (c_ptr->spell == PRIEST) {
    //   calc_spells(A_WIS);
    //   calc_mana(A_WIS);
    // }
  }

  // if (p_ptr->exp > p_ptr->max_exp) p_ptr->max_exp = p_ptr->exp;

  // prt_long(p_ptr->exp, 14, STAT_COLUMN + 6);
  uD.exp = exp;
  uD.lev = lev;
}
void py_attack(y, x) int y, x;
{
  register int k, blows;
  int base_tohit;

  int midx = caveD[y][x].midx;
  struct monS* mon = &entity_monD[midx];
  struct objS* obj = obj_get(invenD[INVEN_WIELD]);

  mon_desc(midx);
  descD[0] = tolower(descD[0]);

  blows = 1;
  base_tohit = 30;

  int creature_ac = 0;
  /* Loop for number of blows,  trying to hit the critter.	  */
  for (int it = 0; it < blows; ++it) {
    if (test_hit(base_tohit, uD.lev, 0, creature_ac)) {
      MSG("You hit %s.", descD);
      if (obj->tval) {
        k = pdamroll(obj->damage);
        k = critical_blow(obj->weight, 0, k);
      } else {
        k = damroll(1, 1);
        k = critical_blow(1, 0, k);
      }
      // k += p_ptr->ptodam;
      if (k < 0) k = 0;

      /* See if we done it in.  			 */
      if (mon_take_hit(midx, k) >= 0) {
        MSG("You have slain %s.", descD);
        py_experience();
        blows = 0;
      }
    } else {
      MSG("You miss %s.", descD);
    }
  }
}
static void mon_attack(midx) int midx;
{
  struct monS* mon = &entity_monD[midx];
  struct creatureS* cre = &creatureD[mon->cidx];

  mon_desc(midx);
  int creature_level = 1;
  int adice = 1;
  int asides = 4;
  for (int it = 0; it < AL(cre->damage); ++it) {
    if (death) break;
    if (!cre->damage[it]) break;
    disturb(1, 0);
    int flag = FALSE;
    int tac = uD.ac + uD.toac;
    if (test_hit(60, creature_level, 0, tac)) flag = TRUE;
    if (flag) {
      MSG("%s hits you.", descD);
      int damage = damroll(adice, asides);
      damage -= (tac * damage) / 200;
      py_take_hit(damage);
    } else {
      MSG("%s misses you.", descD);
    }
  }
}
static void
close_object()
{
  int y, x, dir, no_object, valid_object;
  struct caveS* c_ptr;
  struct objS* obj;

  y = uD.y;
  x = uD.x;
  if (get_dir(0, &dir)) {
    mmove(dir, &y, &x);
    c_ptr = &caveD[y][x];
    obj = &entity_objD[c_ptr->oidx];

    no_object = (obj->id == 0);
    valid_object = obj->tval == TV_OPEN_DOOR;

    if (valid_object) {
      if (c_ptr->midx == 0) {
        if (obj->p1 == 0) {
          // invcopy(&t_list[c_ptr->tptr], OBJ_CLOSED_DOOR);
          obj->tval = TV_CLOSED_DOOR;
          obj->tchar = '+';
          c_ptr->fval = FLOOR_OBST;
          // lite_spot(y, x);
        } else
          msg_print("The door appears to be broken.");
      } else {
        msg_print("Something is in your way!");
      }
    }

    if (no_object) {
      msg_print("I do not see anything you can close there.");
      free_turn_flag = TRUE;
    }
  }
}
void disarm_trap(uy, ux) int *uy, *ux;
{
  int y, x, dir, valid, skill;
  struct caveS* c_ptr;
  struct objS* obj;

  y = uD.y;
  x = uD.x;
  if (get_dir(0, &dir)) {
    mmove(dir, &y, &x);
    c_ptr = &caveD[y][x];
    obj = &entity_objD[c_ptr->oidx];
    valid = (obj->tval == TV_VIS_TRAP);

    if (valid) {
      if (c_ptr->midx) {
        msg_print("Something is in your way!");
      } else if (obj->tval == TV_VIS_TRAP) {
        skill = randint(100) - obj->level;
        if (skill >= 0) {
          msg_print("You have disarmed the trap.");
          uD.exp += obj->p1;
          delete_object(y, x);
          *uy = y;
          *ux = x;
          py_experience();
        }
        /* avoid randint(0) call */
        else if (skill > -20)
          msg_print("You failed to disarm the trap.");
        else {
          msg_print("You set the trap off!");
          /* make sure we move onto the trap even if confused */
          // tmp = py.flags.confused;
          // py.flags.confused = 0;
          *uy = y;
          *ux = x;
          // py.flags.confused += tmp;
        }
      }
    } else {
      msg_print("I do not see anything to disarm there.");
      free_turn_flag = TRUE;
    }
  }
}
static void bash(uy, ux) int *uy, *ux;
{
  int y, x, dir, tmp;
  int valid_obj;
  struct caveS* c_ptr;
  struct objS* obj;

  y = uD.y;
  x = uD.x;
  if (get_dir(0, &dir)) {
    // if (py.flags.confused > 0) ...
    mmove(dir, &y, &x);
    c_ptr = &caveD[y][x];
    obj = &entity_objD[c_ptr->oidx];
    valid_obj = (obj->id != 0);

    if (c_ptr->midx) {
      // TBD: py_bash() vs monster
      py_attack(y, x);
    } else if (valid_obj) {
      if (obj->tval == TV_CLOSED_DOOR) {
        msg_print("You smash into the door!");
        tmp = 100;  // py.stats.use_stat[A_STR] + py.misc.wt / 2;
        /* Use (roughly) similar method as for monsters. */
        if (randint(tmp * (20 + ABS(obj->p1))) < 10 * (tmp - ABS(obj->p1))) {
          msg_print("The door crashes open!");
          // invcopy(&t_list[c_ptr->tptr], OBJ_OPEN_DOOR);
          obj->tval = TV_OPEN_DOOR;
          obj->tchar = '\'';
          obj->p1 = 1 - randint(2); /* 50% chance of breaking door */
          c_ptr->fval = FLOOR_CORR;
          // if (py.flags.confused == 0)
          *uy = y;
          *ux = x;
          // else
          //   lite_spot(y, x);
        } else if (randint(150) > 18) {  // py.stats.use_stat[A_DEX]) {
          msg_print("You are off-balance.");
          // py.flags.paralysis = 1 + randint(2);
        } else
          msg_print("The door holds firm.");
      } else
        /* Can't give free turn, or else player could try directions
           until he found invisible creature */
        msg_print("You bash it, but nothing interesting happens.");
    } else /* same message for wall as for secret door */
      msg_print("You bash it, but nothing interesting happens.");
  }
}
static void
open_object()
{
  int y, x, i, dir;
  int no_object, valid_object;
  struct caveS* c_ptr;
  struct objS* obj;

  y = uD.y;
  x = uD.x;
  if (get_dir(0, &dir)) {
    mmove(dir, &y, &x);
    c_ptr = &caveD[y][x];
    obj = &entity_objD[c_ptr->oidx];
    no_object = (obj->id == 0);
    valid_object = (obj->tval == TV_CLOSED_DOOR);

    if (valid_object) {
      // Monster may be invisible and will retaliate
      if (c_ptr->midx) {
        msg_print("Something is in your way!");
      } else if (obj->tval == TV_CLOSED_DOOR) {
        if (obj->p1 > 0) /* It's locked.  */
        {
          i = 20;
          // p_ptr = &py.misc;
          // i = p_ptr->disarm + 2 * todis_adj() + stat_adj(A_INT) +
          //     (class_level_adj[p_ptr->pclass][CLA_DISARM] * p_ptr->lev / 3);
          // Too much whiskey
          // if (py.flags.confused > 0)
          //   msg_print("You are too confused to pick the lock.");
          // else
          if ((i - obj->p1) > randint(100)) {
            msg_print("You have picked the lock.");
            uD.exp += 1;
            py_experience();
            obj->p1 = 0;
          } else
            msg_print("You failed to pick the lock.");
        } else if (obj->p1 < 0) /* It's stuck    */
          msg_print("It appears to be stuck.");
        if (obj->p1 == 0) {
          // invcopy(&t_list[c_ptr->tptr], OBJ_OPEN_DOOR);
          obj->tval = TV_OPEN_DOOR;
          obj->tchar = '\'';
          c_ptr->fval = FLOOR_CORR;
          // lite_spot(y, x);
          // command_count = 0;
        }
      }
    }

    if (no_object) {
      msg_print("I do not see anything you can open there.");
      free_turn_flag = TRUE;
    }
  }
}
static void search(y, x, chance) int y, x, chance;
{
  register int i, j;
  struct caveS* c_ptr;
  struct objS* obj;

  // p_ptr = &py.flags;
  // if (p_ptr->confused > 0) chance = chance / 10;
  // if ((p_ptr->blind > 0) || no_light()) chance = chance / 10;
  // if (p_ptr->image > 0) chance = chance / 10;
  for (i = (y - 1); i <= (y + 1); i++)
    for (j = (x - 1); j <= (x + 1); j++)
      if (randint(100) < chance) /* always in_bounds here */
      {
        c_ptr = &caveD[i][j];
        obj = &entity_objD[c_ptr->oidx];

        if (obj->tval == TV_SECRET_DOOR) {
          msg_print("You have found a secret door.");
          obj->tval = TV_CLOSED_DOOR;
          obj->tchar = '+';
          // lite_spot(y,x);
          // end_find();
        } else if (obj->tval == TV_INVIS_TRAP) {
          msg_print("You have found a trap.");
          obj->tval = TV_VIS_TRAP;
          obj->tchar = '^';
        }
      }
}
static void make_move(monptr, mm, rcmove) int monptr;
int* mm;
uint32_t* rcmove;
{
  int i, newy, newx, do_turn, do_move, stuck_door;
  register struct caveS* c_ptr;
  register struct monS* m_ptr;

  i = 0;
  do_turn = FALSE;
  do_move = FALSE;
  m_ptr = &entity_monD[monptr];
  for (int i = 0; i < 5; ++i) {
    /* Get new position  	*/
    newy = m_ptr->fy;
    newx = m_ptr->fx;
    mmove(mm[i], &newy, &newx);
    c_ptr = &caveD[newy][newx];
    if (c_ptr->fval == BOUNDARY_WALL) continue;

    /* Floor is open?  	   */
    if (c_ptr->fval <= MAX_OPEN_SPACE) do_move = TRUE;
    /* Creature has attempted to move on player?     */
    if (do_move) {
      if (newy == uD.y && newx == uD.x) {
        /* if the monster is not lit, must call update_mon, it may
           be faster than character, and hence could have just
           moved next to character this same turn */
        // TBD:
        // if (!m_ptr->ml) update_mon(monptr);
        mon_attack(monptr);
        do_move = FALSE;
        do_turn = TRUE;
      }
      /* Creature is attempting to move on other creature?     */
      else if ((c_ptr->midx > 1) &&
               ((newy != m_ptr->fy) || (newx != m_ptr->fx))) {
        do_move = FALSE;
      }
    }
    /* Creature has been allowed move.   */
    if (do_move) {
      /* Move creature record  	       */
      move_rec(m_ptr->fy, m_ptr->fx, newy, newx);
      // if (m_ptr->ml) {
      //   m_ptr->ml = FALSE;
      //   lite_spot((int)m_ptr->fy, (int)m_ptr->fx);
      // }
      m_ptr->fy = newy;
      m_ptr->fx = newx;
      m_ptr->cdis = distance(uD.y, uD.x, newy, newx);
      do_turn = TRUE;
    }
    if (do_turn) break;
  }
}
static void mon_move(monptr, rcmove) int monptr;
uint32_t* rcmove;
{
  int mm[9];
  get_moves(monptr, mm);
  make_move(monptr, mm, rcmove);
}
void creatures(move) int move;
{
  uint32_t rcmove;
  int move_count;

  FOR_EACH(mon, {
    mon->cdis = distance(uD.y, uD.x, mon->fy, mon->fx);
    if (move) mon_move(it_index, &rcmove);
    update_mon(it_index);
  });
}
void
status_update()
{
  memset(statusD, '    ', sizeof(statusD));
  int count, line;
  line = 0;
#define PR_STAT(ABBR, val)                                      \
  {                                                             \
    int count = snprintf(AP(statusD[line]), ABBR ": %6d", val); \
    statusD[line][count] = ' ';                                 \
    line += 1;                                                  \
  }
  PR_STAT("CHP ", uD.chp);
  PR_STAT("LEV ", uD.lev);
  PR_STAT("EXP ", uD.exp);
  line += 1;
  PR_STAT("GOLD", uD.gold);
}
void py_teleport_near(y, x, uy, ux) int y, x;
int *uy, *ux;
{
  for (int ro = y - 1; ro <= y + 1; ++ro) {
    for (int co = x - 1; co <= x + 1; ++co) {
      if ((caveD[ro][co].fval >= MIN_CLOSED_SPACE || caveD[ro][co].midx != 0)) {
        continue;
      }

      *uy = ro;
      *ux = co;
      MSG("Teleport near (%d, %d)", ro, co);
      return;
    }
  }
}
static void hit_trap(y, x) int y, x;
{
  int dam, tac;
  struct caveS* c_ptr;
  struct objS* obj;

  tac = uD.ac + uD.toac;
  // end_find();
  c_ptr = &caveD[y][x];
  obj = &entity_objD[c_ptr->oidx];
  if (obj->tval == TV_INVIS_TRAP) {
    obj->tval = TV_VIS_TRAP;
    obj->tchar = '^';
    // lite_spot(y, x);
  }
  dam = pdamroll(obj->damage);
  // TBD: dam may be conditional by trap subval
  py_take_hit(dam);
  switch (obj->subval) {
    case 1:
      msg_print("You fell into a covered pit");
      break;
    case 2:
      if (test_hit(125, 0, 0, tac)) {
        msg_print("An arrow hits you.");
      } else {
        msg_print("An arrow barely misses you.");
      }
      break;
    case 3:
      msg_print("You fell through a trap door!");
      new_level_flag = TRUE;
      dun_level += 1;
      break;
  }
}
void
dungeon()
{
  int c, y, x;
  new_level_flag = FALSE;
  obj_teleport_idxD = 0;
  while (1) {
    free_turn_flag = FALSE;
    status_update();
    symmap_update();

    draw();
    c = inkey();
    if (c == -1) break;

    x = uD.x;
    y = uD.y;
    switch (c) {
      case ' ':
        free_turn_flag = TRUE;
        break;
      case 'k':
      case 'j':
      case 'l':
      case 'h':
      case 'n':
      case 'b':
      case 'y':
      case 'u':
        mmove(map_roguedir(c) - '0', &y, &x);
        break;
      case 'c':
        close_object();
        break;
      case 'd':
        py_drop();
        break;
      case 'e': {
        int count = py_inven(INVEN_EQUIP, MAX_INVEN);
        MSG("You wearing %d items.", count);
      } break;
      case 'f':
        bash(&y, &x);
        break;
      case 'i': {
        int count = py_inven(0, INVEN_EQUIP);
        MSG("You carrying %d items.", count);
      } break;
      case 'o':
        open_object();
        break;
      case 's':
        search(y, x, 25);
        break;
      case 'w':
        py_wear();
        break;
      case '<':
        go_up();
        break;
      case '>':
        go_down();
        break;
      case 'C':
        py_screen();
        break;
      case 'D':
        disarm_trap(&y, &x);
        break;
      case 'T':
        py_takeoff();
        break;
      case 'W':
        py_map();
        break;
      case CTRL('h'):
        uD.chp = uD.mhp;
        break;
      case CTRL('t'):
        msg_print("teleport");
        do {
          x = randint(MAX_WIDTH - 2);
          y = randint(MAX_HEIGHT - 2);
        } while (caveD[y][x].fval >= MIN_CLOSED_SPACE || caveD[y][x].midx != 0);
        break;
      case CTRL('m'):
        if (mon_usedD) {
          int rv = randint(mon_usedD);
          struct monS* mon = mon_get(monD[rv - 1]);
          MSG("Teleport to monster id %d (%d/%d)", mon->id, rv, mon_usedD);

          int fy = mon->fy;
          int fx = mon->fx;
          py_teleport_near(fy, fx, &y, &x);
        }
        break;
      case CTRL('o'): {
        int seek_tval = obj_teleportD[obj_teleport_idxD];

        for (int row = 1; row < MAX_HEIGHT - 1; ++row) {
          for (int col = 1; col < MAX_WIDTH - 1; ++col) {
            int oidx = caveD[row][col].oidx;
            struct objS* obj = &entity_objD[oidx];
            if (obj->tval != seek_tval) continue;

            log_usedD =
                snprintf(AP(logD), "%s: %d oidx", "Teleport to object", oidx);
            py_teleport_near(row, col, &y, &x);
            row = MAX_HEIGHT;
            col = MAX_WIDTH;
          }
        }
        if (y == uD.y && x == uD.x) {
          obj_teleport_idxD = ((obj_teleport_idxD + 1) % AL(obj_teleportD));
          log_usedD =
              snprintf(AP(logD), "Switching object type %d", obj_teleport_idxD);
        }
      } break;
    }

    if (uD.y != y || uD.x != x) {
      struct caveS* c_ptr = &caveD[y][x];
      if (c_ptr->midx) {
        py_attack(y, x);
      } else if (c_ptr->fval <= MAX_OPEN_SPACE) {
        struct objS* obj = &entity_objD[c_ptr->oidx];
        uD.y = y;
        uD.x = x;
        panel_update(&panelD, uD.y, uD.x, FALSE);
        if (obj->tval) {
          if (obj->tval <= TV_MAX_PICK_UP) {
            py_carry(y, x, TRUE);
          } else if (obj->tval == TV_INVIS_TRAP || obj->tval == TV_VIS_TRAP) {
            hit_trap(y, x);
          }
        }
      }
    }

    if (new_level_flag) break;
    if (free_turn_flag) continue;
    creatures(TRUE);
  }
}
void
mon_level_init()
{
  int i, k;

  memset(m_level, 0, sizeof(m_level));

  k = AL(creatureD) - MAX_WIN_MON;
  for (i = 1; i < k; i++) m_level[creatureD[i].level]++;

  for (i = 1; i <= MAX_MON_LEVEL; i++) m_level[i] += m_level[i - 1];
}
void
obj_level_init()
{
  int i, l;
  int tmp[MAX_OBJ_LEVEL + 1];

  for (i = 0; i <= MAX_OBJ_LEVEL; i++) o_level[i] = 0;
  for (i = 1; i < AL(treasureD); i++) o_level[treasureD[i].level]++;
  for (i = 1; i <= MAX_OBJ_LEVEL; i++) o_level[i] += o_level[i - 1];

  /* now produce an array with object indexes sorted by level, by using
     the info in o_level, this is an O(n) sort! */
  /* this is not a stable sort, but that does not matter */
  for (i = 0; i <= MAX_OBJ_LEVEL; i++) tmp[i] = 1;
  for (i = 1; i < AL(treasureD); i++) {
    l = treasureD[i].level;
    sorted_objects[o_level[l] - tmp[l]] = i;
    tmp[l]++;
  }
}
int
main()
{
  platform_tty_init();

  dun_level = 1;
  mon_level_init();
  obj_level_init();
  generate_cave();
  py_init();

  while (!death) {
    panel_update(&panelD, uD.y, uD.x, TRUE);
    dungeon();

    if (!death) generate_cave();
  }

  platform_tty_reset();
  return 0;
}
