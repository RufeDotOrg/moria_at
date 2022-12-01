#include "game.c"

static int cycle[] = {1, 2, 3, 6, 9, 8, 7, 4, 1, 2, 3, 6, 9, 8, 7, 4, 1};
static int chome[] = {-1, 8, 9, 10, 7, -1, 11, 6, 5, 4};
static int find_cut = 1;
static int find_examine = 1;
static int find_ignore_doors;
static int find_direction;
static int find_flag;
static int find_openarea;
static int find_breakright, find_breakleft;
static int find_prevdir;

ARR_REUSE(obj, 256);
ARR_REUSE(mon, 256);

#define MSG(x, ...)                                             \
  {                                                             \
    char vtype[80];                                             \
    int len = snprintf(vtype, sizeof(vtype), x, ##__VA_ARGS__); \
    msg_print2(vtype, len);                                     \
  }
#define BufMsg(name, text, ...)                                         \
  {                                                                     \
    int used = name##_usedD[line];                                      \
    used += snprintf(name##D[line] + used, AL(name##D[0]) - used, text, \
                     ##__VA_ARGS__);                                    \
    name##_usedD[line++] = used;                                        \
  }
#define BufPad(name, line, len)            \
  for (int it = 0; it < line; ++it) {      \
    while (name##_usedD[it] < len) {       \
      name##D[it][name##_usedD[it]] = ' '; \
      name##_usedD[it] += 1;               \
    }                                      \
  }

// Inventory of object IDs; obj_get(id)
// Zero is an available or empty slot
// [INVEN_WIELD, INVEN_AUX] are equipment
static int invenD[MAX_INVEN];

static char
inkey()
{
  char c;
  do {
    c = platform_readansi();
  } while (c == 0);
  if (c == CTRL('c')) {
    death = 1;
    new_level_flag = TRUE;
    return -1;
  }
  return c;
}
static char log_extD[] = " -more-";
static int log_countD;
static void msg_print2(msg, msglen) char* msg;
int msglen;
{
  char c;
  int len = log_usedD;
  int maxlen = AL(logD) - AL(log_extD);

  // wait for user to acknowledge prior buffer -more-
  if (len + msglen >= maxlen) {
    log_usedD += snprintf(logD + len, AL(logD), "%s", log_extD);
    do {
      im_print();
      c = inkey();
    } while (c != ' ');
    len = 0;
  } else if (len) {
    logD[len] = ' ';
    len += 1;
  }

  if (!len) {
    log_countD += 1;
    len = snprintf(logD, AL(logD), "%d| ", log_countD);
  }
  len += snprintf(logD + len, AL(logD) - len, "%s", msg);
  log_usedD = len;
}
static void
msg_print(char* text)
{
  msg_print2(text, strlen(text));
}
int
in_subcommand(prompt, command)
char* prompt;
char* command;
{
  char c;
  log_usedD = snprintf(logD, AL(logD), "%s", prompt);
  do {
    im_print();
    c = inkey();
  } while (c == ' ');
  *command = c;
  log_usedD = 0;
  return (*command != ESCAPE);
}
static char
map_roguedir(comval)
register char comval;
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
int
get_dir(prompt, dir)
char* prompt;
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
int
bit_pos(test)
uint32_t* test;
{
  register int i;
  register uint32_t mask = 0x1;

  for (i = 0; i < sizeof(*test) * 8; i++) {
    if (*test & mask) {
      *test &= ~mask;
      return (i);
    }
    mask <<= 1;
  }

  /* no one bits found */
  return (-1);
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
int
cave_floor_near(y, x)
{
  for (int col = y - 1; col <= y + 1; ++col) {
    for (int row = x - 1; row <= x + 1; ++row) {
      if (caveD[col][row].fval <= MAX_FLOOR) return TRUE;
    }
  }
  return FALSE;
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

int
randint(maxval)
int maxval;
{
  register long randval;

  randval = rnd();
  return ((int)(randval % maxval) + 1);
}
int
randnor(mean, stand)
int mean, stand;
{
  register int offset, low, iindex, high;
  int16_t tmp;

  tmp = randint(INT16_MAX);

  /* binary search normal normal_table to get index that matches tmp */
  /* this takes up to 8 iterations */
  low = 0;
  iindex = AL(normal_table) >> 1;
  high = AL(normal_table);
  while (TRUE) {
    if (high <= low + 1) break;
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

  /* off scale, boost random value between 4 and 5 times SD */
  if (tmp == INT16_MAX) offset += randint(stand);

  /* one half should be negative */
  if (randint(2) == 1) offset = -offset;

  return mean + offset;
}
int
damroll(num, sides)
int num, sides;
{
  register int i, sum = 0;

  for (i = 0; i < num; i++) sum += randint(sides);
  return (sum);
}
int
pdamroll(array)
uint8_t* array;
{
  return damroll(array[0], array[1]);
}
int
critical_blow(weight, plus, dam)
register int weight, plus, dam;
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
  if (uD.rest != 0) uD.rest = 0;
  find_flag = FALSE;
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

  y_height = yval - randint(ROOM_HEIGHT / 2);
  y_depth = yval + randint(ROOM_HEIGHT / 2) - 1;
  x_left = xval - randint(ROOM_WIDTH / 2);
  x_right = xval + randint(ROOM_WIDTH / 2);

  for (i = y_height; i <= y_depth; i++) {
    c_ptr = &caveD[i][x_left];
    for (j = x_left; j <= x_right; j++) {
      c_ptr->fval = floor;
      c_ptr->cflag |= CF_ROOM;
      c_ptr++;
    }
  }

  for (i = (y_height - 1); i <= (y_depth + 1); i++) {
    c_ptr = &caveD[i][x_left - 1];
    c_ptr->fval = GRANITE_WALL;
    c_ptr->cflag |= CF_ROOM;
    c_ptr = &caveD[i][x_right + 1];
    c_ptr->fval = GRANITE_WALL;
    c_ptr->cflag |= CF_ROOM;
  }

  c_ptr = &caveD[y_height - 1][x_left];
  d_ptr = &caveD[y_depth + 1][x_left];
  for (i = x_left; i <= x_right; i++) {
    c_ptr->fval = GRANITE_WALL;
    c_ptr->cflag |= CF_ROOM;
    c_ptr++;
    d_ptr->fval = GRANITE_WALL;
    d_ptr->cflag |= CF_ROOM;
    d_ptr++;
  }
}
BOOL
near_light(y, x)
int y, x;
{
  for (int row = y - 1; row <= y + 1; ++row) {
    for (int col = x - 1; col <= x + 1; ++col) {
      if (caveD[row][col].fval == FLOOR_LIGHT) return TRUE;
    }
  }
  return FALSE;
}
void light_room(y, x) int y, x;
{
  register int i, j, start_col, end_col;
  int tmp1, tmp2, start_row, end_row;
  struct caveS* c_ptr;
  int tval;

  tmp1 = (SYMMAP_HEIGHT / 2);
  tmp2 = (SYMMAP_WIDTH / 2);
  start_row = (y / tmp1) * tmp1;
  start_col = (x / tmp2) * tmp2;
  end_row = start_row + tmp1 - 1;
  end_col = start_col + tmp2 - 1;
  for (i = start_row; i <= end_row; i++)
    for (j = start_col; j <= end_col; j++) {
      c_ptr = &caveD[i][j];
      if ((c_ptr->cflag & CF_ROOM)) {
        c_ptr->cflag |= CF_PERM_LIGHT;
        if (c_ptr->fval == FLOOR_DARK) c_ptr->fval = FLOOR_LIGHT;
        if (c_ptr->oidx) {
          struct objS* obj = &entity_objD[c_ptr->oidx];
          if (obj->tval >= TV_MIN_VISIBLE && obj->tval <= TV_MAX_VISIBLE) {
            c_ptr->cflag |= CF_FIELDMARK;
          }
        }
      }
    }
}
int
light_area(y, x)
{
  if (caveD[y][x].cflag & CF_ROOM) light_room(y, x);
  for (int col = y - 1; col <= y + 1; ++col) {
    for (int row = x - 1; x <= x + 1; ++row) {
      caveD[col][row].cflag |= CF_PERM_LIGHT;
    }
  }

  // TBD: unknown when blind...  (py.flags.blind)
  return TRUE;
}
void
unlight_room(y, x)
{
  int i, j, start_col, end_col;
  int tmp1, tmp2, start_row, end_row;
  struct caveS* c_ptr;
  int tval;

  tmp1 = (SYMMAP_HEIGHT / 2);
  tmp2 = (SYMMAP_WIDTH / 2);
  start_row = (y / tmp1) * tmp1;
  start_col = (x / tmp2) * tmp2;
  end_row = start_row + tmp1 - 1;
  end_col = start_col + tmp2 - 1;
  for (i = start_row; i <= end_row; i++)
    for (j = start_col; j <= end_col; j++) {
      c_ptr = &caveD[i][j];
      if ((c_ptr->cflag & CF_ROOM)) {
        c_ptr->cflag &= ~CF_PERM_LIGHT;
        if (c_ptr->fval == FLOOR_LIGHT) c_ptr->fval = FLOOR_DARK;
        if (c_ptr->oidx) {
          struct objS* obj = &entity_objD[c_ptr->oidx];
          if (obj->tval >= TV_MIN_VISIBLE && obj->tval <= TV_MAX_VISIBLE) {
            c_ptr->cflag &= ~CF_FIELDMARK;
          }
        }
      }
    }
}
int
unlight_area(y, x)
{
  if (caveD[y][x].cflag & CF_ROOM) unlight_room(y, x);
  for (int col = y - 1; col <= y + 1; ++col) {
    for (int row = x - 1; x <= x + 1; ++row) {
      caveD[col][row].cflag &= ~CF_PERM_LIGHT;
    }
  }

  // TBD: unknown when blind.. (py.flags.blind)
  return TRUE;
}
typedef struct {
  int y;
  int x;
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
  obj->fy = y;
  obj->fx = x;
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
  obj->fy = y;
  obj->fx = x;
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
  obj->fy = y;
  obj->fx = x;
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
  cave_ptr->cflag &= ~CF_FIELDMARK;
}
char*
describe_use(iidx)
{
  char* p;

  switch (iidx) {
    case INVEN_WIELD:
      p = "wielding";
      break;
    case INVEN_HEAD:
      p = "wearing on your head";
      break;
    case INVEN_NECK:
      p = "wearing around your neck";
      break;
    case INVEN_BODY:
      p = "wearing on your body";
      break;
    case INVEN_ARM:
      p = "wearing on your arm";
      break;
    case INVEN_HANDS:
      p = "wearing on your hands";
      break;
    case INVEN_RIGHT:
      p = "wearing on your right hand";
      break;
    case INVEN_LEFT:
      p = "wearing on your left hand";
      break;
    case INVEN_FEET:
      p = "wearing on your feet";
      break;
    case INVEN_OUTER:
      p = "wearing about your body";
      break;
    case INVEN_LIGHT:
      p = "using to light the way";
      break;
    case INVEN_AUX:
      p = "holding ready by your side";
      break;
    default:
      p = "carrying in your pack";
      break;
  }
  return p;
}
static void inven_destroy_one(iidx) int iidx;
{
  struct objS* obj = obj_get(invenD[iidx]);
  if (obj->number > 1) {
    obj->number -= 1;
  } else {
    obj_unuse(obj);
    invenD[iidx] = 0;
  }
}
static void place_stair_tval_tchar(y, x, tval, tchar) int y, x, tval, tchar;
{
  register struct objS* obj;
  register struct caveS* cave_ptr;

  cave_ptr = &caveD[y][x];
  if (cave_ptr->oidx != 0) delete_object(y, x);
  obj = obj_use();
  obj->fy = y;
  obj->fx = x;
  // invcopy(&t_list[cur_pos], obj_up_stair);
  obj->tval = tval;
  obj->tchar = tchar;
  obj->subval = 1;
  obj->number = 1;

  cave_ptr->oidx = obj_index(obj);
}
static void new_spot(y, x) int *y, *x;
{
  register int i, j;
  register struct caveS* c_ptr;

  do {
    i = randint(MAX_HEIGHT - 2);
    j = randint(MAX_WIDTH - 2);
    c_ptr = &caveD[i][j];
  } while (c_ptr->fval >= MIN_CLOSED_SPACE || (c_ptr->oidx != 0) ||
           (c_ptr->midx != 0));
  *y = i;
  *x = j;
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
int
distance(y1, x1, y2, x2)
int y1, x1, y2, x2;
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
int
set_null()
{
  return FALSE;
}
int
set_room(element)
register int element;
{
  return (element == FLOOR_DARK || element == FLOOR_LIGHT);
}
int
set_corr(element)
register int element;
{
  return (element == FLOOR_CORR || element == FLOOR_OBST);
}
int
set_floor(element)
int element;
{
  return (element <= MAX_FLOOR);
}
void tr_obj_copy(tidx, obj) int tidx;
struct objS* obj;
{
  struct treasureS* tr_ptr = &treasureD[tidx];
  obj->name2 = 0;
  obj->flags = tr_ptr->flags;
  obj->fy = 0;
  obj->fx = 0;
  obj->tval = tr_ptr->tval;
  obj->tchar = tr_ptr->tchar;
  obj->tidx = tidx;
  obj->p1 = tr_ptr->p1;
  obj->cost = tr_ptr->cost;
  obj->subval = tr_ptr->subval;
  obj->number = tr_ptr->number;
  obj->weight = tr_ptr->weight;
  obj->tohit = tr_ptr->tohit;
  obj->todam = tr_ptr->todam;
  obj->ac = tr_ptr->ac;
  obj->toac = tr_ptr->toac;
  memcpy(obj->damage, tr_ptr->damage, sizeof(obj->damage));
  obj->level = tr_ptr->level;
  obj->idflag = 0;
}
int
magik(chance)
{
  return (randint(100) <= chance);
}
int
m_bonus(base, max_std, level)
{
  int x, stand_dev, tmp;

  stand_dev = (OBJ_STD_ADJ * level / 100) + OBJ_STD_MIN;
  /* Check for level > max_std since that may have generated an overflow.  */
  if (stand_dev > max_std || level > max_std) stand_dev = max_std;
  tmp = randnor(0, stand_dev);
  x = (ABS(tmp) / 10) + base;
  return (x);
}
#define SN(x) special_nameD[x]
void magic_treasure(obj, level) struct objS* obj;
{
  int chance, special, cursed, i;
  int tmp;

  chance = OBJ_BASE_MAGIC + level;
  if (chance > OBJ_BASE_MAX) chance = OBJ_BASE_MAX;
  special = chance / OBJ_DIV_SPECIAL;
  cursed = (10 * chance) / OBJ_DIV_CURSED;

  /* Depending on treasure type, it can have certain magical properties*/
  switch (obj->tval) {
    case TV_SHIELD:
    case TV_HARD_ARMOR:
    case TV_SOFT_ARMOR:
      if (magik(chance)) {
        obj->toac += m_bonus(1, 30, level);
        if (magik(special)) switch (randint(9)) {
            case 1:
              obj->flags |=
                  (TR_RES_LIGHT | TR_RES_COLD | TR_RES_ACID | TR_RES_FIRE);
              obj->name2 = SN(SN_R);
              obj->toac += 5;
              obj->cost += 2500;
              break;
            case 2: /* Resist Acid    */
              obj->flags |= TR_RES_ACID;
              obj->name2 = SN(SN_RA);
              obj->cost += 1000;
              break;
            case 3:
            case 4: /* Resist Fire    */
              obj->flags |= TR_RES_FIRE;
              obj->name2 = SN(SN_RF);
              obj->cost += 600;
              break;
            case 5:
            case 6: /* Resist Cold   */
              obj->flags |= TR_RES_COLD;
              obj->name2 = SN(SN_RC);
              obj->cost += 600;
              break;
            case 7:
            case 8:
            case 9: /* Resist Lightning*/
              obj->flags |= TR_RES_LIGHT;
              obj->name2 = SN(SN_RL);
              obj->cost += 500;
              break;
          }
      } else if (magik(cursed)) {
        obj->toac -= m_bonus(1, 40, level);
        obj->cost = 0;
        obj->flags |= TR_CURSED;
      }
      break;

    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
      if (magik(chance)) {
        obj->tohit += m_bonus(0, 40, level);
        /* Magical damage bonus now proportional to weapon base damage */
        tmp = obj->damage[0] * obj->damage[1];
        obj->todam += m_bonus(0, 4 * tmp, tmp * level / 10);
        /* the 3*special/2 is needed because weapons are not as common as
           before change to treasure distribution, this helps keep same
           number of ego weapons same as before, see also missiles */
        if (magik(3 * special / 2)) switch (randint(16)) {
            case 1: /* Holy Avenger   */
              obj->flags |= (TR_SEE_INVIS | TR_SUST_STAT | TR_SLAY_UNDEAD |
                             TR_SLAY_EVIL | TR_STR);
              obj->tohit += 5;
              obj->todam += 5;
              obj->toac += randint(4);
              /* the value in p1 is used for strength increase */
              /* p1 is also used for sustain stat */
              obj->p1 = randint(4);
              obj->name2 = SN(SN_HA);
              obj->cost += obj->p1 * 500;
              obj->cost += 10000;
              break;
            case 2: /* Defender   */
              obj->flags |= (TR_FFALL | TR_RES_LIGHT | TR_SEE_INVIS |
                             TR_FREE_ACT | TR_RES_COLD | TR_RES_ACID |
                             TR_RES_FIRE | TR_REGEN | TR_STEALTH);
              obj->tohit += 3;
              obj->todam += 3;
              obj->toac += 5 + randint(5);
              obj->name2 = SN(SN_DF);
              /* the value in p1 is used for stealth */
              obj->p1 = randint(3);
              obj->cost += obj->p1 * 500;
              obj->cost += 7500;
              break;
            case 3:
            case 4: /* Slay Animal  */
              obj->flags |= TR_SLAY_ANIMAL;
              obj->tohit += 2;
              obj->todam += 2;
              obj->name2 = SN(SN_SA);
              obj->cost += 3000;
              break;
            case 5:
            case 6: /* Slay Dragon   */
              obj->flags |= TR_SLAY_DRAGON;
              obj->tohit += 3;
              obj->todam += 3;
              obj->name2 = SN(SN_SD);
              obj->cost += 4000;
              break;
            case 7:
            case 8: /* Slay Evil     */
              obj->flags |= TR_SLAY_EVIL;
              obj->tohit += 3;
              obj->todam += 3;
              obj->name2 = SN(SN_SE);
              obj->cost += 4000;
              break;
            case 9:
            case 10: /* Slay Undead    */
              obj->flags |= (TR_SEE_INVIS | TR_SLAY_UNDEAD);
              obj->tohit += 3;
              obj->todam += 3;
              obj->name2 = SN(SN_SU);
              obj->cost += 5000;
              break;
            case 11:
            case 12:
            case 13: /* Flame Tongue  */
              obj->flags |= TR_FLAME_TONGUE;
              obj->tohit++;
              obj->todam += 3;
              obj->name2 = SN(SN_FT);
              obj->cost += 2000;
              break;
            case 14:
            case 15:
            case 16: /* Frost Brand   */
              obj->flags |= TR_FROST_BRAND;
              obj->tohit++;
              obj->todam++;
              obj->name2 = SN(SN_FB);
              obj->cost += 1200;
              break;
          }
      } else if (magik(cursed)) {
        obj->tohit -= m_bonus(1, 55, level);
        /* Magical damage bonus now proportional to weapon base damage */
        tmp = obj->damage[0] * obj->damage[1];
        obj->todam -= m_bonus(1, 11 * tmp / 2, tmp * level / 10);
        obj->flags |= TR_CURSED;
        obj->cost = 0;
      }
      break;

    case TV_BOW:
      if (magik(chance)) {
        obj->tohit += m_bonus(1, 30, level);
        obj->todam += m_bonus(1, 20, level); /* add damage. -CJS- */
      } else if (magik(cursed)) {
        obj->tohit -= m_bonus(1, 50, level);
        obj->todam -= m_bonus(1, 30, level); /* add damage. -CJS- */
        obj->flags |= TR_CURSED;
        obj->cost = 0;
      }
      break;

    case TV_DIGGING:
      if (magik(chance)) {
        tmp = randint(3);
        if (tmp < 3)
          obj->p1 += m_bonus(0, 25, level);
        else {
          /* a cursed digging tool */
          obj->p1 = -m_bonus(1, 30, level);
          obj->cost = 0;
          obj->flags |= TR_CURSED;
        }
      }
      break;

    case TV_GLOVES:
      if (magik(chance)) {
        obj->toac += m_bonus(1, 20, level);
        if (magik(special)) {
          if (randint(2) == 1) {
            obj->flags |= TR_FREE_ACT;
            obj->name2 = SN(SN_FREE_ACTION);
            obj->cost += 1000;
          } else {
            obj->tohit += 1 + randint(3);
            obj->todam += 1 + randint(3);
            obj->name2 = SN(SN_SLAYING);
            obj->cost += (obj->tohit + obj->todam) * 250;
          }
        }
      } else if (magik(cursed)) {
        if (magik(special)) {
          if (randint(2) == 1) {
            obj->flags |= TR_DEX;
            obj->name2 = SN(SN_CLUMSINESS);
          } else {
            obj->flags |= TR_STR;
            obj->name2 = SN(SN_WEAKNESS);
          }
          obj->p1 = -m_bonus(1, 10, level);
        }
        obj->toac -= m_bonus(1, 40, level);
        obj->flags |= TR_CURSED;
        obj->cost = 0;
      }
      break;

    case TV_BOOTS:
      if (magik(chance)) {
        obj->toac += m_bonus(1, 20, level);
        if (magik(special)) {
          tmp = randint(12);
          if (tmp > 5) {
            obj->flags |= TR_FFALL;
            obj->name2 = SN(SN_SLOW_DESCENT);
            obj->cost += 250;
          } else if (tmp == 1) {
            obj->flags |= TR_SPEED;
            obj->name2 = SN(SN_SPEED);
            obj->p1 = 1;
            obj->cost += 5000;
          } else /* 2 - 5 */
          {
            obj->flags |= TR_STEALTH;
            obj->p1 = randint(3);
            obj->name2 = SN(SN_STEALTH);
            obj->cost += 500;
          }
        }
      } else if (magik(cursed)) {
        tmp = randint(3);
        if (tmp == 1) {
          obj->flags |= TR_SPEED;
          obj->name2 = SN(SN_SLOWNESS);
          obj->p1 = -1;
        } else if (tmp == 2) {
          obj->flags |= TR_AGGRAVATE;
          obj->name2 = SN(SN_NOISE);
        } else {
          obj->name2 = SN(SN_GREAT_MASS);
          obj->weight = obj->weight * 5;
        }
        obj->cost = 0;
        obj->toac -= m_bonus(2, 45, level);
        obj->flags |= TR_CURSED;
      }
      break;

    case TV_HELM: /* Helms */
      if ((obj->subval >= 6) && (obj->subval <= 8)) {
        /* give crowns a higher chance for magic */
        chance += (int)(obj->cost / 100);
        special += special;
      }
      if (magik(chance)) {
        obj->toac += m_bonus(1, 20, level);
        if (magik(special)) {
          if (obj->subval < 6) {
            tmp = randint(3);
            if (tmp == 1) {
              obj->p1 = randint(2);
              obj->flags |= TR_INT;
              obj->name2 = SN(SN_INTELLIGENCE);
              obj->cost += obj->p1 * 500;
            } else if (tmp == 2) {
              obj->p1 = randint(2);
              obj->flags |= TR_WIS;
              obj->name2 = SN(SN_WISDOM);
              obj->cost += obj->p1 * 500;
            } else {
              obj->p1 = 1 + randint(4);
              obj->flags |= TR_INFRA;
              obj->name2 = SN(SN_INFRAVISION);
              obj->cost += obj->p1 * 250;
            }
          } else {
            switch (randint(6)) {
              case 1:
                obj->p1 = randint(3);
                obj->flags |= (TR_FREE_ACT | TR_CON | TR_DEX | TR_STR);
                obj->name2 = SN(SN_MIGHT);
                obj->cost += 1000 + obj->p1 * 500;
                break;
              case 2:
                obj->p1 = randint(3);
                obj->flags |= (TR_CHR | TR_WIS);
                obj->name2 = SN(SN_LORDLINESS);
                obj->cost += 1000 + obj->p1 * 500;
                break;
              case 3:
                obj->p1 = randint(3);
                obj->flags |= (TR_RES_LIGHT | TR_RES_COLD | TR_RES_ACID |
                               TR_RES_FIRE | TR_INT);
                obj->name2 = SN(SN_MAGI);
                obj->cost += 3000 + obj->p1 * 500;
                break;
              case 4:
                obj->p1 = randint(3);
                obj->flags |= TR_CHR;
                obj->name2 = SN(SN_BEAUTY);
                obj->cost += 750;
                break;
              case 5:
                obj->p1 = 5 * (1 + randint(4));
                obj->flags |= (TR_SEE_INVIS | TR_SEARCH);
                obj->name2 = SN(SN_SEEING);
                obj->cost += 1000 + obj->p1 * 100;
                break;
              case 6:
                obj->flags |= TR_REGEN;
                obj->name2 = SN(SN_REGENERATION);
                obj->cost += 1500;
                break;
            }
          }
        }
      } else if (magik(cursed)) {
        obj->toac -= m_bonus(1, 45, level);
        obj->flags |= TR_CURSED;
        obj->cost = 0;
        if (magik(special)) switch (randint(7)) {
            case 1:
              obj->p1 = -randint(5);
              obj->flags |= TR_INT;
              obj->name2 = SN(SN_STUPIDITY);
              break;
            case 2:
              obj->p1 = -randint(5);
              obj->flags |= TR_WIS;
              obj->name2 = SN(SN_DULLNESS);
              break;
            case 3:
              obj->flags |= TR_BLIND;
              obj->name2 = SN(SN_BLINDNESS);
              break;
            case 4:
              obj->flags |= TR_TIMID;
              obj->name2 = SN(SN_TIMIDNESS);
              break;
            case 5:
              obj->p1 = -randint(5);
              obj->flags |= TR_STR;
              obj->name2 = SN(SN_WEAKNESS);
              break;
            case 6:
              obj->flags |= TR_TELEPORT;
              obj->name2 = SN(SN_TELEPORTATION);
              break;
            case 7:
              obj->p1 = -randint(5);
              obj->flags |= TR_CHR;
              obj->name2 = SN(SN_UGLINESS);
              break;
          }
      }
      break;

    case TV_RING: /* Rings        */
      switch (obj->subval) {
        case 0:
        case 1:
        case 2:
        case 3:
          if (magik(cursed)) {
            obj->p1 = -m_bonus(1, 20, level);
#ifdef ATARIST_MWC
            obj->flags |= TR_CURSED;
#else
            obj->flags |= TR_CURSED;
#endif
            obj->cost = -obj->cost;
          } else {
            obj->p1 = m_bonus(1, 10, level);
            obj->cost += obj->p1 * 100;
          }
          break;
        case 4:
          if (magik(cursed)) {
            obj->p1 = -randint(3);
            obj->flags |= TR_CURSED;
            obj->cost = -obj->cost;
          } else
            obj->p1 = 1;
          break;
        case 5:
          obj->p1 = 5 * m_bonus(1, 20, level);
          obj->cost += obj->p1 * 50;
          if (magik(cursed)) {
            obj->p1 = -obj->p1;
            obj->flags |= TR_CURSED;
            obj->cost = -obj->cost;
          }
          break;
        case 19: /* Increase damage        */
          obj->todam += m_bonus(1, 20, level);
          obj->cost += obj->todam * 100;
          if (magik(cursed)) {
            obj->todam = -obj->todam;
            obj->flags |= TR_CURSED;
            obj->cost = -obj->cost;
          }
          break;
        case 20: /* Increase To-Hit        */
          obj->tohit += m_bonus(1, 20, level);
          obj->cost += obj->tohit * 100;
          if (magik(cursed)) {
            obj->tohit = -obj->tohit;
            obj->flags |= TR_CURSED;
            obj->cost = -obj->cost;
          }
          break;
        case 21: /* Protection        */
          obj->toac += m_bonus(1, 20, level);
          obj->cost += obj->toac * 100;
          if (magik(cursed)) {
            obj->toac = -obj->toac;
            obj->flags |= TR_CURSED;
            obj->cost = -obj->cost;
          }
          break;
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
          break;
        case 30: /* Slaying        */
          obj->todam += m_bonus(1, 25, level);
          obj->tohit += m_bonus(1, 25, level);
          obj->cost += (obj->tohit + obj->todam) * 100;
          if (magik(cursed)) {
            obj->tohit = -obj->tohit;
            obj->todam = -obj->todam;
            obj->flags |= TR_CURSED;
            obj->cost = -obj->cost;
          }
          break;
        default:
          break;
      }
      break;

    case TV_AMULET: /* Amulets        */
      if (obj->subval < 2) {
        if (magik(cursed)) {
          obj->p1 = -m_bonus(1, 20, level);
          obj->flags |= TR_CURSED;
          obj->cost = -obj->cost;
        } else {
          obj->p1 = m_bonus(1, 10, level);
          obj->cost += obj->p1 * 100;
        }
      } else if (obj->subval == 2) {
        obj->p1 = 5 * m_bonus(1, 25, level);
        if (magik(cursed)) {
          obj->p1 = -obj->p1;
          obj->cost = -obj->cost;
          obj->flags |= TR_CURSED;
        } else
          obj->cost += 50 * obj->p1;
      } else if (obj->subval == 8) {
        /* amulet of the magi is never cursed */
        obj->p1 = 5 * m_bonus(1, 25, level);
        obj->cost += 20 * obj->p1;
      }
      break;

    case TV_LIGHT:
      obj->p1 = randint(obj->p1);
      break;

    case TV_WAND:
      switch (obj->subval) {
        case 0:
          obj->p1 = randint(10) + 6;
          break;
        case 1:
          obj->p1 = randint(8) + 6;
          break;
        case 2:
          obj->p1 = randint(5) + 6;
          break;
        case 3:
          obj->p1 = randint(8) + 6;
          break;
        case 4:
          obj->p1 = randint(4) + 3;
          break;
        case 5:
          obj->p1 = randint(8) + 6;
          break;
        case 6:
          obj->p1 = randint(20) + 12;
          break;
        case 7:
          obj->p1 = randint(20) + 12;
          break;
        case 8:
          obj->p1 = randint(10) + 6;
          break;
        case 9:
          obj->p1 = randint(12) + 6;
          break;
        case 10:
          obj->p1 = randint(10) + 12;
          break;
        case 11:
          obj->p1 = randint(3) + 3;
          break;
        case 12:
          obj->p1 = randint(8) + 6;
          break;
        case 13:
          obj->p1 = randint(10) + 6;
          break;
        case 14:
          obj->p1 = randint(5) + 3;
          break;
        case 15:
          obj->p1 = randint(5) + 3;
          break;
        case 16:
          obj->p1 = randint(5) + 6;
          break;
        case 17:
          obj->p1 = randint(5) + 4;
          break;
        case 18:
          obj->p1 = randint(8) + 4;
          break;
        case 19:
          obj->p1 = randint(6) + 2;
          break;
        case 20:
          obj->p1 = randint(4) + 2;
          break;
        case 21:
          obj->p1 = randint(8) + 6;
          break;
        case 22:
          obj->p1 = randint(5) + 2;
          break;
        case 23:
          obj->p1 = randint(12) + 12;
          break;
        default:
          break;
      }
      break;

    case TV_STAFF:
      switch (obj->subval) {
        case 0:
          obj->p1 = randint(20) + 12;
          break;
        case 1:
          obj->p1 = randint(8) + 6;
          break;
        case 2:
          obj->p1 = randint(5) + 6;
          break;
        case 3:
          obj->p1 = randint(20) + 12;
          break;
        case 4:
          obj->p1 = randint(15) + 6;
          break;
        case 5:
          obj->p1 = randint(4) + 5;
          break;
        case 6:
          obj->p1 = randint(5) + 3;
          break;
        case 7:
          obj->p1 = randint(3) + 1;
          obj->level = 10;
          break;
        case 8:
          obj->p1 = randint(3) + 1;
          break;
        case 9:
          obj->p1 = randint(5) + 6;
          break;
        case 10:
          obj->p1 = randint(10) + 12;
          break;
        case 11:
          obj->p1 = randint(5) + 6;
          break;
        case 12:
          obj->p1 = randint(5) + 6;
          break;
        case 13:
          obj->p1 = randint(5) + 6;
          break;
        case 14:
          obj->p1 = randint(10) + 12;
          break;
        case 15:
          obj->p1 = randint(3) + 4;
          break;
        case 16:
          obj->p1 = randint(5) + 6;
          break;
        case 17:
          obj->p1 = randint(5) + 6;
          break;
        case 18:
          obj->p1 = randint(3) + 4;
          break;
        case 19:
          obj->p1 = randint(10) + 12;
          break;
        case 20:
          obj->p1 = randint(3) + 4;
          break;
        case 21:
          obj->p1 = randint(3) + 4;
          break;
        case 22:
          obj->p1 = randint(10) + 6;
          obj->level = 5;
          break;
        default:
          break;
      }
      break;

    case TV_CLOAK:
      if (magik(chance)) {
        if (magik(special)) {
          if (randint(2) == 1) {
            obj->name2 = SN(SN_PROTECTION);
            obj->toac += m_bonus(2, 40, level);
            obj->cost += 250;
          } else {
            obj->toac += m_bonus(1, 20, level);
            obj->p1 = randint(3);
            obj->flags |= TR_STEALTH;
            obj->name2 = SN(SN_STEALTH);
            obj->cost += 500;
          }
        } else
          obj->toac += m_bonus(1, 20, level);
      } else if (magik(cursed)) {
        tmp = randint(3);
        if (tmp == 1) {
          obj->flags |= TR_AGGRAVATE;
          obj->name2 = SN(SN_IRRITATION);
          obj->toac -= m_bonus(1, 10, level);
          obj->tohit -= m_bonus(1, 10, level);
          obj->todam -= m_bonus(1, 10, level);
          obj->cost = 0;
        } else if (tmp == 2) {
          obj->name2 = SN(SN_VULNERABILITY);
          obj->toac -= m_bonus(10, 100, level + 50);
          obj->cost = 0;
        } else {
          obj->name2 = SN(SN_ENVELOPING);
          obj->toac -= m_bonus(1, 10, level);
          obj->tohit -= m_bonus(2, 40, level + 10);
          obj->todam -= m_bonus(2, 40, level + 10);
          obj->cost = 0;
        }
        obj->flags |= TR_CURSED;
      }
      break;

      // case TV_CHEST:
      //   switch (randint(level + 4)) {
      //     case 1:
      //       obj->flags = 0;
      //       obj->name2 = SN(SN_EMPTY);
      //       break;
      //     case 2:
      //       obj->flags |= CH_LOCKED;
      //       obj->name2 = SN(SN_LOCKED);
      //       break;
      //     case 3:
      //     case 4:
      //       obj->flags |= (CH_LOSE_STR | CH_LOCKED);
      //       obj->name2 = SN(SN_POISON_NEEDLE);
      //       break;
      //     case 5:
      //     case 6:
      //       obj->flags |= (CH_POISON | CH_LOCKED);
      //       obj->name2 = SN(SN_POISON_NEEDLE);
      //       break;
      //     case 7:
      //     case 8:
      //     case 9:
      //       obj->flags |= (CH_PARALYSED | CH_LOCKED);
      //       obj->name2 = SN(SN_GAS_TRAP);
      //       break;
      //     case 10:
      //     case 11:
      //       obj->flags |= (CH_EXPLODE | CH_LOCKED);
      //       obj->name2 = SN(SN_EXPLOSION_DEVICE);
      //       break;
      //     case 12:
      //     case 13:
      //     case 14:
      //       obj->flags |= (CH_SUMMON | CH_LOCKED);
      //       obj->name2 = SN(SN_SUMMONING_RUNES);
      //       break;
      //     case 15:
      //     case 16:
      //     case 17:
      //       obj->flags |= (CH_PARALYSED | CH_POISON | CH_LOSE_STR |
      //       CH_LOCKED); obj->name2 = SN(SN_MULTIPLE_TRAPS); break;
      //     default:
      //       obj->flags |= (CH_SUMMON | CH_EXPLODE | CH_LOCKED);
      //       obj->name2 = SN(SN_MULTIPLE_TRAPS);
      //       break;
      //   }
      //   break;

    case TV_SLING_AMMO:
    case TV_SPIKE:
    case TV_BOLT:
    case TV_ARROW:
      if (obj->tval == TV_SLING_AMMO || obj->tval == TV_BOLT ||
          obj->tval == TV_ARROW) {
        if (magik(chance)) {
          obj->tohit += m_bonus(1, 35, level);
          obj->todam += m_bonus(1, 35, level);
          /* see comment for weapons */
          if (magik(3 * special / 2)) switch (randint(10)) {
              case 1:
              case 2:
              case 3:
                obj->name2 = SN(SN_SLAYING);
                obj->tohit += 5;
                obj->todam += 5;
                obj->cost += 20;
                break;
              case 4:
              case 5:
                obj->flags |= TR_FLAME_TONGUE;
                obj->tohit += 2;
                obj->todam += 4;
                obj->name2 = SN(SN_FIRE);
                obj->cost += 25;
                break;
              case 6:
              case 7:
                obj->flags |= TR_SLAY_EVIL;
                obj->tohit += 3;
                obj->todam += 3;
                obj->name2 = SN(SN_SLAY_EVIL);
                obj->cost += 25;
                break;
              case 8:
              case 9:
                obj->flags |= TR_SLAY_ANIMAL;
                obj->tohit += 2;
                obj->todam += 2;
                obj->name2 = SN(SN_SLAY_ANIMAL);
                obj->cost += 30;
                break;
              case 10:
                obj->flags |= TR_SLAY_DRAGON;
                obj->tohit += 3;
                obj->todam += 3;
                obj->name2 = SN(SN_DRAGON_SLAYING);
                obj->cost += 35;
                break;
            }
        } else if (magik(cursed)) {
          obj->tohit -= m_bonus(5, 55, level);
          obj->todam -= m_bonus(5, 55, level);
          obj->flags |= TR_CURSED;
          obj->cost = 0;
        }
      }

      obj->number = 0;
      for (i = 0; i < 7; i++) obj->number += randint(6);
      break;

    default:
      break;
  }
}
int
tr_subval(tr_ptr)
struct treasureS* tr_ptr;
{
  return MASK_SUBVAL & tr_ptr->subval;
}
// Known refers to stackable treasures that are instanced
// Distinct from identification which is PER object
static uint8_t knownD[7][MAX_SUBVAL];
int
tr_known_row(tr_ptr)
struct treasureS* tr_ptr;
{
  switch (tr_ptr->tval) {
    case TV_AMULET:
      return (0);
    case TV_RING:
      return (1);
    case TV_STAFF:
      return (2);
    case TV_WAND:
      return (3);
    case TV_SCROLL1:
    case TV_SCROLL2:
      return (4);
    case TV_POTION1:
    case TV_POTION2:
      return (5);
    case TV_FOOD:
      if (tr_subval(tr_ptr) < AL(mushrooms)) return (6);
      return (-1);
    default:
      return (-1);
  }
}
BOOL
tr_known(tr_ptr)
struct treasureS* tr_ptr;
{
  int krow = tr_known_row(tr_ptr);
  if (krow < 0) return TRUE;
  int subval = tr_subval(tr_ptr);
  return knownD[krow][subval];
}
BOOL
tr_make_known(tr_ptr)
struct treasureS* tr_ptr;
{
  int krow = tr_known_row(tr_ptr);
  int subval = tr_subval(tr_ptr);
  if (krow < 0) return FALSE;
  BOOL change = 1 ^ knownD[krow][subval];
  knownD[krow][subval] = 1;
  return change;
}
BOOL
obj_reveal(obj)
struct objS* obj;
{
  return ((obj->idflag & ID_REVEAL) != 0);
}
BOOL
vuln_fire(obj)
struct objS* obj;
{
  switch (obj->tval) {
    case TV_ARROW:
    case TV_BOW:
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_BOOTS:
    case TV_GLOVES:
    case TV_CLOAK:
    case TV_SOFT_ARMOR:
      /* Items of (RF) should not be destroyed.  */
      if (obj->flags & TR_RES_FIRE)
        return FALSE;
      else
        return TRUE;

    case TV_STAFF:
    case TV_SCROLL1:
    case TV_SCROLL2:
      return TRUE;
  }
  return (FALSE);
}
BOOL
vuln_acid(obj)
struct objS* obj;
{
  switch (obj->tval) {
    case TV_MISC:
    case TV_CHEST:
      return TRUE;
    case TV_BOLT:
    case TV_ARROW:
    case TV_BOW:
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_BOOTS:
    case TV_GLOVES:
    case TV_CLOAK:
    case TV_SOFT_ARMOR:
      if (obj->flags & TR_RES_ACID)
        return (FALSE);
      else
        return (TRUE);
  }
  return (FALSE);
}
BOOL
vuln_frost(obj)
struct objS* obj;
{
  return ((obj->tval == TV_POTION1) || (obj->tval == TV_POTION2) ||
          (obj->tval == TV_FLASK));
}
BOOL
vuln_lightning(obj)
struct objS* obj;
{
  return ((obj->tval == TV_RING) || (obj->tval == TV_WAND) ||
          (obj->tval == TV_SPIKE));
}
BOOL
vuln_gas(obj)
struct objS* obj;
{
  // DESIGN: (R) armor is destroyed by gas. Ego weapons too. Yeesh.
  switch (obj->tval) {
    case TV_SWORD:
    case TV_HELM:
    case TV_SHIELD:
    case TV_HARD_ARMOR:
    case TV_WAND:
      return (TRUE);
  }
  return (FALSE);
}
BOOL
is_door(tval)
int tval;
{
  switch (tval) {
    case TV_OPEN_DOOR:
    case TV_CLOSED_DOOR:
    case TV_SECRET_DOOR:
      return TRUE;
  }
  return FALSE;
}
int
oset_always_visible(obj)
struct objS* obj;
{
  return (obj->tval >= TV_MIN_VISIBLE && obj->tval <= TV_MAX_VISIBLE);
}
int
oset_zap(obj)
struct objS* obj;
{
  switch (obj->tval) {
    case TV_WAND:
    case TV_STAFF:
      return TRUE;
  }
  return FALSE;
}
int
oset_hitdam(obj)
struct objS* obj;
{
  switch (obj->tval) {
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
    case TV_BOW:
    case TV_DIGGING:
      return TRUE;
    case TV_GLOVES:
      return (obj->tohit || obj->todam);
    case TV_RING:
      return (obj->tohit && obj->todam);
  }
  return FALSE;
}
int
oset_missile(obj)
struct objS* obj;
{
  switch (obj->tval) {
    case TV_SLING_AMMO:
    case TV_SPIKE:
    case TV_BOLT:
    case TV_ARROW:
      return TRUE;
  }
  return FALSE;
}
int
oset_actuate(obj)
struct objS* obj;
{
  switch (obj->tval) {
    case TV_FOOD:
    case TV_POTION1:
    case TV_POTION2:
    case TV_SCROLL1:
    case TV_SCROLL2:
    case TV_STAFF:
      return TRUE;
  }
  return FALSE;
}
int
set_large(item)         /* Items too large to fit in chests   -DJG- */
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
int
slot_equip(tval)
int tval;
{
  int slot = -1;
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
void
map_area()
{
  struct caveS* c_ptr;
  int row, col;
  int i, j, k, l;

  // TBD: tuning
  i = panelD.panel_row_min - randint(10);
  j = panelD.panel_row_max + randint(10);
  k = panelD.panel_col_min - randint(20);
  l = panelD.panel_col_max + randint(20);
  for (col = i; col <= j; col++)
    for (row = k; row <= l; row++) {
      c_ptr = &caveD[col][row];
      if (in_bounds(col, row) && cave_floor_near(col, row)) {
        if (c_ptr->fval >= MIN_WALL)
          c_ptr->cflag |= CF_PERM_LIGHT;
        else if ((c_ptr->oidx != 0) &&
                 oset_always_visible(obj_get(c_ptr->oidx)))
          c_ptr->cflag |= CF_FIELDMARK;
      }
    }
}
int
get_obj_num(level, must_be_small)
int level, must_be_small;
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
int
get_mon_num(level)
int level;
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
int
place_monster(y, x, z, slp)
register int y, x, z;
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
  mon->ml = FALSE;

  caveD[y][x].midx = mon_index(mon);
  return TRUE;
}
int
summon_monster(y, x, slp)
{
  int i, j, k;
  int l, summon;
  struct caveS* cave_ptr;

  i = 0;
  summon = FALSE;
  l = get_mon_num(dun_level + MON_SUMMON_ADJ);
  for (int it = 0; it < 9; ++it) {
    j = y - 2 + randint(3);
    k = x - 2 + randint(3);
    if (in_bounds(j, k)) {
      cave_ptr = &caveD[j][k];
      if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->midx == 0)) {
        summon = (place_monster(j, k, l, slp));
        break;
      }
    }
  }
  return (summon);
}
int
summon_undead(y, x)
{
  int j, k, cidx;
  int l, m, summon;
  struct caveS* cave_ptr;

  summon = FALSE;
  l = m_level[MAX_MON_LEVEL];
  while (l) {
    m = randint(l) - 1;
    for (int it = 0; it < 20; ++it) {
      cidx = m + it;

      if (cidx < l && creatureD[cidx].cdefense & CD_UNDEAD) {
        l = 0;
        break;
      }
    }
  }

  for (int it = 0; it < 9; ++it) {
    j = y - 2 + randint(3);
    k = x - 2 + randint(3);
    if (in_bounds(j, k)) {
      cave_ptr = &caveD[j][k];
      if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->midx == 0)) {
        summon = place_monster(j, k, cidx, FALSE);
        break;
      }
    }
  }
  return (summon);
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
static char* gold_nameD[MAX_GOLD] = {
    "copper",    "copper",  "copper", "silver",   "silver",   "silver",
    "garnets",   "garnets", "gold",   "gold",     "gold",     "opals",
    "sapphires", "gold",    "rubies", "diamonds", "emeralds", "mithril",
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
  obj->fy = y;
  obj->fx = x;
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

  tr_obj_copy(z, obj);
  obj->fy = y;
  obj->fx = x;
  struct treasureS* t_ptr = &treasureD[obj->tidx];

  magic_treasure(obj, dun_level);

  if (uD.y == y && uD.x == x)
    msg_print("You feel something roll beneath your feet.");
}
void place_trap(y, x, subval) int y, x, subval;
{
  struct objS* obj = obj_use();

  caveD[y][x].oidx = obj_index(obj);
  // invcopy(&t_list[cur_pos], OBJ_TRAP_LIST + subval);
  obj->fy = y;
  obj->fx = x;
  obj->tval = TV_INVIS_TRAP;
  obj->tchar = '9';
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
  int room_map[CHUNK_COL][CHUNK_ROW] = {0};
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
        yloc[k] = i * CHUNK_HEIGHT + (CHUNK_HEIGHT >> 2);
        xloc[k] = j * CHUNK_WIDTH + (CHUNK_WIDTH >> 2);
        build_room(yloc[k], xloc[k]);
        k++;
      }

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
  new_spot(&uD.y, &uD.x);

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
  // Clear the cave
  memset(caveD, 0, sizeof(caveD));

  // Release objects in the cave
  FOR_EACH(obj, {
    if (obj->fx || obj->fy) {
      obj_unuse(obj);
    }
  });

  // Release all monsters
  mon_usedD = 0;
  memset(entity_monD, 0, sizeof(entity_monD));

  // a fresh cave!
  cave_gen();
}
BOOL
cave_lit(cave)
struct caveS* cave;
{
  return (cave->cflag & (CF_TEMP_LIGHT | CF_PERM_LIGHT)) != 0;
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
    if (mon->ml) return creature->cchar;
  }
  if (((cave_ptr->cflag & CF_FIELDMARK) == 0) && !cave_lit(cave_ptr))
    return ' ';
  if (cave_ptr->oidx) {
    struct objS* obj = &entity_objD[cave_ptr->oidx];
    return obj->tchar;
  }
  switch (cave_ptr->fval) {
    case FLOOR_LIGHT:
    case FLOOR_DARK:
    case FLOOR_CORR:
      return '.';
    case FLOOR_OBST:
      return ';';
  }
  return '#';
}
BOOL
panel_contains(panel, y, x)
struct panelS* panel;
int y, x;
{
  int rmin = panelD.panel_row_min;
  int rmax = panelD.panel_row_max;
  int cmin = panelD.panel_col_min;
  int cmax = panelD.panel_col_max;
  return (y >= rmin && y < rmax && x >= cmin && x < cmax);
}
void
panel_bounds(struct panelS* panel)
{
  int panel_row = panel->panel_row;
  int panel_col = panel->panel_col;
  panel->panel_row_min = panel_row * (SYMMAP_HEIGHT / 2);
  panel->panel_row_max = panel->panel_row_min + SYMMAP_HEIGHT;
  panel->panel_col_min = panel_col * (SYMMAP_WIDTH / 2);
  panel->panel_col_max = panel->panel_col_min + SYMMAP_WIDTH;
}

void
panel_update(struct panelS* panel, int y, int x, BOOL force)
{
  BOOL yd = (y < panel->panel_row_min + 2 || y > panel->panel_row_max - 3);
  if (force || yd) {
    int prow = (y - SYMMAP_HEIGHT / 4) / (SYMMAP_HEIGHT / 2);
    panel->panel_row = CLAMP(prow, 0, MAX_ROW - 2);
  }

  BOOL xd = (x < panel->panel_col_min + 2 || x > panel->panel_col_max - 3);
  if (force || xd) {
    int pcol = (x - SYMMAP_WIDTH / 4) / (SYMMAP_WIDTH / 2);
    panel->panel_col = CLAMP(pcol, 0, MAX_COL - 2);
  }

  panel_bounds(panel);
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
int
mmove(dir, y, x)
int dir;
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
static int
see_wall(dir, y, x)
int dir, y, x;
{
  char c;
  if (!mmove(dir, &y, &x)) /* check to see if movement there possible */
    return TRUE;
  else if ((c = get_sym(y, x)) == '#' || c == '%')
    return TRUE;
  else
    return FALSE;
}
void find_init(dir, y_ptr, x_ptr) int dir;
int *y_ptr, *x_ptr;
{
  int deepleft, deepright;
  int i, shortleft, shortright;

  if (!mmove(dir, y_ptr, x_ptr))
    find_flag = FALSE;
  else {
    find_direction = dir;
    find_flag = TRUE;
    find_breakright = find_breakleft = FALSE;
    find_prevdir = dir;
    // TBD: if (py.flags.blind < 1)
    i = chome[dir];
    deepleft = deepright = FALSE;
    shortright = shortleft = FALSE;
    if (see_wall(cycle[i + 1], uD.y, uD.x)) {
      find_breakleft = TRUE;
      shortleft = TRUE;
    } else if (see_wall(cycle[i + 1], *y_ptr, *x_ptr)) {
      find_breakleft = TRUE;
      deepleft = TRUE;
    }
    if (see_wall(cycle[i - 1], uD.y, uD.x)) {
      find_breakright = TRUE;
      shortright = TRUE;
    } else if (see_wall(cycle[i - 1], *y_ptr, *x_ptr)) {
      find_breakright = TRUE;
      deepright = TRUE;
    }
    if (find_breakleft && find_breakright) {
      find_openarea = FALSE;
      if (dir & 1) { /* a hack to allow angled corridor entry */
        if (deepleft && !deepright)
          find_prevdir = cycle[i - 1];
        else if (deepright && !deepleft)
          find_prevdir = cycle[i + 1];
      }
      /* else if there is a wall two spaces ahead and seem to be in a
         corridor, then force a turn into the side corridor, must
         be moving straight into a corridor here */
      else if (see_wall(cycle[i], *y_ptr, *x_ptr)) {
        if (shortleft && !shortright)
          find_prevdir = cycle[i - 2];
        else if (shortright && !shortleft)
          find_prevdir = cycle[i + 2];
      }
    } else
      find_openarea = TRUE;
  }
}
static int
see_nothing(dir, y, x)
int dir, y, x;
{
  if (!mmove(dir, &y, &x)) /* check to see if movement there possible */
    return FALSE;
  else if (get_sym(y, x) == ' ')
    return TRUE;
  else
    return FALSE;
}
void find_event(y, x) int y, x;
{
  int dir, newdir, t, inv, check_dir, row, col;
  register int i, max, option, option2;
  struct caveS* c_ptr;

  // if (py.flags.blind < 1)
  option = 0;
  option2 = 0;
  dir = find_prevdir;
  max = (dir & 1) + 1;
  /* Look at every newly adjacent square. */
  for (i = -max; i <= max; i++) {
    newdir = cycle[chome[dir] + i];
    row = y;
    col = x;
    if (mmove(newdir, &row, &col)) {
      /* Objects player can see (Including doors?) cause a stop. */
      c_ptr = &caveD[row][col];
      // inv = TRUE;
      // if (player_light || c_ptr->tl || c_ptr->pl || c_ptr->fm)
      if (c_ptr->oidx != 0) {
        t = entity_objD[c_ptr->oidx].tval;
        if (t != TV_INVIS_TRAP && t != TV_SECRET_DOOR &&
            (t != TV_OPEN_DOOR || !find_ignore_doors)) {
          find_flag = FALSE;
          return;
        }
      }
      /* Also Creatures  	*/
      /* the monster should be visible since update_mon() checks
         for the special case of being in find mode */
      if (c_ptr->midx != 0 && entity_monD[c_ptr->midx].ml) {
        find_flag = FALSE;
        return;
      }
      inv = FALSE;

      if (c_ptr->fval <= MAX_OPEN_SPACE || inv) {
        if (find_openarea) {
          /* Have we found a break? */
          if (i < 0) {
            if (find_breakright) {
              find_flag = FALSE;
              return;
            }
          } else if (i > 0) {
            if (find_breakleft) {
              find_flag = FALSE;
              return;
            }
          }
        } else if (option == 0)
          option = newdir; /* The first new direction. */
        else if (option2 != 0) {
          find_flag = FALSE; /* Three new directions. STOP. */
          return;
        } else if (option != cycle[chome[dir] + i - 1]) {
          find_flag = FALSE; /* If not adjacent to prev, STOP */
          return;
        } else {
          /* Two adjacent choices. Make option2 the diagonal,
             and remember the other diagonal adjacent to the first
             option. */
          if ((newdir & 1) == 1) {
            check_dir = cycle[chome[dir] + i - 2];
            option2 = newdir;
          } else {
            check_dir = cycle[chome[dir] + i + 1];
            option2 = option;
            option = newdir;
          }
        }
      } else if (find_openarea) {
        /* We see an obstacle. In open area, STOP if on a side
           previously open. */
        if (i < 0) {
          if (find_breakleft) {
            find_flag = FALSE;
            return;
          }
          find_breakright = TRUE;
        } else if (i > 0) {
          if (find_breakright) {
            find_flag = FALSE;
            return;
          }
          find_breakleft = TRUE;
        }
      }
    }
  }

  if (find_openarea == FALSE) { /* choose a direction. */
    if (option2 == 0 || (find_examine && !find_cut)) {
      /* There is only one option, or if two, then we always examine
         potential corners and never cur known corners, so you step
         into the straight option. */
      if (option != 0) find_direction = option;
      if (option2 == 0)
        find_prevdir = option;
      else
        find_prevdir = option2;
    } else {
      /* Two options! */
      row = y;
      col = x;
      (void)mmove(option, &row, &col);
      if (!see_wall(option, row, col) || !see_wall(check_dir, row, col)) {
        /* Don't see that it is closed off.  This could be a
           potential corner or an intersection. */
        if (find_examine && see_nothing(option, row, col) &&
            see_nothing(option2, row, col))
        /* Can not see anything ahead and in the direction we are
           turning, assume that it is a potential corner. */
        {
          find_direction = option;
          find_prevdir = option2;
        } else
          /* STOP: we are next to an intersection or a room */
          find_flag = FALSE;
      } else if (find_cut) {
        /* This corner is seen to be enclosed; we cut the corner. */
        find_direction = option2;
        find_prevdir = option2;
      } else {
        /* This corner is seen to be enclosed, and we deliberately
           go the long way. */
        find_direction = option;
        find_prevdir = option2;
      }
    }
  }
}
void move_rec(y1, x1, y2, x2) register int y1, x1, y2, x2;
{
  int tmp = caveD[y1][x1].midx;
  caveD[y1][x1].midx = 0;
  caveD[y2][x2].midx = tmp;
}
void update_mon(midx) int midx;
{
  register int flag, fy, fx;
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;

  m_ptr = &entity_monD[midx];
  cr_ptr = &creatureD[m_ptr->cidx];
  flag = FALSE;
  fy = m_ptr->fy;
  fx = m_ptr->fx;
  if ((m_ptr->cdis <= MAX_SIGHT) && (panel_contains(&panelD, fy, m_ptr->fx))) {
    // TBD: line-of-sight
    // if (los(uD.y, uD.x, fy, m_ptr->fx)) {
    c_ptr = &caveD[fy][fx];
    /* Normal sight.       */
    if (cave_lit(c_ptr)) {
      if ((CM_INVISIBLE & cr_ptr->cmove) == 0) flag = TRUE;
      // else if (py.flags.see_inv) {
      //   flag = TRUE;
      //   c_recall[m_ptr->mptr].r_cmove |= CM_INVISIBLE;
      // }
    }
    /* Infra vision.   */
    // else if ((py.flags.see_infra > 0) && (m_ptr->cdis <=
    // py.flags.see_infra)
    // &&
    //          (CD_INFRA & cr_ptr->cdefense)) {
    //   flag = TRUE;
    //   c_recall[m_ptr->mptr].r_cdefense |= CD_INFRA;
    // }
    // }
  }
  /* Light it up.   */
  if (flag) {
    if (!m_ptr->ml) {
      disturb(1, 0);
      m_ptr->ml = TRUE;
    }
  }
  /* Turn it off.   */
  else if (m_ptr->ml) {
    m_ptr->ml = FALSE;
  }
  SYMMAP_PATCH(fy, fx);
}
int
enchant(int16_t* bonus, int16_t limit)
{
  register int chance, res;

  if (limit <= 0) /* avoid randint(0) call */
    return (FALSE);
  chance = 0;
  res = FALSE;
  if (*bonus > 0) {
    chance = *bonus;
    if (randint(100) == 1) /* very rarely allow enchantment over limit */
      chance = randint(chance) - 1;
  }
  if (randint(limit) > chance) {
    *bonus += 1;
    res = TRUE;
  }
  return (res);
}
int
bth_adj(attype)
int attype;
{
  switch (attype) {
    case 1:
      return 60;
    case 2:
      return -3;
    case 3:
      return 10;
    case 4:
      return 10;
    case 5:
      return 10;
    case 6:
      return 0;
    case 7:
      return 10;
    case 8:
      return 10;
    case 9:
      return 0;
    case 10:
      return 2;
    case 11:
      return 2;
    case 12:
      return 5;
    case 13:
      return 2;
    case 14:
      return 5;
    case 15:
      return 0;
    case 16:
      return 0;
    case 17:
      return 2;
    case 18:
      return 2;
    case 19:
      return 5;
    case 20:
      return 120;  // TBD: originally cannot miss
    case 21:
      return 20;
    case 22:
      return 5;
    case 23:
      return 5;
    case 24:
      return 15;
    case 99:
      return 120;  // TBD: originally cannot miss
  }
  return -60;
}
char*
attack_string(adesc)
int adesc;
{
  switch (adesc) {
    case 1:
      return ((" hits you."));
    case 2:
      return ((" bites you."));
    case 3:
      return ((" claws you."));
    case 4:
      return ((" stings you."));
    case 5:
      return ((" touches you."));
    case 6:
      return ((" kicks you."));
    case 7:
      return ((" gazes at you."));
    case 8:
      return ((" breathes on you."));
    case 9:
      return ((" spits on you."));
    case 10:
      return ((" makes a horrible wail."));
    case 11:
      return ((" embraces you."));
    case 12:
      return ((" crawls on you."));
    case 13:
      return ((" releases a cloud of spores."));
    case 14:
      return ((" begs you for money."));
    case 15:
      descD[0] = 0;
      return ("You've been slimed!");
    case 16:
      return ((" crushes you."));
    case 17:
      return ((" tramples you."));
    case 18:
      return ((" drools on you."));
    case 19:
      switch (randint(9)) {
        case 1:
          return ((" insults you!"));
        case 2:
          return ((" insults your mother!"));
        case 3:
          return ((" gives you the finger!"));
        case 4:
          return ((" humiliates you!"));
        case 5:
          return ((" wets on your leg!"));
        case 6:
          return ((" defiles you!"));
        case 7:
          return ((" dances around you!"));
        case 8:
          return ((" makes obscene gestures!"));
        case 9:
          return ((" moons you!!!"));
      }
    case 99:
      return ((" is repelled."));
  }
  return " hits you.";
}
int
con_adj()
{
  int con;

  con = statD.use_stat[A_CON];
  if (con < 7)
    return (con - 7);
  else if (con < 17)
    return (0);
  else if (con == 17)
    return (1);
  else if (con < 94)
    return (2);
  else if (con < 117)
    return (3);
  else
    return (4);
}
int
poison_adj()
{
  int i;

  i = 0;
  switch (con_adj()) {
    case -4:
      i = 4;
      break;
    case -3:
    case -2:
      i = 3;
      break;
    case -1:
      i = 2;
      break;
    case 0:
      i = 1;
      break;
    case 1:
    case 2:
    case 3:
      i = ((turnD % 2) == 0);
      break;
    case 4:
    case 5:
      i = ((turnD % 3) == 0);
      break;
    case 6:
      i = ((turnD % 4) == 0);
      break;
  }
  return i;
}
int
think_adj(stat)
int stat;
{
  register int value;

  value = statD.use_stat[stat];
  if (value > 117)
    return (7);
  else if (value > 107)
    return (6);
  else if (value > 87)
    return (5);
  else if (value > 67)
    return (4);
  else if (value > 17)
    return (3);
  else if (value > 14)
    return (2);
  else if (value > 7)
    return (1);
  else
    return (0);
}
int
tohit_adj()
{
  register int total, stat;

  stat = statD.use_stat[A_DEX];
  if (stat < 4)
    total = -3;
  else if (stat < 6)
    total = -2;
  else if (stat < 8)
    total = -1;
  else if (stat < 16)
    total = 0;
  else if (stat < 17)
    total = 1;
  else if (stat < 18)
    total = 2;
  else if (stat < 69)
    total = 3;
  else if (stat < 118)
    total = 4;
  else
    total = 5;
  stat = statD.use_stat[A_STR];
  if (stat < 4)
    total -= 3;
  else if (stat < 5)
    total -= 2;
  else if (stat < 7)
    total -= 1;
  else if (stat < 18)
    total -= 0;
  else if (stat < 94)
    total += 1;
  else if (stat < 109)
    total += 2;
  else if (stat < 117)
    total += 3;
  else
    total += 4;
  return (total);
}
int
toac_adj()
{
  register int stat;

  stat = statD.use_stat[A_DEX];
  if (stat < 4)
    return (-4);
  else if (stat == 4)
    return (-3);
  else if (stat == 5)
    return (-2);
  else if (stat == 6)
    return (-1);
  else if (stat < 15)
    return (0);
  else if (stat < 18)
    return (1);
  else if (stat < 59)
    return (2);
  else if (stat < 94)
    return (3);
  else if (stat < 117)
    return (4);
  else
    return (5);
}
int
todis_adj()
{
  register int stat;

  stat = statD.use_stat[A_DEX];
  if (stat < 4)
    return (-8);
  else if (stat == 4)
    return (-6);
  else if (stat == 5)
    return (-4);
  else if (stat == 6)
    return (-2);
  else if (stat == 7)
    return (-1);
  else if (stat < 13)
    return (0);
  else if (stat < 16)
    return (1);
  else if (stat < 18)
    return (2);
  else if (stat < 59)
    return (4);
  else if (stat < 94)
    return (5);
  else if (stat < 117)
    return (6);
  else
    return (8);
}
int
todam_adj()
{
  register int stat;

  stat = statD.use_stat[A_STR];
  if (stat < 4)
    return (-2);
  else if (stat < 5)
    return (-1);
  else if (stat < 16)
    return (0);
  else if (stat < 17)
    return (1);
  else if (stat < 18)
    return (2);
  else if (stat < 94)
    return (3);
  else if (stat < 109)
    return (4);
  else if (stat < 117)
    return (5);
  else
    return (6);
}
int
test_hit(bth, level_adj, pth, ac)
int bth, level_adj, pth, ac;
{
  register int i, die;

  disturb(1, 0);
  i = bth + pth * BTH_PLUS_ADJ + level_adj;

  // pth could be less than 0 if player wielding weapon too heavy for him
  // bth can be less than 0 for creatures
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
static int
mon_take_hit(midx, dam)
int midx, dam;
{
  struct monS* mon = &entity_monD[midx];
  struct creatureS* cre = &creatureD[mon->cidx];
  mon->hp -= dam;
  if (mon->hp >= 0) return -1;

  mon_death(mon->fy, mon->fx);
  // TBD: frac_exp
  uD.exp += (cre->mexp * cre->level) / uD.lev;
  int cidx = mon->cidx;
  mon_unuse(mon);
  return cidx;
}
BOOL
is_a_vowel(c)
char c;
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
void obj_prefix(obj, prefix) struct objS* obj;
BOOL prefix;
{
  char obj_name[160];

  int offset = 0;
  for (int it = 0; it < AL(descD); ++it) {
    if (descD[it] != '~')
      obj_name[it - offset] = descD[it];
    else if (obj->number > 1)
      obj_name[it - offset] = 's';
    else
      offset += 1;
    if (descD[it] == 0) break;
  }

  if (prefix) {
    /* ampersand is always the first character */
    if (obj_name[0] == '&') {
      /* use &obj_name[1], so that & does not appear in output */
      if (obj->number > 1)
        snprintf(descD, AL(descD), "%d%s", (int)obj->number, &obj_name[1]);
      else if (obj->number < 1)
        snprintf(descD, AL(descD), "%s%s", "no more", &obj_name[1]);
      else if (is_a_vowel(obj_name[2]))
        snprintf(descD, AL(descD), "an%s", &obj_name[1]);
      else
        snprintf(descD, AL(descD), "a%s", &obj_name[1]);
    }
    /* handle 'no more' case specially */
    else if (obj->number < 1) {
      /* check for "some" at start */
      if (!strncmp("some", obj_name, 4))
        snprintf(descD, AL(descD), "no more %s", &obj_name[5]);
      /* here if no article */
      else
        snprintf(descD, AL(descD), "no more %s", obj_name);
    } else
      strcpy(descD, obj_name);
  } else {
    if (!strncmp("some", obj_name, 4))
      strcpy(descD, &obj_name[5]);
    else if (obj_name[0] == '&')
      /* eliminate the '& ' at the beginning */
      strcpy(descD, &obj_name[2]);
    else
      strcpy(descD, obj_name);
  }
}
void obj_detail(obj) struct objS* obj;
{
  char tmp_str[80];

  if (obj->name2 && obj_reveal(obj)) {
    strcat(descD, " ");
    strcat(descD, obj->name2);
  }
  if (obj_reveal(obj)) {
    if (oset_hitdam(obj))
      snprintf(tmp_str, AL(tmp_str), " (%+d,%+d)", obj->tohit, obj->todam);
    else if (obj->tohit != 0)
      snprintf(tmp_str, AL(tmp_str), " (%+d)", obj->tohit);
    else if (obj->todam != 0)
      snprintf(tmp_str, AL(tmp_str), " (%+d)", obj->todam);
    else
      tmp_str[0] = '\0';
    strcat(descD, tmp_str);
  }
  /* Crowns have a zero base AC, so make a special test for them. */
  if (obj->ac != 0 || (obj->tval == TV_HELM)) {
    snprintf(tmp_str, AL(tmp_str), " [%d", obj->ac);
    strcat(descD, tmp_str);
    if (obj_reveal(obj)) {
      snprintf(tmp_str, AL(tmp_str), ",%+d", obj->toac);
      strcat(descD, tmp_str);
    }
    strcat(descD, "]");
  } else if ((obj->toac != 0) && obj_reveal(obj)) {
    snprintf(tmp_str, AL(tmp_str), " [%+d]", obj->toac);
    strcat(descD, tmp_str);
  }

  /* override defaults, check for p1 flags in the ident field */
  // if (obj->ident & ID_NOSHOW_P1)
  //   p1_use = IGNORED;
  // else if (obj->ident & ID_SHOW_P1)
  //   p1_use = Z_PLUSSES;
  // tmp_str[0] = '\0';
  // if (p1_use == LIGHT)
  //   sprintf(tmp_str, " with %d turns of light", obj->p1);
  // else if (p1_use == IGNORED)
  //   ;
  // else if (known2_p(obj)) {
  //   if (p1_use == Z_PLUSSES)
  //     sprintf(tmp_str, " (%c%d)", (obj->p1 < 0) ? '-' : '+', abs(obj->p1));
  //   else if (p1_use == CHARGES)
  //     sprintf(tmp_str, " (%d charges)", obj->p1);
  //   else if (obj->p1 != 0) {
  //     if (p1_use == PLUSSES)
  //       sprintf(tmp_str, " (%c%d)", (obj->p1 < 0) ? '-' : '+',
  //       abs(obj->p1));
  //     else if (p1_use == FLAGS) {
  //       if (obj->flags & TR_STR)
  //         sprintf(tmp_str, " (%c%d to STR)", (obj->p1 < 0) ? '-' : '+',
  //                 abs(obj->p1));
  //       else if (obj->flags & TR_STEALTH)
  //         sprintf(tmp_str, " (%c%d to stealth)", (obj->p1 < 0) ? '-' : '+',
  //                 abs(obj->p1));
  //     }
  //   }
  // }
  // strcat(descD, tmp_str);

  // TBD: magik empty damned
}
void obj_desc(obj, prefix) struct objS* obj;
BOOL prefix;
{
  char* basenm;
  char damstr[80];
  int indexx, modify, append_name, tmp;
  struct treasureS* tr_ptr;

  tr_ptr = &treasureD[obj->tidx];
  indexx = obj->subval & (ITEM_SINGLE_STACK - 1);
  basenm = tr_ptr->name;
  damstr[0] = 0;
  modify = tr_known(tr_ptr) ? FALSE : TRUE;
  append_name = FALSE;
  switch (obj->tval) {
    case TV_MISC:
    case TV_CHEST:
      break;
    case TV_SLING_AMMO:
    case TV_BOLT:
    case TV_ARROW:
      snprintf(damstr, AL(damstr), " (%dd%d)", obj->damage[0], obj->damage[1]);
      break;
    case TV_LIGHT:
      break;
    case TV_SPIKE:
      break;
    case TV_BOW:
      if (obj->p1 == 1 || obj->p1 == 2)
        tmp = 2;
      else if (obj->p1 == 3 || obj->p1 == 5)
        tmp = 3;
      else if (obj->p1 == 4 || obj->p1 == 6)
        tmp = 4;
      else
        tmp = -1;
      snprintf(damstr, AL(damstr), " (x%d)", tmp);
      break;
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
      snprintf(damstr, AL(damstr), " (%dd%d)", obj->damage[0], obj->damage[1]);
      break;
    case TV_DIGGING:
      snprintf(damstr, AL(damstr), " (%dd%d)", obj->damage[0], obj->damage[1]);
      break;
    case TV_BOOTS:
    case TV_GLOVES:
    case TV_CLOAK:
    case TV_HELM:
    case TV_SHIELD:
    case TV_HARD_ARMOR:
    case TV_SOFT_ARMOR:
      break;
    case TV_AMULET:
      if (modify) {
        snprintf(descD, AL(descD), "& %s Amulet", amulets[indexx]);
        basenm = 0;
      } else {
        basenm = "& Amulet";
        append_name = TRUE;
      }
      break;
    case TV_RING:
      if (modify) {
        basenm = 0;
        snprintf(descD, AL(descD), "& %s Ring", rocks[indexx]);
      } else {
        basenm = "& Ring";
        append_name = TRUE;
      }
      break;
    case TV_STAFF:
      if (modify) {
        basenm = 0;
        snprintf(descD, AL(descD), "& %s Staff", woods[indexx]);
      } else {
        basenm = "& Staff";
        append_name = TRUE;
      }
      break;
    case TV_WAND:
      if (modify) {
        basenm = 0;
        snprintf(descD, AL(descD), "& %s Wand", metals[indexx]);
      } else {
        basenm = "& Wand";
        append_name = TRUE;
      }
      break;
    case TV_SCROLL1:
    case TV_SCROLL2:
      if (modify) {
        basenm = 0;
        snprintf(descD, AL(descD), "& Scroll~ titled \"%s\"", titleD[indexx]);
      } else {
        basenm = "& Scroll~";
        append_name = TRUE;
      }
      break;
    case TV_POTION1:
    case TV_POTION2:
      if (modify) {
        basenm = 0;
        snprintf(descD, AL(descD), "& %s Potion~", colors[indexx]);
      } else {
        basenm = "& Potion~";
        append_name = TRUE;
      }
      break;
    case TV_FLASK:
      break;
    case TV_FOOD:
      if (modify) {
        if (indexx <= 15)
          snprintf(descD, AL(descD), "& %s Mushroom~", mushrooms[indexx]);
        else if (indexx <= 20)
          snprintf(descD, AL(descD), "& Hairy %s Mold~", mushrooms[indexx]);
        if (indexx <= 20) basenm = 0;
      } else {
        append_name = TRUE;
        if (indexx <= 15)
          basenm = "& Mushroom~";
        else if (indexx <= 20)
          basenm = "& Hairy Mold~";
        else
          /* Ordinary food does not have a name appended.  */
          append_name = FALSE;
      }
      break;
    case TV_MAGIC_BOOK:
      snprintf(descD, AL(descD), "& Book~ of Magic Spells %s", basenm);
      basenm = 0;
      break;
    case TV_PRAYER_BOOK:
      snprintf(descD, AL(descD), "& Holy Book~ of Prayers %s", basenm);
      basenm = 0;
      return;
    case TV_OPEN_DOOR:
    case TV_CLOSED_DOOR:
    case TV_SECRET_DOOR:
    case TV_RUBBLE:
    case TV_GOLD:
    case TV_INVIS_TRAP:
    case TV_VIS_TRAP:
    case TV_UP_STAIR:
    case TV_DOWN_STAIR:
      descD[0] = 0;
      return;
    // case TV_STORE_DOOR:
    //   sprintf(descD, "the entrance to the %s.",
    //   object_list[obj->index].name); return;
    default:
      strcpy(descD, "Error in objdes()");
      return;
  }
  if (basenm) strcpy(descD, basenm);
  if (append_name) {
    strcat(descD, " of ");
    strcat(descD, tr_ptr->name);
  }
  if (damstr[0]) strcat(descD, damstr);
  obj_prefix(obj, prefix);
  if (prefix) obj_detail(obj);
}
static void mon_desc(midx) int midx;
{
  struct monS* mon = &entity_monD[midx];
  struct creatureS* cre = &creatureD[mon->cidx];

  // if (!mon->ml)
  //  strcpy(cdesc, "It ");
  // else
  snprintf(descD, AL(descD), "The %s", cre->name);

  // if (CM_WIN & cre->cmove)
  //  snprintf(death_descD, AL(death_descD), "The %s", cre->name);
  if (is_a_vowel(cre->name[0]))
    snprintf(death_descD, AL(death_descD), "An %s", cre->name);
  else
    snprintf(death_descD, AL(death_descD), "A %s", cre->name);
}
void calc_hitpoints(level) int level;
{
  int hitpoints;

  hitpoints = player_hpD[level - 1] + (con_adj() * level);
  /* always give at least one point per level + 1 */
  if (hitpoints < (level + 1)) hitpoints = level + 1;

  // TBD: heroism
  // if (py.flags.status & PY_HERO) hitpoints += 10;
  // if (py.flags.status & PY_SHERO) hitpoints += 20;

  // Scale current hp to the new maximum
  int value = ((uD.chp << 16) + uD.chp_frac) / uD.mhp * hitpoints;
  uD.chp = value >> 16;
  uD.chp_frac = value & 0xFFFF;
  uD.mhp = hitpoints;
}
void
calc_bonuses()
{
  int it;
  // p_ptr = &py.flags;
  // m_ptr = &py.misc;
  // if (p_ptr->slow_digest) p_ptr->food_digest++;
  // if (p_ptr->regenerate) p_ptr->food_digest -= 3;
  // p_ptr->see_inv = FALSE;
  // p_ptr->teleport = FALSE;
  // p_ptr->free_act = FALSE;
  // p_ptr->slow_digest = FALSE;
  // p_ptr->aggravate = FALSE;
  // p_ptr->sustain_str = FALSE;
  // p_ptr->sustain_int = FALSE;
  // p_ptr->sustain_wis = FALSE;
  // p_ptr->sustain_con = FALSE;
  // p_ptr->sustain_dex = FALSE;
  // p_ptr->sustain_chr = FALSE;
  // p_ptr->fire_resist = FALSE;
  // p_ptr->acid_resist = FALSE;
  // p_ptr->cold_resist = FALSE;
  // p_ptr->regenerate = FALSE;
  // p_ptr->lght_resist = FALSE;
  // p_ptr->ffall = FALSE;

  uD.ptohit = tohit_adj(); /* Real To Hit   */
  uD.ptodam = todam_adj(); /* Real To Dam   */
  uD.ptoac = toac_adj();   /* Real To AC    */
  uD.pac = 0;              /* Real AC       */
  for (it = INVEN_WIELD; it < INVEN_LIGHT; it++) {
    struct objS* obj = obj_get(invenD[it]);
    uD.ptohit += obj->tohit;
    if (obj->tval != TV_BOW) /* Bows can't damage. -CJS- */
      uD.ptodam += obj->todam;
    uD.ptoac += obj->toac;
    uD.pac += obj->ac;
  }

  // if (weapon_heavy)
  //  uD.dis_th +=
  //      (py.stats.use_stat[A_STR] * 15 - inventory[INVEN_WIELD].weight);

  /* Add in temporary spell increases  */
  // if (p_ptr->invuln > 0) {
  //  m_ptr->pac += 100;
  //  m_ptr->dis_ac += 100;
  //}
  // if (p_ptr->blessed > 0) {
  //  m_ptr->pac += 2;
  //  m_ptr->dis_ac += 2;
  //}
  // if (p_ptr->detect_inv > 0) p_ptr->see_inv = TRUE;

  // item_flags = 0;
  // i_ptr = &inventory[INVEN_WIELD];
  // for (i = INVEN_WIELD; i < INVEN_LIGHT; i++) {
  //  item_flags |= i_ptr->flags;
  //  i_ptr++;
  //}
  // if (TR_SLOW_DIGEST & item_flags) p_ptr->slow_digest = TRUE;
  // if (TR_AGGRAVATE & item_flags) p_ptr->aggravate = TRUE;
  // if (TR_TELEPORT & item_flags) p_ptr->teleport = TRUE;
  // if (TR_REGEN & item_flags) p_ptr->regenerate = TRUE;
  // if (TR_RES_FIRE & item_flags) p_ptr->fire_resist = TRUE;
  // if (TR_RES_ACID & item_flags) p_ptr->acid_resist = TRUE;
  // if (TR_RES_COLD & item_flags) p_ptr->cold_resist = TRUE;
  // if (TR_FREE_ACT & item_flags) p_ptr->free_act = TRUE;
  // if (TR_SEE_INVIS & item_flags) p_ptr->see_inv = TRUE;
  // if (TR_RES_LIGHT & item_flags) p_ptr->lght_resist = TRUE;
  // if (TR_FFALL & item_flags) p_ptr->ffall = TRUE;

  // i_ptr = &inventory[INVEN_WIELD];
  // for (i = INVEN_WIELD; i < INVEN_LIGHT; i++) {
  //  if (TR_SUST_STAT & i_ptr->flags) switch (i_ptr->p1) {
  //      case 1:
  //        p_ptr->sustain_str = TRUE;
  //        break;
  //      case 2:
  //        p_ptr->sustain_int = TRUE;
  //        break;
  //      case 3:
  //        p_ptr->sustain_wis = TRUE;
  //        break;
  //      case 4:
  //        p_ptr->sustain_con = TRUE;
  //        break;
  //      case 5:
  //        p_ptr->sustain_dex = TRUE;
  //        break;
  //      case 6:
  //        p_ptr->sustain_chr = TRUE;
  //        break;
  //      default:
  //        break;
  //    }
  //  i_ptr++;
  //}

  // if (p_ptr->slow_digest) p_ptr->food_digest--;
  // if (p_ptr->regenerate) p_ptr->food_digest += 3;
}
int8_t
modify_stat(stat, amount)
int stat, amount;
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

  if (stat == A_STR) {
    calc_bonuses();
  } else if (stat == A_DEX) {
    calc_bonuses();
  } else if (stat == A_CON)
    calc_hitpoints(uD.lev);
  // else if (stat == A_INT) {
  //   if (class[py.misc.pclass].spell == MAGE) calc_spells(A_INT);
  //   calc_mana(A_INT);
  // } else if (stat == A_WIS) {
  //   if (class[py.misc.pclass].spell == PRIEST) calc_spells(A_WIS);
  //   calc_mana(A_WIS);
  // }
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
  if (TR_SEARCH & obj->flags) {
    uD.search += amount;
    uD.fos -= amount;
  }
  if (TR_STEALTH & obj->flags) uD.stealth += amount;
  if (TR_SPEED & obj->flags) uD.pspeed -= amount;
  // if ((TR_BLIND & obj->flags) && (factor > 0)) py.flags.blind += 1000;
  // if ((TR_TIMID & obj->flags) && (factor > 0)) py.flags.afraid += 50;
  // if (TR_INFRA & obj->flags) py.flags.see_infra += amount;
}
static int
equip_cursed()
{
  for (int it = INVEN_EQUIP; it < MAX_INVEN; ++it) {
    struct objS* obj = obj_get(invenD[it]);
    if (obj->flags & TR_CURSED) return it;
  }
  return -1;
}
static int
equip_random()
{
  int tmp[6], k;

  if (invenD[INVEN_BODY]) tmp[k++] = INVEN_BODY;
  if (invenD[INVEN_ARM]) tmp[k++] = INVEN_ARM;
  if (invenD[INVEN_OUTER]) tmp[k++] = INVEN_OUTER;
  if (invenD[INVEN_HANDS]) tmp[k++] = INVEN_HANDS;
  if (invenD[INVEN_HEAD]) tmp[k++] = INVEN_HEAD;
  if (invenD[INVEN_FEET]) tmp[k++] = INVEN_FEET;
  if (k > 0) return tmp[randint(k) - 1];
  return -1;
}
static int
equip_enchant(amount)
{
  struct objS* i_ptr;
  int affect, l;

  l = equip_cursed();
  if (l < 0) l = equip_random();

  if (l >= 0) {
    i_ptr = obj_get(invenD[l]);
    obj_desc(i_ptr, FALSE);
    affect = 0;
    for (int it = 0; it < amount; ++it) {
      affect += (enchant(&i_ptr->toac, 10));
    }

    if (affect) {
      MSG("Your %s glows %s!", descD, affect > 1 ? "brightly" : "faintly");
      i_ptr->flags &= ~TR_CURSED;
      calc_bonuses();
    } else
      msg_print("The enchantment fails.");
    return TRUE;
  }

  return FALSE;
}
static int
equip_curse()
{
  struct objS* i_ptr;
  int l;

  l = equip_random();
  if (l > 0) {
    i_ptr = obj_get(invenD[l]);
    obj_desc(i_ptr, FALSE);
    MSG("Your %s glows black, fades.", descD);
    i_ptr->name2 = 0;
    i_ptr->tohit = 0;
    i_ptr->todam = 0;
    i_ptr->toac = -randint(5) - randint(5);

    // TBD: Test this, I believe moria contains a bug here;
    // The behavior is changed to match curse weapon
    // py_bonuses removes adjustments from obj->flags
    // obj->flags are changed
    // calc_bonuses() sum tohit, todam, toac
    py_bonuses(i_ptr, -1);
    i_ptr->flags = TR_CURSED;
    calc_bonuses();
    return TRUE;
  }

  return FALSE;
}
static int
equip_remove_curse()
{
  int l;
  struct objS* i_ptr;

  l = equip_cursed();
  if (l > -1) {
    i_ptr->flags &= ~TR_CURSED;
    calc_bonuses();
    return TRUE;
  }
  return FALSE;
}
static int
equip_disenchant()
{
  int flag, i;
  struct objS* obj;

  flag = FALSE;
  // INVEN_AUX is protected
  switch (randint(7)) {
    case 1:
      i = INVEN_WIELD;
      break;
    case 2:
      i = INVEN_BODY;
      break;
    case 3:
      i = INVEN_ARM;
      break;
    case 4:
      i = INVEN_OUTER;
      break;
    case 5:
      i = INVEN_HANDS;
      break;
    case 6:
      i = INVEN_HEAD;
      break;
    case 7:
      i = INVEN_FEET;
      break;
  }
  obj = obj_get(invenD[i]);

  // Weapons may lose tohit/todam. Armor may lose toac.
  // Ego weapon toac is protected.
  // Gauntlets of Slaying tohit/todam are protected.
  if (i == INVEN_WIELD) {
    flag = (obj->tohit > 0 || obj->todam > 0);
    obj->tohit -= randint(2);
    obj->todam = MAX(obj->todam - randint(2), 0);
  } else {
    flag = (obj->toac > 0);
    obj->toac = MAX(obj->toac - randint(2), 0);
  }
  if (flag) {
    msg_print("There is a static feeling in the air.");
    calc_bonuses();
  }

  return flag;
}
static int
inven_random()
{
  int k, tmp[INVEN_EQUIP];

  k = 0;
  for (int it = 0; it < INVEN_EQUIP; ++it) {
    if (invenD[it]) tmp[k++] = invenD[it];
  }

  return tmp[randint(k) - 1];
}
static int
inven_food()
{
  for (int it = 0; it < INVEN_EQUIP; ++it) {
    struct objS* obj = obj_get(invenD[it]);
    if (obj->tval == TV_FOOD) return it;
  }
  return -1;
}
static void
inven_ident(iidx)
{
  struct objS* obj;
  struct treasureS* t_ptr;

  obj = obj_get(invenD[iidx]);
  t_ptr = &treasureD[obj->tidx];
  tr_make_known(t_ptr);
  obj->idflag |= ID_REVEAL;
  obj_desc(obj, TRUE);
  if (iidx >= INVEN_EQUIP) {
    calc_bonuses();
    MSG("%s: %s", describe_use(iidx), descD);
  } else {
    MSG("%c %s", iidx + 'a', descD);
  }
}
static int
tohit_enchant(amount)
{
  int affect;
  struct objS* i_ptr = obj_get(invenD[INVEN_WIELD]);

  if (i_ptr->tval != TV_NOTHING) {
    obj_desc(i_ptr, FALSE);
    affect = 0;
    for (int it = 0; it < amount; ++it) {
      affect += (enchant(&i_ptr->tohit, 10));
    }
    if (affect) {
      MSG("Your %s glows %s!", descD, affect > 1 ? "brightly" : "faintly");
      i_ptr->flags &= ~TR_CURSED;
      calc_bonuses();
    } else
      msg_print("The enchantment fails.");
    return TRUE;
  }

  return FALSE;
}
static int
todam_enchant(amount)
{
  int affect, limit;
  struct objS* i_ptr = obj_get(invenD[INVEN_WIELD]);

  if (i_ptr->tval != TV_NOTHING) {
    obj_desc(i_ptr, FALSE);
    if ((i_ptr->tval >= TV_HAFTED) && (i_ptr->tval <= TV_DIGGING))
      limit = i_ptr->damage[0] * i_ptr->damage[1];
    else /* Bows' and arrows' enchantments should not be limited
            by their low base damages */
      limit = 10;

    affect = 0;
    for (int it = 0; it < amount; ++it) {
      affect += (enchant(&i_ptr->todam, limit));
    }

    if (affect) {
      MSG("Your %s glows %s!", descD, affect > 1 ? "brightly" : "faintly");
      i_ptr->flags &= ~TR_CURSED;
      calc_bonuses();
    } else
      msg_print("The enchantment fails.");
    return TRUE;
  }

  return FALSE;
}
void py_move_light(y1, x1, y2, x2) int y1, x1, y2, x2;
{
  int row, col;
  for (row = y1 - 1; row <= y1 + 1; ++row) {
    for (col = x1 - 1; col <= x1 + 1; ++col) {
      caveD[row][col].cflag &= ~CF_TEMP_LIGHT;
    }
  }

  for (row = y2 - 1; row <= y2 + 1; ++row) {
    for (col = x2 - 1; col <= x2 + 1; ++col) {
      struct caveS* cave = &caveD[row][col];
      if (cave->fval >= MIN_WALL)
        cave->cflag |= CF_PERM_LIGHT;
      else
        caveD[row][col].cflag |= CF_TEMP_LIGHT;
      if (cave->oidx) {
        struct objS* obj = &entity_objD[cave->oidx];
        if (obj->tval >= TV_MIN_VISIBLE && obj->tval <= TV_MAX_VISIBLE) {
          cave->cflag |= CF_FIELDMARK;
        }
      }
    }
  }
}
int
py_class_select()
{
  char c;
  int line, it;
  line = 0;
  for (int it = 0; it < AL(classD); ++it) {
    overlay_usedD[line] = snprintf(overlayD[line], AL(overlayD[line]), "%c) %s",
                                   'a' + it, classD[it].name);
    line += 1;
  }

  platform_draw();
  if (in_subcommand("Which character would you like to play?", &c)) {
    int iidx = c - 'a';
    if (iidx > 0 && iidx < AL(classD)) return iidx;
  }
  AC(overlayD);
  AC(overlay_usedD);

  return 0;
}
static void py_stats(stats, len) int8_t* stats;
int len;
{
  register int i, tot;
  int dice[18];

  do {
    tot = 0;
    for (i = 0; i < 18; i++) {
      dice[i] = randint(3 + i % 3); /* Roll 3,4,5 sided dice once each */
      tot += dice[i];
    }
  } while (tot <= 42 || tot >= 54);

  for (i = 0; i < len; i++)
    stats[i] = 5 + dice[3 * i] + dice[3 * i + 1] + dice[3 * i + 2];
}
void
py_init()
{
  int it, hitdie;
  int csel, rsel;
  int8_t stat[MAX_A];

  csel = 0;  // py_class_select();
  do {
    rsel = randint(AL(raceD)) - 1;
  } while ((raceD[rsel].rtclass & (1 << csel)) == 0);

  // Race & class
  int ridx = rsel;
  struct raceS* r_ptr = &raceD[ridx];
  hitdie = r_ptr->bhitdie;
  uD.ridx = ridx;
  uD.bth = r_ptr->bth;
  uD.search = r_ptr->srh;
  uD.fos = r_ptr->fos;
  uD.disarm = r_ptr->dis;
  uD.stealth = r_ptr->stl;
  uD.mult_exp = r_ptr->b_exp;
  int male = 1 - randint(2);
  if (male) {
    uD.wt = randnor(r_ptr->m_b_wt, r_ptr->m_m_wt);
  } else {
    uD.wt = randnor(r_ptr->f_b_wt, r_ptr->f_m_wt);
  }
  uD.male = male;

  py_stats(stat, AL(stat));

  int clidx = csel;
  struct classS* cl_ptr = &classD[clidx];
  hitdie += cl_ptr->adj_hd;
  uD.clidx = clidx;
  uD.bth += cl_ptr->mbth;
  uD.search += cl_ptr->msrh;
  uD.fos += cl_ptr->mfos;
  uD.disarm += cl_ptr->mdis;
  uD.stealth += cl_ptr->mstl;
  uD.save += cl_ptr->msav;
  uD.mult_exp += cl_ptr->m_exp;

  for (int it = 0; it < MAX_A; ++it) {
    stat[it] += cl_ptr->mattr[it];
  }

  memcpy(statD.max_stat, AP(stat));
  memcpy(statD.cur_stat, AP(stat));
  AC(statD.mod_stat);
  memcpy(statD.use_stat, AP(stat));

  // TEMP: Test delta cur/max
  for (int it = 0; it < MAX_A; ++it) {
    statD.max_stat[it] += 1;
  }

  uD.mhp = uD.chp = hitdie + con_adj();
  uD.chp_frac = 0;
  uD.lev = 1;

  int min_value = (MAX_PLAYER_LEVEL * 3 / 8 * (hitdie - 1)) + MAX_PLAYER_LEVEL;
  int max_value = (MAX_PLAYER_LEVEL * 5 / 8 * (hitdie - 1)) + MAX_PLAYER_LEVEL;
  player_hpD[0] = hitdie;
  do {
    for (it = 1; it < MAX_PLAYER_LEVEL; it++) {
      player_hpD[it] = randint(hitdie);
      player_hpD[it] += player_hpD[it - 1];
    }
  } while ((player_hpD[MAX_PLAYER_LEVEL - 1] < min_value) ||
           (player_hpD[MAX_PLAYER_LEVEL - 1] > max_value));

  // Starting equipment..
  struct objS* dagger = obj_use();
  tr_obj_copy(30, dagger);
  dagger->idflag |= ID_REVEAL;
  invenD[INVEN_WIELD] = dagger->id;
  for (int it = 0; it < 1; ++it) {
    struct objS* food = obj_use();
    // 22 = 1 ration of food
    tr_obj_copy(22, food);
    invenD[it] = food->id;
  }
  int actuate_test[] = {214, 190};  //{238, 185, 314};
  for (int it = 0; it < AL(actuate_test); ++it) {
    int iidx = INVEN_EQUIP - 3 + it;
    if (iidx >= INVEN_EQUIP) break;
    struct objS* obj = obj_use();
    tr_obj_copy(actuate_test[it], obj);
    invenD[iidx] = obj->id;
  }
  int magik_test[] = {91};
  for (int it = 0; it < AL(magik_test); ++it) {
    struct objS* obj = obj_use();
    tr_obj_copy(actuate_test[it], obj);
    do {
      magic_treasure(obj, dun_level);
    } while ((TR_SPEED & obj->flags) == 0);
  }

  calc_bonuses();

  uD.food = 7500;
  uD.food_digest = 2;
}
void
magic_init()
{
  int i, j, k, h;
  char string[80];
  for (h = 0; h < AL(titleD); h++) {
    string[0] = 0;
    k = randint(2) + 1;
    for (i = 0; i < k; i++) {
      for (j = randint(2); j > 0; j--)
        strcat(string, syllableD[randint(AL(syllableD)) - 1]);
      if (i < k - 1) strcat(string, " ");
    }
    if (string[8] == ' ')
      string[8] = '\0';
    else
      string[9] = '\0';
    strcat(titleD[h], string);
  }
}
int
dec_stat(stat)
register int stat;
{
  int tmp_stat, loss;

  tmp_stat = statD.cur_stat[stat];
  if (tmp_stat > 3) {
    if (tmp_stat < 19)
      tmp_stat--;
    else if (tmp_stat < 117) {
      loss = (((118 - tmp_stat) >> 1) + 1) >> 1;
      tmp_stat += -randint(loss) - loss;
      if (tmp_stat < 18) tmp_stat = 18;
    } else
      tmp_stat--;

    statD.cur_stat[stat] = tmp_stat;
    set_use_stat(stat);
    return TRUE;
  }

  return FALSE;
}
int
inc_stat(stat)
{
  int tmp_stat, gain;

  tmp_stat = statD.cur_stat[stat];
  if (tmp_stat < 118) {
    if (tmp_stat < 18)
      tmp_stat++;
    else if (tmp_stat < 116) {
      /* stat increases by 1/6 to 1/3 of difference from max */
      gain = ((118 - tmp_stat) / 3 + 1) >> 1;
      tmp_stat += randint(gain) + gain;
    } else
      tmp_stat++;

    statD.cur_stat[stat] = tmp_stat;
    if (tmp_stat > statD.max_stat[stat]) statD.max_stat[stat] = tmp_stat;
    set_use_stat(stat);
    return TRUE;
  } else
    return FALSE;
}
int
res_stat(stat)
{
  if (statD.max_stat[stat] != statD.cur_stat[stat]) {
    statD.cur_stat[stat] = statD.max_stat[stat];
    set_use_stat(stat);
    return TRUE;
  }
  return FALSE;
}
static void
py_where()
{
  int y, x, dir;
  while (get_dir("Map: Look which direction?", &dir)) {
    mmove(dir, &panelD.panel_row, &panelD.panel_col);
    if (panelD.panel_row > MAX_ROW - 2) panelD.panel_row = MAX_ROW - 2;
    if (panelD.panel_col > MAX_COL - 2) panelD.panel_col = MAX_COL - 2;
    panel_bounds(&panelD);
    symmap_update();
    platform_draw();
  }
  panel_update(&panelD, uD.y, uD.x, TRUE);
  free_turn_flag = TRUE;
}
static int
py_inven_filter(begin, end, valid)
int begin, end;
int (*valid)();
{
  int line = 0;

  for (int it = begin; it < end; ++it) {
    int obj_id = invenD[it];
    int len = 1;
    overlayD[line][0] = ' ';
    struct objS* obj = obj_get(obj_id);
    if (valid(obj)) {
      obj_desc(obj, TRUE);
      len = snprintf(overlayD[line], AL(overlayD[line]), "%c) %s",
                     'a' + it - begin, descD);
    }

    overlay_usedD[line] = len;
    line += 1;
  }
  return line;
}
static int
py_heal_hit(num)
{
  int res;

  res = FALSE;
  if (uD.chp < uD.mhp) {
    uD.chp += num;
    if (uD.chp > uD.mhp) {
      uD.chp = uD.mhp;
      uD.chp_frac = 0;
    }

    num = num / 5;
    if (num < 3) {
      if (num == 0)
        msg_print("You feel a little better.");
      else
        msg_print("You feel better.");
    } else {
      if (num < 7)
        msg_print("You feel much better.");
      else
        msg_print("You feel very good.");
    }
    res = TRUE;
  }
  return (res);
}
void
py_experience()
{
  int exp = uD.exp;
  int lev = uD.lev;
  int expfact = uD.mult_exp;

  if (exp > MAX_EXP) exp = MAX_EXP;

  while ((lev < MAX_PLAYER_LEVEL) &&
         (player_exp[lev - 1] * expfact / 100) <= exp) {
    int dif_exp, need_exp;

    lev += 1;
    MSG("Welcome to level %d.", lev);
    calc_hitpoints(lev);

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

  uD.exp = exp;
  uD.max_exp = MAX(exp, uD.max_exp);
  uD.lev = lev;
}
int
restore_level()
{
  int restore, lev, exp, expfact;

  restore = FALSE;
  if (uD.max_exp > uD.exp) {
    restore = TRUE;
    msg_print("You feel your life energies returning.");
    exp = uD.max_exp;
    expfact = uD.mult_exp;
    lev = uD.lev;
    while ((lev < MAX_PLAYER_LEVEL) &&
           (player_exp[lev - 1] * expfact / 100) <= exp) {
      lev += 1;
      MSG("Welcome to level %d.", lev);
    }
    uD.exp = exp;
    uD.lev = lev;
  }
  return (restore);
}
void
py_lose_experience(amount)
{
  int lev, exp;
  struct classS* c_ptr;

  exp = MAX(uD.exp - amount, 0);

  lev = 0;
  while ((player_exp[lev] * uD.mult_exp / 100) <= exp) lev++;
  /* increment lev once more, because level 1 exp is stored in player_exp[0] */
  lev++;

  uD.exp = exp;
  if (uD.lev != lev) {
    uD.lev = lev;

    calc_hitpoints(lev);
    c_ptr = &classD[uD.clidx];
    // if (c_ptr->spell == MAGE) {
    //   calc_spells(A_INT);
    //   calc_mana(A_INT);
    // } else if (c_ptr->spell == PRIEST) {
    //   calc_spells(A_WIS);
    //   calc_mana(A_WIS);
    // }
    // prt_level();
    // prt_title();
  }
}
void py_teleport(dis, uy, ux) int dis;
int *uy, *ux;
{
  int ty, tx, y, x;

  y = uD.y;
  x = uD.x;
  do {
    ty = randint(MAX_HEIGHT - 2);
    tx = randint(MAX_WIDTH - 2);
    while (distance(ty, tx, y, x) > dis) {
      ty += ((y - ty) / 2);
      tx += ((x - tx) / 2);
    }
  } while ((caveD[ty][tx].fval >= MIN_CLOSED_SPACE) ||
           (caveD[ty][tx].midx != 0));
  *uy = ty;
  *ux = tx;
}
void teleport_away(midx, dis) int midx, dis;
{
  int fy, fx, yn, xn, ctr;
  struct monS* m_ptr;

  m_ptr = &entity_monD[midx];
  fy = m_ptr->fy;
  fx = m_ptr->fx;
  ctr = 0;
  do {
    do {
      yn = fy + (randint(2 * dis + 1) - (dis + 1));
      xn = fx + (randint(2 * dis + 1) - (dis + 1));
    } while (!in_bounds(yn, xn));
    ctr++;
    if (ctr > 9) {
      ctr = 0;
      dis += 5;
    }
  } while ((caveD[yn][xn].fval >= MIN_CLOSED_SPACE) ||
           (caveD[yn][xn].midx != 0));
  move_rec(fy, fx, yn, xn);
  m_ptr->fy = yn;
  m_ptr->fx = xn;
  SYMMAP_PATCH(fy, fx);

  m_ptr->ml = FALSE;
  m_ptr->cdis = distance(uD.y, uD.x, yn, xn);
  update_mon(midx);
}
void
get_flags(int typ, uint32_t* weapon_type, int* harm_type, int (**destroy)())
{
  // TBD:
  *destroy = set_null;

  switch (typ) {
    case GF_MAGIC_MISSILE:
      *weapon_type = 0;
      *harm_type = 0;
      //*destroy = set_null;
      break;
    case GF_LIGHTNING:
      *weapon_type = CS_BR_LIGHT;
      *harm_type = CD_LIGHT;
      //*destroy = set_lightning_destroy;
      break;
    case GF_POISON_GAS:
      *weapon_type = CS_BR_GAS;
      *harm_type = CD_POISON;
      //*destroy = set_null;
      break;
    case GF_ACID:
      *weapon_type = CS_BR_ACID;
      *harm_type = CD_ACID;
      //*destroy = set_acid_destroy;
      break;
    case GF_FROST:
      *weapon_type = CS_BR_FROST;
      *harm_type = CD_FROST;
      //*destroy = set_frost_destroy;
      break;
    case GF_FIRE:
      *weapon_type = CS_BR_FIRE;
      *harm_type = CD_FIRE;
      //*destroy = set_fire_destroy;
      break;
    case GF_HOLY_ORB:
      *weapon_type = 0;
      *harm_type = CD_EVIL;
      //*destroy = set_null;
      break;
    default:
      msg_print("ERROR in get_flags()\n");
  }
}
void fire_bolt(typ, dir, y, x, dam, bolt_typ) int typ, dir, y, x, dam;
char* bolt_typ;
{
  int i, oldy, oldx, dist, flag;
  uint32_t weapon_type;
  int harm_type;
  int (*dummy)();
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cre;

  flag = FALSE;
  get_flags(typ, &weapon_type, &harm_type, &dummy);
  oldy = y;
  oldx = x;
  dist = 0;
  do {
    mmove(dir, &y, &x);
    dist++;
    c_ptr = &caveD[y][x];
    // lite_spot(oldy, oldx);
    if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
      flag = TRUE;
    else {
      if (c_ptr->midx) {
        flag = TRUE;
        m_ptr = &entity_monD[c_ptr->midx];
        cre = &creatureD[m_ptr->cidx];

        /* light up monster and draw monster, temporarily set
           pl so that update_mon() will work */
        i = c_ptr->cflag;
        c_ptr->cflag |= CF_PERM_LIGHT;
        update_mon(c_ptr->midx);
        c_ptr->cflag = i;
        /* draw monster and clear previous bolt */
        // put_qio();

        // TBD: lowercase
        mon_desc(c_ptr->midx);
        MSG("The %s strikes %s.", bolt_typ, descD);
        if (harm_type & cre->cdefense) {
          dam = dam * 2;
          // if (m_ptr->ml) c_recall[m_ptr->mptr].r_cdefense |= harm_type;
        } else if (weapon_type & cre->spells) {
          dam = dam / 4;
          // if (m_ptr->ml) c_recall[m_ptr->mptr].r_spells |= weapon_type;
        }
        mon_desc(c_ptr->midx);
        i = mon_take_hit(c_ptr->midx, dam);
        if (i >= 0) {
          MSG("%s dies in a fit of agony.", descD);
          py_experience();
        } else if (dam > 0) {
          MSG("%s screams in agony.", descD);
        }
      } else if (panel_contains(&panelD, y, x)) {
        // TBD: && (py.flags.blind < 1)
        // print('*', y, x);
        /* show the bolt */
        // put_qio();
      }
    }
    oldy = y;
    oldx = x;
  } while (!flag);
}
static int
py_inven(begin, end)
int begin, end;
{
  int line, count;

  line = count = 0;
  for (int it = begin; it < end; ++it) {
    int obj_id = invenD[it];
    int len = 1;
    overlayD[line][0] = ' ';

    if (obj_id) {
      count += 1;
      struct objS* obj = obj_get(obj_id);
      obj_desc(obj, TRUE);
      len = snprintf(overlayD[line], AL(overlayD[line]), "%c) %s",
                     'a' + it - begin, descD);
    }

    overlay_usedD[line] = len;
    line += 1;
  }
  return count;
}
int
weapon_curse()
{
  struct objS* i_ptr = obj_get(invenD[INVEN_WIELD]);
  if (i_ptr->tval != TV_NOTHING) {
    obj_desc(i_ptr, FALSE);
    MSG("Your %s glows black, fades.", descD);
    i_ptr->name2 = 0;
    i_ptr->tohit = -randint(5) - randint(5);
    i_ptr->todam = -randint(5) - randint(5);
    i_ptr->toac = 0;

    /* Must call py_bonuses() before set (clear) flags, and
       must call calc_bonuses() after set (clear) flags, so that
       all attributes will be properly turned off. */
    py_bonuses(i_ptr, -1);
    i_ptr->flags = TR_CURSED;
    calc_bonuses();
    return TRUE;
  }

  return FALSE;
}
static int
choice(char* prompt)
{
  char c;
  int count = py_inven(0, INVEN_EQUIP);
  if (count) {
    platform_draw();
    if (in_subcommand(prompt, &c)) {
      int iidx = c - 'a';
      if (iidx < MAX_INVEN) return iidx;
    }
  } else
    msg_print("You are not carrying anything!");
  return -1;
}
void
inven_eat(iidx)
{
  struct objS* obj = obj_get(invenD[iidx]);
  if (obj->tval == TV_FOOD) {
    uD.food = CLAMP(uD.food + obj->p1, 0, 15000);
    inven_destroy_one(iidx);
    msg_print("nom nom nom!!");
  } else {
    msg_print("You can't eat that!");
  }
}
int
inven_quaff(iidx)
{
  BOOL ident;
  uint32_t i, j;
  struct objS* obj = obj_get(invenD[iidx]);
  if (obj->tval == TV_POTION1 || obj->tval == TV_POTION2) {
    uD.food = CLAMP(uD.food + obj->p1, 0, 15000);
    inven_destroy_one(iidx);

    i = obj->flags;
    MSG("flags %d", i);
    if (i == 0) msg_print("You feel less thirsty.");
    while (i != 0) {
      j = bit_pos(&i) + 1;
      j += (obj->tval == TV_POTION2) * 32;
      switch (j) {
        case 1:
          if (inc_stat(A_STR)) {
            msg_print("Wow!  What bulging muscles!");
            ident = TRUE;
          }
          break;
        case 2:
          ident = TRUE;
          // TBD: was lose_str(), diff?
          dec_stat(A_STR);
          break;
        case 3:
          if (res_stat(A_STR)) {
            msg_print("You feel warm all over.");
            ident = TRUE;
          }
          break;
        case 4:
          if (inc_stat(A_INT)) {
            msg_print("Aren't you brilliant!");
            ident = TRUE;
          }
          break;
        case 5:
          ident = TRUE;
          // TBD: was lose_int();
          dec_stat(A_INT);
          break;
        case 6:
          if (res_stat(A_INT)) {
            msg_print("You have have a warm feeling.");
            ident = TRUE;
          }
          break;
        case 7:
          if (inc_stat(A_WIS)) {
            msg_print("You suddenly have a profound thought!");
            ident = TRUE;
          }
          break;
        case 8:
          ident = TRUE;
          // TBD: lose_wis();
          dec_stat(A_WIS);
          break;
        case 9:
          if (res_stat(A_WIS)) {
            msg_print("You feel your wisdom returning.");
            ident = TRUE;
          }
          break;
        case 10:
          if (inc_stat(A_CHR)) {
            msg_print("Gee, ain't you cute!");
            ident = TRUE;
          }
          break;
        case 11:
          ident = TRUE;
          // lose_chr();
          dec_stat(A_CHR);
          break;
        case 12:
          if (res_stat(A_CHR)) {
            msg_print("You feel your looks returning.");
            ident = TRUE;
          }
          break;
        case 13:
          ident = py_heal_hit(damroll(2, 7));
          break;
        case 14:
          ident = py_heal_hit(damroll(4, 7));
          break;
        case 15:
          ident = py_heal_hit(damroll(6, 7));
          break;
        case 16:
          ident = py_heal_hit(1000);
          break;
        case 17:
          if (inc_stat(A_CON)) {
            msg_print("You feel tingly for a moment.");
            ident = TRUE;
          }
          break;
        case 18:
          if (uD.exp < MAX_EXP) {
            int l = (uD.exp / 2) + 10;
            if (l > 100000L) l = 100000L;
            uD.exp += l;
            msg_print("You feel more experienced.");
            py_experience();
            ident = TRUE;
          }
          break;
        // case 19:
        //   f_ptr = &py.flags;
        //   if (!f_ptr->free_act) {
        //     /* paralysis must == 0, otherwise could not drink potion */
        //     msg_print("You fall asleep.");
        //     f_ptr->paralysis += randint(4) + 4;
        //     ident = TRUE;
        //   }
        //   break;
        // case 20:
        //   f_ptr = &py.flags;
        //   if (f_ptr->blind == 0) {
        //     msg_print("You are covered by a veil of darkness.");
        //     ident = TRUE;
        //   }
        //   f_ptr->blind += randint(100) + 100;
        //   break;
        // case 21:
        //   f_ptr = &py.flags;
        //   if (f_ptr->confused == 0) {
        //     msg_print("Hey!  This is good stuff!  * Hick! *");
        //     ident = TRUE;
        //   }
        //   f_ptr->confused += randint(20) + 12;
        //   break;
        // case 22:
        //   f_ptr = &py.flags;
        //   if (f_ptr->poisoned == 0) {
        //     msg_print("You feel very sick.");
        //     ident = TRUE;
        //   }
        //   f_ptr->poisoned += randint(15) + 10;
        //   break;
        // case 23:
        //   if (py.flags.fast == 0) ident = TRUE;
        //   py.flags.fast += randint(25) + 15;
        //   break;
        // case 24:
        //   if (py.flags.slow == 0) ident = TRUE;
        //   py.flags.slow += randint(25) + 15;
        //   break;
        case 26:
          if (inc_stat(A_DEX)) {
            msg_print("You feel more limber!");
            ident = TRUE;
          }
          break;
        case 27:
          if (res_stat(A_DEX)) {
            msg_print("You feel less clumsy.");
            ident = TRUE;
          }
          break;
        case 28:
          if (res_stat(A_CON)) {
            msg_print("You feel your health returning!");
            ident = TRUE;
          }
          break;
        // case 29:
        //   ident = cure_blindness();
        //   break;
        // case 30:
        //   ident = cure_confusion();
        //   break;
        // case 31:
        //   ident = cure_poison();
        //   break;
        case 34:
          if (uD.exp > 0) {
            int m, scale;
            msg_print("You feel your memories fade.");
            /* Lose between 1/5 and 2/5 of your experience */
            m = uD.exp / 5;
            if (uD.exp > INT16_MAX) {
              scale = INT32_MAX / uD.exp;
              m += (randint(scale) * uD.exp) / (scale * 5);
            } else
              m += randint(uD.exp) / 5;
            py_lose_experience(m);
            ident = TRUE;
          }
          break;
          // case 35:
          //   f_ptr = &py.flags;
          //   (void)cure_poison();
          //   if (f_ptr->food > 150) f_ptr->food = 150;
          //   f_ptr->paralysis = 4;
          //   msg_print("The potion makes you vomit!");
          //   ident = TRUE;
          //   break;
          // case 36:
          //   if (py.flags.invuln == 0) ident = TRUE;
          //   py.flags.invuln += randint(10) + 10;
          //   break;
          // case 37:
          //   if (py.flags.hero == 0) ident = TRUE;
          //   py.flags.hero += randint(25) + 25;
          //   break;
          // case 38:
          //   if (py.flags.shero == 0) ident = TRUE;
          //   py.flags.shero += randint(25) + 25;
          //   break;
          // case 39:
          //   ident = remove_fear();
          //   break;
        case 40:
          ident = restore_level();
          break;
        // case 41:
        //   f_ptr = &py.flags;
        //   if (f_ptr->resist_heat == 0) ident = TRUE;
        //   f_ptr->resist_heat += randint(10) + 10;
        //   break;
        // case 42:
        //   f_ptr = &py.flags;
        //   if (f_ptr->resist_cold == 0) ident = TRUE;
        //   f_ptr->resist_cold += randint(10) + 10;
        //   break;
        // case 43:
        //   if (py.flags.detect_inv == 0) ident = TRUE;
        //   detect_inv2(randint(12) + 12);
        //   break;
        // case 44:
        //   ident = slow_poison();
        //   break;
        // case 45:
        //   ident = cure_poison();
        //   break;
        // case 46:
        //   m_ptr = &py.misc;
        //   if (m_ptr->cmana < m_ptr->mana) {
        //     m_ptr->cmana = m_ptr->mana;
        //     ident = TRUE;
        //     msg_print("Your feel your head clear.");
        //     prt_cmana();
        //   }
        //   break;
        // case 47:
        //   f_ptr = &py.flags;
        //   if (f_ptr->tim_infra == 0) {
        //     msg_print("Your eyes begin to tingle.");
        //     ident = TRUE;
        //   }
        //   f_ptr->tim_infra += 100 + randint(100);
        //   break;
        default:
          msg_print("Internal error in potion()");
          break;
      }
    }

    return TRUE;
  }

  return FALSE;
}
int
inven_read(iidx, uy, ux)
int *uy, *ux;
{
  uint32_t i;
  int j, k, y, x;
  int flag, used_up;
  register int ident, l;
  struct objS* i_ptr;
  struct treasureS* t_ptr;

  i_ptr = obj_get(invenD[iidx]);
  t_ptr = &treasureD[i_ptr->tidx];
  if (i_ptr->tval == TV_SCROLL1 || i_ptr->tval == TV_SCROLL2) {
    free_turn_flag = FALSE;
    used_up = TRUE;
    i = i_ptr->flags;
    ident = FALSE;

    while (i != 0) {
      j = bit_pos(&i) + 1;
      if (i_ptr->tval == TV_SCROLL2) j += 32;

      /* Scrolls.  		*/
      switch (j) {
        case 1:
          ident = tohit_enchant(1);
          break;
        case 2:
          ident = todam_enchant(1);
          break;
        case 3:
          ident = equip_enchant(1);
          break;
        case 4:
          msg_print("This is an identify scroll.");
          ident = TRUE;
          l = choice("Which item do you wish identified?");
          if (l >= 0) inven_ident(l);
          used_up = TRUE;
          break;
        case 5:
          if (equip_remove_curse()) {
            msg_print("You feel as if someone is watching over you.");
            ident = TRUE;
          }
          break;
        case 6:
          ident = light_area(uD.y, uD.x);
          break;
        case 7:
          for (k = 0; k < randint(3); k++) {
            ident |= summon_monster(uD.y, uD.x, FALSE);
          }
          break;
        case 8:
          py_teleport(10, uy, ux);
          ident = TRUE;
          break;
        case 9:
          py_teleport(100, uy, ux);
          ident = TRUE;
          break;
        case 10:
          dun_level += (-3) + 2 * randint(2);
          if (dun_level < 1) dun_level = 1;
          new_level_flag = TRUE;
          ident = TRUE;
          break;
          // case 11:
          //   if (py.flags.confuse_monster == 0) {
          //     msg_print("Your hands begin to glow.");
          //     py.flags.confuse_monster = TRUE;
          //     ident = TRUE;
          //   }
          //   break;
        case 12:
          ident = TRUE;
          map_area();
          break;
        // case 13:
        //   ident = sleep_monsters1(char_row, char_col);
        //   break;
        // case 14:
        //   ident = TRUE;
        //   warding_glyph();
        //   break;
        // case 15:
        //   ident = detect_treasure();
        //   break;
        // case 16:
        //   ident = detect_object();
        //   break;
        // case 17:
        //   ident = detect_trap();
        //   break;
        // case 18:
        //   ident = detect_sdoor();
        //   break;
        // case 19:
        //   msg_print("This is a mass genocide scroll.");
        //   (void)mass_genocide();
        //   ident = TRUE;
        //   break;
        // case 20:
        //   ident = detect_invisible();
        //   break;
        // case 21:
        //   msg_print("There is a high pitched humming noise.");
        //   (void)aggravate_monster(20);
        //   ident = TRUE;
        //   break;
        // case 22:
        //   ident = trap_creation();
        //   break;
        // case 23:
        //   ident = td_destroy();
        //   break;
        // case 24:
        //   ident = door_creation();
        //   break;
        // case 25:
        //   msg_print("This is a Recharge-Item scroll.");
        //   ident = TRUE;
        //   used_up = recharge(60);
        //   break;
        // case 26:
        //   msg_print("This is a genocide scroll.");
        //   (void)genocide();
        //   ident = TRUE;
        //   break;
        case 27:
          ident = unlight_area(uD.y, uD.x);
          break;
          // case 28:
          //   ident = protect_evil();
          //   break;
          // case 29:
          //   ident = TRUE;
          //   create_food();
          //   break;
          // case 30:
          //   ident = dispel_creature(CD_UNDEAD, 60);
          //   break;
        case 33:
          ident = tohit_enchant(randint(2));
          ident |= todam_enchant(randint(2));
          break;
        case 34:
          ident = weapon_curse();
          break;
        case 35:
          k = randint(2) + 1;
          ident = equip_enchant(k);
          break;
        case 36:
          ident = equip_curse();
          break;
        case 37:
          for (k = 0; k < randint(3); k++) {
            ident |= summon_undead(uD.y, uD.x);
          }
          break;
        // case 38:
        //  ident = TRUE;
        //  bless(randint(12) + 6);
        //  break;
        // case 39:
        //  ident = TRUE;
        //  bless(randint(24) + 12);
        //  break;
        // case 40:
        //  ident = TRUE;
        //  bless(randint(48) + 24);
        //  break;
        // case 41:
        //   ident = TRUE;
        //   if (py.flags.word_recall == 0)
        //     py.flags.word_recall = 25 + randint(30);
        //   msg_print("The air about you becomes charged.");
        //   break;
        // case 42:
        //   destroy_area(char_row, char_col);
        //   ident = TRUE;
        //   break;
        default:
          msg_print("Internal error in scroll()");
          break;
      }
      /* End of Scrolls.  		       */
    }
    if (ident) {
      if (!tr_known(t_ptr)) {
        /* round half-way case up */
        // TDB: xp tuning
        uD.exp += (i_ptr->level + (uD.lev >> 1)) / uD.lev;
        py_experience();

        tr_make_known(t_ptr);
      }
    }
    // else if (!tr_known(t_ptr))
    //   sample(i_ptr);
    if (used_up) {
      i_ptr->number -= 1;
      obj_desc(i_ptr, TRUE);
      i_ptr->number += 1;
      MSG("You have %s", descD);
      inven_destroy_one(iidx);
    }
    return TRUE;
  }

  return FALSE;
}
int
inven_zap_dir(iidx, dir)
{
  uint32_t flags, j;
  int y, x, ident, chance;
  struct objS* i_ptr;
  struct treasureS* t_ptr;

  y = uD.y;
  x = uD.x;
  i_ptr = obj_get(invenD[iidx]);
  t_ptr = &treasureD[i_ptr->tidx];
  ident = FALSE;
  chance = uD.save + think_adj(A_INT) - (int)i_ptr->level +
           (level_adj[uD.clidx][LA_DEVICE] * uD.lev / 3);
  // if (py.flags.confused > 0) chance = chance / 2;
  if ((chance < USE_DEVICE) && (randint(USE_DEVICE - chance + 1) == 1))
    chance = USE_DEVICE; /* Give everyone a slight chance */
  if (chance <= 0) chance = 1;
  if (randint(chance) < USE_DEVICE)
    msg_print("You failed to use the wand properly.");
  else if (i_ptr->p1 > 0) {
    flags = i_ptr->flags;
    (i_ptr->p1)--;
    while (flags != 0) {
      j = bit_pos(&flags) + 1;
      /* Wands  		 */
      switch (j) {
        // case 1:
        //   msg_print("A line of blue shimmering light appears.");
        //   light_line(dir, char_row, char_col);
        //   ident = TRUE;
        //   break;
        case 2:
          fire_bolt(GF_LIGHTNING, dir, y, x, damroll(4, 8), spell_nameD[8]);
          ident = TRUE;
          break;
        case 3:
          fire_bolt(GF_FROST, dir, y, x, damroll(6, 8), spell_nameD[14]);
          ident = TRUE;
          break;
        case 4:
          fire_bolt(GF_FIRE, dir, y, x, damroll(9, 8), spell_nameD[22]);
          ident = TRUE;
          break;
        // case 5:
        //   ident = wall_to_mud(dir, y, x);
        //   break;
        // case 6:
        //   ident = poly_monster(dir, y, x);
        //   break;
        // case 7:
        //   ident = hp_monster(dir, y, x, -damroll(4, 6));
        //   break;
        // case 8:
        //   ident = speed_monster(dir, y, x, 1);
        //   break;
        // case 9:
        //   ident = speed_monster(dir, y, x, -1);
        //   break;
        // case 10:
        //   ident = confuse_monster(dir, y, x);
        //   break;
        // case 11:
        //   ident = sleep_monster(dir, y, x);
        //   break;
        // case 12:
        //   ident = drain_life(dir, y, x);
        //   break;
        // case 13:
        //   ident = td_destroy2(dir, y, x);
        //   break;
        case 14:
          fire_bolt(GF_MAGIC_MISSILE, dir, y, x, damroll(2, 6), spell_nameD[0]);
          ident = TRUE;
          break;
        // case 15:
        //   ident = build_wall(dir, y, x);
        //   break;
        // case 16:
        //   ident = clone_monster(dir, y, x);
        //   break;
        // case 17:
        //   ident = teleport_monster(dir, y, x);
        //   break;
        // case 18:
        //   ident = disarm_all(dir, y, x);
        //   break;
        // case 19:
        //   fire_ball(GF_LIGHTNING, dir, y, x, 32, "Lightning Ball");
        //   ident = TRUE;
        //   break;
        // case 20:
        //   fire_ball(GF_FROST, dir, y, x, 48, "Cold Ball");
        //   ident = TRUE;
        //   break;
        // case 21:
        //   fire_ball(GF_FIRE, dir, y, x, 72, spell_nameD[28]);
        //   ident = TRUE;
        //   break;
        // case 22:
        //   fire_ball(GF_POISON_GAS, dir, y, x, 12, spell_nameD[6]);
        //   ident = TRUE;
        //   break;
        // case 23:
        //   fire_ball(GF_ACID, dir, y, x, 60, "Acid Ball");
        //   ident = TRUE;
        //   break;
        // case 24:
        //   flags = 1L << (randint(23) - 1);
        //   break;
        default:
          msg_print("Internal error in wands()");
          break;
      }
      /* End of Wands.  	    */
    }
    if (ident) {
      if (!tr_known(t_ptr)) {
        /* round half-way case up */
        // TBD: tuning
        uD.exp += (i_ptr->level + (uD.lev >> 1)) / uD.lev;
        py_experience();

        tr_make_known(t_ptr);
      }
    }
    // else if (!known1_p(i_ptr))
    //   sample(i_ptr);
    if (i_ptr->idflag & ID_REVEAL)
      MSG("You have %d charges remaining.", i_ptr->p1);
  } else {
    msg_print("The wand has no charges left.");
    i_ptr->idflag |= ID_EMPTY;
  }
  return ident;
}
void
choice_actuate()
{
  char c;
  struct objS* obj;

  int count = py_inven_filter(0, INVEN_EQUIP, oset_actuate);
  platform_draw();
  if (count) {
    if (in_subcommand("Use what?", &c)) {
      int iidx = c - 'a';
      if (iidx < INVEN_EQUIP) {
        struct objS* obj = obj_get(invenD[iidx]);
        if (obj->tval == TV_FOOD) {
          inven_eat(iidx);
        } else if (obj->tval == TV_POTION1 || obj->tval == TV_POTION2) {
          MSG("would quaff %d", iidx);
        } else if (obj->tval == TV_SCROLL1 || obj->tval == TV_SCROLL2) {
          MSG("would read %d", iidx);
        } else if (obj->tval == TV_STAFF) {
          MSG("would Zap_staff %d", iidx);
        }
      }
    }
  }
}
void
py_wear()
{
  char c;
  struct objS* obj;

  int count = py_inven(0, INVEN_EQUIP);
  platform_draw();
  if (count) {
    if (in_subcommand("Wear/Wield which item?", &c)) {
      int iidx = c - 'a';
      if (iidx < INVEN_EQUIP) {
        obj = obj_get(invenD[iidx]);
        int slot = slot_equip(obj->tval);
        if (slot > 0) {
          int slot_count = (slot == INVEN_RING) ? 2 : 1;
          // TBD: Replace equipped item?
          for (int it = 0; it < slot_count; ++it) {
            if (invenD[slot + it] == 0) {
              invenD[slot] = obj->id;
              invenD[iidx] = 0;

              py_bonuses(obj, 1);
              obj_desc(obj, TRUE);
              MSG("You are wearing %s.", descD);
            }
          }
        }
      }
    }
  }
  calc_bonuses();
}
void
py_zap(iidx)
{
  int dir;

  free_turn_flag = FALSE;
  if (get_dir(0, &dir)) {
    // if (py.flags.confused > 0) {
    //   msg_print("You are confused.");
    //   do {
    //     dir = randint(9);
    //   } while (dir == 5);
    // }
    inven_zap_dir(iidx, dir);
  }
}
void inven_invoke(iidx, uy, ux) int *uy, *ux;
{
  uint32_t flags, j;
  int k, chance;
  int ident;
  struct objS* i_ptr;
  struct treasureS* t_ptr;

  i_ptr = obj_get(invenD[iidx]);
  t_ptr = &treasureD[i_ptr->tidx];
  free_turn_flag = FALSE;
  chance = uD.save + think_adj(A_INT) - i_ptr->level - 5 +
           (level_adj[uD.clidx][LA_DEVICE] * uD.lev / 3);
  // if (py.flags.confused > 0) chance = chance / 2;
  if ((chance < USE_DEVICE) && (randint(USE_DEVICE - chance + 1) == 1))
    chance = USE_DEVICE; /* Give everyone a slight chance */
  if (chance <= 0) chance = 1;
  if (randint(chance) < USE_DEVICE)
    msg_print("You failed to use the staff properly.");
  else if (i_ptr->p1 > 0) {
    flags = i_ptr->flags;
    ident = FALSE;
    (i_ptr->p1)--;
    while (flags != 0) {
      j = bit_pos(&flags) + 1;
      /* Staffs.  			*/
      switch (j) {
        case 1:
          ident = light_area(uD.y, uD.x);
          break;
          // case 2:
          //   ident = detect_sdoor();
          //   break;
          // case 3:
          //   ident = detect_trap();
          //   break;
          // case 4:
          //   ident = detect_treasure();
          //   break;
          // case 5:
          //   ident = detect_object();
          //   break;
        case 6:
          py_teleport(100, uy, ux);
          ident = TRUE;
          break;
          // case 7:
          //   ident = TRUE;
          //   earthquake();
          //   break;
        case 8:
          ident = FALSE;
          for (k = 0; k < randint(4); k++) {
            ident |= summon_monster(uD.y, uD.x, FALSE);
          }
          break;
          // case 10:
          //   ident = TRUE;
          //   destroy_area(char_row, char_col);
          //   break;
          // case 11:
          //   ident = TRUE;
          //   starlite(char_row, char_col);
          //   break;
          // case 12:
          //   ident = speed_monsters(1);
          //   break;
          // case 13:
          //   ident = speed_monsters(-1);
          //   break;
          // case 14:
          //   ident = sleep_monsters2();
          //   break;
        case 15:
          ident = py_heal_hit(randint(8));
          break;
          // case 16:
          //   ident = detect_invisible();
          //   break;
          // case 17:
          //   if (py.flags.fast == 0) ident = TRUE;
          //   py.flags.fast += randint(30) + 15;
          //   break;
          // case 18:
          //   if (py.flags.slow == 0) ident = TRUE;
          //   py.flags.slow += randint(30) + 15;
          //   break;
          // case 19:
          //   ident = mass_poly();
          //   break;
        case 20:
          if (equip_remove_curse()) {
            // if (py.flags.blind < 1)
            msg_print("The staff glows blue for a moment..");
            ident = TRUE;
          }
          break;
        // case 21:
        //   ident = detect_evil();
        //   break;
        // case 22:
        //   if ((cure_blindness()) || (cure_poison()) || (cure_confusion()))
        //     ident = TRUE;
        //   break;
        // case 23:
        //   ident = dispel_creature(CD_EVIL, 60);
        //   break;
        case 25:
          ident = unlight_area(uD.y, uD.x);
          break;
        case 32:
          /* store bought flag */
          break;
        default:
          msg_print("Internal error in staffs()");
          break;
      }
      /* End of staff actions.  	*/
    }
    if (ident) {
      if (!tr_known(t_ptr)) {
        /* round half-way case up */
        uD.exp += (i_ptr->level + (uD.lev >> 1)) / uD.lev;
        py_experience();

        tr_make_known(t_ptr);
      }
    }
    // else if (!known1_p(i_ptr))
    //   sample(i_ptr);
    if (i_ptr->idflag & ID_REVEAL)
      MSG("You have %d charges remaining.", i_ptr->p1);
  } else {
    msg_print("The staff has no charges left.");
    i_ptr->idflag |= ID_EMPTY;
  }
}
static void py_drop(y, x) int y, x;
{
  char c;
  struct caveS* c_ptr;
  struct objS* obj;

  c_ptr = &caveD[y][x];
  if (c_ptr->oidx != 0) {
    MSG("There is already an object on the ground here.");
    return;
  }

  int count = py_inven(0, INVEN_EQUIP);
  if (!count) {
    MSG("You aren't carrying anything");
    return;
  }
  platform_draw();

  if (in_subcommand("Drop which item?", &c)) {
    int iidx = c - 'a';
    if (iidx < INVEN_EQUIP) {
      obj = obj_get(invenD[iidx]);
      obj->fy = y;
      obj->fx = x;
      c_ptr->oidx = obj_index(obj);
      invenD[iidx] = 0;
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
static int
inven_merge(obj_id)
{
  int subval;
  struct objS* obj = obj_get(obj_id);

  subval = obj->subval;
  if (subval >= ITEM_SINGLE_STACK) {
    for (int it = 0; it < INVEN_EQUIP; ++it) {
      struct objS* i_ptr = obj_get(invenD[it]);
      if (i_ptr->subval == subval && i_ptr->number < 255) {
        obj->number += i_ptr->number;
        obj_unuse(i_ptr);
        invenD[it] = obj_id;
        return it;
      }
    }
  }
  return -1;
}
static int
inven_carry(obj_id)
int obj_id;
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
enum { RATIO = (MAX_WIDTH / SYMMAP_WIDTH) };
#define TL 0 /* top left */
#define TR 1
#define BL 2
#define BR 3
#define HE 4 /* horizontal edge */
#define VE 5
#define CH(x) (screen_border[0][x])
void
py_map()
{
  register int i, j;
  static uint8_t screen_border[][6] = {
      {'+', '+', '+', '+', '-', '|'}, /* normal chars */
  };
  uint8_t map[MAX_WIDTH / RATIO + 1];
  uint8_t tmp;
  int priority[256];
  int row, orow, col, myrow, mycol = 0;
  char* iter;

  for (i = 0; i < 256; i++) priority[i] = 0;
  priority['<'] = 5;
  priority['>'] = 5;
  priority['@'] = 10;
  priority[(unsigned char)240] = -5;
  priority['.'] = -10;
  priority['\''] = -3;
  priority[' '] = -15;

  iter = screenD[0];
  *iter++ = CH(TL);
  for (int it = 0; it < SYMMAP_WIDTH; ++it) *iter++ = CH(HE);
  *iter++ = CH(TR);
  screen_usedD[0] = (SYMMAP_WIDTH + 2);

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
  for (int it = 0; it < SYMMAP_WIDTH; ++it) *iter++ = CH(HE);
  *iter++ = CH(BR);
  screen_usedD[orow + 2] = (SYMMAP_WIDTH + 2);

  free_turn_flag = TRUE;
}

void
py_character()
{
  int line, col, col_width;
  col_width = 20;
  line = 0;
  col = 0;

  BufMsg(screen, "%-17.017s: %s", "Name", "...");
  BufMsg(screen, "%-17.017s: %s", "Race", raceD[uD.ridx].name);
  BufMsg(screen, "%-17.017s: %s", "Gender", uD.male ? "Male" : "Female");
  BufMsg(screen, "%-17.017s: %s", "Class", classD[uD.clidx].name);

  BufPad(screen, MAX_A, 38);

  line = 0;
  BufMsg(screen, "%-13.013s: %d", "Age", 16);
  BufMsg(screen, "%-13.013s: %d", "Height", 74);
  BufMsg(screen, "%-13.013s: %d", "Weight", uD.wt);
  BufMsg(screen, "%-13.013s: %d", "Social Class", 1);

  BufPad(screen, MAX_A, 61);

  line = 0;
  for (int it = 0; it < MAX_A; ++it) {
    BufMsg(screen, "%-4.04s:%5d  %d", stat_nameD[it], statD.use_stat[it],
           statD.max_stat[it]);
  }
  line += 1;

  BufMsg(screen, "ToHit: %+6d", uD.ptohit);
  BufMsg(screen, "ToDam: %+6d", uD.ptodam);
  BufMsg(screen, "Ac   : %6d", uD.pac);
  BufMsg(screen, "ToAc : %+6d", uD.ptoac);
  free_turn_flag = TRUE;
}
void
py_takeoff()
{
  char c;
  struct objS* obj;

  int carry_count = py_carry_count();
  int equip_count = py_inven(INVEN_EQUIP, MAX_INVEN);
  platform_draw();
  if (carry_count && equip_count) {
    if (in_subcommand("Take off which item?", &c)) {
      int iidx = INVEN_EQUIP + (c - 'a');
      if (iidx < MAX_INVEN) {
        obj = obj_get(invenD[iidx]);
        if (inven_carry(obj->id) >= 0) {
          invenD[iidx] = 0;

          py_bonuses(obj, -1);
          obj_desc(obj, TRUE);
          MSG("You take off %s.", descD);
        } else {
          msg_print("You don't have room in your inventory.");
        }
      }
    }
  }
  calc_bonuses();
}
static void
py_help()
{
  int line = 0;
  BufMsg(screen, "? - help");
  BufMsg(screen, ",: pickup object");
  BufMsg(screen, "c: close object");
  BufMsg(screen, "d: drop object");
  BufMsg(screen, "e: equipment");
  BufMsg(screen, "f: force Bash");
  BufMsg(screen, "i: inventory");
  BufMsg(screen, "q: quaff potion");
  BufMsg(screen, "r: read scroll");
  BufMsg(screen, "o: open object");
  BufMsg(screen, "s: search for traps/doors");
  BufMsg(screen, "w: wear object");
  BufMsg(screen, "z: zap wand");
  BufMsg(screen, "C: character screen");
  BufMsg(screen, "D: disarm trap");
  BufMsg(screen, "E: eat object");
  BufMsg(screen, "M: map dungeon");
  BufMsg(screen, "R: rest until healed");
  BufMsg(screen, "T: take off equipment");
  BufMsg(screen, "W: where about the dungeon");
  BufMsg(screen, "Z: staff invocation");

  BufPad(screen, AL(screenD), 30);

  line = 1;
  BufMsg(screen, "<: up stairs");
  BufMsg(screen, ">: down stairs");
  BufMsg(screen, "CTRL('f'): food (cheat)");
  BufMsg(screen, "CTRL('h'): heal (cheat)");
  BufMsg(screen, "CTRL('t'): teleport (cheat)");
  BufMsg(screen, "CTRL('m'): teleport-to-monster (cheat)");
  BufMsg(screen, "CTRL('o'): teleport-to-object (cheat)");
  BufMsg(screen, ".: object interaction (experimental)");
  BufMsg(screen, "m: make_known (test_cmd)");
}
static void py_pickup(y, x, pickup) int y, x;
int pickup;
{
  struct caveS* c_ptr;
  struct objS* obj;
  int locn;

  c_ptr = &caveD[y][x];
  obj = &entity_objD[c_ptr->oidx];
  obj_desc(obj, TRUE);

  if (obj->tval == 0) {
    msg_print("You see nothing here.");
    free_turn_flag = TRUE;
  }
  /* There's GOLD in them thar hills!      */
  else if (obj->tval == TV_GOLD) {
    uD.gold += obj->cost;
    MSG("You have found %d gold pieces worth of %s.", obj->cost,
        gold_nameD[obj->subval]);
    delete_object(y, x);
  } else if (obj->tval <= TV_MAX_PICK_UP) {
    if (pickup) {
      if ((locn = inven_merge(obj->id)) >= 0 ||
          (locn = inven_carry(obj->id)) >= 0) {
        MSG("You pickup %s (%c)", descD, locn + 'a');
        obj->fy = 0;
        obj->fx = 0;
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
int inven_damage(typ, perc) int (*typ)();
register int perc;
{
  register int it, j;

  j = 0;
  for (it = 0; it < INVEN_EQUIP; it++) {
    struct objS* obj = obj_get(invenD[it]);
    if ((*typ)(obj) && (randint(100) < perc)) {
      // TBD: Single stack items may decrement obj->number
      invenD[it] = 0;
      obj_unuse(obj);
      j++;
    }
  }
  return (j);
}
int
minus_ac(typ_dam)
uint32_t typ_dam;
{
  register int i, j;
  int tmp[6], minus;
  struct objS* obj;

  i = 0;
  if (invenD[INVEN_BODY]) {
    tmp[i] = INVEN_BODY;
    i++;
  }
  if (invenD[INVEN_ARM]) {
    tmp[i] = INVEN_ARM;
    i++;
  }
  if (invenD[INVEN_OUTER]) {
    tmp[i] = INVEN_OUTER;
    i++;
  }
  if (invenD[INVEN_HANDS]) {
    tmp[i] = INVEN_HANDS;
    i++;
  }
  if (invenD[INVEN_HEAD]) {
    tmp[i] = INVEN_HEAD;
    i++;
  }
  if (invenD[INVEN_FEET]) {
    tmp[i] = INVEN_FEET;
    i++;
  }
  minus = FALSE;
  if (i > 0) {
    j = tmp[randint(i) - 1];
    obj = obj_get(invenD[j]);
    obj_desc(obj, FALSE);
    if (obj->flags & typ_dam) {
      MSG("Your %s resists damage!", descD);
      minus = TRUE;
    } else if ((obj->ac + obj->toac) > 0) {
      MSG("Your %s is damaged!", descD);
      obj->toac--;
      calc_bonuses();
      minus = TRUE;
    }
  }
  return (minus);
}
void fire_dam(dam) int dam;
{
  // TBD: Resistance
  // if (py.flags.fire_resist) dam = dam / 3;
  // if (py.flags.resist_heat > 0) dam = dam / 3;
  py_take_hit(dam);
  if (inven_damage(vuln_fire, 3) > 0)
    msg_print("There is smoke coming from your pack!");
}
void acid_dam(dam) int dam;
{
  register int flag;

  flag = 0;
  if (minus_ac(TR_RES_ACID)) flag = 1;
  // TBD: Resistance
  // if (py.flags.acid_resist) flag += 2;
  py_take_hit(dam / (flag + 1));
  if (inven_damage(vuln_acid, 3) > 0)
    msg_print("There is an acrid smell coming from your pack!");
}
void cold_dam(dam) int dam;
{
  // if (py.flags.cold_resist) dam = dam / 3;
  // if (py.flags.resist_cold > 0) dam = dam / 3;
  py_take_hit(dam);
  if (inven_damage(vuln_frost, 5) > 0)
    msg_print("Something shatters inside your pack!");
}
void light_dam(dam) int dam;
{
  // if (py.flags.lght_resist)
  //   take_hit((dam / 3), kb_str);
  // else
  py_take_hit(dam);
  if (inven_damage(vuln_lightning, 3) > 0)
    msg_print("There are sparks coming from your pack!");
}
void
corrode_gas()
{
  if (!minus_ac(TR_RES_ACID)) py_take_hit(randint(8));
  if (inven_damage(vuln_gas, 5) > 0)
    msg_print("There is an acrid smell coming from your pack.");
}
int
attack_blows(weight, wtohit)
int weight;
int* wtohit;
{
  register int adj_weight;
  register int str_index, dex_index, s, d;

  s = statD.use_stat[A_STR];
  d = statD.use_stat[A_DEX];
  if (s * 15 < weight) {
    *wtohit = s * 15 - weight;
    return 1;
  } else {
    *wtohit = 0;
    if (d < 10)
      dex_index = 0;
    else if (d < 19)
      dex_index = 1;
    else if (d < 68)
      dex_index = 2;
    else if (d < 108)
      dex_index = 3;
    else if (d < 118)
      dex_index = 4;
    else
      dex_index = 5;
    adj_weight = (s * 10 / weight);
    if (adj_weight < 2)
      str_index = 0;
    else if (adj_weight < 3)
      str_index = 1;
    else if (adj_weight < 4)
      str_index = 2;
    else if (adj_weight < 5)
      str_index = 3;
    else if (adj_weight < 7)
      str_index = 4;
    else if (adj_weight < 9)
      str_index = 5;
    else
      str_index = 6;
    return blows_table[str_index][dex_index];
  }
}
void py_attack(y, x) int y, x;
{
  register int k, blows;
  int base_tohit, tohit;

  int midx = caveD[y][x].midx;
  struct monS* mon = &entity_monD[midx];
  struct creatureS* cre = &creatureD[mon->cidx];
  struct objS* obj = obj_get(invenD[INVEN_WIELD]);

  mon_desc(midx);
  descD[0] = tolower(descD[0]);

  switch (obj->tval) {
    case 0:
      blows = 2;
      tohit = -3;
      break;
    case TV_SLING_AMMO:
    case TV_BOLT:
    case TV_ARROW:
    case TV_SPIKE:
      blows = 1;
      tohit = 0;
      break;
    default:
      blows = attack_blows(obj->weight, &tohit);
      break;
  }
  // TBD: reduce hit if monster not lit
  base_tohit = uD.bth;

  int adj = uD.lev * level_adj[uD.clidx][LA_BTH];
  int creature_ac = cre->ac;
  /* Loop for number of blows,  trying to hit the critter.	  */
  for (int it = 0; it < blows; ++it) {
    if (test_hit(uD.bth, adj, tohit, creature_ac)) {
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
  int bth;
  struct monS* mon = &entity_monD[midx];
  struct creatureS* cre = &creatureD[mon->cidx];

  mon_desc(midx);
  int adj = cre->level * CRE_LEV_ADJ;
  int uac = uD.pac + uD.ptoac;
  for (int it = 0; it < AL(cre->attack_list); ++it) {
    if (death) break;
    if (!cre->attack_list[it]) break;
    struct attackS* attack = &attackD[cre->attack_list[it]];

    int attack_type = attack->attack_type;
    int attack_desc = attack->attack_desc;
    bth = bth_adj(attack_type);
    disturb(1, 0);
    int flag = FALSE;
    if (test_hit(bth, adj, 0, uac)) flag = TRUE;
    if (flag) {
      int damage = damroll(attack->attack_dice, attack->attack_sides);
      MSG("%s%s", descD, attack_string(attack_desc));
      switch (attack_type) {
        case 1: /*Normal attack  */
                /* round half-way case down */
          damage -= (uac * damage) / 200;
          py_take_hit(damage);
          break;
        case 2: /*Lose Strength*/
          py_take_hit(damage);
          // if (py.flags.sustain_str)
          //  msg_print("You feel weaker for a moment, but it passes.");
          // else if (randint(2) == 1) {
          msg_print("You feel weaker.");
          dec_stat(A_STR);
          // }
          break;
        case 3: /*Confusion attack*/
          py_take_hit(damage);
          // f_ptr = &py.flags;
          // if (randint(2) == 1) {
          //  if (f_ptr->confused < 1) {
          //    msg_print("You feel confused.");
          //    f_ptr->confused += randint((int)r_ptr->level);
          //  } else
          //    notice = FALSE;
          //  f_ptr->confused += 3;
          //} else
          //  notice = FALSE;
          break;
        case 4: /*Fear attack  */
          py_take_hit(damage);
          // f_ptr = &py.flags;
          // if (player_saves())
          //  msg_print("You resist the effects!");
          // else if (f_ptr->afraid < 1) {
          //  msg_print("You are suddenly afraid!");
          //  f_ptr->afraid += 3 + randint((int)r_ptr->level);
          //} else {
          //  f_ptr->afraid += 3;
          //  notice = FALSE;
          //}
          break;
        case 5: /*Fire attack  */
          msg_print("You are enveloped in flames!");
          fire_dam(damage);
          break;
        case 6: /*Acid attack  */
          msg_print("You are covered in acid!");
          acid_dam(damage);
          break;
        case 7: /*Cold attack  */
          msg_print("You are covered with frost!");
          cold_dam(damage);
          break;
        case 8: /*Lightning attack*/
          msg_print("Lightning strikes you!");
          light_dam(damage);
          break;
        case 9: /*Corrosion attack*/
          msg_print("A stinging red gas swirls about you.");
          corrode_gas();
          py_take_hit(damage);
          break;
        case 10: /*Blindness attack*/
          py_take_hit(damage);
          // f_ptr = &py.flags;
          // if (f_ptr->blind < 1) {
          //  f_ptr->blind += 10 + randint((int)r_ptr->level);
          //  msg_print("Your eyes begin to sting.");
          //} else {
          //  f_ptr->blind += 5;
          //  notice = FALSE;
          //}
          break;
        case 11: /*Paralysis attack*/
          py_take_hit(damage);
          // f_ptr = &py.flags;
          // if (player_saves())
          //  msg_print("You resist the effects!");
          // else if (f_ptr->paralysis < 1) {
          //  if (f_ptr->free_act)
          //    msg_print("You are unaffected.");
          //  else {
          //    f_ptr->paralysis = randint((int)r_ptr->level) + 3;
          //    msg_print("You are paralyzed.");
          //  }
          //} else
          //  notice = FALSE;
          break;
        case 12: /*Steal Money    */
                 // if ((py.flags.paralysis < 1) &&
          if (randint(124) < statD.use_stat[A_DEX])
            msg_print("You quickly protect your money pouch!");
          else {
            int gold = (uD.gold / 10) + randint(25);
            uD.gold = MAX(uD.gold - gold, 0);
            msg_print("Your purse feels lighter.");
          }
          if (randint(2) == 1) {
            msg_print("There is a puff of smoke!");
            teleport_away(midx, MAX_SIGHT);
          }
          break;
        case 13: /*Steal Object   */
          // if ((py.flags.paralysis < 1) &&
          if (randint(124) < statD.use_stat[A_DEX])
            msg_print("You grab hold of your backpack!");
          else {
            int i = inven_random();
            inven_destroy_one(i);
            msg_print("Your backpack feels lighter.");
          }
          if (randint(2) == 1) {
            msg_print("There is a puff of smoke!");
            teleport_away(midx, MAX_SIGHT);
          }
          break;
        case 14: /*Poison   */
          py_take_hit(damage);
          msg_print("You feel very sick.");
          cD.poison += randint(cre->level) + 5;
          break;
        case 15: /*Lose dexterity */
          py_take_hit(damage);
          // f_ptr = &py.flags;
          // if (f_ptr->sustain_dex)
          //  msg_print("You feel clumsy for a moment, but it passes.");
          // else {
          msg_print("You feel more clumsy.");
          dec_stat(A_DEX);
          //}
          break;
        case 16: /*Lose constitution */
          py_take_hit(damage);
          // f_ptr = &py.flags;
          // if (f_ptr->sustain_con)
          //  msg_print("Your body resists the effects of the disease.");
          // else {
          msg_print("Your health is damaged!");
          dec_stat(A_CON);
          //}
          break;
        case 17: /*Lose intelligence */
          py_take_hit(damage);
          // f_ptr = &py.flags;
          msg_print("You have trouble thinking clearly.");
          // if (f_ptr->sustain_int)
          //  msg_print("But your mind quickly clears.");
          // else
          dec_stat(A_INT);
          break;
        case 18: /*Lose wisdom     */
          py_take_hit(damage);
          // f_ptr = &py.flags;
          // if (f_ptr->sustain_wis)
          //  msg_print("Your wisdom is sustained.");
          // else {
          msg_print("Your wisdom is drained.");
          dec_stat(A_WIS);
          //}
          break;
        case 19: /*Lose experience  */
          msg_print("You feel your life draining away!");
          py_lose_experience(damage + (uD.exp / 100) * MON_DRAIN_LIFE);
          break;
        case 20: /*Aggravate monster*/
          // aggravate_monster(20);
          break;
        case 21: /*Disenchant     */
          equip_disenchant();
          break;
        case 22: /*Eat food     */
        {
          int l = inven_food();
          if (l >= 0) {
            inven_destroy_one(l);
            msg_print("It got at your rations!");
          }
        } break;
        case 23: /*Eat light     */
        {
          struct objS* obj = obj_get(invenD[INVEN_LIGHT]);
          if (obj->p1 > 0) {
            obj->p1 = MAX(obj->p1 - 250 + randint(250), 1);
            // if (py.flags.blind < 1)
            msg_print("Your light dims.");
            // else
            //   notice = FALSE;
          }
        } break;
        case 24: /*Eat charges    */
        {
          int iidx = inven_random();
          struct objS* obj = obj_get(invenD[iidx]);
          if (oset_zap(obj) && obj->p1 > 0) {
            mon->hp += cre->level * obj->p1;
            obj->p1 = 0;
            obj->idflag |= ID_EMPTY;
            msg_print("Energy drains from your pack!");
          }
        } break;
      }
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
static int
bash(y, x)
{
  int tmp, movement;
  struct caveS* c_ptr;
  struct objS* obj;

  c_ptr = &caveD[y][x];
  obj = &entity_objD[c_ptr->oidx];

  movement = 0;
  if (c_ptr->midx) {
    // TBD: py_bash() vs monster
    py_attack(y, x);
  } else if (c_ptr->oidx) {
    if (obj->tval == TV_CLOSED_DOOR) {
      msg_print("You smash into the door!");
      tmp = statD.use_stat[A_STR] + uD.wt / 2;
      /* Use (roughly) similar method as for monsters. */
      if (randint(tmp * (20 + ABS(obj->p1))) < 10 * (tmp - ABS(obj->p1))) {
        msg_print("The door crashes open!");
        // invcopy(&t_list[c_ptr->tptr], OBJ_OPEN_DOOR);
        obj->tval = TV_OPEN_DOOR;
        obj->tchar = '\'';
        obj->p1 = 1 - randint(2); /* 50% chance of breaking door */
        c_ptr->fval = FLOOR_CORR;
        // if (py.flags.confused == 0)
        movement = 1;
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
  {
    msg_print("You bash it, but nothing interesting happens.");
  }

  return movement;
}
static void py_bash(uy, ux) int *uy, *ux;
{
  int y, x, dir;

  y = uD.y;
  x = uD.x;
  if (get_dir(0, &dir)) {
    // if (py.flags.confused > 0) ...
    mmove(dir, &y, &x);
    if (bash(y, x)) {
      *uy = y;
      *ux = x;
    }
  }
}
static void
open_object(y, x)
{
  int i;
  struct caveS* c_ptr;
  struct objS* obj;
  c_ptr = &caveD[y][x];
  obj = &entity_objD[c_ptr->oidx];

  if (obj->tval == TV_CLOSED_DOOR) {
    // Monster may be invisible and will retaliate
    if (c_ptr->midx) {
      msg_print("Something is in your way!");
    } else {
      /* Known to be locked */
      if ((obj->idflag & ID_REVEAL) && obj->p1 > 0) {
        i = uD.disarm + 2 * todis_adj() + think_adj(A_INT) +
            (level_adj[uD.clidx][LA_DISARM] * uD.lev / 3);
        // Could be the whiskey
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
      } else if (obj->p1 > 0) {
        msg_print("The door is locked.");
        obj->idflag |= ID_REVEAL;
      } else if (obj->p1 < 0) {
        msg_print("The door is stuck.");
        obj->idflag |= ID_REVEAL;
      }

      if (obj->p1 == 0) {
        msg_print("You open the door.");
        obj->tval = TV_OPEN_DOOR;
        obj->tchar = '\'';
        c_ptr->fval = FLOOR_CORR;
      }
    }
  } else {
    msg_print("I do not see anything you can open there.");
    free_turn_flag = TRUE;
  }
}
static void
py_open()
{
  int y, x, dir;

  y = uD.y;
  x = uD.x;
  if (get_dir(0, &dir)) {
    mmove(dir, &y, &x);
    open_object(y, x);
  }
}
static void
py_make_known()
{
  char c;
  struct objS* obj;

  // TBD: filter?
  int count = py_inven(0, INVEN_EQUIP);
  platform_draw();
  if (count) {
    if (in_subcommand("Make known which item?", &c)) {
      int iidx = c - 'a';
      if (iidx < INVEN_EQUIP) {
        obj = obj_get(invenD[iidx]);
        struct treasureS* tr_ptr = &treasureD[obj->tidx];
        tr_make_known(tr_ptr);
        obj->idflag |= ID_REVEAL;
      }
    }
  }
}
static void py_search(y, x, chance) int y, x, chance;
{
  register int i, j;
  struct caveS* c_ptr;
  struct objS* obj;

  msg_print("You search the area.");
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
          find_flag = FALSE;
        } else if (obj->tval == TV_INVIS_TRAP) {
          msg_print("You have found a trap.");
          obj->tval = TV_VIS_TRAP;
          obj->tchar = '^';
          find_flag = FALSE;
        }
      }
}
static void make_move(midx, mm, rcmove) int midx;
int* mm;
uint32_t* rcmove;
{
  int i, fy, fx, newy, newx, do_turn, do_move, stuck_door;
  register struct caveS* c_ptr;
  register struct monS* m_ptr;

  i = 0;
  do_turn = FALSE;
  do_move = FALSE;
  m_ptr = &entity_monD[midx];
  for (int i = 0; i < 5; ++i) {
    /* Get new position  	*/
    fy = newy = m_ptr->fy;
    fx = newx = m_ptr->fx;
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
        if (!m_ptr->ml) update_mon(midx);
        mon_attack(midx);
        do_move = FALSE;
        do_turn = TRUE;
      }
      /* Creature is attempting to move on other creature?     */
      else if (c_ptr->midx && c_ptr->midx != midx) {
        //   // TBD: Creatures can eat creatures
        do_move = FALSE;
      }
    }
    /* Creature has been allowed move.   */
    if (do_move) {
      /* Move creature record  	       */
      move_rec(fy, fx, newy, newx);
      SYMMAP_PATCH(fy, fx);
      SYMMAP_PATCH(newy, newx);
      m_ptr->ml = FALSE;
      m_ptr->fy = newy;
      m_ptr->fx = newx;
      m_ptr->cdis = distance(uD.y, uD.x, newy, newx);
      do_turn = TRUE;
    }
    if (do_turn) break;
  }
}
static void mon_move(midx, rcmove) int midx;
uint32_t* rcmove;
{
  int mm[9];
  get_moves(midx, mm);
  make_move(midx, mm, rcmove);
}
static int
movement_rate(speed)
int speed;
{
  if (speed > 0) {
    if (uD.rest != 0)
      return 1;
    else
      return speed;
  } else {
    /* speed must be negative here */
    return ((turnD % (2 - speed)) == 0);
  }
}
void creatures(move) int move;
{
  uint32_t rcmove;
  int move_count;

  FOR_EACH(mon, {
    struct creatureS* cr_ptr = &creatureD[mon->cidx];
    mon->cdis = distance(uD.y, uD.x, mon->fy, mon->fx);
    if (move) {
      if ((cr_ptr->cmove & CM_ATTACK_ONLY) == 0 || mon->cdis < 2) {
        move_count = movement_rate(cr_ptr->speed - 10 + uD.pspeed);
        for (; move_count > 0; --move_count) {
          mon_move(it_index, &rcmove);
        }
      }
    }
    update_mon(it_index);
  });
}
void
status_update()
{
  int line = 0;
  BufMsg(status, "CHP : %6d", uD.chp);
  BufMsg(status, "MHP : %6d", uD.mhp);
  BufMsg(status, "LEV : %6d", uD.lev);
  BufMsg(status, "EXP : %6d", uD.exp);
  line += 1;
  BufMsg(status, "GOLD: %6d", uD.gold);
  BufMsg(status, "AC  : %6d", uD.pac);

  BufPad(status, AL(statusD), AL(statusD[0]));
}
BOOL
py_teleport_near(y, x, uy, ux)
int y, x;
int *uy, *ux;
{
  for (int ro = y - 1; ro <= y + 1; ++ro) {
    for (int co = x - 1; co <= x + 1; ++co) {
      if (ro == y && co == x) continue;
      if ((caveD[ro][co].fval >= MIN_CLOSED_SPACE || caveD[ro][co].midx != 0)) {
        continue;
      }

      *uy = ro;
      *ux = co;
      MSG("Teleport near (%d, %d)", ro, co);
      return TRUE;
    }
  }

  return FALSE;
}
static void hit_trap(y, x) int y, x;
{
  int dam, tac;
  struct caveS* c_ptr;
  struct objS* obj;

  tac = uD.pac + uD.ptoac;
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
static void regenhp(percent) int percent;
{
  uint32_t new_value;
  int chp, mhp, chp_frac;

  mhp = uD.mhp;
  new_value = mhp * percent + PLAYER_REGEN_HPBASE + uD.chp_frac;
  chp = uD.chp + (new_value >> 16);
  chp_frac = (new_value & 0xFFFF);

  /* set frac to zero even if equal */
  if (chp >= mhp) {
    chp = mhp;
    chp_frac = 0;
  }

  uD.chp = chp;
  uD.chp_frac = chp_frac;
}
void
tick()
{
  int regen_amount;
  turnD += 1;

  if (uD.food < 0)
    regen_amount = 0;
  else if (uD.food < PLAYER_FOOD_FAINT)
    regen_amount = PLAYER_REGEN_FAINT;
  else if (uD.food < PLAYER_FOOD_WEAK)
    regen_amount = PLAYER_REGEN_WEAK;
  else
    regen_amount = PLAYER_REGEN_NORMAL;

  uD.food -= uD.food_digest;
  if (uD.food < 0) {
    strcpy(death_descD, "starvation");
    py_take_hit(-uD.food / 16);
    disturb(1, 0);
  }

  // if (uD.regenerate) regen_amount = regen_amount * 3 / 2;
  if (uD.rest != 0)  // || (py.flags.status & PY_SEARCH)
    regen_amount = regen_amount * 2;
  // if (p_ptr->cmana < p_ptr->mana) regenmana(regen_amount);

  if (cD.poison == 0)
    regenhp(regen_amount);
  else if (cD.poison > 0) {
    if (cD.poison == 1) {
      msg_print("You feel better.");
    } else {
      strcpy(death_descD, "poison");
      py_take_hit(poison_adj());
    }
    cD.poison -= 1;
    disturb(1, 0);
  }

  if (uD.rest < 0) {
    uD.rest += 1;
    if (uD.chp == uD.mhp) uD.rest = 0;
  }
}
void py_check_view(y, x) int y, x;
{
  panel_update(&panelD, y, x, TRUE);
  py_move_light(y, x, y, x);
  struct caveS* c_ptr = &caveD[y][x];
  if ((c_ptr->cflag & CF_PERM_LIGHT) == 0 && c_ptr->cflag & CF_ROOM) {
    if (near_light(y, x)) light_room(y, x);
  }
}
void
dungeon()
{
  int c, y, x, iidx;
  uint32_t dir;
  new_level_flag = FALSE;

  while (1) {
    tick();

    do {
      if (uD.rest != 0) break;
      panel_update(&panelD, uD.y, uD.x, FALSE);
      status_update();
      symmap_update();

      debug_usedD = snprintf(
          AP(debugD), "[%d,%d xy] [%d,%d quadrant] [%d turn] %d feet", uD.x,
          uD.y, panelD.panel_col, panelD.panel_row, turnD, dun_level * 50);
      platform_draw();
      AC(screen_usedD);
      AC(overlay_usedD);
      AC(status_usedD);
      log_countD = 0;
      if (!free_turn_flag) log_usedD = 0;
      free_turn_flag = FALSE;

      y = uD.y;
      x = uD.x;
      if (find_flag) {
        mmove(find_direction, &y, &x);
      } else {
        c = inkey();
        if (c == -1) break;
        prev_cmdD = c;

        // AWN: Period attempts auto-detection of a situational command
        if (c == '.') {
          struct caveS* c_ptr = &caveD[y][x];
          int tval = entity_objD[c_ptr->oidx].tval;
          c = 's';
          if (tval == TV_UP_STAIR)
            c = '<';
          else if (tval == TV_DOWN_STAIR)
            c = '>';
          else if (tval - 1 < TV_MAX_PICK_UP)
            c = ',';
        }

        dir = map_roguedir(c | 0x20) - '0';
        if (dir <= 9) {
          // Primary movement (lowercase)
          if (c & 0x20) {
            mmove(dir, &y, &x);
          } else  // Secondary movement (uppercase)
          {
            find_init(dir, &y, &x);
          }
        }

        switch (c) {
          case '?':
            py_help();
            break;
          case ' ':
            free_turn_flag = TRUE;
            break;
          case ',':
            py_pickup(y, x, TRUE);
            break;
          case '1' ... '9':
            MSG("Numlock is required for arrowkey movement");
            free_turn_flag = TRUE;
            break;
          case 'c':
            close_object();
            break;
          case 'd':
            py_drop(y, x);
            break;
          case 'e': {
            free_turn_flag = TRUE;
            int count = py_inven(INVEN_EQUIP, MAX_INVEN);
            MSG("You wearing %d items.", count);
          } break;
          case 'f':
            py_bash(&y, &x);
            break;
          case 'i': {
            free_turn_flag = TRUE;
            int count = py_inven(0, INVEN_EQUIP);
            MSG("You carrying %d items.", count);
          } break;
          case 'm':
            // TEMP: testing tr_make_known
            py_make_known();
            break;
          case 'q':
            iidx = choice("Quaff what?");
            if (iidx >= 0) inven_quaff(iidx);
            break;
          case 'r':
            iidx = choice("Read what?");
            if (iidx >= 0) inven_read(iidx, &y, &x);
            break;
          case 'o':
            py_open();
            break;
          case 's':
            py_search(y, x, 25);
            break;
          case 'w':
            py_wear();
            break;
          case 'z':
            iidx = choice("Aim which wand?");
            if (iidx >= 0) py_zap(iidx);
            break;
          case 'Z':
            iidx = choice("Invoke which staff?");
            if (iidx >= 0) inven_invoke(iidx, &y, &x);
            break;
          case '<':
            go_up();
            break;
          case '>':
            go_down();
            break;
          case 'A':
            // Generalized inventory interaction
            choice_actuate();
            break;
          case 'C':
            py_character();
            break;
          case 'D':
            disarm_trap(&y, &x);
            break;
          case 'E':
            iidx = choice("Eat what?");
            if (iidx >= 0) inven_eat(iidx);
            break;
          case 'M':
            py_map();
            break;
          case 'R':
            uD.rest = -9999;
            break;
          case 'T':
            py_takeoff();
            break;
          case 'W':
            py_where();
            break;
          case CTRL('f'): {
            struct objS* obj = obj_use();
            caveD[y][x].oidx = obj_index(obj);

            tr_obj_copy(22, obj);
            obj->fy = y;
            obj->fx = x;
            msg_print("Food rolls beneath your feet.");
          } break;
          case CTRL('h'):
            if (uD.mhp < 100) uD.mhp = 100;
            uD.chp = uD.mhp;
            msg_print("You are healed.");
            break;
          case CTRL('t'):
            msg_print("teleport");
            do {
              x = randint(MAX_WIDTH - 2);
              y = randint(MAX_HEIGHT - 2);
            } while (caveD[y][x].fval >= MIN_CLOSED_SPACE ||
                     caveD[y][x].midx != 0);
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
            static int y_obj_teleportD;
            static int x_obj_teleportD;
            int row, col;
            int fy, fx;
            fy = y_obj_teleportD;
            fx = x_obj_teleportD;

            for (row = 1; row < MAX_HEIGHT - 1; ++row) {
              for (col = 1; col < MAX_WIDTH - 1; ++col) {
                int oidx = caveD[row][col].oidx;
                if (!oidx) continue;
                struct objS* obj = &entity_objD[oidx];
                if (is_door(obj->tval)) continue;

                if (row * MAX_WIDTH + col <= fy * MAX_WIDTH + fx) continue;

                if (py_teleport_near(row, col, &y, &x)) {
                  MSG("Teleport to obj %d", oidx);
                  y_obj_teleportD = row;
                  x_obj_teleportD = col;
                  row = col = MAX(MAX_HEIGHT, MAX_WIDTH);
                }
              }
            }
            if (row == MAX_HEIGHT - 1 && col == MAX_WIDTH - 1) {
              y_obj_teleportD = x_obj_teleportD = 0;
              msg_print("Reset object teleport");
            }
          } break;
        }
      }

      if (uD.y != y || uD.x != x) {
        struct caveS* c_ptr = &caveD[y][x];

        if (find_flag && (c_ptr->midx || c_ptr->fval > MAX_OPEN_SPACE)) {
          find_flag = FALSE;
          free_turn_flag = TRUE;
        } else {
          struct objS* obj = &entity_objD[c_ptr->oidx];
          // doors known to be jammed are bashed prior to movement
          if (obj->tval == TV_CLOSED_DOOR) {
            if (obj->p1 < 0 && (obj->idflag & ID_REVEAL)) {
              bash(y, x);
            }
          }

          if (c_ptr->midx) {
            py_attack(y, x);
          } else if (c_ptr->fval <= MAX_OPEN_SPACE) {
            py_move_light(uD.y, uD.x, y, x);
            if ((c_ptr->cflag & CF_PERM_LIGHT) == 0 && c_ptr->cflag & CF_ROOM) {
              if (near_light(y, x)) light_room(y, x);
            }
            int oy = uD.y;
            int ox = uD.x;
            uD.y = y;
            uD.x = x;
            SYMMAP_PATCH(oy, ox);
            SYMMAP_PATCH(y, x);
            if (find_flag) find_event(y, x);
            if (obj->tval) {
              find_flag = FALSE;
              if (obj->tval == TV_INVIS_TRAP || obj->tval == TV_VIS_TRAP) {
                hit_trap(y, x);
              } else {
                py_pickup(y, x, FALSE);
              }
            }
          } else if (obj->tval == TV_CLOSED_DOOR) {
            open_object(y, x);
          } else {
            free_turn_flag = TRUE;
          }
        }
      }
    } while (free_turn_flag && !new_level_flag);

    if (new_level_flag) break;
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
  platform_init();

  dun_level = 1;
  mon_level_init();
  obj_level_init();
  generate_cave();
  py_init();
  magic_init();

  while (!death) {
    py_check_view(uD.y, uD.x);
    dungeon();

    if (!death) generate_cave();
  }

  platform_reset();
  return 0;
}
