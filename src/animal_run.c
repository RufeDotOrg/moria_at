#include "game.c"

enum { HACK = 0 };
enum { TEST = 0 };
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

#define MSG(x, ...)                                             \
  {                                                             \
    char vtype[80];                                             \
    int len = snprintf(vtype, sizeof(vtype), x, ##__VA_ARGS__); \
    msg_game(vtype, len);                                       \
  }

#define BufMsg(name, text, ...)                                         \
  {                                                                     \
    int used = name##_usedD[line];                                      \
    used += snprintf(name##D[line] + used, AL(name##D[0]) - used, text, \
                     ##__VA_ARGS__);                                    \
    name##_usedD[line++] = used;                                        \
  }
#define BufLineAppend(name, line, text, ...)                             \
  {                                                                      \
    int used, append;                                                    \
    used = name##_usedD[line];                                           \
    append = snprintf(name##D[line] + used, AL(name##D[0]) - used, text, \
                      ##__VA_ARGS__);                                    \
    if (append > 0) name##_usedD[line] += append;                        \
  }

#define BufPad(name, line, len)            \
  for (int it = 0; it < line; ++it) {      \
    while (name##_usedD[it] < len) {       \
      name##D[it][name##_usedD[it]] = ' '; \
      name##_usedD[it] += 1;               \
    }                                      \
  }

static char
inkey()
{
  char c;
  do {
    c = platformD.readansi();
    if (!RELEASE && c == CTRL('c')) {
      strcpy(death_descD, "quitting");
      death = 1;
      new_level_flag = TRUE;
      break;
    }
  } while (c == 0);
  return c;
}
void
vital_update()
{
  AC(vitalinfo_usedD);

  int line = 0;
  for (int it = 0; it < MAX_A; ++it) {
    BufMsg(vitalinfo, "%-4.04s: %6d", stat_nameD[it], statD.use_stat[it]);
  }
  line += 1;
  BufMsg(vitalinfo, "LEV : %6d", uD.lev);
  BufMsg(vitalinfo, "EXP : %6d", uD.exp);
  BufMsg(vitalinfo, "MHP : %6d", uD.mhp);
  BufMsg(vitalinfo, "CHP : %6d", uD.chp);
  line += 1;
  BufMsg(vitalinfo, "AC  : %6d", cbD.pac - cbD.hide_toac);
  BufMsg(vitalinfo, "GOLD: %6d", uD.gold);
  line += 1;
  if (HACK) {
    BufMsg(vitalinfo, "DEBUG");
    BufMsg(vitalinfo, "y,x :%3d,%3d", uD.y, uD.x);
    BufMsg(vitalinfo, "sec :%3d,%3d", panelD.panel_col, panelD.panel_row);
  }
  BufMsg(vitalinfo, "turn: %6d", turnD);

  BufPad(vitalinfo, AL(vitalinfoD), AL(vitalinfoD[0]));
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

  if (row == uD.y && col == uD.x)
    return '@';
  else if (countD.blind)
    return ' ';

  cave_ptr = &caveD[row][col];
  if (cave_ptr->midx) {
    struct monS* mon = &entity_monD[cave_ptr->midx];
    struct creatureS* creature = &creatureD[mon->cidx];
    if (mon->mlit) return creature->cchar;
  }
  if (((cave_ptr->cflag & CF_FIELDMARK) == 0) && !cave_lit(cave_ptr))
    return ' ';
  if (cave_ptr->oidx) {
    struct objS* obj = &entity_objD[cave_ptr->oidx];
    if (obj->tval != TV_INVIS_TRAP) return obj->tchar;
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
int
cave_obj(row, col)
{
  struct caveS* c_ptr;
  struct objS* obj;

  if (row != uD.y || col != uD.x) {
    c_ptr = &caveD[row][col];
    obj = &entity_objD[c_ptr->oidx];

    if (c_ptr->cflag & CF_FIELDMARK || c_ptr->cflag & CF_LIT) {
      switch (obj->tval) {
          // Misc
        case TV_MISC:
          if (obj->tchar == 's')
            return 25;
          else if (obj->tchar == '!')
            return 4;
          else  // '~'
            return 23;
          // Ranged
        case TV_SLING_AMMO:
        case TV_BOLT:
        case TV_ARROW:
          return 13;
        case TV_SPIKE:
          return 22;
        case TV_LIGHT:
          return 21;
        case TV_BOW:
          return 15;
          // Worn
        case TV_HAFTED:
          return 10;
        case TV_POLEARM:
          return 6;
        case TV_SWORD:
          return 14;
        case TV_DIGGING:
          return 10;
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_CLOAK:
        case TV_HELM:
          return 11;
        case TV_SHIELD:
          return 2;
        case TV_HARD_ARMOR:
          return 9;
        case TV_SOFT_ARMOR:
          return 1;
        case TV_AMULET:
          return 3;
        case TV_RING:
          return 7;
          // Activate
        case TV_STAFF:
          return 12;
        case TV_WAND:
          return 5;
        case TV_SCROLL1:
        case TV_SCROLL2:
          return 8;
        case TV_POTION1:
        case TV_POTION2:
        case TV_FLASK:
          return 4;
        case TV_FOOD:
          return 19;
        case TV_MAGIC_BOOK:
          return 18;
        case TV_PRAYER_BOOK:
          return 17;
          // Gold
          // TBD: copper/silver/gold/mithril/gems by subval
        case TV_GOLD:
          return 16;
      }
    }
  }
  return 0;
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
altmap_update()
{
  int rmin = panelD.panel_row_min;
  int rmax = panelD.panel_row_max;
  int cmin = panelD.panel_col_min;
  int cmax = panelD.panel_col_max;

  memset(cremapD, 0, sizeof(cremapD));
  memset(tremapD, 0, sizeof(tremapD));
  memset(litmapD, 0, sizeof(litmapD));

  if (countD.blind == 0) {
    uint16_t* cidx_ptr = &cremapD[0][0];
    for (int row = rmin; row < rmax; ++row) {
      for (int col = cmin; col < cmax; ++col) {
        struct caveS* cave_ptr = &caveD[row][col];
        if (cave_ptr->midx) {
          struct monS* mon = &entity_monD[cave_ptr->midx];
          if (mon->mlit) *cidx_ptr = mon->cidx;
        }
        cidx_ptr += 1;
      }
    }

    uint8_t* oidx_ptr = &tremapD[0][0];
    for (int row = rmin; row < rmax; ++row)
      for (int col = cmin; col < cmax; ++col) {
        *oidx_ptr++ = cave_obj(row, col);
      }

    uint8_t* lit_ptr = &litmapD[0][0];
    for (int row = rmin; row < rmax; ++row)
      for (int col = cmin; col < cmax; ++col) {
        if (caveD[row][col].fval < MAX_FLOOR) {
          int cflag = caveD[row][col].cflag;
          *lit_ptr = cflag;
        }
        lit_ptr += 1;
      }
  } else {
    litmapD[uD.y][uD.x] = CF_TEMP_LIGHT;
  }
}
static char* affectD[][8] = {
    {"Recall"},   {"Slow"},     {"Fast"},
    {"Blind"},    {"Confused"}, {"Afraid"},
    {"Paralyse"}, {"Poison"},   {"Hungry", "Weak", "Faint"},
};
// Match single index
static int
py_affect(maid)
{
  return uD.mflag & (1 << maid);
}
// Match ALL trflag
static int
py_tr(trflag)
{
  return (cbD.tflag & trflag) == trflag;
}
void
affect_update()
{
  int active[AL(affectD)];
  int idx, count, pad, len, sum;

  idx = 0;
  active[idx++] = py_affect(MA_RECALL) != 0;
  active[idx++] = py_affect(MA_SLOW) != 0;
  active[idx++] = py_affect(MA_FAST) != 0;
  active[idx++] = (countD.blind != 0);
  active[idx++] = (countD.confusion != 0);
  active[idx++] = (countD.fear != 0);
  active[idx++] = (countD.paralysis != 0);
  active[idx++] = (countD.poison != 0);
  active[idx] = (uD.food <= PLAYER_FOOD_ALERT);
  active[idx] += (uD.food <= PLAYER_FOOD_WEAK);
  active[idx++] += (uD.food <= PLAYER_FOOD_FAINT);
  // Currently paralysis/rest skips rendering
  // active[idx++] = (countD.rest != 0);

  len = 0;
  sum = 0;
  for (int it = 0; it < idx; ++it) {
    if (active[it]) {
      count = snprintf(&affectinfoD[len], AL(affectinfoD) - len, "%s ",
                       affectD[it][active[it] - 1]);
      if (count > 0) len += count;
    }
    pad = MIN((1 + it) * AL(affectD[0]), AL(affectinfoD) - 1);
    while (len < pad) affectinfoD[len++] = ' ';
  }

  static char d0[] = "Town Level";
  if (dun_level != 0) {
    count = snprintf(&affectinfoD[len], AL(affectinfoD) - len, "%d feet",
                     dun_level * 50);
  } else if (AL(affectinfoD) - len > AL(d0)) {
    count = AL(d0);
    strcpy(&affectinfoD[len], d0);
  }

  if (count > 0) len += count;
  affectinfo_usedD = len;
}
void
draw()
{
  vital_update();
  symmap_update();
  altmap_update();
  affect_update();

  platformD.draw();
  AC(screen_usedD);
  AC(overlay_usedD);
}

static char log_extD[] = " -more-";
#define MAX_MSGLEN AL(msg_cqD[0])
void
msg_advance()
{
  int log_used;
  log_used = AS(msglen_cqD, msg_writeD);
  msg_writeD += (log_used != 0);
  AS(msglen_cqD, msg_writeD) = 0;
}
void
msg_pause()
{
  char c;
  int log_used;
  char* log;

  log_used = AS(msglen_cqD, msg_writeD);
  if (log_used) {
    log = AS(msg_cqD, msg_writeD);
    log_used += snprintf(log + log_used, MAX_MSGLEN - log_used, "%s", log_extD);
    AS(msglen_cqD, msg_writeD) = log_used;

    // wait for user to acknowledge prior buffer -more-
    draw();
    do {
      c = inkey();
      if (c == ESCAPE) break;
    } while (c != ' ');
    msg_advance();
  }
}
static void msg_game(msg, msglen) char* msg;
{
  char* log;
  int log_used;

  log_used = AS(msglen_cqD, msg_writeD);
  if (log_used + msglen + AL(log_extD) >= MAX_MSGLEN) {
    msg_pause();
    msg_countD += 1;
  }

  log = AS(msg_cqD, msg_writeD);
  log_used = AS(msglen_cqD, msg_writeD);
  if (log_used) {
    log_used += snprintf(log + log_used, MAX_MSGLEN - log_used, " %s", msg);
  } else {
    log_used = snprintf(log, MAX_MSGLEN, "%d| %s", msg_countD, msg);
  }

  if (log_used > 0) AS(msglen_cqD, msg_writeD) = log_used;
}
static void
msg_print(char* text)
{
  msg_game(text, strlen(text));
}
static int
see_print(char* text)
{
  BOOL see = (countD.blind == 0);
  if (see) msg_print(text);
  return see;
}
void
msg_history()
{
  char* log;
  int log_used, line;

  line = 0;
  for (int it = 1; it < MAX_MSG; ++it) {
    log = AS(msg_cqD, msg_writeD + it);
    log_used = AS(msglen_cqD, msg_writeD + it);
    if (log_used) {
      memcpy(screenD[line], log, log_used);
      screen_usedD[line] = log_used;
      line += 1;
    }
  }
  if (!line) {
    static char no_historyD[] = "No message history";
    memcpy(screenD[0], AP(no_historyD));
    screen_usedD[0] = AL(no_historyD);
  }
}
int
in_subcommand(prompt, command)
char* prompt;
char* command;
{
  char c;
  char* msg;

  msg = AS(msg_cqD, msg_writeD);
  AS(msglen_cqD, msg_writeD) =
      snprintf(msg, MAX_MSGLEN, "%s", prompt ? prompt : "");
  draw();
  do {
    c = inkey();
  } while (c == ' ');
  *command = c;
  AS(msglen_cqD, msg_writeD) = 0;
  return (c != ESCAPE && c != CTRL('c'));
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
    rnd_seed = test + RNG_M - (test == 0);
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
    // 1000 max itemization (you can wield iron chests apparently)
    // 300 for lance (TV_POLEARM)
    // 280 for two-handed great flail (TV_HAFTED)
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
int
tot_dam(obj, tdam, cidx)
struct objS* obj;
{
  struct creatureS* cr_ptr;

  cr_ptr = &creatureD[cidx];
  if ((obj->flags & TR_EGO_WEAPON) &&
      (((obj->tval >= TV_SLING_AMMO) && (obj->tval <= TV_ARROW)) ||
       ((obj->tval >= TV_HAFTED) && (obj->tval <= TV_SWORD)) ||
       (obj->tval == TV_FLASK))) {
    /* Slay Dragon  */
    if ((cr_ptr->cdefense & CD_DRAGON) && (obj->flags & TR_SLAY_DRAGON)) {
      tdam = tdam * 4;
    }
    /* Slay Undead  */
    else if ((cr_ptr->cdefense & CD_UNDEAD) && (obj->flags & TR_SLAY_UNDEAD)) {
      tdam = tdam * 3;
    }
    /* Slay Animal  */
    else if ((cr_ptr->cdefense & CD_ANIMAL) && (obj->flags & TR_SLAY_ANIMAL)) {
      tdam = tdam * 2;
    }
    /* Slay Evil     */
    else if ((cr_ptr->cdefense & CD_EVIL) && (obj->flags & TR_SLAY_EVIL)) {
      tdam = tdam * 2;
    }
    /* Frost         */
    else if ((cr_ptr->cdefense & CD_FROST) && (obj->flags & TR_FROST_BRAND)) {
      tdam = tdam * 3 / 2;
    }
    /* Fire        */
    else if ((cr_ptr->cdefense & CD_FIRE) && (obj->flags & TR_FLAME_TONGUE)) {
      tdam = tdam * 3 / 2;
    }
  }
  return (tdam);
}
void disturb(search, light) int search, light;
{
  if (countD.rest != 0) countD.rest = 0;
  find_flag = FALSE;
}

static void build_room(yval, xval) int yval, xval;
{
  register int i, j, x, xmax, y, ymax;
  uint8_t floor;
  register struct caveS *c_ptr, *d_ptr;

  if (dun_level <= randint(25))
    floor = FLOOR_LIGHT;
  else
    floor = FLOOR_DARK;

  y = yval - randint(ROOM_HEIGHT / 2);
  ymax = yval + randint(ROOM_HEIGHT / 2) - 1;
  x = xval - randint(ROOM_WIDTH / 2);
  xmax = xval + randint(ROOM_WIDTH / 2) - 1;

  for (i = y; i <= ymax; i++) {
    for (j = x; j <= xmax; j++) {
      c_ptr = &caveD[i][j];
      c_ptr->fval = floor;
      c_ptr->cflag |= CF_ROOM;
    }
  }

  y -= 1;
  ymax += 1;
  x -= 1;
  xmax += 1;
  for (i = y; i <= ymax; i++) {
    c_ptr = &caveD[i][x];
    c_ptr->fval = GRANITE_WALL;
    c_ptr->cflag |= CF_ROOM;
    c_ptr = &caveD[i][xmax];
    c_ptr->fval = GRANITE_WALL;
    c_ptr->cflag |= CF_ROOM;
  }

  c_ptr = &caveD[y][x];
  d_ptr = &caveD[ymax][x];
  for (i = x; i <= xmax; i++) {
    c_ptr->fval = GRANITE_WALL;
    c_ptr->cflag |= CF_ROOM;
    c_ptr++;
    d_ptr->fval = GRANITE_WALL;
    d_ptr->cflag |= CF_ROOM;
    d_ptr++;
  }
}
static void
build_store(sidx, y, x)
{
  int yval, y_height, y_depth;
  int xval, x_left, x_right;
  int i, j, tmp;
  struct objS* obj;

  yval = y * 7 + 4;
  xval = x * 8 + 8;
  y_height = yval - randint(2);
  y_depth = yval + randint(2);
  x_left = xval - randint(3);
  x_right = xval + randint(3);
  for (i = y_height; i <= y_depth; i++)
    for (j = x_left; j <= x_right; j++) {
      caveD[i][j].fval = BOUNDARY_WALL;
      caveD[i][j].cflag |= CF_PERM_LIGHT;
    }
  tmp = randint(4);
  if (tmp < 3) {
    i = randint(y_depth - y_height) + y_height - 1;
    if (tmp == 1)
      j = x_left;
    else
      j = x_right;
  } else {
    j = randint(x_right - x_left) + x_left - 1;
    if (tmp == 3)
      i = y_depth;
    else
      i = y_height;
  }

  obj = obj_use();
  obj->fy = i;
  obj->fx = j;
  obj->tval = TV_STORE_DOOR;
  obj->tchar = '0' + sidx + 1;
  obj->subval = 100 + sidx + 1;
  obj->number = 1;
  caveD[i][j].oidx = obj_index(obj);
  caveD[i][j].fval = FLOOR_CORR;
}
static void
build_pawn()
{
  int tmp, i, j, y, x, x_left, x_right;
  struct objS* obj;

  tmp = randint(2);
  if (tmp == 1) {
    x_left = 1;
    x = x_right = 1 + randint(2) - 1;
  } else {
    x = x_left = SYMMAP_WIDTH - randint(2) - 1;
    x_right = SYMMAP_WIDTH - 2;
  }
  y = SYMMAP_HEIGHT / 4 + randint(SYMMAP_HEIGHT / 2);

  for (i = y - 1; i <= y + 1; i++)
    for (j = x_left; j <= x_right; j++) {
      caveD[i][j].fval = BOUNDARY_WALL;
      caveD[i][j].cflag |= CF_PERM_LIGHT;
    }

  obj = obj_use();
  obj->fy = y;
  obj->fx = x;
  obj->tval = TV_PAWN_DOOR;
  obj->tchar = '0';
  obj->number = 1;
  caveD[y][x].oidx = obj_index(obj);
  caveD[y][x].fval = FLOOR_CORR;
}
BOOL
near_light(y, x)
{
  if ((caveD[y][x].cflag & CF_LIT_ROOM) == CF_ROOM) {
    for (int row = y - 1; row <= y + 1; ++row) {
      for (int col = x - 1; col <= x + 1; ++col) {
        if (caveD[row][col].fval == FLOOR_LIGHT) return TRUE;
      }
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

  tmp1 = CHUNK_HEIGHT / 2;
  tmp2 = CHUNK_WIDTH / 2;
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
    for (int row = x - 1; row <= x + 1; ++row) {
      caveD[col][row].cflag |= CF_PERM_LIGHT;
    }
  }

  return countD.blind == 0;
}
void
unlight_room(y, x)
{
  int i, j, start_col, end_col;
  int tmp1, tmp2, start_row, end_row;
  struct caveS* c_ptr;
  int tval;

  tmp1 = CHUNK_HEIGHT / 2;
  tmp2 = CHUNK_WIDTH / 2;
  start_row = (y / tmp1) * tmp1;
  start_col = (x / tmp2) * tmp2;
  end_row = start_row + tmp1 - 1;
  end_col = start_col + tmp2 - 1;
  for (i = start_row; i <= end_row; i++)
    for (j = start_col; j <= end_col; j++) {
      c_ptr = &caveD[i][j];
      if (c_ptr->cflag & CF_ROOM && c_ptr->fval < MAX_FLOOR) {
        c_ptr->cflag &= ~CF_PERM_LIGHT;
        c_ptr->fval = FLOOR_DARK;
      }
    }
}
int
unlight_area(y, x)
{
  int known;

  known = FALSE;
  if (caveD[y][x].cflag & CF_LIT_ROOM) {
    unlight_room(y, x);
    known = TRUE;
  }
  for (int col = y - 1; col <= y + 1; ++col) {
    for (int row = x - 1; row <= x + 1; ++row) {
      if (caveD[col][row].fval == FLOOR_CORR &&
          caveD[col][row].cflag & CF_PERM_LIGHT) {
        caveD[col][row].cflag &= ~CF_PERM_LIGHT;
        known = TRUE;
      }
    }
  }

  if (known && countD.blind == 0) msg_print("Darkness surrounds you.");

  return known;
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
/* A simple, fast, integer-based line-of-sight algorithm.  By Joseph Hall,
   4116 Brewster Drive, Raleigh NC 27606.  Email to jnh@ecemwl.ncsu.edu.

   Returns TRUE if a line of sight can be traced from x0, y0 to x1, y1.

   The LOS begins at the center of the tile [x0, y0] and ends at
   the center of the tile [x1, y1].  If los() is to return TRUE, all of
   the tiles this line passes through must be transparent, WITH THE
   EXCEPTIONS of the starting and ending tiles.

   We don't consider the line to be "passing through" a tile if
   it only passes across one corner of that tile. */

/* Because this function uses (short) ints for all calculations, overflow
   may occur if deltaX and deltaY exceed 90. */

int
los(fromY, fromX, toY, toX)
int fromY, fromX, toY, toX;
{
  int tmp, deltaX, deltaY;

  deltaX = toX - fromX;
  deltaY = toY - fromY;

  /* Adjacent? */
  if ((deltaX < 2) && (deltaX > -2) && (deltaY < 2) && (deltaY > -2))
    return TRUE;

  /* Handle the cases where deltaX or deltaY == 0. */
  if (deltaX == 0) {
    int p_y; /* y position -- loop variable  */

    if (deltaY < 0) {
      tmp = fromY;
      fromY = toY;
      toY = tmp;
    }
    for (p_y = fromY + 1; p_y < toY; p_y++)
      if (caveD[p_y][fromX].fval >= MIN_CLOSED_SPACE) return FALSE;
    return TRUE;
  } else if (deltaY == 0) {
    int px; /* x position -- loop variable  */

    if (deltaX < 0) {
      tmp = fromX;
      fromX = toX;
      toX = tmp;
    }
    for (px = fromX + 1; px < toX; px++)
      if (caveD[fromY][px].fval >= MIN_CLOSED_SPACE) return FALSE;
    return TRUE;
  }

  /* Now, we've eliminated all the degenerate cases.
     In the computations below, dy (or dx) and m are multiplied by a
     scale factor, scale = ABS(deltaX * deltaY * 2), so that we can use
     integer arithmetic. */

  {
    int px,     /* x position  			*/
        p_y,    /* y position  			*/
        scale2; /* above scale factor / 2  	*/
    int scale,  /* above scale factor  		*/
        xSign,  /* sign of deltaX  		*/
        ySign,  /* sign of deltaY  		*/
        m;      /* slope or 1/slope of LOS  	*/

    scale2 = ABS(deltaX * deltaY);
    scale = scale2 << 1;
    xSign = (deltaX < 0) ? -1 : 1;
    ySign = (deltaY < 0) ? -1 : 1;

    /* Travel from one end of the line to the other, oriented along
       the longer axis. */

    if (ABS(deltaX) >= ABS(deltaY)) {
      int dy; /* "fractional" y position  */
      /* We start at the border between the first and second tiles,
         where the y offset = .5 * slope.  Remember the scale
         factor.  We have:

         m = deltaY / deltaX * 2 * (deltaY * deltaX)
           = 2 * deltaY * deltaY. */

      dy = deltaY * deltaY;
      m = dy << 1;
      px = fromX + xSign;

      /* Consider the special case where slope == 1. */
      if (dy == scale2) {
        p_y = fromY + ySign;
        dy -= scale;
      } else
        p_y = fromY;

      while (toX - px) {
        if (caveD[p_y][px].fval >= MIN_CLOSED_SPACE) return FALSE;

        dy += m;
        if (dy < scale2)
          px += xSign;
        else if (dy > scale2) {
          p_y += ySign;
          if (caveD[p_y][px].fval >= MIN_CLOSED_SPACE) return FALSE;
          px += xSign;
          dy -= scale;
        } else {
          /* This is the case, dy == scale2, where the LOS
             exactly meets the corner of a tile. */
          px += xSign;
          p_y += ySign;
          dy -= scale;
        }
      }
      return TRUE;
    } else {
      int dx; /* "fractional" x position  */
      dx = deltaX * deltaX;
      m = dx << 1;

      p_y = fromY + ySign;
      if (dx == scale2) {
        px = fromX + xSign;
        dx -= scale;
      } else
        px = fromX;

      while (toY - p_y) {
        if (caveD[p_y][px].fval >= MIN_CLOSED_SPACE) return FALSE;
        dx += m;
        if (dx < scale2)
          p_y += ySign;
        else if (dx > scale2) {
          px += xSign;
          if (caveD[p_y][px].fval >= MIN_CLOSED_SPACE) return FALSE;
          p_y += ySign;
          dx -= scale;
        } else {
          px += xSign;
          p_y += ySign;
          dx -= scale;
        }
      }
      return TRUE;
    }
  }
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
static void
delete_object(y, x)
{
  struct caveS* cave_ptr;
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
static void
inven_destroy_one(iidx)
{
  struct objS* obj = obj_get(invenD[iidx]);
  if (obj->number > 1 && obj->subval & STACK_SINGLE) {
    obj->number -= 1;
  } else {
    obj_unuse(obj);
    invenD[iidx] = 0;
  }
}
static int
inven_copy_num(iidx, copy, num)
struct objS* copy;
{
  struct objS* obj;
  int id;

  obj = obj_use();
  id = obj->id;
  if (id) {
    *obj = *copy;
    obj->id = id;
    obj->number = num;
    invenD[iidx] = id;
  }

  return id != 0;
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
  obj->sn = 0;
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
              obj->sn = SN_R;
              obj->toac += 5;
              obj->cost += 2500;
              break;
            case 2: /* Resist Acid    */
              obj->flags |= TR_RES_ACID;
              obj->sn = SN_RA;
              obj->cost += 1000;
              break;
            case 3:
            case 4: /* Resist Fire    */
              obj->flags |= TR_RES_FIRE;
              obj->sn = SN_RF;
              obj->cost += 600;
              break;
            case 5:
            case 6: /* Resist Cold   */
              obj->flags |= TR_RES_COLD;
              obj->sn = SN_RC;
              obj->cost += 600;
              break;
            case 7:
            case 8:
            case 9: /* Resist Lightning*/
              obj->flags |= TR_RES_LIGHT;
              obj->sn = SN_RL;
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
              obj->sn = SN_HA;
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
              obj->sn = SN_DF;
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
              obj->sn = SN_SA;
              obj->cost += 3000;
              break;
            case 5:
            case 6: /* Slay Dragon   */
              obj->flags |= TR_SLAY_DRAGON;
              obj->tohit += 3;
              obj->todam += 3;
              obj->sn = SN_SD;
              obj->cost += 4000;
              break;
            case 7:
            case 8: /* Slay Evil     */
              obj->flags |= TR_SLAY_EVIL;
              obj->tohit += 3;
              obj->todam += 3;
              obj->sn = SN_SE;
              obj->cost += 4000;
              break;
            case 9:
            case 10: /* Slay Undead    */
              obj->flags |= (TR_SEE_INVIS | TR_SLAY_UNDEAD);
              obj->tohit += 3;
              obj->todam += 3;
              obj->sn = SN_SU;
              obj->cost += 5000;
              break;
            case 11:
            case 12:
            case 13: /* Flame Tongue  */
              obj->flags |= TR_FLAME_TONGUE;
              obj->tohit++;
              obj->todam += 3;
              obj->sn = SN_FT;
              obj->cost += 2000;
              break;
            case 14:
            case 15:
            case 16: /* Frost Brand   */
              obj->flags |= TR_FROST_BRAND;
              obj->tohit++;
              obj->todam++;
              obj->sn = SN_FB;
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
            obj->sn = SN_FREE_ACTION;
            obj->cost += 1000;
          } else {
            obj->tohit += 1 + randint(3);
            obj->todam += 1 + randint(3);
            obj->sn = SN_SLAYING;
            obj->cost += (obj->tohit + obj->todam) * 250;
          }
        }
      } else if (magik(cursed)) {
        if (magik(special)) {
          if (randint(2) == 1) {
            obj->flags |= TR_DEX;
            obj->sn = SN_CLUMSINESS;
          } else {
            obj->flags |= TR_STR;
            obj->sn = SN_WEAKNESS;
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
            obj->sn = SN_SLOW_DESCENT;
            obj->cost += 250;
          } else if (tmp == 1) {
            obj->flags |= TR_SPEED;
            obj->sn = SN_SPEED;
            obj->p1 = 1;
            obj->cost += 5000;
          } else /* 2 - 5 */
          {
            obj->flags |= TR_STEALTH;
            obj->p1 = randint(3);
            obj->sn = SN_STEALTH;
            obj->cost += 500;
          }
        }
      } else if (magik(cursed)) {
        tmp = randint(3);
        if (tmp == 1) {
          obj->flags |= TR_SPEED;
          obj->sn = SN_SLOWNESS;
          obj->p1 = -1;
        } else if (tmp == 2) {
          obj->flags |= TR_AGGRAVATE;
          obj->sn = SN_NOISE;
        } else {
          obj->sn = SN_GREAT_MASS;
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
              obj->sn = SN_INTELLIGENCE;
              obj->cost += obj->p1 * 500;
            } else if (tmp == 2) {
              obj->p1 = randint(2);
              obj->flags |= TR_WIS;
              obj->sn = SN_WISDOM;
              obj->cost += obj->p1 * 500;
            } else {
              obj->p1 = 1 + randint(4);
              obj->flags |= TR_INFRA;
              obj->sn = SN_INFRAVISION;
              obj->cost += obj->p1 * 250;
            }
          } else {
            switch (randint(6)) {
              case 1:
                obj->p1 = randint(3);
                obj->flags |= (TR_FREE_ACT | TR_CON | TR_DEX | TR_STR);
                obj->sn = SN_MIGHT;
                obj->cost += 1000 + obj->p1 * 500;
                break;
              case 2:
                obj->p1 = randint(3);
                obj->flags |= (TR_CHR | TR_WIS);
                obj->sn = SN_LORDLINESS;
                obj->cost += 1000 + obj->p1 * 500;
                break;
              case 3:
                obj->p1 = randint(3);
                obj->flags |= (TR_RES_LIGHT | TR_RES_COLD | TR_RES_ACID |
                               TR_RES_FIRE | TR_INT);
                obj->sn = SN_MAGI;
                obj->cost += 3000 + obj->p1 * 500;
                break;
              case 4:
                obj->p1 = randint(3);
                obj->flags |= TR_CHR;
                obj->sn = SN_BEAUTY;
                obj->cost += 750;
                break;
              case 5:
                obj->p1 = 5 * (1 + randint(4));
                obj->flags |= (TR_SEE_INVIS | TR_SEARCH);
                obj->sn = SN_SEEING;
                obj->cost += 1000 + obj->p1 * 100;
                break;
              case 6:
                obj->flags |= TR_REGEN;
                obj->sn = SN_REGENERATION;
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
              obj->sn = SN_STUPIDITY;
              break;
            case 2:
              obj->p1 = -randint(5);
              obj->flags |= TR_WIS;
              obj->sn = SN_DULLNESS;
              break;
            case 3:
              obj->flags |= TR_BLIND;
              obj->sn = SN_BLINDNESS;
              break;
            case 4:
              obj->flags |= TR_TIMID;
              obj->sn = SN_TIMIDNESS;
              break;
            case 5:
              obj->p1 = -randint(5);
              obj->flags |= TR_STR;
              obj->sn = SN_WEAKNESS;
              break;
            case 6:
              obj->flags |= TR_TELEPORT;
              obj->sn = SN_TELEPORTATION;
              break;
            case 7:
              obj->p1 = -randint(5);
              obj->flags |= TR_CHR;
              obj->sn = SN_UGLINESS;
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
            obj->flags |= TR_CURSED;
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
            obj->sn = SN_PROTECTION;
            obj->toac += m_bonus(2, 40, level);
            obj->cost += 250;
          } else {
            obj->toac += m_bonus(1, 20, level);
            obj->p1 = randint(3);
            obj->flags |= TR_STEALTH;
            obj->sn = SN_STEALTH;
            obj->cost += 500;
          }
        } else
          obj->toac += m_bonus(1, 20, level);
      } else if (magik(cursed)) {
        tmp = randint(3);
        if (tmp == 1) {
          obj->flags |= TR_AGGRAVATE;
          obj->sn = SN_IRRITATION;
          obj->toac -= m_bonus(1, 10, level);
          obj->tohit -= m_bonus(1, 10, level);
          obj->todam -= m_bonus(1, 10, level);
          obj->cost = 0;
        } else if (tmp == 2) {
          obj->sn = SN_VULNERABILITY;
          obj->toac -= m_bonus(10, 100, level + 50);
          obj->cost = 0;
        } else {
          obj->sn = SN_ENVELOPING;
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
      //       obj->sn = SN_EMPTY;
      //       break;
      //     case 2:
      //       obj->flags |= CH_LOCKED;
      //       obj->sn = SN_LOCKED;
      //       break;
      //     case 3:
      //     case 4:
      //       obj->flags |= (CH_LOSE_STR | CH_LOCKED);
      //       obj->sn = SN_POISON_NEEDLE;
      //       break;
      //     case 5:
      //     case 6:
      //       obj->flags |= (CH_POISON | CH_LOCKED);
      //       obj->sn = SN_POISON_NEEDLE;
      //       break;
      //     case 7:
      //     case 8:
      //     case 9:
      //       obj->flags |= (CH_PARALYSED | CH_LOCKED);
      //       obj->sn = SN_GAS_TRAP;
      //       break;
      //     case 10:
      //     case 11:
      //       obj->flags |= (CH_EXPLODE | CH_LOCKED);
      //       obj->sn = SN_EXPLOSION_DEVICE;
      //       break;
      //     case 12:
      //     case 13:
      //     case 14:
      //       obj->flags |= (CH_SUMMON | CH_LOCKED);
      //       obj->sn = SN_SUMMONING_RUNES;
      //       break;
      //     case 15:
      //     case 16:
      //     case 17:
      //       obj->flags |= (CH_PARALYSED | CH_POISON | CH_LOSE_STR |
      //       CH_LOCKED); obj->sn = SN_MULTIPLE_TRAPS; break;
      //     default:
      //       obj->flags |= (CH_SUMMON | CH_EXPLODE | CH_LOCKED);
      //       obj->sn = SN_MULTIPLE_TRAPS;
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
                obj->sn = SN_SLAYING;
                obj->tohit += 5;
                obj->todam += 5;
                obj->cost += 20;
                break;
              case 4:
              case 5:
                obj->flags |= TR_FLAME_TONGUE;
                obj->tohit += 2;
                obj->todam += 4;
                obj->sn = SN_FIRE;
                obj->cost += 25;
                break;
              case 6:
              case 7:
                obj->flags |= TR_SLAY_EVIL;
                obj->tohit += 3;
                obj->todam += 3;
                obj->sn = SN_SLAY_EVIL;
                obj->cost += 25;
                break;
              case 8:
              case 9:
                obj->flags |= TR_SLAY_ANIMAL;
                obj->tohit += 2;
                obj->todam += 2;
                obj->sn = SN_SLAY_ANIMAL;
                obj->cost += 30;
                break;
              case 10:
                obj->flags |= TR_SLAY_DRAGON;
                obj->tohit += 3;
                obj->todam += 3;
                obj->sn = SN_DRAGON_SLAYING;
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
static int
mask_subval(sv)
{
  return MASK_SUBVAL & sv;
}
int
knowable_tval_subval(tval, sv)
{
  int k;
  k = 0;
  switch (tval) {
    case TV_AMULET:
      k = 1;
      break;
    case TV_RING:
      k = 2;
      break;
    case TV_STAFF:
      k = 3;
      break;
    case TV_WAND:
      k = 4;
      break;
    case TV_SCROLL1:
    case TV_SCROLL2:
      k = 5;
      break;
    case TV_POTION1:
    case TV_POTION2:
      k = 6;
      break;
    case TV_FOOD:
      if (sv < AL(mushrooms)) k = 7;
      break;
  }
  return k;
}
BOOL
tr_is_known(tr_ptr)
struct treasureS* tr_ptr;
{
  int subval = mask_subval(tr_ptr->subval);
  int k = knowable_tval_subval(tr_ptr->tval, subval);
  if (k == 0) return TRUE;
  return knownD[k - 1][subval];
}
BOOL
tr_make_known(tr_ptr)
struct treasureS* tr_ptr;
{
  int subval = mask_subval(tr_ptr->subval);
  int k = knowable_tval_subval(tr_ptr->tval, subval);
  if (k == 0) return FALSE;
  BOOL change = 1 ^ knownD[k - 1][subval];
  knownD[k - 1][subval] = 1;
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
vuln_fire_breath(obj)
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
      if (obj->flags & TR_RES_FIRE)
        return FALSE;
      else
        return TRUE;
    case TV_STAFF:
    case TV_SCROLL1:
    case TV_SCROLL2:
    case TV_POTION1:
    case TV_POTION2:
    case TV_FLASK:
    case TV_FOOD:
    case TV_OPEN_DOOR:
    case TV_CLOSED_DOOR:
      return (TRUE);
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
vuln_acid_breath(obj)
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
    case TV_HELM:
    case TV_SHIELD:
    case TV_HARD_ARMOR:
    case TV_SOFT_ARMOR:
      if (obj->flags & TR_RES_ACID)
        return FALSE;
      else
        return TRUE;
    case TV_STAFF:
    case TV_SCROLL1:
    case TV_SCROLL2:
    case TV_FOOD:
    case TV_OPEN_DOOR:
    case TV_CLOSED_DOOR:
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
crset_evil(cre)
struct creatureS* cre;
{
  return (cre->cdefense & CD_EVIL);
}
int
crset_visible(cre)
struct creatureS* cre;
{
  return (cre->cmove & CM_INVISIBLE) == 0;
}
int
crset_invisible(cre)
struct creatureS* cre;
{
  return (cre->cmove & CM_INVISIBLE);
}
int
oset_always_visible(obj)
struct objS* obj;
{
  return (obj->tval >= TV_MIN_VISIBLE && obj->tval <= TV_MAX_VISIBLE);
}
static int
oset_pickup(obj)
struct objS* obj;
{
  // Underflow to exclude 0
  uint8_t tval = obj->tval - 1;
  return (tval < TV_MAX_PICK_UP);
}
static int
obj_mon_pickup(obj)
struct objS* obj;
{
  // Underflow to exclude 0
  uint8_t tval = obj->tval - 1;
  return (tval < TV_MON_PICK_UP);
}
int
oset_trap(obj)
struct objS* obj;
{
  return (obj->tval == TV_INVIS_TRAP || obj->tval == TV_CHEST);
}
int
oset_sdoor(obj)
struct objS* obj;
{
  return (obj->tval == TV_SECRET_DOOR || obj->tval == TV_UP_STAIR ||
          obj->tval == TV_DOWN_STAIR);
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
oset_tohitdam(obj)
struct objS* obj;
{
  switch (obj->tval) {
    case TV_ARROW:
    case TV_BOLT:
    case TV_SLING_AMMO:
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
    case TV_BOW:
    case TV_DIGGING:
      return TRUE;
    case TV_GLOVES:
      return (obj->tohit || obj->todam);
    case TV_RING:
      return (obj->tohit || obj->todam);
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
    case TV_WAND:
      return TRUE;
  }
  return FALSE;
}
int
oset_gold(obj)
struct objS* obj;
{
  return obj->tval == TV_GOLD;
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
may_equip(tval)
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
int
ring_slot()
{
  int slot = INVEN_RING;
  for (int it = 0; it < 2; ++it, ++slot) {
    if (invenD[slot] == 0) return slot;
  }
  return -1;
}
void
store_init()
{
  int i, j;
  i = AL(ownerD) / AL(storeD);
  for (j = 0; j < AL(storeD); ++j) {
    storeD[j] = MAX_STORE * (randint(i) - 1) + j;
  }

  if (HACK) {
    for (j = 0; j < AL(ownerD); ++j) {
      ownerD[j].max_cost = INT16_MAX;
    }
  }
}
int
store_item_destroy(sidx, item, count)
{
  struct objS* obj;
  int consume;

  obj = &store_objD[sidx][item];

  consume = obj->number <= count;
  if (consume) {
    store_objD[sidx][item] = DFT(struct objS);
  } else {
    obj->number -= count;
  }
  return consume;
}
int
store_tr_stack(sidx, tr_index, stack)
{
  int item;
  struct treasureS* tr_ptr;
  struct objS* obj;

  tr_ptr = &treasureD[tr_index];
  if (tr_ptr->subval & STACK_SINGLE) {
    for (int it = 0; it < AL(store_objD[0]); ++it) {
      obj = &store_objD[sidx][it];
      if (obj->tidx == tr_index) {
        obj->number += stack;
        return FALSE;
      }
    }
  }

  for (int it = 0; it < AL(store_objD[0]); ++it) {
    obj = &store_objD[sidx][it];
    if (obj->tidx == 0) {
      do {
        tr_obj_copy(tr_index, obj);
        magic_treasure(obj, OBJ_TOWN_LEVEL);
      } while (obj->cost <= 0 || obj->cost >= ownerD[storeD[sidx]].max_cost);
      obj->idflag = ID_REVEAL;
      return TRUE;
    }
  }

  return FALSE;
}
void
store_maint()
{
  int i, j, k, store_ctr;
  struct objS* obj;

  for (i = 0; i < MAX_STORE; i++) {
    store_ctr = 0;
    for (j = 0; j < MAX_STORE_INVEN; ++j) {
      store_ctr += (store_objD[i][j].tidx != 0);
    }

    if (store_ctr >= MIN_STORE_INVEN) {
      j = randint(STORE_TURN_AROUND);

      do {
        k = randint(MAX_STORE_INVEN) - 1;
        obj = &store_objD[i][k];
        if (obj->number) {
          store_ctr -= store_item_destroy(i, k, randint(obj->number));
          j -= 1;
        }
      } while (j > 0);
    }

    if (store_ctr < MAX_STORE_INVEN) {
      j = 0;
      if (store_ctr < MIN_STORE_INVEN) j = MIN_STORE_INVEN - store_ctr;
      j += randint(STORE_TURN_AROUND);

      if (0) {
        do {
          k = randint(MAX_DUNGEON_OBJ - 1);
          if (treasureD[k].cost > 0) {
            store_ctr += store_tr_stack(i, k, 1);
            j -= 1;
          }
        } while (j > 0);
      } else {
        do {
          k = randint(MAX_STORE_CHOICE - 1);
          store_ctr +=
              store_tr_stack(i, store_choiceD[i][k], store_stockD[i][k]);
          j -= 1;
        } while (j > 0);
      }
    }
  }
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
{
  struct monS* mon;
  struct creatureS* cre;
  int midx;

  cre = &creatureD[z];
  mon = mon_use();

  if (mon->id) {
    mon->cidx = z;
    if (!cre->sleep || !slp)
      mon->msleep = 0;
    else
      mon->msleep = (cre->sleep * 2) + randint(cre->sleep * 10);
    if (cre->cdefense & CD_MAX_HP)
      mon->hp = cre->hd[0] * cre->hd[1];
    else
      mon->hp = pdamroll(cre->hd);
    mon->fy = y;
    mon->fx = x;
    mon->mlit = FALSE;

    midx = mon_index(mon);
    caveD[y][x].midx = midx;
    return midx;
  }

  return FALSE;
}
static int
mon_multiply(mon)
struct monS* mon;
{
  register int y, x, fy, fx, i, j, k;
  register struct caveS* c_ptr;
  int count;

  y = uD.y;
  x = uD.x;
  fy = mon->fy;
  fx = mon->fx;
  i = 0;
  count = 0;
  do {
    j = fy - 2 + randint(3);
    k = fx - 2 + randint(3);
    // don't create a new creature on top of the old one
    // don't create a creature on top of the player
    if ((j != fy || k != fx) && (j != y || k != x)) {
      c_ptr = &caveD[j][k];
      if (c_ptr->fval <= MAX_OPEN_SPACE) {
        // TBD: CM_EATS_OTHER
        if (c_ptr->midx == 0) {
          return place_monster(j, k, mon->cidx, FALSE);
        }
      }
    }
    i++;
  } while (i <= 18);
  return FALSE;
}
int
summon_monster(y, x)
{
  int i, j, k;
  int l, summon;
  struct caveS* cave_ptr;

  i = 0;
  summon = 0;
  l = get_mon_num(dun_level + MON_SUMMON_ADJ);
  for (int it = 0; it < 9; ++it) {
    j = y - 2 + randint(3);
    k = x - 2 + randint(3);
    if (j != y || k != x) {
      cave_ptr = &caveD[j][k];
      if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->midx == 0)) {
        summon = place_monster(j, k, l, FALSE);
        break;
      }
    }
  }
  return summon;
}
int
summon_undead(y, x)
{
  int j, k, cidx;
  int l, m, summon;
  struct caveS* cave_ptr;

  summon = 0;
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
    if (j != y || k != x) {
      cave_ptr = &caveD[j][k];
      if (cave_ptr->fval <= MAX_OPEN_SPACE && (cave_ptr->midx == 0)) {
        summon = place_monster(j, k, cidx, FALSE);
        break;
      }
    }
  }

  return summon;
}
void
alloc_mon(num, dis, slp)
{
  int y, x, i;
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

  magic_treasure(obj, dun_level);

  if (uD.y == y && uD.x == x)
    msg_print("You feel something roll beneath your feet.");
}
void
place_trap(y, x, offset)
{
  struct objS* obj;

  obj = obj_use();
  caveD[y][x].oidx = obj_index(obj);
  tr_obj_copy(OBJ_TRAP_BEGIN + offset, obj);
  obj->fy = y;
  obj->fx = x;
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
        place_trap(y, x, randint(MAX_TRAP) - 1);
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
  int16_t yloc[CHUNK_SQ + 1], xloc[CHUNK_SQ + 1];

  k = randnor(DUN_ROOM_MEAN, 2);
  for (i = 0; i < k; i++)
    room_map[randint(AL(room_map)) - 1][randint(AL(room_map[0])) - 1] = 1;
  k = 0;
  for (i = 0; i < AL(room_map); i++)
    for (j = 0; j < AL(room_map[0]); j++)
      if (room_map[i][j]) {
        // Offset the room within the viewport
        yloc[k] = i * CHUNK_HEIGHT + (CHUNK_HEIGHT / 4);
        xloc[k] = j * CHUNK_WIDTH + (CHUNK_WIDTH / 4);
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

  int alloc_level = CLAMP(dun_level / 2, 2, 15);
  alloc_mon((randint(RND_MALLOC_LEVEL) + MIN_MALLOC_LEVEL + alloc_level), 0,
            TRUE);
  alloc_obj(set_corr, 3, randint(alloc_level));
  alloc_obj(set_room, 5, randnor(TREAS_ROOM_MEAN, 3));
  alloc_obj(set_floor, 5, randnor(TREAS_ANY_ALLOC, 3));
  alloc_obj(set_floor, 4, randnor(TREAS_GOLD_ALLOC, 3));
  alloc_obj(set_floor, 1, randint(alloc_level));
  // if (dun_level >= WIN_MON_APPEAR) place_win_monster();
}
static int
town_night()
{
  return ((turnD / (1 << 12)) & 0x1);
}
void
town_gen()
{
  int i, j, k;
  struct caveS* c_ptr;
  int room[MAX_STORE];
  int room_used;
  uint32_t seed;

  seed = rnd_seed;
  rnd_seed = town_seed;

  uint8_t cflag = town_night() ? 0 : (CF_PERM_LIGHT | CF_ROOM);
  for (int row = 0; row < MAX_HEIGHT; ++row) {
    for (int col = 0; col < MAX_WIDTH; ++col) {
      c_ptr = &caveD[row][col];
      if ((row == 0 || row + 1 >= SYMMAP_HEIGHT) ||
          (col == 0 || col + 1 >= SYMMAP_WIDTH)) {
        c_ptr->fval = BOUNDARY_WALL;
        c_ptr->cflag = CF_PERM_LIGHT;
      } else {
        c_ptr->fval = FLOOR_DARK;
        c_ptr->cflag = cflag;
      }
    }
  }

  for (i = 0; i < MAX_STORE; ++i) room[i] = i;
  room_used = MAX_STORE;
  for (i = 0; i < 2; i++)
    for (j = 0; j < 3; j++) {
      k = randint(room_used) - 1;

      build_store(room[k], i, j);
      while (k < MAX_STORE - 1) {
        room[k] = room[k + 1];
        k += 1;
      }
      room_used -= 1;
    }

  build_pawn();

  do {
    i = randint(SYMMAP_HEIGHT - 2);
    j = randint(SYMMAP_WIDTH - 2);
    c_ptr = &caveD[i][j];
  } while (c_ptr->fval >= MIN_CLOSED_SPACE || (c_ptr->oidx != 0) ||
           (c_ptr->midx != 0));
  place_stair_tval_tchar(i, j, TV_DOWN_STAIR, '>');
  caveD[i][j].cflag |= CF_FIELDMARK;
  rnd_seed = seed;

  do {
    i = randint(SYMMAP_HEIGHT - 2);
    j = randint(SYMMAP_WIDTH - 2);
    c_ptr = &caveD[i][j];
  } while (c_ptr->fval >= MIN_CLOSED_SPACE || (c_ptr->oidx != 0) ||
           (c_ptr->midx != 0));
  uD.y = i;
  uD.x = j;

  store_maint();
}
void
cave_reset()
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
}
void
generate_cave()
{
  // a fresh cave!
  if (dun_level != 0)
    cave_gen();
  else
    town_gen();
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
  if (dun_level != 0) {
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
  } else {
    panel->panel_row = 0;
    panel->panel_col = 0;
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
int
dir_x(dir)
{
  switch (dir) {
    case 1:
    case 4:
    case 7:
      return -1;
    case 3:
    case 6:
    case 9:
      return 1;
  }
  return 0;
}
int
dir_y(dir)
{
  switch (dir) {
    case 1:
    case 2:
    case 3:
      return 1;
    case 7:
    case 8:
    case 9:
      return -1;
  }
  return 0;
}
int
mmove(dir, y, x)
int dir;
register int *y, *x;
{
  register int new_row, new_col;
  int b;

  new_row = dir_y(dir);
  new_col = dir_x(dir);
  new_row += *y;
  new_col += *x;
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
    if (countD.blind == 0) {
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
void
find_event(y, x)
{
  int dir, newdir, t, check_dir, row, col;
  register int i, max, option, option2;
  struct caveS* c_ptr;

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
      if (c_ptr->oidx != 0) {
        t = entity_objD[c_ptr->oidx].tval;
        if (t != TV_INVIS_TRAP && t != TV_SECRET_DOOR &&
            (t != TV_OPEN_DOOR || !find_ignore_doors)) {
          find_flag = FALSE;
          return;
        }
      }

      if (c_ptr->fval <= MAX_OPEN_SPACE) {
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
      mmove(option, &row, &col);
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
int
detect_obj(int (*valid)())
{
  register int i, j, detect;
  register struct caveS* c_ptr;
  struct objS* obj;

  int rmin = panelD.panel_row_min;
  int rmax = panelD.panel_row_max;
  int cmin = panelD.panel_col_min;
  int cmax = panelD.panel_col_max;

  detect = FALSE;
  for (i = rmin; i < rmax; i++)
    for (j = cmin; j < cmax; j++) {
      c_ptr = &caveD[i][j];
      obj = &entity_objD[c_ptr->oidx];
      if (valid(obj)) {
        c_ptr->cflag |= CF_FIELDMARK;
        // dungeon fixtures become known
        // enables locked/stuck door interaction, trap auto-disarm
        if (obj->tval > TV_MAX_PICK_UP) {
          obj->idflag |= ID_REVEAL;
          if (obj->tval == TV_INVIS_TRAP) obj->tval = TV_VIS_TRAP;
        }
        detect = TRUE;
      }
    }
  return (detect);
}
int
detect_mon(int (*valid)())
{
  struct creatureS* cr_ptr;

  FOR_EACH(mon, {
    if (panel_contains(&panelD, mon->fy, mon->fx)) {
      cr_ptr = &creatureD[mon->cidx];
      if (valid(cr_ptr)) {
        return TRUE;
      }
    }
  });
  return FALSE;
}
void
move_rec(y1, x1, y2, x2)
{
  int tmp = caveD[y1][x1].midx;
  caveD[y1][x1].midx = 0;
  caveD[y2][x2].midx = tmp;
}
void
update_mon(midx)
{
  int flag, fy, fx, cdis;
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;

  m_ptr = &entity_monD[midx];
  cr_ptr = &creatureD[m_ptr->cidx];
  flag = FALSE;
  fy = m_ptr->fy;
  fx = m_ptr->fx;
  cdis = distance(uD.y, uD.x, fy, fx);
  if ((panel_contains(&panelD, fy, fx))) {
    if (py_affect(MA_DETECT_EVIL) && (CD_EVIL & cr_ptr->cdefense)) {
      flag = TRUE;
    } else if (py_affect(MA_DETECT_MON) &&
               ((CM_INVISIBLE & cr_ptr->cmove) == 0)) {
      flag = TRUE;
    } else if (py_affect(MA_DETECT_INVIS) && (CM_INVISIBLE & cr_ptr->cmove)) {
      flag = TRUE;
    } else if (countD.blind == 0 && (cdis <= MAX_SIGHT) &&
               los(uD.y, uD.x, fy, fx)) {
      c_ptr = &caveD[fy][fx];
      /* Normal sight.       */
      if (cave_lit(c_ptr)) {
        if ((CM_INVISIBLE & cr_ptr->cmove) == 0)
          flag = TRUE;
        else if (py_tr(TR_SEE_INVIS))
          flag = TRUE;
      } else if ((CD_INFRA & cr_ptr->cdefense) && (cdis <= uD.infra)) {
        flag = TRUE;
      }
    }
  }

  if (HACK) flag = TRUE;
  /* Light it up.   */
  if (flag) {
    if (!m_ptr->mlit) {
      disturb(1, 0);
      m_ptr->mlit = TRUE;
    }
  }
  /* Turn it off.   */
  else if (m_ptr->mlit) {
    m_ptr->mlit = FALSE;
  }
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
attack_blows(weight)
{
  register int adj_weight;
  register int str_index, dex_index, s, d;

  d = statD.use_stat[A_DEX];
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

  s = statD.use_stat[A_STR];
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
uint32_t
sustain_stat(sidx)
uint32_t sidx;
{
  return ((1 << sidx) | TR_SUST_STAT);
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
chr_adj()
{
  int charisma;

  charisma = statD.use_stat[A_CHR];
  if (charisma > 117)
    return (90);
  else if (charisma > 107)
    return (92);
  else if (charisma > 87)
    return (94);
  else if (charisma > 67)
    return (96);
  else if (charisma > 18)
    return (98);
  else
    switch (charisma) {
      case 18:
        return (100);
      case 17:
        return (101);
      case 16:
        return (102);
      case 15:
        return (103);
      case 14:
        return (104);
      case 13:
        return (106);
      case 12:
        return (108);
      case 11:
        return (110);
      case 10:
        return (112);
      case 9:
        return (114);
      case 8:
        return (116);
      case 7:
        return (118);
      case 6:
        return (120);
      case 5:
        return (122);
      case 4:
        return (125);
      case 3:
        return (130);
      default:
        return (100);
    }
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
// TBD: rewrite
static void summon_object(y, x, num, typ) int y, x, num, typ;
{
  register int i, j, k;
  register struct caveS* c_ptr;
  int real_typ;

  if ((typ == 1) || (typ == 5))
    real_typ = 1; /* typ == 1 -> objects */
  else
    real_typ = 256; /* typ == 2 -> gold */
  do {
    i = 0;
    do {
      j = y - 3 + randint(5);
      k = x - 3 + randint(5);
      if (in_bounds(j, k) && los(y, x, j, k)) {
        c_ptr = &caveD[j][k];
        if (c_ptr->fval <= MAX_OPEN_SPACE && (c_ptr->oidx == 0)) {
          if ((typ == 3) || (typ == 7))
          /* typ == 3 -> 50% objects, 50% gold */
          {
            if (randint(100) < 50)
              real_typ = 1;
            else
              real_typ = 256;
          }
          if (real_typ == 1)
            place_object(j, k, (typ >= 4));
          else
            place_gold(j, k);

          if (c_ptr->cflag & CF_LIT) c_ptr->cflag |= CF_FIELDMARK;
          i = 20;
        }
      }
      i++;
    } while (i <= 20);
    num--;
  } while (num != 0);
}
void
mon_death(y, x, flags)
{
  int i, number;

  if (flags & CM_CARRY_OBJ)
    i = 1;
  else
    i = 0;
  if (flags & CM_CARRY_GOLD) i += 2;
  if (flags & CM_SMALL_OBJ) i += 4;

  number = 0;
  if ((flags & CM_60_RANDOM) && (randint(100) < 60)) number++;
  if ((flags & CM_90_RANDOM) && (randint(100) < 90)) number++;
  if (flags & CM_1D2_OBJ) number += randint(2);
  if (flags & CM_2D2_OBJ) number += damroll(2, 2);
  if (flags & CM_4D2_OBJ) number += damroll(4, 2);
  if (number > 0) summon_object(y, x, number, i);

  if (flags & CM_WIN)
    if (!death) /* maybe the player died in mid-turn */
    {
      total_winner = TRUE;
      msg_print("*** CONGRATULATIONS *** You have won the game.");
      msg_print("You cannot save this game, but you may retire when ready.");
    }
}
static int
mon_take_hit(midx, dam)
{
  struct monS* mon = &entity_monD[midx];
  struct creatureS* cre = &creatureD[mon->cidx];
  int death_blow;

  mon->msleep = 0;
  mon->hp -= MAX(dam, 1);
  death_blow = mon->hp < 0;

  if (death_blow) {
    // TBD: frac_exp
    uD.exp += (cre->mexp * cre->level) / uD.lev;

    caveD[mon->fy][mon->fx].midx = 0;
    mon_death(mon->fy, mon->fx, cre->cmove);
    mon_unuse(mon);
  }

  return death_blow;
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
void obj_prefix(obj, plural) struct objS* obj;
{
  char obj_name[160];

  int offset = 0;
  for (int it = 0; it < AL(descD); ++it) {
    if (descD[it] != '~')
      obj_name[it - offset] = descD[it];
    else if (plural && obj->number != 1)
      obj_name[it - offset] = 's';
    else
      offset += 1;
    if (descD[it] == 0) break;
  }

  if (plural) {
    /* ampersand is always the first character */
    if (obj_name[0] == '&') {
      /* use &obj_name[1], so that & does not appear in output */
      if (obj->number > 1)
        snprintf(descD, AL(descD), "%d%s", obj->number, &obj_name[1]);
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

  if (obj->sn && obj_reveal(obj)) {
    snprintf(tmp_str, AL(tmp_str), " %s", special_nameD[obj->sn]);
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
  if (obj_reveal(obj)) {
    if (oset_tohitdam(obj)) {
      snprintf(tmp_str, AL(tmp_str), " (%+d,%+d)", obj->tohit, obj->todam);
      strcat(descD, tmp_str);
    }
  }

  // Check p1 value
  tmp_str[0] = 0;
  if (obj->idflag & ID_REVEAL) {
    if (obj->tval == TV_DIGGING || obj->tval == TV_AMULET ||
        obj->tval == TV_RING) {
      if (obj->p1 && (obj->flags & TR_SUST_STAT) == 0)
        sprintf(tmp_str, " (%+d)", obj->p1);
    } else if (obj->tval == TV_STAFF || obj->tval == TV_WAND) {
      sprintf(tmp_str, " (%d charges)", obj->p1);
    } else if (obj->tval == TV_HAFTED || obj->tval == TV_POLEARM ||
               obj->tval == TV_SWORD) {
      if (obj->flags & TR_STR)
        sprintf(tmp_str, " (%+d to STR)", obj->p1);
      else if (obj->flags & TR_STEALTH)
        sprintf(tmp_str, " (%+d to stealth)", obj->p1);
    }
  }
  strcat(descD, tmp_str);

  if (obj->idflag & ID_MAGIK) strcat(descD, "{magik}");
  if (obj->idflag & ID_DAMD) strcat(descD, "{damned}");
  if (obj->idflag & ID_CORRODED) strcat(descD, "{corroded}");
  if (obj->idflag & ID_EMPTY) strcat(descD, "{empty}");
}
void obj_desc(obj, prefix) struct objS* obj;
BOOL prefix;
{
  char* basenm;
  char damstr[80];
  int indexx, unknown, append_name, tmp;
  struct treasureS* tr_ptr;

  tr_ptr = &treasureD[obj->tidx];
  indexx = mask_subval(obj->subval);
  basenm = tr_ptr->name;
  damstr[0] = 0;
  unknown = !(tr_is_known(tr_ptr) || (obj->idflag & ID_REVEAL));
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
      if (unknown) {
        snprintf(descD, AL(descD), "& %s Amulet", amulets[indexx]);
        basenm = 0;
      } else {
        basenm = "& Amulet";
        append_name = TRUE;
      }
      break;
    case TV_RING:
      if (unknown) {
        basenm = 0;
        snprintf(descD, AL(descD), "& %s Ring", rocks[indexx]);
      } else {
        basenm = "& Ring";
        append_name = TRUE;
      }
      break;
    case TV_STAFF:
      if (unknown) {
        basenm = 0;
        snprintf(descD, AL(descD), "& %s Staff", woods[indexx]);
      } else {
        basenm = "& Staff";
        append_name = TRUE;
      }
      break;
    case TV_WAND:
      if (unknown) {
        basenm = 0;
        snprintf(descD, AL(descD), "& %s Wand", metals[indexx]);
      } else {
        basenm = "& Wand";
        append_name = TRUE;
      }
      break;
    case TV_SCROLL1:
    case TV_SCROLL2:
      if (unknown) {
        basenm = 0;
        snprintf(descD, AL(descD), "& Scroll~ titled \"%s\"", titleD[indexx]);
      } else {
        basenm = "& Scroll~";
        append_name = TRUE;
      }
      break;
    case TV_POTION1:
    case TV_POTION2:
      if (unknown) {
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
      if (unknown) {
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
      break;
    case TV_OPEN_DOOR:
    case TV_CLOSED_DOOR:
    case TV_SECRET_DOOR:
    case TV_RUBBLE:
    case TV_GOLD:
    case TV_INVIS_TRAP:
    case TV_UP_STAIR:
    case TV_DOWN_STAIR:
      descD[0] = 0;
      return;
    case TV_VIS_TRAP:
      break;
    // case TV_STORE_DOOR:
    //   sprintf(descD, "the entrance to the %s.",
    //   object_list[obj->index].name); return;
    case TV_GLYPH:
      break;
    default:
      snprintf(descD, AL(descD), "Error in objdes(): %d", obj->tval);
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

  if (mon->mlit)
    snprintf(descD, AL(descD), "The %s", cre->name);
  else
    strcpy(descD, "It");

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

  if (py_affect(MA_HERO)) hitpoints += 10;
  if (py_affect(MA_SUPERHERO)) hitpoints += 20;

  // Scale current hp to the new maximum
  int value = ((uD.chp << 16) + uD.chp_frac) / uD.mhp * hitpoints;
  uD.chp = value >> 16;
  uD.chp_frac = value & 0xFFFF;
  uD.mhp = hitpoints;
}
int
weight_tohit_adj()
{
  struct objS* obj;
  int use_weight = statD.use_stat[A_STR] * 15;

  obj = obj_get(invenD[INVEN_WIELD]);
  if (use_weight < obj->weight) return use_weight - obj->weight;
  return 0;
}
void
calc_bonuses()
{
  int tflag;
  int ac, toac;
  int tohit, todam;
  int hide_tohit, hide_todam, hide_toac;
  struct objS* obj;

  tohit = weight_tohit_adj() + tohit_adj();
  todam = todam_adj();
  toac = toac_adj();
  ac = 0;
  hide_tohit = hide_todam = hide_toac = 0;
  tflag = 0;
  for (int it = INVEN_EQUIP; it < INVEN_EQUIP_END; it++) {
    struct objS* obj = obj_get(invenD[it]);
    // TBD: Bow damage is skipped
    tohit += obj->tohit;
    todam += obj->todam;
    toac += obj->toac;
    ac += obj->ac;
    if ((obj->idflag & ID_REVEAL) == 0) {
      hide_tohit += obj->tohit;
      hide_todam += obj->todam;
      hide_toac += obj->toac;
    }
    tflag |= obj->flags;
  }

  /* Add in temporary spell increases  */
  ac += uD.ma_ac;
  if (py_affect(MA_SEE_INVIS)) tflag |= TR_SEE_INVIS;

  // Summarize ac
  cbD.ptohit = tohit;
  cbD.ptodam = todam;
  cbD.ptoac = toac;
  cbD.pac = ac + toac;
  cbD.hide_tohit = hide_tohit;
  cbD.hide_todam = hide_todam;
  cbD.hide_toac = hide_toac;

  tflag &= ~TR_STATS;
  for (int it = INVEN_EQUIP; it < INVEN_EQUIP_END; it++) {
    struct objS* obj = obj_get(invenD[it]);
    if (TR_SUST_STAT & obj->flags) tflag |= sustain_stat(obj->p1 - 1);
  }
  cbD.tflag = tflag;
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
{
  int amount;

  if ((TR_BLIND & obj->flags) && (factor > 0)) countD.blind += 1000;
  if ((TR_TIMID & obj->flags) && (factor > 0)) countD.fear += 50;
  if (TR_SLOW_DIGEST & obj->flags) uD.food_digest -= factor;
  if (TR_REGEN & obj->flags) uD.food_digest += factor * 3;

  amount = obj->p1 * factor;
  if (obj->flags & TR_STATS) {
    for (int it = 0; it < MAX_A; it++)
      if ((1 << it) & obj->flags) {
        statD.mod_stat[it] += amount;
        set_use_stat(it);
      }
  }
  if (TR_SEARCH & obj->flags) {
    uD.search += amount;
    uD.fos -= amount;
  }
  if (TR_STEALTH & obj->flags) uD.stealth += amount;
  if (TR_SPEED & obj->flags) uD.pspeed -= amount;
  if (TR_INFRA & obj->flags) uD.infra += amount;
}
// TBD: various
// Combat bonuses are applied in calc_bonuses
// Hp bonuses are reapplied in calc_hitpoints
void
ma_bonuses(maffect, factor)
{
  disturb(0, 0);
  switch (maffect) {
    case MA_BLESS:
      uD.ma_ac += factor * 2;
      uD.bth += factor * 5;
      // uD.bthb += factor * 5;
      if (factor > 0)
        msg_print("You feel righteous!");
      else if (factor < 0)
        msg_print("The prayer has expired.");
      break;
    case MA_HERO:
      uD.chp += (factor > 0) * 10;
      uD.mhp += factor * 10;
      uD.bth += factor * 12;
      // uD.bthb += factor * 12;
      if (factor > 0)
        msg_print("You feel like a HERO!");
      else if (factor < 0)
        msg_print("The heroism wears off.");
      break;
    case MA_SUPERHERO:
      uD.chp += (factor > 0) * 20;
      uD.mhp += factor * 20;
      uD.bth += factor * 24;
      // uD.bthb += factor * 24;
      if (factor > 0)
        msg_print("You feel like a SUPER-HERO!");
      else if (factor < 0)
        msg_print("The super heroism wears off.");
      break;
    case MA_FAST:
      uD.pspeed -= factor * 1;
      if (factor > 0)
        msg_print("You feel yourself moving faster.");
      else if (factor < 0)
        msg_print("You feel yourself slow down.");
      break;
    case MA_SLOW:
      uD.pspeed += factor * 1;
      if (factor > 0)
        msg_print("You feel yourself moving slower.");
      else if (factor < 0)
        msg_print("You feel yourself speed up.");
      break;
    case MA_AFIRE:
      if (factor > 0)
        msg_print("You feel safe from flame.");
      else if (factor < 0)
        msg_print("You no longer feel safe from flame.");
      break;
    case MA_AFROST:
      if (factor > 0)
        msg_print("You feel safe from cold.");
      else if (factor < 0)
        msg_print("You no longer feel safe from cold.");
      break;
    case MA_INVULN:
      uD.ma_ac += factor * 100;
      if (factor > 0)
        msg_print("Your skin turns into steel!");
      else if (factor < 0)
        msg_print("Your skin returns to normal.");
      break;
    case MA_SEE_INVIS:
      break;
    case MA_SEE_INFRA:
      uD.infra += factor * 3;
      break;
    case MA_DETECT_MON:
      break;
    case MA_DETECT_EVIL:
      break;
    case MA_DETECT_INVIS:
      break;
    case MA_RECALL:
      if (factor < 0) {
        new_level_flag = 1;
        countD.paralysis += 1;
        if (dun_level) {
          dun_level = 0;
          msg_print("You feel yourself yanked upwards!");
        } else {
          dun_level = uD.max_dlv;
          msg_print("You feel yourself yanked downwards!");
        }
      }
      break;
    default:
      msg_print("Error in ma_bonuses()");
      break;
  }
}
BOOL
player_saves()
{
  int adj = level_adj[uD.clidx][LA_SAVE];

  return (randint(100) <= (uD.save + think_adj(A_WIS) + (adj * uD.lev / 3)));
}
static int
equip_count()
{
  int count = 0;
  for (int it = INVEN_EQUIP; it < MAX_INVEN; ++it) {
    count += (invenD[it] != 0);
  }
  return count;
}
static int
equip_cursed()
{
  for (int it = INVEN_EQUIP; it < INVEN_EQUIP_END; ++it) {
    struct objS* obj = obj_get(invenD[it]);
    if (obj->flags & TR_CURSED) return it;
  }
  return -1;
}
static int
equip_random()
{
  int tmp[6], k;

  k = 0;
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
  if (l >= 0) {
    i_ptr = obj_get(invenD[l]);
    obj_desc(i_ptr, FALSE);
    MSG("Your %s glows black, fades.", descD);
    i_ptr->tohit = 0;
    i_ptr->todam = 0;
    i_ptr->toac = -randint(5) - randint(5);
    i_ptr->sn = 0;

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
  int eqidx;
  struct objS* obj;

  eqidx = equip_cursed();
  if (eqidx > -1) {
    obj = obj_get(invenD[eqidx]);
    obj->flags &= ~TR_CURSED;
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
    if (invenD[it]) tmp[k++] = it;
  }

  if (k)
    return tmp[randint(k) - 1];
  else
    return -1;
}
static int
inven_count()
{
  int count = 0;
  for (int it = 0; it < INVEN_EQUIP; ++it) {
    count += (invenD[it] != 0);
  }
  return count;
}
static int
inven_slot()
{
  for (int it = 0; it < INVEN_EQUIP; ++it) {
    if (!invenD[it]) return it;
  }
  return -1;
}
static int
inven_merge_slot(obj)
struct objS* obj;
{
  int tval, p1, subval, number;

  tval = obj->tval;
  p1 = obj->p1;
  subval = obj->subval;
  number = obj->number;
  if (subval & STACK_ANY) {
    for (int it = 0; it < INVEN_EQUIP; ++it) {
      struct objS* i_ptr = obj_get(invenD[it]);
      if (tval == i_ptr->tval && p1 == i_ptr->p1 && subval == i_ptr->subval &&
          number + i_ptr->number < 256) {
        return it;
      }
    }
  }
  return -1;
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
static int
inven_ident(iidx)
{
  struct objS* obj;
  struct treasureS* tr_ptr;
  int used;

  obj = obj_get(invenD[iidx]);
  tr_ptr = &treasureD[obj->tidx];
  used = tr_make_known(tr_ptr);
  if ((obj->idflag & ID_REVEAL) == 0) {
    used |= TRUE;
    obj->idflag |= ID_REVEAL;
  }
  obj_desc(obj, TRUE);
  if (iidx >= INVEN_EQUIP) {
    calc_bonuses();
    MSG("%s: %s", describe_use(iidx), descD);
  } else {
    MSG("%c) %s", iidx + 'a', descD);
  }
  return used;
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
      uint32_t cflag = cave->cflag;

      if (countD.blind == 0) {
        cflag |= CF_TEMP_LIGHT;
        if (cave->fval >= MIN_WALL) cflag |= CF_PERM_LIGHT;
        if (cave->oidx) {
          struct objS* obj = &entity_objD[cave->oidx];
          if (obj->tval >= TV_MIN_VISIBLE && obj->tval <= TV_MAX_VISIBLE) {
            cflag |= CF_FIELDMARK;
          }
        }
      }
      cave->cflag = cflag;
    }
  }
}
static void
py_take_hit(damage)
{
  uD.chp -= damage;
  if (uD.chp < 0) {
    death = TRUE;
    new_level_flag = TRUE;
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

  if (in_subcommand("Which character would you like to play?", &c)) {
    uint8_t iidx = c - 'a';
    if (iidx > 0 && iidx < AL(classD)) return iidx;
  }

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
  int hitdie;
  int csel, rsel;
  int8_t stat[MAX_A];

  csel = 0;  // py_class_select();
  do {
    rsel = randint(AL(raceD)) - 1;
  } while ((raceD[rsel].rtclass & (1 << csel)) == 0);

  py_stats(stat, AL(stat));

  // Race & class
  struct raceS* r_ptr = &raceD[rsel];
  for (int it = 0; it < MAX_A; ++it) {
    stat[it] += r_ptr->attr[it];
  }
  // TBD: age
  // TBD: height
  hitdie = r_ptr->bhitdie;
  uD.ridx = rsel;
  uD.bth = r_ptr->bth;
  uD.search = r_ptr->srh;
  uD.fos = r_ptr->fos;
  uD.disarm = r_ptr->dis;
  uD.stealth = r_ptr->stl;
  uD.save = r_ptr->bsav;
  uD.infra = r_ptr->infra;
  uD.mult_exp = r_ptr->b_exp;
  int male = 1 - randint(2);
  if (male) {
    uD.wt = randnor(r_ptr->m_b_wt, r_ptr->m_m_wt);
  } else {
    uD.wt = randnor(r_ptr->f_b_wt, r_ptr->f_m_wt);
  }
  uD.male = male;

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

  if (TEST) {
    for (int it = 0; it < MAX_A; ++it) {
      statD.max_stat[it] += 1;
    }
  }

  uD.mhp = uD.chp = hitdie + con_adj();
  uD.chp_frac = 0;
  uD.lev = 1;

  int min_value = (MAX_PLAYER_LEVEL * 3 / 8 * (hitdie - 1)) + MAX_PLAYER_LEVEL;
  int max_value = (MAX_PLAYER_LEVEL * 5 / 8 * (hitdie - 1)) + MAX_PLAYER_LEVEL;
  player_hpD[0] = hitdie;
  do {
    for (int it = 1; it < MAX_PLAYER_LEVEL; it++) {
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
    food->number = 5;
    invenD[it] = food->id;
  }
  int inven_test[] = {};  // scrolls: 174 222
  for (int it = 0; it < AL(inven_test); ++it) {
    int iidx = inven_slot();
    if (iidx == -1) break;

    struct objS* obj = obj_use();
    tr_obj_copy(inven_test[it], obj);
    magic_treasure(obj, dun_level);
    tr_make_known(&treasureD[obj->tidx]);
    invenD[iidx] = obj->id;
  }
  int magik_test[] = {};
  for (int it = 0; it < AL(magik_test); ++it) {
    int iidx = inven_slot();
    if (iidx == -1) break;

    struct objS* obj = obj_use();
    do {
      tr_obj_copy(magik_test[it], obj);
      magic_treasure(obj, dun_level);
    } while ((TR_SPEED & obj->flags) == 0 || obj->p1 <= 0);
    invenD[iidx] = obj->id;
  }

  uD.food = 7500;
  uD.food_digest = 2;
  uD.gold = 100;

  if (HACK) {
    uD.gold = 100000;
  }
}
void
magic_init()
{
  int i, j, k, h;
  void* tmp;
  uint32_t seed;
  char string[80];

  seed = rnd_seed;
  rnd_seed = obj_seed;

  store_init();

  /* The first 3 entries for colors are fixed, (slime & apple juice, water) */
  for (i = 3; i < AL(colors); i++) {
    j = randint(AL(colors) - 3) + 2;
    tmp = colors[i];
    colors[i] = colors[j];
    colors[j] = tmp;
  }
  for (i = 0; i < AL(woods); i++) {
    j = randint(AL(woods)) - 1;
    tmp = woods[i];
    woods[i] = woods[j];
    woods[j] = tmp;
  }
  for (i = 0; i < AL(metals); i++) {
    j = randint(AL(metals)) - 1;
    tmp = metals[i];
    metals[i] = metals[j];
    metals[j] = tmp;
  }
  for (i = 0; i < AL(rocks); i++) {
    j = randint(AL(rocks)) - 1;
    tmp = rocks[i];
    rocks[i] = rocks[j];
    rocks[j] = tmp;
  }
  for (i = 0; i < AL(amulets); i++) {
    j = randint(AL(amulets)) - 1;
    tmp = amulets[i];
    amulets[i] = amulets[j];
    amulets[j] = tmp;
  }
  for (i = 0; i < AL(mushrooms); i++) {
    j = randint(AL(mushrooms)) - 1;
    tmp = mushrooms[i];
    mushrooms[i] = mushrooms[j];
    mushrooms[j] = tmp;
  }
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
  rnd_seed = seed;
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
void
lose_stat(sidx)
{
  uint32_t sustain = sustain_stat(sidx);
  if (py_tr(sustain)) {
    MSG("%s for a moment,  it passes.", stat_lossD[sidx]);
  } else {
    dec_stat(sidx);
    MSG("%s.", stat_lossD[sidx]);
  }
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
py_death()
{
  char c;
  int row, col;

  msg_pause();
  MSG("Killed by %s.", death_descD);
  row = col = 0;
  for (int it = 0; it < AL(grave); ++it) {
    if (grave[it] == '\n') {
      screenD[row][col] = 0;
      screen_usedD[row] = col;
      row += 1;
      col = 0;
    } else {
      screenD[row][col] = grave[it];
      col += 1;
    }
  }

  // TBD: platform considerations...
  draw();
  c = inkey();
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
  }
  panel_update(&panelD, uD.y, uD.x, TRUE);
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
void
py_add_food(num)
{
  int food, extra, penalty;

  food = uD.food;
  if (food < 0) food = 0;
  food += num;
  if (food > PLAYER_FOOD_MAX) {
    msg_print("You are bloated from overeating.");

    /* Calculate how much of num is responsible for the bloating.
       Give the player food credit for 1/50, and slow him for that many
       turns also.  */
    extra = food - PLAYER_FOOD_MAX;
    if (extra > num) extra = num;
    penalty = extra / 32;

    maD[MA_SLOW] += penalty;
    if (extra == num)
      food = food - num + penalty;
    else
      food = PLAYER_FOOD_MAX + penalty;
  } else if (food > PLAYER_FOOD_FULL)
    msg_print("You are full.");
  uD.food = food;
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
int
lev_exp(lev)
{
  return player_exp[lev - 1] * uD.mult_exp / 100;
}
void
py_experience()
{
  int exp = uD.exp;
  int lev = uD.lev;

  if (exp > MAX_EXP) exp = MAX_EXP;

  while ((lev < MAX_PLAYER_LEVEL) && lev_exp(lev) <= exp) {
    int dif_exp, need_exp;

    lev += 1;
    MSG("Welcome to level %d.", lev);
    calc_hitpoints(lev);

    need_exp = lev_exp(lev);
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
    while ((lev < MAX_PLAYER_LEVEL) && lev_exp(lev) <= exp) {
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

  lev = 1;
  while (lev_exp(lev) <= exp) lev++;

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
// TBD: We may loop infinitely with the added restriction of oidx != 0
// Phase door and thief (short range) teleport run a higher risk
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
           (caveD[ty][tx].midx != 0) || (caveD[ty][tx].oidx != 0));
  *uy = ty;
  *ux = tx;
}
void teleport_away(midx, dis) int midx, dis;
{
  int fy, fx, yn, xn, ctr, cdis;
  struct monS* m_ptr;

  m_ptr = &entity_monD[midx];
  fy = m_ptr->fy;
  fx = m_ptr->fx;
  ctr = 0;
  do {
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
    cdis = distance(uD.y, uD.x, yn, xn);
  } while (cdis < 1);

  move_rec(fy, fx, yn, xn);
  m_ptr->fy = yn;
  m_ptr->fx = xn;
  update_mon(midx);
}
void
get_flags(int typ, uint32_t* weapon_type, int* harm_type, int (**destroy)())
{
  *destroy = set_null;

  switch (typ) {
    case GF_MAGIC_MISSILE:
      *weapon_type = 0;
      *harm_type = 0;
      break;
    case GF_LIGHTNING:
      *weapon_type = CS_BR_LIGHT;
      *harm_type = CD_LIGHT;
      *destroy = vuln_lightning;
      break;
    case GF_POISON_GAS:
      *weapon_type = CS_BR_GAS;
      *harm_type = CD_POISON;
      break;
    case GF_ACID:
      *weapon_type = CS_BR_ACID;
      *harm_type = CD_ACID;
      // Distinct from vuln_acid (acid_dam attack)
      *destroy = vuln_acid_breath;
      break;
    case GF_FROST:
      *weapon_type = CS_BR_FROST;
      *harm_type = CD_FROST;
      *destroy = vuln_frost;
      break;
    case GF_FIRE:
      *weapon_type = CS_BR_FIRE;
      *harm_type = CD_FIRE;
      // Distinct from vuln_fire (fire_dam attack)
      *destroy = vuln_fire_breath;
      break;
    case GF_HOLY_ORB:
      *weapon_type = 0;
      *harm_type = CD_EVIL;
      break;
    default:
      msg_print("ERROR in get_flags()\n");
  }
}
void
light_line(dir, y, x)
{
  int i;
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;
  int dist, flag;

  dist = -1;
  flag = FALSE;
  do {
    /* put mmove at end because want to light up current spot */
    dist++;
    c_ptr = &caveD[y][x];
    if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
      flag = TRUE;
    else {
      if ((c_ptr->cflag & CF_PERM_LIGHT) == 0) {
        if (c_ptr->fval == FLOOR_DARK) c_ptr->fval = FLOOR_LIGHT;
        if ((c_ptr->cflag & CF_ROOM) != 0 && panel_contains(&panelD, y, x))
          light_room(y, x);
      }
      c_ptr->cflag |= CF_PERM_LIGHT;

      if (c_ptr->midx) {
        m_ptr = &entity_monD[c_ptr->midx];
        cr_ptr = &creatureD[m_ptr->cidx];
        mon_desc(c_ptr->midx);
        /* light up and draw monster */
        update_mon(c_ptr->midx);
        if (CD_LIGHT & cr_ptr->cdefense) {
          if (mon_take_hit(c_ptr->midx, damroll(2, 8))) {
            MSG("%s shrivels away in the light!", descD);
            py_experience();
          } else {
            MSG("%s cringes from the light!", descD);
          }
        }
      }
    }
    (void)mmove(dir, &y, &x);
  } while (!flag);
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
    if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
      flag = TRUE;
    else {
      if (c_ptr->midx) {
        flag = TRUE;
        m_ptr = &entity_monD[c_ptr->midx];
        cre = &creatureD[m_ptr->cidx];

        // TBD: animate?
        m_ptr->mlit = TRUE;

        // TBD: lowercase
        mon_desc(c_ptr->midx);
        MSG("The %s strikes %s.", bolt_typ, descD);
        if (harm_type & cre->cdefense) {
          dam = dam * 2;
        } else if (weapon_type & cre->spells) {
          dam = dam / 4;
        }
        mon_desc(c_ptr->midx);
        if (mon_take_hit(c_ptr->midx, dam)) {
          MSG("%s dies in a fit of agony.", descD);
          py_experience();
        } else {
          MSG("%s screams in agony.", descD);
        }
      } else if (panel_contains(&panelD, y, x)) {
        // TBD: && (py.flags.blind < 1)
        // print('*', y, x);
        /* show the fire_bolt */
        // put_qio();
      }
    }
    oldy = y;
    oldx = x;
  } while (!flag);
}
void fire_ball(typ, dir, y, x, dam_hp, descrip) char* descrip;
{
  int i, j;
  int dam, max_dis, thit, tkill, k, tmp;
  int oldy, oldx, dist, flag, harm_type;
  uint32_t weapon_type;
  int (*destroy)();
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;

  thit = 0;
  tkill = 0;
  max_dis = 2;
  get_flags(typ, &weapon_type, &harm_type, &destroy);
  flag = FALSE;
  oldy = y;
  oldx = x;
  dist = 0;
  do {
    mmove(dir, &y, &x);
    dist++;
    if (dist > OBJ_BOLT_RANGE)
      flag = TRUE;
    else {
      c_ptr = &caveD[y][x];
      if ((c_ptr->fval >= MIN_CLOSED_SPACE) || (c_ptr->midx)) {
        flag = TRUE;
        if (c_ptr->fval >= MIN_CLOSED_SPACE) {
          y = oldy;
          x = oldx;
        }
        /* The ball hits and explodes.  	     */
        /* The explosion.  		     */
        for (i = y - max_dis; i <= y + max_dis; i++)
          for (j = x - max_dis; j <= x + max_dis; j++)
            if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis) &&
                los(y, x, i, j)) {
              c_ptr = &caveD[i][j];
              if ((c_ptr->oidx) && (*destroy)(&entity_objD[c_ptr->oidx]))
                delete_object(i, j);
              if (c_ptr->fval <= MAX_OPEN_SPACE) {
                if (c_ptr->midx) {
                  m_ptr = &entity_monD[c_ptr->midx];
                  cr_ptr = &creatureD[m_ptr->cidx];

                  thit++;
                  dam = dam_hp;
                  if (harm_type & cr_ptr->cdefense) {
                    dam = dam * 2;
                  } else if (weapon_type & cr_ptr->spells) {
                    dam = dam / 4;
                  }
                  dam = (dam / (distance(i, j, y, x) + 1));
                  if (mon_take_hit(c_ptr->midx, dam)) tkill++;
                }
                // else if (panel_contains(&panelD, i, j) && (countD.blind ==
                // 0))
                //   print('*', i, j);
              }
            }
        /* show ball of whatever */
        // put_qio();

        if (thit == 1) {
          MSG("The %s envelops a creature!", descrip);
        } else if (thit > 1) {
          MSG("The %s envelops several creatures!", descrip);
        }
        if (tkill == 1)
          msg_print("There is a scream of agony!");
        else if (tkill > 1)
          msg_print("There are several screams of agony!");
        if (tkill >= 0) py_experience();
        /* End ball hitting.  	     */
      }
      // else if (panel_contains(&panelD, y, x) && (countD.blind == 0)) {
      //   print('*', y, x);
      //   /* show bolt */
      //   put_qio();
      // }
      oldy = y;
      oldx = x;
    }
  } while (!flag);
}
int
twall(y, x)
{
  int i, j;
  struct caveS* c_ptr;
  int res, found;

  res = FALSE;
  c_ptr = &caveD[y][x];
  if (c_ptr->cflag & CF_ROOM) {
    /* should become a room space, check to see whether it should be
       LIGHT_FLOOR or DARK_FLOOR */
    found = FALSE;
    for (i = y - 1; i <= y + 1; i++)
      for (j = x - 1; j <= x + 1; j++)
        if (caveD[i][j].fval <= FLOOR_CORR) {
          c_ptr->fval = caveD[i][j].fval;
          c_ptr->cflag = caveD[i][j].cflag;
          found = TRUE;
          break;
        }
    if (!found) {
      c_ptr->fval = FLOOR_CORR;
    }
  } else {
    /* should become a corridor space */
    c_ptr->fval = FLOOR_CORR;
  }
  c_ptr->cflag &= ~CF_FIELDMARK;
  if (panel_contains(&panelD, y, x))
    if (cave_lit(c_ptr) && c_ptr->oidx) msg_print("You have found something!");
  res = TRUE;

  return (res);
}
int
wall_to_mud(dir, y, x)
{
  int i, wall, rubble, dist, lit;
  int flag;
  struct caveS* c_ptr;
  struct objS* obj;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;

  wall = FALSE;
  flag = FALSE;
  dist = 0;
  do {
    mmove(dir, &y, &x);
    dist++;
    c_ptr = &caveD[y][x];
    obj = &entity_objD[c_ptr->oidx];
    lit = cave_lit(c_ptr) || (c_ptr->cflag & CF_FIELDMARK) != 0;
    rubble = (obj->tval == TV_RUBBLE);
    m_ptr = &entity_monD[c_ptr->midx];
    cr_ptr = &creatureD[m_ptr->cidx];

    /* note, this ray can move through walls as it turns them to mud */
    if (dist == OBJ_BOLT_RANGE) flag = TRUE;
    if ((c_ptr->fval >= MIN_WALL) && (c_ptr->fval != BOUNDARY_WALL)) {
      flag = TRUE;
      twall(y, x);
      if (lit) {
        msg_print("The wall turns into mud.");
        wall = TRUE;
      }
    } else if (obj->id && (c_ptr->fval >= MIN_CLOSED_SPACE)) {
      flag = TRUE;
      if (panel_contains(&panelD, y, x) && lit) {
        obj_desc(obj, FALSE);
        MSG("The %s turns into mud.", descD);
        wall = TRUE;
      }
      delete_object(y, x);
      if (rubble) {
        if (randint(10) == 1) {
          place_object(y, x, FALSE);
          if (lit) msg_print("You have found something!");
        }
      }
    }
    if (c_ptr->midx) {
      if (CD_STONE & cr_ptr->cdefense) {
        mon_desc(c_ptr->midx);
        /* Should get these messages even if the monster is not
           visible.  */
        if (mon_take_hit(c_ptr->midx, 100)) {
          MSG("%s dissolves!", descD);
          py_experience();
        } else {
          MSG("%s grunts in pain!", descD);
        }
        flag = TRUE;
      }
    }
  } while (!flag);
  return (wall);
}
int
poly_monster(dir, y, x)
int dir, y, x;
{
  int dist, flag, poly;
  struct caveS* c_ptr;
  struct creatureS* cr_ptr;
  struct monS* m_ptr;

  poly = FALSE;
  flag = FALSE;
  dist = 0;
  do {
    mmove(dir, &y, &x);
    dist++;
    c_ptr = &caveD[y][x];
    if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
      flag = TRUE;
    else if (c_ptr->midx) {
      m_ptr = &entity_monD[c_ptr->midx];
      cr_ptr = &creatureD[m_ptr->cidx];
      if (randint(MAX_MON_LEVEL) > cr_ptr->level) {
        flag = TRUE;
        mon_unuse(m_ptr);
        c_ptr->midx = 0;
        /* Place_monster() should always return TRUE here.  */
        poly = place_monster(
            y, x, randint(m_level[MAX_MON_LEVEL] - m_level[0]) - 1 + m_level[0],
            FALSE);
        /* don't test c_ptr->fm here, only pl/tl */
        if (poly && panel_contains(&panelD, y, x) && cave_lit(c_ptr))
          poly = TRUE;
      } else {
        mon_desc(c_ptr->midx);
        MSG("%s is unaffected.", descD);
      }
    }
  } while (!flag);
  return (poly);
}
int
hp_monster(dir, y, x, dam)
{
  int i;
  int flag, dist, monster;
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;

  monster = FALSE;
  flag = FALSE;
  dist = 0;
  do {
    mmove(dir, &y, &x);
    dist++;
    c_ptr = &caveD[y][x];
    if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
      flag = TRUE;
    else if (c_ptr->midx) {
      flag = TRUE;
      m_ptr = &entity_monD[c_ptr->midx];
      cr_ptr = &creatureD[m_ptr->cidx];
      mon_desc(c_ptr->midx);
      monster = TRUE;
      if (mon_take_hit(c_ptr->midx, dam)) {
        MSG("%s dies in a fit of agony.", descD);
        py_experience();
      } else if (dam > 0) {
        MSG("%s screams in agony.", descD);
      } else {
        MSG("%s appears healthier.", descD);
      }
    }
  } while (!flag);
  return (monster);
}
int
confuse_monster(dir, y, x)
{
  int flag, dist, confuse;
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;

  confuse = FALSE;
  flag = FALSE;
  dist = 0;
  do {
    mmove(dir, &y, &x);
    dist++;
    c_ptr = &caveD[y][x];
    if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
      flag = TRUE;
    else if (c_ptr->midx) {
      m_ptr = &entity_monD[c_ptr->midx];
      cr_ptr = &creatureD[m_ptr->cidx];
      mon_desc(c_ptr->midx);
      flag = TRUE;
      if ((randint(MAX_MON_LEVEL) < cr_ptr->level) ||
          (CD_NO_SLEEP & cr_ptr->cdefense)) {
        /* Monsters which resisted the attack should wake up.
           Monsters with innate resistence ignore the attack.  */
        if (!(CD_NO_SLEEP & cr_ptr->cdefense)) m_ptr->msleep = 0;
        MSG("%s is unaffected.", descD);
      } else {
        if (m_ptr->mconfused)
          m_ptr->mconfused += 3;
        else
          m_ptr->mconfused = 2 + randint(16);
        confuse = TRUE;
        m_ptr->msleep = 0;
        MSG("%s appears confused.", descD);
      }
    }
  } while (!flag);
  return (confuse);
}
int
sleep_monster(dir, y, x)
{
  int flag, dist, sleep;
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;

  sleep = FALSE;
  flag = FALSE;
  dist = 0;
  do {
    mmove(dir, &y, &x);
    dist++;
    c_ptr = &caveD[y][x];
    if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
      flag = TRUE;
    else if (c_ptr->midx) {
      m_ptr = &entity_monD[c_ptr->midx];
      cr_ptr = &creatureD[m_ptr->cidx];
      flag = TRUE;
      mon_desc(c_ptr->midx);
      if ((randint(MAX_MON_LEVEL) < cr_ptr->level) ||
          (CD_NO_SLEEP & cr_ptr->cdefense)) {
        MSG("%s is unaffected.", descD);
      } else {
        m_ptr->msleep = 500;
        sleep = TRUE;
        MSG("%s falls asleep.", descD);
      }
    }
  } while (!flag);
  return (sleep);
}
int
sleep_monster_aoe()
{
  int y, x, i, sleep, cdis;
  struct creatureS* cr_ptr;

  y = uD.y;
  x = uD.x;
  sleep = FALSE;
  FOR_EACH(mon, {
    cr_ptr = &creatureD[mon->cidx];
    mon_desc(it_index);
    cdis = distance(y, x, mon->fy, mon->fx);
    if ((cdis > MAX_SIGHT) || !los(y, x, mon->fy, mon->fx)) continue;

    if ((randint(MAX_MON_LEVEL) < cr_ptr->level) ||
        (CD_NO_SLEEP & cr_ptr->cdefense)) {
      if (mon->mlit) MSG("%s is unaffected.", descD);
    } else {
      mon->msleep = 500;
      if (mon->mlit) {
        MSG("%s falls asleep.", descD);
        sleep = TRUE;
      }
    }
  });
  return (sleep);
}
int
mass_poly()
{
  int cdis, y, x, fy, fx, mass;
  struct creatureS* cr_ptr;

  y = uD.y;
  x = uD.x;
  mass = FALSE;
  FOR_EACH(mon, {
    fy = mon->fy;
    fx = mon->fx;
    cdis = distance(y, x, fy, fx);
    if (cdis <= MAX_SIGHT) {
      cr_ptr = &creatureD[mon->cidx];
      if ((cr_ptr->cmove & CM_WIN) == 0) {
        caveD[fy][fx].midx = 0;
        mon_unuse(mon);

        mass = place_monster(
            fy, fx,
            randint(m_level[MAX_MON_LEVEL] - m_level[0]) - 1 + m_level[0],
            FALSE);
      }
    }
  });
  return (mass);
}
int
drain_life(dir, y, x)
{
  int i;
  int flag, dist, drain;
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* r_ptr;

  drain = FALSE;
  flag = FALSE;
  dist = 0;
  do {
    mmove(dir, &y, &x);
    dist++;
    c_ptr = &caveD[y][x];
    if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
      flag = TRUE;
    else if (c_ptr->midx) {
      flag = TRUE;
      m_ptr = &entity_monD[c_ptr->midx];
      r_ptr = &creatureD[m_ptr->cidx];
      if ((r_ptr->cdefense & CD_UNDEAD) == 0) {
        drain = TRUE;
        mon_desc(c_ptr->midx);
        if (mon_take_hit(c_ptr->midx, 75)) {
          MSG("%s dies in a fit of agony.", descD);
          py_experience();
        } else {
          MSG("%s screams in agony.", descD);
        }
      }
    }
  } while (!flag);
  return (drain);
}
int
td_destroy2(dir, y, x)
{
  int destroy2, dist;
  struct caveS* c_ptr;
  struct objS* obj;

  destroy2 = FALSE;
  dist = 0;
  do {
    mmove(dir, &y, &x);
    dist++;
    c_ptr = &caveD[y][x];
    /* must move into first closed spot, as it might be a secret door */
    if (c_ptr->oidx) {
      obj = &entity_objD[c_ptr->oidx];
      if ((obj->tval == TV_INVIS_TRAP) || (obj->tval == TV_CLOSED_DOOR) ||
          (obj->tval == TV_VIS_TRAP) || (obj->tval == TV_OPEN_DOOR) ||
          (obj->tval == TV_SECRET_DOOR)) {
        delete_object(y, x);
        msg_print("There is a bright flash of light!");
        destroy2 = TRUE;
      }
      // TBD: Chest
      // else if ((obj->tval == TV_CHEST) && (obj->flags != 0)) {
      //  msg_print("Click!");
      //  obj->flags &= ~(CH_TRAPPED | CH_LOCKED);
      //  destroy2 = TRUE;
      //  obj->sn = SN_UNLOCKED;
      //  obj->idflag = ID_REVEAL;
      //}
    }
  } while ((dist <= OBJ_BOLT_RANGE) || c_ptr->fval <= MAX_OPEN_SPACE);
  return (destroy2);
}
int
build_wall(dir, y, x)
{
  int i;
  int build, damage, dist, flag;
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;

  build = FALSE;
  dist = 0;
  flag = FALSE;
  do {
    mmove(dir, &y, &x);
    dist++;
    c_ptr = &caveD[y][x];
    if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
      flag = TRUE;
    else {
      if (c_ptr->oidx) delete_object(y, x);
      if (c_ptr->midx) {
        /* stop the wall building */
        flag = TRUE;
        m_ptr = &entity_monD[c_ptr->midx];
        cr_ptr = &creatureD[m_ptr->cidx];
        mon_desc(c_ptr->midx);

        if (!(cr_ptr->cmove & CM_PHASE)) {
          /* monster does not move, can't escape the wall */
          if (cr_ptr->cmove & CM_ATTACK_ONLY)
            damage = 3000; /* this will kill everything */
          else
            damage = damroll(4, 8);

          MSG("%s wails out in pain!", descD);
          if (mon_take_hit(c_ptr->midx, damage)) {
            MSG("%s is embedded in the rock.", descD);
            py_experience();
          }
        } else if (cr_ptr->cchar == 'E' || cr_ptr->cchar == 'X') {
          if (m_ptr->mlit) MSG("%s grows larger.", descD);
          /* must be an earth elemental or an earth spirit, or a Xorn
             increase its hit points */
          m_ptr->hp += damroll(4, 8);
        }
      }
      c_ptr->fval = GRANITE_WALL;  // TBD: was MAGMA_WALL;
      c_ptr->cflag &= ~CF_FIELDMARK;
      i++;
      build = TRUE;
    }
  } while (!flag);
  return (build);
}
int
clone_monster(dir, y, x)
{
  struct caveS* c_ptr;
  int dist, flag;

  dist = 0;
  flag = FALSE;
  do {
    mmove(dir, &y, &x);
    dist++;
    c_ptr = &caveD[y][x];
    if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
      flag = TRUE;
    else if (c_ptr->midx) {
      entity_monD[c_ptr->midx].msleep = 0;
      return mon_multiply(&entity_monD[c_ptr->midx]);
    }
  } while (!flag);
  return (FALSE);
}
int
teleport_monster(dir, y, x)
{
  int flag, result, dist;
  struct caveS* c_ptr;

  flag = FALSE;
  result = FALSE;
  dist = 0;
  do {
    mmove(dir, &y, &x);
    dist++;
    c_ptr = &caveD[y][x];
    if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
      flag = TRUE;
    else if (c_ptr->midx) {
      entity_monD[c_ptr->midx].msleep = 0; /* wake it up */
      teleport_away(c_ptr->midx, MAX_SIGHT);
      result = TRUE;
    }
  } while (!flag);
  return (result);
}
int
disarm_all(dir, y, x)
{
  struct caveS* c_ptr;
  struct objS* obj;
  int disarm, dist;

  disarm = FALSE;
  dist = -1;
  do {
    /* put mmove at end, in case standing on a trap */
    dist++;
    c_ptr = &caveD[y][x];
    obj = &entity_objD[c_ptr->oidx];
    /* note, must continue upto and including the first non open space,
       because closed doors have fval greater than MAX_OPEN_SPACE */
    if ((obj->tval == TV_INVIS_TRAP) || (obj->tval == TV_VIS_TRAP)) {
      delete_object(y, x);
      disarm = TRUE;
    } else if (obj->tval == TV_CLOSED_DOOR)
      obj->p1 = 0; /* Locked or jammed doors become merely closed. */
    else if (obj->tval == TV_SECRET_DOOR) {
      c_ptr->cflag |= CF_FIELDMARK;
      obj->tval = TV_CLOSED_DOOR;
      obj->tchar = '+';
      disarm = TRUE;
    }
    // else if ((obj->tval == TV_CHEST) && (obj->flags != 0)) {
    //   msg_print("Click!");
    //   obj->flags &= ~(CH_TRAPPED | CH_LOCKED);
    //   disarm = TRUE;
    //   obj->sn = SN_UNLOCKED;
    //   known2(obj);
    // }

    mmove(dir, &y, &x);
  } while ((dist <= OBJ_BOLT_RANGE) && c_ptr->fval <= MAX_OPEN_SPACE);
  return (disarm);
}
int
dispel_creature(cflag, damage)
{
  int y, x, dispel;
  register struct creatureS* cr_ptr;

  y = uD.y;
  x = uD.x;
  dispel = FALSE;
  FOR_EACH(mon, {
    cr_ptr = &creatureD[mon->cidx];
    if ((cflag & cr_ptr->cdefense) &&
        (distance(y, x, mon->fy, mon->fx) <= MAX_SIGHT) &&
        los(y, x, mon->fy, mon->fx)) {
      dispel = TRUE;
      // c_recall[mon->mptr].r_cdefense |= cflag;
      mon_desc(it_index);
      /* Should get these messages even if the monster is not
         visible.  */
      if (mon_take_hit(it_index, randint(damage))) {
        MSG("%s dissolves!", descD);
        py_experience();
      } else {
        MSG("%s shudders.", descD);
      }
    }
  });
  return (dispel);
}
int
mass_genocide(y, x)
{
  register int count;
  register struct creatureS* cr_ptr;

  count = 0;
  FOR_EACH(mon, {
    if (distance(y, x, mon->fy, mon->fx) <= MAX_SIGHT) {
      cr_ptr = &creatureD[mon->cidx];
      if ((cr_ptr->cmove & CM_WIN) == 0) {
        caveD[mon->fy][mon->fx].midx = 0;
        mon_unuse(mon);
        count += 1;
      }
    }
  });
  return count > 0;
}
void
inven_recharge(iidx, amount)
{
  int chance;
  int res;
  struct objS* i_ptr;

  i_ptr = obj_get(invenD[iidx]);
  /* recharge I = recharge(20) = 1/6 failure for empty 10th level wand */
  /* recharge II = recharge(60) = 1/10 failure for empty 10th level wand*/
  /* make it harder to recharge high level, and highly charged wands, note
     that chance can be negative, so check its value before trying to call
     randint().  */
  chance = amount + 50 - i_ptr->level - i_ptr->p1;
  if (chance < 19)
    chance = 1; /* Automatic failure.  */
  else
    chance = randint(chance / 10);

  if (chance == 1) {
    msg_print("There is a bright flash of light.");
    obj_unuse(i_ptr);
    invenD[iidx] = 0;
  } else {
    amount = (amount / (i_ptr->level + 2)) + 1;
    i_ptr->p1 += 2 + randint(amount);
    i_ptr->idflag &= ~(ID_REVEAL | ID_EMPTY);
  }
}
int
extermination()
{
  struct creatureS* cr_ptr;
  int count;

  count = 0;
  FOR_EACH(mon, {
    cr_ptr = &creatureD[mon->cidx];
    if (cr_ptr->cmove & CM_MULTIPLY) {
      caveD[mon->fy][mon->fx].midx = 0;
      mon_unuse(mon);
      count += 1;
    }
  });
  return count;
}
int
sleep_adjacent(y, x)
{
  register int i, j;
  register struct caveS* c_ptr;
  register struct monS* m_ptr;
  register struct creatureS* cr_ptr;
  int sleep;

  sleep = FALSE;
  for (i = y - 1; i <= y + 1; i++)
    for (j = x - 1; j <= x + 1; j++) {
      c_ptr = &caveD[i][j];
      if (c_ptr->midx) {
        m_ptr = &entity_monD[c_ptr->midx];
        cr_ptr = &creatureD[m_ptr->cidx];

        mon_desc(c_ptr->midx);
        if ((randint(MAX_MON_LEVEL) < cr_ptr->level) ||
            (CD_NO_SLEEP & cr_ptr->cdefense)) {
          MSG("%s is unaffected.", descD);
        } else {
          sleep = TRUE;
          m_ptr->msleep = 500;
          MSG("%s falls asleep.", descD);
        }
      }
    }
  return (sleep);
}
void
create_food(y, x)
{
  struct objS* obj;

  if (caveD[y][x].oidx) {
    msg_print("There is already an object under you.");
    return;
  }

  obj = obj_use();
  if (obj->id) {
    caveD[y][x].oidx = obj_index(obj);
    tr_obj_copy(OBJ_MUSH_TIDX, obj);
    obj->fy = y;
    obj->fx = x;
    msg_print("Food rolls beneath your feet.");
  }
}
void
warding_glyph(y, x)
{
  struct objS* obj;

  obj = obj_use();
  if (obj->id) {
    caveD[y][x].oidx = obj_index(obj);
    tr_obj_copy(OBJ_RUNE_TIDX, obj);
    obj->fy = y;
    obj->fx = x;
  }
}
int
trap_creation(y, x)
{
  register int i, j, trap;
  register struct caveS* c_ptr;

  trap = FALSE;
  for (i = y - 1; i <= y + 1; i++)
    for (j = x - 1; j <= x + 1; j++) {
      if (i != y || j != x) {
        c_ptr = &caveD[i][j];
        if (c_ptr->fval <= MAX_FLOOR) {
          if (c_ptr->oidx) delete_object(i, j);
          place_trap(i, j, randint(MAX_TRAP) - 1);
          trap = TRUE;
        }
      }
    }
  return (trap);
}
int
td_destroy(y, x)
{
  register int i, j, destroy;
  register struct caveS* c_ptr;
  register struct objS* obj;

  destroy = FALSE;
  for (i = y - 1; i <= y + 1; i++)
    for (j = x - 1; j <= x + 1; j++) {
      c_ptr = &caveD[i][j];
      obj = &entity_objD[c_ptr->oidx];
      if (obj->tval == TV_CLOSED_DOOR || obj->tval == TV_OPEN_DOOR ||
          obj->tval == TV_SECRET_DOOR) {
        delete_object(i, j);
        destroy = TRUE;
      }
      if (obj->tval == TV_VIS_TRAP || obj->tval == TV_INVIS_TRAP) {
        delete_object(i, j);
        destroy = TRUE;
      }
      // TBD: Chests?
    }
  return (destroy);
}
int
mon_speed(mon)
struct monS* mon;
{
  struct creatureS* cr_ptr;
  cr_ptr = &creatureD[mon->cidx];
  return mon->mspeed + cr_ptr->speed - 10 + uD.pspeed;
}
int
aggravate_monster(dis_affect)
{
  register int y, x, count;

  y = uD.y;
  x = uD.x;
  count = 0;
  FOR_EACH(mon, {
    mon->msleep = 0;
    if (distance(mon->fy, mon->fx, y, x) <= dis_affect) {
      count += 1;
      if (mon_speed(mon) < 2) mon->mspeed += 1;
    }
  });
  if (count) msg_print("You hear a sudden stirring in the distance!");
  return (count > 0);
}
int
door_creation()
{
  register int y, x, i, j, door;
  register struct caveS* c_ptr;

  door = FALSE;
  y = uD.y;
  x = uD.x;
  for (i = y - 1; i <= y + 1; i++)
    for (j = x - 1; j <= x + 1; j++)
      if ((i != y) || (j != x)) {
        c_ptr = &caveD[i][j];
        if (c_ptr->fval <= MAX_FLOOR) {
          door = TRUE;
          if (c_ptr->oidx != 0) delete_object(i, j);
          place_closed_door(0, i, j);
        }
      }
  return (door);
}
int
speed_monster_aoe(spd)
{
  register int y, x, see_count;
  struct creatureS* cr_ptr;

  y = uD.y;
  x = uD.x;
  see_count = 0;
  FOR_EACH(mon, {
    if (distance(y, x, mon->fy, mon->fx) <= MAX_SIGHT &&
        los(y, x, mon->fy, mon->fx)) {
      mon->msleep = 0;
      mon_desc(it_index);
      cr_ptr = &creatureD[mon->cidx];
      if (spd > 0) {
        mon->mspeed += spd;
        if (mon->mlit) {
          see_count += 1;
          MSG("%s starts moving faster.", descD);
        }
      } else if (randint(MAX_MON_LEVEL) > cr_ptr->level) {
        mon->mspeed += spd;
        if (mon->mlit) {
          MSG("%s starts moving slower.", descD);
          see_count += 1;
        }
      } else if (mon->mlit) {
        MSG("%s resists the affects.", descD);
      }
    }
  });
  return (see_count);
}
// TBD: very similar bolt()
int
speed_monster(dir, y, x, spd)
{
  int flag, dist, see_count;
  register struct caveS* c_ptr;
  register struct monS* m_ptr;
  register struct creatureS* cr_ptr;

  see_count = 0;
  flag = FALSE;
  dist = 0;
  do {
    mmove(dir, &y, &x);
    dist++;
    c_ptr = &caveD[y][x];
    if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE)
      flag = TRUE;
    else if (c_ptr->midx) {
      flag = TRUE;
      m_ptr = &entity_monD[c_ptr->midx];
      cr_ptr = &creatureD[m_ptr->cidx];
      m_ptr->msleep = 0;
      mon_desc(c_ptr->midx);
      if (spd > 0) {
        m_ptr->mspeed += spd;
        MSG("%s starts moving faster.", descD);
        see_count += 1;
      } else if (randint(MAX_MON_LEVEL) > cr_ptr->level) {
        m_ptr->mspeed += spd;
        MSG("%s starts moving slower.", descD);
        see_count += 1;
      } else {
        MSG("%s is unaffected.", descD);
      }
    }
  } while (!flag);
  return (see_count);
}
static void
replace_spot(y, x, typ)
{
  register struct caveS* c_ptr;

  c_ptr = &caveD[y][x];
  switch (typ) {
    case 1:
    case 2:
    case 3:
      c_ptr->fval = FLOOR_CORR;
      break;
    case 4:
    case 7:
    case 10:
      // c_ptr->fval = QUARTZ_WALL;
      // break;
    case 5:
    case 8:
    case 11:
      // c_ptr->fval = MAGMA_WALL;
      // break;
    case 6:
    case 9:
    case 12:
      c_ptr->fval = GRANITE_WALL;
      break;
  }
  c_ptr->cflag = 0; /* this is no longer part of a room */
  if (c_ptr->oidx) delete_object(y, x);
  if (c_ptr->midx) {
    mon_unuse(&entity_monD[c_ptr->midx]);
    c_ptr->midx = 0;
  }
}
void
earthquake()
{
  int y, x, i, j;
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;
  int damage, tmp;

  y = uD.y;
  x = uD.x;
  for (i = y - 8; i <= y + 8; i++)
    for (j = x - 8; j <= x + 8; j++)
      if (((i != y) || (j != x)) && in_bounds(i, j) && (randint(8) == 1)) {
        c_ptr = &caveD[i][j];
        if (c_ptr->oidx) delete_object(i, j);
        if (c_ptr->midx) {
          m_ptr = &entity_monD[c_ptr->midx];
          cr_ptr = &creatureD[m_ptr->cidx];
          mon_desc(c_ptr->midx);

          if (!(cr_ptr->cmove & CM_PHASE)) {
            if (cr_ptr->cmove & CM_ATTACK_ONLY)
              damage = 3000; /* this will kill everything */
            else
              damage = damroll(4, 8);

            MSG("%s wails out in pain!", descD);
            if (mon_take_hit(c_ptr->midx, damage)) {
              MSG("%s is embedded in the rock.", descD);
              py_experience();
            }
          } else if (cr_ptr->cchar == 'E' || cr_ptr->cchar == 'X') {
            if (m_ptr->mlit) MSG("%s grows larger.", descD);
            /* must be an earth elemental or an earth spirit, or a Xorn
               increase its hit points */
            m_ptr->hp += damroll(4, 8);
          }
        }

        if ((c_ptr->fval >= MIN_WALL) && (c_ptr->fval != BOUNDARY_WALL)) {
          c_ptr->fval = FLOOR_CORR;
          c_ptr->cflag = 0;
        } else if (c_ptr->fval <= MAX_FLOOR) {
          // TBD: wall material
          // tmp = randint(10);
          // if (tmp < 6)
          //  c_ptr->fval = QUARTZ_WALL;
          // else if (tmp < 9)
          //  c_ptr->fval = MAGMA_WALL;
          // else
          c_ptr->fval = GRANITE_WALL;
          c_ptr->cflag = 0;
        }
      }
}
void
destroy_area(y, x)
{
  register int i, j, k;

  if (dun_level > 0) {
    for (i = (y - 15); i <= (y + 15); i++)
      for (j = (x - 15); j <= (x + 15); j++)
        if (in_bounds(i, j) && (caveD[i][j].fval != BOUNDARY_WALL)) {
          k = distance(i, j, y, x);
          if (k == 0) /* clear player's spot, but don't put wall there */
            replace_spot(i, j, 1);
          else if (k < 13)
            replace_spot(i, j, randint(6));
          else if (k < 16)
            replace_spot(i, j, randint(9));
        }
  }
  msg_print("There is a searing blast of light!");
  countD.blind += 10 + randint(10);
}
void
starlite(y, x)
{
  if (countD.blind == 0)
    msg_print("The end of the staff bursts into a blue shimmering light.");
  for (int it = 1; it <= 9; it++)
    if (it != 5) light_line(it, y, x);
}
static void*
ptr_xor(void* a, void* b)
{
  return (void*)((uint64_t)a ^ (uint64_t)b);
}
static int
inven_sort()
{
  int i, j;
  struct objS* obj[INVEN_EQUIP];
  void* swap;

  for (i = 0; i < INVEN_EQUIP; ++i) obj[i] = obj_get(invenD[i]);

  for (i = 0; i < INVEN_EQUIP; ++i) {
    for (j = i + 1; j < INVEN_EQUIP; ++j) {
      if (obj[j]->tval > obj[i]->tval) {
        swap = ptr_xor(obj[j], obj[i]);
        obj[j] = ptr_xor(obj[j], swap);
        obj[i] = ptr_xor(obj[i], swap);
      }
    }
  }
  for (i = 0; i < INVEN_EQUIP; ++i) invenD[i] = obj[i]->id;
  return 1;
}
static int
inven_screen(begin, end)
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
    i_ptr->tohit = -randint(5) - randint(5);
    i_ptr->todam = -randint(5) - randint(5);
    i_ptr->toac = 0;
    i_ptr->sn = 0;

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
inven_choice(char* prompt)
{
  char c;
  int inum;

  inum = inven_count();
  if (inum) {
    do {
      inven_screen(0, INVEN_EQUIP);
      if (in_subcommand(prompt, &c)) {
        uint8_t iidx = c - 'a';
        if (iidx < INVEN_EQUIP) {
          if (invenD[iidx]) return iidx;
        } else if (c == 'I') {
          inven_sort();
        }
      }
    } while (c == 'I');
  } else
    msg_print("You are not carrying anything!");
  return -1;
}
int
inven_eat(iidx)
{
  uint32_t i;
  int j, ident;
  struct objS* obj = obj_get(invenD[iidx]);
  struct treasureS* tr_ptr = &treasureD[obj->tidx];

  if (obj->tval == TV_FOOD) {
    i = obj->flags;
    // Some food has no effect; thus becomes known
    ident = (i == 0);
    while (i != 0) {
      j = bit_pos(&i) + 1;
      /* Foods  				*/
      switch (j) {
        case 1:
          countD.poison += randint(10) + obj->level;
          ident |= TRUE;
          break;
        case 2:
          countD.blind += randint(250) + 10 * obj->level + 100;
          msg_print("A veil of darkness surrounds you.");
          ident |= TRUE;
          break;
        case 3:
          countD.fear += randint(10) + obj->level;
          msg_print("You feel terrified!");
          ident |= TRUE;
          break;
        case 4:
          countD.confusion += randint(10) + obj->level;
          msg_print("You feel drugged.");
          ident |= TRUE;
          break;
        // case 5:
        //  f_ptr->image += randint(200) + 25 * obj->level + 200;
        //  msg_print("You feel drugged.");
        //  ident |= TRUE;
        //  break;
        case 6:
          ident |= countD.poison > 0;
          countD.poison = MIN(countD.poison, 1);
          break;
        case 7:
          ident |= countD.blind > 0;
          countD.blind = MIN(countD.blind, 1);
          break;
        case 8:
          ident |= countD.fear > 0;
          countD.fear = MIN(countD.fear, 1);
          break;
        case 9:
          ident |= countD.confusion > 0;
          countD.confusion = MIN(countD.confusion, 1);
          break;
        case 10:
          ident |= TRUE;
          lose_stat(A_STR);
          break;
        case 11:
          ident |= TRUE;
          lose_stat(A_CON);
          break;
        case 16:
          if (res_stat(A_STR)) {
            msg_print("You feel your strength returning.");
            ident |= TRUE;
          }
          break;
        case 17:
          if (res_stat(A_CON)) {
            msg_print("You feel your health returning.");
            ident |= TRUE;
          }
          break;
        case 18:
          if (res_stat(A_INT)) {
            msg_print("Your head spins a moment.");
            ident |= TRUE;
          }
          break;
        case 19:
          if (res_stat(A_WIS)) {
            msg_print("You feel your wisdom returning.");
            ident |= TRUE;
          }
          break;
        case 20:
          if (res_stat(A_DEX)) {
            msg_print("You feel more dextrous.");
            ident |= TRUE;
          }
          break;
        case 21:
          if (res_stat(A_CHR)) {
            msg_print("Your skin stops itching.");
            ident |= TRUE;
          }
          break;
        case 22:
          ident |= py_heal_hit(randint(6));
          break;
        case 23:
          ident |= py_heal_hit(randint(12));
          break;
        case 24:
          ident |= py_heal_hit(randint(18));
          break;
        case 26:
          ident |= py_heal_hit(damroll(3, 12));
          break;
        case 27:
          strcpy(death_descD, "poisonous food");
          py_take_hit(randint(18));
          ident |= TRUE;
          break;
        default:
          msg_print("Internal error in eat()");
          break;
      }
      /* End of food actions.  			*/
    }
    if (ident) {
      if (!tr_is_known(tr_ptr)) {
        /* round half-way case up */
        uD.exp += (obj->level + (uD.lev >> 1)) / uD.lev;
        py_experience();

        tr_make_known(tr_ptr);
      }
    }
    // else if (!known1_p(obj))
    //   sample(obj);
    py_add_food(obj->p1);
    inven_destroy_one(iidx);
    msg_print("nom nom nom!!");
    turn_flag = TRUE;
    return TRUE;
  } else {
    msg_print("You can't eat that!");
  }
  return FALSE;
}
void
inven_study(iidx)
{
  struct objS* obj;
  struct treasureS* tr_ptr;
  int line;
  int blows, eqidx;

  obj = obj_get(invenD[iidx]);
  tr_ptr = &treasureD[obj->tidx];
  if (obj->id) {
    if (HACK) obj->idflag = ID_REVEAL;
    eqidx = may_equip(obj->tval);

    obj_desc(obj, TRUE);
    MSG("You study %s.", descD);

    line = 0;
    strcpy(descD, tr_ptr->name);
    obj_prefix(obj, FALSE);
    BufMsg(screen, "%-17.017s: %s", "Name", descD);
    if (obj->idflag & ID_REVEAL) {
      if (oset_tohitdam(obj)) {
        BufMsg(screen, "%-17.017s: %+d", "+ To Hit", obj->tohit);
        BufMsg(screen, "%-17.017s: %+d", "+ To Damage", obj->todam);
      } else if (eqidx == INVEN_BODY) {
        BufMsg(screen, "%-17.017s: %+d", "+ To Hit", obj->tohit);
      }
      if (obj->ac) {
        BufMsg(screen, "%-17.017s: %+d", "Base Armor", obj->ac);
      }
      if (obj->toac) {
        BufMsg(screen, "%-17.017s: %+d", "+ To Armor", obj->toac);
      }
      if (obj->toac || obj->ac || eqidx == INVEN_HEAD) {
        BufMsg(screen, "%-17.017s: %+d", "Total Armor", obj->ac + obj->toac);
      }
    } else {
      BufMsg(screen, "... is unidentified!");
    }
    if (eqidx == INVEN_WIELD) {
      BufMsg(screen, "%-17.017s: (%dd%d)", "Damage Dice", obj->damage[0],
             obj->damage[1]);

      BufMsg(screen, "%-17.017s: [%d - %d]", "Damage per Blow",
             (obj->damage[0]), (obj->damage[0] * obj->damage[1]));

      if (obj->idflag & ID_REVEAL) {
        if (obj->todam >= 0) {
          BufLineAppend(screen, line - 1, " + %d", obj->todam);
        } else {
          BufLineAppend(screen, line - 1, " - %d", -obj->todam);
        }
      } else {
        BufLineAppend(screen, line - 1, "?");
      }

      blows = attack_blows(obj->weight);
      BufMsg(screen, "%-17.017s: %d", "Number of Blows", blows);
      if (obj->idflag & ID_REVEAL) {
        BufMsg(screen, "%-17.017s: [%d - %d]", "Total Damage",
               blows * MAX(obj->damage[0] + obj->todam, 1),
               blows * MAX((obj->damage[0] * obj->damage[1] + obj->todam), 1));
      } else {
        BufMsg(screen, "%-17.017s: [%d - %d]?", "Total Damage",
               blows * MAX(obj->damage[0], 1),
               blows * MAX((obj->damage[0] * obj->damage[1]), 1));
      }
    }
    if (obj->idflag & ID_DAMD) {
      BufMsg(screen, "... is known to be cursed!");
    }
    if (obj->idflag & ID_MAGIK) {
      BufMsg(screen, "... is known to be magical!");
    }
    if (obj->idflag & ID_CORRODED) {
      BufMsg(screen, "... is known to be corroded!");
    }
    if (!tr_is_known(tr_ptr)) {
      BufMsg(screen, "... has unknown effects!");
    }
  }
}
int
inven_quaff(iidx)
{
  BOOL ident;
  uint32_t i, j;
  struct objS* obj = obj_get(invenD[iidx]);
  struct treasureS* tr_ptr = &treasureD[obj->tidx];
  if (obj->tval == TV_POTION1 || obj->tval == TV_POTION2) {
    i = obj->flags;
    if (i == 0) msg_print("You feel less thirsty.");
    ident = FALSE;
    while (i != 0) {
      j = bit_pos(&i) + 1;
      j += (obj->tval == TV_POTION2) * 32;
      switch (j) {
        case 1:
          if (inc_stat(A_STR)) {
            msg_print("Wow!  What bulging muscles!");
            ident |= TRUE;
          }
          break;
        case 2:
          ident |= TRUE;
          lose_stat(A_STR);
          break;
        case 3:
          if (res_stat(A_STR)) {
            msg_print("You feel warm all over.");
            ident |= TRUE;
          }
          break;
        case 4:
          if (inc_stat(A_INT)) {
            msg_print("Aren't you brilliant!");
            ident |= TRUE;
          }
          break;
        case 5:
          ident |= TRUE;
          lose_stat(A_INT);
          break;
        case 6:
          if (res_stat(A_INT)) {
            msg_print("You have have a warm feeling.");
            ident |= TRUE;
          }
          break;
        case 7:
          if (inc_stat(A_WIS)) {
            msg_print("You suddenly have a profound thought!");
            ident |= TRUE;
          }
          break;
        case 8:
          ident |= TRUE;
          lose_stat(A_WIS);
          break;
        case 9:
          if (res_stat(A_WIS)) {
            msg_print("You feel your wisdom returning.");
            ident |= TRUE;
          }
          break;
        case 10:
          if (inc_stat(A_CHR)) {
            msg_print("Gee, ain't you cute!");
            ident |= TRUE;
          }
          break;
        case 11:
          ident |= TRUE;
          lose_stat(A_CHR);
          break;
        case 12:
          if (res_stat(A_CHR)) {
            msg_print("You feel your looks returning.");
            ident |= TRUE;
          }
          break;
        case 13:
          ident |= py_heal_hit(damroll(2, 7));
          break;
        case 14:
          ident |= py_heal_hit(damroll(4, 7));
          break;
        case 15:
          ident |= py_heal_hit(damroll(6, 7));
          break;
        case 16:
          ident |= py_heal_hit(1000);
          break;
        case 17:
          if (inc_stat(A_CON)) {
            msg_print("You feel tingly for a moment.");
            ident |= TRUE;
          }
          break;
        case 18:
          if (uD.exp < MAX_EXP) {
            int l = (uD.exp / 2) + 10;
            if (l > 100000L) l = 100000L;
            uD.exp += l;
            msg_print("You feel more experienced.");
            py_experience();
            ident |= TRUE;
          }
          break;
        case 19:
          if (py_tr(TR_FREE_ACT) == 0) {
            msg_print("You fall asleep.");
            countD.paralysis += randint(4) + 4;
            ident |= TRUE;
          }
          break;
        case 20:
          if (countD.blind == 0) {
            msg_print("You are covered by a veil of darkness.");
            ident |= TRUE;
          }
          countD.blind += randint(100) + 100;
          break;
        case 21:
          if (countD.confusion == 0) {
            msg_print("Hey!  This is good stuff!  * Hick! *");
            ident |= TRUE;
          }
          countD.confusion += randint(20) + 12;
          break;
        case 22:
          if (countD.poison == 0) {
            msg_print("You feel very sick.");
            ident |= TRUE;
          }
          countD.poison += randint(15) + 10;
          break;
        case 23:
          if (py_affect(MA_FAST) == 0) ident |= TRUE;
          maD[MA_FAST] += randint(25) + 15;
          break;
        case 24:
          if (py_affect(MA_SLOW) == 0) ident |= TRUE;
          maD[MA_SLOW] += randint(25) + 15;
          break;
        case 26:
          if (inc_stat(A_DEX)) {
            msg_print("You feel more limber!");
            ident |= TRUE;
          }
          break;
        case 27:
          if (res_stat(A_DEX)) {
            msg_print("You feel less clumsy.");
            ident |= TRUE;
          }
          break;
        case 28:
          if (res_stat(A_CON)) {
            msg_print("You feel your health returning!");
            ident |= TRUE;
          }
          break;
        case 29:
          ident |= countD.blind > 0;
          countD.blind = MIN(countD.blind, 1);
          break;
        case 30:
          ident |= countD.confusion > 0;
          countD.confusion = MIN(countD.confusion, 1);
          break;
        case 31:
          ident |= countD.poison > 0;
          countD.poison = MIN(countD.poison, 1);
          break;
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
            ident |= TRUE;
          }
          break;
        case 35:
          countD.poison = 1;
          if (uD.food > 150) uD.food = 150;
          countD.paralysis = 4;
          msg_print("The potion makes you vomit!");
          ident |= TRUE;
          break;
        case 36:
          if (py_affect(MA_INVULN) == 0) ident |= TRUE;
          maD[MA_INVULN] += randint(10) + 10;
          break;
        case 37:
          if (py_affect(MA_HERO) == 0) ident |= TRUE;
          maD[MA_HERO] += randint(25) + 25;
          break;
        case 38:
          if (py_affect(MA_SUPERHERO) == 0) ident |= TRUE;
          maD[MA_SUPERHERO] += randint(25) + 25;
          break;
        case 39:
          ident |= countD.fear > 0;
          countD.fear = MIN(countD.fear, 1);
          break;
        case 40:
          ident |= restore_level();
          break;
        case 41:
          if (py_affect(MA_AFIRE) == 0) ident |= TRUE;
          maD[MA_AFIRE] += randint(10) + 10;
          break;
        case 42:
          if (py_affect(MA_AFROST) == 0) ident |= TRUE;
          maD[MA_AFROST] += randint(10) + 10;
          break;
        case 43:
          if (py_tr(TR_SEE_INVIS) == 0) ident |= TRUE;
          maD[MA_SEE_INVIS] += randint(12) + 12;
          break;
        case 44:
          if (countD.poison > 0) {
            ident |= TRUE;
            msg_print("The effect of the poison has been reduced.");
            countD.poison = MAX(countD.poison / 2, 1);
          }
          break;
        case 45:
          ident |= countD.poison > 0;
          countD.poison = MIN(countD.poison, 1);
          break;
        case 46:
          //   if (m_ptr->cmana < m_ptr->mana) {
          //     m_ptr->cmana = m_ptr->mana;
          //     ident |= TRUE;
          msg_print("Your feel your head clear.");
          //   }
          break;
        case 47:
          if (py_affect(MA_SEE_INFRA) == 0) {
            msg_print("Your eyes begin to tingle.");
            ident |= TRUE;
          }
          maD[MA_SEE_INFRA] += 100 + randint(100);
          break;
        default:
          msg_print("Internal error in potion()");
          break;
      }
    }
    if (!tr_is_known(tr_ptr)) {
      if (ident) {
        // TBD: XP tuning
        tr_make_known(tr_ptr);
      }
      // else sample(...);
    }

    py_add_food(obj->p1);
    inven_destroy_one(iidx);
    turn_flag = TRUE;

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
  int ident, used_up, choice_idx;
  struct objS* i_ptr;
  struct treasureS* tr_ptr;

  i_ptr = obj_get(invenD[iidx]);
  tr_ptr = &treasureD[i_ptr->tidx];
  if (i_ptr->tval == TV_SCROLL1 || i_ptr->tval == TV_SCROLL2) {
    ident = FALSE;
    used_up = TRUE;

    i = i_ptr->flags;
    while (i != 0) {
      j = bit_pos(&i) + 1;
      if (i_ptr->tval == TV_SCROLL2) j += 32;

      /* Scrolls.  		*/
      switch (j) {
        case 1:
          ident |= tohit_enchant(1);
          break;
        case 2:
          ident |= todam_enchant(1);
          break;
        case 3:
          ident |= equip_enchant(1);
          break;
        case 4:
          msg_print("This is an identify scroll.");
          ident |= TRUE;
          choice_idx = inven_choice("Which item do you wish identified?");
          if (choice_idx >= 0) {
            used_up = inven_ident(choice_idx);
          } else {
            used_up = FALSE;
          }
          break;
        case 5:
          if (equip_remove_curse()) {
            msg_print("You feel as if someone is watching over you.");
            ident |= TRUE;
          }
          break;
        case 6:
          ident |= light_area(uD.y, uD.x);
          break;
        case 7:
          for (k = 0; k < randint(3); k++) {
            ident |= (summon_monster(uD.y, uD.x) != 0);
          }
          break;
        case 8:
          py_teleport(10, uy, ux);
          ident |= TRUE;
          break;
        case 9:
          py_teleport(100, uy, ux);
          ident |= TRUE;
          break;
        case 10:
          dun_level += (-3) + 2 * randint(2);
          if (dun_level < 1) dun_level = 1;
          new_level_flag = TRUE;
          ident |= TRUE;
          break;
        case 11:
          if (uD.confuse_monster == 0) {
            msg_print("Your hands begin to glow.");
            uD.confuse_monster = 1;
            ident |= TRUE;
          }
          break;
        case 12:
          ident |= TRUE;
          map_area();
          break;
        case 13:
          ident |= sleep_adjacent(uD.y, uD.x);
          break;
        case 14:
          ident |= TRUE;
          warding_glyph(uD.y, uD.x);
          break;
        case 15:
          ident |= detect_obj(oset_gold);
          break;
        case 16:
          ident |= detect_obj(oset_pickup);
          break;
        case 17:
          ident |= detect_obj(oset_trap);
          break;
        case 18:
          ident |= detect_obj(oset_sdoor);
          break;
        case 19:
          msg_print("This is a mass genocide scroll.");
          mass_genocide(uD.y, uD.x);
          ident |= TRUE;
          break;
        case 20:
          if (detect_mon(crset_visible)) {
            ident |= TRUE;
            maD[MA_DETECT_INVIS] = 1;
            msg_print("You sense the presence of invisible creatures!");
          }
          break;
        case 21:
          if (aggravate_monster(20)) {
            msg_print("There is a high pitched humming noise.");
            ident |= TRUE;
          }
          break;
        case 22:
          ident |= trap_creation(uD.y, uD.x);
          break;
        case 23:
          ident |= td_destroy(uD.y, uD.x);
          break;
        case 24:
          ident |= door_creation();
          break;
        case 25:
          msg_print("This is a Recharge-Item scroll.");
          ident |= TRUE;
          choice_idx = inven_choice("Recharge which item?");
          if (choice_idx >= 0 && invenD[choice_idx]) {
            inven_recharge(choice_idx, 60);
          } else
            used_up = FALSE;
          break;
        case 26:
          ident |= extermination();
          if (ident) {
            msg_print("This is an extermination scroll.");
            ident |= TRUE;
          }
          break;
        case 27:
          ident |= unlight_area(uD.y, uD.x);
          break;
        case 28:
          ident |= (countD.protevil == 0);
          countD.protevil += randint(25) + 3 * uD.lev;
          break;
        case 29:
          ident |= TRUE;
          create_food(uD.y, uD.x);
          break;
        case 30:
          ident |= dispel_creature(CD_UNDEAD, 60);
          break;
        case 33:
          ident |= tohit_enchant(randint(2));
          ident |= todam_enchant(randint(2));
          break;
        case 34:
          ident |= weapon_curse();
          break;
        case 35:
          k = randint(2) + 1;
          ident |= equip_enchant(k);
          break;
        case 36:
          ident |= equip_curse();
          break;
        case 37:
          for (k = 0; k < randint(3); k++) {
            ident |= (summon_undead(uD.y, uD.x) != 0);
          }
          break;
        case 38:
          ident |= TRUE;
          maD[MA_BLESS] += (randint(12) + 6);
          break;
        case 39:
          ident |= TRUE;
          maD[MA_BLESS] += (randint(24) + 12);
          break;
        case 40:
          ident |= TRUE;
          maD[MA_BLESS] += (randint(48) + 24);
          break;
        case 41:
          ident |= TRUE;
          if (py_affect(MA_RECALL) == 0) maD[MA_RECALL] += 25 + randint(30);
          msg_print("The air about you becomes charged.");
          break;
        case 42:
          destroy_area(uD.y, uD.x);
          ident |= TRUE;
          break;
        default:
          msg_print("Internal error in scroll()");
          break;
      }
      /* End of Scrolls.  		       */
    }
    if (!tr_is_known(tr_ptr)) {
      if (ident) {
        /* round half-way case up */
        // TDB: xp tuning
        uD.exp += (i_ptr->level + (uD.lev >> 1)) / uD.lev;
        py_experience();

        tr_make_known(tr_ptr);
      }
      // else
      //   sample(i_ptr);
    }
    if (used_up) {
      i_ptr->number -= 1;
      obj_desc(i_ptr, TRUE);
      i_ptr->number += 1;
      MSG("You have %s.", descD);
      inven_destroy_one(iidx);
    }
    turn_flag = TRUE;
    return TRUE;
  }

  return FALSE;
}
int
inven_try_wand_dir(iidx, dir)
{
  uint32_t flags, j;
  int y, x, ident, chance;
  struct objS* i_ptr;
  struct treasureS* tr_ptr;

  i_ptr = obj_get(invenD[iidx]);
  tr_ptr = &treasureD[i_ptr->tidx];
  if (i_ptr->tval == TV_WAND) {
    y = uD.y;
    x = uD.x;
    ident = FALSE;
    chance = uD.save + think_adj(A_INT) - (int)i_ptr->level +
             (level_adj[uD.clidx][LA_DEVICE] * uD.lev / 3);
    if (countD.confusion) chance = chance / 2;
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
          case 1:
            msg_print("A line of blue shimmering light appears.");
            light_line(dir, uD.y, uD.x);
            ident |= TRUE;
            break;
          case 2:
            fire_bolt(GF_LIGHTNING, dir, y, x, damroll(4, 8), spell_nameD[8]);
            ident |= TRUE;
            break;
          case 3:
            fire_bolt(GF_FROST, dir, y, x, damroll(6, 8), spell_nameD[14]);
            ident |= TRUE;
            break;
          case 4:
            fire_bolt(GF_FIRE, dir, y, x, damroll(9, 8), spell_nameD[22]);
            ident |= TRUE;
            break;
          case 5:
            ident |= wall_to_mud(dir, y, x);
            break;
          case 6:
            ident |= poly_monster(dir, y, x);
            break;
          case 7:
            ident |= hp_monster(dir, y, x, -damroll(4, 6));
            break;
          case 8:
            ident |= speed_monster(dir, y, x, 1);
            break;
          case 9:
            ident |= speed_monster(dir, y, x, -1);
            break;
          case 10:
            ident |= confuse_monster(dir, y, x);
            break;
          case 11:
            ident |= sleep_monster(dir, y, x);
            break;
          case 12:
            ident |= drain_life(dir, y, x);
            break;
          case 13:
            ident |= td_destroy2(dir, y, x);
            break;
          case 14:
            fire_bolt(GF_MAGIC_MISSILE, dir, y, x, damroll(2, 6),
                      spell_nameD[0]);
            ident |= TRUE;
            break;
          case 15:
            ident |= build_wall(dir, y, x);
            break;
          case 16:
            ident |= clone_monster(dir, y, x);
            break;
          case 17:
            ident |= teleport_monster(dir, y, x);
            break;
          case 18:
            ident |= disarm_all(dir, y, x);
            break;
          case 19:
            fire_ball(GF_LIGHTNING, dir, y, x, 32, "Lightning Ball");
            ident |= TRUE;
            break;
          case 20:
            fire_ball(GF_FROST, dir, y, x, 48, "Cold Ball");
            ident |= TRUE;
            break;
          case 21:
            fire_ball(GF_FIRE, dir, y, x, 72, "Fire Ball");
            ident |= TRUE;
            break;
          case 22:
            fire_ball(GF_POISON_GAS, dir, y, x, 12, "Stinking Cloud");
            ident |= TRUE;
            break;
          case 23:
            fire_ball(GF_ACID, dir, y, x, 60, "Acid Ball");
            ident |= TRUE;
            break;
          case 24:
            msg_print("The wand vibrates for a moment.");
            ident |= TRUE;
            flags = 1L << (randint(23) - 1);
            break;
          default:
            msg_print("Internal error in wands()");
            break;
        }
        /* End of Wands.  	    */
      }
      if (ident) {
        if (!tr_is_known(tr_ptr)) {
          /* round half-way case up */
          // TBD: tuning
          uD.exp += (i_ptr->level + (uD.lev >> 1)) / uD.lev;
          py_experience();

          tr_make_known(tr_ptr);
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
    turn_flag = TRUE;
    return TRUE;
  }

  return FALSE;
}
void
py_zap(iidx)
{
  int dir;

  if (get_dir(0, &dir)) {
    if (countD.confusion) {
      msg_print("You are confused.");
      do {
        dir = randint(9);
      } while (dir == 5);
    }
    inven_try_wand_dir(iidx, dir);
  }
}
int
inven_try_staff(iidx, uy, ux)
int *uy, *ux;
{
  uint32_t flags, j;
  int k, chance;
  int ident;
  struct objS* i_ptr;
  struct treasureS* tr_ptr;

  i_ptr = obj_get(invenD[iidx]);
  tr_ptr = &treasureD[i_ptr->tidx];
  if (i_ptr->tval == TV_STAFF) {
    chance = uD.save + think_adj(A_INT) - i_ptr->level - 5 +
             (level_adj[uD.clidx][LA_DEVICE] * uD.lev / 3);
    if (countD.confusion) chance = chance / 2;
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
        switch (j) {
          case 1:
            ident |= light_area(uD.y, uD.x);
            break;
          case 2:
            ident |= detect_obj(oset_sdoor);
            break;
          case 3:
            ident |= detect_obj(oset_trap);
            break;
          case 4:
            ident |= detect_obj(oset_gold);
            break;
          case 5:
            ident |= detect_obj(oset_pickup);
            break;
          case 6:
            py_teleport(100, uy, ux);
            ident |= TRUE;
            break;
          case 7:
            ident |= TRUE;
            earthquake();
            break;
          case 8:
            ident |= FALSE;
            for (k = 0; k < randint(4); k++) {
              ident |= (summon_monster(uD.y, uD.x) != 0);
            }
            break;
          case 10:
            ident |= TRUE;
            destroy_area(uD.y, uD.x);
            break;
          case 11:
            ident |= TRUE;
            starlite(uD.y, uD.x);
            break;
          case 12:
            ident |= speed_monster_aoe(1);
            break;
          case 13:
            ident |= speed_monster_aoe(-1);
            break;
          case 14:
            ident |= sleep_monster_aoe();
            break;
          case 15:
            ident |= py_heal_hit(randint(8));
            break;
          case 16:
            if (detect_mon(crset_invisible)) {
              ident |= TRUE;
              maD[MA_DETECT_INVIS] = 1;
            }
            break;
          case 17:
            if (py_affect(MA_FAST) == 0) ident |= TRUE;
            maD[MA_FAST] += randint(30) + 15;
            break;
          case 18:
            if (py_affect(MA_SLOW) == 0) ident |= TRUE;
            maD[MA_SLOW] += randint(30) + 15;
            break;
          case 19:
            ident |= mass_poly();
            break;
          case 20:
            if (equip_remove_curse()) {
              ident |= see_print("The staff glows blue for a moment..");
            }
            break;
          case 21:
            if (detect_mon(crset_evil)) {
              ident |= TRUE;
              maD[MA_DETECT_EVIL] = 1;
              msg_print("You sense the presence of evil!");
            }
            break;
          case 22:
            ident |=
                countD.poison > 0 || countD.blind > 0 || countD.confusion > 0;
            countD.poison = 1;
            countD.blind = 1;
            countD.confusion = 1;
            break;
          case 23:
            ident |= dispel_creature(CD_EVIL, 60);
            break;
          case 25:
            ident |= unlight_area(uD.y, uD.x);
            break;
          default:
            msg_print("Internal error in staffs()");
            break;
        }
      }
      if (!tr_is_known(tr_ptr)) {
        if (ident) {
          /* round half-way case up */
          uD.exp += (i_ptr->level + (uD.lev >> 1)) / uD.lev;
          py_experience();

          tr_make_known(tr_ptr);
        }
        // else {
        //   sample(i_ptr);
        // }
      }
      if (i_ptr->idflag & ID_REVEAL)
        MSG("You have %d charges remaining.", i_ptr->p1);
    } else {
      msg_print("The staff has no charges left.");
      i_ptr->idflag |= ID_EMPTY;
    }
    turn_flag = TRUE;
    return TRUE;
  }

  return FALSE;
}
static void
py_drop(y, x)
{
  char c;
  int iidx;
  struct objS* obj;

  if (caveD[y][x].oidx == 0) {
    iidx = inven_choice("Drop which item?");
    if (iidx >= 0 && iidx < INVEN_EQUIP) {
      obj = obj_get(invenD[iidx]);
      obj->fy = y;
      obj->fx = x;
      caveD[y][x].oidx = obj_index(obj);
      invenD[iidx] = 0;

      obj_desc(obj, TRUE);
      MSG("You drop %s.", descD);
      turn_flag = TRUE;
    }
  } else {
    MSG("There is already an object on the ground here.");
  }
}
static int
py_inven_slot()
{
  int count = 0;
  for (int it = 0; it < INVEN_EQUIP; ++it) {
    count += (invenD[it] == 0);
  }
  return count;
}
static int
inven_merge(obj_id, locn)
int* locn;
{
  int tval, p1, subval, number;
  struct objS* obj = obj_get(obj_id);

  tval = obj->tval;
  p1 = obj->p1;
  subval = obj->subval;
  number = obj->number;
  if (subval & STACK_ANY) {
    for (int it = 0; it < INVEN_EQUIP; ++it) {
      struct objS* i_ptr = obj_get(invenD[it]);
      if (tval == i_ptr->tval && p1 == i_ptr->p1 && subval == i_ptr->subval &&
          number + i_ptr->number < 256) {
        obj->number += i_ptr->number;
        obj_unuse(i_ptr);
        invenD[it] = obj_id;
        *locn = it;
        return TRUE;
      }
    }
  }
  return FALSE;
}
static int
inven_carry(obj_id)
{
  for (int it = 0; it < INVEN_EQUIP; ++it) {
    if (!invenD[it]) {
      invenD[it] = obj_id;
      return it;
    }
  }
  return -1;
}
void
equip_takeoff(iidx, into_slot)
{
  struct objS* obj;
  obj = obj_get(invenD[iidx]);
  if (obj->flags & TR_CURSED) {
    MSG("Hmm, the item you are %s seems to be cursed.", describe_use(iidx));
  } else if (into_slot >= 0) {
    invenD[iidx] = 0;
    invenD[into_slot] = obj->id;

    py_bonuses(obj, -1);
    obj_desc(obj, TRUE);
    MSG("You take off %s. (%c)", descD, 'a' + into_slot);
  }
}
void
inven_wear(iidx)
{
  int eqidx, eqidx_count;
  struct objS* obj;

  obj = obj_get(invenD[iidx]);
  eqidx = may_equip(obj->tval);

  // Ring is not worn unless a eqidx is open
  if (eqidx == INVEN_RING) eqidx = ring_slot();

  if (eqidx >= INVEN_EQUIP) {
    if (invenD[eqidx])
      equip_takeoff(eqidx, iidx);
    else
      invenD[iidx] = 0;

    if (invenD[eqidx] == 0) {
      invenD[eqidx] = obj->id;

      py_bonuses(obj, 1);
      obj_desc(obj, TRUE);
      MSG("You are wearing %s.", descD);
      if (eqidx == INVEN_BODY && obj->tohit) {
        MSG("Cumbersome armor makes more difficult to hit (%+d).", obj->tohit);
      }
      if (obj->flags & TR_CURSED) {
        msg_print("Oops! It feels deathly cold!");
        obj->cost = -1;
        obj->idflag |= ID_DAMD;
      }
      calc_bonuses();
      turn_flag = TRUE;
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
  int row, orow, col;
  char* iter;

  for (i = 0; i < 256; i++) priority[i] = 0;
  priority['<'] = 5;
  priority['>'] = 5;
  priority['@'] = 10;
  priority['+'] = 3;
  priority['#'] = -5;
  priority['.'] = -10;
  priority['\''] = -3;
  priority[' '] = -15;

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
void
show_version()
{
  int line;

  versionD[AL(versionD) - 1] = 0;

  line = 0;
  BufMsg(screen, "Version: %s", versionD);
  BufMsg(screen, "Git Hash: %s", git_hashD);
  BufMsg(screen, "Programming: %s", "Alan Newton");
  BufMsg(screen, "Art: %s", "Nathan Miller");
  line += 1;
  BufMsg(screen, "License");
  BufMsg(screen, "Source from Umoria (GPLv3)");
  BufMsg(screen, "  Robert Alan Koeneke (1958-2022)");
  BufMsg(screen, "  David J. Grabiner");
  BufMsg(screen, "  James E. Wilson");
  BufMsg(screen, "Source from Cosmopolitan LibC (ISC)");
  BufMsg(screen, "  Justine Tunney");
  BufMsg(screen, "Source from puff.c (Zlib)");
  BufMsg(screen, "  Mark Adler");
  BufMsg(screen, "Source from SDL2 (Zlib)");
  BufMsg(screen, "  Sam Lantinga");
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
    BufMsg(screen, "%-4.04s:%5d", stat_nameD[it], statD.use_stat[it]);
    if (statD.max_stat[it] > statD.cur_stat[it]) {
      BufLineAppend(screen, line - 1, "  %d", statD.max_stat[it]);
    }
  }

  line = MAX_A + 1;
  BufMsg(screen, "%-13.013s: %6d", "+ To Hit", cbD.ptohit - cbD.hide_tohit);
  BufMsg(screen, "%-13.013s: %6d", "+ To Damage", cbD.ptodam - cbD.hide_todam);
  BufMsg(screen, "%-13.013s: %6d", "+ To Armor", cbD.ptoac - cbD.hide_toac);
  BufMsg(screen, "%-13.013s: %6d", "Total Armor", cbD.pac - cbD.hide_toac);

  BufPad(screen, MAX_A * 2, 28);

  line = MAX_A + 1;
  BufMsg(screen, "%-11.011s: %6d", "Level", uD.lev);
  BufMsg(screen, "%-11.011s: %6d", "Experience", uD.exp);
  BufMsg(screen, "%-11.011s: %6d", "Max Exp", uD.max_exp);
  BufMsg(screen, "%-11.011s: %6d", "Exp to Adv", lev_exp(uD.lev));
  BufMsg(screen, "%-11.011s: %6d", "Gold", uD.gold);

  BufPad(screen, MAX_A * 2, 52);

  line = MAX_A + 1;
  BufMsg(screen, "%-15.015s: %6d", "Max Hit Points", uD.mhp);
  BufMsg(screen, "%-15.015s: %6d", "Cur Hit Points", uD.chp);
  BufMsg(screen, "%-15.015s: %6d", "Max Mana", 0);
  BufMsg(screen, "%-15.015s: %6d", "Cur Mana", 0);
  BufMsg(screen, "%-15.015s: %6d", "Speed", -uD.pspeed);

  line = 2 * MAX_A + 1;
  BufMsg(screen, "%-13.013s: %6d", "Fighting", uD.bth);
  BufMsg(screen, "%-13.013s: %6d", "Bows", 0);
  BufMsg(screen, "%-13.013s: %6d", "Saving Throw", uD.save);
  BufPad(screen, MAX_A * 3, 28);

  line = 2 * MAX_A + 1;
  // TBD: xdev calc from wand/staff
  BufMsg(screen, "%-12.012s: %6d", "Stealth", uD.stealth);
  BufMsg(screen, "%-12.012s: %6d", "Disarming", uD.disarm);
  BufMsg(screen, "%-12.012s: %6d", "Magic Device", uD.save);
  BufPad(screen, MAX_A * 3, 55);

  line = 2 * MAX_A + 1;
  BufMsg(screen, "%-12.012s: %6d", "Perception", MAX(40 - uD.fos, 0));
  BufMsg(screen, "%-12.012s: %6d", "Searching", uD.search);
  BufMsg(screen, "%-12.012s: %d feet", "Infra-Vision", uD.infra * 10);
}
void
py_takeoff()
{
  char c;
  int into;

  if (py_inven_slot() == 0) {
    msg_print("You don't have room in your inventory.");
  } else if (equip_count() == 0) {
    msg_print("You aren't wearing anything!");
  } else {
    inven_screen(INVEN_EQUIP, MAX_INVEN);
    if (in_subcommand("Take off which item?", &c)) {
      uint8_t iidx = INVEN_EQUIP + (c - 'a');
      if (iidx < MAX_INVEN) {
        if (invenD[iidx]) {
          into = inven_slot();
          if (into >= 0) {
            equip_takeoff(iidx, into);
            calc_bonuses();
            turn_flag = TRUE;
          }
        }
      }
    }
  }
}
static void
py_help()
{
  int line = 0;

  msg_print("Gameplay Commands");
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
  BufMsg(screen, "x: examine monsters");
  BufMsg(screen, "z: zap wand");
  BufMsg(screen, "<: up stairs");
  BufMsg(screen, ">: down stairs");

  BufPad(screen, AL(screenD), 30);

  line = 1;
  BufMsg(screen, "C: character screen");
  BufMsg(screen, "D: disarm trap");
  BufMsg(screen, "E: eat object");
  BufMsg(screen, "I: inventory sort");
  BufMsg(screen, "M: map dungeon");
  BufMsg(screen, "R: rest until healed");
  BufMsg(screen, "S: study an object");
  BufMsg(screen, "T: take off equipment");
  BufMsg(screen, "W: where about the dungeon");
  BufMsg(screen, "X: examine objects");
  BufMsg(screen, "Z: staff invocation");
  BufMsg(screen, ".: automatic object interaction (experimental)");
  line += 1;
  BufMsg(screen, "CTRL-p: message history");
  BufMsg(screen, "CTRL-x: save and exit (experiemntal)");

  if (HACK) {
    msg_pause();
    msg_print("Hack Commands");

    char c;
    line = 0;
    BufMsg(screen, "CTRL('f'): food");
    BufMsg(screen, "CTRL('h'): heal");
    BufMsg(screen, "CTRL('t'): teleport");
    BufMsg(screen, "CTRL('m'): teleport-to-monster");
    BufMsg(screen, "CTRL('o'): teleport-to-object");
  }
}
static void
py_pickup(y, x, pickup)
{
  struct caveS* c_ptr;
  struct objS* obj;
  int locn, merge;

  c_ptr = &caveD[y][x];
  obj = &entity_objD[c_ptr->oidx];

  if (obj->tval == 0) {
    msg_print("You see nothing here.");
  }
  /* There's GOLD in them thar hills!      */
  else if (obj->tval == TV_GOLD) {
    uD.gold += obj->cost;
    MSG("You have found %d gold pieces worth of %s.", obj->cost,
        gold_nameD[obj->subval]);
    delete_object(y, x);
    turn_flag = TRUE;
  } else if (obj->tval <= TV_MAX_PICK_UP) {
    locn = -1;
    merge = inven_merge(obj->id, &locn);
    if (!merge && pickup) locn = inven_carry(obj->id);

    obj_desc(obj, TRUE);
    if (locn >= 0) {
      obj->fy = 0;
      obj->fx = 0;
      caveD[y][x].oidx = 0;

      MSG("You have %s (%s%c)", descD, merge ? "merge " : "", locn + 'a');
      turn_flag = TRUE;
    } else if (!pickup) {
      MSG("You see %s here.", descD);
    } else {
      MSG("You can't carry %s.", descD);
    }
  }
}
int inven_damage(typ, perc) int (*typ)();
{
  int it, j;

  j = 0;
  for (it = 0; it < INVEN_EQUIP; it++) {
    struct objS* obj = obj_get(invenD[it]);
    if ((*typ)(obj) && (randint(100) < perc)) {
      inven_destroy_one(it);
      j++;
    }
  }
  return (j);
}
int
minus_ac(typ_dam)
uint32_t typ_dam;
{
  register int j;
  int minus;
  struct objS* obj;

  minus = FALSE;
  j = equip_random();
  if (j >= 0) {
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
    } else if (typ_dam == TR_RES_ACID) {
      if ((obj->idflag & ID_CORRODED) == 0) {
        obj->idflag |= ID_CORRODED;
        MSG("Acid leaks through your damaged %s.", descD);
      }
    }
  }
  return (minus);
}
void
poison_gas(dam)
{
  py_take_hit(dam);
  countD.poison += 12 + randint(dam);
}
void
fire_dam(dam)
{
  if (py_tr(TR_RES_FIRE)) dam = dam / 3;
  if (py_affect(MA_AFIRE)) dam = dam / 3;
  py_take_hit(dam);
  if (inven_damage(vuln_fire, 3) > 0)
    msg_print("There is smoke coming from your pack!");
}
void
acid_dam(dam)
{
  int flag;

  flag = 0;
  if (minus_ac(TR_RES_ACID)) flag = 1;
  if (py_tr(TR_RES_ACID)) flag += 2;
  py_take_hit(dam / (flag + 1));
  if (inven_damage(vuln_acid, 3) > 0)
    msg_print("There is an acrid smell coming from your pack!");
}
void
frost_dam(dam)
{
  if (py_tr(TR_RES_COLD)) dam = dam / 3;
  if (py_affect(MA_AFROST)) dam = dam / 3;
  py_take_hit(dam);
  if (inven_damage(vuln_frost, 5) > 0)
    msg_print("Something shatters inside your pack!");
}
void
light_dam(dam)
{
  if (py_tr(TR_RES_LIGHT)) dam = dam / 3;
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
void
py_shield_attack(y, x)
{
  int midx, k, avg_max_hp, base_tohit, adj;
  struct creatureS* cr_ptr;
  struct monS* m_ptr;
  struct objS* shield;

  turn_flag = TRUE;
  if (countD.fear) {
    MSG("You are too afraid to bash anyone!");
  } else if (!invenD[INVEN_ARM]) {
    MSG("You must wear a shield to bash monsters!");
  } else {
    midx = caveD[y][x].midx;
    m_ptr = &entity_monD[midx];
    cr_ptr = &creatureD[m_ptr->cidx];
    shield = obj_get(invenD[INVEN_ARM]);
    m_ptr->msleep = 0;
    mon_desc(midx);
    base_tohit = statD.use_stat[A_STR] + shield->weight / 2 + uD.wt / 10;
    adj = uD.lev * level_adj[uD.clidx][LA_BTH];
    if (!m_ptr->mlit)
      base_tohit = (base_tohit / 2) -
                   (statD.use_stat[A_DEX] * (BTH_PLUS_ADJ - 1)) - (adj / 2);

    if (test_hit(base_tohit, adj, statD.use_stat[A_DEX], cr_ptr->ac)) {
      MSG("You hit %s.", descD);
      k = pdamroll(shield->damage);
      k = critical_blow(shield->weight / 4 + statD.use_stat[A_STR], 0, k);
      k += uD.wt / 60 + 3;

      /* See if we done it in.  			     */
      if (mon_take_hit(midx, k)) {
        MSG("You have slain %s.", descD);
        py_experience();
      } else {
        /* Can not stun Balrog */
        avg_max_hp = (cr_ptr->cdefense & CD_MAX_HP
                          ? cr_ptr->hd[0] * cr_ptr->hd[1]
                          : (cr_ptr->hd[0] * (cr_ptr->hd[1] + 1)) >> 1);
        if ((100 + randint(400) + randint(400)) > (m_ptr->hp + avg_max_hp)) {
          m_ptr->mstunned += randint(3) + 1;
          if (m_ptr->mstunned > 24) m_ptr->mstunned = 24;
          MSG("%s appears stunned!", descD);
        } else
          MSG("%s ignores your bash!", descD);
      }
    } else {
      MSG("You miss %s.", descD);
    }
    if (randint(150) > statD.use_stat[A_DEX]) {
      msg_print("You are off balance.");
      countD.paralysis = 1 + randint(2);
    }
  }
}
void
py_attack(y, x)
{
  register int k, blows;
  int base_tohit, lev_adj, tohit, todam, creature_ac;

  int midx = caveD[y][x].midx;
  struct monS* mon = &entity_monD[midx];
  struct creatureS* cre = &creatureD[mon->cidx];
  struct objS* obj = obj_get(invenD[INVEN_WIELD]);

  turn_flag = TRUE;
  if (countD.fear) {
    msg_print("You are too afraid!");
  } else {
    tohit = cbD.ptohit;
    todam = cbD.ptodam;
    base_tohit = uD.bth;
    lev_adj = uD.lev * level_adj[uD.clidx][LA_BTH];
    // reduce hit if monster not lit
    if (mon->mlit == 0) {
      tohit = 0;
      base_tohit /= 2;
      lev_adj /= 2;
    }

    switch (obj->tval) {
      default:
        blows = attack_blows(obj->weight);
        break;
      case 0:
        blows = 2;
        tohit -= 3;
        break;
      case TV_SLING_AMMO:
      case TV_BOLT:
      case TV_ARROW:
      case TV_SPIKE:
        blows = 1;
        break;
    }

    mon->msleep = 0;
    mon_desc(midx);
    descD[0] = tolower(descD[0]);
    creature_ac = cre->ac;
    /* Loop for number of blows,  trying to hit the critter.	  */
    for (int it = 0; it < blows; ++it) {
      if (test_hit(base_tohit, lev_adj, tohit, creature_ac)) {
        MSG("You hit %s.", descD);
        if (obj->tval) {
          k = pdamroll(obj->damage);
          k = tot_dam(obj, k, mon->cidx);
          k = critical_blow(obj->weight, 0, k);
        } else {
          k = damroll(1, 1);
          k = critical_blow(1, 0, k);
        }
        k += todam;

        if (uD.confuse_monster) {
          uD.confuse_monster = 0;
          msg_print("Your hands stop glowing.");
          if ((cre->cdefense & CD_NO_SLEEP) ||
              randint(MAX_MON_LEVEL) < cre->level) {
            MSG("%s is unaffected by confusion.", descD);
          } else {
            MSG("%s appears confused.", descD);
            mon->mconfused += 2 + randint(16);
          }
        }

        /* See if we done it in.  			 */
        if (mon_take_hit(midx, k)) {
          MSG("You have slain %s.", descD);
          py_experience();
          blows = 0;
        }
      } else {
        MSG("You miss %s.", descD);
      }
    }
  }
}
static void
mon_attack(midx)
{
  int bth, flag;
  struct monS* mon = &entity_monD[midx];
  struct creatureS* cre = &creatureD[mon->cidx];

  // TBD: perf draw/attack
  draw();
  mon_desc(midx);
  int adj = cre->level * CRE_LEV_ADJ;
  for (int it = 0; it < AL(cre->attack_list); ++it) {
    if (death) break;
    if (!cre->attack_list[it]) break;
    struct attackS* attack = &attackD[cre->attack_list[it]];

    int attack_type = attack->attack_type;
    int attack_desc = attack->attack_desc;
    bth = bth_adj(attack_type);
    disturb(1, 0);
    flag = test_hit(bth, adj, 0, cbD.pac);
    if (countD.protevil && (cre->cdefense & CD_EVIL) && uD.lev <= cre->level) {
      MSG("%s%s", descD, attack_string(99));
    } else if (flag) {
      int damage = damroll(attack->attack_dice, attack->attack_sides);
      MSG("%s%s", descD, attack_string(attack_desc));
      switch (attack_type) {
        case 1: /*Normal attack  */
                /* round half-way case down */
          damage -= (cbD.pac * damage) / 200;
          py_take_hit(damage);
          break;
        case 2: /*Lose Strength*/
          py_take_hit(damage);
          if (randint(2) == 1) {
            lose_stat(A_STR);
          }
          break;
        case 3: /*Confusion attack*/
          py_take_hit(damage);
          if (randint(2) == 1) {
            if (countD.confusion == 0) {
              msg_print("You feel confused.");
              countD.confusion += 3 + randint(cre->level);
            } else {
              countD.confusion += 3;
            }
          }
          break;
        case 4: /*Fear attack  */
          py_take_hit(damage);

          if (player_saves()) {
            if (countD.fear == 0)
              msg_print("You resist the urge to be afraid!");
          } else if (countD.fear == 0) {
            msg_print("You are suddenly afraid!");
            countD.fear += 3 + randint(cre->level);
          } else {
            countD.fear += 3;
          }
          break;
        case 5: /*Fire attack  */
          msg_print("You are enveloped in flames!");
          fire_dam(damage);
          break;
        case 6: /*Acid attack  */
          msg_print("You are covered in acid!");
          acid_dam(damage);
          break;
        case 7: /*Frost attack  */
          msg_print("You are covered with frost!");
          frost_dam(damage);
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
          if (countD.blind < 1) {
            countD.blind += 10 + randint(cre->level);
            msg_print("Your eyes begin to sting.");
          } else {
            countD.blind += 5;
          }
          break;
        case 11: /*Paralysis attack*/
          py_take_hit(damage);
          if (py_tr(TR_FREE_ACT))
            msg_print("You are unaffected by paralysis.");
          else if (player_saves())
            msg_print("You resist paralysis!");
          else if (countD.paralysis < 1) {
            countD.paralysis = randint(cre->level) + 3;
            msg_print("You are paralyzed.");
          }
          break;
        case 12: /*Steal Money    */
          if (countD.paralysis < 1 && randint(124) < statD.use_stat[A_DEX])
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
          if (!countD.paralysis && randint(124) < statD.use_stat[A_DEX])
            msg_print("You grab hold of your backpack!");
          else {
            int i = inven_random();
            if (i >= 0) {
              inven_destroy_one(i);
              msg_print("Your backpack feels lighter.");
            }
          }
          if (randint(2) == 1) {
            msg_print("There is a puff of smoke!");
            teleport_away(midx, MAX_SIGHT);
          }
          break;
        case 14: /*Poison   */
          py_take_hit(damage);
          msg_print("You feel very sick.");
          countD.poison += randint(cre->level) + 5;
          break;
        case 15: /*Lose dexterity */
          py_take_hit(damage);
          lose_stat(A_DEX);
          break;
        case 16: /*Lose constitution */
          py_take_hit(damage);
          lose_stat(A_CON);
          break;
        case 17: /*Lose intelligence */
          py_take_hit(damage);
          lose_stat(A_INT);
          break;
        case 18: /*Lose wisdom     */
          py_take_hit(damage);
          lose_stat(A_WIS);
          break;
        case 19: /*Lose experience  */
          msg_print("You feel your life draining away!");
          py_lose_experience(damage + (uD.exp / 100) * MON_DRAIN_EXP);
          break;
        case 20: /*Aggravate monster*/
          aggravate_monster(20);
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
            see_print("Your light dims.");
          }
        } break;
        case 24: /*Eat charges    */
        {
          int iidx = inven_random();
          if (iidx >= 0) {
            struct objS* obj = obj_get(invenD[iidx]);
            if (oset_zap(obj) && obj->p1 > 0) {
              mon->hp += cre->level * obj->p1;
              obj->p1 = 0;
              obj->idflag |= ID_EMPTY;
              msg_print("Energy drains from your pack!");
            }
          }
        } break;
      }

      if (uD.confuse_monster) {
        uD.confuse_monster = 0;
        msg_print("Your hands stop glowing.");
        if ((cre->cdefense & CD_NO_SLEEP) ||
            randint(MAX_MON_LEVEL) < cre->level) {
          MSG("%s is unaffected by confusion.", descD);
        } else {
          MSG("%s appears confused.", descD);
          mon->mconfused += 2 + randint(16);
        }
      }
    } else {
      MSG("%s misses you.", descD);
    }
  }
}
static void
close_object()
{
  int y, x, dir, no_object;
  struct caveS* c_ptr;
  struct objS* obj;

  y = uD.y;
  x = uD.x;
  if (get_dir(0, &dir)) {
    mmove(dir, &y, &x);
    c_ptr = &caveD[y][x];
    obj = &entity_objD[c_ptr->oidx];

    no_object = (obj->id == 0);

    if (obj->tval == TV_OPEN_DOOR) {
      turn_flag = TRUE;
      if (c_ptr->midx == 0) {
        if (obj->p1 == 0) {
          // invcopy(&t_list[c_ptr->tptr], OBJ_CLOSED_DOOR);
          obj->tval = TV_CLOSED_DOOR;
          obj->tchar = '+';
          c_ptr->fval = FLOOR_OBST;
        } else
          msg_print("The door appears to be broken.");
      } else {
        // Costs a turn, otherwise can be abused for detection
        msg_print("Something is in your way!");
      }
    } else {
      msg_print("I do not see anything you can close there.");
    }
  }
}
void
try_disarm_trap(y, x)
{
  int chance;
  struct caveS* c_ptr;
  struct objS* obj;

  c_ptr = &caveD[y][x];
  obj = &entity_objD[c_ptr->oidx];

  if (obj->tval == TV_VIS_TRAP) {
    // TBD: div is used; verify this number is positive. clean-up code.
    chance = uD.disarm + 2 * todis_adj() + think_adj(A_INT) +
             level_adj[uD.clidx][LA_DISARM] * uD.lev / 3;
    if (countD.confusion) chance /= 8;
    if (chance + 100 - obj->level > randint(100)) {
      msg_print("You have disarmed the trap.");
      uD.exp += obj->p1;
      delete_object(y, x);
      py_experience();
    }
    // TBD: balance tuning; harmless failure to disarm is gone
    // else if (chance > 5 && randint(chance) > 5)
    //    msg_print("You failed to disarm the trap.");
    else {
      msg_print("You fail to disarm the trap.");
    }
  }
}
void py_disarm(uy, ux) int *uy, *ux;
{
  int y, x, dir;
  struct caveS* c_ptr;
  struct objS* obj;

  y = *uy;
  x = *ux;
  if (countD.confusion) {
    msg_print("You are too confused to disarm.");
  } else if (get_dir(0, &dir)) {
    mmove(dir, &y, &x);
    c_ptr = &caveD[y][x];
    obj = &entity_objD[c_ptr->oidx];

    if (obj->tval != TV_VIS_TRAP) {
      msg_print("I do not see anything to disarm there.");
    } else if (c_ptr->midx) {
      // Prevent invis-detection via disarm: no free turn
      msg_print("Something is in your way!");
    } else {
      *uy = y;
      *ux = x;
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
    py_shield_attack(y, x);
  } else if (obj->tval == TV_CLOSED_DOOR) {
    turn_flag = TRUE;
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
      if (countD.confusion == 0) movement = 1;
    } else if (randint(150) > statD.use_stat[A_DEX]) {
      msg_print("You are off-balance.");
      countD.paralysis = 1 + randint(2);
    } else
      msg_print("The door holds firm.");
  } else {
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
    if (countD.confusion) {
      msg_print("You are confused.");
      do {
        dir = randint(9);
      } while (dir == 5);
    }
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
  int chance;
  struct caveS* c_ptr;
  struct objS* obj;
  c_ptr = &caveD[y][x];
  obj = &entity_objD[c_ptr->oidx];

  if (obj->tval == TV_CLOSED_DOOR) {
    turn_flag = TRUE;
    // Monster may be invisible and will retaliate
    if (c_ptr->midx) {
      msg_print("Something is in your way!");
    } else {
      /* Known to be locked */
      if ((obj->idflag & ID_REVEAL) && obj->p1 > 0) {
        chance = uD.disarm + 2 * todis_adj() + think_adj(A_INT) +
                 level_adj[uD.clidx][LA_DISARM] * uD.lev / 3;
        if (countD.confusion)
          msg_print("You are too confused to pick the lock.");
        else if ((chance - obj->p1) > randint(100)) {
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
py_search(y, x)
{
  int i, j, chance;
  struct caveS* c_ptr;
  struct objS* obj;

  chance = uD.search;
  // TBD: tuning; used to divide by 10
  if (countD.confusion) chance /= 8;
  // TBD: light also reduced search, like blindness
  if (countD.blind) chance /= 8;
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
          find_flag = FALSE;
        } else if (obj->tval == TV_INVIS_TRAP) {
          msg_print("You have found a trap.");
          obj->tval = TV_VIS_TRAP;
          obj->tchar = '^';
          obj->idflag |= ID_REVEAL;
          find_flag = FALSE;
        }
      }
  turn_flag = TRUE;
}
static void
py_look_mon()
{
  register int y, x, ly, lx, oy, ox;
  int dir;

  if (countD.blind) msg_print("You can't see a damn thing!");
  // else if (py.flags.image > 0)
  //   msg_print("You can't believe what you are seeing! It's like a dream!");
  else if (get_dir("Look which direction?", &dir)) {
    y = uD.y;
    x = uD.x;
    ly = dir_y(dir);
    lx = dir_x(dir);
    int seen = 0;
    FOR_EACH(mon, {
      if (mon->mlit && distance(y, x, mon->fy, mon->fx) <= MAX_SIGHT) {
        oy = (ly != 0) * (-((mon->fy - y) < 0) + ((mon->fy - y) > 0));
        ox = (lx != 0) * (-((mon->fx - x) < 0) + ((mon->fx - x) > 0));
        if ((oy == ly) && (ox == lx) && los(y, x, mon->fy, mon->fx)) {
          seen += 1;
          mon_desc(it_index);
          // hack: mon death_descD pronoun is a/an
          MSG("You see %s.", death_descD);
          msg_pause();
        }
      }
    });
    if (seen > 0)
      msg_print("That's all you see in that direction");
    else
      msg_print("You see no monsters of interest in that direction.");
  }
}
static void
py_look_obj()
{
  register int y, x, ly, lx, oy, ox;
  int dir;

  if (countD.blind) msg_print("You can't see a damn thing!");
  // else if (py.flags.image > 0)
  //   msg_print("You can't believe what you are seeing! It's like a dream!");
  else if (get_dir("Look which direction?", &dir)) {
    y = uD.y;
    x = uD.x;
    ly = dir_y(dir);
    lx = dir_x(dir);
    int seen = 0;
    FOR_EACH(obj, {
      if (obj->tval > TV_MAX_PICK_UP && obj->tval != TV_VIS_TRAP) continue;
      if (obj->fy && distance(y, x, obj->fy, obj->fx) <= MAX_SIGHT) {
        oy = (ly != 0) * (-((obj->fy - y) < 0) + ((obj->fy - y) > 0));
        ox = (lx != 0) * (-((obj->fx - x) < 0) + ((obj->fx - x) > 0));
        if (oy == ly && ox == lx && los(y, x, obj->fy, obj->fx)) {
          seen += 1;
          obj_desc(obj, TRUE);
          MSG("You see %s.", descD);
          msg_pause();
        }
      }
    });
    if (seen > 0)
      msg_print("That's all you see in that direction");
    else
      msg_print("You see no objects of interest in that direction.");
  }
}
static void make_move(midx, mm) int* mm;
{
  int i, fy, fx, newy, newx, do_turn, do_move, stuck_door;
  register struct caveS* c_ptr;
  register struct monS* m_ptr;
  struct creatureS* cr_ptr;
  struct objS* obj;

  i = 0;
  do_turn = FALSE;
  do_move = FALSE;
  m_ptr = &entity_monD[midx];
  cr_ptr = &creatureD[m_ptr->cidx];
  for (int i = 0; i < 5; ++i) {
    /* Get new position  	*/
    fy = newy = m_ptr->fy;
    fx = newx = m_ptr->fx;
    mmove(mm[i], &newy, &newx);
    c_ptr = &caveD[newy][newx];
    obj = &entity_objD[c_ptr->oidx];

    if (c_ptr->fval == BOUNDARY_WALL)
      continue;
    else if (cr_ptr->cmove & CM_PHASE)
      do_move = TRUE;
    else if (obj->tval == TV_CLOSED_DOOR || obj->tval == TV_SECRET_DOOR) {
      do_turn = TRUE;
      do_move = FALSE;
      if (obj->p1 == 0) {
        obj->tval = TV_OPEN_DOOR;
        obj->tchar = '\'';
        if (c_ptr->cflag & CF_LIT) msg_print("A door creaks open.");
      } else if (obj->p1 > 0) {
        if (randint((m_ptr->hp + 1) * (50 + obj->p1)) <
            40 * (m_ptr->hp - 10 - obj->p1)) {
          msg_print("You hear the click of a lock being opened.");
          obj->p1 = 0;
        }
      } else if (obj->p1 < 0) {
        if (randint((m_ptr->hp + 1) * (50 - obj->p1)) <
            40 * (m_ptr->hp - 10 + obj->p1)) {
          obj->tval = TV_OPEN_DOOR;
          obj->tchar = '\'';
          // 50% chance to break the door
          obj->p1 = 1 - randint(2);
          msg_print("You hear a door burst open!");
          disturb(1, 0);
          do_move = TRUE;
        }
      }
      if (obj->tval == TV_OPEN_DOOR) c_ptr->fval = FLOOR_CORR;
    } else if (c_ptr->fval <= MAX_OPEN_SPACE)
      do_move = TRUE;

    if (do_move && obj->tval == TV_GLYPH) {
      if (randint(obj->p1) < cr_ptr->level) {
        msg_print("The glyph of protection is broken!");
        delete_object(newy, newx);
      } else {
        do_move = FALSE;
        do_turn = (cr_ptr->cmove & CM_ATTACK_ONLY);
      }
    }
    if (do_move) {
      /* Creature has attempted to move on player?     */
      if (newy == uD.y && newx == uD.x) {
        mon_attack(midx);
        do_move = FALSE;
        do_turn = TRUE;
      }
      /* Creature is attempting to move on other creature?     */
      else if (c_ptr->midx && c_ptr->midx != midx) {
        // TBD: Creatures can eat creatures
        do_move = FALSE;
      }
    }
    /* Creature has been allowed move.   */
    if (do_move) {
      if (cr_ptr->cmove & CM_PICKS_UP && obj_mon_pickup(obj)) {
        delete_object(newy, newx);
      }
      /* Move creature record  	       */
      move_rec(fy, fx, newy, newx);
      m_ptr->fy = newy;
      m_ptr->fx = newx;
      update_mon(midx);
      do_turn = TRUE;
    }
    if (do_turn) break;
  }
}
void
breath(typ, y, x, dam_hp, midx)
{
  int i, j;
  int dam, max_dis, harm_type;
  uint32_t weapon_type, tmp, treas;
  int (*destroy)();
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;

  max_dis = 2;
  get_flags(typ, &weapon_type, &harm_type, &destroy);
  for (i = y - 2; i <= y + 2; i++)
    for (j = x - 2; j <= x + 2; j++)
      if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis) &&
          los(y, x, i, j)) {
        c_ptr = &caveD[i][j];
        if ((c_ptr->oidx != 0) && (*destroy)(&entity_objD[c_ptr->oidx]))
          delete_object(i, j);
        if (c_ptr->fval <= MAX_OPEN_SPACE) {
          /* must test status bit, not py.flags.blind here, flag could have
             been set by a previous monster, but the breath should still
             be visible until the blindness takes effect */
          // if (panel_contains(i, j) && !(py.flags.status & PY_BLIND))
          //   print('*', i, j);
          if (c_ptr->midx) {
            m_ptr = &entity_monD[c_ptr->midx];
            cr_ptr = &creatureD[m_ptr->cidx];
            dam = dam_hp;
            if (harm_type & cr_ptr->cdefense)
              dam = dam * 2;
            else if (weapon_type & cr_ptr->spells)
              dam = (dam / 4);
            dam = (dam / (distance(i, j, y, x) + 1));
            /* can not call mon_take_hit here, since player does not
               get experience for kill */
            m_ptr->hp = m_ptr->hp - dam;
            m_ptr->msleep = 0;
            if (m_ptr->hp < 0) {
              // TBD: treasure drop
              // treas = monster_death(m_ptr->fy, m_ptr->fx, cr_ptr->cmove);
              // if (m_ptr->mlit) {
              //   tmp = (c_recall[m_ptr->mptr].r_cmove & CM_TREASURE) >>
              //         CM_TR_SHIFT;
              //   if (tmp > ((treas & CM_TREASURE) >> CM_TR_SHIFT))
              //     treas = (treas & ~CM_TREASURE) | (tmp << CM_TR_SHIFT);
              //   c_recall[m_ptr->mptr].r_cmove =
              //       treas | (c_recall[m_ptr->mptr].r_cmove & ~CM_TREASURE);
              // }

              mon_unuse(m_ptr);
              c_ptr->midx = 0;
            }
          }
        }
      }

  /* let's do at least one point of damage to the player */
  /* prevents randint(0) problem with poison_gas, also */
  dam = MAX(dam_hp / (distance(i, j, y, x) + 1), 1);
  switch (typ) {
    case GF_LIGHTNING:
      light_dam(dam);
      break;
    case GF_POISON_GAS:
      poison_gas(dam);
      break;
    case GF_ACID:
      acid_dam(dam);
      break;
    case GF_FROST:
      frost_dam(dam);
      break;
    case GF_FIRE:
      fire_dam(dam);
      break;
  }
  /* show the ball of gas */
  // put_qio();
}
static void mon_try_multiply(mon) struct monS* mon;
{
  int i, j, k;

  k = 0;
  for (i = mon->fy - 1; i <= mon->fy + 1; i++)
    for (j = mon->fx - 1; j <= mon->fx + 1; j++)
      if (caveD[i][j].midx) k++;

  if ((k < 4) && (randint((k + 1) * MON_MULT_ADJ) == 1)) mon_multiply(mon);
}
static int
mon_try_spell(midx, cdis)
{
  uint32_t i;
  int k, chance, thrown_spell, r1;
  int spell_choice[32];
  int took_turn;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;

  m_ptr = &entity_monD[midx];
  cr_ptr = &creatureD[m_ptr->cidx];

  chance = cr_ptr->spells & CS_FREQ;

  /* confused monsters don't cast; including turn undead */
  if (m_ptr->mconfused) took_turn = FALSE;
  /* 1 in x chance of casting spell  	   */
  else if (randint(chance) != 1)
    took_turn = FALSE;
  /* Must be within certain range  	   */
  else if (cdis > MAX_SIGHT)
    took_turn = FALSE;
  // Must have unobstructed Line-Of-Sight
  else if (!los(uD.y, uD.x, m_ptr->fy, m_ptr->fx))
    took_turn = FALSE;
  else /* Creature is going to cast a spell   */
  {
    took_turn = TRUE;
    mon_desc(midx);

    /* Extract all possible spells into spell_choice */
    i = (cr_ptr->spells & ~CS_FREQ);
    k = 0;
    while (i != 0) {
      spell_choice[k] = bit_pos(&i);
      k++;
    }
    /* Choose a spell to cast  		       */
    thrown_spell = spell_choice[randint(k) - 1];
    thrown_spell++;
    /* all except teleport_away() and drain mana spells always disturb */
    if (thrown_spell > 6 && thrown_spell != 17) disturb(1, 0);
    /* save some code/data space here, with a small time penalty */
    if ((thrown_spell < 14 && thrown_spell > 6) || (thrown_spell == 16)) {
      MSG("%s casts a spell.", descD);
    }
    /* Cast the spell.  		     */
    switch (thrown_spell) {
      case 5: /*Teleport Short*/
        teleport_away(midx, 5);
        break;
      case 6: /*Teleport Long */
        teleport_away(midx, MAX_SIGHT);
        break;
      // case 7: /*Teleport To (aka. Summon)  */
      //   teleport_to(m_ptr->fy, m_ptr->fx);
      //   break;
      case 8: /*Light Wound   */
        if (player_saves())
          msg_print("You resist the effects of the spell.");
        else
          py_take_hit(damroll(3, 8));
        break;
      case 9: /*Serious Wound */
        if (player_saves())
          msg_print("You resist the effects of the spell.");
        else
          py_take_hit(damroll(8, 8));
        break;
      case 10: /*Hold Person    */
        if (py_tr(TR_FREE_ACT))
          msg_print("You are unaffected by paralysis.");
        else if (player_saves())
          msg_print("You resist the effects of the spell.");
        else if (countD.paralysis > 0)
          countD.paralysis += 2;
        else
          countD.paralysis = randint(5) + 4;
        break;
      case 11: /*Cause Blindness*/
        if (player_saves())
          msg_print("You resist the effects of the spell.");
        else if (countD.blind)
          countD.blind += 6;
        else
          countD.blind += 12 + randint(3);
        break;
      case 12: /*Cause Confuse */
        if (player_saves())
          msg_print("You resist the effects of the spell.");
        else if (countD.confusion)
          countD.confusion += 2;
        else
          countD.confusion = randint(5) + 3;
        break;
      case 13: /*Cause Fear    */
        if (player_saves())
          msg_print("You resist the effects of the spell.");
        else if (countD.fear > 0)
          countD.fear += 2;
        else
          countD.fear = randint(5) + 3;
        break;
      case 14: /*Summon Monster*/
      {
        MSG("%s magically summons a monster!", descD);
        int midx = summon_monster(uD.y, uD.x);
        update_mon(midx);
      } break;
      case 15: /*Summon Undead*/
      {
        MSG("%s magically summons an undead!", descD);
        int midx = summon_undead(uD.y, uD.x);
        update_mon(midx);
      } break;
      case 16: /*Slow Person   */
        if (py_tr(TR_FREE_ACT))
          msg_print("You are unaffected by the spell.");
        else if (player_saves())
          msg_print("You resist the effects of the spell.");
        else if (py_affect(MA_SLOW))
          maD[MA_SLOW] += 2;
        else
          maD[MA_SLOW] = randint(5) + 3;
        break;
      case 17: /*Drain Mana   */
        //   if (uD.cmana > 0) {
        //     disturb(1, 0);
        //     MSG("%sdraws psychic energy from you!", descD);
        //     if (m_ptr->mlit) {
        //       MSG("%sappears healthier.", descD);
        //     }
        //     r1 = (randint(cr_ptr->level) >> 1) + 1;
        //     if (r1 > uD.cmana) {
        //       r1 = uD.cmana;
        //       uD.cmana = 0;
        //       uD.cmana_frac = 0;
        //     } else
        //       uD.cmana -= r1;
        //     m_ptr->hp += 6 * (r1);
        //   }
        break;
      case 20: /*Breath Light */
        MSG("%s breathes lightning.", descD);
        breath(GF_LIGHTNING, uD.y, uD.x, (m_ptr->hp / 4), midx);
        break;
      case 21: /*Breath Gas   */
        MSG("%s breathes gas.", descD);
        breath(GF_POISON_GAS, uD.y, uD.x, (m_ptr->hp / 3), midx);
        break;
      case 22: /*Breath Acid   */
        MSG("%s breathes acid.", descD);
        breath(GF_ACID, uD.y, uD.x, (m_ptr->hp / 3), midx);
        break;
      case 23: /*Breath Frost */
        MSG("%s breathes frost.", descD);
        breath(GF_FROST, uD.y, uD.x, (m_ptr->hp / 3), midx);
        break;
      case 24: /*Breath Fire   */
        MSG("%s breathes fire.", descD);
        breath(GF_FIRE, uD.y, uD.x, (m_ptr->hp / 3), midx);
        break;
      default:
        MSG("%s cast unknown spell.", descD);
    }
    /* End of spells  			       */
    // if (m_ptr->mlit) {
    //   c_recall[m_ptr->mptr].r_spells |= 1L << (thrown_spell - 1);
    //   if ((c_recall[m_ptr->mptr].r_spells & CS_FREQ) != CS_FREQ)
    //     c_recall[m_ptr->mptr].r_spells++;
    //   if (death && c_recall[m_ptr->mptr].r_deaths < MAX_SHORT)
    //     c_recall[m_ptr->mptr].r_deaths++;
    // }
  }
  return took_turn;
}
static void
mon_move(midx, cdis)
{
  struct monS* m_ptr;
  struct creatureS* cr_ptr;
  int mm[9];
  int took_turn, random, flee;

  m_ptr = &entity_monD[midx];
  cr_ptr = &creatureD[m_ptr->cidx];
  AC(mm);
  took_turn = FALSE;
  random = FALSE;
  flee = FALSE;

  if (cr_ptr->cmove & CM_MULTIPLY) mon_try_multiply(m_ptr);
  if (cr_ptr->spells & CS_FREQ) took_turn = mon_try_spell(midx, cdis);

  if (!took_turn) {
    if (m_ptr->mconfused) {
      m_ptr->mconfused -= 1;
      random = TRUE;
      // Undead are confused by turn undead and should flee below
      flee = (cr_ptr->cdefense & CD_UNDEAD);
    } else if ((cr_ptr->cmove & CM_75_RANDOM) && randint(100) < 75) {
      random = TRUE;
    } else if ((cr_ptr->cmove & CM_40_RANDOM) && randint(100) < 40) {
      random = TRUE;
    } else if ((cr_ptr->cmove & CM_20_RANDOM) && randint(100) < 20) {
      random = TRUE;
    } else if ((cr_ptr->cmove & CM_MOVE_NORMAL) && randint(200) == 1) {
      random = TRUE;
    }

    if (flee) {
      get_moves(midx, mm);
      mm[0] = 10 - mm[0];
      mm[1] = 10 - mm[1];
      mm[2] = 10 - mm[2];
      mm[3] = randint(9); /* May attack only if cornered */
      mm[4] = randint(9);
    } else if (random) {
      for (int it = 0; it < 5; ++it) {
        mm[it] = randint(9);
      }
    } else if (cdis < 2 || (cr_ptr->cmove & CM_ATTACK_ONLY) == 0) {
      get_moves(midx, mm);
    }

    if (mm[0]) {
      make_move(midx, mm);
    }
  }
}
static int
movement_rate(speed)
{
  if (speed <= 0) {
    return ((turnD % (2 - speed)) == 0);
  }

  return speed;
}
void
creatures()
{
  int move_count, y, x, cdis;

  y = uD.y;
  x = uD.x;
  FOR_EACH(mon, {
    update_mon(it_index);

    struct creatureS* cr_ptr = &creatureD[mon->cidx];
    move_count = movement_rate(mon_speed(mon));
    for (; move_count > 0; --move_count) {
      cdis = distance(y, x, mon->fy, mon->fx);
      if (mon->mlit || cdis <= cr_ptr->aaf) {
        if (mon->msleep) {
          if (py_tr(TR_AGGRAVATE))
            mon->msleep = 0;
          else {
            uint32_t notice = randint(1024);
            if (notice * notice * notice <= (1 << (29 - uD.stealth))) {
              mon->msleep = MAX(mon->msleep - (100 / cdis), 0);
            }
          }
        }
        if (mon->mstunned != 0) {
          /* NOTE: Balrog = 100*100 = 10000, it always
             recovers instantly */
          if (randint(5000) < cr_ptr->level * cr_ptr->level)
            mon->mstunned = 0;
          else
            mon->mstunned--;
          if (mon->mstunned == 0) {
            if (mon->mlit) {
              MSG("The %s recovers and glares at you.", cr_ptr->name);
            }
          }
        }
        if (mon->msleep == 0 && mon->mstunned == 0) mon_move(it_index, cdis);
      }
    }
  });
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
static void hit_trap(uy, ux) int *uy, *ux;
{
  int y, x, num, dam;
  struct caveS* c_ptr;
  struct objS* obj;

  y = *uy;
  x = *ux;
  c_ptr = &caveD[y][x];
  obj = &entity_objD[c_ptr->oidx];

  obj->tval = TV_VIS_TRAP;
  obj->tchar = '^';
  obj->idflag |= ID_REVEAL;
  find_flag = FALSE;

  dam = pdamroll(obj->damage);

  obj_desc(obj, TRUE);
  switch (obj->subval) {
    case 1: /* Open pit*/
      msg_print("You fell into a pit!");
      if (py_tr(TR_FFALL))
        msg_print("You gently float down.");
      else {
        py_take_hit(dam);
      }
      break;
    case 2: /* Arrow trap*/
      if (test_hit(125, 0, 0, cbD.pac)) {
        py_take_hit(dam);
        msg_print("An arrow hits you.");
      } else
        msg_print("An arrow barely misses you.");
      break;
    case 3: /* Covered pit*/
      msg_print("You fell into a covered pit.");
      if (py_tr(TR_FFALL))
        msg_print("You gently float down.");
      else {
        py_take_hit(dam);
      }
      place_trap(y, x, 0);
      break;
    case 4: /* Trap door*/
      msg_print("You fell through a trap door!");
      new_level_flag = TRUE;
      dun_level++;
      if (py_tr(TR_FFALL))
        msg_print("You gently float down.");
      else {
        py_take_hit(dam);
      }
      /* Force the messages to display before starting to generate the
         next level.  */
      msg_pause();
      break;
    case 5: /* Sleep gas*/
      if (countD.paralysis == 0) {
        msg_print("A strange white mist surrounds you!");
        if (py_tr(TR_FREE_ACT))
          msg_print("You are unaffected.");
        else {
          msg_print("You fall asleep.");
          countD.paralysis += randint(10) + 4;
        }
      }
      break;
    case 6: /* Hid Obj*/
      delete_object(y, x);
      place_object(y, x, FALSE);
      msg_print("Hmmm, there was something under this rock.");
      break;
    case 7: /* STR Dart*/
      if (test_hit(125, 0, 0, cbD.pac)) {
        if (py_tr(sustain_stat(TR_STR)))
          msg_print("A small dart hits you.");
        else {
          dec_stat(A_STR);
          py_take_hit(dam);
          msg_print("A small dart weakens you!");
        }
      } else
        msg_print("A small dart barely misses you.");
      break;
    case 8: /* Teleport*/
      msg_print("You hit a teleport trap!");
      // Display prior to changing movement
      msg_pause();
      py_teleport(40, uy, ux);
      break;
    case 9: /* Rockfall*/
      strcpy(death_descD, "a falling rock");
      py_take_hit(dam);
      delete_object(y, x);
      // TBD: rubble
      // place_rubble(y, x);
      msg_print("You are hit by falling rock.");
      break;
    case 10: /* Corrode gas*/
      /* Makes more sense to print the message first, then damage an
         object.  */
      msg_print("A strange red gas surrounds you.");
      corrode_gas();
      break;
    case 11: /* Summon mon*/
      msg_print("A strange rune on the floor glows and fades.");
      delete_object(y, x);
      num = 2 + randint(3);
      for (int it = 0; it < num; it++) {
        summon_monster(y, x);
      }
      break;
    case 12: /* Fire trap*/
      msg_print("You are enveloped in flames!");
      fire_dam(dam);
      break;
    case 13: /* Acid trap*/
      msg_print("You are splashed with acid!");
      acid_dam(dam);
      break;
    case 14: /* Poison gas*/
      msg_print("A pungent green gas surrounds you!");
      poison_gas(dam);
      break;
    case 15: /* Blind Gas */
      msg_print("A black gas surrounds you!");
      countD.blind += randint(50) + 50;
      break;
    case 16: /* Confuse Gas*/
      msg_print("A gas of scintillating colors surrounds you!");
      countD.confusion += randint(15) + 15;
      break;
    case 17: /* Slow Dart*/
      if (test_hit(125, 0, 0, cbD.pac)) {
        py_take_hit(dam);
        msg_print("A small dart hits you!");
        if (py_tr(TR_FREE_ACT))
          msg_print("You are unaffected.");
        else
          maD[MA_SLOW] += randint(20) + 10;
      } else
        msg_print("A small dart barely misses you.");
      break;
    case 18: /* CON Dart*/
      if (test_hit(125, 0, 0, cbD.pac)) {
        if (py_tr(sustain_stat(TR_CON)))
          msg_print("A small dart hits you.");
        else {
          dec_stat(A_CON);
          py_take_hit(dam);
          msg_print("A small dart saps your health!");
        }
      } else
        msg_print("A small dart barely misses you.");
      break;

    default:
      msg_print("Unknown trap value.");
      break;
  }
}
static int
obj_value(obj)
struct objS* obj;
{
  int value;
  int known;
  struct treasureS* tr_ptr;

  tr_ptr = &treasureD[obj->tidx];
  value = obj->cost;
  known = ((obj->idflag & ID_REVEAL) != 0) || tr_is_known(tr_ptr);
  /* don't purchase known cursed items */
  if (obj->idflag & (ID_DAMD | ID_CORRODED))
    value = 0;
  else if (((obj->tval >= TV_BOW) && (obj->tval <= TV_SWORD)) ||
           ((obj->tval >= TV_BOOTS) &&
            (obj->tval <= TV_SOFT_ARMOR))) { /* Weapons and armor  */
    if ((obj->idflag & ID_REVEAL) == 0)
      value = tr_ptr->cost;
    else if ((obj->tval >= TV_BOW) && (obj->tval <= TV_SWORD)) {
      if (obj->tohit < 0)
        value = 0;
      else if (obj->todam < 0)
        value = 0;
      else if (obj->toac < 0)
        value = 0;
      else
        value = obj->cost + (obj->tohit + obj->todam + obj->toac) * 100;
    } else {
      if (obj->toac < 0)
        value = 0;
      else
        value = obj->cost + obj->toac * 100;
    }
  } else if ((obj->tval >= TV_SLING_AMMO) &&
             (obj->tval <= TV_SPIKE)) { /* Ammo  		*/
    if ((obj->idflag & ID_REVEAL) == 0)
      value = tr_ptr->cost;
    else {
      if (obj->tohit < 0)
        value = 0;
      else if (obj->todam < 0)
        value = 0;
      else if (obj->toac < 0)
        value = 0;
      else
        /* use 5, because missiles generally appear in groups of 20,
           so 20 * 5 == 100, which is comparable to weapon bonus above */
        value = obj->cost + (obj->tohit + obj->todam + obj->toac) * 5;
    }
  } else if ((obj->tval == TV_SCROLL1) || (obj->tval == TV_SCROLL2) ||
             (obj->tval == TV_POTION1) ||
             (obj->tval == TV_POTION2)) { /* Potions, Scrolls, and Food  */
    if (!known) value = 20;
  } else if (obj->tval == TV_FOOD) {
    if (!known) value = 1;
  } else if ((obj->tval == TV_AMULET) ||
             (obj->tval == TV_RING)) { /* Rings and amulets  */
    if (!known) /* player does not know what type of ring/amulet this is */
      value = 45;
    else if ((obj->idflag & ID_REVEAL) == 0)
      /* player knows what type of ring, but does not know whether it is
         cursed or not, if refuse to buy cursed objects here, then
         player can use this to 'identify' cursed objects */
      value = tr_ptr->cost;
  } else if ((obj->tval == TV_STAFF) ||
             (obj->tval == TV_WAND)) { /* Wands and staffs*/
    if (!known) {
      if (obj->tval == TV_WAND)
        value = 50;
      else
        value = 70;
    } else if (obj->idflag & ID_REVEAL)
      value = obj->cost + (obj->cost / 20) * obj->p1;
  }
  /* picks and shovels */
  else if (obj->tval == TV_DIGGING) {
    if ((obj->idflag & ID_REVEAL) == 0)
      value = tr_ptr->cost;
    else {
      if (obj->p1 < 0)
        value = 0;
      else {
        /* some digging tools start with non-zero p1 values, so only
           multiply the plusses by 100, make sure result is positive */
        value = obj->cost + (obj->p1 - tr_ptr->p1) * 100;
        if (value < 0) value = 0;
      }
    }
  }
  /* multiply value by number of items if it is a batch stack item */
  if (obj->subval & STACK_BATCH) value = value * obj->number;
  return (value);
}
// Object Value * Chrisma Adjustment * Racial Adjustment * Inflation
// factor 1 or -1
static int
store_value(sidx, obj_value, factor)
{
  int cadj, radj, iadj;
  struct ownerS* owner;

  owner = &ownerD[storeD[sidx]];

  cadj = chr_adj();
  radj = rgold_adjD[owner->owner_race][uD.ridx];
  iadj = owner->min_inflate;

  if (factor < 0) {
    cadj = (200 - chr_adj());
    radj = (200 - radj);
    iadj = MAX(200 - iadj, 1);
  }

  // Use a 64-bit range when scaling; narrow to int on return
  return MAX((int64_t)obj_value * cadj * radj * iadj / 1e6, 0LL);
}
static int
obj_store_index(obj)
struct objS* obj;
{
  switch (obj->tval) {
      // int general_store(element) int element;
    case TV_DIGGING:
    case TV_CLOAK:
    case TV_FOOD:
    case TV_FLASK:
    case TV_LIGHT:
    case TV_SPIKE:
      return 0;
      // int armory(element) int element;
    case TV_BOOTS:
    case TV_GLOVES:
    case TV_HELM:
    case TV_SHIELD:
    case TV_HARD_ARMOR:
    case TV_SOFT_ARMOR:
      return 1;
      // int weaponsmith(element) int element;
    case TV_SLING_AMMO:
    case TV_BOLT:
    case TV_ARROW:
    case TV_BOW:
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
      return 2;
      // int temple(element) int element;
    case TV_PRAYER_BOOK:
      return 3;
      // int alchemist(element) int element;
    case TV_SCROLL1:
    case TV_SCROLL2:
    case TV_POTION1:
    case TV_POTION2:
      return 4;
      // int magic_shop(element) int element;
    case TV_AMULET:
    case TV_RING:
    case TV_STAFF:
    case TV_WAND:
    case TV_MAGIC_BOOK:
      return 5;
  }
  return -1;
}
static void
inven_pawn(iidx)
{
  struct objS* obj;
  struct treasureS* tr_ptr;
  int sidx, cost, number;

  obj = obj_get(invenD[iidx]);
  tr_ptr = &treasureD[obj->tidx];
  sidx = obj_store_index(obj);
  if (sidx >= 0) {
    cost = store_value(sidx, obj_value(obj), -1);
    tr_make_known(tr_ptr);
    obj->idflag = ID_REVEAL;
    // TBD: ouch
    number = obj->number;
    obj->number = 1;
    obj_desc(obj, TRUE);
    obj->number = number;
    uD.gold += cost;
    // TBD: copy obj to a store inventory?
    inven_destroy_one(iidx);
    if (cost == 0) {
      MSG("You donate %s.", descD);
    } else {
      MSG("You sold %s for %d gold piece%s.", descD, cost, cost > 1 ? "s" : "");
    }
    msg_pause();
  }
}
static void
pawn_display()
{
  int line, flag;
  int cost, sidx;
  struct objS* obj;

  line = 0;
  BufMsg(screen, "   Item");
  for (int it = 0; it < INVEN_EQUIP; ++it) {
    obj = obj_get(invenD[it]);
    flag = FALSE;
    if (obj->id) {
      obj_desc(obj, TRUE);
      sidx = obj_store_index(obj);
      if (sidx >= 0) {
        cost = store_value(sidx, obj_value(obj), -1);
        flag = TRUE;
      }
    }

    if (flag) {
      BufMsg(screen, "%c) %-57.057s %d", 'a' + it, descD, cost);
    } else {
      line += 1;
    }
  }
}
static void
store_display(sidx)
{
  int line;
  int cost, number;
  struct objS* obj;

  line = 0;
  BufMsg(screen, "         %s", ownerD[storeD[sidx]].name);
  line += 1;
  BufMsg(screen, "   Item");
  for (int it = 0; it < AL(store_objD[0]); ++it) {
    obj = &store_objD[sidx][it];
    cost = store_value(sidx, obj_value(obj), 1);
    if (obj->tidx) {
      // TBD: ouch
      number = obj->number;
      obj->number = 1;
      obj_desc(obj, TRUE);
      obj->number = number;

      BufMsg(screen, "%c) %-57.057s %d", 'a' + it, descD, cost);
    } else {
      line += 1;
    }
  }
  BufMsg(screen, "                 Your Gold Remaining : %d", uD.gold);
}
static void
store_item_purchase(sidx, item)
{
  int iidx, count, cost, flag;
  struct objS* obj;
  struct treasureS* tr_ptr;

  flag = FALSE;
  if (item < MAX_STORE_INVEN) {
    obj = &store_objD[sidx][item];
    if (obj->tidx) {
      count = obj->subval & STACK_BATCH ? obj->number : 1;
      cost = store_value(sidx, obj_value(obj), 1);
      if (uD.gold >= cost) {
        if ((iidx = inven_merge_slot(obj)) >= 0) {
          obj_get(invenD[iidx])->number += count;
          flag = TRUE;
        } else if ((iidx = inven_slot()) >= 0) {
          flag = inven_copy_num(iidx, obj, count);
        } else {
          msg_print("You don't have room in your inventory!");
        }
      } else {
        msg_print("You can't afford that!");
      }

      if (flag) {
        obj = obj_get(invenD[iidx]);
        tr_ptr = &treasureD[obj->tidx];
        tr_make_known(tr_ptr);
        obj_desc(obj, TRUE);
        MSG("You have %s.", descD);
        uD.gold -= cost;
        store_item_destroy(sidx, item, count);
      }
      msg_pause();
    }
  }
}
static void
pawn_entrance()
{
  char c;

  while (1) {
    inven_sort();
    pawn_display();
    if (!in_subcommand("What would you like to sell to Gilbrook The Thrifty?",
                       &c)) {
      break;
    }
    uint8_t item = c - 'a';

    if (item < INVEN_EQUIP) inven_pawn(item);
  }
}
static void
store_entrance(sidx)
{
  char c;

  while (1) {
    inven_sort();
    store_display(sidx);
    if (!in_subcommand("What would you like to purchase?", &c)) break;
    uint8_t item = c - 'a';

    if (item < MAX_STORE_INVEN) store_item_purchase(sidx, item);
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
py_check_view(y, x)
{
  py_move_light(y, x, y, x);
  if (near_light(y, x)) light_room(y, x);
  FOR_EACH(mon, { update_mon(it_index); });
}
void
ma_tick()
{
  uint32_t new_mflag, delta;

  new_mflag = 0;
  for (int it = 0; it < AL(maD); ++it) {
    int val = maD[it];
    if (val) {
      val -= 1;
      maD[it] = val;
      new_mflag |= (1 << it);
    }
  }

  delta = uD.mflag ^ new_mflag;
  for (int it = 0; it < AL(maD); ++it) {
    if (delta & (1 << it)) {
      if (new_mflag & (1 << it)) {
        ma_bonuses(it, 1);
      } else {
        ma_bonuses(it, -1);
      }
    }
  }
  uD.mflag = new_mflag;
  if (delta) {
    calc_bonuses();
    // Vision changes, TBD: optimize?
    FOR_EACH(mon, { update_mon(it_index); });
  }
}
static int
obj_enchanted(obj)
struct objS* obj;
{
  if (obj->tval < TV_MIN_ENCHANT || obj->tval > TV_MAX_ENCHANT ||
      obj->flags & TR_CURSED)
    return FALSE;
  if (obj->idflag & (ID_REVEAL | ID_MAGIK)) return FALSE;
  if (obj->tohit > 0 || obj->todam > 0 || obj->toac > 0) return TRUE;
  if ((0x4000107fL & obj->flags) && obj->p1 > 0) return TRUE;
  if (0x07ffe980L & obj->flags) return TRUE;

  return FALSE;
}
void
sense_magik()
{
  struct objS* obj;

  /* Allow for a slim chance of detect enchantment -CJS- */
  /* for 1st level char, check once every 2160 turns
     for 40th level char, check once every 416 turns */
  if (((turnD & 0xF) == 0) && (countD.confusion == 0) &&
      (randint((10 + 750 / (5 + uD.lev))) == 1)) {
    for (int it = 0; it < MAX_INVEN; it++) {
      obj = obj_get(invenD[it]);
      /* if in inventory, succeed 1 out of 50 times,
         if in equipment list, success 1 out of 10 times */
      if (obj_enchanted(obj) && (randint(it < INVEN_EQUIP ? 50 : 10) == 1)) {
        MSG("There's something about what you are %s...", describe_use(it));
        disturb(0, 0);
        obj->idflag |= ID_MAGIK;
      }
    }
  }
}
void
tick()
{
  int regen_amount, tmp;

  if (uD.food < 0)
    regen_amount = 0;
  else if (uD.food < PLAYER_FOOD_FAINT)
    regen_amount = PLAYER_REGEN_FAINT;
  else if (uD.food < PLAYER_FOOD_WEAK)
    regen_amount = PLAYER_REGEN_WEAK;
  else
    regen_amount = PLAYER_REGEN_NORMAL;
  if (uD.food < PLAYER_FOOD_FAINT && randint(8) == 1) {
    if (countD.paralysis == 0) MSG("You faint from lack of food.");
    countD.paralysis += randint(5);
  }

  tmp = uD.food - uD.food_digest;
  if (tmp < PLAYER_FOOD_ALERT && uD.food >= PLAYER_FOOD_ALERT) {
    MSG("You are getting hungry.");
  } else if (tmp < PLAYER_FOOD_WEAK && uD.food >= PLAYER_FOOD_WEAK) {
    MSG("You are getting weak from hunger.");
  } else if (tmp < 0) {
    strcpy(death_descD, "starvation");
    py_take_hit(-tmp / 16);
    disturb(1, 0);
  }
  uD.food = tmp;

  if (py_tr(TR_REGEN)) regen_amount = regen_amount * 3 / 2;
  // TBD: Search gives resting status
  if (countD.rest != 0) regen_amount = regen_amount * 2;
  // if (p_ptr->cmana < p_ptr->mana) regenmana(regen_amount);

  if (countD.poison == 0) {
    regenhp(regen_amount);
  } else if (countD.poison > 0) {
    if (countD.poison == 1) {
      msg_print("You feel better.");
    } else {
      strcpy(death_descD, "poison");
      py_take_hit(poison_adj());
    }
    countD.poison -= 1;
    disturb(1, 0);
  }

  if (countD.blind) {
    countD.blind -= 1;
    if (countD.blind == 0) {
      msg_print("The veil of darkness lifts.");
      disturb(0, 1);
      py_check_view(uD.y, uD.x);
    }
  }

  if (countD.confusion > 0) {
    countD.confusion -= 1;
    if (countD.confusion == 0) {
      msg_print("You feel less confused.");
      disturb(0, 0);
    }
  }
  if (countD.fear > 0) {
    if (py_affect(MA_HERO) || py_affect(MA_SUPERHERO)) countD.fear = 1;
    countD.fear -= 1;
    if (countD.fear == 0) {
      msg_print("You feel bolder now.");
      disturb(0, 0);
    }
  }

  if (countD.rest < 0) {
    countD.rest += 1;
    if (uD.chp == uD.mhp) {
      int mal = countD.blind + countD.confusion + countD.fear;
      if (mal == 0) countD.rest = 0;
    }
  } else if (countD.rest > 0) {
    countD.rest -= 1;
  }
  if (countD.paralysis) countD.paralysis -= 1;
  if (countD.protevil > 0) {
    countD.protevil -= 1;
    if (countD.protevil == 0) msg_print("You no longer feel safe from evil.");
  }

  sense_magik();
  ma_tick();
}
void
dungeon()
{
  int c, y, x, iidx;
  uint32_t dir;
  new_level_flag = FALSE;

  uD.max_dlv = MAX(uD.max_dlv, dun_level);
  while (!new_level_flag) {
    msg_countD = 1;
    turnD += 1;
    if (dun_level != 0 && (turnD & ~-1024) == 0) store_maint();
    if (randint(MAX_MALLOC_CHANCE) == 1) alloc_mon(1, MAX_SIGHT, FALSE);
    tick();

    do {
      draw();
      if (countD.rest != 0) break;
      if (countD.paralysis != 0) break;
      turn_flag = FALSE;

      y = uD.y;
      x = uD.x;
      if (py_tr(TR_TELEPORT) && randint(100) == 1) {
        disturb(0, 0);
        py_teleport(40, &y, &x);
      } else if (find_flag) {
        mmove(find_direction, &y, &x);
      } else {
        msg_advance();
        c = inkey();

        // AWN: Period attempts auto-detection of a situational command
        if (c == '.') {
          struct caveS* c_ptr = &caveD[y][x];
          uint8_t tval = entity_objD[c_ptr->oidx].tval;
          uint8_t tchar = entity_objD[c_ptr->oidx].tchar;
          switch (tval) {
            case TV_UP_STAIR:
              c = '<';
              break;
            case TV_DOWN_STAIR:
              c = '>';
              break;
            case 1 ... TV_MAX_PICK_UP:
              c = ',';
              break;
            case TV_STORE_DOOR:
              c = ' ';
              store_entrance(tchar - '1');
              break;
            case TV_PAWN_DOOR:
              c = ' ';
              pawn_entrance();
              break;
            default:
              c = 's';
              break;
          }
        }

        dir = map_roguedir(tolower(c)) - '0';
        if (dir <= 9) {
          // 75% random movement
          if (countD.confusion) {
            c = tolower(c);  // Can't run
            if (randint(4) > 1) dir = randint(9);
          }

          // Primary movemenlowercase)
          if (c & 0x20) {
            mmove(dir, &y, &x);
          } else  // Secondary movement (uppercase)
          {
            find_init(dir, &y, &x);
          }
        } else {
          switch (c) {
            case '?':
              py_help();
              break;
            case ' ':
              break;
            case ',':
              py_pickup(y, x, TRUE);
              break;
            case '1' ... '9':
              MSG("Numlock is required for arrowkey movement");
              break;
            case 'c':
              close_object();
              break;
            case 'd':
              py_drop(y, x);
              break;
            case 'e': {
              int count = inven_screen(INVEN_EQUIP, MAX_INVEN);
              MSG("You wearing %d items.", count);
            } break;
            case 'f':
              py_bash(&y, &x);
              break;
            case 'i': {
              int count = inven_screen(0, INVEN_EQUIP);
              MSG("You carrying %d items.", count);
            } break;
            case 'q':
              iidx = inven_choice("Quaff what?");
              if (iidx >= 0) inven_quaff(iidx);
              break;
            case 'r':
              iidx = inven_choice("Read what?");
              if (iidx >= 0) inven_read(iidx, &y, &x);
              break;
            case 'o':
              py_open();
              break;
            case 's':
              py_search(y, x);
              msg_print("You search the area.");
              break;
            case 'v':
              show_version();
              break;
            case 'w':
              iidx = inven_choice("Wear/Wield which item?");
              if (iidx >= 0 && iidx < INVEN_EQUIP) inven_wear(iidx);
              break;
            case 'x':
              py_look_mon();
              break;
            case 'X':
              py_look_obj();
              break;
            case 'z':
              iidx = inven_choice("Aim which wand?");
              if (iidx >= 0) py_zap(iidx);
              break;
            case 'Z':
              iidx = inven_choice("Invoke which staff?");
              if (iidx >= 0) inven_try_staff(iidx, &y, &x);
              break;
            case '<':
              go_up();
              break;
            case '>':
              go_down();
              break;
            case 'A':
              // Generalized inventory interaction
              iidx = inven_choice("Use what?");
              if (iidx >= 0) {
                struct objS* obj = obj_get(invenD[iidx]);
                if (obj->tval == TV_FOOD) {
                  inven_eat(iidx);
                } else if (obj->tval == TV_POTION1 || obj->tval == TV_POTION2) {
                  inven_quaff(iidx);
                } else if (obj->tval == TV_SCROLL1 || obj->tval == TV_SCROLL2) {
                  inven_read(iidx, &y, &x);
                } else if (obj->tval == TV_STAFF) {
                  inven_try_staff(iidx, &x, &x);
                } else if (obj->tval == TV_WAND) {
                  py_zap(iidx);
                } else if (iidx < INVEN_EQUIP) {
                  inven_wear(iidx);
                }
              }
              break;
            case 'C':
              py_character();
              break;
            case 'D':
              py_disarm(&y, &x);
              break;
            case 'E':
              iidx = inven_choice("Eat what?");
              if (iidx >= 0) inven_eat(iidx);
              break;
            case 'I':
              inven_sort();
              int count = inven_screen(0, INVEN_EQUIP);
              MSG("You carrying %d items.", count);
              break;
            case 'M':
              py_map();
              break;
            case 'R':
              countD.rest = -9999;
              break;
            case 'S':
              iidx = inven_choice("Which item do you wish identified?");
              if (iidx >= 0) inven_study(iidx);
              break;
            case 'T':
              py_takeoff();
              break;
            case 'W':
              py_where();
              break;
            case CTRL('p'): {
              msg_history();
            } break;
            case CTRL('x'): {
              save_exit_flag = TRUE;
              new_level_flag = TRUE;
              strcpy(death_descD, "quitting");
            } break;
            default:
              break;
          }
          if (HACK) {
            switch (c) {
              case CTRL('a'): {
                int* af_ptr = (void*)&countD;
                for (int it = 0; it < sizeof(countD) / sizeof(int); ++it) {
                  af_ptr[it] = 1;
                }
              } break;
              case CTRL('d'): {
                detect_obj(oset_gold);
                detect_obj(oset_pickup);
                detect_obj(oset_trap);
                detect_obj(oset_sdoor);
                maD[MA_DETECT_MON] = 1;
                maD[MA_DETECT_INVIS] = 1;
                maD[MA_DETECT_EVIL] = 1;
              } break;
              case CTRL('f'): {
                create_food(y, x);
              } break;
              case CTRL('h'):
                if (uD.mhp < 1000) uD.mhp = 1000;
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
                  MSG("Teleport to monster id %d (%d/%d)", mon->id, rv,
                      mon_usedD);

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
              case CTRL('p'): {
                countD.paralysis += 1000;
              } break;
              case CTRL('w'): {
                msg_print("The air about you becomes charged.");
                maD[MA_RECALL] = 1;
              } break;
            }
          }
        }
      }

      if (uD.y != y || uD.x != x) {
        struct caveS* c_ptr = &caveD[y][x];
        struct monS* mon = &entity_monD[c_ptr->midx];
        struct objS* obj = &entity_objD[c_ptr->oidx];

        if (find_flag && (mon->mlit || c_ptr->fval > MAX_OPEN_SPACE)) {
          find_flag = FALSE;
        } else {
          // doors known to be jammed are bashed prior to movement
          if (obj->tval == TV_CLOSED_DOOR) {
            if (obj->p1 < 0 && (obj->idflag & ID_REVEAL)) {
              bash(y, x);
            }
          }

          if (mon->id) {
            py_attack(y, x);
          } else if (c_ptr->fval <= MAX_OPEN_SPACE) {
            if (obj->tval == TV_VIS_TRAP) {
              if (obj->idflag & ID_REVEAL) try_disarm_trap(y, x);
            }
            if (obj->tval == TV_INVIS_TRAP || obj->tval == TV_VIS_TRAP) {
              hit_trap(&y, &x);
            }

            py_move_light(uD.y, uD.x, y, x);
            if (countD.blind == 0) {
              if (near_light(y, x)) light_room(y, x);
            }

            // Perception check on movement
            turn_flag = TRUE;
            uD.y = y;
            uD.x = x;
            if (uD.fos <= 1 || randint(uD.fos) == 1) py_search(y, x);
            if (countD.blind == 0 && find_flag) find_event(y, x);

            if (obj->tval == TV_STORE_DOOR) {
              store_entrance(obj->tchar - '1');
            } else if (obj->tval == TV_PAWN_DOOR) {
              pawn_entrance();
            } else if (oset_pickup(obj)) {
              disturb(0, 0);
              py_pickup(y, x, FALSE);
            }
          } else if (obj->tval == TV_CLOSED_DOOR) {
            open_object(y, x);
          }
        }
        panel_update(&panelD, uD.y, uD.x, FALSE);
      }
    } while (!turn_flag && !new_level_flag);

    creatures();
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
  for (i = 1; i < MAX_DUNGEON_OBJ; i++) o_level[treasureD[i].level]++;
  for (i = 1; i <= MAX_OBJ_LEVEL; i++) o_level[i] += o_level[i - 1];

  /* now produce an array with object indexes sorted by level, by using
     the info in o_level, this is an O(n) sort! */
  /* this is not a stable sort, but that does not matter */
  for (i = 0; i <= MAX_OBJ_LEVEL; i++) tmp[i] = 1;
  for (i = 1; i < MAX_DUNGEON_OBJ; i++) {
    l = treasureD[i].level;
    sorted_objects[o_level[l] - tmp[l]] = i;
    tmp[l]++;
  }
}
void
seed_init()
{
  uint32_t seed;

  seed = 0;
  if (platformD.seed) seed = platformD.seed();
  if (seed == 0) seed = 5381;

  obj_seed = seed;

  seed += 8762;
  town_seed = seed;

  seed += 113452;
  rnd_seed = seed;

  // Burn randomness after seeding
  for (int it = randint(100); it != 0; --it) rnd();
}
int
main()
{
  platform_init();

  seed_init();

  dun_level = 1;
  mon_level_init();
  obj_level_init();
  if (platformD.load && platformD.load()) {
  } else {
    py_init();
  }
  calc_bonuses();

  magic_init();

  while (!death) {
    generate_cave();
    panel_update(&panelD, uD.y, uD.x, TRUE);
    py_check_view(uD.y, uD.x);
    dungeon();

    if (!death) {
      cave_reset();
      if (platformD.save && platformD.save()) {
        if (save_exit_flag) break;
      }
    }
  }

  if (strcmp("quitting", death_descD) != 0) py_death();

  platform_reset();
  return 0;
}
