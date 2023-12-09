#include "game.c"

enum { HACK = 0 };
DATA int cycle[] = {1, 2, 3, 6, 9, 8, 7, 4, 1, 2, 3, 6, 9, 8, 7, 4, 1};
DATA int chome[] = {-1, 8, 9, 10, 7, -1, 11, 6, 5, 4};
GAME int find_threat;
DATA int find_direction;
DATA int find_flag;
DATA int find_openarea;
DATA int find_breakright, find_breakleft;
DATA int find_prevdir;
DATA jmp_buf restartD;
DATA int drop_modeD;
DATA char quit_stringD[] = "quitting";
#define MSG(x, ...)                                             \
  {                                                             \
    char vtype[STRLEN_MSG + 1];                                 \
    int len = snprintf(vtype, sizeof(vtype), x, ##__VA_ARGS__); \
    len = CLAMP(len, 0, sizeof(vtype) - 1);                     \
    msg_game(vtype, len);                                       \
  }
// This clobbers unflushed messages, and does not persist to history
#define DRAWMSG(x, ...)                                        \
  {                                                            \
    char* msg = AS(msg_cqD, msg_writeD);                       \
    int len = snprintf(msg, STRLEN_MSG + 1, x, ##__VA_ARGS__); \
    AS(msglen_cqD, msg_writeD) = CLAMP(len, 0, STRLEN_MSG);    \
    draw();                                                    \
    AS(msglen_cqD, msg_writeD) = 0;                            \
  }

#define BufMsg(name, text, ...)                                     \
  {                                                                 \
    int used = name##_usedD[line];                                  \
    int avail = AL(name##D[0]) - used;                              \
    int r;                                                          \
    r = snprintf(name##D[line] + used, avail, text, ##__VA_ARGS__); \
    if (r > 0 && r <= avail) name##_usedD[line++] = used + r;       \
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

  if (input_record_readD < input_record_writeD) {
    c = AS(input_recordD, input_record_readD++);
  } else {
    do {
      c = platformD.readansi();
    } while (c == 0);
    AS(input_recordD, input_record_writeD++) = c;
    input_record_readD += 1;
  }

  return c;
}
void
vital_update()
{
  int used = 0;
  vitalD[used++] = uD.lev;
  vitalD[used++] = uD.exp;
  vitalD[used++] = uD.cmana;
  vitalD[used++] = uD.mhp;
  vitalD[used++] = uD.chp;
  vitalD[used++] = cbD.pac - cbD.hide_toac;
  vitalD[used++] = uD.gold;
  vital_usedD = used;

  for (int it = 0; it < MAX_A; ++it) {
    vital_statD[it] = statD.use_stat[it];
  }
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
// Match single index
static int
py_affect(maid)
{
  return (uD.mflag & (1 << maid)) != 0;
}
// Match ALL trflag
static int
py_tr(trflag)
{
  return (cbD.tflag & trflag) == trflag;
}
int
py_speed()
{
  return (py_affect(MA_SLOW) + py_tr(TR_SLOWNESS)) -
         (py_affect(MA_FAST) + py_tr(TR_SPEED));
}
int
think_adj(stat)
{
  int value;

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
uspellcount()
{
  int splev, tadj, sptype;
  sptype = classD[uD.clidx].spell;
  if (sptype) {
    splev = uD.lev - classD[uD.clidx].first_spell_lev + 1;
    if (splev > 0) {
      tadj = think_adj(sptype == SP_MAGE ? A_INT : A_WIS);
      if (tadj > 0) {
        tadj = CLAMP(tadj - 2, 2, 5);
        return CLAMP(tadj * splev / 2, 1, SP_MAX);
      }
    }
  }
  return 0;
}
int
uspellmask()
{
  int spcount;
  uint32_t spmask;
  spcount = uspellcount();
  spmask = 0;
  for (int it = 0; it < spcount; ++it) {
    if (spell_orderD[it]) {
      spmask |= (1 << (spell_orderD[it] - 1));
    }
  }
  return spmask;
}
struct spellS*
uspelltable()
{
  int clidx = uD.clidx;
  if (clidx) {
    return spellD[clidx - 1];
  }
  return 0;
}
void
affect_update()
{
  int idx = 0;
  int spcount = uspellcount();
  int pspeed = py_speed();

  // Recall, hungry, pack_heavy
  active_affectD[idx++] = py_affect(MA_RECALL) != 0;
  active_affectD[idx] = (uD.food <= PLAYER_FOOD_ALERT);
  active_affectD[idx++] += (uD.food <= PLAYER_FOOD_WEAK);
  active_affectD[idx++] = (pack_heavy != 0);

  // Fast
  active_affectD[idx] = pspeed < 0;
  active_affectD[idx++] += pspeed < -1;
  // Slow
  active_affectD[idx] = pspeed > 0;
  active_affectD[idx++] += pspeed > 1;
  // Blind
  active_affectD[idx++] = (maD[MA_BLIND] != 0);

  // Hero, feare, confusion
  active_affectD[idx++] = py_affect(MA_HERO) + (2 * py_affect(MA_SUPERHERO));
  active_affectD[idx++] = py_affect(MA_FEAR);
  active_affectD[idx++] = (countD.confusion != 0);

  // SeeInvis, paralysis, poison
  active_affectD[idx++] = (cbD.tflag & TR_SEE_INVIS) != 0;
  active_affectD[idx++] = (countD.paralysis != 0);
  active_affectD[idx++] = (countD.poison != 0);

  // Gain spells, imagine, ...
  active_affectD[idx++] = (spcount && spell_orderD[spcount - 1] == 0);
  active_affectD[idx++] = (countD.imagine != 0);
}
void
draw()
{
  if (!replay_flag) {
    vital_update();
    affect_update();

    platformD.predraw();
    platformD.draw();
  }
  AC(screen_usedD);
  AC(overlay_usedD);
  minimap_enlargeD = FALSE;
}

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

  log_used = AS(msglen_cqD, msg_writeD);
  if (log_used) {
    msg_moreD += 1;

    // wait for user to acknowledge prior buffer -more-
    if (replay_flag) {
      draw();
    } else {
      do {
        draw();
        do {
          c = platformD.readansi();
        } while (c == 0);
        if (c == ESCAPE) break;
        if (c == CTRL('c')) break;
      } while (c != ' ');
    }
    msg_advance();
  }
}

static void msg_game(msg, msglen) char* msg;
{
  char* log;
  int log_used, msg_used;
  USE(msg_width);

  log_used = AS(msglen_cqD, msg_writeD);
  if (log_used + msglen >= msg_width) {
    msg_pause();
    log_used = 0;
  }

  log = AS(msg_cqD, msg_writeD);
  msg_used = snprintf(log + log_used, msg_width - log_used, "%s ", msg);

  if (msg_used > 0)
    AS(msglen_cqD, msg_writeD) = MIN(log_used + msg_used, msg_width);

  if (countD.rest) countD.rest = 0;
  if (find_flag) find_flag = 0;
}
#define msg_print(x) msg_game(x, AL(x) - 1)
#define see_print(x) \
  if (maD[MA_BLIND] == 0) msg_print(x)
int
show_history()
{
  char* log;
  int log_used;
  int line = AL(screenD);

  screenD[0][0] = ' ';
  screen_usedD[0] = 1;
  screen_submodeD = 2;

  for (int it = MAX_MSG; it > 0; --it) {
    log = AS(msg_cqD, msg_writeD + it);
    log_used = AS(msglen_cqD, msg_writeD + it);
    if (log_used) {
      line -= 1;
      memcpy(screenD[line], log, log_used);
      screen_usedD[line] = log_used;
    }
  }

  int count = AL(screenD) - line;
  DRAWMSG("Message History (%d)", count);
  return inkey();
}
int
in_subcommand(prompt, command)
char* prompt;
char* command;
{
  char c;

  DRAWMSG("%s", prompt ? prompt : "");
  do {
    c = inkey();
    if (is_ctrl(c)) break;
  } while (c == ' ');
  *command = c;
  return is_ctrl(c) ? 0 : 1;
}
static char
map_roguedir(comval)
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
  char c, command;
  if (!prompt) prompt = "Which direction?";
  DRAWMSG("%s", prompt);
  do {
    c = inkey();
  } while (c == ' ');
  command = map_roguedir(c);
  if (command >= '1' && command <= '9' && command != '5') {
    *dir = command - '0';
    return 1;
  }

  return 0;
}

int
bit_pos(test)
uint32_t* test;
{
  int i;
  uint32_t mask = 0x1;

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
  struct caveS* c_ptr;
  int no_stairs = FALSE;

  c_ptr = &caveD[uD.y][uD.x];
  if (c_ptr->oidx != 0)
    if (entity_objD[c_ptr->oidx].tval == TV_UP_STAIR) {
      turn_flag = TRUE;
      dun_level -= 1;
      uD.new_level_flag = NL_UP_STAIR;
    } else
      no_stairs = TRUE;
  else
    no_stairs = TRUE;

  if (no_stairs) {
    msg_print("You see no up staircase here.");
  }
}
static void
go_down()
{
  struct caveS* c_ptr;
  int no_stairs = FALSE;

  c_ptr = &caveD[uD.y][uD.x];
  if (c_ptr->oidx != 0)
    if (entity_objD[c_ptr->oidx].tval == TV_DOWN_STAIR) {
      turn_flag = TRUE;
      dun_level += 1;
      uD.new_level_flag = NL_DOWN_STAIR;
    } else
      no_stairs = TRUE;
  else
    no_stairs = TRUE;

  if (no_stairs) {
    msg_print("You see no down staircase here.");
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
  long low, high, test;

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
{
  long randval;

  randval = rnd();
  return ((int)(randval % maxval) + 1);
}
int
randnor(mean, stand)
{
  int offset, low, iindex, high;
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
void
seed_init(prng)
{
  uint32_t seed;

  seed = 0;
  seed = prng;
  if (seed == 0) seed = 5381;

  obj_seed = seed;

  seed += 8762;
  town_seed = seed;

  seed += 113452;
  rnd_seed = seed;

  // Burn randomness after seeding
  for (int it = randint(100); it != 0; --it) rnd();
}
int fixed_seed_func(seed, func) int (*func)();
{
  uint32_t keep_seed;

  keep_seed = rnd_seed;
  rnd_seed = seed;
  int ret = func();
  rnd_seed = keep_seed;
  return ret;
}
int
damroll(num, sides)
{
  int i, sum = 0;

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
critical_blow(weight, plus, lev_adj, dam)
{
  int critical;

  critical = dam;
  /* Weight of weapon, plusses to hit, and character level all      */
  /* contribute to the chance of a critical  		   */
  if (randint(5000) <= (int)(weight + 5 * plus + lev_adj)) {
    weight += randint(650);
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
      (obj->tval == TV_HAFTED || obj->tval == TV_POLEARM ||
       obj->tval == TV_SWORD)) {
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
static void build_type1(ychunk, xchunk, ycenter, xcenter) int* ycenter;
int* xcenter;
{
  int x, y;
  uint8_t floor;
  struct caveS* c_ptr;

  if (dun_level <= randint(25))
    floor = FLOOR_LIGHT;
  else
    floor = FLOOR_DARK;

  y = ychunk * CHUNK_HEIGHT + CHUNK_HEIGHT / 2;
  x = xchunk * CHUNK_WIDTH + CHUNK_WIDTH / 2;

  int limit = 3;
  for (int it = 0; it < limit; it++) {
    int cxmin, cxmax;
    int cymin, cymax;

    cymin = y - randint(CHUNK_HEIGHT / 2);
    cymax = y + randint(CHUNK_HEIGHT / 2 - 1);
    cxmin = x - randint(CHUNK_WIDTH / 2);
    cxmax = x + randint(CHUNK_WIDTH / 2 - 1);

    for (int i = cymin; i <= cymax; i++) {
      for (int j = cxmin; j <= cxmax; j++) {
        c_ptr = &caveD[i][j];
        c_ptr->cflag |= CF_ROOM;

        if (i == cymin || i == cymax) {
          if (c_ptr->fval == 0) c_ptr->fval = GRANITE_WALL;
        } else if (j == cxmin || j == cxmax) {
          if (c_ptr->fval == 0) c_ptr->fval = GRANITE_WALL;
        } else {
          c_ptr->fval = floor;
        }
      }
    }
  }

  *ycenter = y;
  *xcenter = x;
}
static void build_room(ychunk, xchunk, ycenter, xcenter) int *ycenter, *xcenter;
{
  int x, xmax, y, ymax;
  int wroom, hroom;
  uint8_t floor;
  struct caveS *c_ptr, *d_ptr;

  if (dun_level <= randint(25))
    floor = FLOOR_LIGHT;
  else
    floor = FLOOR_DARK;

  if (ychunk + 1 == MAX_HEIGHT / CHUNK_HEIGHT) {
    hroom = CHUNK_HEIGHT - 1;
  } else {
    hroom = CHUNK_HEIGHT - randint(CHUNK_HEIGHT - 4);
  }
  if (xchunk + 1 == MAX_WIDTH / CHUNK_WIDTH) {
    wroom = CHUNK_WIDTH - 1;
  } else {
    wroom = CHUNK_WIDTH - randint(CHUNK_WIDTH - 4);
  }
  y = ychunk * CHUNK_HEIGHT;
  x = xchunk * CHUNK_WIDTH;
  ymax = y + hroom;
  xmax = x + wroom;
  *ycenter = y + hroom / 2;
  *xcenter = x + wroom / 2;

  for (int i = y; i <= ymax; i++) {
    for (int j = x; j <= xmax; j++) {
      c_ptr = &caveD[i][j];
      c_ptr->fval = floor;
      c_ptr->cflag |= CF_ROOM;
    }
  }

  for (int i = y; i <= ymax; i++) {
    c_ptr = &caveD[i][x];
    c_ptr->fval = GRANITE_WALL;
    c_ptr = &caveD[i][xmax];
    c_ptr->fval = GRANITE_WALL;
  }

  c_ptr = &caveD[y][x];
  d_ptr = &caveD[ymax][x];
  for (int i = x; i <= xmax; i++) {
    c_ptr->fval = GRANITE_WALL;
    c_ptr++;
    d_ptr->fval = GRANITE_WALL;
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
typedef struct {
  int y;
  int x;
} coords;
int
in_bounds(row, col)
{
  uint32_t urow = row - 1;
  uint32_t ucol = col - 1;

  return urow < (MAX_HEIGHT - 2) && ucol < (MAX_WIDTH - 2);
}
int
diff_chunk(y1, x1, y2, x2)
{
  return (y1 / CHUNK_HEIGHT != y2 / CHUNK_HEIGHT ||
          x1 / CHUNK_WIDTH != x2 / CHUNK_WIDTH);
}
int
distance(y1, x1, y2, x2)
{
  int dy, dx;

  dy = y1 - y2;
  if (dy < 0) dy = -dy;
  dx = x1 - x2;
  if (dx < 0) dx = -dx;

  return ((((dy + dx) << 1) - (dy > dx ? dx : dy)) >> 1);
}
static void rand_dir(rdir, cdir) int *rdir, *cdir;
{
  int tmp;

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
static void
place_broken_door(broken, y, x)
{
  struct objS* obj;
  struct caveS* cave_ptr;

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
static void
place_closed_door(locked, y, x)
{
  struct objS* obj;
  struct caveS* cave_ptr;

  // invcopy(&t_list[cur_pos], OBJ_CLOSED_DOOR);
  obj = obj_use();
  if (obj->id) {
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
}
static void
place_secret_door(y, x)
{
  struct objS* obj;
  struct caveS* cave_ptr;

  // invcopy(&t_list[cur_pos], OBJ_SECRET_DOOR);
  obj = obj_use();
  obj->fy = y;
  obj->fx = x;
  obj->tval = TV_SECRET_DOOR;
  obj->tchar = '#';
  obj->subval = 1;
  obj->number = 1;

  cave_ptr = &caveD[y][x];
  cave_ptr->oidx = obj_index(obj);
  cave_ptr->fval = FLOOR_OBST;
}
static void
place_door(y, x)
{
  int tmp;
  int lock;

  tmp = randint(2 + (dun_level >= 5));
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
DATA coords doorstk[100];
DATA int doorindex;
static void
build_corridor(row1, col1, row2, col2)
{
  int tmp_row, tmp_col, i, j;
  struct caveS* c_ptr;
  struct caveS* d_ptr;
  coords tunstk[1000], wallstk[1000];
  coords* tun_ptr;
  int row_dir, col_dir, tunindex, wallindex;
  int stop_flag, door_flag, wall_flag, main_loop_count;
  int start_row, start_col;

  /* Main procedure for Tunnel  		*/
  stop_flag = FALSE;
  door_flag = FALSE;
  wall_flag = FALSE;
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
    if (c_ptr->fval == TMP2_WALL) continue;

    // Room pass-through
    if (wall_flag && c_ptr->fval == FLOOR_NULL) {
      if (caveD[row1][col1].fval == GRANITE_WALL) {
        if (wallindex < 1000) {
          wallstk[wallindex].y = row1;
          wallstk[wallindex].x = col1;
          wallindex++;
        }
      }
      wall_flag = FALSE;
    }

    row1 = tmp_row;
    col1 = tmp_col;
    if (c_ptr->fval == FLOOR_NULL) {
      if (tunindex < 1000) {
        tunstk[tunindex].y = row1;
        tunstk[tunindex].x = col1;
        tunindex++;
      }
      door_flag = FALSE;
    } else if (!wall_flag && c_ptr->fval == GRANITE_WALL) {
      if (wallindex < 1000) {
        wallstk[wallindex].y = tmp_row;
        wallstk[wallindex].x = tmp_col;
        wallindex++;
      }

      // Puncture through wall in this direction
      if (diff_chunk(tmp_row, tmp_col, tmp_row + row_dir, tmp_col + col_dir)) {
        if (wallindex < 1000) {
          wallstk[wallindex].y = tmp_row + row_dir;
          wallstk[wallindex].x = tmp_col + col_dir;
          wallindex++;
        }
      }

      // Protect adjacent remaining wall
      for (i = tmp_row - 1; i <= tmp_row + 1; i++)
        for (j = tmp_col - 1; j <= tmp_col + 1; j++) {
          d_ptr = &caveD[i][j];
          if (d_ptr->fval == GRANITE_WALL) {
            d_ptr->fval = TMP2_WALL;
          }
        }

      wall_flag = TRUE;

    } else if (c_ptr->fval == FLOOR_CORR || c_ptr->fval == FLOOR_OBST) {
      if (!door_flag) {
        if (doorindex < AL(doorstk)) {
          doorstk[doorindex].y = row1;
          doorstk[doorindex].x = col1;
          doorindex++;
        }
        door_flag = TRUE;
      }
      if (randint(100) > DUN_TUN_CON) {
        int cdis = distance(start_row, start_col, tmp_row, tmp_col);
        if (cdis > 10) {
          stop_flag = TRUE;
        }
      }
    }
  } while (((row1 != row2) || (col1 != col2)) && (!stop_flag));

  tun_ptr = &tunstk[0];
  for (i = 0; i < tunindex; i++) {
    d_ptr = &caveD[tun_ptr->y][tun_ptr->x];
    d_ptr->fval = FLOOR_CORR;
    tun_ptr++;
  }
  for (i = 0; i < wallindex; i++) {
    tmp_row = wallstk[i].y;
    tmp_col = wallstk[i].x;
    c_ptr = &caveD[tmp_row][tmp_col];
    // Filter duplicates in wallstk; protects TMP1_WALL
    if (c_ptr->fval > TMP1_WALL) {
      if (c_ptr->cflag & CF_UNUSUAL) {
        if (randint(3) == 1) {
          place_secret_door(wallstk[i].y, wallstk[i].x);
        } else {
          place_closed_door(randint(21) - 11, wallstk[i].y, wallstk[i].x);
        }
      } else {
        if (randint(100) < DUN_TUN_PEN)
          place_door(wallstk[i].y, wallstk[i].x);
        else {
          /* these have to be doorways to rooms */
          c_ptr->fval = FLOOR_CORR;
        }
      }
    }
  }
}
static void
fill_cave(fval)
{
  int i, j;
  struct caveS* c_ptr;

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
static char*
detail_nosp()
{
  return detailD + 1;
}
char*
describe_use(iidx)
{
  char* p;

  switch (iidx) {
    case INVEN_LAUNCHER:
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
inven_destroy(iidx)
{
  struct objS* obj = obj_get(invenD[iidx]);
  obj_unuse(obj);
  invenD[iidx] = 0;
}
static void
inven_destroy_num(iidx, number)
{
  struct objS* obj = obj_get(invenD[iidx]);
  if (obj->number <= number)
    inven_destroy(iidx);
  else
    obj->number -= number;
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
static void
place_stair_tval(y, x, tval)
{
  struct objS* obj;
  struct caveS* cave_ptr;
  int tchar;

  tchar = tval == TV_UP_STAIR ? '<' : '>';
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
  int i, j;
  struct caveS* c_ptr;

  do {
    i = randint(MAX_HEIGHT - 2);
    j = randint(MAX_WIDTH - 2);
    c_ptr = &caveD[i][j];
  } while (c_ptr->fval >= MIN_CLOSED_SPACE || (c_ptr->oidx != 0) ||
           (c_ptr->midx != 0));
  *y = i;
  *x = j;
}
static int
next_to_object(y1, x1)
{
  for (int col = y1 - 1; col <= y1 + 1; ++col) {
    for (int row = x1 - 1; row <= x1 + 1; ++row) {
      if (caveD[col][row].oidx) return TRUE;
    }
  }
  return FALSE;
}
static void
place_stairs(tval, num)
{
  struct caveS* cave_ptr;
  int i, j, flag;
  int y1, x1, y2, x2;

  for (i = 0; i < num; i++) {
    flag = FALSE;
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
          //  TBD: (next_to_walls(y1, x1) >= 3)
          if (cave_ptr->fval <= MAX_OPEN_SPACE) {
            if (next_to_object(y1, x1) == 0) {
              flag = TRUE;
              place_stair_tval(y1, x1, tval);
            }
          }
          x1++;
        } while ((x1 != x2) && (!flag));
        x1 = x2 - 12;
        y1++;
      } while ((y1 != y2) && (!flag));
      j++;
    } while ((!flag) && (j <= 30));
  }
}
int
next_to_corr(y, x)
{
  int k, j, i;
  struct caveS* c_ptr;

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
{
  int next;

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
static void
try_door(y, x)
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
{
  return (element == FLOOR_DARK || element == FLOOR_LIGHT);
}
int
set_corr(element)
{
  return (element == FLOOR_CORR || element == FLOOR_OBST);
}
int
set_floor(element)
{
  return (element <= MAX_FLOOR);
}
void tr_obj_copy(tidx, obj) struct objS* obj;
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
  obj->number = 1;
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
int
light_adj(p1)
{
  int bonus;
  if (p1 > 7500) {
    bonus = 3;
  } else if (p1 > 3000) {
    bonus = 2;
  } else if (p1) {
    bonus = 1;
  } else {
    bonus = 0;
  }
  return bonus;
}
static int
ustackweight()
{
  return 10 * statD.use_stat[A_STR];
}
static int
stacklimit_by_max_weight(max, weight)
{
  int stacklimit = 1;
  while (2 * stacklimit * weight <= max) {
    stacklimit *= 2;
  }
  return MIN(stacklimit, 255);
}
int
number_by_weight_roll(weight, num, sides)
{
  int k, tmp;
  int stackweight, limit;
  stackweight = ustackweight();
  limit = stacklimit_by_max_weight(stackweight, weight);
  tmp = 0;
  for (int i = 0; i < num; i++) {
    k = randint(sides);
    if (tmp + k < limit) tmp += k;
  }
  return MAX(1, tmp);
}
void magic_treasure(obj, level) struct objS* obj;
{
  int chance, special, cursed;
  int tmp;

  chance = OBJ_BASE_MAGIC + level;
  if (chance > OBJ_BASE_MAX) chance = OBJ_BASE_MAX;
  special = chance / OBJ_DIV_SPECIAL;
  cursed = (10 * chance) / OBJ_DIV_CURSED;

  /* Depending on treasure type, it can have certain magical properties*/
  switch (obj->tval) {
    case TV_LAUNCHER:
      if (magik(chance)) {
        obj->tohit += m_bonus(1, 30, level);
        obj->todam += m_bonus(1, 20, level);
      } else if (magik(cursed)) {
        obj->tohit -= m_bonus(1, 50, level);
        obj->todam -= m_bonus(1, 30, level);
        obj->flags |= TR_CURSED;
        obj->cost = 0;
      }
      break;
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
            case 1:              /* Holy Avenger   */
              tmp = randint(4);  // str, dex, toac
              obj->flags |= (TR_SEE_INVIS | TR_SUST_STAT | TR_SLAY_UNDEAD |
                             TR_SLAY_EVIL | TR_STR | TR_DEX);
              obj->tohit += 5;
              obj->todam += 5;
              obj->toac += tmp;
              obj->p1 = tmp;
              obj->sn = SN_HA;
              obj->cost += tmp * 500;
              obj->cost += 10000;
              break;
            case 2:              /* Defender   */
              tmp = randint(5);  // stealth, toac
              obj->flags |= (TR_FFALL | TR_RES_LIGHT | TR_SEE_INVIS |
                             TR_FREE_ACT | TR_RES_COLD | TR_RES_ACID |
                             TR_RES_FIRE | TR_REGEN | TR_STEALTH);
              obj->tohit += 3;
              obj->todam += 3;
              obj->ac += 5;
              obj->toac += tmp;
              obj->sn = SN_DF;
              obj->p1 = tmp;
              obj->cost += tmp * 500;
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
          obj->flags |= TR_SLOWNESS;
          obj->sn = SN_SLOWNESS;
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
                obj->flags |= (TR_CHR | TR_CON | TR_SUST_STAT | TR_HERO);
                obj->sn = SN_COURAGE;
                obj->cost += 750;
                break;
              case 5:
                obj->p1 = 5 * (1 + randint(4));
                obj->flags |= (TR_SEE_INVIS | TR_SEARCH | TR_SEEING);
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
              obj->sn = SN_BLINDNESS;
              break;
            case 4:
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
        case 0: /* Attributes */
        case 1:
        case 2:
        case 3:
          obj->p1 = m_bonus(1, 10, level);
          obj->cost += obj->p1 * 100;
          break;
        case 4: /* Speed */
          obj->flags = TR_SPEED;
          break;
        case 5: /* Searching */
          obj->p1 = 5 * m_bonus(1, 20, level);
          obj->cost += obj->p1 * 50;
          break;
        case 19: /* Increase damage        */
          obj->todam += m_bonus(1, 20, level);
          obj->cost += obj->todam * 100;
          break;
        case 20: /* Increase To-Hit        */
          obj->tohit += m_bonus(1, 20, level);
          obj->cost += obj->tohit * 100;
          break;
        case 21: /* Protection        */
          obj->toac += m_bonus(1, 20, level);
          obj->cost += obj->toac * 100;
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
          break;
        default:
          break;
      }
      break;

    case TV_AMULET: /* Amulets        */
      if (obj->subval < 2) {
        obj->p1 = m_bonus(1, 10, level);
        obj->cost += obj->p1 * 100;
      } else if (obj->subval == 2) {
        obj->p1 = 5 * m_bonus(1, 25, level);
        obj->cost += 50 * obj->p1;
      } else if (obj->subval == 8) {
        obj->p1 = 5 * m_bonus(1, 25, level);
        obj->cost += 20 * obj->p1;
      }
      break;

    case TV_LIGHT:
      tmp = randint(obj->p1);
      obj->p1 = tmp;
      obj->tohit = light_adj(tmp);
      obj->idflag = ID_REVEAL;
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

    case TV_CHEST:
      switch (randint(level + 4)) {
        case 1:
          obj->flags = 0;
          obj->sn = SN_EMPTY;
          obj->idflag = ID_REVEAL;
          break;
        case 2:
          obj->flags |= CH_LOCKED;
          obj->sn = SN_LOCKED;
          break;
        case 3:
        case 4:
          obj->flags |= (CH_LOSE_STR | CH_LOCKED);
          obj->sn = SN_POISON_NEEDLE;
          break;
        case 5:
        case 6:
          obj->flags |= (CH_POISON | CH_LOCKED);
          obj->sn = SN_POISON_NEEDLE;
          break;
        case 7:
        case 8:
        case 9:
          obj->flags |= (CH_PARALYSED | CH_LOCKED);
          obj->sn = SN_GAS_TRAP;
          break;
        case 10:
        case 11:
          obj->flags |= (CH_EXPLODE | CH_LOCKED);
          obj->sn = SN_EXPLOSION_DEVICE;
          break;
        case 12:
        case 13:
        case 14:
          obj->flags |= (CH_SUMMON | CH_LOCKED);
          obj->sn = SN_SUMMONING_RUNES;
          break;
        case 15:
        case 16:
        case 17:
          obj->flags |= (CH_PARALYSED | CH_POISON | CH_LOSE_STR | CH_LOCKED);
          obj->sn = SN_MULTIPLE_TRAPS;
          break;
        default:
          obj->flags |= (CH_SUMMON | CH_EXPLODE | CH_LOCKED);
          obj->sn = SN_MULTIPLE_TRAPS;
          break;
      }
      break;

    case TV_SPIKE:
      obj->number = number_by_weight_roll(obj->weight, 3, 3);
      break;
    case TV_PROJECTILE:
      obj->number = number_by_weight_roll(obj->weight, 9, 6);
      break;

    case TV_FOOD:
      if (obj->flags == 0)
        obj->number = number_by_weight_roll(obj->weight, 1, 5);
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
static void tr_unknown_sample(tr_ptr, unknown, sample) struct treasureS* tr_ptr;
int* unknown;
char** sample;
{
  int subval = mask_subval(tr_ptr->subval);
  int k = knowable_tval_subval(tr_ptr->tval, subval);
  int trk = 0;
  if (k) trk = knownD[k - 1][subval];
  *unknown = (trk & TRK_FULL) == 0;
  *sample = trk & TRK_SAMPLE ? " {sampled}" : "";
}
BOOL
tr_is_known(tr_ptr)
struct treasureS* tr_ptr;
{
  int subval = mask_subval(tr_ptr->subval);
  int k = knowable_tval_subval(tr_ptr->tval, subval);
  if (k == 0) return TRUE;
  return knownD[k - 1][subval] & TRK_FULL;
}
void tr_discovery(tr_ptr) struct treasureS* tr_ptr;
{
  // TDB: xp tuning
  uD.exp += (tr_ptr->level + (uD.lev >> 1)) / uD.lev;
}
BOOL
tr_make_known(tr_ptr)
struct treasureS* tr_ptr;
{
  int subval = mask_subval(tr_ptr->subval);
  int k = knowable_tval_subval(tr_ptr->tval, subval);
  if (k == 0) return FALSE;
  int current = knownD[k - 1][subval] & TRK_FULL;
  knownD[k - 1][subval] = TRK_FULL;
  return current == 0;
}
void tr_sample(tr_ptr) struct treasureS* tr_ptr;
{
  int subval = mask_subval(tr_ptr->subval);
  int k = knowable_tval_subval(tr_ptr->tval, subval);
  if (k) knownD[k - 1][subval] = TRK_SAMPLE;
}
BOOL
vuln_fire(obj)
struct objS* obj;
{
  switch (obj->tval) {
    case TV_PROJECTILE:
      return obj->p1 > 1;
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

    case TV_LAUNCHER:
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
    case TV_PROJECTILE:
      return obj->p1 > 1;
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
    case TV_LAUNCHER:
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
    case TV_PROJECTILE:
      return obj->p1 > 1;
    case TV_LAUNCHER:
    case TV_MISC:
    case TV_CHEST:
      return TRUE;
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
    case TV_PROJECTILE:
      return obj->p1 > 1;
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
    case TV_LAUNCHER:
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
  return ((obj->tval == TV_RING) || (obj->tval == TV_WAND));
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
static int
oset_sightfm(obj)
struct objS* obj;
{
  return (obj->tval >= TV_MIN_VISIBLE && obj->tval <= TV_MAX_VISIBLE);
}
static int
oset_mon_pickup(obj)
struct objS* obj;
{
  // Underflow to exclude 0
  uint8_t tval = obj->tval - 1;
  return (tval < TV_MON_PICK_UP);
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
  return (obj->tval == TV_VIS_TRAP || obj->tval == TV_INVIS_TRAP ||
          obj->tval == TV_CHEST);
}
int
oset_doorstair(obj)
struct objS* obj;
{
  return (obj->tval == TV_SECRET_DOOR || obj->tval == TV_UP_STAIR ||
          obj->tval == TV_DOWN_STAIR);
}
int
oset_hidden(obj)
struct objS* obj;
{
  return (obj->tval == TV_VIS_TRAP || obj->tval == TV_INVIS_TRAP ||
          obj->tval == TV_CHEST || obj->tval == TV_CLOSED_DOOR ||
          obj->tval == TV_SECRET_DOOR);
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
    case TV_LAUNCHER:
      return TRUE;
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
      return TRUE;
    case TV_GLOVES:
      return (obj->tohit || obj->todam);
    case TV_RING:
      return (obj->tohit || obj->todam);
    case TV_LIGHT:
      return TRUE;
  }
  return FALSE;
}
static int
oset_enchant(obj)
struct objS* obj;
{
  uint32_t tv = obj->tval;
  tv -= TV_MIN_ENCHANT;
  return tv <= (TV_MAX_ENCHANT - TV_MIN_ENCHANT);
}
static int
oset_rare(obj)
struct objS* obj;
{
  switch (obj->tval) {
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
    case TV_BOOTS:
    case TV_GLOVES:
    case TV_CLOAK:
    case TV_HELM:
    case TV_SHIELD:
    case TV_HARD_ARMOR:
    case TV_SOFT_ARMOR:
      return TRUE;
  }
  return FALSE;
}
static int
oset_armor(obj)
struct objS* obj;
{
  switch (obj->tval) {
    case TV_BOOTS:
    case TV_GLOVES:
    case TV_CLOAK:
    case TV_HELM:
    case TV_SHIELD:
    case TV_HARD_ARMOR:
    case TV_SOFT_ARMOR:
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
oset_obj(obj)
struct objS* obj;
{
  return obj->id != 0;
}
int
set_large(item)         /* Items too large to fit in chests   -DJG- */
struct treasureS* item; /* Use treasure_type since item not yet created */
{
  switch (item->tval) {
    case TV_CHEST:
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
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
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
    case TV_LAUNCHER:
      slot = INVEN_LAUNCHER;
      break;
  }
  return slot;
}
int
may_enchant_ac(tval)
{
  switch (tval) {
    case TV_BOOTS:
    case TV_GLOVES:
    case TV_CLOAK:
    case TV_HELM:
    case TV_SHIELD:
    case TV_HARD_ARMOR:
    case TV_SOFT_ARMOR:
      return TRUE;
  }
  return FALSE;
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
  struct treasureS* tr_ptr;
  struct objS* obj;

  tr_ptr = &treasureD[tr_index];
  if (tr_ptr->subval & STACK_SINGLE) {
    for (int it = 0; it < AL(store_objD[0]); ++it) {
      obj = &store_objD[sidx][it];
      if (obj->tidx == tr_index) {
        stack = MIN(255, stack + obj->number);
        obj->number = stack;
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
      if (obj->subval & STACK_SINGLE) obj->number = stack;
      obj->idflag = ID_REVEAL;
      return TRUE;
    }
  }

  return FALSE;
}
void
store_sort(sidx)
{
  struct objS tmp_obj;
  int i, j;
  for (i = 0; i < AL(store_objD[0]); ++i) {
    for (j = i + 1; j < AL(store_objD[0]); ++j) {
      if (store_objD[sidx][j].tval > store_objD[sidx][i].tval) {
        tmp_obj = store_objD[sidx][i];
        store_objD[sidx][i] = store_objD[sidx][j];
        store_objD[sidx][j] = tmp_obj;
      }
    }
  }
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
          store_ctr -= store_item_destroy(i, k,
                                          (obj->subval & STACK_PROJECTILE)
                                              ? obj->number
                                              : randint(obj->number));
          j -= 1;
        }
      } while (j > 0);
    }

    if (store_ctr < MAX_STORE_INVEN) {
      j = 0;
      if (store_ctr < MIN_STORE_INVEN) j = MIN_STORE_INVEN - store_ctr;
      j += randint(STORE_TURN_AROUND);

      do {
        k = randint(MAX_STORE_CHOICE) - 1;
        store_ctr += store_tr_stack(i, store_choiceD[i][k], store_stockD[i][k]);
        j -= 1;
      } while (j > 0);
    }
    store_sort(i);
  }
}
void
map_area()
{
  struct caveS* c_ptr;

  for (int row = 0; row < MAX_HEIGHT; row++) {
    for (int col = 0; col < MAX_WIDTH; col++) {
      c_ptr = &caveD[row][col];
      if (c_ptr->fval >= MIN_WALL) {
        if (c_ptr->fval == BOUNDARY_WALL || cave_floor_near(row, col)) {
          c_ptr->cflag |= CF_PERM_LIGHT;
        }
      } else if (c_ptr->oidx && oset_sightfm(&entity_objD[c_ptr->oidx])) {
        c_ptr->cflag |= CF_FIELDMARK;
      }
    }
  }
}
int
get_obj_num(level, must_be_small)
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
{
  int i, j, num;

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
    mon->mspeed = cre->speed - 10;
    mon->fy = y;
    mon->fx = x;

    midx = mon_index(mon);
    caveD[y][x].midx = midx;
    return midx;
  }

  return 0;
}
int
place_win_monster()
{
  int cidx, fy, fx, y, x, k;
  struct monS* mon;
  struct creatureS* cr_ptr;

  k = randint(MAX_WIN_MON);
  if (k == MAX_WIN_MON)
    msg_print("You hear a low rumble echo through the caverns.");
  cidx = k + m_level[MAX_MON_LEVEL];
  cr_ptr = &creatureD[cidx];
  y = uD.y;
  x = uD.x;

  if (!total_winner) {
    do {
      fy = randint(MAX_HEIGHT - 2);
      fx = randint(MAX_WIDTH - 2);
    } while ((caveD[fy][fx].fval >= MIN_CLOSED_SPACE) || (caveD[fy][fx].midx) ||
             (caveD[fy][fx].oidx) || (distance(fy, fx, y, x) <= MAX_SIGHT));

    return place_monster(fy, fx, cidx, 1);
  }

  return 0;
}
int
summon_monster(y, x)
{
  int j, k;
  int l, summon;
  struct caveS* cave_ptr;

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
alloc_townmon(num)
{
  int y, x;
  int z;

  for (int i = 0; i < num; i++) {
    do {
      y = randint(SYMMAP_HEIGHT - 2);
      x = randint(SYMMAP_WIDTH - 2);
    } while (caveD[y][x].fval >= MIN_CLOSED_SPACE || (caveD[y][x].midx != 0));

    z = get_mon_num(0);
    place_monster(y, x, z, 1);
  }
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
    place_monster(y, x, z, slp);
  }
}
void
place_rubble(y, x)
{
  struct objS* obj;
  obj = obj_use();
  if (obj->id) {
    caveD[y][x].oidx = obj_index(obj);
    caveD[y][x].fval = FLOOR_OBST;

    // invcopy(... OBJ_RUBBLE);
    obj->fy = y;
    obj->fx = x;
    obj->tval = TV_RUBBLE;
    obj->tchar = ':';
    obj->subval = 1;
    obj->number = 1;
  }
}

#define MAX_GOLD 18
DATA int goldD[MAX_GOLD] = {
    3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 24, 28, 32, 40, 80,
};
DATA char* gold_nameD[MAX_GOLD] = {
    "copper",    "copper",  "copper", "silver",   "silver",   "silver",
    "garnets",   "garnets", "gold",   "gold",     "gold",     "opals",
    "sapphires", "gold",    "rubies", "diamonds", "emeralds", "mithril",
};
void
place_gold(y, x)
{
  int i;
  struct objS* obj;

  obj = obj_use();
  if (obj->id) {
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
  }
}
void
place_object(y, x, must_be_small)
{
  struct objS* obj;

  obj = obj_use();
  if (obj->id) {
    caveD[y][x].oidx = obj_index(obj);

    int sn = get_obj_num(dun_level, must_be_small);
    int z = sorted_objects[sn];

    tr_obj_copy(z, obj);
    obj->fy = y;
    obj->fx = x;

    magic_treasure(obj, dun_level);
  }
}
void
place_trap(y, x, offset)
{
  struct objS* obj;

  obj = obj_use();
  if (obj->id) {
    caveD[y][x].oidx = obj_index(obj);
    tr_obj_copy(OBJ_TRAP_BEGIN + offset, obj);
    obj->fy = y;
    obj->fx = x;
  }
}
void alloc_obj(alloc_set, typ, num) int (*alloc_set)();
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
        place_rubble(y, x);
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
static void
build_vault(y, x)
{
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      if (i != 0 || j != 0) caveD[y + i][x + j].fval = TMP1_WALL;
    }
  }
}
static void
build_pillar(y, x)
{
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      caveD[y + i][x + j].fval = TMP1_WALL;
    }
  }
}
static void
build_chamber(y, x, h, w)
{
  for (int j = -w; j <= w; ++j) {
    caveD[y + h][x + j].fval = TMP1_WALL;
    caveD[y - h][x + j].fval = TMP1_WALL;
  }
  caveD[y][x - w].fval = TMP1_WALL;
  caveD[y][x + w].fval = TMP1_WALL;
  caveD[y][x].fval = TMP1_WALL;
}
static void
chunk_trap(ychunk, xchunk, num)
{
  int ymin, xmin;
  int y, x;
  struct caveS* c_ptr;

  xmin = xchunk * CHUNK_WIDTH;
  ymin = ychunk * CHUNK_HEIGHT;

  do {
    y = ymin + randint(CHUNK_HEIGHT) - 1;
    x = xmin + randint(CHUNK_WIDTH) - 1;
    c_ptr = &caveD[y][x];
    if (c_ptr->fval != 0 && c_ptr->fval <= MAX_OPEN_SPACE && c_ptr->oidx == 0) {
      place_trap(y, x, randint(MAX_TRAP) - 1);
      num -= 1;
    }
  } while (num);
}
static void
room_object(y, x, chance)
{
  struct caveS* c_ptr;
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      c_ptr = &caveD[y + i][x + j];
      if (c_ptr->fval && c_ptr->fval <= MAX_OPEN_SPACE && c_ptr->oidx == 0) {
        if (randint(chance) == 1) place_object(y + i, x + j, FALSE);
      }
    }
  }
}
static void
room_monster(y, x, chance)
{
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      if (caveD[y + i][x + j].fval <= MAX_OPEN_SPACE && randint(chance) == 1)
        place_monster(y + i, x + j, get_mon_num(dun_level + MON_SUMMON_ADJ),
                      TRUE);
    }
  }
}
static void build_type2(ychunk, xchunk, ycenter, xcenter, type1,
                        type2) int* ycenter;
int* xcenter;
{
  int rflag, floor, ydoor;
  int xmin, xmax, ymin, ymax;
  int y, x;
  struct caveS* c_ptr;

  rflag = CF_ROOM;
  if (dun_level <= randint(25))
    floor = FLOOR_LIGHT;
  else
    floor = FLOOR_DARK;

  ymin = ychunk * CHUNK_HEIGHT;
  xmin = xchunk * CHUNK_WIDTH;
  ymax = ymin + CHUNK_HEIGHT - 1;
  xmax = xmin + CHUNK_WIDTH - 1;
  y = (ymin + ymax) / 2;
  x = (xmin + xmax) / 2;

  switch (type1) {
    case 1:
      ymax -= 1;
      for (int i = ymin; i <= ymax; ++i) {
        for (int j = xmin; j <= xmax; ++j) {
          c_ptr = &caveD[i][j];
          c_ptr->cflag |= rflag;
          if (i == ymin || i == ymax)
            c_ptr->fval = GRANITE_WALL;
          else if (j == xmin || j == xmax)
            c_ptr->fval = GRANITE_WALL;
          else
            c_ptr->fval = floor;
        }
      }

      build_chamber(y, x, 1, 3);
      place_secret_door(y - 3 + (randint(2) << 1), x + randint(2));
      place_secret_door(y - 3 + (randint(2) << 1), x - randint(2));
      switch (randint(2)) {
        case 1:
          place_object(y, x - 2, FALSE);
          break;
        case 2:
          place_object(y, x + 2, FALSE);
          break;
      }
      room_monster(y, x - 1, 2);
      room_monster(y, x + 1, 2);
      break;
    case 2:
      x += 4 - (type2 % 8);
      build_vault(y, x);
      place_closed_door(randint(10), y - 3 + (randint(2) << 1), x);
      place_object(y, x, FALSE);
      room_monster(y, x, 1);

      ymin = y - 3;
      ymax = y + 3;
      xmin = x - 3;
      xmax = x + 3;
      for (int i = ymin; i <= ymax; ++i) {
        for (int j = xmin; j <= xmax; ++j) {
          c_ptr = &caveD[i][j];
          c_ptr->cflag |= rflag;
          if (i == ymin || i == ymax)
            c_ptr->fval = GRANITE_WALL;
          else if (j == xmin || j == xmax)
            c_ptr->fval = GRANITE_WALL;
          else if (c_ptr->fval == 0)
            c_ptr->fval = floor;
        }
      }
      break;
    case 3:
      xmax -= 1;
      ymax -= 1;
      for (int i = ymin; i <= ymax; ++i) {
        for (int j = xmin; j <= xmax; ++j) {
          c_ptr = &caveD[i][j];
          c_ptr->cflag |= rflag;
          if (i == ymin || i == ymax)
            c_ptr->fval = GRANITE_WALL;
          else if (j == xmin || j == xmax)
            c_ptr->fval = GRANITE_WALL;
          else
            c_ptr->fval = floor;
        }
      }

      if (type2 & 0x1) {
        build_pillar(y, x - 4);
      }
      if (type2 & 0x2) {
        build_pillar(y, x + 4);
      }
      if (type2 & 0x4) {
        build_pillar(y, x);
      }
      if (type2 == 8) {
        build_vault(y, x - 4);
        build_vault(y, x + 4);
        build_vault(y, x);
        switch (randint(3)) {
          case 1:
            place_object(y, x - 4, FALSE);
            break;
          case 2:
            place_object(y, x + 4, FALSE);
            break;
          case 3:
            place_object(y, x, FALSE);
            break;
        }
      }
      break;
    case 4:
      rflag |= CF_UNUSUAL;
      for (int i = ymin; i <= ymax; ++i) {
        for (int j = xmin; j <= xmax; ++j) {
          c_ptr = &caveD[i][j];
          c_ptr->cflag |= rflag;
          if (i == ymin || i == ymax)
            c_ptr->fval = GRANITE_WALL;
          else if (j == xmin || j == xmax)
            c_ptr->fval = GRANITE_WALL;
          else if ((i ^ j) & 0x1)
            c_ptr->fval = TMP1_WALL;
          else
            c_ptr->fval = floor;
        }
      }
      // Maze
      chunk_trap(ychunk, xchunk, 2);
      switch (randint(2)) {
        case 1:
          room_object(y, x - 5, 4);
          break;
        case 2:
          room_object(y, x + 5, 4);
          break;
      }
      room_monster(y, x - 5, 3);
      room_monster(y, x + 5, 3);
      break;
    case 5:
      rflag |= CF_UNUSUAL;
      ymax -= 1;
      for (int i = ymin; i <= ymax; ++i) {
        for (int j = xmin; j <= xmax; ++j) {
          c_ptr = &caveD[i][j];
          c_ptr->cflag |= rflag;
          if (i == ymin || i == ymax)
            c_ptr->fval = GRANITE_WALL;
          else if (j == xmin || j == xmax)
            c_ptr->fval = GRANITE_WALL;
          else if (i == y)
            c_ptr->fval = TMP1_WALL;
          else if (j == x)
            c_ptr->fval = TMP1_WALL;
          else
            c_ptr->fval = floor;
        }
      }

      ydoor = ymin + randint(ymax - ymin);
      place_secret_door(ydoor, x);
      if (ydoor != y) {
        place_secret_door(y, xmin + randint(CHUNK_WIDTH / 2 - 2));
        place_secret_door(y, x + randint(CHUNK_WIDTH / 2 - 2));
      }
      // Quadrant
      room_object(y, x, 4);
      room_monster(y + 1, x - 4, 4);
      room_monster(y + 1, x + 4, 4);
      room_monster(y - 1, x - 4, 4);
      room_monster(y - 1, x + 4, 4);
      break;
    case 6:
      ymin = y - 1;
      ymax = y + 2;
      xmin = xchunk * CHUNK_WIDTH;
      xmax = xmin + CHUNK_WIDTH - 2;
      for (int i = ymin; i <= ymax; ++i) {
        for (int j = xmin; j <= xmax; ++j) {
          c_ptr = &caveD[i][j];
          c_ptr->cflag |= rflag;
          if (i == ymin || i == ymax)
            c_ptr->fval = GRANITE_WALL;
          else if (j == xmin || j == xmax)
            c_ptr->fval = GRANITE_WALL;
          else
            c_ptr->fval = floor;
        }
      }

      ymin = ychunk * CHUNK_HEIGHT;
      ymax = ymin + CHUNK_HEIGHT - 1;
      xmin = x - 2;
      xmax = x + 2;
      for (int i = ymin; i <= ymax; ++i) {
        for (int j = xmin; j <= xmax; ++j) {
          c_ptr = &caveD[i][j];
          c_ptr->cflag |= rflag;
          if (i == ymin || i == ymax)
            c_ptr->fval = (c_ptr->fval == 0) ? GRANITE_WALL : c_ptr->fval;
          else if (j == xmin || j == xmax)
            c_ptr->fval = (c_ptr->fval == 0) ? GRANITE_WALL : c_ptr->fval;
          else
            c_ptr->fval = floor;
        }
      }

      // + shaped rooms fill to y-max
      // This is a work around for vertical stacking
      // TBD: all room types may fail to connect when y-adjacent fill to y-max?
      caveD[ymax][x].fval = 0;

      if (type2 & 0x1) {
        caveD[y][x].fval = TMP1_WALL;
        caveD[y + 1][x].fval = TMP1_WALL;
      }
      if (type2 > 4) {
        caveD[y - 1][x - 1].fval = TMP1_WALL;
        caveD[y - 1][x + 1].fval = TMP1_WALL;
        caveD[y + 2][x - 1].fval = TMP1_WALL;
        caveD[y + 2][x + 1].fval = TMP1_WALL;
      }
      chunk_trap(ychunk, xchunk, 1 + randint(2));
      break;
  }
  *ycenter = y;
  *xcenter = x;
}
int
mmove(dir, y, x)
int *y, *x;
{
  int new_row, new_col;
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
static void
place_streamer(fval, treas_chance)
{
  int i, tx, ty;
  int y, x, t1, t2, dir;
  struct caveS* c_ptr;

  /* Choose starting point and direction  	*/
  y = (MAX_HEIGHT / 2) - randint(SYMMAP_HEIGHT);
  x = (MAX_WIDTH / 2) - randint(SYMMAP_WIDTH);

  dir = randint(8); /* Number 1-4, 6-9  */
  if (dir > 4) dir = dir + 1;

  /* Place streamer into dungeon  		*/
  t1 = 2 * DUN_STR_RNG + 1; /* Constants  */
  t2 = DUN_STR_RNG + 1;
  do {
    for (i = 0; i < DUN_STR_DEN; i++) {
      ty = y + randint(t1) - t2;
      tx = x + randint(t1) - t2;
      if (in_bounds(ty, tx)) {
        c_ptr = &caveD[ty][tx];
        if (c_ptr->fval == GRANITE_WALL) {
          c_ptr->fval = fval;
          if (randint(treas_chance) == 1) place_gold(ty, tx);
        }
      }
    }
  } while (mmove(dir, &y, &x));
}
void
cave_gen()
{
  int room_map[CHUNK_COL][CHUNK_ROW] = {0};
  int i, j, k, alloc_level;
  int y1, x1, y2, x2, pick1, pick2;
  int yloc[CHUNK_AREA + 1], xloc[CHUNK_AREA + 1];

  alloc_level = CLAMP(dun_level / 2, 2, 15);
  k = randnor(DUN_ROOM_MEAN + alloc_level, 2);
  for (i = 0; i < k; i++)
    room_map[randint(AL(room_map)) - 1][randint(AL(room_map[0])) - 1] += 1;
  k = 0;
  pick1 = pick2 = 0;
  for (i = 0; i < AL(room_map); i++)
    for (j = 0; j < AL(room_map[0]); j++)
      if (room_map[i][j]) {
        if (dun_level > randint(DUN_UNUSUAL)) {
          pick2 += 1;
          build_type2(i, j, &yloc[k], &xloc[k], randint(6), randint(8));
        } else {
          if (room_map[i][j] == 1) {
            build_room(i, j, &yloc[k], &xloc[k]);
          } else {
            pick1 += 1;
            build_type1(i, j, &yloc[k], &xloc[k]);
          }
        }
        k++;
      }
  // MSG("feet %d room: %d (type1: %d type2: %d)", dun_level * 50, k, pick1,
  // pick2);

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
  /* move zero entry to k, so that can call build_corridor all k times */
  yloc[k] = yloc[0];
  xloc[k] = xloc[0];
  for (i = 0; i < k; i++) {
    y1 = yloc[i];
    x1 = xloc[i];
    y2 = yloc[i + 1];
    x2 = xloc[i + 1];
    build_corridor(y2, x2, y1, x1);
  }

  fill_cave(GRANITE_WALL);
  for (i = 0; i < DUN_STR_MAG; i++) place_streamer(MAGMA_WALL, DUN_STR_MC);
  for (i = 0; i < DUN_STR_QUA; i++) place_streamer(QUARTZ_WALL, DUN_STR_QC);
  place_boundary();
  /* Place intersection doors  */
  for (i = 0; i < doorindex; i++) {
    try_door(doorstk[i].y, doorstk[i].x - 1);
    try_door(doorstk[i].y, doorstk[i].x + 1);
    try_door(doorstk[i].y - 1, doorstk[i].x);
    try_door(doorstk[i].y + 1, doorstk[i].x);
  }
  place_stairs(TV_DOWN_STAIR, randint(2));
  place_stairs(TV_UP_STAIR, 1);
  /* Set up the character co-ords, used by alloc_monster, place_win_monster
   */
  new_spot(&uD.y, &uD.x);

  if (dun_level >= 7) alloc_obj(set_corr, 3, randint(alloc_level));
  alloc_obj(set_room, 5, randnor(TREAS_ROOM_MEAN, 3));
  alloc_obj(set_floor, 5, randnor(TREAS_ANY_ALLOC, 3));
  alloc_obj(set_floor, 4, randnor(TREAS_GOLD_ALLOC, 3));
  alloc_obj(set_floor, 1, randint(alloc_level));

  alloc_mon((randint(RND_MALLOC_LEVEL) + MIN_MALLOC_LEVEL + alloc_level), 0,
            TRUE);
  if (dun_level >= WIN_MON_APPEAR) place_win_monster();
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
  cflag |= CF_SEEN;
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
  place_stair_tval(i, j, TV_DOWN_STAIR);
  caveD[i][j].cflag |= CF_FIELDMARK;
  rnd_seed = seed;

  alloc_townmon(randint(RND_MALLOC_LEVEL) + MIN_MALLOC_TOWN);

  do {
    i = randint(SYMMAP_HEIGHT - 2);
    j = randint(SYMMAP_WIDTH - 2);
    c_ptr = &caveD[i][j];
  } while (c_ptr->fval >= MIN_CLOSED_SPACE || (c_ptr->oidx != 0) ||
           (c_ptr->midx != 0));
  uD.y = i;
  uD.x = j;
}
void
hard_reset()
{
  // Clear game state
  memset(__start_game, 0, __stop_game - __start_game);

  // Message history
  AC(msglen_cqD);
  AC(msg_cqD);
  msg_writeD = 0;

  // Replay state
  input_record_writeD = input_record_readD = 0;

  // Reset overlay modes
  overlay_submodeD = 0;
  screen_submodeD = 0;
}
BOOL
panel_contains(panel, y, x)
struct panelS* panel;
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
static void get_moves(midx, mm) int* mm;
{
  int y, ay, x, ax, move_val;

  y = entity_monD[midx].fy - uD.y;
  x = entity_monD[midx].fx - uD.x;
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
static int
see_wall(dir, y, x)
{
  if (!mmove(dir, &y, &x)) /* check to see if movement there possible */
    return TRUE;
  else if (caveD[y][x].fval >= MIN_WALL && CF_VIZ & caveD[y][x].cflag)
    return TRUE;
  else
    return FALSE;
}
void find_init(dir, y_ptr, x_ptr) int *y_ptr, *x_ptr;
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
    if (py_affect(MA_BLIND) == 0) {
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
{
  if (!mmove(dir, &y, &x)) /* check to see if movement there possible */
    return FALSE;
  else if ((CF_VIZ & caveD[y][x].cflag) == 0)
    return TRUE;
  else
    return FALSE;
}
int
find_event(y, x)
{
  int dir, newdir, t, check_dir, row, col;
  int i, max, option, option2;
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
      c_ptr = &caveD[row][col];

      /* Objects player can see an object causing a stop */
      if (c_ptr->oidx != 0) {
        t = entity_objD[c_ptr->oidx].tval;
        if (t == TV_INVIS_TRAP || t == TV_SECRET_DOOR || t == TV_OPEN_DOOR) {
        } else if (t == TV_GOLD && (c_ptr->fval >= MIN_CLOSED_SPACE &&
                                    (CF_FIELDMARK & c_ptr->cflag) == 0)) {
        } else {
          return 1;
        }
      }

      /* Detect adjacent visible monsters that may not otherwise disturb */
      if (c_ptr->midx != 0) {
        if (entity_monD[c_ptr->midx].mlit) {
          return 1;
        }
      }

      if (c_ptr->fval <= MAX_OPEN_SPACE) {
        if (find_openarea) {
          /* Have we found a break? */
          if (i < 0) {
            if (find_breakright) {
              return 1;
            }
          } else if (i > 0) {
            if (find_breakleft) {
              return 1;
            }
          }
        } else if (option == 0)
          option = newdir; /* The first new direction. */
        else if (option2 != 0) {
          return 1; /* Three new directions. STOP. */
        } else if (option != cycle[chome[dir] + i - 1]) {
          return 1; /* If not adjacent to prev, STOP */
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
            return 1;
          }
          find_breakright = TRUE;
        } else if (i > 0) {
          if (find_breakright) {
            return 1;
          }
          find_breakleft = TRUE;
        }
      }
    }
  }

  if (find_openarea == FALSE) { /* choose a direction. */
    if (option2 == 0) {
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
        if (find_threat) {
          /* STOP: the player can make a choice */
          return 1;
        } else {
          /* Don't see that it is closed off.  This could be a
             potential corner or an intersection. */
          if (see_nothing(option, row, col) && see_nothing(option2, row, col))
          /* Can not see anything ahead and in the direction we are
             turning, assume that it is a potential corner. */
          {
            find_direction = option;
            find_prevdir = option2;
          } else {
            /* STOP: we are next to an intersection or a room */
            return 1;
          }
        }
      } else {
        /* This corner is seen to be enclosed; we cut the corner. */
        find_direction = option2;
        find_prevdir = option2;
      }
    }
  }

  return 0;
}
int
detect_obj(int (*valid)(), int known)
{
  int detect;
  struct caveS* c_ptr;
  int py, px;

  py = uD.y;
  px = uD.x;
  detect = FALSE;
  FOR_EACH(obj, {
    if (distance(py, px, obj->fy, obj->fx) < 32 && valid(obj)) {
      c_ptr = &caveD[obj->fy][obj->fx];

      // Gold is fieldmarked too, affecting auto-run
      if (obj->tval >= TV_MAX_PICK_UP) {
        if ((obj->idflag & ID_REVEAL) == 0) {
          detect = TRUE;
          obj->idflag |= ID_REVEAL;
          if (obj->tval == TV_INVIS_TRAP) {
            obj->tval = TV_VIS_TRAP;
            obj->tchar = '^';
          } else if (obj->tval == TV_SECRET_DOOR) {
            obj->tval = TV_CLOSED_DOOR;
            obj->tchar = '+';
          }
          if (obj->tval != TV_CHEST || obj->flags & CH_TRAPPED)
            c_ptr->cflag |= CF_FIELDMARK;
        }
      } else if (obj->tval < TV_MAX_PICK_UP) {
        if ((CF_LIT & c_ptr->cflag) == 0) {
          detect = TRUE;
          c_ptr->cflag |= CF_TEMP_LIGHT;
        }
      }
    }
  });

  if (detect) {
    msg_print("Your senses tingle!");
  } else if (known) {
    msg_print("You detect nothing further.");
  }

  return (detect);
}
int
detect_mon(int (*valid)(), int known)
{
  struct creatureS* cr_ptr;
  int flag;

  flag = FALSE;
  FOR_EACH(mon, {
    if (panel_contains(&panelD, mon->fy, mon->fx)) {
      cr_ptr = &creatureD[mon->cidx];
      if (!mon->mlit) {
        if (valid(cr_ptr)) {
          mon->mlit = TRUE;
          flag = TRUE;
        }
      }
    }
  });

  if (flag) {
    msg_print("Your senses tingle!");
  } else if (known) {
    msg_print("You detect nothing further.");
  }

  return flag;
}
void
move_rec(y1, x1, y2, x2)
{
  int tmp = caveD[y1][x1].midx;
  caveD[y1][x1].midx = 0;
  caveD[y2][x2].midx = tmp;
}
static int
cr_seen(cr_ptr)
struct creatureS* cr_ptr;
{
  if ((CM_INVISIBLE & cr_ptr->cmove) == 0)
    return TRUE;
  else if (py_tr(TR_SEE_INVIS))
    return TRUE;
  else
    return FALSE;
}
void
update_mon(midx)
{
  int flag, fy, fx, cdis, infra;
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;

  m_ptr = &entity_monD[midx];
  cr_ptr = &creatureD[m_ptr->cidx];
  flag = FALSE;
  fy = m_ptr->fy;
  fx = m_ptr->fx;
  if (py_affect(MA_DETECT_EVIL) && (CD_EVIL & cr_ptr->cdefense)) {
    flag = TRUE;
  } else if (py_affect(MA_DETECT_MON) &&
             ((CM_INVISIBLE & cr_ptr->cmove) == 0)) {
    flag = TRUE;
  } else if (py_affect(MA_DETECT_INVIS) && (CM_INVISIBLE & cr_ptr->cmove)) {
    flag = TRUE;
  } else if (maD[MA_BLIND] == 0) {
    infra = (CD_INFRA & cr_ptr->cdefense);
    cdis = distance(uD.y, uD.x, fy, fx);
    if (cdis <= MAX_SIGHT && los(uD.y, uD.x, fy, fx)) {
      c_ptr = &caveD[fy][fx];
      if (infra && (cdis <= uD.infra)) {
        flag = TRUE;
      } else if (CF_LIT & c_ptr->cflag) {
        flag = cr_seen(cr_ptr);
      }
    }
  }

  m_ptr->mlit = flag;
}
static int
mon_multiply(mon)
struct monS* mon;
{
  int y, x, fy, fx, i, j, k, midx;
  struct caveS* c_ptr;
  int eats_others;
  int mexp;

  eats_others = creatureD[mon->cidx].cmove & CM_EATS_OTHER;
  mexp = creatureD[mon->cidx].mexp;
  y = uD.y;
  x = uD.x;
  fy = mon->fy;
  fx = mon->fx;
  i = 0;
  do {
    j = fy - 2 + randint(3);
    k = fx - 2 + randint(3);
    // don't create a new creature on top of the old one
    // don't create a creature on top of the player
    if ((j != fy || k != fx) && (j != y || k != x)) {
      c_ptr = &caveD[j][k];
      if (c_ptr->fval <= MAX_OPEN_SPACE) {
        if (eats_others) {
          if (mexp >= creatureD[c_ptr->midx].mexp) {
            mon_unuse(&entity_monD[c_ptr->midx]);
            c_ptr->midx = 0;
          }
        }
        if (c_ptr->midx == 0) {
          midx = place_monster(j, k, mon->cidx, FALSE);
          if (midx) update_mon(midx);
          return midx;
        }
      }
    }
    i++;
  } while (i <= 18);
  return FALSE;
}
void
light_room(y, x)
{
  int i, j, start_col, end_col;
  int start_row, end_row;
  struct caveS* c_ptr;

  start_row = (y / CHUNK_HEIGHT) * CHUNK_HEIGHT;
  start_col = (x / CHUNK_WIDTH) * CHUNK_WIDTH;
  end_row = start_row + CHUNK_HEIGHT;
  end_col = start_col + CHUNK_WIDTH;
  for (i = start_row; i < end_row; i++)
    for (j = start_col; j < end_col; j++) {
      c_ptr = &caveD[i][j];
      if ((c_ptr->cflag & CF_ROOM)) {
        c_ptr->cflag |= CF_PERM_LIGHT | CF_SEEN;
        if (c_ptr->fval == FLOOR_DARK) c_ptr->fval = FLOOR_LIGHT;
        if (c_ptr->oidx && oset_sightfm(&entity_objD[c_ptr->oidx]))
          c_ptr->cflag |= CF_FIELDMARK;
      }
    }
}
int
illuminate(y, x)
{
  if (caveD[y][x].cflag & CF_ROOM) light_room(y, x);
  for (int col = y - 1; col <= y + 1; ++col) {
    for (int row = x - 1; row <= x + 1; ++row) {
      caveD[col][row].cflag |= (CF_PERM_LIGHT | CF_SEEN);
    }
  }

  see_print("You are surrounded by a white light.");

  return py_affect(MA_BLIND) == 0;
}
void
unlight_room(y, x)
{
  int i, j, start_col, end_col;
  int start_row, end_row;
  struct caveS* c_ptr;

  start_row = (y / CHUNK_HEIGHT) * CHUNK_HEIGHT;
  start_col = (x / CHUNK_WIDTH) * CHUNK_WIDTH;
  end_row = start_row + CHUNK_HEIGHT;
  end_col = start_col + CHUNK_WIDTH;
  for (i = start_row; i < end_row; i++)
    for (j = start_col; j < end_col; j++) {
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

  if (known) see_print("Darkness surrounds you.");

  return known;
}
static void
py_light_off(y, x)
{
  int row, col;
  for (row = y - 1; row <= y + 1; ++row) {
    for (col = x - 1; col <= x + 1; ++col) {
      caveD[row][col].cflag &= ~CF_TEMP_LIGHT;
    }
  }
}
static void
py_light_on(y, x)
{
  int row, col;
  for (row = y - 1; row <= y + 1; ++row) {
    for (col = x - 1; col <= x + 1; ++col) {
      struct caveS* cave = &caveD[row][col];
      uint32_t cflag = cave->cflag | CF_SEEN;

      if (cave->fval >= MIN_WALL)
        cflag |= (CF_PERM_LIGHT);
      else
        cflag |= (CF_TEMP_LIGHT);
      if (cave->oidx && oset_sightfm(&entity_objD[cave->oidx]))
        cflag |= CF_FIELDMARK;
      cave->cflag = cflag;
    }
  }

  if (near_light(y, x)) light_room(y, x);
}
void
py_check_view(full)
{
  if (py_affect(MA_BLIND)) {
    py_light_off(uD.y, uD.x);
  } else {
    py_light_on(uD.y, uD.x);
  }

  if (full) FOR_EACH(mon, { update_mon(it_index); });
}
int
enchant(int16_t* bonus, int16_t limit)
{
  int chance, res;

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
tohit_by_weight(weight)
{
  int use_weight = statD.use_stat[A_STR] * 15;

  if (use_weight < weight) return use_weight - weight;
  return 0;
}
int
attack_blows(weight)
{
  int adj_weight;
  int str_index, dex_index, s, d;

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
      switch (randint(6)) {
        case 1:
          return ((" insults you!"));
        case 2:
          return ((" insults your mother!"));
        case 3:
          return ((" gives you the finger!"));
        case 4:
          return ((" humiliates you!"));
        case 5:
          return ((" dances around you!"));
        case 6:
          return ((" makes obscene gestures!"));
      }
    case 99:
      return ((" is repelled."));
  }
  return " hits you.";
}
uint32_t
sustain_stat(sidx)
{
  uint32_t val = sidx;
  return ((1 << val) | TR_SUST_STAT);
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
tohit_adj()
{
  int total, stat;

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
  int stat;

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
  int stat;

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
  int stat;

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
umana_by_level(level)
{
  int splev, tadj, sptype;
  sptype = classD[uD.clidx].spell;
  if (sptype) {
    splev = level - classD[uD.clidx].first_spell_lev + 1;
    if (splev > 0) {
      tadj = think_adj(sptype == SP_MAGE ? A_INT : A_WIS);
      if (tadj > 0) {
        tadj = 1 + CLAMP(tadj - 1, 1, 6);
        return 1 + tadj * splev / 2;
      }
    }
  }
  return 0;
}
int
usave()
{
  return uD.save + think_adj(A_WIS) +
         (level_adj[uD.clidx][LA_SAVE] * uD.lev / 3);
}
int
udevice()
{
  int xdev = uD.save + think_adj(A_INT) +
             (level_adj[uD.clidx][LA_DEVICE] * uD.lev / 3);

  if (countD.confusion) xdev /= 2;
  return xdev;
}
int
udisarm()
{
  int xdis = uD.disarm + 2 * todis_adj() + think_adj(A_INT) +
             level_adj[uD.clidx][LA_DISARM] * uD.lev / 3;
  if (py_affect(MA_BLIND)) xdis /= 8;
  if (countD.confusion) xdis /= 8;
  return xdis;
}
int
gain_prayer()
{
  int spcount = uspellcount();
  struct spellS* spelltable;
  uint32_t knowable, known;
  int open[32], gain[32];
  int open_count, gain_count;
  int idx;

  known = 0;
  open_count = 0;
  for (int it = 0; it < spcount; ++it) {
    if (spell_orderD[it])
      known |= (1 << (spell_orderD[it] - 1));
    else
      open[open_count++] = it;
  }

  if (open_count) {
    knowable = 0;
    spelltable = uspelltable();
    for (int it = 0; it < AL(spellD[0]); ++it) {
      if (spelltable[it].splevel <= uD.lev) knowable |= (1 << it);
    }

    knowable &= ~known;
    gain_count = 0;
    while (knowable) {
      gain[gain_count++] = bit_pos(&knowable);
    }

    while (open_count && gain_count) {
      idx = randint(gain_count) - 1;
      open_count -= 1;
      spell_orderD[open[open_count]] = 1 + gain[idx];
      MSG("You believe in the prayer %s.", prayer_nameD[gain[idx]]);
      gain[idx] = gain[gain_count - 1];
      gain_count -= 1;
    }
  }
  return 0;
}
int
test_hit(bth, level_adj, pth, ac)
{
  int i, die;

  i = bth + pth * BTH_PLUS_ADJ + level_adj;

  // pth could be less than 0 if player wielding weapon too heavy for him
  // bth can be less than 0 for creatures
  // always miss 1 out of 20, always hit 1 out of 20
  die = randint(20);
  if (HACK || (die != 1) && ((die == 20) || ((i > 0) && (randint(i) > ac))))
    return TRUE;
  else
    return FALSE;
}
BOOL
is_a_vowel(chr)
{
  char c = chr | 0x20;
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
void
desc_fixup(number)
{
  char obj_name[AL(descD) - 8];

  int offset = 0;
  for (int it = 0; it < AL(descD); ++it) {
    if (descD[it] != '~')
      obj_name[it - offset] = descD[it];
    else if (number != 1)
      obj_name[it - offset] = 's';
    else
      offset += 1;
    if (descD[it] == 0) break;
  }

  /* ampersand is always the first character */
  if (obj_name[0] == '&') {
    /* use &obj_name[1], so that & does not appear in output */
    if (number > 1)
      snprintf(descD, AL(descD), "%d%s", number, &obj_name[1]);
    else if (number < 1)
      snprintf(descD, AL(descD), "%s%s", "no more", &obj_name[1]);
    else if (is_a_vowel(obj_name[2]))
      snprintf(descD, AL(descD), "an%s", &obj_name[1]);
    else
      snprintf(descD, AL(descD), "a%s", &obj_name[1]);
  }
  /* handle 'no more' case specially */
  else if (number < 1) {
    /* check for "some" at start */
    snprintf(descD, AL(descD), "no more %s", obj_name);
  } else
    strcpy(descD, obj_name);
}
void obj_detail(obj) struct objS* obj;
{
  char tmp_str[80];
  int eqidx, reveal;

  memset(detailD, 0x20202020, AL(detailD));

  eqidx = may_equip(obj->tval);
  reveal = (obj->idflag & ID_REVEAL) != 0;
  tmp_str[0] = 0;
  detailD[0] = 0;

  if (reveal) {
    if (eqidx > INVEN_WIELD && TR_CURSED & obj->flags)
      strcat(detailD, " {cursed}");

    if ((obj->tval == TV_STAFF || obj->tval == TV_WAND)) {
      snprintf(tmp_str, AL(tmp_str), " (%d charges)", obj->p1);
      strcat(detailD, tmp_str);
    }

    if (oset_tohitdam(obj)) {
      snprintf(tmp_str, AL(tmp_str), " (%+d,%+d)", obj->tohit, obj->todam);
      strcat(detailD, tmp_str);
    }
  } else {
    if (obj->idflag & ID_MAGIK) strcat(detailD, " {magik}");
    if (obj->idflag & ID_DAMD) strcat(detailD, " {cursed}");
    if (obj->idflag & ID_EMPTY) strcat(detailD, " {empty}");
    if (obj->idflag & ID_CORRODED) strcat(detailD, " {corroded}");
    if (obj->idflag & ID_PLAIN) strcat(detailD, " {plain}");
    if (obj->idflag & ID_RARE) strcat(detailD, " {rare}");
  }

  if (obj->tval == TV_PROJECTILE) {
    snprintf(tmp_str, AL(tmp_str), " (%dd%d)", obj->damage[0], obj->damage[1]);
    strcat(detailD, tmp_str);
  } else if (obj->tval == TV_LAUNCHER) {
    snprintf(tmp_str, AL(tmp_str), " (%dx)", obj->damage[1]);
    strcat(detailD, tmp_str);
  } else if (eqidx == INVEN_WIELD) {
    snprintf(tmp_str, AL(tmp_str), " (%dx %dd%d)", attack_blows(obj->weight),
             obj->damage[0], obj->damage[1]);
    strcat(detailD, tmp_str);
  } else if (eqidx > INVEN_WIELD) {
    if (reveal && (oset_armor(obj) || obj->toac)) {
      snprintf(tmp_str, AL(tmp_str), " [%d%+d AC]", obj->ac, obj->toac);
      strcat(detailD, tmp_str);
    } else if (oset_armor(obj)) {
      snprintf(tmp_str, AL(tmp_str), " [%d AC]", obj->ac);
      strcat(detailD, tmp_str);
    }
  }
}
void obj_desc(obj, number) struct objS* obj;
{
  char* name;
  char* suffix;
  char* sample;
  int indexx, unknown, p1;
  struct treasureS* tr_ptr;

  memset(descD, 0x20202020, AL(descD));
  tr_ptr = &treasureD[obj->tidx];
  name = tr_ptr->name;

  suffix = 0;
  p1 = 0;
  tr_unknown_sample(tr_ptr, &unknown, &sample);
  if (obj->idflag & ID_REVEAL) {
    unknown = 0;
    suffix = special_nameD[obj->sn];
    p1 = obj->p1;
  }
  indexx = mask_subval(obj->subval);
  switch (obj->tval) {
    case TV_MISC:
    case TV_CHEST:
      break;
    case TV_SPIKE:
      break;
    case TV_PROJECTILE:
      break;
    case TV_LAUNCHER:
      break;
    case TV_LIGHT:
      break;
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
      break;
    case TV_DIGGING:
      if (!unknown) suffix = "digging";
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
        name = 0;
      } else {
        name = "& Amulet";
        suffix = tr_ptr->name;
      }
      break;
    case TV_RING:
      if (unknown) {
        name = 0;
        snprintf(descD, AL(descD), "& %s Ring", rocks[indexx]);
      } else {
        name = "& Ring";
        suffix = tr_ptr->name;
      }
      break;
    case TV_STAFF:
      if (unknown) {
        name = 0;
        snprintf(descD, AL(descD), "& %s Staff%s", woods[indexx], sample);
      } else {
        name = "& Staff";
        suffix = tr_ptr->name;
        p1 = 0;
      }
      break;
    case TV_WAND:
      if (unknown) {
        name = 0;
        snprintf(descD, AL(descD), "& %s Wand%s", metals[indexx], sample);
      } else {
        name = "& Wand";
        suffix = tr_ptr->name;
        p1 = 0;
      }
      break;
    case TV_SCROLL1:
    case TV_SCROLL2:
      if (unknown) {
        name = 0;
        snprintf(descD, AL(descD), "& Scroll~ titled \"%s\"%s", titleD[indexx],
                 sample);
      } else {
        name = "& Scroll~";
        suffix = tr_ptr->name;
      }
      break;
    case TV_POTION1:
    case TV_POTION2:
      if (unknown) {
        name = 0;
        snprintf(descD, AL(descD), "& %s Potion~%s", colors[indexx], sample);
      } else {
        name = "& Potion~";
        suffix = tr_ptr->name;
        p1 = 0;
      }
      break;
    case TV_FLASK:
      break;
    case TV_FOOD:
      if (indexx <= 20) {
        if (unknown) {
          if (indexx <= 15)
            snprintf(descD, AL(descD), "& %s Mushroom~%s", mushrooms[indexx],
                     sample);
          else if (indexx <= 20)
            snprintf(descD, AL(descD), "& Hairy %s Mold~%s", mushrooms[indexx],
                     sample);
          name = 0;
        } else {
          suffix = tr_ptr->name;
          p1 = 0;
          if (indexx <= 15)
            name = "& Mushroom~";
          else if (indexx <= 20)
            name = "& Hairy Mold~";
        }
      }
      break;
    case TV_MAGIC_BOOK:
      name = "& Book~";
      suffix = tr_ptr->name;
      break;
    case TV_PRAYER_BOOK:
      name = "& Book~";
      suffix = tr_ptr->name;
      break;
    case TV_GOLD:
      name = gold_nameD[indexx];
      break;
    case TV_OPEN_DOOR:
      name = "& open door";
      break;
    case TV_CLOSED_DOOR:
      name = "& closed door";
      break;
    case TV_RUBBLE:
      name = "rubble";
      break;
    case TV_SECRET_DOOR:
      descD[0] = 0;
      return;
    case TV_UP_STAIR:
      name = "& staircase up";
      break;
    case TV_DOWN_STAIR:
      name = "& staircase down";
      break;
    case TV_VIS_TRAP:
    case TV_GLYPH:
      break;
    case TV_PAWN_DOOR:
      name = "& pawn shop";
      break;
    case TV_STORE_DOOR:
      name = "& store entrance";
      break;
    default:
      snprintf(descD, AL(descD), "Error in objdes(): %d", obj->tval);
      return;
  }
  if (suffix) {
    if (p1)
      snprintf(descD, AL(descD), "%s of %s (%+d)", name, suffix, p1);
    else
      snprintf(descD, AL(descD), "%s of %s", name, suffix);
  } else if (name) {
    strcpy(descD, name);
  }

  desc_fixup(number);
}
static void
mon_desc(midx)
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
// TBD: rewrite
static void
summon_object(y, x, num, typ)
{
  int i, j, k;
  int py, px;
  struct caveS* c_ptr;
  int real_typ;

  py = uD.y;
  px = uD.x;

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

          if (j == py && k == px) {
            msg_print("You feel something roll beneath your feet.");
          }
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

  if (flags & CM_WIN) {
    if (uD.new_level_flag == 0) /* maybe the player died in mid-turn */
    {
      total_winner = TRUE;
      msg_print("*** CONGRATULATIONS *** You have won the game.");
    }
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
void
calc_hitpoints()
{
  int level;
  int hitpoints;

  level = uD.lev;
  hitpoints = player_hpD[level - 1] + (con_adj() * level);
  /* always give at least one point per level + 1 */
  if (hitpoints < (level + 1)) hitpoints = level + 1;

  if (py_affect(MA_HERO)) hitpoints += 10;
  if (py_affect(MA_SUPERHERO)) hitpoints += 20;
  if (HACK) hitpoints += 1000;

  // Scale current hp to the new maximum
  int value = ((uD.chp << 16) + uD.chp_frac) / uD.mhp * hitpoints;
  uD.chp = value >> 16;
  uD.chp_frac = value & 0xFFFF;
  uD.mhp = hitpoints;
}
void
calc_mana()
{
  int new_mana = umana_by_level(uD.lev);
  struct spellS* spelltable;
  int sptype, chance;

  if (uD.mmana != new_mana) {
    // Scale current mana to the new maximum
    if (uD.mmana) {
      int value = ((uD.cmana << 16) + uD.cmana_frac) / uD.mmana * new_mana;
      uD.cmana = value >> 16;
      uD.cmana_frac = value & 0xFFFF;
    } else {
      uD.cmana = new_mana;
      uD.cmana_frac = 0;
    }
    uD.mmana = new_mana;
  }

  sptype = classD[uD.clidx].spell;

  if (sptype) {
    if (sptype == SP_PRIEST) gain_prayer();

    spelltable = uspelltable();
    for (int it = 0; it < AL(spellD[0]); ++it) {
      chance = spelltable[it].spfail - 3 * (uD.lev - spelltable[it].splevel);
      chance -= 3 * (think_adj(sptype == SP_MAGE ? A_INT : A_WIS) - 1);
      spell_chanceD[it] = CLAMP(chance, 5, 95);
    }
  }
}
void
calc_bonuses()
{
  int tflag;
  int ac, toac;
  int wtohit, tohit, todam;
  int hide_tohit, hide_todam, hide_toac;

  wtohit = invenD[INVEN_WIELD]
               ? tohit_by_weight(obj_get(invenD[INVEN_WIELD])->weight)
               : 0;
  tohit = tohit_adj() + wtohit;
  todam = todam_adj();
  toac = toac_adj();
  ac = 0;
  hide_tohit = hide_todam = hide_toac = 0;
  tflag = 0;
  for (int it = INVEN_EQUIP; it < INVEN_EQUIP_END; it++) {
    struct objS* obj = obj_get(invenD[it]);
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
  toac += uD.ma_ac;
  if (py_affect(MA_SEE_INVIS)) tflag |= TR_SEE_INVIS;
  if (py_affect(MA_HERO) || py_affect(MA_SUPERHERO)) tflag |= TR_HERO;

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
    if (TR_SUST_STAT & obj->flags) {
      tflag |= (obj->flags & TR_STATS);
    }
  }
  cbD.tflag = tflag;

  if (cbD.weapon_heavy ^ (wtohit != 0)) {
    cbD.weapon_heavy = (wtohit != 0);
    if (wtohit) {
      msg_print("You have trouble wielding such a heavy weapon.");
    } else {
      msg_print("You are strong enough to wield your weapon.");
    }
  } else if (cbD.prev_weapon != invenD[INVEN_WIELD]) {
    if (wtohit) {
      msg_print("You have trouble wielding such a heavy weapon.");
    }
  }
  cbD.prev_weapon = invenD[INVEN_WIELD];
}
// uD.mflags && maD are NOT up-to-date at the time of this call
void
ma_bonuses(maffect, factor)
{
  switch (maffect) {
    case MA_BLESS:
      if (factor > 0)
        msg_print("You feel righteous!");
      else if (factor < 0)
        msg_print("The prayer has expired.");
      uD.ma_ac += factor * 2;
      uD.bth += factor * 5;
      uD.bowth += factor * 5;
      break;
    case MA_HERO:
      if (factor > 0)
        msg_print("You feel like a HERO!");
      else if (factor < 0)
        msg_print("The heroism wears off.");
      uD.chp += (factor > 0) * 10;
      uD.mhp += factor * 10;
      uD.bth += factor * 12;
      uD.bowth += factor * 12;
      break;
    case MA_SUPERHERO:
      if (factor > 0)
        msg_print("You feel like a SUPER-HERO!");
      else if (factor < 0)
        msg_print("The super heroism wears off.");
      uD.chp += (factor > 0) * 20;
      uD.mhp += factor * 20;
      uD.bth += factor * 24;
      uD.bowth += factor * 24;
      break;
    case MA_FAST:
      if (factor > 0)
        msg_print("You feel yourself moving faster.");
      else if (factor < 0)
        msg_print("You feel yourself slow down.");
      break;
    case MA_SLOW:
      if (factor > 0)
        msg_print("You feel yourself moving slower.");
      else if (factor < 0)
        msg_print("You feel yourself speed up.");
      break;
    case MA_AFIRE:
      if (factor > 0)
        msg_print("You are shielded from flame.");
      else if (factor < 0)
        msg_print("You no longer feel safe from flame.");
      break;
    case MA_AFROST:
      if (factor > 0)
        msg_print("You are shielded from frost.");
      else if (factor < 0)
        msg_print("You no longer feel safe from frost.");
      break;
    case MA_INVULN:
      if (factor > 0)
        msg_print("Your skin turns into steel!");
      else if (factor < 0)
        msg_print("Your skin returns to normal.");
      uD.ma_ac += factor * 100;
      break;
    case MA_SEE_INVIS:
      break;
    case MA_SEE_INFRA:
      uD.infra += factor * 3;
      break;
    case MA_BLIND:
      if (factor < 0) msg_print("The veil of darkness lifts.");
      break;
    case MA_FEAR:
      if (factor < 0) msg_print("You feel bolder now.");
      break;
    case MA_DETECT_MON:
      break;
    case MA_DETECT_EVIL:
      break;
    case MA_DETECT_INVIS:
      break;
    case MA_RECALL:
      if (factor < 0) {
        if (dun_level) {
          dun_level = 0;
        } else {
          dun_level = uD.max_dlv;
        }
        uD.new_level_flag = NL_RECALL;
        countD.paralysis += 1;
      }
      break;
    default:
      msg_print("Error in ma_bonuses()");
      break;
  }
}
// Effects are ticked twice per turn (rising/falling edge)
static void
add_ma_count(maidx, count)
{
  if (maidx == MA_BLIND && py_tr(TR_SEEING)) {
    msg_print("Your sight is no worse.");
    count = 0;
  } else if (maidx == MA_FEAR && (py_tr(TR_HERO) || HACK)) {
    msg_print("A hero recovers quickly.");
    count = 0;
  } else if (maidx == MA_HERO || maidx == MA_SUPERHERO) {
    maD[MA_FEAR] = 0;
  } else if (maidx == MA_DETECT_MON || maidx == MA_DETECT_EVIL ||
             maidx == MA_DETECT_INVIS) {
    // Falling edge expiration to allow the player one action with detection
    count += (maD[maidx] % 2 == 0);
  }

  maD[maidx] += count;
}
static void
ma_duration(maidx, nturn)
{
  add_ma_count(maidx, nturn * 2);
}
// Combat affects result in an odd tick count
// Such that the player recovers from an affect before their turn
static void
ma_combat(maidx, nturn)
{
  int tick_count;
  tick_count = maD[maidx];
  add_ma_count(maidx, nturn * 2 + (tick_count % 2 == 0));
}
static int
ma_clear(maidx)
{
  int turn;
  turn = maD[maidx];
  maD[maidx] = 0;
  return turn != 0;
}
static int8_t
modify_stat(tmp_stat, amount)
{
  int loop, i;

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
void
set_use_stat(stat)
{
  statD.use_stat[stat] =
      modify_stat(statD.cur_stat[stat], statD.mod_stat[stat]);

  if (stat == A_STR) {
    calc_bonuses();
  } else if (stat == A_DEX) {
    calc_bonuses();
  } else if (stat == A_CON) {
    calc_hitpoints();
  } else if (stat == A_INT || stat == A_WIS) {
    calc_mana();
  }
}
void py_bonuses(obj, factor) struct objS* obj;
{
  int amount;

  if (obj->sn == SN_BLINDNESS && factor > 0) ma_duration(MA_BLIND, 1000);
  if (obj->sn == SN_TIMIDNESS && factor > 0) ma_duration(MA_FEAR, 50);
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
  if (obj->sn == SN_INFRAVISION) uD.infra += amount;
  if (obj->sn == SN_LORDLINESS) uD.save += (amount * 10);
}
int
equip_swap_into(iidx, into_slot)
{
  struct objS* obj;
  obj = obj_get(invenD[iidx]);
  if (obj->flags & TR_CURSED) {
    MSG("Hmm, the item you are %s seems to be cursed.", describe_use(iidx));
  } else {
    if (into_slot >= 0) {
      invenD[into_slot] = obj->id;
      obj_desc(obj, 1);
      MSG("You take off %s (%c).", descD, 'a' + into_slot);
    }
    invenD[iidx] = 0;

    if (iidx != INVEN_AUX) {
      py_bonuses(obj, -1);
      calc_bonuses();
    }
    turn_flag = TRUE;
    return TRUE;
  }

  return FALSE;
}
void
inven_drop(iidx)
{
  int y, x, ok;
  struct objS* obj;
  struct caveS* c_ptr;
  obj = obj_get(invenD[iidx]);

  if (obj->id) {
    y = uD.y;
    x = uD.x;

    if (caveD[y][x].oidx == 0)
      c_ptr = &caveD[y][x];
    else {
      for (int it = 1; it < 9; ++it) {
        int dir = it + (it >= 5);
        int i = dir_y(dir);
        int j = dir_x(dir);
        c_ptr = &caveD[y + i][x + j];
        if (c_ptr->fval <= MAX_OPEN_SPACE && c_ptr->oidx == 0) {
          y += i;
          x += j;
          it = 9;
        }
      }
    }

    if (c_ptr->fval <= MAX_OPEN_SPACE && c_ptr->oidx == 0) {
      if (iidx >= INVEN_EQUIP) {
        ok = equip_swap_into(iidx, -1);
      } else {
        ok = TRUE;
        invenD[iidx] = 0;
      }

      if (ok) {
        obj->fy = y;
        obj->fx = x;
        c_ptr->oidx = obj_index(obj);

        obj_desc(obj, obj->number);
        obj_detail(obj);
        MSG("You drop %s%s.", descD, detailD);
        turn_flag = TRUE;
      }
    } else {
      msg_print("There are too many objects on the ground here.");
    }
  }
}
BOOL
player_saves()
{
  return (randint(100) <= usave());
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
equip_vibrate(flag)
{
  for (int it = INVEN_EQUIP; it < INVEN_EQUIP_END; ++it) {
    struct objS* obj = obj_get(invenD[it]);
    if (obj->flags & flag) {
      obj_desc(obj, 1);
      descD[0] &= ~0x20;
      MSG("%s vibrates for a moment.", descD);
      return 1;
    }
  }
  return 0;
}
static int
equip_random()
{
  int slot[] = {
      INVEN_BODY, INVEN_ARM, INVEN_OUTER, INVEN_HANDS, INVEN_HEAD, INVEN_FEET,
  };
  int k;

  k = slot[randint(AL(slot) - 1)];
  return invenD[k] ? k : -1;
}
static int
equip_enchant(iidx, amount)
{
  struct objS* i_ptr;
  int affect;

  if (iidx >= 0) {
    i_ptr = obj_get(invenD[iidx]);
    if (may_enchant_ac(i_ptr->tval)) {
      obj_desc(i_ptr, 1);
      affect = 0;
      for (int it = 0; it < amount; ++it) {
        affect += (enchant(&i_ptr->toac, 10));
      }

      if (affect) {
        MSG("Your %s glows %s!", descD, affect > 1 ? "brightly" : "faintly");
        i_ptr->flags &= ~TR_CURSED;
        i_ptr->idflag &= ~ID_CORRODED;
        calc_bonuses();
      } else
        msg_print("The enchantment fails.");
      return TRUE;
    } else {
      msg_print("Invalid target.");
    }
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
    obj_desc(i_ptr, 1);
    MSG("Your %s glows black, fades.", descD);
    i_ptr->tohit = 0;
    i_ptr->todam = 0;
    i_ptr->toac = -randint(5) - randint(5);
    /* Must call py_bonuses() before set (clear) sn & flags, and
       must call calc_bonuses() after set (clear) sn & flags, so that
       all attributes will be properly turned off. */
    py_bonuses(i_ptr, -1);
    i_ptr->sn = 0;
    i_ptr->flags = TR_CURSED;
    calc_bonuses();
    return TRUE;
  }

  return FALSE;
}
static int
equip_remove_curse()
{
  int flag;
  struct objS* obj;

  flag = FALSE;
  for (int it = INVEN_EQUIP; it < MAX_INVEN; ++it) {
    obj = obj_get(invenD[it]);
    if (obj->flags & TR_CURSED) {
      flag = TRUE;
      obj->flags &= ~TR_CURSED;
    }
  }

  if (flag) msg_print("You feel as if someone is watching over you.");

  return flag;
}
static int
equip_disenchant()
{
  int flag, i;
  struct objS* obj;

  flag = FALSE;
  // INVEN_AUX is protected
  switch (randint(8)) {
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
    case 8:
      i = INVEN_LAUNCHER;
      break;
  }
  obj = obj_get(invenD[i]);

  // Weapons may lose tohit/todam. Armor may lose toac.
  // Ego weapon toac is protected.
  // Gauntlets of Slaying tohit/todam are protected.
  if (i == INVEN_WIELD) {
    if (obj->tohit > 0 || obj->todam > 0) {
      obj->tohit = MAX(obj->tohit - randint(2), 0);
      obj->todam = MAX(obj->todam - randint(2), 0);
      flag = TRUE;
    }
  } else {
    if (obj->toac > 0) {
      obj->toac = MAX(obj->toac - randint(2), 0);
      flag = TRUE;
    }
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
static void inven_used_obj(obj) struct objS* obj;
{
  obj->number -= 1;
  if (obj->number < 1) {
    for (int it = 0; it < INVEN_EQUIP; ++it) {
      if (invenD[it] == obj->id) invenD[it] = 0;
    }
    obj_unuse(obj);
  }
}
static int
inven_obj_mergecount(obj, number)
struct objS* obj;
{
  int tval, subval;
  int stackweight, stacklimit;

  stackweight = ustackweight();
  tval = obj->tval;
  subval = obj->subval;
  if (subval & STACK_ANY) {
    for (int it = 0; it < INVEN_EQUIP; ++it) {
      struct objS* i_ptr = obj_get(invenD[it]);
      if (tval == i_ptr->tval && subval == i_ptr->subval) {
        stacklimit = stacklimit_by_max_weight(stackweight, i_ptr->weight);
        if (number + i_ptr->number <= stacklimit) {
          return it;
        }
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

  used = FALSE;
  if (iidx >= 0) {
    obj = obj_get(invenD[iidx]);
    tr_ptr = &treasureD[obj->tidx];
    used |= tr_make_known(tr_ptr);
    if ((obj->idflag & ID_REVEAL) == 0) {
      used |= TRUE;
      obj->idflag = ID_REVEAL;
    }
    obj_desc(obj, obj->number);
    obj_detail(obj);
    if (iidx >= INVEN_EQUIP) {
      calc_bonuses();
    }
    MSG("%s%s.", descD, detailD);
  }
  return used;
}
static int
weapon_enchant(iidx, tohit, todam)
{
  int affect, limit;
  struct objS* i_ptr;

  limit = 0;
  if (iidx >= 0) {
    i_ptr = obj_get(invenD[iidx]);
    if (i_ptr->tval == TV_LAUNCHER)
      limit = 10;
    else if (may_equip(i_ptr->tval) == INVEN_WIELD)
      limit = i_ptr->damage[0] * i_ptr->damage[1];
  }

  if (limit) {
    affect = 0;
    for (int it = 0; it < tohit; ++it) {
      affect += (enchant(&i_ptr->tohit, 10));
    }
    for (int it = 0; it < todam; ++it) {
      affect += (enchant(&i_ptr->todam, limit));
    }

    if (affect) {
      obj_desc(i_ptr, 1);
      MSG("Your %s glows %s!", descD, affect > 1 ? "brightly" : "faintly");
      i_ptr->flags &= ~TR_CURSED;
      calc_bonuses();
    } else
      msg_print("The enchantment fails.");
  }

  return limit != 0;
}
static int
make_special_type(iidx, lev, weapon_enchant)
{
  int is_weapon;
  struct objS* obj;

  if (iidx >= 0) {
    obj = obj_get(invenD[iidx]);
    if (oset_rare(obj)) {
      is_weapon = (INVEN_WIELD == may_equip(obj->tval));

      if (is_weapon == weapon_enchant) {
        int tidx = obj->tidx;
        if (iidx >= INVEN_EQUIP) py_bonuses(obj, -1);
        do {
          tr_obj_copy(tidx, obj);
          magic_treasure(obj, lev);
        } while (!obj->sn || obj->cost <= 0);
        obj->idflag = ID_REVEAL;
        if (iidx >= INVEN_EQUIP) py_bonuses(obj, 1);

        obj_desc(obj, 1);
        MSG("Your %s glows brightly.", descD);

        return TRUE;
      }
    }
  }

  return FALSE;
}
int
rest_affect()
{
  return py_affect(MA_BLIND) + countD.confusion + py_affect(MA_FEAR) +
         py_affect(MA_RECALL);
}
static void
py_rest()
{
  if (uD.chp < uD.mhp || rest_affect())
    countD.rest = 9999;
  else
    countD.rest = -9999;
}
static int
py_take_hit(damage)
{
  int death_blow;

  death_blow = (uD.chp - damage < 0);
  uD.chp -= damage;

  if (death_blow) uD.new_level_flag = NL_DEATH;
  return death_blow;
}
static void py_stats(stats, len) int8_t* stats;
{
  int i, tot;
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
int
social_bonus()
{
  int hist_ptr = uD.ridx * 3 + 1;
  int social_bonus = 0;
  int cur_ptr = 0;
  AC(historyD);
  do {
    int flag = 0;
    do {
      if (backgroundD[cur_ptr].chart == hist_ptr) {
        int test_roll = randint(100);
        while (test_roll > backgroundD[cur_ptr].roll) cur_ptr++;
        struct backgroundS* b_ptr = &backgroundD[cur_ptr];
        strcat(historyD, b_ptr->info);
        social_bonus += b_ptr->bonus - 50;
        if (hist_ptr > b_ptr->next) cur_ptr = 0;
        hist_ptr = b_ptr->next;
        flag = 1;
      } else
        cur_ptr++;
    } while (!flag);
  } while (hist_ptr >= 1);

  return social_bonus;
}
int
value_by_stat(stat)
{
  return 5 * (stat - 10);
}
void
py_gold_init()
{
  int roll_value = 0;
  for (int it = 0; it < A_CHR; ++it) {
    roll_value += value_by_stat(statD.cur_stat[it]);
  }

  int gold = value_by_stat(statD.cur_stat[A_CHR]) + uD.sc * 6 + 325;
  gold -= roll_value;
  if (!uD.male) gold += 50;

  while (gold < 80) {
    gold += randint(32);
  }

  uD.gold = gold;
}
void
py_race_class_seed_init(rsel, csel, prng)
{
  int hitdie;
  int8_t stat[MAX_A];

  seed_init(prng);

  py_stats(stat, AL(stat));

  // Race & class
  struct raceS* r_ptr = &raceD[rsel];
  for (int it = 0; it < MAX_A; ++it) {
    stat[it] += r_ptr->attr[it];
  }

  uD.male = 1 - randint(2);
  uD.age = r_ptr->b_age + randint(r_ptr->m_age);

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
  if (uD.male) {
    uD.ht = randnor(r_ptr->m_b_ht, r_ptr->m_m_ht);
    uD.wt = randnor(r_ptr->m_b_wt, r_ptr->m_m_wt);
  } else {
    uD.ht = randnor(r_ptr->f_b_ht, r_ptr->f_m_ht);
    uD.wt = randnor(r_ptr->f_b_wt, r_ptr->f_m_wt);
  }
  uD.bowth = r_ptr->bthb;

  struct classS* cl_ptr = &classD[csel];
  hitdie += cl_ptr->adj_hd;
  uD.clidx = csel;
  uD.bth += cl_ptr->mbth;
  uD.search += cl_ptr->msrh;
  uD.fos += cl_ptr->mfos;
  uD.disarm += cl_ptr->mdis;
  uD.stealth += cl_ptr->mstl;
  uD.save += cl_ptr->msav;
  uD.mult_exp += cl_ptr->m_exp;
  uD.bowth += cl_ptr->mbthb;

  for (int it = 0; it < MAX_A; ++it) {
    stat[it] += cl_ptr->mattr[it];
  }
  for (int it = 0; it < MAX_A; ++it) {
    stat[it] = MAX(3, stat[it]);
  }

  memcpy(statD.max_stat, AP(stat));
  memcpy(statD.cur_stat, AP(stat));
  AC(statD.mod_stat);
  memcpy(statD.use_stat, AP(stat));

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

  uD.lev = 1;
  uD.max_dlv = 1;
  uD.food = 7500;
  uD.food_digest = 2;

  uD.mhp = uD.chp = hitdie + con_adj();
  uD.chp_frac = 0;
  uD.cmana = uD.mmana = umana_by_level(1);
  uD.cmana_frac = 0;

  int sc = fixed_seed_func(town_seed, social_bonus);
  uD.sc = CLAMP(randint(4) + sc, 1, 100);

  py_gold_init();
  calc_bonuses();
}
void
py_inven_init()
{
  int start_equip[] = {30, 87, 22, 0, 0};
  int class_equip[AL(classD)] = {221, 319, 323, 124, 343, 323};
  int race_equip[AL(raceD)] = {336, 81, 82, 341, 90, 78, 127, 128};
  int clidx = uD.clidx;
  int rsel = uD.ridx;
  start_equip[AL(start_equip) - 2] = class_equip[clidx];
  start_equip[AL(start_equip) - 1] = race_equip[rsel];
  int iidx = 0;
  for (int it = 0; it < AL(start_equip); ++it) {
    int tidx = start_equip[it];
    struct treasureS* tr_ptr = &treasureD[tidx];
    int eqidx = may_equip(tr_ptr->tval);

    struct objS* obj = obj_use();
    tr_make_known(tr_ptr);
    tr_obj_copy(tidx, obj);
    obj->idflag = ID_REVEAL;

    switch (tidx) {
      case 22:  // More Food rations
        obj->number = 5;
        break;
      case 341:
      case 343:
      case 336:
        magic_treasure(obj, 1);
        break;
    }

    obj->idflag |= ID_REVEAL;
    if (eqidx > 0 && invenD[eqidx] == 0)
      invenD[eqidx] = obj->id;
    else
      invenD[iidx++] = obj->id;
  }
}
static void*
ptr_xor(void* a, void* b)
{
  return (void*)((uint64_t)a ^ (uint64_t)b);
}
void sort(array, len) void* array;
{
  int i, j;
  void* swap;
  void** arr = array;
  for (i = 0; i < len; ++i) {
    for (j = i + 1; j < len; ++j) {
      if (arr[i] > arr[j]) {
        swap = ptr_xor(arr[j], arr[i]);
        arr[j] = ptr_xor(arr[j], swap);
        arr[i] = ptr_xor(arr[i], swap);
      }
    }
  }
}
int
magic_init()
{
  int i, j, k, h;
  void* tmp;

  store_init();

  /* The first 3 entries for colors are fixed, (slime & apple juice, water) */
  sort(&colors[3], AL(colors) - 3);
  for (i = 3; i < AL(colors); i++) {
    j = randint(AL(colors) - 3) + 2;
    tmp = colors[i];
    colors[i] = colors[j];
    colors[j] = tmp;
  }
  sort(AP(woods));
  for (i = 0; i < AL(woods); i++) {
    j = randint(AL(woods)) - 1;
    tmp = woods[i];
    woods[i] = woods[j];
    woods[j] = tmp;
  }
  sort(AP(metals));
  for (i = 0; i < AL(metals); i++) {
    j = randint(AL(metals)) - 1;
    tmp = metals[i];
    metals[i] = metals[j];
    metals[j] = tmp;
  }
  sort(AP(rocks));
  for (i = 0; i < AL(rocks); i++) {
    j = randint(AL(rocks)) - 1;
    tmp = rocks[i];
    rocks[i] = rocks[j];
    rocks[j] = tmp;
  }
  sort(AP(amulets));
  for (i = 0; i < AL(amulets); i++) {
    j = randint(AL(amulets)) - 1;
    tmp = amulets[i];
    amulets[i] = amulets[j];
    amulets[j] = tmp;
  }
  sort(AP(mushrooms));
  for (i = 0; i < AL(mushrooms); i++) {
    j = randint(AL(mushrooms)) - 1;
    tmp = mushrooms[i];
    mushrooms[i] = mushrooms[j];
    mushrooms[j] = tmp;
  }
  for (h = 0; h < AL(titleD); h++) {
    descD[0] = 0;
    k = randint(2) + 1;
    for (i = 0; i < k; i++) {
      for (j = randint(2); j > 0; j--)
        strcat(descD, syllableD[randint(AL(syllableD)) - 1]);
      if (i < k - 1) strcat(descD, " ");
    }
    if (descD[8] == ' ')
      descD[8] = '\0';
    else
      descD[9] = '\0';
    strcpy(titleD[h], descD);
  }
  return 0;
}
int
dec_stat(stat)
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
    MSG("%s!", stat_gainD[stat]);
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
    MSG("You feel your natural %s returning.", stat_nameD[stat]);
    return TRUE;
  }
  return FALSE;
}
static void
py_where()
{
  int dir;
  while (get_dir("Map: Which direction?", &dir)) {
    mmove(dir, &panelD.panel_row, &panelD.panel_col);
    if (panelD.panel_row > MAX_ROW - 2) panelD.panel_row = MAX_ROW - 2;
    if (panelD.panel_col > MAX_COL - 2) panelD.panel_col = MAX_COL - 2;
    panel_bounds(&panelD);
  }
  panel_update(&panelD, uD.y, uD.x, TRUE);
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

    ma_duration(MA_SLOW, penalty);
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

    need_exp = lev_exp(lev);
    if (exp > need_exp) {
      /* lose some of the 'extra' exp when gaining several levels at once */
      dif_exp = exp - need_exp;
      exp = need_exp + (dif_exp / 2);
    }
  }

  uD.exp = exp;
  uD.max_exp = MAX(exp, uD.max_exp);
  uD.lev = lev;

  calc_hitpoints();
  calc_mana();
}
int
restore_level()
{
  int restore, lev, exp;

  restore = FALSE;
  if (uD.max_exp > uD.exp) {
    restore = TRUE;
    msg_print("You feel your life energies returning.");
    exp = uD.max_exp;
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
int
py_lose_experience(amount)
{
  int lev, exp;
  struct objS* obj;

  obj = obj_get(invenD[INVEN_WIELD]);
  if (obj->sn != SN_SU) {
    exp = MAX(uD.exp - amount, 0);

    lev = 1;
    while (lev_exp(lev) <= exp) lev++;

    uD.exp = exp;
    if (uD.lev != lev) {
      uD.lev = lev;

      calc_hitpoints();
      calc_mana();
    }

    return TRUE;
  }

  return FALSE;
}
void
teleport_to(ny, nx)
{
  int dis, ctr, y, x;

  y = uD.y;
  x = uD.x;
  py_light_off(y, x);

  dis = 1;
  ctr = 0;
  do {
    y = ny + (randint(2 * dis + 1) - (dis + 1));
    x = nx + (randint(2 * dis + 1) - (dis + 1));
    ctr++;
    if (ctr > 9) {
      ctr = 0;
      dis++;
    }
  } while (!in_bounds(y, x) || (caveD[y][x].fval >= MIN_CLOSED_SPACE) ||
           (caveD[y][x].midx != 0));

  uD.y = y;
  uD.x = x;
  panel_update(&panelD, y, x, FALSE);
  py_check_view(TRUE);
}
// TBD: We may loop infinitely with the added restriction of oidx != 0
// Phase door (short range) teleport runs a higher risk
void py_teleport(dis, uy, ux) int *uy, *ux;
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
           (caveD[ty][tx].midx != 0) || (caveD[ty][tx].oidx != 0) ||
           distance(ty, tx, y, x) <= 2);
  *uy = ty;
  *ux = tx;
}
void
teleport_away(midx, dis)
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
      c_ptr->cflag |= (CF_PERM_LIGHT | CF_SEEN);

      if (c_ptr->midx) {
        m_ptr = &entity_monD[c_ptr->midx];
        cr_ptr = &creatureD[m_ptr->cidx];
        /* light up and draw monster */
        update_mon(c_ptr->midx);
        mon_desc(c_ptr->midx);
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
    mmove(dir, &y, &x);
  } while (!flag);
}
void magic_bolt(typ, dir, y, x, dam, bolt_typ) char* bolt_typ;
{
  int dist, flag;
  uint32_t weapon_type;
  int harm_type;
  int (*dummy)();
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cre;

  flag = FALSE;
  get_flags(typ, &weapon_type, &harm_type, &dummy);
  dist = 0;
  do {
    mmove(dir, &y, &x);
    dist++;
    c_ptr = &caveD[y][x];
    if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE) {
      flag = TRUE;
      MSG("The %s strikes a wall.", bolt_typ);
    } else {
      if (c_ptr->midx) {
        flag = TRUE;
        m_ptr = &entity_monD[c_ptr->midx];
        cre = &creatureD[m_ptr->cidx];

        // Bolt provides a temporary light
        if (cr_seen(cre)) m_ptr->mlit = TRUE;

        mon_desc(c_ptr->midx);
        descD[0] = descD[0] | 0x20;
        MSG("The %s strikes %s.", bolt_typ, descD);

        // draw with temporary visibility
        msg_pause();

        if (harm_type & cre->cdefense) {
          dam = dam * 2;
        } else if (weapon_type & cre->spells) {
          dam = dam / 4;
        }

        descD[0] = descD[0] & ~0x20;
        if (mon_take_hit(c_ptr->midx, dam)) {
          MSG("%s dies in a fit of agony.", descD);
          py_experience();
        } else {
          MSG("%s screams in agony.", descD);
        }
      }
    }
  } while (!flag);
}
void fire_ball(typ, dir, y, x, dam_hp, descrip) char* descrip;
{
  int i, j;
  int dam, max_dis, thit, tkill;
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
              if ((c_ptr->oidx) && (*destroy)(&entity_objD[c_ptr->oidx])) {
                if (c_ptr->fval == FLOOR_OBST) c_ptr->fval = FLOOR_CORR;
                delete_object(i, j);
              }
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
              }
            }

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
      }
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
          c_ptr->cflag |= (CF_PERM_LIGHT & caveD[i][j].cflag);
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
  if (panel_contains(&panelD, y, x))
    if (CF_LIT & c_ptr->cflag && c_ptr->oidx)
      msg_print("You have found something!");
  res = TRUE;

  return (res);
}
int
wall_to_mud(dir, y, x)
{
  int rubble, door, dist, lit;
  int flag;
  struct caveS* c_ptr;
  struct objS* obj;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;
  char* known;

  flag = FALSE;
  known = 0;
  dist = 0;
  do {
    mmove(dir, &y, &x);
    dist++;
    c_ptr = &caveD[y][x];
    obj = &entity_objD[c_ptr->oidx];
    lit = (CF_VIZ & c_ptr->cflag) != 0;
    rubble = (obj->tval == TV_RUBBLE);
    door = (obj->tval == TV_CLOSED_DOOR || obj->tval == TV_SECRET_DOOR);
    m_ptr = &entity_monD[c_ptr->midx];
    cr_ptr = &creatureD[m_ptr->cidx];

    /* note, this ray can move through walls as it turns them to mud */
    if (dist == OBJ_BOLT_RANGE) flag = TRUE;
    if ((c_ptr->fval >= MIN_WALL) && (c_ptr->fval != BOUNDARY_WALL)) {
      flag = TRUE;
      twall(y, x);
      if (lit) known = "wall";
    } else if (rubble) {
      flag = TRUE;
      if (lit) known = "rubble";
      delete_object(y, x);
      if (randint(10) == 1) {
        place_object(y, x, FALSE);
      }
      twall(y, x);
    } else if (door) {
      flag = TRUE;
      if (lit) known = "door";
      delete_object(y, x);
      twall(y, x);
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

  if (known) MSG("The %s turns into mud.", known);

  return known != 0;
}
int
poly_monster(dir, y, x)
{
  int dist, flag, poly, midx;
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
        midx = place_monster(
            y, x, randint(m_level[MAX_MON_LEVEL] - m_level[0]) - 1 + m_level[0],
            FALSE);
        if (midx) update_mon(midx);
        m_ptr = &entity_monD[midx];
        if (m_ptr->mlit) poly = TRUE;
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
  int flag, dist, monster;
  struct caveS* c_ptr;

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
      /* Monsters with innate resistence ignore the attack.
         Monsters which resisted the attack should wake up.  */
      if (CD_NO_SLEEP & cr_ptr->cdefense) {
        MSG("%s is unaffected.", descD);
      } else if (randint(MAX_MON_LEVEL) < cr_ptr->level) {
        MSG("%s sounds disoriented, only for a moment.", descD);
        m_ptr->msleep = 0;
      } else {
        if (m_ptr->mconfused)
          m_ptr->mconfused += 3;
        else
          m_ptr->mconfused = 2 + randint(16);
        confuse = TRUE;
        m_ptr->msleep = 0;
        MSG("%s sounds confused.", descD);
      }
    }
  } while (!flag);
  return (confuse);
}
int
sleep_monster(dir, y, x)
{
  int flag, dist, sleep, seen;
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;

  sleep = FALSE;
  flag = FALSE;
  seen = 0;
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

      sleep = ((CD_NO_SLEEP & cr_ptr->cdefense) == 0) &&
              (randint(MAX_MON_LEVEL) >= cr_ptr->level);

      if (sleep) m_ptr->msleep = 500;

      if (m_ptr->mlit) {
        mon_desc(c_ptr->midx);
        if (sleep) {
          seen += 1;
          MSG("%s falls asleep.", descD);
        } else {
          MSG("%s is unaffected.", descD);
        }
      }
    }
  } while (!flag);
  return seen != 0;
}
int
sleep_monster_aoe(maxdis)
{
  int y, x, sleep, cdis, seen;
  struct creatureS* cr_ptr;

  y = uD.y;
  x = uD.x;
  seen = 0;
  FOR_EACH(mon, {
    cr_ptr = &creatureD[mon->cidx];
    cdis = distance(y, x, mon->fy, mon->fx);
    if ((cdis > maxdis) || !los(y, x, mon->fy, mon->fx)) continue;

    sleep = ((CD_NO_SLEEP & cr_ptr->cdefense) == 0) &&
            (randint(MAX_MON_LEVEL) >= cr_ptr->level);

    if (sleep) mon->msleep = 500;

    if (mon->mlit) {
      mon_desc(it_index);
      if (sleep) {
        seen += 1;
        MSG("%s falls asleep.", descD);
      } else {
        MSG("%s is unaffected.", descD)
      }
    }
  });
  return seen != 0;
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
        mass = TRUE;
        caveD[fy][fx].midx = 0;
        mon_unuse(mon);

        place_monster(
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
td_destroy_bolt(dir, y, x)
{
  int destroy2, dist, exp;
  struct caveS* c_ptr;
  struct objS* obj;

  destroy2 = FALSE;
  dist = 0;
  exp = 0;
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
        exp += (obj->tval == TV_CLOSED_DOOR && obj->p1 > 0);
        exp += (obj->tval == TV_VIS_TRAP) * obj->p1;
        delete_object(y, x);
        if (c_ptr->fval == FLOOR_OBST) c_ptr->fval = FLOOR_CORR;
        msg_print("There is a bright flash of light!");
        destroy2 = TRUE;
      } else if ((obj->tval == TV_CHEST) && (obj->flags != 0)) {
        exp += obj->level;
        msg_print("Click!");
        obj->flags &= ~(CH_TRAPPED | CH_LOCKED);
        destroy2 = TRUE;
        obj->sn = SN_UNLOCKED;
        obj->idflag = ID_REVEAL;
      }
    }
  } while (dist <= OBJ_BOLT_RANGE && c_ptr->fval <= MAX_OPEN_SPACE);

  if (exp) {
    uD.exp += exp;
    py_experience();
  }

  return (destroy2);
}
int
build_wall(dir, y, x)
{
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
      c_ptr->fval = MAGMA_WALL;
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
    } else if ((obj->tval == TV_CHEST) && (obj->flags != 0)) {
      obj->flags &= ~(CH_TRAPPED | CH_LOCKED);
      disarm = TRUE;
      obj->sn = SN_UNLOCKED;
      obj->idflag = ID_REVEAL;
    }

    mmove(dir, &y, &x);
  } while ((dist <= OBJ_BOLT_RANGE) && c_ptr->fval <= MAX_OPEN_SPACE);
  if (disarm) msg_print("Click!");
  return (disarm);
}
int
dispel_creature(cflag, damage)
{
  int y, x, dispel;
  struct creatureS* cr_ptr;

  y = uD.y;
  x = uD.x;
  dispel = FALSE;
  FOR_EACH(mon, {
    cr_ptr = &creatureD[mon->cidx];
    if ((cflag & cr_ptr->cdefense) &&
        (distance(y, x, mon->fy, mon->fx) <= MAX_SIGHT) &&
        los(y, x, mon->fy, mon->fx)) {
      dispel = TRUE;
      mon->mlit = TRUE;
      mon->msilenced = TRUE;
      mon_desc(it_index);
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
genocide(cidx)
{
  int count = 0;

  FOR_EACH(mon, {
    if (mon->cidx == cidx) {
      caveD[mon->fy][mon->fx].midx = 0;
      mon_unuse(mon);
      count += 1;
    }
  });
  return count != 0;
}
int
mass_genocide(y, x)
{
  int count;
  struct creatureS* cr_ptr;

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
int
inven_recharge(iidx, rigor)
{
  int chance, amount;
  int res;
  struct objS* i_ptr;

  res = 0;
  if (iidx >= 0) {
    i_ptr = obj_get(invenD[iidx]);
    if (i_ptr->tval == TV_WAND || i_ptr->tval == TV_STAFF) {
      if ((i_ptr->idflag & ID_REVEAL) == 0) {
        msg_print(
            "You are uncertain about the magical properties of this device.");
      } else {
        /* recharge I = recharge(20) = 1/6 failure for empty 10th level wand */
        /* recharge II = recharge(60) = 1/10 failure for empty 10th level wand*/
        amount = rigor ? 60 : 20;

        /* make it harder to recharge high level, and highly charged wands, note
           that chance can be negative, so check its value before trying to call
           randint().  */
        chance = amount + 50 - i_ptr->level - i_ptr->p1;
        if (chance < 19)
          chance = 1; /* Automatic failure.  */
        else
          chance = randint(chance / 10);

        if (chance == 1) {
          if (rigor == 0) {
            msg_print("You are blinded by a bright flash of light.");
            ma_duration(MA_BLIND, 1);
            obj_unuse(i_ptr);
            invenD[iidx] = 0;
          } else {
            MSG("Energy crackles and discharges into the air.");
            if (i_ptr->level <= 250) i_ptr->level += 5;
          }
        } else {
          obj_desc(i_ptr, 1);
          MSG("Energy flows into %s!", descD);
          amount = (amount / (i_ptr->level + 2)) + 1;
          i_ptr->p1 += 2 + randint(amount);
          i_ptr->idflag = ID_REVEAL;
        }

        res = 1;
      }
    }
  }
  return res;
}
int
sleep_adjacent(y, x)
{
  int i, j;
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;
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
int
turn_undead()
{
  int py, px, cdis, turn_und;
  struct creatureS* cr_ptr;

  py = uD.y;
  px = uD.x;
  turn_und = FALSE;
  FOR_EACH(mon, {
    cr_ptr = &creatureD[mon->cidx];
    cdis = distance(py, px, mon->fy, mon->fx);
    if ((cdis <= MAX_SIGHT) && (CD_UNDEAD & cr_ptr->cdefense) &&
        los(py, px, mon->fy, mon->fx)) {
      int success = ((uD.lev + 1) > cr_ptr->level) || (randint(5) == 1);

      if (success) {
        turn_und += 1;
        mon->mconfused = uD.lev;
        mon->msleep = 0;
      }

      if (mon->mlit) {
        mon_desc(it_index);
        if (success) {
          MSG("%s runs frantically!", descD);
        } else {
          MSG("%s is unaffected.", descD);
        }
      }
    }
  });
  return (turn_und);
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
  int i, j, trap;
  struct caveS* c_ptr;

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
  int i, j, destroy, exp;
  struct caveS* c_ptr;
  struct objS* obj;

  destroy = FALSE;
  exp = 0;
  for (i = y - 1; i <= y + 1; i++)
    for (j = x - 1; j <= x + 1; j++) {
      c_ptr = &caveD[i][j];
      obj = &entity_objD[c_ptr->oidx];
      if ((obj->tval == TV_INVIS_TRAP) || (obj->tval == TV_CLOSED_DOOR) ||
          (obj->tval == TV_VIS_TRAP) || (obj->tval == TV_OPEN_DOOR) ||
          (obj->tval == TV_SECRET_DOOR)) {
        exp += (obj->tval == TV_CLOSED_DOOR && obj->p1 > 0);
        exp += (obj->tval == TV_VIS_TRAP) * obj->p1;
        delete_object(i, j);
        if (c_ptr->fval == FLOOR_OBST) c_ptr->fval = FLOOR_CORR;
        msg_print("There is a bright flash of light!");
        destroy = TRUE;
      } else if ((obj->tval == TV_CHEST) && (obj->flags != 0)) {
        exp += obj->level;
        msg_print("Click!");
        obj->flags &= ~(CH_TRAPPED | CH_LOCKED);
        destroy = TRUE;
        obj->sn = SN_UNLOCKED;
        obj->idflag = ID_REVEAL;
      }
    }

  if (exp) {
    uD.exp += exp;
    py_experience();
  }

  return (destroy);
}
int
aggravate_monster(dis_affect)
{
  int y, x, count;

  y = uD.y;
  x = uD.x;
  count = 0;
  FOR_EACH(mon, {
    mon->msleep = 0;
    if (distance(mon->fy, mon->fx, y, x) <= dis_affect) {
      count += 1;
      if (mon->mspeed < 2) mon->mspeed += 1;
    }
  });
  if (count) msg_print("You hear a sudden stirring in the distance!");
  return (count > 0);
}
int
door_creation()
{
  int y, x, i, j, door;
  struct caveS* c_ptr;

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
  int y, x, see_count;
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
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;

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
  struct caveS* c_ptr;

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
      c_ptr->fval = QUARTZ_WALL;
      break;
    case 5:
    case 8:
    case 11:
      c_ptr->fval = MAGMA_WALL;
      break;
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
            if (cr_ptr->cmove & (CM_ATTACK_ONLY | CM_ONLY_MAGIC))
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
        } else if (c_ptr->fval <= MAX_FLOOR) {
          tmp = randint(10);
          if (tmp < 6)
            c_ptr->fval = QUARTZ_WALL;
          else if (tmp < 9)
            c_ptr->fval = MAGMA_WALL;
          else
            c_ptr->fval = GRANITE_WALL;
        }
        c_ptr->cflag &= ~(CF_PERM_LIGHT | CF_FIELDMARK);
      }
}
void
destroy_area(y, x)
{
  int i, j, k;

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
  ma_duration(MA_BLIND, 10 + randint(10));
  py_light_off(uD.y, uD.x);
}
void
starlite(y, x)
{
  see_print("The end of the staff bursts into a blue shimmering light.");
  for (int it = 1; it <= 9; it++)
    if (it != 5) light_line(it, y, x);
}
static int
obj_cmp(a, b)
struct objS *a, *b;
{
  int aid, bid, astack, bstack, ar, br, at, bt, ak, bk, known;

  aid = (a->id != 0);
  bid = (b->id != 0);
  if (aid != bid) return aid - bid;

  // Stackable first
  astack = a->subval & STACK_SINGLE;
  bstack = b->subval & STACK_SINGLE;
  if (astack != bstack) return astack - bstack;

  // Projectile last
  astack = a->subval & STACK_PROJECTILE;
  bstack = b->subval & STACK_PROJECTILE;
  if (astack != bstack) return bstack - astack;

  if (a->subval & STACK_SINGLE) {
    ak = tr_is_known(&treasureD[a->tidx]);
    bk = tr_is_known(&treasureD[b->tidx]);
    if (ak != bk) return ak - bk;
    known = ak;
  } else {
    ar = a->idflag & ID_REVEAL;
    br = b->idflag & ID_REVEAL;
    if (ar != br) return ar - br;
    known = ar;
  }

  at = a->tval;
  bt = b->tval;
  if (at != bt) return at - bt;

  return known ? mask_subval(b->subval) - mask_subval(a->subval) : 0;
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
      if (obj_cmp(obj[j], obj[i]) > 0) {
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
inven_reveal()
{
  int flag = 0;
  struct objS* obj;
  struct treasureS* tr_ptr;

  for (int it = 0; it < MAX_INVEN; ++it) {
    obj = obj_get(invenD[it]);
    tr_ptr = &treasureD[obj->tidx];

    if (obj->id && (obj->idflag & ID_REVEAL) == 0) {
      obj->idflag = ID_REVEAL;
      tr_make_known(tr_ptr);
      flag = TRUE;
    }
  }
  return flag;
}
enum { INVEN_DETAIL = 18 };
enum { DROP_DETAIL = 8 };
static int
inven_overlay(begin, end)
{
  USE(overlay_width);
  USE(drop_mode);
  int line, count;
  int limitw = MIN(overlay_width, 80);
  int descw = 4;
  int detailw = limitw - INVEN_DETAIL;
  int dropw = limitw - DROP_DETAIL;

  if (drop_mode) {
    detailw -= DROP_DETAIL;
  }

  line = count = 0;
  overlay_submodeD = begin == 0 ? 'i' : 'e';
  for (int it = begin; it < end; ++it) {
    int len = 1;
    int obj_id = invenD[it];

    overlayD[line][0] = '(';
    overlayD[line][1] = 'a' + it - begin;
    overlayD[line][2] = ')';
    overlayD[line][3] = ' ';
    if (obj_id) {
      struct objS* obj = obj_get(obj_id);
      obj_desc(obj, obj->number);
      obj_detail(obj);

      memcpy(overlayD[line] + descw, AP(descD));
      if (detailD[1] != ' ') {
        if (descD[detailw - descw - 1] != ' ') {
          memcpy(overlayD[line] + detailw - 3, AP("..."));
        }

        memcpy(overlayD[line] + detailw, detail_nosp(), INVEN_DETAIL);
      }

      if (drop_mode) {
        int stack_weight = obj->number * obj->weight;
        snprintf(overlayD[line] + dropw, DROP_DETAIL, " %2d.%01dlb",
                 stack_weight / 10, stack_weight % 10);
      }

      len = limitw;
      count += 1;
    } else {
      overlayD[line][0] = 0;
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
    obj_desc(i_ptr, 1);
    MSG("Your %s glows black, fades.", descD);
    i_ptr->tohit = -randint(5) - randint(5);
    i_ptr->todam = -randint(5) - randint(5);
    i_ptr->toac = 0;

    /* Must call py_bonuses() before set (clear) sn & flags, and
       must call calc_bonuses() after set (clear) sn & flags, so that
       all attributes will be properly turned off. */
    py_bonuses(i_ptr, -1);
    i_ptr->sn = 0;
    i_ptr->flags = TR_CURSED;
    calc_bonuses();
    return TRUE;
  }

  return FALSE;
}
int
inven_eat(iidx)
{
  uint32_t i;
  int j, ident, known;
  struct objS* obj = obj_get(invenD[iidx]);
  struct treasureS* tr_ptr = &treasureD[obj->tidx];
  known = tr_is_known(tr_ptr);

  if (obj->tval == TV_FOOD) {
    i = obj->flags;
    ident = FALSE;
    while (i != 0) {
      j = bit_pos(&i) + 1;
      /* Foods  				*/
      switch (j) {
        case 1:
          countD.poison += randint(10) + obj->level;
          ident |= TRUE;
          break;
        case 2:
          msg_print("A veil of darkness surrounds you.");
          ma_duration(MA_BLIND, randint(250) + 10 * obj->level + 100);
          ident |= TRUE;
          break;
        case 3:
          msg_print("You feel terrified!");
          ma_duration(MA_FEAR, randint(10) + obj->level);
          ident |= TRUE;
          break;
        case 4:
          countD.confusion += randint(10) + obj->level;
          msg_print("You feel confused.");
          ident |= TRUE;
          break;
        case 5:
          countD.imagine += randint(200) + 25 * obj->level + 200;
          msg_print("You feel drugged.");
          ident |= TRUE;
          break;
        case 6:
          if (countD.poison > 0) {
            countD.poison = 1;
            ident |= TRUE;
          }
          break;
        case 7:
          ident |= ma_clear(MA_BLIND);
          break;
        case 8:
          ident |= ma_clear(MA_FEAR);
          break;
        case 9:
          if (countD.confusion > 0) {
            countD.confusion = 1;
            ident = TRUE;
          }
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
            ident |= TRUE;
          }
          break;
        case 17:
          if (res_stat(A_CON)) {
            ident |= TRUE;
          }
          break;
        case 18:
          if (res_stat(A_INT)) {
            ident |= TRUE;
          }
          break;
        case 19:
          if (res_stat(A_WIS)) {
            ident |= TRUE;
          }
          break;
        case 20:
          if (res_stat(A_DEX)) {
            ident |= TRUE;
          }
          break;
        case 21:
          if (res_stat(A_CHR)) {
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
    if (!known) {
      if (ident) {
        tr_make_known(tr_ptr);
        tr_discovery(tr_ptr);
        py_experience();
      } else {
        tr_sample(tr_ptr);
      }
    }
    py_add_food(obj->p1);
    obj_desc(obj, obj->number - 1);
    MSG("You have %s.", descD);
    inven_destroy_num(iidx, 1);
    turn_flag = TRUE;
    return TRUE;
  } else {
    msg_print("You can't eat that!");
  }
  return FALSE;
}
int
obj_tabil(obj, truth)
struct objS* obj;
{
  int tabil;
  tabil = -1;
  if (obj->tval == TV_DIGGING || may_equip(obj->tval) == INVEN_WIELD) {
    tabil = HACK ? 9000 : 0;
    if (truth || (obj->idflag & ID_REVEAL)) {
      if (TR_TUNNEL & obj->flags)
        tabil += obj->p1 * 50;
      else {
        tabil += obj->tohit + obj->todam;
      }
    }

    if (TR_TUNNEL & obj->flags)
      tabil += 25;
    else {
      tabil += obj->damage[0] * obj->damage[1];
      /* divide by two so that digging without shovel isn't too easy */
      tabil >>= 1;
    }
  }
  return tabil;
}
int
obj_study(obj, for_sale)
struct objS* obj;
{
  struct treasureS* tr_ptr;
  int line;
  int reveal, blows, eqidx;
  int number, stackweight, stacklimit;
  int wtohit;

  reveal = obj->idflag & ID_REVEAL;
  if (obj->tidx) {
    tr_ptr = &treasureD[obj->tidx];
    screen_submodeD = 1;
    if (HACK) obj->idflag = ID_REVEAL;
    eqidx = may_equip(obj->tval);
    number = obj->number;
    if (for_sale && (STACK_SINGLE & obj->subval)) number = 1;

    line = 0;
    BufMsg(screen, "%-17.017s: %d.%01d Lbs", "Weight (single)",
           obj->weight / 10, obj->weight % 10);
    if (number > 1) {
      int sum_weight = obj->number * obj->weight;
      BufMsg(screen, "%-17.017s: %d.%01d Lbs", "Total Weight", sum_weight / 10,
             sum_weight % 10);
    }
    if (STACK_ANY & obj->subval) {
      stackweight = ustackweight();
      stacklimit = stacklimit_by_max_weight(stackweight, obj->weight);
      BufMsg(screen, "%-17.017s: %d", "Stack Limit (STR)", stacklimit);
    }

    if (tr_is_known(tr_ptr)) {
      if (obj->tval == TV_WAND || obj->tval == TV_STAFF) {
        int diff = udevice() - obj->level - ((obj->tval == TV_STAFF) * 5);
        BufMsg(screen, "%-17.017s: %d", "Device Level", obj->level);
        BufMsg(screen, "%-17.017s: %+d", "Device Skill", diff);
      }
    } else {
      BufMsg(screen, "... has unknown effects!");
    }

    if (obj->tval == TV_DIGGING) {
      BufMsg(screen, "%-17.017s: %+d", "+ To Digging", obj_tabil(obj, reveal));
    }
    if (obj->tval == TV_LAUNCHER) {
      if (obj->p1 < AL(projectile_nameD)) {
        BufMsg(screen, "%-17.017s: %s", "Projectile",
               projectile_nameD[obj->p1]);
      }
    }
    if (obj->tval == TV_PROJECTILE) {
      if (obj->p1 < AL(launcher_nameD)) {
        BufMsg(screen, "%-17.017s: %s", "Launcher", launcher_nameD[obj->p1]);
      }
    }
    if (eqidx == INVEN_WIELD) {
      wtohit = tohit_by_weight(obj->weight);
      if (wtohit) BufMsg(screen, "%-17.017s: %+d", "Heavy Penalty", wtohit);
      BufMsg(screen, "%-17.017s: %+d", "+ To Hit", obj->tohit);
      BufMsg(screen, "%-17.017s: %+d", "+ To Damage", obj->todam);

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
    } else if (eqidx > INVEN_WIELD) {
      if (eqidx == INVEN_BODY) {
        BufMsg(screen, "%-17.017s: %+d", "+ To Hit", obj->tohit);
      } else if (reveal && oset_tohitdam(obj)) {
        BufMsg(screen, "%-17.017s: %+d", "+ To Hit", obj->tohit);
        BufMsg(screen, "%-17.017s: %+d", "+ To Damage", obj->todam);
      }
    }

    if (eqidx >= INVEN_EQUIP) {
      if (reveal && (obj->ac || obj->toac)) {
        line += 1;
        if (obj->ac) BufMsg(screen, "%-17.017s: %d", "Base Armor", obj->ac);
        if (obj->toac)
          BufMsg(screen, "%-17.017s: %+d", "+ To Armor", obj->toac);
        BufMsg(screen, "%-17.017s: %+d", "Total Armor", obj->ac + obj->toac);
      }
    }

    if (eqidx >= INVEN_EQUIP) {
      line += 1;
      if (obj->idflag & ID_REVEAL) {
        if (obj->flags & TR_SEARCH) {
          if (obj->p1) {
            BufMsg(screen, "%-17.017s: %+d", "Search", obj->p1);
            BufMsg(screen, "%-17.017s: %+d", "Perception", obj->p1);
          }
        }
        if (obj->flags & TR_STEALTH) {
          if (obj->p1) BufMsg(screen, "%-17.017s: %+d", "Stealth", obj->p1);
        }
        if (obj->flags & TR_TUNNEL) {
          BufMsg(screen, "%-17.017s: %+d", "Digging", obj->p1);
        }
        for (int it = 0; it < MAX_A; ++it) {
          if (obj->flags & (1 << it)) {
            BufMsg(screen, "%-17.017s: %+d", stat_nameD[it], obj->p1);
            if (obj->flags & TR_SUST_STAT)
              BufMsg(screen, "%s cannot be reduced", stat_nameD[it]);
          }
        }
        if (obj->flags & TR_SLOW_DIGEST) {
          BufMsg(screen, "slows digestion");
        }
        if (obj->flags & TR_AGGRAVATE) {
          BufMsg(screen, "aggravates monsters");
        }
        if (obj->flags & TR_TELEPORT) {
          BufMsg(screen, "randomly teleports you");
        }
        if (obj->flags & TR_REGEN) {
          BufMsg(screen, "increases health regeneration");
        }
        if (obj->flags & TR_SPEED) {
          BufMsg(screen, "increases movement and attack speed");
        }

        if (obj->flags & TR_EGO_WEAPON) {
          BufMsg(screen, "one of the following damage multipliers:");
        }
        if (obj->flags & TR_SLAY_DRAGON) {
          BufMsg(screen, "  x4 vs dragons");
        }
        if (obj->flags & TR_SLAY_UNDEAD) {
          BufMsg(screen, "  x3 vs undead");
        }
        if (obj->flags & TR_SLAY_ANIMAL) {
          BufMsg(screen, "  x2 vs animals");
        }
        if (obj->flags & TR_SLAY_EVIL) {
          BufMsg(screen, "  x2 vs evil");
        }
        if (obj->flags & TR_FROST_BRAND) {
          BufMsg(screen, "  x1.5 vs vulnerable to cold");
        }
        if (obj->flags & TR_FLAME_TONGUE) {
          BufMsg(screen, "  x1.5 vs vulnerable to fire");
        }

        if (obj->flags & TR_RES_FIRE) {
          BufMsg(screen, "grants resistence to fire damage");
        }
        if (obj->flags & TR_RES_ACID) {
          BufMsg(screen, "grants resistence to acid damage");
        }
        if (obj->flags & TR_RES_COLD) {
          BufMsg(screen, "grants resistence to cold damage");
        }
        if (obj->flags & TR_RES_LIGHT) {
          BufMsg(screen, "grants resistence to lightning damage");
        }
        if (obj->flags & TR_SEEING) {
          BufMsg(screen, "grants immunity to blindness");
        }
        if (obj->flags & TR_HERO) {
          BufMsg(screen, "grants immunity to fear");
        }
        if (obj->flags & TR_FREE_ACT) {
          BufMsg(screen, "grants immunity to paralysis");
        }
        if (obj->flags & TR_SEE_INVIS) {
          BufMsg(screen, "grants sight of invisible monsters");
        }
        if (obj->flags & TR_FFALL) {
          BufMsg(screen, "prevents falling through trap doors");
        }
        if (obj->flags & TR_SLOWNESS) {
          BufMsg(screen, "slows movement and attack speed");
        }
        if (obj->flags & TR_CURSED) {
          BufMsg(screen, "... is known to be cursed!");
        }
        if (obj->sn == SN_SU) {
          BufMsg(screen, "grants resistence to life drain");
        }
        if (obj->sn == SN_INFRAVISION) {
          BufMsg(screen, "grants infra-vision up to %+d feet", obj->p1 * 10);
        }
        if (obj->sn == SN_LORDLINESS) {
          BufMsg(screen, "improves skill with magic devices by %+d",
                 obj->p1 * 10);
        }
      } else {
        if (obj->idflag & ID_CORRODED) {
          BufMsg(screen, "... is corroded, providing no protection from acid.");
        }

        if (obj->idflag & ID_PLAIN) {
          BufMsg(screen, "... is known to be plain.");
        } else if (obj->idflag & ID_DAMD) {
          BufMsg(screen, "... is known to be cursed!");
        } else if (obj->idflag & ID_MAGIK) {
          BufMsg(screen, "... is known to be magical!");
        } else if (obj->idflag & ID_RARE) {
          BufMsg(screen, "... is known to be rare!");
        } else {
          BufMsg(screen, "... is unidentified!");
        }
      }
    }

    obj_desc(obj, number);
    DRAWMSG("You study %s.", descD);
    return inkey();
  }

  return 0;
}
static int
inven_choice(char* prompt, char* mode_list)
{
  char c;
  int mode;
  int begin, end;
  char subprompt[80];
  char* prefix;

  mode = mode_list[0];
  while (mode) {
    switch (mode) {
      case '*':
        begin = 0;
        end = INVEN_EQUIP;
        prefix = "Inventory";
        break;
      default:
      case '/':
        begin = INVEN_WIELD;
        end = MAX_INVEN;
        prefix = "Equipment";
        break;
    }

    // TBD: Console Append (mode_list[1] ? "(/ equip, * inven, - sort)" : "")
    snprintf(subprompt, AL(subprompt), "%s: %s", prefix, prompt);
    inven_overlay(begin, end);

    if (!in_subcommand(subprompt, &c)) return -1;

    if (is_lower(c)) {
      uint8_t iidx = c - 'a';
      iidx += begin;
      if (iidx < end && invenD[iidx]) return iidx;
    } else if (is_upper(c)) {
      uint8_t iidx = c - 'A';
      iidx += begin;
      if (iidx < end && invenD[iidx]) {
        c = obj_study(obj_get(invenD[iidx]), FALSE);
        // Prohibited on the death screen
        if (platformD.selection && uD.new_level_flag == 0) {
          switch (c) {
            case 'o':
              inven_drop(iidx);
              return -1;
            case ESCAPE:
              return iidx;
          }
        }
      }
    } else if (c == '-') {
      inven_sort();
    } else {
      mode = 0;
      for (int it = 0; it < 2; ++it) {
        if (mode_list[it] == c) mode = c;
      }
    }
  }

  return -1;
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
    // Water and Apple Juice have no affect
    ident = (i == 0);
    while (i != 0) {
      j = bit_pos(&i) + 1;
      j += (obj->tval == TV_POTION2) * 32;
      switch (j) {
        case 1:
          ident |= inc_stat(A_STR);
          break;
        case 2:
          ident |= TRUE;
          lose_stat(A_STR);
          break;
        case 3:
          ident |= res_stat(A_STR);
          ident |= inc_stat(A_STR);
          break;
        case 4:
          ident |= inc_stat(A_INT);
          break;
        case 5:
          ident |= TRUE;
          lose_stat(A_INT);
          break;
        case 6:
          ident |= res_stat(A_INT);
          ident |= inc_stat(A_INT);
          break;
        case 7:
          ident |= inc_stat(A_WIS);
          break;
        case 8:
          ident |= TRUE;
          lose_stat(A_WIS);
          break;
        case 9:
          ident |= res_stat(A_WIS);
          ident |= inc_stat(A_WIS);
          break;
        case 10:
          ident |= inc_stat(A_CHR);
          break;
        case 11:
          ident |= TRUE;
          lose_stat(A_CHR);
          break;
        case 12:
          ident |= res_stat(A_CHR);
          ident |= inc_stat(A_CHR);
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
          ident |= inc_stat(A_CON);
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
          if (py_affect(MA_BLIND) == 0) {
            msg_print("You are covered by a veil of darkness.");
            ident |= TRUE;
          }
          ma_duration(MA_BLIND, randint(100) + 100);
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
          ident |= py_affect(MA_FAST) == 0;
          ma_duration(MA_FAST, randint(25) + 15);
          break;
        case 24:
          ident |= py_affect(MA_SLOW) == 0;
          ma_duration(MA_SLOW, randint(25) + 15);
          break;
        case 26:
          ident |= inc_stat(A_DEX);
          break;
        case 27:
          ident |= res_stat(A_DEX);
          ident |= inc_stat(A_DEX);
          break;
        case 28:
          ident |= res_stat(A_CON);
          ident |= inc_stat(A_CON);
          break;
        case 29:
          ident |= ma_clear(MA_BLIND);
          break;
        case 30:
          if (countD.confusion > 0) {
            ident |= TRUE;
            countD.confusion = 1;
          }
          break;
        case 31:
          if (countD.poison > 0) {
            ident |= TRUE;
            countD.poison = 1;
          }
          break;
        case 34:
          if (uD.exp > 0) {
            int m, scale;
            /* Lose between 1/5 and 2/5 of your experience */
            m = uD.exp / 5;
            if (uD.exp > INT16_MAX) {
              scale = INT32_MAX / uD.exp;
              m += (randint(scale) * uD.exp) / (scale * 5);
            } else
              m += randint(uD.exp) / 5;
            if (py_lose_experience(m)) {
              msg_print("You feel your memories fade.");
              ident |= TRUE;
            }
          }
          break;
        case 35:
          countD.poison = 1;
          if (uD.food > 150) uD.food = 150;
          countD.paralysis = 4;
          msg_print("Ugh! Salt water!");
          ident |= TRUE;
          break;
        case 36:
          ident |= py_affect(MA_INVULN) == 0;
          ma_duration(MA_INVULN, randint(10) + 10);
          break;
        case 37:
          ident |= py_affect(MA_HERO) == 0;
          ma_duration(MA_HERO, randint(25) + 25);
          break;
        case 38:
          ident |= py_affect(MA_SUPERHERO) == 0;
          ma_duration(MA_SUPERHERO, randint(25) + 25);
          break;
        case 39:
          ident |= ma_clear(MA_FEAR);
          break;
        case 40:
          ident |= restore_level();
          break;
        case 41:
          ident |= (py_affect(MA_AFIRE) == 0);
          ma_duration(MA_AFIRE, randint(20) + 20);
          break;
        case 42:
          ident |= (py_affect(MA_AFROST) == 0);
          ma_duration(MA_AFROST, randint(20) + 20);
          break;
        case 43:
          ident |= py_tr(TR_SEE_INVIS) == 0;
          ma_duration(MA_SEE_INVIS, randint(12) + 12);
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
          if (uD.cmana < uD.mmana) {
            msg_print("Your feel your head clear.");
            uD.cmana = uD.mmana;
            ident |= TRUE;
          }
          break;
        case 47:
          if (py_affect(MA_SEE_INFRA)) {
            msg_print("Your eyes begin to tingle.");
            ident |= TRUE;
          }
          ma_duration(MA_SEE_INFRA, 100 + randint(100));
          break;
        default:
          msg_print("Internal error in potion()");
          break;
      }
    }
    if (!tr_is_known(tr_ptr)) {
      if (ident) {
        tr_make_known(tr_ptr);
        tr_discovery(tr_ptr);
        py_experience();
      } else {
        tr_sample(tr_ptr);
      }
    }

    py_add_food(obj->p1);
    obj_desc(obj, obj->number - 1);
    MSG("You have %s.", descD);
    inven_destroy_num(iidx, 1);
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
  int j, k;
  int ident, used_up, choice_idx, known;
  struct objS* i_ptr;
  struct treasureS* tr_ptr;

  if (py_affect(MA_BLIND))
    msg_print("You can't see to read the scroll.");
  else if (countD.confusion) {
    msg_print("You are too confused to read a scroll.");
  } else {
    i_ptr = obj_get(invenD[iidx]);
    tr_ptr = &treasureD[i_ptr->tidx];
    known = tr_is_known(tr_ptr);
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
            ident |= TRUE;
            choice_idx =
                inven_choice("Which weapon do you wish to enchant?", "/*");
            used_up = weapon_enchant(choice_idx, 1, 0);
            break;
          case 2:
            ident |= TRUE;
            choice_idx =
                inven_choice("Which weapon do you wish to enchant?", "/*");
            used_up = weapon_enchant(choice_idx, 0, 1);
            break;
          case 3:
            ident |= TRUE;
            choice_idx =
                inven_choice("Which armor do you wish to enchant?", "/*");
            used_up = equip_enchant(choice_idx, 1);
            break;
          case 4:
            msg_print("This is an identify scroll.");
            ident |= TRUE;
            choice_idx =
                inven_choice("Which item do you wish identified?", "*/");
            used_up = inven_ident(choice_idx);
            break;
          case 5:
            ident |= equip_remove_curse();
            break;
          case 6:
            ident |= illuminate(uD.y, uD.x);
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
            uD.new_level_flag = NL_TELEPORT;
            ident |= TRUE;
            break;
          case 11:
            if (uD.melee_confuse == 0) {
              msg_print("Your hands begin to glow blue.");
              uD.melee_confuse = 1;
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
            ident |= detect_obj(oset_gold, known);
            break;
          case 16:
            ident |= detect_obj(oset_mon_pickup, known);
            break;
          case 17:
            ident |= detect_obj(oset_trap, known);
            break;
          case 18:
            ident |= detect_obj(oset_doorstair, known);
            break;
          case 19:
            msg_print("This is a mass genocide scroll.");
            mass_genocide(uD.y, uD.x);
            ident |= TRUE;
            break;
          case 20:
            if (detect_mon(crset_invisible, known)) {
              ma_duration(MA_DETECT_INVIS, 1);
              ident |= TRUE;
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
            choice_idx = inven_choice("Recharge which item?", "*");
            used_up = inven_recharge(choice_idx, 0);
            break;
          case 26:
            msg_print("Your hands begin to glow black.");
            uD.melee_genocide = 1;
            ident |= TRUE;
            break;
          case 27:
            ident |= unlight_area(uD.y, uD.x);
            break;
          case 28:
            ident |= (countD.life_prot == 0);
            countD.life_prot += randint(25) + uD.lev;
            break;
          case 29:
            ident |= TRUE;
            create_food(uD.y, uD.x);
            break;
          case 30:
            ident |= dispel_creature(CD_UNDEAD, 60);
            break;
          case 33:
            ident |= TRUE;
            choice_idx =
                inven_choice("Which weapon do you wish to *Enchant*?", "/*");
            used_up = make_special_type(choice_idx, i_ptr->level, 1);
            break;
          case 34:
            ident |= weapon_curse();
            break;
          case 35:
            ident |= TRUE;
            choice_idx =
                inven_choice("Which armor do you wish to *Enchant*?", "/*");
            used_up = make_special_type(choice_idx, i_ptr->level, 0);
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
            ma_duration(MA_BLESS, randint(12) + 6);
            break;
          case 39:
            ident |= TRUE;
            ma_duration(MA_BLESS, randint(24) + 12);
            break;
          case 40:
            ident |= TRUE;
            ma_duration(MA_BLESS, randint(48) + 24);
            break;
          case 41:
            ident |= TRUE;
            if (!py_affect(MA_RECALL)) ma_duration(MA_RECALL, 25 + randint(30));
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

      if (used_up) {
        if (!known) {
          if (ident) {
            tr_make_known(tr_ptr);
            tr_discovery(tr_ptr);
            py_experience();
          } else {
            tr_sample(tr_ptr);
          }
        }

        obj_desc(i_ptr, i_ptr->number - 1);
        MSG("You have %s.", descD);
        // Choice menu allows for sort above, iidx may be invalid
        inven_used_obj(i_ptr);
        turn_flag = TRUE;
        return TRUE;
      }
    }
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
    chance = udevice() - i_ptr->level;
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
            magic_bolt(GF_LIGHTNING, dir, y, x, damroll(4, 8), spell_nameD[8]);
            ident |= TRUE;
            break;
          case 3:
            magic_bolt(GF_FROST, dir, y, x, damroll(6, 8), spell_nameD[14]);
            ident |= TRUE;
            break;
          case 4:
            magic_bolt(GF_FIRE, dir, y, x, damroll(9, 8), spell_nameD[22]);
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
            ident |= td_destroy_bolt(dir, y, x);
            break;
          case 14:
            magic_bolt(GF_MAGIC_MISSILE, dir, y, x, damroll(2, 6),
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
      if (!tr_is_known(tr_ptr)) {
        if (ident) {
          tr_make_known(tr_ptr);
          tr_discovery(tr_ptr);
          py_experience();
        } else {
          tr_sample(tr_ptr);
          msg_print("You try the wand, to unknown effect.");
        }
      }
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

  if (get_dir("Which direction will you zap?", &dir)) {
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
gain_spell(spidx)
{
  int spcount = uspellcount();
  for (int it = 0; it < spcount; ++it) {
    if (spell_orderD[it] == 0) {
      spell_orderD[it] = 1 + spidx;
      return 1;
    }
  }
  return 0;
}
int
spelldir_prompt(spidx)
{
  int dir;
  char tmp[STRLEN_MSG + 1];

  switch (spidx + 1) {
    case 1:
    case 7:
    case 8:
    case 9:
    case 11:
    case 15:
    case 16:
    case 20:
    case 23:
    case 24:
    case 25:
    case 27:
    case 29: {
      snprintf(tmp, AL(tmp), "Which direction will you cast %s?",
               spell_nameD[spidx]);
      if (!get_dir(tmp, &dir)) dir = -1;
    } break;
    default:
      dir = 0;
  }

  return dir;
}
void spell_dir(spidx, y_ptr, x_ptr, dir) int* y_ptr;
int* x_ptr;
{
  switch (spidx + 1) {
    case 1:
      magic_bolt(GF_MAGIC_MISSILE, dir, uD.y, uD.x, damroll(2, 6),
                 spell_nameD[0]);
      break;
    case 2:
      detect_mon(crset_visible, TRUE);
      ma_duration(MA_DETECT_MON, 1);
      break;
    case 3:
      py_teleport(10, y_ptr, x_ptr);
      break;
    case 4:
      illuminate(uD.y, uD.x);
      break;
    case 5:
      py_heal_hit(damroll(4, 4));
      break;
    case 6:
      detect_obj(oset_hidden, TRUE);
      break;
    case 7:
      fire_ball(GF_POISON_GAS, dir, uD.y, uD.x, 12, spell_nameD[6]);
      break;
    case 8:
      confuse_monster(dir, uD.y, uD.x);
      break;
    case 9:
      magic_bolt(GF_LIGHTNING, dir, uD.y, uD.x, damroll(4, 8), spell_nameD[8]);
      break;
    case 10:
      td_destroy(uD.y, uD.x);
      break;
    case 11:
      sleep_monster(dir, uD.y, uD.x);
      break;
    case 12:
      if (countD.poison > 0) {
        countD.poison = 1;
      }
      break;
    case 13:
      py_teleport(uD.lev * 3, y_ptr, x_ptr);
      break;
    case 14:
      equip_remove_curse();
      break;
    case 15:
      magic_bolt(GF_FROST, dir, uD.y, uD.x, damroll(6, 8), spell_nameD[14]);
      break;
    case 16:
      wall_to_mud(dir, uD.y, uD.x);
      break;
    case 17:
      create_food(uD.y, uD.x);
      break;
    case 18:
      // TBD: allow to fail without mana cost?
      inven_recharge(inven_choice("Recharge which item?", "*"), 0);
      break;
    case 19:
      sleep_monster_aoe(1);
      break;
    case 20:
      poly_monster(dir, uD.y, uD.x);
      break;
    case 21:
      inven_ident(inven_choice("Which item do you wish identified?", "*/"));
      break;
    case 22:
      sleep_monster_aoe(MAX_SIGHT);
      break;
    case 23:
      magic_bolt(GF_FIRE, dir, uD.y, uD.x, damroll(9, 8), spell_nameD[22]);
      break;
    case 24:
      speed_monster(dir, uD.y, uD.x, -1);
      break;
    case 25:
      fire_ball(GF_FROST, dir, uD.y, uD.x, 48, spell_nameD[24]);
      break;
    case 26:
      inven_recharge(inven_choice("Recharge which item?", "*"), 1);
      break;
    case 27:
      teleport_monster(dir, uD.y, uD.x);
      break;
    case 28:
      ma_duration(MA_FAST, randint(20) + uD.lev);
      break;
    case 29:
      fire_ball(GF_FIRE, dir, uD.y, uD.x, 72, spell_nameD[28]);
      break;
    case 30:
      destroy_area(uD.y, uD.x);
      break;
    case 31:
      msg_print("Your hands begin to glow black.");
      uD.melee_genocide = 1;
      break;
    default:
      break;
  }
}
int
book_prompt(names, spelltable, spmask, bookflags)
char** names;
struct spellS* spelltable;
uint32_t bookflags;
{
  char c;
  int cmana;
  int book[32], book_used, line;
  int spidx, spknown, splevel, spmana, spchance;
  int lev = uD.lev;

  cmana = uD.cmana;
  book_used = 0;
  while (bookflags) {
    book[book_used] = bit_pos(&bookflags);
    book_used += 1;
  }
  overlay_submodeD = 0;
  do {
    line = 0;
    for (int it = 0; it < book_used; ++it) {
      spidx = book[it];
      spknown = ((1 << spidx) & spmask);
      splevel = spelltable[spidx].splevel;
      spmana = spelltable[spidx].spmana;
      spchance = spell_chanceD[spidx];

      char field[2][16];
      if (spknown) {
        snprintf(field[0], AL(field[0]), "%d%% failure", spchance);
        if (cmana < spmana)
          snprintf(field[1], AL(field[1]), "-low mana-");
        else
          snprintf(field[1], AL(field[1]), "mana %d", spmana);
      } else if (splevel == 99) {
        field[0][0] = 0;
        field[1][0] = 0;
      } else {
        field[0][0] = 0;
        if (splevel > lev)
          snprintf(field[1], AL(field[0]), "level %d", splevel);
        else
          memcpy(&field[1], AP("unknown"));
      }

      BufMsg(overlay,
             "(%c) "
             "%-32.032s "
             "%13.013s "
             "%13.013s",
             'a' + it, splevel < 99 ? names[spidx] : "???", field[0], field[1]);
    }

    if (!in_subcommand("Read which spell?", &c)) break;
    uint8_t choice = c - 'a';
    if (choice < book_used) {
      return book[choice];
    }
  } while (!turn_flag);
  return -1;
}
void py_magic(iidx, y_ptr, x_ptr) int* y_ptr;
int* x_ptr;
{
  struct objS* obj;
  uint32_t flags, first_spell;
  int cmana, sptype, spmask, spidx, dir;
  struct spellS* spelltable;

  obj = obj_get(invenD[iidx]);
  flags = obj->flags;
  if (py_affect(MA_BLIND))
    msg_print("You can't see to read the book.");
  else if (countD.confusion) {
    msg_print("You are too confused to read a book.");
  } else if (obj->tval == TV_MAGIC_BOOK && flags) {
    sptype = classD[uD.clidx].spell;
    first_spell = classD[uD.clidx].first_spell_lev - 1;
    if (sptype != SP_MAGE)
      msg_print("You cannot make sense of the magical runes in this book.");
    else if (first_spell >= uD.lev)
      msg_print("You yearn to understand the magical runes filling this book.");
    else {
      cmana = uD.cmana;
      spmask = uspellmask();
      spelltable = uspelltable();

      if (last_castD == 0) {
        spidx = book_prompt(spell_nameD, spelltable, spmask, flags);
        last_castD = spidx + 1;
        if (spidx < 0) return;
      } else {
        spidx = last_castD - 1;
      }

      if ((1 << spidx) & spmask) {
        dir = spelldir_prompt(spidx);
        if (dir < 0) return;

        if (randint(100) < spell_chanceD[spidx]) {
          msg_print("You failed to get the spell off!");
        } else {
          spell_dir(spidx, y_ptr, x_ptr, dir);

          if ((uD.spell_worked & (1 << spidx)) == 0) {
            uD.spell_worked |= (1 << spidx);
            uD.exp += spelltable[spidx].spexp * SP_EXP_MULT;
            py_experience();
          }
        }

        if (uD.cmana < spelltable[spidx].spmana) {
          if (statD.cur_stat[A_CON] > 3) {
            uD.cmana = 0;
            uD.cmana_frac = 0;
            msg_print("Your low mana has damaged your health!");
            dec_stat(A_CON);
          } else {
            msg_print("You are out of mana and your body is weak.");
          }
        } else {
          uD.cmana -= spelltable[spidx].spmana;
        }
      } else {
        msg_print("You read the magical runes.");
        if (spelltable[spidx].splevel > uD.lev || !gain_spell(spidx)) {
          MSG("You are unable to retain the knowledge%s.",
              spelltable[spidx].splevel != 99 ? " at this time" : "");
        } else {
          MSG("You learn the spell of %s!", spell_nameD[spidx]);
        }
      }
      turn_flag = TRUE;
    }
  }
}
int
prayerdir_prompt(pridx)
{
  int dir;
  char tmp[STRLEN_MSG + 1];

  switch (pridx + 1) {
    case 9:
    case 18:
      snprintf(tmp, AL(tmp), "Which direction will you incant %s?",
               prayer_nameD[pridx]);
      if (!get_dir(tmp, &dir)) dir = -1;
      break;
    default:
      dir = 0;
      break;
  }
  return dir;
}
int
prayer_dir(pridx, y_ptr, x_ptr, dir)
int* y_ptr;
int* x_ptr;
{
  switch (pridx + 1) {
    case 1:
      detect_mon(crset_evil, TRUE);
      ma_duration(MA_DETECT_EVIL, 1);
      break;
    case 2:
      py_heal_hit(damroll(3, 3));
      break;
    case 3:
      ma_duration(MA_BLESS, randint(12) + 12);
      break;
    case 4:
      ma_clear(MA_FEAR);
      break;
    case 5:
      illuminate(uD.y, uD.x);
      break;
    case 6:
      detect_obj(oset_trap, TRUE);
      break;
    case 7:
      detect_obj(oset_doorstair, TRUE);
      break;
    case 8:
      if (countD.poison) countD.poison = MAX(1, countD.poison / 2);
      break;
    case 9:
      confuse_monster(dir, uD.y, uD.x);
      break;
    case 10:
      py_teleport(uD.lev * 3, y_ptr, x_ptr);
      break;
    case 11:
      py_heal_hit(damroll(4, 4));
      break;
    case 12:
      ma_duration(MA_BLESS, randint(24) + 24);
      break;
    case 13:
      sleep_monster_aoe(1);
      break;
    case 14:
      create_food(uD.y, uD.x);
      break;
    case 15:
      equip_remove_curse();
      break;
    case 16:
      ma_duration(MA_AFIRE, randint(10) + 10);
      ma_duration(MA_AFROST, randint(10) + 10);
      break;
    case 17:
      if (countD.poison > 0) countD.poison = 1;
      break;
    case 18:
      fire_ball(GF_HOLY_ORB, dir, uD.y, uD.x, (damroll(3, 6) + uD.lev),
                "Black Sphere");
      break;
    case 19:
      py_heal_hit(damroll(8, 4));
      break;
    case 20:
      ma_duration(MA_SEE_INVIS, randint(24) + 24);
      break;
    case 21:
      countD.life_prot += randint(25) + uD.lev;
      break;
    case 22:
      earthquake();
      break;
    case 23:
      map_area();
      break;
    case 24:
      py_heal_hit(damroll(16, 4));
      break;
    case 25:
      turn_undead();
      break;
    case 26:
      ma_duration(MA_BLESS, randint(48) + 48);
      break;
    case 27:
      dispel_creature(CD_UNDEAD, 3 * uD.lev);
      break;
    case 28:
      py_heal_hit(200);
      break;
    case 29:
      dispel_creature(CD_EVIL, 3 * uD.lev);
      break;
    case 30:
      warding_glyph(uD.y, uD.x);
      break;
    case 31:
      ma_clear(MA_FEAR);
      if (countD.poison > 0) countD.poison = 1;
      py_heal_hit(1000);
      for (int i = A_STR; i <= A_CHR; i++) res_stat(i);
      dispel_creature(CD_EVIL, 4 * uD.lev);
      turn_undead();
      maD[MA_INVULN] = 0;
      ma_duration(MA_INVULN, 3);
      break;
    default:
      break;
  }
  return 1;
}
void py_prayer(iidx, y_ptr, x_ptr) int* y_ptr;
int* x_ptr;
{
  struct objS* obj;
  uint32_t flags, first_spell;
  int cmana, sptype, spmask, spidx, dir;
  struct spellS* spelltable;

  obj = obj_get(invenD[iidx]);
  flags = obj->flags;
  if (py_affect(MA_BLIND))
    msg_print("You can't see to read the book.");
  else if (countD.confusion) {
    msg_print("You are too confused to read a book.");
  } else if (obj->tval == TV_PRAYER_BOOK && flags) {
    sptype = classD[uD.clidx].spell;
    first_spell = classD[uD.clidx].first_spell_lev - 1;
    if (sptype != SP_PRIEST)
      msg_print("You cannot believe the text written in this book.");
    else if (first_spell >= uD.lev)
      msg_print("You read of lineages and legends; what meaning do they hold?");
    else {
      cmana = uD.cmana;
      spmask = uspellmask();
      spelltable = uspelltable();

      if (last_castD == 0) {
        spidx = book_prompt(prayer_nameD, spelltable, spmask, flags);
        last_castD = spidx + 1;
        if (spidx < 0) return;
      } else {
        spidx = last_castD - 1;
      }

      if ((1 << spidx) & spmask) {
        dir = prayerdir_prompt(spidx);
        if (dir < 0) return;

        if (randint(100) < spell_chanceD[spidx]) {
          msg_print("You lost your concentration!");
        } else {
          prayer_dir(spidx, y_ptr, x_ptr, dir);

          if ((uD.spell_worked & (1 << spidx)) == 0) {
            uD.spell_worked |= (1 << spidx);
            uD.exp += spelltable[spidx].spexp * SP_EXP_MULT;
            py_experience();
          }
        }

        if (uD.cmana < spelltable[spidx].spmana) {
          if (statD.cur_stat[A_CON] > 3) {
            uD.cmana = 0;
            uD.cmana_frac = 0;
            msg_print("Your low mana has damaged your health!");
            dec_stat(A_CON);
          } else {
            msg_print("You are out of mana and your body is weak.");
          }
        } else {
          uD.cmana -= spelltable[spidx].spmana;
        }
      } else {
        MSG("You have no belief in the prayer of %s.", prayer_nameD[spidx]);
      }
      turn_flag = TRUE;
    }
  }
}
int
inven_try_staff(iidx, uy, ux)
int *uy, *ux;
{
  uint32_t flags, j;
  int k, chance, ident, known;
  struct objS* i_ptr;
  struct treasureS* tr_ptr;

  i_ptr = obj_get(invenD[iidx]);
  tr_ptr = &treasureD[i_ptr->tidx];
  known = tr_is_known(tr_ptr);
  if (i_ptr->tval == TV_STAFF) {
    chance = udevice() - i_ptr->level - 5;
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
            ident |= illuminate(uD.y, uD.x);
            break;
          case 2:
            ident |= detect_obj(oset_doorstair, known);
            break;
          case 3:
            ident |= detect_obj(oset_trap, known);
            break;
          case 4:
            ident |= detect_obj(oset_gold, known);
            break;
          case 5:
            ident |= detect_obj(oset_mon_pickup, known);
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
            ident |= sleep_monster_aoe(MAX_SIGHT);
            break;
          case 15:
            ident |= py_heal_hit(randint(8));
            break;
          case 16:
            if (detect_mon(crset_invisible, known)) {
              ma_duration(MA_DETECT_INVIS, 1);
              ident |= TRUE;
            }
            break;
          case 17:
            ident |= py_affect(MA_FAST) == 0;
            ma_duration(MA_FAST, randint(30) + 15);
            break;
          case 18:
            ident |= py_affect(MA_SLOW) == 0;
            ma_duration(MA_SLOW, randint(30) + 15);
            break;
          case 19:
            ident |= mass_poly();
            break;
          case 20:
            ident |= equip_remove_curse();
            break;
          case 21:
            if (detect_mon(crset_evil, known)) {
              ma_duration(MA_DETECT_EVIL, 1);
              ident |= TRUE;
            }
            break;
          case 22:
            if (countD.poison > 0) {
              ident = TRUE;
              countD.poison = 1;
            }
            ident |= ma_clear(MA_BLIND);
            if (countD.confusion > 0) {
              ident = TRUE;
              countD.confusion = 1;
            }
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
      if (!known) {
        if (ident) {
          tr_make_known(tr_ptr);
          tr_discovery(tr_ptr);
          py_experience();
        } else {
          tr_sample(tr_ptr);
          msg_print("You try the staff to unknown effect.");
        }
      }
    } else {
      msg_print("The staff has no charges left.");
      i_ptr->idflag |= ID_EMPTY;
    }
    turn_flag = TRUE;
    return TRUE;
  }

  return FALSE;
}
int
inven_flask(iidx)
{
  struct objS* flask;
  struct objS* light;

  light = obj_get(invenD[INVEN_LIGHT]);
  flask = obj_get(invenD[iidx]);
  if ((light->tval == TV_LIGHT && light->subval == 1) &&
      flask->tval == TV_FLASK) {
    inven_destroy_num(iidx, 1);
    light->p1 = CLAMP(light->p1 + 7500, 0, 15000);
    msg_print("Your lantern's light is renewed by pouring in a flask of oil.");
    turn_flag = TRUE;
    return TRUE;
  }
  return FALSE;
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
  int tval, subval, number;
  int stackweight, stacklimit;
  struct objS* obj = obj_get(obj_id);

  stackweight = ustackweight();
  tval = obj->tval;
  subval = obj->subval;
  number = obj->number;
  if (subval & STACK_ANY) {
    for (int it = 0; it < INVEN_EQUIP; ++it) {
      struct objS* i_ptr = obj_get(invenD[it]);
      if (tval == i_ptr->tval && subval == i_ptr->subval) {
        stacklimit = stacklimit_by_max_weight(stackweight, obj->weight);
        if (number + i_ptr->number <= stacklimit) {
          MSG("Merging %d items.", obj->number);
          obj->number += i_ptr->number;
          obj_unuse(i_ptr);
          invenD[it] = obj_id;
          *locn = it;
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}
int
weight_limit()
{
  int weight_cap;

  weight_cap = statD.use_stat[A_STR] * 130 + uD.wt;
  if (weight_cap > 3000) weight_cap = 3000;
  return (weight_cap);
}
void
inven_check_weight()
{
  int iwt, ilimit, penalty;
  iwt = 0;
  for (int it = 0; it < MAX_INVEN; ++it) {
    struct objS* obj = obj_get(invenD[it]);
    iwt += obj->weight * obj->number;
  }
  ilimit = weight_limit();

  penalty = iwt / (ilimit + 1);
  if (pack_heavy != penalty) {
    if (pack_heavy < penalty) {
      msg_print("Your pack is so heavy that it slows you down.");
    } else {
      msg_print("You move more easily under the weight of your pack.");
    }
    pack_heavy = penalty;
  }
}
int
inven_consume_flask()
{
  struct objS* obj;
  for (int it = 0; it < INVEN_EQUIP; ++it) {
    obj = obj_get(invenD[it]);
    if (obj->tval == TV_FLASK) {
      return inven_flask(it);
    }
  }

  return FALSE;
}
void
inven_check_light()
{
  struct objS* obj;
  int tohit;

  if (invenD[INVEN_LIGHT]) {
    obj = obj_get(invenD[INVEN_LIGHT]);

    if (obj->subval == 1 && obj->p1 <= 7500) {
      inven_consume_flask();
    }

    obj->p1 = MAX(obj->p1 - 1, 0);
    tohit = light_adj(obj->p1);
    if (tohit != obj->tohit) {
      obj->tohit = tohit;
      calc_bonuses();
    }
  }
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
static int
obj_sense(obj)
struct objS* obj;
{
  if (!oset_enchant(obj)) return FALSE;
  if (obj->idflag & (ID_REVEAL | ID_RARE | ID_MAGIK | ID_PLAIN | ID_DAMD))
    return FALSE;
  return TRUE;
}
static int
obj_rare(obj)
struct objS* obj;
{
  if ((obj->flags & TR_CURSED) == 0) {
    return obj->sn != 0;
  }
  return FALSE;
}
static int
obj_magik(obj)
struct objS* obj;
{
  if ((obj->flags & TR_CURSED) == 0) {
    if (obj->tohit > 0 || obj->todam > 0 || obj->toac > 0) return TRUE;
    if ((TR_P1 & obj->flags) && obj->p1 > 0) return TRUE;
    if (0x03fff880L & obj->flags) return TRUE;
  }

  return FALSE;
}
void
inven_wear(iidx)
{
  int eqidx;
  int swap_id;
  struct objS* obj;

  obj = obj_get(invenD[iidx]);
  eqidx = may_equip(obj->tval);

  // Ring is not worn unless a eqidx is open
  if (eqidx == INVEN_RING) eqidx = ring_slot();

  if (eqidx >= INVEN_EQUIP) {
    if (invenD[eqidx]) {
      swap_id = invenD[eqidx];
      if (equip_swap_into(eqidx, iidx)) {
        last_actuateD = swap_id;
      }
    } else {
      invenD[iidx] = 0;
    }

    if (invenD[eqidx] == 0) {
      invenD[eqidx] = obj->id;

      py_bonuses(obj, 1);
      obj_desc(obj, 1);
      MSG("You are wearing %s.", descD);
      if (eqidx == INVEN_BODY && obj->tohit) {
        MSG("Cumbersome armor makes more difficult to hit (%+d).", obj->tohit);
      }
      if (obj->flags & TR_CURSED) {
        msg_print("Oops! It feels deathly cold!");
        obj->cost = -1;
        obj->idflag |= ID_DAMD;
      } else if (obj_sense(obj)) {
        if (obj_rare(obj)) {
          msg_print("Eureka! It feels rare in origin.");
          obj->idflag |= ID_RARE;
        } else if (obj_magik(obj)) {
          MSG("There's something about what you are %s...",
              describe_use(eqidx));
          obj->idflag |= ID_MAGIK;
        } else {
          obj->idflag |= ID_PLAIN;
        }
      }
    }

    calc_bonuses();
    turn_flag = TRUE;
  }
}
int
bow_by_projectile(pidx)
{
  struct objS *obj, *objp;
  objp = obj_get(invenD[pidx]);
  obj = obj_get(invenD[INVEN_LAUNCHER]);

  return (obj->p1 == objp->p1) ? invenD[INVEN_LAUNCHER] : 0;
}
static int
obj_dupe_num(obj, num)
struct objS* obj;
{
  int id = obj_use()->id;
  if (id) {
    struct objS* other = obj_get(id);
    memcpy(other, obj, sizeof(struct objS));
    other->id = id;
    other->number = num;
    return id;
  }

  return 0;
}
static int
obj_thrown(obj, y, x)
struct objS* obj;
{
  struct caveS* c_ptr;
  int dropid;
  int flag, i, j, k;

  i = y;
  j = x;
  k = 0;
  flag = FALSE;
  do {
    if (in_bounds(i, j)) {
      c_ptr = &caveD[i][j];
      if (c_ptr->fval <= MAX_OPEN_SPACE && c_ptr->oidx == 0) flag = TRUE;
    }
    if (!flag) {
      i = y + randint(3) - 2;
      j = x + randint(3) - 2;
      k++;
    }
  } while (!flag && k <= 9);

  if (flag) {
    dropid = obj_dupe_num(obj, 1);
    c_ptr->oidx = dropid;
    return dropid != 0;
  }

  return FALSE;
}
void
inven_throw_dir(iidx, dir)
{
  int tbth, tpth, tdam, adj;
  int wtohit, wtodam;
  int y, x, fromy, fromx, cdis;
  int flag, drop;
  struct caveS* c_ptr;
  struct objS* obj;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;
  char tname[AL(descD)];

  if (invenD[INVEN_WIELD]) {
    obj = obj_get(invenD[INVEN_WIELD]);
    wtohit = -obj->tohit;
    wtodam = -obj->todam;
  } else {
    wtohit = wtodam = 0;
  }

  int bowid = bow_by_projectile(iidx);
  if (bowid) {
    obj = obj_get(bowid);
    obj_desc(obj, 1);
    obj_detail(obj);
    MSG("You grasp %s%s.", descD, detailD);

    wtohit += obj->tohit;
    wtodam += obj->todam;
  }

  obj = obj_get(invenD[iidx]);
  if (obj->tval == TV_PROJECTILE) {
    obj_desc(obj, 1);

    fromy = y = uD.y;
    fromx = x = uD.x;
    cdis = 0;
    flag = FALSE;
    drop = FALSE;
    do {
      mmove(dir, &y, &x);
      c_ptr = &caveD[y][x];
      cdis++;

      if (cdis > MAX_SIGHT || c_ptr->fval >= MIN_CLOSED_SPACE) {
        drop = TRUE;
      } else if (c_ptr->midx) {
        flag = TRUE;
        m_ptr = &entity_monD[c_ptr->midx];
        cr_ptr = &creatureD[m_ptr->cidx];

        adj = uD.lev * level_adj[uD.clidx][LA_BTHB];
        if (bowid) {
          tbth = uD.bowth;
        } else {
          tbth = uD.bowth * 24 / 32;
        }
        tpth = cbD.ptohit + obj->tohit + wtohit;
        if (m_ptr->mlit == 0) {
          tpth /= 2;
          tbth /= 2;
          adj /= 2;
        }
        tbth = tbth - cdis;

        if (test_hit(tbth, adj, tpth, cr_ptr->ac)) {
          strcpy(tname, descD);
          mon_desc(c_ptr->midx);
          descD[0] |= 0x20;
          MSG("You hear a cry as the %s strikes %s.", tname, descD);

          tdam = pdamroll(obj->damage) + obj->todam + wtodam;
          if (bowid) tdam *= obj_get(bowid)->damage[1];

          // TBD: named projectile weapons with damage multipliers?
          // tdam = tot_dam(obj, tdam, i);
          tdam = critical_blow(obj->weight, tpth, adj, tdam);
          if (tdam < 0) tdam = 0;

          if (mon_take_hit(c_ptr->midx, tdam)) {
            MSG("You have killed %s.", descD);
            py_experience();
          }
        } else {
          drop = TRUE;
        }
      }

      if (drop) {
        flag = TRUE;
        descD[0] &= ~0x20;
        if (randint(10) > 1 && obj_thrown(obj, fromy, fromx)) {
          MSG("%s strikes the dungeon floor.", descD);
        } else {
          MSG("%s breaks on the dungeon floor.", descD);
        }
      }

      fromy = y;
      fromx = x;
    } while (!flag);

    inven_destroy_num(iidx, 1);
    turn_flag = TRUE;
  }
}
void
py_throw(iidx)
{
  int dir;
  if (get_dir("Which direction will you launch a projectile?", &dir)) {
    if (countD.confusion) {
      msg_print("You are confused.");
      do {
        dir = randint(9);
      } while (dir == 5);
    }
    inven_throw_dir(iidx, dir);
  }
}
static void
py_offhand()
{
  struct objS* obj;
  int tmp, swap;
  tmp = invenD[INVEN_WIELD] ^ invenD[INVEN_AUX];
  swap = (tmp != 0);
  if (invenD[INVEN_WIELD]) {
    obj = obj_get(invenD[INVEN_WIELD]);
    if (obj->flags & TR_CURSED) {
      MSG("Hmm, the item you are %s seems to be cursed.",
          describe_use(INVEN_WIELD));
      swap = FALSE;
    } else {
      py_bonuses(obj, -1);
    }
  }

  if (swap) {
    obj = obj_get(invenD[INVEN_AUX]);
    invenD[INVEN_WIELD] ^= tmp;
    invenD[INVEN_AUX] ^= tmp;
    if (obj->id) {
      py_bonuses(obj, 1);
      obj_desc(obj, 1);
      MSG("primary weapon: %s.", descD);
    } else {
      msg_print("No primary weapon.");
    }
    calc_bonuses();
    turn_flag = TRUE;
  }
}
static int
door_try_close(y, x)
{
  struct caveS* c_ptr;
  struct objS* obj;
  int flag;
  c_ptr = &caveD[y][x];
  obj = &entity_objD[c_ptr->oidx];

  flag = 0;
  if (obj->tval == TV_OPEN_DOOR) {
    if (obj->p1 == 0) {
      flag = 1;
      obj->tval = TV_CLOSED_DOOR;
      obj->tchar = '+';
      c_ptr->fval = FLOOR_OBST;
    } else
      msg_print("The door appears to be broken.");
  }

  return flag;
}
static int
door_try_jam(y, x)
{
  struct caveS* c_ptr;
  struct objS* obj;
  int flag;
  c_ptr = &caveD[y][x];
  obj = &entity_objD[c_ptr->oidx];

  flag = 0;
  if (obj->tval == TV_CLOSED_DOOR) {
    flag = 1;
    /* Negative p1 values are "stuck", positive values are "locked"
       Successive spikes have a progressively smaller effect.
       Series is: 0 20 30 37 43 48 52 56 60 64 67 70 ... */
    obj->p1 = -1 * (1 + 190 / (10 + ABS(obj->p1)));
    // The player knows the door is stuck
    obj->idflag = ID_REVEAL;
  }

  return flag;
}
static int
try_spike_dir(dir)
{
  struct caveS* c_ptr;
  struct objS* obj;
  int y, x, ret;

  y = uD.y;
  x = uD.x;
  mmove(dir, &y, &x);
  c_ptr = &caveD[y][x];
  obj = &entity_objD[c_ptr->oidx];

  ret = 0;
  if (obj->tval == TV_OPEN_DOOR || obj->tval == TV_CLOSED_DOOR) {
    if (c_ptr->midx == 0) {
      if (door_try_close(y, x)) {
      }

      if (door_try_jam(y, x)) {
        ret = 1;
        msg_print("You jam the door with a spike.");
      }
    } else {
      msg_print("Something is in your way!");
    }

    // Costs a turn, otherwise can be abused for detecting invis monsters
    turn_flag = TRUE;
  } else {
    msg_print("You do not see anything you can close there.");
  }
  return ret;
}
void
py_spike(iidx)
{
  struct objS* obj;
  int dir;

  obj = obj_get(invenD[iidx]);
  if (obj->tval == TV_SPIKE) {
    if (get_dir("Which direction will you jam a door spike?", &dir)) {
      if (countD.confusion) {
        turn_flag = TRUE;
        msg_print("You are confused.");
        do {
          dir = randint(9);
        } while (dir == 5);
      }
      if (try_spike_dir(dir)) {
        inven_destroy_num(iidx, 1);
      }
    }
  }
}
int
show_version()
{
  int line;

  versionD[AL(versionD) - 1] = 0;

  screen_submodeD = 1;
  line = 0;
  BufMsg(screen, "Git Hash: %s", git_hashD);
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
  line += 1;
  BufMsg(screen, "Programming: %s", "Alan Newton");
  BufMsg(screen, "Art: %s", "Nathan Miller");

  DRAWMSG("Version %s", versionD);
  return inkey();
}
char*
ugender()
{
  if (uD.male)
    return "Male";
  else
    return "Female";
}
int
show_character(narrow)
{
  USE(msg_width);
  int line;
  int xbth, xbowth;
  int sptype;
  line = 0;
  int col[2];

  if (narrow || msg_width < 80) {
    screen_submodeD = 1;
    col[0] = 20;
    col[1] = 44;
    // Portrait col2 63
  } else {
    // Widescreen
    screen_submodeD = 2;
    col[0] = 25;
    col[1] = 53;
    // Landscape col2 92
  }

  BufMsg(screen, "%-13.013s: %3d", "Age", uD.age);
  BufMsg(screen, "%-13.013s: %3d", "Height", uD.ht);
  BufMsg(screen, "%-13.013s: %3d", "Weight", uD.wt);
  BufMsg(screen, "%-13.013s: %3d", "Social Class", uD.sc);

  BufPad(screen, MAX_A, col[0]);

  line = 0;
  for (int it = 0; it < MAX_A; ++it) {
    BufMsg(screen, "%c%-12.012s: %3d", stat_nameD[it][0] & ~0x20,
           &stat_nameD[it][1], statD.use_stat[it]);
    if (statD.max_stat[it] > statD.cur_stat[it]) {
      BufLineAppend(screen, line - 1, " %d",
                    statD.cur_stat[it] - statD.max_stat[it]);
    }
  }

  BufPad(screen, MAX_A, col[1]);
  line = 0;
  BufMsg(screen, "%s: %-6.06s", "Gender", ugender());

  line = MAX_A + 1;
  BufMsg(screen, "%-13.013s: %3d", "+ To Hit", cbD.ptohit - cbD.hide_tohit);
  BufMsg(screen, "%-13.013s: %3d", "+ To Damage", cbD.ptodam - cbD.hide_todam);
  BufMsg(screen, "%-13.013s: %3d", "+ To Armor", cbD.ptoac - cbD.hide_toac);
  BufMsg(screen, "%-13.013s: %3d", "Total Armor", cbD.pac - cbD.hide_toac);

  BufPad(screen, MAX_A * 2, col[0]);

  line = MAX_A + 1;
  BufMsg(screen, "%-13.013s: %7d", "Level", uD.lev);
  BufMsg(screen, "%-13.013s: %7d", "Experience", uD.exp);
  BufMsg(screen, "%-13.013s: %7d", "Max Exp", uD.max_exp);
  BufMsg(screen, "%-13.013s: %7d", "Exp to Adv", lev_exp(uD.lev));
  BufMsg(screen, "%-13.013s: %7d", "Gold", uD.gold);

  BufPad(screen, MAX_A * 2, col[1]);

  line = MAX_A + 1;
  BufMsg(screen, "%-15.015s: %3d", "Max Hit Points", uD.mhp);
  BufMsg(screen, "%-15.015s: %3d", "Cur Hit Points", uD.chp);
  BufMsg(screen, "%-15.015s: %3d", "Max Mana", uD.mmana);
  BufMsg(screen, "%-15.015s: %3d", "Cur Mana", uD.cmana);

  xbth = uD.bth + uD.lev * level_adj[uD.clidx][LA_BTH];
  xbowth = uD.bowth + uD.lev * level_adj[uD.clidx][LA_BTHB];
  line = 2 * MAX_A + 1;
  BufMsg(screen, "%-13.013s: %3d", "Fighting", xbth);
  BufMsg(screen, "%-13.013s: %3d", "Bows", xbowth);
  BufMsg(screen, "%-13.013s: %3d", "Saving Throw", usave());
  sptype = classD[uD.clidx].spell;
  if (sptype) {
    BufMsg(screen, "%-13.013s: %d",
           sptype == SP_MAGE ? "Spell Memory" : "Prayer Memory", uspellcount());
  }
  BufPad(screen, MAX_A * 3, col[0]);

  line = 2 * MAX_A + 1;
  BufMsg(screen, "%-13.013s: %3d", "Stealth", uD.stealth);
  BufMsg(screen, "%-13.013s: %3d", "Disarming", udisarm());
  BufMsg(screen, "%-13.013s: %3d", "Magic Device", udevice());
  BufPad(screen, MAX_A * 3, col[1]);

  line = 2 * MAX_A + 1;
  BufMsg(screen, "%-15.015s: %3d", "Perception", MAX(40 - uD.fos, 0));
  BufMsg(screen, "%-15.015s: %3d", "Searching", uD.search);
  BufMsg(screen, "%-15.015s: %3d", "Infra-Vision", uD.infra);

  int history_used = 0;
  int history_len = strlen(historyD);
  line = AL(screenD) - 4;
  enum { HISTORY_WIDTH = 64 };
  for (int it = 0; it < 4; ++it) {
    char* begin = &historyD[history_used];
    int len = 0;
    if (history_used + HISTORY_WIDTH < history_len) {
      char* end = begin + HISTORY_WIDTH;
      while (*end != ' ') --end;
      len = end - begin;
    } else {
      len = history_len - history_used;
    }

    if (len > 0) {
      screen_usedD[line] = len;
      memcpy(&screenD[line], begin, len);
      line += 1;
      history_used += len + 1;
    }
  }

  if (msg_width < 80) {
    DRAWMSG("Name: %-13.013s Race: %-17.017s Class: %-13.013s", "...",
            raceD[uD.ridx].name, classD[uD.clidx].name);
  } else {
    DRAWMSG("Name: %-18.018s Race: %-21.021s Class: %-13.013s", "...",
            raceD[uD.ridx].name, classD[uD.clidx].name);
  }
  return inkey();
}

// Bounds checks on variables that may cause memory corruption
int
saveslot_validation()
{
  if (uD.lev <= 0 || uD.lev > MAX_PLAYER_LEVEL) return 0;
  if (uD.clidx >= AL(classD)) return 0;
  if (uD.ridx >= AL(raceD)) return 0;
  for (int it = 0; it < MAX_A; ++it) {
    if (statD.max_stat[it] < 3 || statD.max_stat[it] > 118) return 0;
    if (statD.cur_stat[it] < 3 || statD.max_stat[it] > 118) return 0;
  }
  return 1;
}
void
summary_update(struct summaryS* summary)
{
  summary->invalid = !saveslot_validation();
  summary->slevel = uD.lev;
  summary->sclass = uD.clidx;
  summary->srace = uD.ridx;
  summary->sdepth = MAX(uD.max_dlv, dun_level) * 50;
}
int
py_archive_read(summary, external)
struct summaryS* summary;
{
  int save_count = 0;
  memset(summary, 0, sizeof(struct summaryS) * AL(classD));
  for (int it = 0; it < AL(classD); ++it) {
    if (platformD.load(it, external)) {
      save_count += 1;
      summary_update(&summary[it]);
    }
  }
  return save_count;
}
int
summary_saveslot_deletion(struct summaryS* summary, int saveslot, int external)
{
  int line;
  char c;

  overlay_submodeD = 0;
  do {
    line = 0;
    if (summary->invalid || summary->slevel == 0) {
      c = 'a';
    } else {
      BufMsg(overlay, "a) Alright, delete this character");
      line += 1;
      BufMsg(overlay, "c) Cancel deletion");
      DRAWMSG("Delete level %d %s?", summary->slevel,
              classD[summary->sclass].name);
      c = inkey();
    }

    int ret = 0;
    switch (c) {
      case 'a':
        ret = platformD.erase(saveslot, external);
        memset(summary, 0, sizeof(struct summaryS));
      case 'c':
        return ret;
    }
  } while (!is_ctrl(c));
  return 0;
}
int
saveslot_race_reroll(saveslot, race)
{
  char c = 'o';
  do {
    if (c == 'o') py_race_class_seed_init(race, saveslot, platformD.seed());
    c = show_character(1);
    if (c == CTRL('c')) break;
  } while (c != ESCAPE);
  return 1;
}
int
saveslot_creation(int saveslot)
{
  int line;
  char c;
  uint8_t iidx;
  overlay_submodeD = 'c';

  // Clear game state
  memset(__start_game, 0, __stop_game - __start_game);

  do {
    line = 0;
    for (int it = 0; it < AL(raceD); ++it) {
      if (raceD[it].rtclass & (1 << saveslot)) {
        BufMsg(overlay, "%c) %s", 'a' + it, raceD[it].name);
      } else {
        line += 1;
      }
    }
    DRAWMSG("Play a %s of which race?", classD[saveslot].name);
    c = inkey();
    if (c == CTRL('c')) break;

    iidx = c - 'a';
    if (iidx >= 0 && iidx <= AL(raceD)) {
      if (raceD[iidx].rtclass & (1 << saveslot)) {
        return saveslot_race_reroll(saveslot, iidx);
      }
    }
  } while (c != ESCAPE);
  return 0;
}
int
py_saveslot_select()
{
  struct summaryS in_summary[AL(classD)];
  struct summaryS ex_summary[AL(classD)];
  int line;
  char c;
  int save_count;
  int extern_count = 0;
  uint8_t iidx;
  int has_external = (platformD.saveex != 0);
  int using_external = 0;

  // Disable midpoint resume explicitly
  input_resumeD = -1;

  save_count = py_archive_read(in_summary, 0);
  // Assist with import on re-installation
  if (save_count == 0 && has_external) {
    extern_count = py_archive_read(ex_summary, 1);
  }

  do {
    // Wait for user selection of class
    globalD.saveslot_class = -1;

    iidx = -1;
    line = 0;
    overlay_submodeD = using_external ? 'E' : 'I';
    char* media = using_external ? "External" : "Internal";
    char* other_media = using_external ? "Internal" : "External";
    struct summaryS* summary = using_external ? ex_summary : in_summary;
    for (int it = 0; it < AL(classD); ++it) {
      if (summary[it].invalid) {
        BufMsg(overlay, "%c) Invalid %s data file", 'a' + it, classD[it].name);
      } else if (summary[it].slevel) {
        BufMsg(overlay, "%c) Level %d %s %s (%d feet)", 'a' + it,
               summary[it].slevel, raceD[summary[it].srace].name,
               classD[summary[it].sclass].name, summary[it].sdepth);
      } else {
        if (using_external) {
          BufMsg(overlay, " ");
        } else {
          BufMsg(overlay, "%c)    <create %s>", 'a' + it, classD[it].name);
        }
      }
    }

    if (has_external) {
      if (!using_external) {
        line = 's' - 'a';
        BufMsg(overlay, "s) Save all to external media");
      } else if (extern_count) {
        line = 'i' - 'a';
        BufMsg(overlay, "i) Import all");
      }

      line = 'v' - 'a';
      BufMsg(overlay, "v) View %s media", other_media);
    }

    DRAWMSG("%s Media Archive: Play which class?", media);
    c = inkey();
    // Deletion
    if (c == ESCAPE) {
      if (platformD.selection) {
        int srow, scol;
        platformD.selection(&scol, &srow);
        if (srow >= 0 && srow < AL(classD)) {
          if (summary_saveslot_deletion(&summary[srow], srow, using_external))
            extern_count -= (using_external);
        }
      }
      continue;
    }
    // Import
    if (c == 'i') {
      if (using_external) {
        int count = 0;
        for (int it = 0; it < AL(classD); ++it) {
          if (platformD.load(it, 1)) {
            if (saveslot_validation()) {
              if (platformD.save()) {
                summary_update(&in_summary[it]);
                count += 1;
              }
            }
          }
        }

        line = 0;
        BufMsg(overlay, "Loaded characters (x%d)", count);
        DRAWMSG("%s Media Update", other_media);
        inkey();
        // view swap
        c = 'v';
      }
    }
    // Export
    if (c == 's') {
      if (platformD.saveex) {
        int count = platformD.saveex();
        line = 0;
        BufMsg(overlay, "Saved characters (x%d)", count);
        DRAWMSG("%s Media Update", other_media);
        inkey();
        // view swap
        extern_count = 0;
        c = 'v';
      }
    }
    if (c == 'v') {
      using_external = !using_external;
      if (using_external && extern_count == 0)
        extern_count = py_archive_read(ex_summary, 1);
    }

    iidx = c - 'a';
    if (iidx <= AL(classD)) {
      if (summary[iidx].invalid) continue;

      if (summary[iidx].slevel == 0) {
        if (!saveslot_creation(iidx)) continue;

        py_inven_init();
        inven_sort();

        globalD.saveslot_class = uD.clidx;
        return platformD.save();
      }

      if (summary[iidx].slevel) {
        if (platformD.load(iidx, using_external)) {
          if (using_external) return platformD.save();
          return 1;
        }
      }
    }
  } while (c != CTRL('c'));

  return 0;
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
    inven_overlay(INVEN_EQUIP, MAX_INVEN);
    if (in_subcommand("Take off which item?", &c)) {
      uint8_t iidx = INVEN_EQUIP + (c - 'a');
      if (iidx < MAX_INVEN) {
        if (invenD[iidx]) {
          into = inven_slot();
          if (into >= 0) {
            equip_swap_into(iidx, into);
          }
        }
      }
    }
  }
}
static int
py_grave()
{
  screen_submodeD = 0;
  int row, col;
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
  DRAWMSG("Killed by %s. (CTRL-P log) (C/o/v/ESC)", death_descD);
  return inkey();
}
static void
show_all_inven()
{
  int iidx;
  overlay_submodeD = 'e';
  do {
    int show_equip = overlay_submodeD == 'e';
    iidx = inven_choice("At death", show_equip ? "/*" : "*/");

    if (iidx >= 0) {
      obj_study(obj_get(invenD[iidx]), 0);
    }
  } while (iidx != -1);
}
int
py_reset()
{
  char c;
  int line;
  overlay_submodeD = -1;
  do {
    line = 0;
    BufMsg(overlay, "a) Archive this %s for future play",
           classD[uD.clidx].name);
    BufMsg(overlay, "-");
    BufMsg(overlay, "-");
    BufMsg(overlay, "d) Delete this character");
    DRAWMSG("Game Reset");
    c = inkey();

    if (c == 'd') platformD.erase(globalD.saveslot_class, 0);

    switch (c) {
      case 'a':
        // fs1 overwrite the saveslot_class; cease using "savechar" file
        if (globalD.fsversion == 1) platform_upgrade();
      case 'd':
        globalD.fsversion = 2;
        globalD.saveslot_class = -1;
        longjmp(restartD, 1);
    }
  } while (!is_ctrl(c));
  return -1;
}
int
py_menu()
{
  char c;
  int line, input_action, memory_ok;
  char* prompt;
  int death;

  death = (uD.new_level_flag == NL_DEATH);
  input_action = input_action_usedD;

  if (death) {
    prompt = "You are dead.";
  } else {
    prompt = "Advanced Game Actions";
  }

  while (1) {
    // One command may be written ahead (e.g. py_look) and is invalid for replay
    memory_ok = (input_record_writeD <= AL(input_recordD) - 1 &&
                 input_action_usedD <= AL(input_actionD) - 1);

    overlay_submodeD = 0;
    line = 0;
    BufMsg(overlay, death ? "a) All equipment / inventory "
                          : "a) Await event (health, malady, or recall)");
    if (HACK) {
      BufMsg(overlay, "b) Undo / Gameplay Rewind (%d) (%d) (%s)", input_action,
             input_record_writeD, memory_ok ? "memory OK" : "memory FAIL");
    } else {
      BufMsg(overlay, "b) Undo / Gameplay Rewind (%s)",
             memory_ok ? "memory OK" : "memory FAIL");
    }
    BufMsg(overlay, "-");
    BufMsg(overlay, "d) Dungeon reset");
    BufMsg(overlay, "-");
    BufMsg(overlay, "-");
    BufMsg(overlay, "g) Game reset");

    if (!in_subcommand(prompt, &c)) break;

    switch (c) {
      case 'a':
        if (!death) {
          py_rest();
          return 0;
        }

        show_all_inven();
        continue;

      case 'b':
        if (!memory_ok) return 0;

        input_resumeD = (input_action - 1);
        // Disable midpoint resume explicitly
        if (input_resumeD == 0) input_resumeD = -1;
        longjmp(restartD, 1);

      case 'd':
        input_resumeD = -1;
        longjmp(restartD, 1);

      case 'g':
        py_reset();
        return 0;
    }
  }
  return 0;
}
static void
py_death()
{
  char c;

  do {
    c = 0;
    do {
      if (c == CTRL('p')) {
        c = show_history();
      } else if (c == 'C') {
        c = show_character(1);
      } else if (c == 'o') {
        // Observe game state at time of death
        draw();
        c = inkey();
      } else if (c == 'v') {
        c = show_version();
      } else {
        c = py_grave();
      }

      if (c == CTRL('c')) return;
    } while (c != ESCAPE);

    py_menu();
  } while (1);
}
static void
py_help()
{
  int line = 0;

  screen_submodeD = 1;
  BufMsg(screen, "c: close door/chest");
  BufMsg(screen, "d: drop object");
  BufMsg(screen, "e: equipment");
  BufMsg(screen, "f: force/bash chest/door/monster");
  BufMsg(screen, "i: inventory");
  BufMsg(screen, "q: quaff potion");
  BufMsg(screen, "r: read scroll");
  BufMsg(screen, "o: open object");
  BufMsg(screen, "s: search for traps/doors");
  BufMsg(screen, "v: version info");
  BufMsg(screen, "w: wear object");
  BufMsg(screen, "x: examine objects/monsters");
  BufMsg(screen, "z: zap wand");
  BufMsg(screen, "<: up stairs");
  BufMsg(screen, ">: down stairs");
  BufMsg(screen, "=: Advanced Game Actions");
  BufMsg(screen, "+/-: Zoom in/out");

  BufPad(screen, AL(screenD), 34);

  line = 0;
  BufMsg(screen, "A: actuate inventory item");
  BufMsg(screen, "C: character screen");
  BufMsg(screen, "D: disarm trap");
  BufMsg(screen, "E: eat object");
  BufMsg(screen, "I: inventory sort");
  BufMsg(screen, "M: map dungeon");
  BufMsg(screen, "R: rest until healed");
  BufMsg(screen, "S: study an object");
  BufMsg(screen, "T: take off equipment");
  BufMsg(screen, "W: where about the dungeon");
  BufMsg(screen, "X: exchange primary weapon to offhand");
  BufMsg(screen, "Z: staff invocation");
  BufMsg(screen, ",: pickup object");
  BufMsg(screen, ".: automatic dungeon interaction");
  line += 1;
  BufMsg(screen, "CTRL-a: repeat last actuate");
  BufMsg(screen, "CTRL-p: message history");
  BufMsg(screen, "CTRL-x: save and exit (test)");
  msg_print("? - help");

  if (HACK) {
    msg_pause();
    msg_print("Hack Commands");

    line = 0;
    BufMsg(screen, "CTRL('f'): food");
    BufMsg(screen, "CTRL('h'): heal");
    BufMsg(screen, "CTRL('t'): teleport");
    BufMsg(screen, "CTRL('m'): teleport-to-monster");
    BufMsg(screen, "CTRL('o'): teleport-to-object");
  }
}
int inven_damage(typ, perc) int (*typ)();
{
  int it, j;

  j = 0;
  for (it = 0; it < INVEN_EQUIP; it++) {
    struct objS* obj = obj_get(invenD[it]);
    if ((*typ)(obj) && (randint(100) < perc)) {
      inven_destroy_num(it, 1);
      j++;
    }
  }
  return (j);
}
int
minus_ac(verbose)
{
  int j;
  int minus;
  struct objS* obj;

  minus = FALSE;
  j = equip_random();
  if (j >= 0) {
    obj = obj_get(invenD[j]);
    obj_desc(obj, 1);
    if (obj->flags & TR_RES_ACID) {
      MSG("Your %s resists damage.", descD);
      minus = TRUE;
    } else if ((obj->ac + obj->toac) > 0) {
      MSG("Your %s is damaged.", descD);
      obj->toac--;
      calc_bonuses();
      minus = TRUE;
    } else {
      obj->idflag |= ID_CORRODED;
    }
  }

  if (verbose && !minus) msg_print("Acid is on your flesh!");

  return (minus);
}
int
poison_gas(dam)
{
  py_take_hit(dam / 2);
  countD.poison += 12 + dam / 2;
  return dam;
}
int
fire_dam(dam)
{
  int absfire, resfire;

  absfire = py_affect(MA_AFIRE);
  resfire = py_tr(TR_RES_FIRE);

  if (absfire) dam = dam / 2;
  if (resfire) dam = dam / 2;
  py_take_hit(dam);
  if (!absfire && inven_damage(vuln_fire, 3) > 0)
    msg_print("There is smoke coming from your pack!");
  return dam;
}
int
acid_dam(dam, verbose)
{
  if (minus_ac(verbose)) dam = dam / 2;
  if (py_tr(TR_RES_ACID)) dam = dam / 2;
  py_take_hit(dam);
  if (inven_damage(vuln_acid, 3) > 0)
    msg_print("There is an acrid smell coming from your pack!");
  return dam;
}
int
frost_dam(dam)
{
  int abscold, rescold;

  abscold = py_affect(MA_AFROST);
  rescold = py_tr(TR_RES_COLD);
  if (abscold) dam = dam / 2;
  if (rescold) dam = dam / 2;
  py_take_hit(dam);
  if (!abscold && inven_damage(vuln_frost, 5) > 0)
    msg_print("Something shatters inside your pack!");
  return dam;
}
int
light_dam(dam)
{
  if (py_tr(TR_RES_LIGHT)) dam = dam / 2;
  py_take_hit(dam);
  if (inven_damage(vuln_lightning, 3) > 0)
    msg_print("There are sparks coming from your pack!");
  return dam;
}
void
corrode_gas(verbose)
{
  if (!minus_ac(verbose)) py_take_hit(randint(8));
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
  if (py_affect(MA_FEAR)) {
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
      MSG("You bash %s.", descD);
      k = pdamroll(shield->damage);
      k = critical_blow(shield->weight / 4 + statD.use_stat[A_STR], 0, adj, k);
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
  int k, blows;
  int base_tohit, lev_adj, tohit, todam, creature_ac;

  int midx = caveD[y][x].midx;
  struct monS* mon = &entity_monD[midx];
  struct creatureS* cre = &creatureD[mon->cidx];
  struct objS* obj = obj_get(invenD[INVEN_WIELD]);

  turn_flag = TRUE;
  if (py_affect(MA_FEAR)) {
    msg_print("You are too afraid!");
  } else {
    tohit = cbD.ptohit;
    todam = cbD.ptodam;
    base_tohit = uD.bth;
    lev_adj = uD.lev * level_adj[uD.clidx][LA_BTH];
    // reduce hit if monster not lit
    if (mon->mlit == 0) {
      tohit /= 2;
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
    }

    mon->msleep = 0;
    mon_desc(midx);
    descD[0] = descD[0] | 0x20;
    creature_ac = cre->ac;
    /* Loop for number of blows,  trying to hit the critter.	  */
    for (int it = 0; it < blows; ++it) {
      if (test_hit(base_tohit, lev_adj, tohit, creature_ac)) {
        MSG("You hit %s.", descD);
        if (obj->tval) {
          k = pdamroll(obj->damage);
          k = tot_dam(obj, k, mon->cidx);
          k = critical_blow(obj->weight, tohit, lev_adj, k);
        } else {
          k = damroll(1, 1);
          k = critical_blow(1, tohit, lev_adj, k);
        }
        k += todam;

        if (uD.melee_genocide) {
          uD.melee_genocide = 0;
          msg_print("Your hands stop glowing black.");
          if ((cre->cmove & CM_WIN) == 0) {
            genocide(mon->cidx);
            midx = 0;
          }
        } else if (uD.melee_confuse) {
          uD.melee_confuse = 0;
          msg_print("Your hands stop glowing blue.");
          if ((cre->cdefense & CD_NO_SLEEP) ||
              randint(MAX_MON_LEVEL) < cre->level) {
            MSG("%s is unaffected by confusion.", descD);
          } else {
            MSG("%s appears confused.", descD);
            mon->mconfused += 2 + randint(16);
          }
        }

        /* See if we done it in.  			 */
        if (midx == 0 || mon_take_hit(midx, k)) {
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

  int adj = cre->level * CRE_LEV_ADJ;
  for (int it = 0; it < AL(cre->attack_list); ++it) {
    if (uD.new_level_flag) break;
    if (!cre->attack_list[it]) break;
    mon_desc(midx);
    struct attackS* attack = &attackD[cre->attack_list[it]];

    int attack_type = attack->attack_type;
    int attack_desc = attack->attack_desc;
    bth = bth_adj(attack_type);
    flag = test_hit(bth, adj, 0, cbD.pac);
    if (countD.life_prot && attack_type == 19) {
      attack_type = 99;
      attack_desc = 99;
    }
    if (flag) {
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
            if (maD[MA_FEAR] == 0) msg_print("You remain bold.");
          } else if (maD[MA_FEAR] == 0) {
            msg_print("You are suddenly afraid!");
            ma_combat(MA_FEAR, 3 + randint(cre->level));
          } else {
            ma_combat(MA_FEAR, 3);
          }
          break;
        case 5: /*Fire attack  */
          msg_print("You are enveloped in flame!");
          fire_dam(damage);
          break;
        case 6: /*Acid attack  */
          msg_print("You are covered in acid!");
          acid_dam(damage, FALSE);
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
          corrode_gas(FALSE);
          py_take_hit(damage);
          break;
        case 10: /*Blindness attack*/
          py_take_hit(damage);
          if (maD[MA_BLIND]) {
            ma_combat(MA_BLIND, 5);
          } else {
            msg_print("Your eyes begin to sting.");
            ma_combat(MA_BLIND, 10 + randint(cre->level));
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
          if (!countD.paralysis && randint(124) < statD.use_stat[A_DEX])
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
              inven_destroy_num(i, 1);
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
          if (py_lose_experience(damage + (uD.exp / 100) * MON_DRAIN_EXP)) {
            msg_print("You feel your life draining away!");
          }
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
            inven_destroy_num(l, 1);
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
        case 99: {
        } break;
      }

      if (uD.melee_confuse) {
        uD.melee_confuse = 0;
        msg_print("Your hands stop glowing blue.");
        mon_desc(midx);
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
  int y, x, dir;
  struct caveS* c_ptr;
  struct objS* obj;

  y = uD.y;
  x = uD.x;
  if (get_dir(0, &dir)) {
    mmove(dir, &y, &x);
    c_ptr = &caveD[y][x];
    obj = &entity_objD[c_ptr->oidx];

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
      msg_print("You do not see anything you can close there.");
    }
  }
}
void
chest_trap(y, x)
{
  int i;
  struct objS* obj;

  obj = &entity_objD[caveD[y][x].oidx];
  if (CH_LOSE_STR & obj->flags) {
    msg_print("A small needle has pricked you!");
    strcpy(death_descD, "a poison needle");
    py_take_hit(damroll(1, 4));
    lose_stat(A_STR);
  }
  if (CH_POISON & obj->flags) {
    msg_print("A small needle has pricked you!");
    strcpy(death_descD, "a poison needle");
    py_take_hit(damroll(1, 6));
    countD.poison += 10 + randint(20);
  }
  if (CH_PARALYSED & obj->flags) {
    msg_print("A puff of yellow gas surrounds you!");
    if (TR_FREE_ACT & cbD.tflag)
      msg_print("You are unaffected.");
    else {
      msg_print("You choke and pass out.");
      countD.paralysis = 10 + randint(20);
    }
  }
  if (CH_SUMMON & obj->flags) {
    msg_print("A strange rune on the chest glows and fades.");
    for (i = 0; i < 3; i++) {
      summon_monster(uD.y, uD.x);
    }
  }
  if (CH_EXPLODE & obj->flags) {
    msg_print("There is a sudden explosion!");
    delete_object(y, x);
    strcpy(death_descD, "an exploding chest");
    py_take_hit(damroll(5, 8));
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
    obj_desc(obj, 1);
    chance = udisarm();
    if (chance + 100 - obj->level > randint(100)) {
      MSG("You have disarmed %s.", descD);
      uD.exp += obj->p1;
      delete_object(y, x);
      py_experience();
    } else {
      MSG("You fail to disarm %s.", descD);
    }
  }
}
void
try_disarm_chest(y, x)
{
  struct caveS* c_ptr;
  struct objS* obj;
  int chance;

  c_ptr = &caveD[y][x];
  obj = &entity_objD[c_ptr->oidx];
  if ((obj->idflag & ID_REVEAL) && (CH_TRAPPED & obj->flags)) {
    // TBD: div is used; verify this number is positive. clean-up code.
    chance = uD.disarm + 2 * todis_adj() + think_adj(A_INT) +
             level_adj[uD.clidx][LA_DISARM] * uD.lev / 3;
    if (countD.confusion) chance /= 8;
    if ((chance - obj->level) > randint(100)) {
      obj->flags &= ~CH_TRAPPED;
      if (CH_LOCKED & obj->flags)
        obj->sn = SN_LOCKED;
      else
        obj->sn = SN_DISARMED;
      msg_print("You have disarmed the chest.");
      obj->idflag = ID_REVEAL;
      uD.exp += obj->level;
      py_experience();
    } else {
      msg_print("You failed to disarm the chest.");
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

    if (obj->tval != TV_VIS_TRAP && obj->tval != TV_CHEST) {
      msg_print("You do not see anything to disarm there.");
    }
    if (c_ptr->midx) {
      // Prevent invis-detection via disarm: no free turn
      msg_print("Something is in your way!");
    } else if (obj->tval == TV_VIS_TRAP) {
      *uy = y;
      *ux = x;
    } else if (obj->tval == TV_CHEST) {
      if ((obj->idflag & ID_REVEAL) == 0)
        msg_print("You don't see a trap on the chest.");
      else if ((obj->flags & CH_TRAPPED) == 0)
        msg_print("The chest is not trapped.");
      else {
        *uy = y;
        *ux = x;
      }
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
static void
py_drop()
{
  int iidx;
  drop_modeD = 1;
  iidx = inven_choice("Drop which item?", "*/");

  if (iidx >= 0) inven_drop(iidx);
  drop_modeD = 0;
}
static void py_bash(uy, ux) int *uy, *ux;
{
  int y, x, dir;

  y = uD.y;
  x = uD.x;
  if (get_dir(0, &dir)) {
    if (countD.confusion) {
      turn_flag = TRUE;
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
  int chance, flag;
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
  } else if (obj->tval == TV_CHEST) {
    if (c_ptr->midx) {
      msg_print("Something is in your way!");
    } else {
      chance = uD.disarm + 2 * todis_adj() + think_adj(A_INT) +
               (level_adj[uD.clidx][LA_DISARM] * uD.lev / 3);
      flag = FALSE;
      if (CH_LOCKED & obj->flags)
        if (countD.confusion)
          msg_print("You are too confused to pick the lock.");
        else if ((chance - obj->level) > randint(100)) {
          msg_print("You have picked the lock.");
          flag = TRUE;
          uD.exp += obj->level;
          py_experience();
        } else
          msg_print("You failed to pick the lock.");
      else
        flag = TRUE;
      if (flag) {
        msg_print("You open the chest.");
        obj->flags &= ~CH_LOCKED;
        obj->sn = SN_EMPTY;
        obj->idflag = ID_REVEAL;
        obj->cost = 0;
      }
      flag = FALSE;
      /* Was chest still trapped?   (Snicker)   */
      if ((CH_LOCKED & obj->flags) == 0) {
        chest_trap(y, x);
        if (c_ptr->oidx) flag = TRUE;
      }
      /* Chest treasure is allocated as if a creature   */
      /* had been killed.  			   */
      if (flag) {
        /* clear the cursed chest/monster win flag, so that people
           can not win by opening a cursed chest */
        obj->flags &= ~TR_CURSED;
        mon_death(y, x, obj->flags);
        obj->flags = 0;
      }
    }
  } else {
    msg_print("You do not see anything you can open there.");
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
  if (countD.confusion) chance /= 8;
  if (py_affect(MA_BLIND)) chance /= 8;
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
          c_ptr->cflag |= CF_FIELDMARK;
        } else if (obj->tval == TV_INVIS_TRAP || obj->tval == TV_VIS_TRAP) {
          if ((obj->idflag & ID_REVEAL) == 0) {
            obj->idflag |= ID_REVEAL;
            obj->tval = TV_VIS_TRAP;
            obj->tchar = '^';
            c_ptr->cflag |= CF_FIELDMARK;
            obj_desc(obj, 1);
            MSG("You have found %s.", descD);
          }
        } else if (obj->tval == TV_CHEST) {
          if (CH_TRAPPED & obj->flags) {
            obj->idflag = ID_REVEAL;
            msg_print("The chest is trapped!");
          }
        }
      }
  turn_flag = TRUE;
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
  /* Yarr! */
  else if (obj->tval == TV_CHEST) {
    if (obj->sn != SN_EMPTY) {
      if (obj->idflag & ID_REVEAL) try_disarm_chest(y, x);
      open_object(y, x);
      turn_flag = TRUE;
    } else {
      msg_print("The chest is empty.");
    }
  }
  /* There's GOLD in them thar hills!      */
  else if (obj->tval == TV_GOLD) {
    uD.gold += obj->cost;
    MSG("You found %d gold pieces worth of %s.", obj->cost,
        gold_nameD[obj->subval]);
    delete_object(y, x);
    turn_flag = TRUE;
  } else if (obj->tval <= TV_MAX_PICK_UP) {
    locn = -1;
    merge = inven_merge(obj->id, &locn);
    if (!merge && pickup) locn = inven_carry(obj->id);

    obj_desc(obj, obj->number);
    obj_detail(obj);
    if (locn >= 0) {
      obj->fy = 0;
      obj->fx = 0;
      caveD[y][x].oidx = 0;

      MSG("You have %s%s (%c).", descD, detailD, locn + 'a');
      turn_flag = TRUE;
    } else if (!pickup) {
      MSG("You see %s%s here.", descD, detailD);
    } else {
      MSG("You can't carry %s%s.", descD, detailD);
    }
  }
}
int
py_monlook_dir(dir)
{
  int y, x, oy, ox, ly, lx, seen;
  seen = 0;
  y = uD.y;
  x = uD.x;
  ly = dir_y(dir);
  lx = dir_x(dir);
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
  return seen;
}
int
py_objlook_dir(dir)
{
  int y, x, oy, ox, ly, lx, seen;

  y = uD.y;
  x = uD.x;
  ly = dir_y(dir);
  lx = dir_x(dir);
  FOR_EACH(obj, {
    if (obj->fy == 0 || obj->fx == 0) continue;
    if (obj->tval == TV_INVIS_TRAP) continue;
    if (distance(y, x, obj->fy, obj->fx) <= MAX_SIGHT) {
      oy = (ly != 0) * (-((obj->fy - y) < 0) + ((obj->fy - y) > 0));
      ox = (lx != 0) * (-((obj->fx - x) < 0) + ((obj->fx - x) > 0));
      if (oy == ly && ox == lx && (CF_VIZ & caveD[obj->fy][obj->fx].cflag) &&
          los(y, x, obj->fy, obj->fx)) {
        seen += 1;
        obj_desc(obj, obj->number);
        MSG("You see %s.", descD);
        msg_pause();
      }
    }
  });

  return seen;
}
static void
py_examine()
{
  int dir;
  char* type;

  type = 0;
  if (py_affect(MA_BLIND))
    msg_print("You can't see a thing!");
  else {
    if (get_dir("Which direction will you look?", &dir)) {
      if (py_monlook_dir(dir))
        type = "monsters";
      else if (py_objlook_dir(dir))
        type = "objects";
      else
        msg_print("You see nothing in that direction.");

      if (type) MSG("That's all the %s you see in that direction", type);
    }
  }
}
static void
py_look(y, x)
{
  struct caveS* c_ptr;
  struct objS* obj;
  struct monS* mon;

  if (py_affect(MA_BLIND))
    msg_print("You can't see a thing!");
  else if (countD.imagine > 0)
    msg_print("You can't believe what you are seeing! It's like a dream!");
  else {
    c_ptr = &caveD[y][x];
    mon = &entity_monD[c_ptr->midx];

    if (mon->mlit) {
      mon_desc(c_ptr->midx);
      // hack: mon death_descD pronoun is a/an
      death_descD[0] |= 0x20;
      MSG("You see %s%s.", death_descD, mon->msleep ? " (asleep)" : "");
    } else if (c_ptr->oidx && (CF_VIZ & c_ptr->cflag)) {
      obj = &entity_objD[c_ptr->oidx];
      if (obj->tval != TV_INVIS_TRAP) {
        obj_desc(obj, obj->number);
        MSG("You see %s.", descD);
      }
    }
  }
}
static int
tunnel_tool(y, x, iidx)
{
  struct caveS* c_ptr;
  struct objS* obj;
  int tabil, wall_chance, wall_min, turn_count;
  int wtohit;

  c_ptr = &caveD[y][x];
  obj = obj_get(invenD[iidx]);

  turn_count = 0;

  if (entity_objD[c_ptr->oidx].tval == TV_CLOSED_DOOR) {
    msg_print("You can't tunnel through a door!");
  } else if (c_ptr->midx) {
    msg_print("Something is in your way!");
    // Prevent the player from abusing digging for invis detection
    turn_count = 1;
  } else if (c_ptr->fval < MIN_CLOSED_SPACE) {
    msg_print("Tunnel through what?  Empty air?!?");
  } else if (c_ptr->fval == BOUNDARY_WALL) {
    msg_print("You cannot tunnel into permanent rock.");
  } else if (!obj->id) {
    msg_print("You dig with your hands, making no progress.");
  } else {
    obj_desc(obj, 1);
    wtohit = tohit_by_weight(obj->weight);
    countD.paralysis = 1;

    if (wtohit) {
      MSG("You have trouble digging with %s, it is very heavy.", descD);
    } else {
      MSG("You begin tunneling with %s.", descD);
    }

    /* If this weapon is too heavy for the player to wield properly, then
       also make it harder to dig with it. tabil may be negative.  */
    tabil = obj_tabil(obj, TRUE) + wtohit;

    wall_chance = 0;
    switch (c_ptr->fval) {
      case QUARTZ_WALL:
        wall_min = 80;
        wall_chance = 400;
        msg_print("You tunnel into the quartz vein.");
        break;
      case MAGMA_WALL:
        wall_min = 10;
        wall_chance = 600;
        msg_print("You tunnel into the magma intrusion.");
        break;
      case GRANITE_WALL:
        wall_min = 10;
        wall_chance = 1200;
        msg_print("You tunnel into the granite wall.");
        break;
      default:
        break;
    }

    if (wall_chance) {
      do {
        turn_count += 1;
        if (tabil > randint(wall_chance) + wall_min) {
          twall(y, x);
          msg_print("You have finished the tunnel.");
          break;
        }
      } while (turn_count < MAX_TUNNEL_TURN);
    } else if (entity_objD[c_ptr->oidx].tval == TV_SECRET_DOOR) {
      msg_print("You tunnel into the granite wall.");
      do {
        turn_count += 1;
        py_search(uD.y, uD.x);
        if (entity_objD[c_ptr->oidx].tval == TV_CLOSED_DOOR) break;
      } while (turn_count < MAX_TUNNEL_TURN);
    } else if (entity_objD[c_ptr->oidx].tval == TV_RUBBLE) {
      msg_print("You dig in the rubble.");

      do {
        turn_count += 1;
        if (tabil > randint(180)) {
          c_ptr->fval = FLOOR_CORR;
          delete_object(y, x);
          if (randint(10) == 1) {
            place_object(y, x, FALSE);
            if (CF_LIT & c_ptr->cflag) {
              msg_print("You have found something!");
            }
          } else {
            msg_print("You have removed the rubble.");
          }
          break;
        }
      } while (turn_count < MAX_TUNNEL_TURN);
    }
  }

  if (turn_count) {
    turn_flag = TRUE;
    // TBD: unique counter for mining?
    countD.paralysis += turn_count;
  }

  return turn_count;
}
static int
tunnel(y, x)
{
  int max_tabil;
  struct objS* i_ptr;
  int iidx;

  iidx = INVEN_WIELD;
  i_ptr = obj_get(invenD[iidx]);
  max_tabil = obj_tabil(i_ptr, FALSE);

  if ((i_ptr->flags & TR_CURSED) == 0) {
    for (int it = 0; it < MAX_INVEN; ++it) {
      i_ptr = obj_get(invenD[it]);
      if (i_ptr->tval == TV_DIGGING) {
        int tabil = obj_tabil(i_ptr, FALSE);
        if (tabil > max_tabil) {
          max_tabil = tabil;
          iidx = it;
        }
      }
    }
  }

  return tunnel_tool(y, x, iidx);
}
void
py_tunnel(iidx)
{
  int dir;
  int y, x;

  if (countD.confusion) {
    msg_print("You are too confused for digging.");
  } else if (get_dir("Which direction will you dig?", &dir)) {
    y = uD.y;
    x = uD.x;
    mmove(dir, &y, &x);

    tunnel_tool(y, x, iidx);
  }
}
static void make_move(midx, mm) int* mm;
{
  int fy, fx, newy, newx, do_turn, do_move;
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;
  struct objS* obj;

  do_turn = FALSE;
  do_move = FALSE;
  m_ptr = &entity_monD[midx];
  cr_ptr = &creatureD[m_ptr->cidx];
  fy = m_ptr->fy;
  fx = m_ptr->fx;
  c_ptr = &caveD[fy][fx];
  if ((cr_ptr->cmove & CM_PHASE) == 0 && c_ptr->fval >= MIN_WALL) {
    if (mon_take_hit(midx, damroll(8, 8))) {
      msg_print("You hear a scream muffled by rock!");
      py_experience();
    } else {
      msg_print("A creature digs itself out from the rock!");
      twall(fy, fx);
    }
    return;
  }

  for (int i = 0; i < 5; ++i) {
    newy = fy;
    newx = fx;
    mmove(mm[i], &newy, &newx);
    c_ptr = &caveD[newy][newx];
    obj = &entity_objD[c_ptr->oidx];

    if (c_ptr->fval == BOUNDARY_WALL)
      continue;
    else if (cr_ptr->cmove & CM_PHASE)
      do_move = TRUE;
    else if (c_ptr->fval == FLOOR_OBST) {
      if (obj->tval == TV_CLOSED_DOOR || obj->tval == TV_SECRET_DOOR) {
        do_turn = TRUE;
        do_move = FALSE;
        if (cr_ptr->cmove & CM_OPEN_DOOR && obj->p1 == 0) {
          obj->tval = TV_OPEN_DOOR;
          obj->tchar = '\'';
          if (c_ptr->cflag & CF_LIT) msg_print("A door creaks open.");
        } else if (cr_ptr->cmove & CM_OPEN_DOOR && obj->p1 > 0) {
          if (randint((m_ptr->hp + 1) * (50 + obj->p1)) <
              40 * (m_ptr->hp - 10 - obj->p1)) {
            msg_print("You hear the click of a lock being opened.");
            obj->p1 = 0;
          }
        } else {
          int k = ABS(obj->p1);
          if (randint((m_ptr->hp + 1) * (80 + k)) < 40 * (m_ptr->hp - 20 - k)) {
            obj->tval = TV_OPEN_DOOR;
            obj->tchar = '\'';
            // 50% chance to break the door
            obj->p1 = 1 - randint(2);
            msg_print("You hear a door burst open!");
            do_move = TRUE;
          }
        }
        if (obj->tval == TV_OPEN_DOOR) c_ptr->fval = FLOOR_CORR;
      } else {
        // permit attack-only against a player
        do_move = (newy == uD.y && newx == uD.x);
      }
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
        /* Eat it or wait */
        if ((cr_ptr->cmove & CM_EATS_OTHER) &&
            creatureD[c_ptr->midx].mexp >= cr_ptr->mexp) {
          mon_unuse(&entity_monD[c_ptr->midx]);
          c_ptr->midx = 0;
        } else
          do_move = FALSE;
      }
    }
    /* Creature has been allowed move.   */
    if (do_move) {
      if (cr_ptr->cmove & CM_PICKS_UP && obj_mon_pickup(obj)) {
        if (los(uD.y, uD.x, newy, newx)) {
          mon_desc(midx);
          MSG("%s picks up an object.", descD);
        }
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
mon_breath_dam(midx, fy, fx, breath, dam_hp)
{
  int i, j, y, x;
  int reduce, dam, harm_type;
  uint32_t cdis, weapon_type;
  int (*destroy)();
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;

  y = uD.y;
  x = uD.x;
  cdis = distance(y, x, fy, fx);
  reduce = 0;
  while (cdis) reduce = bit_pos(&cdis);
  reduce += 1;
  if (HACK) MSG("[%d/%d@%d", dam_hp + 1, reduce, distance(y, x, fy, fx));
  /* at least one damage, prevents randint(0) with poison_gas() */
  dam_hp = dam_hp / reduce + 1;
  get_flags(breath, &weapon_type, &harm_type, &destroy);
  for (i = y - 2; i <= y + 2; i++)
    for (j = x - 2; j <= x + 2; j++)
      if (in_bounds(i, j) && distance(y, x, i, j) <= 2 && los(y, x, i, j)) {
        c_ptr = &caveD[i][j];
        if ((c_ptr->oidx != 0) && (*destroy)(&entity_objD[c_ptr->oidx])) {
          if (c_ptr->fval == FLOOR_OBST) c_ptr->fval = FLOOR_CORR;
          delete_object(i, j);
        }
        if (c_ptr->fval <= MAX_OPEN_SPACE) {
          if (c_ptr->midx != midx) {
            m_ptr = &entity_monD[c_ptr->midx];
            cr_ptr = &creatureD[m_ptr->cidx];
            dam = dam_hp;
            if (harm_type & cr_ptr->cdefense)
              dam = dam * 2;
            else if (weapon_type & cr_ptr->spells)
              dam = dam / 4;
            cdis = distance(i, j, y, x);
            dam = dam / (cdis + 1);

            /* can not call mon_take_hit here, since player does not
               get experience for kill */
            m_ptr->hp = m_ptr->hp - dam;
            m_ptr->msleep = 0;
            if (m_ptr->hp < 0) {
              mon_death(m_ptr->fy, m_ptr->fx, cr_ptr->cmove);
              mon_unuse(m_ptr);
              c_ptr->midx = 0;
            }
          }
        }
      }

  /* let's do at least one point of damage to the player */
  switch (breath) {
    case GF_LIGHTNING:
      dam = light_dam(dam_hp);
      break;
    case GF_POISON_GAS:
      dam = poison_gas(dam_hp);
      break;
    case GF_ACID:
      dam = acid_dam(dam_hp, TRUE);
      break;
    case GF_FROST:
      dam = frost_dam(dam_hp);
      break;
    case GF_FIRE:
      dam = fire_dam(dam_hp);
      break;
  }
  if (HACK) {
    MSG("-%d]", dam);
  } else {
    MSG("[-%d hp]", dam);
  }
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
DATA char* mon_spell_nameD[] = {
    "phase door",     "teleport",       "summon",        "wounds",
    "serious wounds", "paralysis",      "blindness",     "confusion",
    "fear",           "summon monster", "summon undead", "slow",
};
static int
mon_try_spell(midx, cdis)
{
  uint32_t i, maxlev;
  int k, chance, thrown_spell, spell_index;
  int spell_choice[32];
  int took_turn;
  struct monS* mon;
  struct creatureS* cr_ptr;

  mon = &entity_monD[midx];
  cr_ptr = &creatureD[mon->cidx];
  maxlev = (cr_ptr->level >= MAX_MON_LEVEL);

  chance = cr_ptr->spells & CS_FREQ;

  /* confused monsters don't cast; including turn undead */
  if (mon->mconfused) took_turn = FALSE;
  /* 1 in x chance of casting spell  	   */
  else if (randint(chance) != 1)
    took_turn = FALSE;
  /* Must be within certain range  	   */
  else if (cdis > MAX_SIGHT)
    took_turn = FALSE;
  // Must have unobstructed Line-Of-Sight
  else if (!los(uD.y, uD.x, mon->fy, mon->fx))
    took_turn = FALSE;
  else /* Creature is going to cast a spell   */
  {
    /* Extract all possible spells into spell_choice */
    i = (cr_ptr->spells & ~CS_FREQ);
    if (mon->msilenced) i &= ~CS_SPELLS;
    k = 0;
    while (i != 0) {
      spell_choice[k] = bit_pos(&i);
      k++;
    }

    if (k == 0) {
      took_turn = FALSE;
    } else {
      took_turn = TRUE;

      thrown_spell = spell_choice[randint(k) - 1];
      spell_index = thrown_spell - 4;
      ++thrown_spell;

      mon_desc(midx);
      if (spell_index < AL(mon_spell_nameD)) {
        MSG("%s casts a spell of %s.", descD, mon_spell_nameD[spell_index]);
      }

      switch (thrown_spell) {
        case 5: /*Teleport Short*/
          teleport_away(midx, 5);
          break;
        case 6: /*Teleport Long */
          teleport_away(midx, MAX_SIGHT);
          break;
        case 7: /*Teleport To (aka. Summon)  */
          teleport_to(mon->fy, mon->fx);
          break;
        case 8: /*Light Wound   */
          if (player_saves())
            msg_print("You resist!");
          else
            py_take_hit(damroll(3, 8));
          break;
        case 9: /*Serious Wound */
          if (player_saves())
            msg_print("You resist!");
          else
            py_take_hit(damroll(8, 8));
          break;
        case 10: /*Hold Person    */
          if (py_tr(TR_FREE_ACT))
            msg_print("You resist!");
          else if (player_saves())
            msg_print("You resist the effects of the spell.");
          else if (countD.paralysis > 0)
            countD.paralysis += 2;
          else
            countD.paralysis = randint(5) + 4;
          break;
        case 11: /*Cause Blindness*/
          if (player_saves())
            msg_print("You resist!");
          else if (maD[MA_BLIND])
            ma_combat(MA_BLIND, 6);
          else {
            ma_combat(MA_BLIND, 12 + randint(3));
          }
          break;
        case 12: /*Cause Confuse */
          if (player_saves())
            msg_print("You resist!");
          else if (countD.confusion)
            countD.confusion += 2;
          else
            countD.confusion = randint(5) + 3;
          break;
        case 13: /*Cause Fear    */
          if (player_saves())
            msg_print("You resist!");
          else if (maD[MA_FEAR])
            ma_combat(MA_FEAR, 2);
          else
            ma_combat(MA_FEAR, randint(5) + 3);
          break;
        case 14: /*Summon Monster*/
        {
          int midx = summon_monster(uD.y, uD.x);
          update_mon(midx);
        } break;
        case 15: /*Summon Undead*/
        {
          int midx = summon_undead(uD.y, uD.x);
          update_mon(midx);
        } break;
        case 16: /*Slow Person   */
          if (py_tr(TR_FREE_ACT))
            msg_print("You are unaffected.");
          else if (player_saves())
            msg_print("You resist!");
          else if (py_affect(MA_SLOW))
            ma_combat(MA_SLOW, 2);
          else
            ma_combat(MA_SLOW, randint(5) + 3);
          break;
        case 17: /*Drain Mana   */
          if (uD.cmana > 0) {
            MSG("%s draws psychic energy from you!", descD);
            if (mon->mlit) {
              MSG("%s appears healthier.", descD);
            }
            int r1 = (randint(cr_ptr->level) >> 1) + 1;
            if (r1 > uD.cmana) {
              r1 = uD.cmana;
              uD.cmana = 0;
              uD.cmana_frac = 0;
            } else
              uD.cmana -= r1;
            mon->hp += 6 * (r1);
          }
          break;
        case 20: /*Breath Light */
          if (HACK) MSG("[%d]", mon->hp);
          MSG("%s breathes lightning.", descD);
          mon_breath_dam(midx, mon->fy, mon->fx, GF_LIGHTNING,
                         (mon->hp >> (1 + maxlev)));
          break;
        case 21: /*Breath Gas   */
          if (HACK) MSG("[%d]", mon->hp);
          MSG("%s breathes gas.", descD);
          mon_breath_dam(midx, mon->fy, mon->fx, GF_POISON_GAS,
                         (mon->hp >> (1 + maxlev)));
          break;
        case 22: /*Breath Acid   */
          if (HACK) MSG("[%d]", mon->hp);
          MSG("%s breathes acid.", descD);
          mon_breath_dam(midx, mon->fy, mon->fx, GF_ACID,
                         (mon->hp >> (1 + maxlev)));
          break;
        case 23: /*Breath Frost */
          if (HACK) MSG("[%d]", mon->hp);
          MSG("%s breathes frost.", descD);
          mon_breath_dam(midx, mon->fy, mon->fx, GF_FROST,
                         (mon->hp >> (1 + maxlev)));
          break;
        case 24: /*Breath Fire   */
          if (HACK) MSG("[%d]", mon->hp);
          MSG("%s breathes fire.", descD);
          mon_breath_dam(midx, mon->fy, mon->fx, GF_FIRE,
                         (mon->hp >> (1 + maxlev)));
          break;
        default:
          MSG("%s cast unknown spell.", descD);
      }
    }
  }
  return took_turn;
}
// Returns true if make_move() is attempted
static int
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
  if (cr_ptr->spells & CS_FREQ)
    took_turn = mon_try_spell(midx, cdis) || (cr_ptr->cmove & CM_ONLY_MAGIC);

  if (!took_turn) {
    if (m_ptr->mconfused) {
      m_ptr->mconfused -= 1;
      if ((cr_ptr->cmove & CM_ATTACK_ONLY)) {
        /* disable confused + attack_only */
        cdis = 99;
      } else {
        random = TRUE;
        // Undead are confused by turn undead and should flee below
        flee = (cr_ptr->cdefense & CD_UNDEAD);
      }
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
      return 1;
    }
  }
  return 0;
}
static int
movement_rate(speed)
{
  if (speed <= 0) {
    return ((turnD % (2 - speed)) == 0);
  }

  return speed;
}
int
creatures()
{
  int move_count, y, x, cdis, seen_act, seen_lit;
  uint8_t waslit[AL(entity_monD)];
  int adj_speed;

  adj_speed = py_speed() + pack_heavy;
  y = uD.y;
  x = uD.x;
  seen_act = 0;
  seen_lit = 0;
  for (int it = 0; it < AL(entity_monD); ++it) {
    struct monS* mon = &entity_monD[it];
    waslit[it] = mon->mlit;
    if (mon->id) update_mon(it);
  }

  FOR_EACH(mon, {
    struct creatureS* cr_ptr = &creatureD[mon->cidx];
    move_count = movement_rate(mon->mspeed + adj_speed);
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
        if (mon->msleep == 0) {
          if (mon->mstunned == 0) {
            if (mon_move(it_index, cdis)) seen_act += mon->mlit;
          }
        }
      }
    }
    if (mon->mlit && !waslit[it_index]) seen_lit += 1;
  });

  find_threat = (seen_act != 0);
  if (seen_act) countD.rest = 0;
  if (seen_lit) find_flag = FALSE;

  return seen_act;
}
BOOL
py_teleport_near(y, x, uy, ux)
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
static void hit_trap(y, x, uy, ux) int *uy, *ux;
{
  int num, dam;
  struct caveS* c_ptr;
  struct objS* obj;

  c_ptr = &caveD[y][x];
  obj = &entity_objD[c_ptr->oidx];

  obj->tval = TV_VIS_TRAP;
  obj->tchar = '^';
  obj->idflag |= ID_REVEAL;
  c_ptr->cflag |= CF_FIELDMARK;
  find_flag = FALSE;

  dam = obj->damage[1] ? pdamroll(obj->damage) : 0;

  obj_desc(obj, 1);
  strcpy(death_descD, descD);
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
      /* Reveal an open pit */
      obj->tidx = OBJ_TRAP_BEGIN + 0;
      obj->subval = 1;
      break;
    case 4: /* Trap door*/
      msg_print("A trap door opens.");
      if (py_tr(TR_FFALL)) {
        msg_print("You fall slowly, catching the ledge.");
      } else {
        uD.new_level_flag = NL_TRAP;
        if (py_take_hit(dam)) {
          msg_print("You fall to your death.");
        } else {
          dun_level++;
          /* Force the messages to display before starting to generate the
             next level.  */
          msg_pause();
        }
      }
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
      py_take_hit(dam);
      delete_object(y, x);
      place_rubble(y, x);
      msg_print("You are hit by falling rock.");
      break;
    case 10: /* Corrode gas*/
      /* Makes more sense to print the message first, then damage an
         object.  */
      msg_print("A strange red gas surrounds you.");
      corrode_gas(TRUE);
      break;
    case 11: /* Summon mon*/
      msg_print("A strange rune on the floor glows and fades.");
      msg_pause();
      delete_object(y, x);
      num = 2 + randint(3);
      for (int it = 0; it < num; it++) {
        summon_monster(y, x);
      }
      break;
    case 12: /* Fire trap*/
      msg_print("You are enveloped in flame!");
      fire_dam(dam);
      break;
    case 13: /* Acid trap*/
      msg_print("You are splashed with acid!");
      acid_dam(dam, TRUE);
      break;
    case 14: /* Poison gas*/
      msg_print("A pungent green gas surrounds you!");
      poison_gas(dam);
      break;
    case 15: /* Blind Gas */
      msg_print("A black gas surrounds you!");
      ma_duration(MA_BLIND, randint(50) + 50);
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
          ma_duration(MA_SLOW, randint(20) + 10);
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
  struct treasureS* tr_ptr;

  tr_ptr = &treasureD[obj->tidx];
  value = obj->cost;
  if (oset_rare(obj)) {
    if (may_equip(obj->tval) == INVEN_WIELD) {
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
  } else if ((obj->tval == TV_STAFF) ||
             (obj->tval == TV_WAND)) { /* Wands and staffs*/
    value = (obj->cost + (obj->cost / 32) * obj->p1) / 2;
  }
  /* picks and shovels */
  else if (obj->tval == TV_DIGGING) {
    if (obj->p1 < 0)
      value = 0;
    else {
      /* some digging tools start with non-zero p1 values, so only
         multiply the plusses by 100, make sure result is positive */
      value = obj->cost + (obj->p1 - tr_ptr->p1) * 100;
      if (value < 0) value = 0;
    }
  } else if (obj->tval == TV_LAUNCHER) {
    if (obj->tohit < 0)
      value = 0;
    else if (obj->todam < 0)
      value = 0;
    else
      value = obj->cost + (obj->tohit + obj->todam) * 100;
  }
  /* multiply value by number of items if it is a batch stack item */
  if (obj->subval & STACK_PROJECTILE) value = value * obj->number;
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
    case TV_SPIKE:
    case TV_DIGGING:
    case TV_CLOAK:
    case TV_FOOD:
    case TV_FLASK:
    case TV_LIGHT:
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
    case TV_LAUNCHER:
    case TV_PROJECTILE:
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
  int sidx, count, cost;

  obj = obj_get(invenD[iidx]);
  tr_ptr = &treasureD[obj->tidx];
  sidx = obj_store_index(obj);
  if (sidx >= 0) {
    cost = store_value(sidx, obj_value(obj), -1);
    count = (cost == 0 || STACK_PROJECTILE & obj->subval) ? obj->number : 1;
    tr_make_known(tr_ptr);
    obj->idflag = ID_REVEAL;
    obj_desc(obj, count);
    inven_destroy_num(iidx, count);
    if (cost == 0) {
      MSG("You donate %s.", descD);
    } else {
      uD.gold += cost;
      MSG("You sold %s for %d gold.", descD, cost);
    }
    msg_pause();
  }
}
enum { COST_DETAIL = 8 };
static void
pawn_display()
{
  USE(overlay_width);
  int line;
  int cost, sidx;
  struct objS* obj;
  int limitw = MIN(overlay_width, 80);
  int descw = 4;
  int detailw = limitw - INVEN_DETAIL - COST_DETAIL;
  int costw = limitw - COST_DETAIL;

  line = 0;
  for (int it = 0; it < INVEN_EQUIP; ++it) {
    int len = 1;
    overlayD[line][0] = ' ';

    obj = obj_get(invenD[it]);
    if (obj->id) {
      sidx = obj_store_index(obj);
      if (sidx >= 0) {
        len = limitw;
        cost = store_value(sidx, obj_value(obj), -1);
        obj_desc(obj, obj->number);
        obj_detail(obj);

        overlayD[line][0] = '(';
        overlayD[line][1] = 'a' + it;
        overlayD[line][2] = ')';
        overlayD[line][3] = ' ';
        memcpy(overlayD[line] + descw, AP(descD));

        if (detailD[1] != ' ') {
          if (descD[detailw - descw - 1] != ' ') {
            memcpy(overlayD[line] + detailw - 3, AP("..."));
          }
          memcpy(overlayD[line] + detailw, detail_nosp(), INVEN_DETAIL);
        }
        snprintf(overlayD[line] + costw, COST_DETAIL, " %6d", cost);
      }
    }

    overlay_usedD[line] = len;
    line += 1;
  }
}
static void
store_display(sidx)
{
  USE(overlay_width);
  int line, cost;
  struct objS* obj;
  int limitw = MIN(overlay_width, 80);
  int descw = 4;
  int detailw = limitw - INVEN_DETAIL - COST_DETAIL;
  int costw = limitw - COST_DETAIL;

  line = 0;
  for (int it = 0; it < AL(store_objD[0]); ++it) {
    int len = 1;
    overlayD[line][0] = ' ';

    obj = &store_objD[sidx][it];
    cost = store_value(sidx, obj_value(obj), 1);
    if (obj->tidx) {
      len = limitw;
      obj_desc(obj, obj->subval & STACK_PROJECTILE ? obj->number : 1);
      obj_detail(obj);

      overlayD[line][0] = '(';
      overlayD[line][1] = 'a' + it;
      overlayD[line][2] = ')';
      overlayD[line][3] = ' ';
      memcpy(overlayD[line] + descw, AP(descD));

      if (detailD[1] != ' ') {
        if (descD[detailw - descw - 1] != ' ') {
          memcpy(overlayD[line] + detailw - 3, AP("..."));
        }
        memcpy(overlayD[line] + detailw, detail_nosp(), INVEN_DETAIL);
      }
      snprintf(overlayD[line] + costw, COST_DETAIL, " %6d", cost);
    }

    overlay_usedD[line] = len;
    line += 1;
  }
}
static void
store_item_purchase(sidx, item)
{
  int iidx, count, cost, flag;
  struct objS* obj;
  struct treasureS* tr_ptr;

  flag = FALSE;
  obj = &store_objD[sidx][item];
  if (obj->tidx) {
    count = obj->subval & STACK_PROJECTILE ? obj->number : 1;
    cost = store_value(sidx, obj_value(obj), 1);
    if (uD.gold >= cost) {
      if ((iidx = inven_obj_mergecount(obj, count)) >= 0) {
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
      obj_desc(obj, count);
      uD.gold -= cost;
      MSG("You bought %s for %d gold (%c).", descD, cost, iidx + 'a');
      if (obj->number != count) MSG("You have %d.", obj->number);
      store_item_destroy(sidx, item, count);
    }
    msg_pause();
  }
}
static void
pawn_entrance()
{
  char c;

  overlay_submodeD = 'p';
  while (1) {
    pawn_display();
    if (!in_subcommand("What would you like to sell to Gilbrook The Thrifty?",
                       &c)) {
      break;
    }

    if (is_lower(c)) {
      uint8_t item = c - 'a';
      if (item < INVEN_EQUIP) inven_pawn(item);
    } else if (is_upper(c)) {
      uint8_t item = c - 'A';
      if (item < INVEN_EQUIP) obj_study(obj_get(invenD[item]), 1);
    } else if (c == '-') {
      inven_sort();
    }
  }
  msg_advance();
}
static void
store_entrance(sidx)
{
  char c;
  char tmp_str[80];

  snprintf(tmp_str, AL(tmp_str), "What would you like to purchase from %s?",
           ownerD[storeD[sidx]].name);
  overlay_submodeD = '0' + sidx;
  while (1) {
    store_display(sidx);
    if (!in_subcommand(tmp_str, &c)) break;

    if (is_lower(c)) {
      uint8_t item = c - 'a';
      if (item < AL(store_objD[0])) store_item_purchase(sidx, item);
    } else if (is_upper(c)) {
      uint8_t item = c - 'A';
      if (item < AL(store_objD[0])) obj_study(&store_objD[sidx][item], 1);
    } else if (c == '-') {
      store_sort(sidx);
    }
  }
  msg_advance();
}
void yx_autoinven(y_ptr, x_ptr, iidx) int *y_ptr, *x_ptr;
{
  struct objS* obj = obj_get(invenD[iidx]);
  if (obj->tval == TV_PROJECTILE) {
    py_throw(iidx);
  } else if (obj->tval == TV_FOOD) {
    inven_eat(iidx);
  } else if (obj->tval == TV_POTION1 || obj->tval == TV_POTION2) {
    inven_quaff(iidx);
  } else if (obj->tval == TV_SCROLL1 || obj->tval == TV_SCROLL2) {
    inven_read(iidx, y_ptr, x_ptr);
  } else if (obj->tval == TV_STAFF) {
    inven_try_staff(iidx, x_ptr, x_ptr);
  } else if (obj->tval == TV_WAND) {
    py_zap(iidx);
  } else if (obj->tval == TV_MAGIC_BOOK) {
    py_magic(iidx, y_ptr, x_ptr);
  } else if (obj->tval == TV_PRAYER_BOOK) {
    py_prayer(iidx, y_ptr, x_ptr);
  } else if (obj->tval == TV_FLASK) {
    inven_flask(iidx);
  } else if (obj->tval == TV_SPIKE) {
    py_spike(iidx);
  } else if (obj->tval == TV_DIGGING) {
    py_tunnel(iidx);
    iidx = -1;
  } else if (iidx < INVEN_EQUIP) {
    inven_wear(iidx);
  } else if (iidx == INVEN_WIELD || iidx == INVEN_AUX) {
    py_offhand();
  } else if (iidx >= INVEN_EQUIP) {
    if (invenD[iidx]) {
      int into = inven_slot();
      if (into >= 0) {
        equip_swap_into(iidx, into);
      }
    }
  }
}
void py_reactuate(y_ptr, x_ptr, obj_id) int *y_ptr, *x_ptr;
{
  if (obj_id) {
    for (int it = 0; it < MAX_INVEN; ++it) {
      if (invenD[it] == obj_id) {
        yx_autoinven(y_ptr, x_ptr, it);
        return;
      }
    }
  }
  msg_print("Unable to repeat command.");
}
void py_actuate(y_ptr, x_ptr) int *y_ptr, *x_ptr;
{
  int iidx;

  overlay_submodeD = 'i';
  do {
    msg_pause();
    iidx =
        inven_choice("Use which item?", overlay_submodeD == 'e' ? "/*" : "*/");

    if (iidx >= 0) {
      last_actuateD = invenD[iidx];
      last_castD = 0;
      yx_autoinven(y_ptr, x_ptr, iidx);
    }
  } while (!turn_flag && iidx >= 0);
}
static void
regenhp(percent)
{
  uint32_t new_value;
  int chp;

  new_value = uD.mhp * percent + PLAYER_REGEN_HPBASE + uD.chp_frac;
  chp = uD.chp + (new_value >> 16);

  if (chp >= uD.mhp) {
    uD.chp = uD.mhp;
    uD.chp_frac = 0;
  } else {
    uD.chp = chp;
    uD.chp_frac = (new_value & 0xFFFF);
  }
}
static void
regenmana(percent)
{
  uint32_t new_value;
  int cmana;

  new_value = uD.mmana * percent + PLAYER_REGEN_MNBASE + uD.cmana_frac;
  cmana = uD.cmana + (new_value >> 16);

  if (cmana >= uD.mmana) {
    uD.cmana = uD.mmana;
    uD.cmana_frac = 0;
  } else {
    uD.cmana = cmana;
    uD.cmana_frac = new_value & 0xFFFF;
  }
}
void
player_maint()
{
  int flag;

  inven_sort();
  flag = inven_reveal();
  if (flag)
    msg_print("Town inhabitants share knowledge of items you gathered.");

  flag = 0;
  for (int it = 0; it < MAX_A; ++it) {
    if (statD.cur_stat[it] < statD.max_stat[it]) {
      if (!flag) {
        flag = 1;
        msg_print("A wind from the Misty Mountains renews your being.");
      }
      res_stat(it);
    }
  }

  for (int sidx = 0; sidx < MAX_STORE; ++sidx) {
    for (int it = 0; it < MAX_STORE_INVEN; ++it) {
      if (store_objD[sidx][it].sn) {
        MSG("Rumor has it, a rare item being sold by (%d) %s.", sidx + 1,
            ownerD[storeD[sidx]].name);
        break;
      }
    }
  }
}
void
ma_tick(check_view)
{
  uint32_t active, delta;
  int32_t tick[AL(maD)];

  active = 0;
  for (int it = 0; it < AL(maD); ++it) {
    int val = maD[it];
    if (val > 1) {
      tick[it] = val - 1;
      active |= (1 << it);
    } else {
      tick[it] = 0;
    }
  }

  delta = uD.mflag ^ active;
  for (int it = 0; it < AL(maD); ++it) {
    if (delta & (1 << it)) {
      if (active & (1 << it)) {
        ma_bonuses(it, 1);
      } else {
        ma_bonuses(it, -1);
      }
    }
  }
  // ma_bonuses() is processed before applying count/flag changes
  memcpy(maD, tick, sizeof(maD));
  uD.mflag = active;
  // calculations are called after count/flag changes
  if (delta) {
    calc_bonuses();
    if (MA_VIEW & delta) py_check_view(check_view);
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
    msg_print("You are getting hungry.");
  } else if (tmp < PLAYER_FOOD_WEAK && uD.food >= PLAYER_FOOD_WEAK) {
    msg_print("You are getting weak from starvation.");
  } else if (tmp < 0) {
    if (turnD % 8 == 1) msg_print("You are starving.");
    strcpy(death_descD, "starvation");
    py_take_hit(-tmp / 16);
  }
  uD.food = tmp;

  if (py_tr(TR_REGEN)) regen_amount = regen_amount * 3 / 2;
  if (countD.rest != 0) regen_amount = regen_amount * 2;
  if (uD.cmana < uD.mmana) regenmana(regen_amount);

  if (countD.poison == 0) {
    regenhp(regen_amount);
  } else if (countD.poison > 0) {
    if (countD.poison == 1) {
      msg_print("You feel less ill.");
    } else {
      strcpy(death_descD, "poison");
      py_take_hit(poison_adj());
      if ((turnD % 16) == 0) {
        msg_print("You shiver from illness.");
      }
    }
    countD.poison -= 1;
  }

  if (countD.confusion > 0) {
    countD.confusion -= 1;
    if (countD.confusion == 0) msg_print("You feel less confused.");
  }

  if (countD.rest < 0) {
    countD.rest += 1;
    if (uD.cmana == uD.mmana) countD.rest = 0;
  } else if (countD.rest > 0) {
    countD.rest -= 1;
    if (uD.chp == uD.mhp && rest_affect() == 0) countD.rest = 0;
  }

  if (countD.paralysis) countD.paralysis -= 1;
  if (countD.life_prot > 0) {
    countD.life_prot -= 1;
    if (countD.life_prot == 0)
      msg_print("You no longer feel safe from life drain.");
  }

  if (countD.imagine) countD.imagine -= 1;
}
int
dir_by_confusion()
{
  struct caveS* c_ptr;
  int mm[8];
  int valid = 0;
  int ny, nx;
  int dir;

  for (int it = 1; it <= 9; ++it) {
    ny = uD.y;
    nx = uD.x;
    if (it != 5 && mmove(it, &ny, &nx)) {
      c_ptr = &caveD[ny][nx];
      if (c_ptr->midx || c_ptr->fval <= MAX_OPEN_SPACE) {
        mm[valid] = it;
        valid += 1;
      }
    }
  }

  dir = mm[randint(valid) - 1];
  msg_print("You are confused.");
  return dir;
}
void
dungeon()
{
  int c, y, x, iidx;
  int ymine, xmine;
  uint32_t dir, teleport;
  int town;
  int omit_replay;

  town = (dun_level == 0);
  uD.max_dlv = MAX(uD.max_dlv, dun_level);

  if (town)
    snprintf(dun_descD, AL(dun_descD), "%s", "town square");
  else
    snprintf(dun_descD, AL(dun_descD), "%d feet", dun_level * 50);

  switch (uD.new_level_flag) {
    default:
    case NL_DOWN_STAIR:
      msg_print("You pass through a maze of down staircases.");
      break;
    case NL_UP_STAIR:
      msg_print("You pass through a maze of up staircases.");
      break;
    case NL_RECALL:
      if (town) {
        msg_print("You feel yourself yanked upwards!");
      } else {
        msg_print("You feel yourself yanked downwards!");
      }
      break;
    case NL_TRAP:
      msg_print("You fall, landing hard on the ground!");
      break;
    case NL_MIDPOINT_LOST:
      msg_print("Game version updated, dungeon reset.");
      break;
  }
  uD.new_level_flag = 0;
  if (town) player_maint();

  teleport = FALSE;
  do {
    turn_flag = FALSE;
    inven_check_weight();
    inven_check_light();
    ymine = xmine = -1;

    do {
      omit_replay = 0;
      msg_moreD = 0;
      replay_flag = (input_record_readD < input_record_writeD);
      c = 0;
      draw();
      if (!teleport && countD.rest != 0) break;
      if (!teleport && countD.paralysis != 0) break;

      y = uD.y;
      x = uD.x;
      if (teleport) {
        if (equip_vibrate(TR_TELEPORT)) py_teleport(40, &y, &x);
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
            case TV_CHEST:
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

        // [1, 9] + (jhklnbyuJHKLNBYU)
        dir = c >= '1' ? map_roguedir(c | 0x20) - '1' : -1;
        if (dir < 9) {
          // 75% random movement
          if (countD.confusion && randint(4) > 1) {
            dir = dir_by_confusion();
          } else {
            dir += 1;
          }

          if (countD.confusion /* can't run during confusion */
              || c & 0x20) {
            // Primary movement (lowercase)
            mmove(dir, &y, &x);
          } else {
            // Secondary movement (uppercase)
            find_init(dir, &y, &x);
          }
        } else {
          switch (c) {
            case '?':
              py_help();
              break;
            case '=':
              py_menu();
              msg_advance();
              break;
            case '-':
              omit_replay = 1;
              zoom_factorD = (zoom_factorD - 1) % MAX_ZOOM;
              break;
            case '+':
              omit_replay = 1;
              zoom_factorD = (zoom_factorD + 1) % MAX_ZOOM;
              break;
            case CTRL('a'):
              py_reactuate(&y, &x, last_actuateD);
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
              py_drop();
              break;
            case 'e': {
              int count = inven_overlay(INVEN_EQUIP, MAX_INVEN);
              MSG("You wear %d %s.", count, count > 1 ? "items" : "item");
            } break;
            case 'f':
              py_bash(&y, &x);
              break;
            case 'i': {
              int count = inven_overlay(0, INVEN_EQUIP);
              MSG("You carry %d %s:", count, count > 1 ? "items" : "item");
            } break;
            case 'q':
              iidx = inven_choice("Quaff what?", "*");
              if (iidx >= 0) inven_quaff(iidx);
              break;
            case 'r':
              iidx = inven_choice("Read what?", "*");
              if (iidx >= 0) inven_read(iidx, &y, &x);
              break;
            case 'o':
              py_open();
              break;
            case 's':
              msg_print("You search the area.");
              py_search(y, x);
              break;
            case 'v':
              show_version();
              break;
            case 'w':
              iidx = inven_choice("Wear/Wield which item?", "*");
              if (iidx >= 0 && iidx < INVEN_EQUIP) inven_wear(iidx);
              break;
            case 'x':
              py_examine();
              break;
            case 'z':
              iidx = inven_choice("Aim which wand?", "*");
              if (iidx >= 0) py_zap(iidx);
              break;
            case '<':
              go_up();
              break;
            case '>':
              go_down();
              break;
            case 'A':
              // Generalized inventory interaction
              py_actuate(&y, &x);
              break;
            case 'C':
              omit_replay = 1;
              show_character(0);
              break;
            case 'D':
              py_disarm(&y, &x);
              break;
            case 'E':
              iidx = inven_choice("Eat what?", "*");
              if (iidx >= 0) inven_eat(iidx);
              break;
            case 'I':
              inven_sort();
              int count = inven_overlay(0, INVEN_EQUIP);
              MSG("You organize %d %s:", count, count > 1 ? "items" : "item");
              break;
            case 'L':
              break;
            case 'M':
              if (HACK) {
                map_area();
                for (int col = 1; col < MAX_HEIGHT; ++col) {
                  for (int row = 1; row < MAX_WIDTH; ++row) {
                    struct caveS* c_ptr = &caveD[col][row];
                    struct objS* obj = &entity_objD[c_ptr->oidx];
                    if (obj->tval == TV_INVIS_TRAP) {
                      obj->tval = TV_VIS_TRAP;
                      obj->tchar = '^';
                      c_ptr->cflag |= CF_FIELDMARK;
                    }
                  }
                }
              } else {
                omit_replay = 1;
              }

              // TBD: fix console moria
              // screen_submodeD = 0;
              // screenD[0][0] = ' ';
              // screen_usedD[0] = 1;

              if (maD[MA_BLIND] == 0) {
                minimap_enlargeD = TRUE;
                // TBD: text only for console mode?
                DRAWMSG("");
                inkey();
              }
              break;
            case 'O': {
              omit_replay = 1;
              int zoom_factor = zoom_factorD;
              int cellh = SYMMAP_HEIGHT >> zoom_factor;
              int cellw = SYMMAP_WIDTH >> zoom_factor;
              int ty = MAX(uD.y - cellh / 2, panelD.panel_row_min);
              int tx = MAX(uD.x - cellw / 2, panelD.panel_col_min);
              py_look(ylookD + ty, xlookD + tx);
            } break;
            case 'R':
              py_rest();
              break;
            case 'S':
              iidx = inven_choice("Study which item?", "*/");
              if (iidx >= 0) obj_study(obj_get(invenD[iidx]), 0);
              break;
            case 'T':
              py_takeoff();
              break;
            case 'W':
              py_where();
              break;
            case 'X':
              py_offhand();
              break;
            case 'Z':
              iidx = inven_choice("Invoke which staff?", "*");
              if (iidx >= 0) inven_try_staff(iidx, &y, &x);
              break;
            case CTRL('c'):
              if (!RELEASE) {
                memcpy(death_descD, AP(quit_stringD));
                uD.new_level_flag = NL_DEATH;
                return;  // Interrupt game
              }
              break;
            case CTRL('p'): {
              show_history();
            } break;
            case CTRL('x'):
              if (!RELEASE) {
                if (platformD.savemidpoint && platformD.savemidpoint()) {
                  memcpy(death_descD, AP(quit_stringD));
                  uD.new_level_flag = NL_DEATH;
                  return;  // Interrupt game
                }
              }
              break;
            default:
              break;
          }
          if (HACK) {
            switch (c) {
              case CTRL('a'): {
                uD.exp += 1000000;
                py_experience();
                dun_level = 0;
                uD.new_level_flag = NL_DOWN_STAIR;
                uD.gold = 10000;
                turn_flag = TRUE;
              } break;
              case CTRL('d'): {
                detect_obj(oset_obj, TRUE);
              } break;
              case CTRL('e'): {
                earthquake();
              } break;
              case CTRL('f'): {
                create_food(y, x);
              } break;
              case CTRL('h'):
                uD.chp = uD.mhp;
                msg_print("You are healed.");
                break;
              case CTRL('l'): {
                static int toggleD;
                for (int row = 0; row < MAX_HEIGHT; ++row) {
                  for (int col = 0; col < MAX_WIDTH; ++col) {
                    if (caveD[row][col].fval <= MAX_OPEN_SPACE) {
                      if (!toggleD)
                        caveD[row][col].cflag |= (CF_PERM_LIGHT | CF_ROOM);
                      else
                        caveD[row][col].cflag &= ~(CF_PERM_LIGHT | CF_ROOM);
                    }
                  }
                }
                toggleD = !toggleD;
              } break;
              case CTRL('t'):
                msg_print("teleport");
                do {
                  x = randint(MAX_WIDTH - 2);
                  y = randint(MAX_HEIGHT - 2);
                } while (caveD[y][x].fval >= MIN_CLOSED_SPACE ||
                         caveD[y][x].midx != 0);
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
              case CTRL('s'): {
                msg_print("Store maintenance.");
                store_maint();
              } break;
              case CTRL('w'): {
                msg_print("The air about you becomes charged.");
                ma_duration(MA_RECALL, 1);
              } break;
              case CTRL('z'): {
                dun_level += 1;
                uD.new_level_flag = NL_DOWN_STAIR;
                turn_flag = TRUE;
              } break;
            }
          }
        }
      }

      if (uD.y != y || uD.x != x) {
        struct caveS* c_ptr = &caveD[y][x];
        struct monS* mon = &entity_monD[c_ptr->midx];
        struct objS* obj = &entity_objD[c_ptr->oidx];

        if (find_flag && mon->mlit) {
          // Run is non-combat movement
          find_flag = FALSE;
        } else {
          // doors known to be jammed are bashed prior to movement
          if (obj->tval == TV_CLOSED_DOOR) {
            if (mon->id == 0) {
              if (obj->p1 < 0 && (obj->idflag & ID_REVEAL)) {
                bash(y, x);
              }
            }
          }

          if (mon->id) {
            py_attack(y, x);
          } else if (c_ptr->fval <= MAX_OPEN_SPACE) {
            if (obj->tval == TV_CHEST) {
              if (obj->idflag & ID_REVEAL) try_disarm_chest(y, x);
            } else if (obj->tval == TV_VIS_TRAP) {
              if (obj->idflag & ID_REVEAL) try_disarm_trap(y, x);
            }
            if (obj->tval == TV_INVIS_TRAP || obj->tval == TV_VIS_TRAP) {
              hit_trap(y, x, &y, &x);
            }

            py_light_off(uD.y, uD.x);
            py_light_on(y, x);
            turn_flag = TRUE;
            uD.y = y;
            uD.x = x;

            // Perception check on movement
            if (uD.fos <= 1 || randint(uD.fos) == 1) py_search(y, x);
            if (py_affect(MA_BLIND) == 0 && find_flag) {
              if (find_event(y, x)) find_flag = FALSE;
            }

            if (obj->tval == TV_CHEST && obj->sn != SN_EMPTY) {
              open_object(y, x);
            } else if (obj->tval == TV_STORE_DOOR) {
              store_entrance(obj->tchar - '1');
            } else if (obj->tval == TV_PAWN_DOOR) {
              pawn_entrance();
            } else if (oset_pickup(obj)) {
              py_pickup(y, x, FALSE);
            }
          } else if (obj->tval == TV_CLOSED_DOOR) {
            open_object(y, x);
          } else {
            find_flag = FALSE;
            if (py_affect(MA_BLIND) == 0) {
              if (ymine == y && xmine == x) {
                if (obj->tval == TV_GOLD || obj->tval == TV_RUBBLE)
                  tunnel(y, x);
              } else {
                if (obj->tval == TV_GOLD) {
                  obj_desc(obj, obj->number);
                  MSG("You see %s glimmering in the %s.", descD,
                      c_ptr->fval == QUARTZ_WALL ? "quartz vein"
                                                 : "magma intrusion");
                } else if (obj->tval == TV_RUBBLE) {
                  msg_print("You see rubble.");
                }
                ymine = y;
                xmine = x;
              }
            }
          }
          panel_update(&panelD, uD.y, uD.x, FALSE);
        }
      }

      if (c) {
        if (omit_replay) {
          input_record_readD = input_record_writeD =
              AS(input_actionD, input_action_usedD - 1);
        } else if (turn_flag) {
          AS(input_actionD, input_action_usedD++) = input_record_readD;
        }
      }
    } while (!turn_flag);

    ma_tick(0);  // rising
    if (!uD.new_level_flag) {
      creatures();
      teleport = (py_tr(TR_TELEPORT) && randint(100) == 1);
      if (randint(MAX_MALLOC_CHANCE) == 1) {
        if (town)
          alloc_townmon(1);
        else
          alloc_mon(1, MAX_SIGHT, FALSE);
      }
    }

    if (!town && (turnD & ~-1024) == 0) store_maint();
    ma_tick(!uD.new_level_flag);  // falling
    tick();  // uD.new_level_flag may change (player dies from poison)
    turnD += 1;
  } while (!uD.new_level_flag);
  msg_pause();
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

static int
platform_init()
{
  platformD.pregame = platform_pregame;
  platformD.postgame = platform_postgame;
  platformD.seed = platform_random;
  platformD.load = platform_load;
  platformD.save = platform_save;
  platformD.erase = platform_erase;
  platformD.readansi = platform_readansi;
  platformD.predraw = platform_predraw;
  platformD.draw = platform_draw;
  msg_widthD = overlay_widthD = 80;

  return 0;
}
int
main(int argc, char** argv)
{
  mon_level_init();
  obj_level_init();

  platform_init();
  platformD.pregame();

  setjmp(restartD);
  hard_reset();

  int ready = platformD.cache ? platformD.cache() : 0;
  if (!ready) ready = py_saveslot_select();

  if (ready) {
    // Per-Player initialization
    fixed_seed_func(obj_seed, magic_init);
    // recreate history text
    fixed_seed_func(town_seed, social_bonus);

    // Replay state reset
    if (input_resumeD > 0 && input_resumeD <= input_action_usedD) {
      replay_flag = TRUE;
      input_record_writeD = AS(input_actionD, input_resumeD - 1);
    } else {
      replay_flag = FALSE;
      input_record_writeD = 0;
    }
    input_resumeD = 0;
    input_record_readD = input_action_usedD = 0;

    // may generate messages in calc_mana()->gain_prayer()
    for (int it = 0; it < MAX_A; ++it) {
      // Perform calculations
      set_use_stat(it);
    }

    // Release objects in the cave
    FOR_EACH(obj, {
      if (obj->tval > TV_MAX_PICK_UP || obj->fx || obj->fy) {
        obj_unuse(obj);
      }
    });

    // a fresh cave!
    if (dun_level != 0) {
      cave_gen();
    } else {
      town_gen();
      store_maint();
    }

    panel_update(&panelD, uD.y, uD.x, TRUE);
    py_check_view(TRUE);
    dungeon();
    replay_flag = 0;

    if (uD.new_level_flag != NL_DEATH) {
      if (platformD.save()) {
        longjmp(restartD, 1);
      } else {
        strcpy(death_descD, "Device I/O Error");
      }
    }
  } else {
    strcpy(death_descD, "Saveslot Error");
  }

  if (memcmp(death_descD, AP(quit_stringD)) != 0) {
    inven_reveal();
    py_death();
  }

  platformD.postgame();
  return 0;
}
