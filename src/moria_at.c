// Rufe.org LLC 2022-2025: GPLv3 License
enum { HACK = 0 };
enum { RESEED = 0 };

#include "src/game.c"

#include "src/mod/savechar.c"

#include "platform/platform.c"

enum { TEST_CREATURE = 0 };
enum { TEST_REPLAY = 0 };
// #include "src/mod/replay.c"
enum { TEST_CAVEGEN = 0 };
// #include "src/mod/cavegen.c"
enum { TEST_CHECKLEN = 0 };
// #include "src/mod/checklen.c"

enum { PRIVACY = !KEYBOARD };
enum { MOBILE = !KEYBOARD };

#ifndef RELEASE
#include "dev.h"
#endif

// Replay System
DATA jmp_buf restartD;

// Platform input without recording
STATIC int
read_input()
{
  char c;
  do {
    c = platformD.input();
  } while (c == 0);
  if (c != CTRL('d')) viz_hookD = 0;
  return c;
}
STATIC int
replay_playback()
{
  return replay_flag == REPLAY_PLAYBACK;
}
STATIC int
replay_recording()
{
  return replay_flag == REPLAY_RECORD;
}
STATIC int
replay_resumed()
{
  return (replay_flag == REPLAY_PLAYBACK &&
          replayD->input_record_readD >= replayD->input_record_writeD);
}
// replay playback or input recording
STATIC char
game_input()
{
  USE(replay);
  char c;
  if (replay_playback() &&
      replay->input_record_readD < replay->input_record_writeD) {
    c = AS(replay->input_recordD, replay->input_record_readD++);
  } else {
    c = read_input();

    // Ignore redraw (system generated input)
    if (c != CTRL('d') && replay_recording()) {
      if (replay->input_record_writeD < AL(replay->input_recordD)) {
        AS(replay->input_recordD, replay->input_record_writeD++) = c;
        replay->input_record_readD = replay->input_record_writeD;
      }
    }
  }

  return c;
}
STATIC void
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
STATIC int
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
STATIC int
py_affect(maid)
{
  return (uD.mflag & (1 << maid)) != 0;
}
// Match ALL trflag
STATIC int
py_tr(trflag)
{
  return (cbD.tflag & trflag) == trflag;
}
STATIC int
uvow(idx)
{
  return (1 << idx) & uD.vow_flag;
}
STATIC int
py_speed()
{
  return (py_affect(MA_SLOW) + py_tr(TR_SLOWNESS)) -
         (py_affect(MA_FAST) + py_tr(TR_SPEED));
}
STATIC int
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
STATIC int
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
STATIC int
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
STATIC struct spellS*
uspelltable()
{
  int clidx = uD.clidx;
  if (clidx) {
    return spellD[clidx - 1];
  }
  return 0;
}
STATIC void
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
  active_affectD[idx++] = py_tr(TR_SEE_INVIS);
  active_affectD[idx++] = (countD.paralysis != 0);
  active_affectD[idx++] = (countD.poison != 0);

  // Gain spells, imagine, ...
  active_affectD[idx++] = (spcount && spell_orderD[spcount - 1] == 0);
  active_affectD[idx++] = (countD.imagine != 0);
  active_affectD[idx++] = (find_threatD != 0);
}
STATIC int
msg_advance()
{
  int log_used = AS(msglen_cqD, msg_writeD);
  int advance = (log_used != 0);
  USE(msg_write);
  msg_write += advance;
  AS(msglen_cqD, msg_write) = 0;
  msg_writeD = msg_write;
  return advance;
}
// Sorting commands modify GAME simulation
// Passing a turn is a work-around to ensure input chain is preserved
// This turn cost can be merged with useful actions (purchase/sell/actuate)
STATIC void
replay_hack()
{
  // TBD: save replay_action():
  // main loop should check input_action_usedD before changing input_records
  turn_flag = 1;
}
// Safeguard the reuse of simulation input
STATIC uint64_t
replay_hash()
{
  uint64_t hv = DJB2;
  hv = djb2(hv, AB(versionD));
  hv = djb2(hv, BP(dun_level));
  hv = djb2(hv, BP(rnd_seed));
  hv = djb2(hv, BP(uD.clidx));
  return hv;
}
STATIC void
replay_start()
{
  if (replayD) {
    showx(replayD->input_rhashD);
    uint64_t rhash = replay_hash();
    showx(rhash);
    if (replayD->input_rhashD != rhash) replayD->input_action_usedD = 0;
    if (replayD->input_rhashD != rhash) replayD->input_mutationD = 0;
    replayD->input_rhashD = rhash;

    if (replayD->input_action_usedD > 0) {
      replay_flag = REPLAY_PLAYBACK;
      replayD->input_record_writeD =
          AS(replayD->input_actionD, replayD->input_action_usedD - 1);
    } else {
      replay_flag = REPLAY_RECORD;
      replayD->input_record_writeD = 0;
    }
    replayD->input_record_readD = 0;
    replayD->input_action_usedD = 0;
  }
}
STATIC void
replay_end()
{
  if (replay_flag) {
    replay_flag = REPLAY_DEAD;
    replayD->input_record_readD = replayD->input_record_writeD;
  }
}
STATIC int
in_bounds(row, col)
{
  uint32_t urow = row - 1;
  uint32_t ucol = col - 1;

  return urow < (MAX_HEIGHT - 2) && ucol < (MAX_WIDTH - 2);
}
STATIC int
viz_magick()
{
  int rmin = panelD.panel_row_min;
  int rmax = panelD.panel_row_max;
  int cmin = panelD.panel_col_min;
  int cmax = panelD.panel_col_max;
  int magick_dist = magick_distD;
  MUSE(magick_loc, x);
  MUSE(magick_loc, y);
  USE(magick_hitu);

  if (x >= cmin && x < cmax && y >= rmin && y < rmax) {
    int ox = x - cmin;
    int oy = y - rmin;

    for (int i = MAX(0, oy - magick_dist);
         i <= MIN(SYMMAP_HEIGHT - 1, oy + magick_dist); ++i) {
      for (int j = MAX(0, ox - magick_dist);
           j <= MIN(SYMMAP_WIDTH - 1, ox + magick_dist); ++j) {
        int ay = rmin + i;
        int ax = cmin + j;
        if (in_bounds(ay, ax) && distance(y, x, ay, ax) <= magick_dist &&
            los(y, x, ay, ax)) {
          if (caveD[ay][ax].fval <= MAX_FLOOR) {
            if (magick_hitu || ay != uD.y || ax != uD.x) {
              vizD[i][j].vflag = VF_MAGICK;
            }
          }
        }
      }
    }
  }

  return 0;
}
// wait 0: don't wait
// wait -1: any key (REPLAY includes key)
// wait non-zero: pedantic mode wait for exact input (REPLAY excludes input)
STATIC int
draw(wait)
{
  int flush_draw = !replay_playback();
  char c = 0;

  // Advance gamelog
  if (wait >= 0 && msg_advance()) msg_turnD = turnD;

  if (replay_playback() && wait < 0) {
    c = game_input();
  }

  if (flush_draw) {
    vital_update();
    affect_update();

    platformD.predraw();

    if (viz_hookD) viz_hookD();
  }

  while (flush_draw) {
    platformD.draw();

    if (wait > 0) {
      do {
        c = read_input();
        // INTERRUPT
        if (c == CTRL('c')) break;
        // REFRESH
        if (c == CTRL('d')) break;
      } while (c != wait);
    }
    if (wait < 0) c = game_input();
    flush_draw = (c == CTRL('d'));
  }

  // Overlay information is reset
  AC(screen_usedD);
  AC(overlay_usedD);
  ylookD = xlookD = -1;
  blipD = 0;

  memset(AS(msg_cqD, msg_writeD), WHITESPACE, STRLEN_MSG + 1);
  AS(msglen_cqD, msg_writeD) = 0;

  return c;
}

STATIC void
msg_pause()
{
  int log_used;

  log_used = AS(msglen_cqD, msg_writeD);
  if (log_used) {
    // non-replay mode waits for user to acknowledge buffer text -more-
    msg_moreD = 1;
    draw(' ');
    msg_moreD = 0;
  }
}

// Use before CLOBBER_MSG macro
// Adds right-justified text to the title text
STATIC void msg_hint(hint, hintlen) char* hint;
{
  char* msg_end = AS(msg_cqD, msg_writeD) + overlay_widthD;
  memcpy(msg_end - hintlen - 1, hint, hintlen);
}
STATIC void msg_write(msg, msglen, msg_width) char* msg;
{
  int log_used = AS(msglen_cqD, msg_writeD);
  if (log_used + msglen >= msg_width) {
    msg_pause();
    log_used = 0;
  }

  char* log = AS(msg_cqD, msg_writeD);
  int msg_used = snprintf(log + log_used, msg_width - log_used, "%s ", msg);

  if (msg_used > 0)
    AS(msglen_cqD, msg_writeD) = MIN(log_used + msg_used, msg_width);

  if (countD.rest) countD.rest = 0;
  if (find_flagD) find_flagD = 0;
}
STATIC void msg_game(msg, msglen, msg_width) char* msg;
{
  msglen = CLAMP(msglen, 0, msg_width);
  msg[msglen] = 0;

  msg_write(msg, msglen, msg_width);
}
STATIC int
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
  return CLOBBER_MSG("Message History (%d)", count);
}
STATIC int
in_subcommand(prompt, command)
char* prompt;
char* command;
{
  char c = CLOBBER_MSG("%s", prompt ? prompt : "");
  *command = c;
  return is_ctrl(c) ? 0 : 1;
}
STATIC char
map_roguedir(comval)
{
  if (comval > ' ') comval |= 0x20;
  return dir_key(comval);
}
STATIC int
get_dir(prompt, dir)
char* prompt;
int* dir;
{
  char c;
  if (!prompt) prompt = "Which direction?";
  // ugh loop
  do {
    c = CLOBBER_MSG("%s", prompt);
    if (c == 'a') break;

    int tmp = map_roguedir(c);
    if (tmp > 0 && tmp != 5) {
      *dir = tmp;
      return 1;
    }
  } while (!is_ctrl(c));

  return 0;
}
// Undefined if called on a pointer to zero
STATIC int
bit_pos(test)
uint32_t* test;
{
  int i = __builtin_ctz(*test);
  *test ^= (1 << i);

  return i;
}
STATIC void
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
STATIC void
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
STATIC int
cave_floor_near(y, x)
{
  for (int row = y - 1; row <= y + 1; ++row) {
    for (int col = x - 1; col <= x + 1; ++col) {
      if (caveD[row][col].fval <= MAX_FLOOR) return TRUE;
    }
  }
  return FALSE;
}
STATIC void
place_boundary()
{
  for (int row = 0; row < MAX_HEIGHT; ++row) caveD[row][0].fval = BOUNDARY_WALL;
  for (int row = 0; row < MAX_HEIGHT; ++row)
    caveD[row][MAX_WIDTH - 1].fval = BOUNDARY_WALL;

  for (int col = 0; col < MAX_WIDTH; ++col) caveD[0][col].fval = BOUNDARY_WALL;
  for (int col = 0; col < MAX_WIDTH; ++col)
    caveD[MAX_HEIGHT - 1][col].fval = BOUNDARY_WALL;
}

STATIC int
rnd()
{
  int low, high, test;

  high = rnd_seed / RNG_Q;
  low = rnd_seed % RNG_Q;
  test = RNG_A * low - RNG_R * high;
  if (test > 0)
    rnd_seed = test;
  else
    rnd_seed = test + RNG_M - (test == 0);
  return rnd_seed;
}
STATIC int
randint(maxval)
{
  return ((rnd() % maxval) + 1);
}
STATIC int
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
STATIC void
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
STATIC int
fixed_seed_func(seed, func)
fn func;
{
  uint32_t keep_seed;

  keep_seed = rnd_seed;
  rnd_seed = seed;
  int ret = func();
  rnd_seed = keep_seed;
  return ret;
}
STATIC int
damroll(num, sides)
{
  int i, sum = 0;

  for (i = 0; i < num; i++) sum += randint(sides);
  return (sum);
}
STATIC int
pdamroll(array)
uint8_t* array;
{
  return damroll(array[0], array[1]);
}
STATIC int
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
      msg_print("Good hit! (x2 damage)");
    } else if (weight < 700) {
      critical = 3 * dam + 10;
      msg_print("Excellent hit! (x3 damage)");
    } else if (weight < 900) {
      critical = 4 * dam + 15;
      msg_print("Superb hit! (x4 damage)");
    } else {
      critical = 5 * dam + 20;
      msg_print("Legendary hit! (x5 damage)");
    }
  }
  return (critical);
}
STATIC int
tot_dam(obj, tdam, cidx)
struct objS* obj;
{
  struct creatureS* cr_ptr;
  struct recallS* r_ptr;

  cr_ptr = &creatureD[cidx];
  r_ptr = &recallD[cidx];
  if ((obj->flags & TR_EGO_WEAPON) &&
      (obj->tval == TV_HAFTED || obj->tval == TV_POLEARM ||
       obj->tval == TV_SWORD)) {
    /* Slay Dragon  */
    if ((cr_ptr->cdefense & CD_DRAGON) && (obj->flags & TR_SLAY_DRAGON)) {
      tdam = tdam * 4;
      r_ptr->r_cdefense |= CD_DRAGON;
    }
    /* Slay Undead  */
    else if ((cr_ptr->cdefense & CD_UNDEAD) && (obj->flags & TR_SLAY_UNDEAD)) {
      tdam = tdam * 3;
      r_ptr->r_cdefense |= CD_UNDEAD;
    }
    /* Slay Animal  */
    else if ((cr_ptr->cdefense & CD_ANIMAL) && (obj->flags & TR_SLAY_ANIMAL)) {
      tdam = tdam * 2;
      r_ptr->r_cdefense |= CD_ANIMAL;
    }
    /* Slay Evil     */
    else if ((cr_ptr->cdefense & CD_EVIL) && (obj->flags & TR_SLAY_EVIL)) {
      tdam = tdam * 2;
      r_ptr->r_cdefense |= CD_EVIL;
    }
    /* Frost         */
    else if ((cr_ptr->cdefense & CD_FROST) && (obj->flags & TR_FROST_BRAND)) {
      tdam = tdam * 3 / 2;
      r_ptr->r_cdefense |= CD_FROST;
    }
    /* Fire        */
    else if ((cr_ptr->cdefense & CD_FIRE) && (obj->flags & TR_FLAME_TONGUE)) {
      tdam = tdam * 3 / 2;
      r_ptr->r_cdefense |= CD_FIRE;
    }
  }
  return (tdam);
}
STATIC void build_type1(ychunk, xchunk, ycenter, xcenter) int* ycenter;
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
STATIC void build_room(ychunk, xchunk, ycenter, xcenter) int *ycenter, *xcenter;
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
STATIC void
build_store(sidx, y, x)
{
  int yval, y_height, y_depth;
  int xval, x_left, x_right;
  int i, j, tmp;

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

  struct objS* obj = obj_use();
  obj->fy = i;
  obj->fx = j;
  obj->tval = TV_STORE_DOOR;
  obj->tchar = '1' + sidx;
  obj->number = 1;
  caveD[i][j].oidx = obj_index(obj);
  caveD[i][j].fval = FLOOR_CORR;
}
STATIC void
side_entrance(rv, tchar)
{
  int y, x, x_left, x_right;

  if (rv) {
    x_left = 1;
    x = x_right = 1 + randint(2) - 1;
  } else {
    x = x_left = SYMMAP_WIDTH - randint(2) - 1;
    x_right = SYMMAP_WIDTH - 2;
  }
  y = SYMMAP_HEIGHT / 4 + randint(SYMMAP_HEIGHT / 2);

  for (int i = y - 1; i <= y + 1; i++)
    for (int j = x_left; j <= x_right; j++) {
      caveD[i][j].fval = BOUNDARY_WALL;
      caveD[i][j].cflag |= CF_PERM_LIGHT;
    }

  struct objS* obj = obj_use();
  obj->fy = y;
  obj->fx = x;
  obj->tval = TV_STORE_DOOR;
  obj->tchar = tchar;
  obj->number = 1;
  caveD[y][x].oidx = obj_index(obj);
  caveD[y][x].fval = FLOOR_CORR;
}
STATIC int
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
STATIC int
build_bounds(row, col)
{
  uint32_t urow = row - 2;
  uint32_t ucol = col - 2;
  return urow < (MAX_HEIGHT - 4) && ucol < (MAX_WIDTH - 4);
}
STATIC int
diff_chunk(y1, x1, y2, x2)
{
  return (y1 / CHUNK_HEIGHT != y2 / CHUNK_HEIGHT ||
          x1 / CHUNK_WIDTH != x2 / CHUNK_WIDTH);
}
STATIC void
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
STATIC void
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
STATIC void
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
STATIC void
place_door(y, x)
{
  int tmp;
  int lock;

  if (obj_usedD < AL(objD)) {
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
}

STATIC int
same_chunk(y1, x1, y2, x2)
{
  if (x1 / CHUNK_WIDTH != x2 / CHUNK_WIDTH) return 0;
  if (y1 / CHUNK_HEIGHT != y2 / CHUNK_HEIGHT) return 0;
  return 1;
}
STATIC point_t
protect_floor(y, x, ydir, xdir)
{
  struct caveS* c_ptr1;
  struct caveS* c_ptr2;
  struct caveS* c_ptr3;
  // Perpendicular
  int oy = xdir;
  int ox = ydir;
  int ret = 0;
  point_t pt = {0};

  for (int it = 0; it < 2; ++it) {
    // require granite from the same room
    int sc = same_chunk(y + oy, x + ox, y - oy, x - ox);
    if (sc) {
      c_ptr1 = &caveD[y][x];
      c_ptr2 = &caveD[y + oy][x + ox];
      c_ptr3 = &caveD[y - oy][x - ox];
      // TBD: NOT MAGMA is important
      ret = (c_ptr2->fval == GRANITE_WALL) + (c_ptr3->fval == GRANITE_WALL);
      ret += (c_ptr2->fval == QUARTZ_WALL) + (c_ptr3->fval == QUARTZ_WALL);

      if (ret == 2) {
        c_ptr1->fval = FLOOR_THRESHOLD;
        c_ptr2->fval = QUARTZ_WALL;
        c_ptr3->fval = QUARTZ_WALL;

        // Write-back movement, if any
        pt.x = x;
        pt.y = y;
        break;
      }
    }

    if (caveD[y + oy][x + ox].fval >= MIN_WALL &&
        same_chunk(y, x, y + oy, x + ox)) {
      y = y + oy;
      x = x + ox;
    } else if (caveD[y - oy][x - ox].fval >= MIN_WALL &&
               same_chunk(y, x, y - oy, x - ox)) {
      y = y - oy;
      x = x - ox;
    }
    if (!build_bounds(y, x)) break;
  }

  return pt;
}
STATIC point_t
find_perp_threshold(row, col, row_dir, col_dir)
{
  point_t pt = {0};
  int oy = col_dir;
  int ox = row_dir;
  if (caveD[row + oy][col + ox].fval == FLOOR_THRESHOLD)
    pt = (point_t){col + ox, row + oy};
  else if (caveD[row - oy][col - ox].fval == FLOOR_THRESHOLD)
    pt = (point_t){col - ox, row - oy};
  return pt;
}
STATIC int
bestdir(row1, col1, row2, col2)
{
  int dy = row2 - row1;
  int dx = col2 - col1;
  if (ABS(dy) > ABS(dx)) {
    if (dy < 0) return 1;
    if (dy > 0) return 2;
  } else {
    if (dx < 0) return 3;
    if (dx > 0) return 4;
  }
  return 0;
}
// exact: (df_dir ^ df_heading) == 0;
// not away from: (df_dir & df_heading) == df_dir;
// any common: (df_dir & df_heading) != 0;
STATIC int
dirflag(row1, col1, row2, col2)
{
  int ret = 0;
  int dy = row2 - row1;
  int dx = col2 - col1;
  ret |= ((dy < 0) << 0);
  ret |= ((dy > 0) << 1);
  ret |= ((dx < 0) << 2);
  ret |= ((dx > 0) << 3);
  // ret |= ((ABS(dy) > ABS(dx)) << 4);
  return ret;
}
STATIC int
build_diag(row1, col1, row2, col2, tunstk, tunindex)
point_t* tunstk;
int* tunindex;
{
  int mr = MIN(row1, row2);
  int mc = MIN(col1, col2);
  int ret = 0;
  for (int r = 0; r <= 1; ++r) {
    for (int c = 0; c <= 1; ++c) {
      struct caveS* c_ptr = &caveD[mr + r][mc + c];
      ret += (c_ptr->fval <= MAX_FLOOR);
      if (!c_ptr->fval) {
        tunstk[*tunindex] = (point_t){mc + c, mr + r};
        *tunindex += 1;
      }
    }
  }
  return ret;
}
STATIC void perp_rc_dir(tmp_row, tmp_col, row_dir, col_dir) int* row_dir;
int* col_dir;
{
  // Perpendicular swap
  int oy = *col_dir;
  int ox = *row_dir;

  if (build_bounds(tmp_row + oy, tmp_col + ox) &&
      same_chunk(tmp_row, tmp_col, tmp_row + oy, tmp_col + ox)) {
    *row_dir = oy;
    *col_dir = ox;
  }
  if (build_bounds(tmp_row - oy, tmp_col - ox) &&
      same_chunk(tmp_row, tmp_col, tmp_row - oy, tmp_col - ox)) {
    *row_dir = -oy;
    *col_dir = -ox;
  }
}
STATIC void
build_corridor(row1, col1, row2, col2)
{
  int tmp_row, tmp_col, i;
  struct caveS* c_ptr;
  struct caveS* d_ptr;
  point_t tunstk[1000], wallstk[1000];
  point_t* tun_ptr;
  int row_dir, col_dir, tunindex, wallindex;
  int door_flag, main_loop_count;
  int start_row, start_col;

  /* Main procedure for Tunnel  		*/
  door_flag = 0;
  tunindex = 0;
  wallindex = 0;
  main_loop_count = 0;
  tmp_row = start_row = row1;
  tmp_col = start_col = col1;
  int tun_chg = 1;

  while (1) {
    /* prevent infinite loops, just in case */
    main_loop_count++;
    if (main_loop_count > 2000) break;
    if (tunindex + 9 >= AL(tunstk)) break;
    if (wallindex >= AL(wallstk)) break;

    if (tun_chg) {
      int choice = bestdir(row1, col1, row2, col2);
      if (!choice) break;

      // Sometimes take a random direction
      if (randint(DUN_TUN_RND) == 1) {
        choice = randint(4);
      }

      switch (choice) {
        case 1:
          row_dir = -1;
          col_dir = 0;
          break;
        case 2:
          row_dir = 1;
          col_dir = 0;
          break;
        case 3:
          row_dir = 0;
          col_dir = -1;
          break;
        case 4:
          row_dir = 0;
          col_dir = 1;
          break;
      }
    }
    tun_chg = randint(100) > DUN_TUN_CHG;

    tmp_row = row1 + row_dir;
    tmp_col = col1 + col_dir;

    if (!build_bounds(tmp_row, tmp_col)) {
      if (tmp_row < 2) tmp_row = 2;
      if (tmp_row + 2 >= MAX_HEIGHT) tmp_row = MAX_HEIGHT - 2;
      if (tmp_col < 2) tmp_col = 2;
      if (tmp_col + 2 >= MAX_WIDTH) tmp_col = MAX_WIDTH - 2;
      continue;
    }
    c_ptr = &caveD[tmp_row][tmp_col];
    int fval = c_ptr->fval;

    if (fval == QUARTZ_WALL) {
      int fill = 0;
      point_t th = find_perp_threshold(tmp_row, tmp_col, row_dir, col_dir);
      if (th.x) {
        int df_heading = dirflag(row1, col1, tmp_row, tmp_col);
        int df_th = dirflag(row1, col1, th.y, th.x);

        int heading = (df_th & df_heading) != 0;

        if (heading) {
          fill = build_diag(row1, col1, th.y + row_dir, th.x + col_dir, tunstk,
                            &tunindex);

          if (fill) {
            tmp_row = th.y;
            tmp_col = th.x;
            tun_chg = 0;
          }
        }
      }

      // fallback to granite wall treatment
      if (!fill) fval = GRANITE_WALL;
    }

    if (fval == FLOOR_NULL) {
      tunstk[tunindex].y = tmp_row;
      tunstk[tunindex].x = tmp_col;
      tunindex++;
      door_flag = FALSE;
    } else if (fval == MAGMA_WALL) {
      // pass-thru
    } else if (fval == GRANITE_WALL) {
      tun_chg = 0;

      // Prevent chewing room boundary
      // Prevent diagonal entrance to rooms
      // (build_corridor does not travel diagonal)
      point_t th = protect_floor(tmp_row, tmp_col, row_dir, col_dir);
      //  otherwise retry following perp
      if (!th.x) {
        perp_rc_dir(tmp_row, tmp_col, &row_dir, &col_dir);
        continue;
      }

      if (th.x != tmp_col || th.y != tmp_row) {
        build_diag(row1, col1, th.y, th.x, tunstk, &tunindex);
        tmp_col = th.x;
        tmp_row = th.y;
      }

      // Review later for door placement
      wallstk[wallindex].y = tmp_row;
      wallstk[wallindex].x = tmp_col;
      wallindex++;
      door_flag = TRUE;
    } else if (fval == FLOOR_CORR) {
      if (doorindex < AL(doorstk)) {
        if (!door_flag) {
          door_flag = TRUE;
          doorstk[doorindex].y = tmp_row;
          doorstk[doorindex].x = tmp_col;
          doorindex++;
        }
      }

      enum { DUN_TUN_CON = 15 };
      if (randint(100) < DUN_TUN_CON) {
        int cdis = distance(tmp_row, tmp_col, start_row, start_col);
        if (cdis > 16) break;
      }
    }

    row1 = tmp_row;
    col1 = tmp_col;

    // check for completion
    if ((c_ptr->cflag & CF_ROOM) && same_chunk(tmp_row, tmp_col, row2, col2))
      break;
  }

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

    if (!c_ptr->oidx) {
      if (c_ptr->cflag & CF_UNUSUAL) {
        if (randint(3) == 1) {
          place_secret_door(wallstk[i].y, wallstk[i].x);
        } else {
          place_closed_door(randint(21) - 11, wallstk[i].y, wallstk[i].x);
        }
      } else {
        if (randint(100) < DUN_TUN_PEN) place_door(wallstk[i].y, wallstk[i].x);
      }
    }
  }
}
STATIC void
granite_cave()
{
  int i, j;
  struct caveS* c_ptr;

  for (i = MAX_HEIGHT - 2; i > 0; i--) {
    c_ptr = &caveD[i][1];
    for (j = MAX_WIDTH - 2; j > 0; j--) {
      if (c_ptr->fval == FLOOR_NULL) c_ptr->fval = GRANITE_WALL;
      c_ptr++;
    }
  }
}
STATIC void
delete_object(y, x)
{
  struct caveS* cave_ptr;
  cave_ptr = &caveD[y][x];
  obj_unuse(&entity_objD[cave_ptr->oidx]);
  cave_ptr->oidx = 0;
  cave_ptr->cflag &= ~CF_FIELDMARK;
}
STATIC char*
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
STATIC void
inven_destroy(iidx)
{
  struct objS* obj = obj_get(invenD[iidx]);
  obj_unuse(obj);
  invenD[iidx] = 0;
}
STATIC void
inven_destroy_num(iidx, number)
{
  struct objS* obj = obj_get(invenD[iidx]);
  if (obj->number <= number)
    inven_destroy(iidx);
  else
    obj->number -= number;
}
STATIC int
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
STATIC void
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

  if (TEST_CAVEGEN) {
    if (y < 2 || y + 2 > MAX_HEIGHT) exit(1);
    if (x < 1 || x + 2 > MAX_WIDTH) exit(1);
  }
}
STATIC void new_spot(y, x) int *y, *x;
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
STATIC int
next_to_object(y1, x1)
{
  for (int row = y1 - 1; row <= y1 + 1; ++row) {
    for (int col = x1 - 1; col <= x1 + 1; ++col) {
      if (caveD[row][col].oidx) return TRUE;
    }
  }
  return FALSE;
}
STATIC void
place_stairs(tval, num)
{
  struct caveS* cave_ptr;
  int i, j, flag;
  int y1, x1, y2, x2;

  for (i = 0; i < num; i++) {
    flag = FALSE;
    j = 0;
    do {
      /* Note: don't let y1/x1 be one
       * don't let y2/x2 be equal to cur_height-2/cur_width-2
       * viz_minimap_stair is simplest when we are not adjacent to BOUNDARY_ROCK
       * */
      y1 = 1 + randint(MAX_HEIGHT - 15);
      x1 = 1 + randint(MAX_WIDTH - 15);
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
STATIC int
num_adjacent_corridor(y, x)
{
  int i;

  i = 0;
  ADJ4(y, x, {
    if (c_ptr->fval == FLOOR_CORR && c_ptr->oidx == 0) i++;
  });
  return (i);
}
STATIC int
is_corridor(y, x)
{
  int ret;
  int adj;

  ret = FALSE;
  adj = num_adjacent_corridor(y, x);
  if (adj >= 2) {
    if ((caveD[y - 1][x].fval >= MIN_WALL) &&
        (caveD[y + 1][x].fval >= MIN_WALL))
      ret = TRUE;
    else if ((caveD[y][x - 1].fval >= MIN_WALL) &&
             (caveD[y][x + 1].fval >= MIN_WALL))
      ret = TRUE;
  }
  return ret;
}
STATIC void
try_door(y, x)
{
  if (caveD[y][x].fval == FLOOR_CORR && randint(100) <= DUN_TUN_JCT) {
    if (is_corridor(y, x)) place_door(y, x);
  }
}
STATIC int
set_null()
{
  return FALSE;
}
STATIC int
check_fval_type(fval, ftyp)
{
  switch (ftyp) {
    case FT_ANY:
      return (fval <= MAX_FLOOR);
    case FT_ROOM:
      return (fval == FLOOR_DARK || fval == FLOOR_LIGHT);
    case FT_CORR:
      return (fval == FLOOR_CORR || fval == FLOOR_OBST);
  }
  return 0;
}
STATIC void tr_obj_copy(tidx, obj) struct objS* obj;
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
STATIC int
magik(chance)
{
  return (randint(100) <= chance);
}
STATIC int
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
STATIC int
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
STATIC int
ustackweight()
{
  return 10 * statD.use_stat[A_STR];
}
STATIC int
stacklimit_by_max_weight(max, weight)
{
  int stacklimit = 1;
  while (2 * stacklimit * weight <= max) {
    stacklimit *= 2;
  }
  return MIN(stacklimit, 255);
}
STATIC int
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
STATIC void magic_treasure(obj, level) struct objS* obj;
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
STATIC int
mask_subval(sv)
{
  return MASK_SUBVAL & sv;
}
STATIC int
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
STATIC void tr_unknown_sample(tr_ptr, unknown, sample) struct treasureS* tr_ptr;
int* unknown;
char** sample;
{
  int subval = mask_subval(tr_ptr->subval);
  int k = knowable_tval_subval(tr_ptr->tval, subval);
  int trk = 0;
  if (k) trk = knownD[k - 1][subval];
  *unknown = (trk & TRK_FULL) == 0;
  *sample = trk & TRK_SAMPLE ? "{sampled}" : "";
}
STATIC int
tr_is_known(tr_ptr)
struct treasureS* tr_ptr;
{
  int subval = mask_subval(tr_ptr->subval);
  int k = knowable_tval_subval(tr_ptr->tval, subval);
  if (k == 0) return TRUE;
  return knownD[k - 1][subval] & TRK_FULL;
}
STATIC void tr_discovery(tr_ptr) struct treasureS* tr_ptr;
{
  // TDB: xp tuning
  uD.exp += (tr_ptr->level + (uD.lev >> 1)) / uD.lev;
}
STATIC int
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
STATIC void tr_sample(tr_ptr) struct treasureS* tr_ptr;
{
  int subval = mask_subval(tr_ptr->subval);
  int k = knowable_tval_subval(tr_ptr->tval, subval);
  if (k) knownD[k - 1][subval] = TRK_SAMPLE;
}
STATIC int
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
STATIC int
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
STATIC int
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
STATIC int
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
STATIC int
vuln_frost(obj)
struct objS* obj;
{
  return ((obj->tval == TV_POTION1) || (obj->tval == TV_POTION2) ||
          (obj->tval == TV_FLASK));
}
STATIC int
vuln_lightning(obj)
struct objS* obj;
{
  return ((obj->tval == TV_RING) || (obj->tval == TV_WAND));
}
STATIC int
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
STATIC int
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
STATIC int
crset_evil(cre)
struct creatureS* cre;
{
  return (cre->cdefense & CD_EVIL);
}
STATIC int
crset_visible(cre)
struct creatureS* cre;
{
  return (cre->cmove & CM_INVISIBLE) == 0;
}
STATIC int
crset_invisible(cre)
struct creatureS* cre;
{
  return (cre->cmove & CM_INVISIBLE);
}
STATIC int
oset_sightfm(obj)
struct objS* obj;
{
  return (obj->tval >= TV_MIN_VISIBLE && obj->tval <= TV_MAX_VISIBLE);
}
STATIC int
oset_mon_pickup(obj)
struct objS* obj;
{
  // Underflow to exclude 0
  uint8_t tval = obj->tval - 1;
  return (tval < TV_MON_PICK_UP);
}
STATIC int
oset_pickup(obj)
struct objS* obj;
{
  // Underflow to exclude 0
  uint8_t tval = obj->tval - 1;
  return (tval < TV_MAX_PICK_UP);
}
STATIC int
mpickup_obj(obj)
struct objS* obj;
{
  // Underflow to exclude 0
  uint8_t tval = obj->tval - 1;
  return (tval < TV_MON_PICK_UP);
}
STATIC int
oset_trap(obj)
struct objS* obj;
{
  return (obj->tval == TV_VIS_TRAP || obj->tval == TV_INVIS_TRAP ||
          obj->tval == TV_CHEST);
}
STATIC int
oset_doorstair(obj)
struct objS* obj;
{
  return (obj->tval == TV_SECRET_DOOR || obj->tval == TV_UP_STAIR ||
          obj->tval == TV_DOWN_STAIR);
}
STATIC int
oset_hidden(obj)
struct objS* obj;
{
  return (obj->tval == TV_VIS_TRAP || obj->tval == TV_INVIS_TRAP ||
          obj->tval == TV_CHEST || obj->tval == TV_CLOSED_DOOR ||
          obj->tval == TV_SECRET_DOOR);
}
STATIC int
oset_zap(obj)
struct objS* obj;
{
  switch (obj->tval) {
    case TV_WAND:
    case TV_STAFF:
      return 1;
  }
  return 0;
}
STATIC int
oset_weightcheck(obj)
struct objS* obj;
{
  switch (obj->tval) {
    case TV_LAUNCHER:
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
    case TV_DIGGING:
      return 1;
  }
  return 0;
}
STATIC int
oset_tohitdam(obj)
struct objS* obj;
{
  switch (obj->tval) {
    case TV_LAUNCHER:
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
    case TV_LIGHT:
      return 1;
    case TV_GLOVES:
    case TV_RING:
      return (obj->tohit || obj->todam);
  }
  return 0;
}
STATIC int
oset_equip(obj)
struct objS* obj;
{
  uint32_t tv = obj->tval;
  tv -= TV_MIN_EQUIP;
  return tv <= (TV_MAX_EQUIP - TV_MIN_EQUIP);
}
STATIC int
oset_enchant(obj)
struct objS* obj;
{
  uint32_t tv = obj->tval;
  tv -= TV_MIN_ENCHANT;
  return tv <= (TV_MAX_ENCHANT - TV_MIN_ENCHANT);
}
STATIC int
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
STATIC int
oset_melee(obj)
struct objS* obj;
{
  switch (obj->tval) {
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
      return 1;
  }
  return 0;
}
STATIC int
oset_dice(obj)
struct objS* obj;
{
  switch (obj->tval) {
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
    case TV_PROJECTILE:
      return 1;
  }
  return 0;
}
STATIC int
oset_dammult(obj)
struct objS* obj;
{
  switch (obj->tval) {
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
    case TV_LAUNCHER:
      return 1;
  }
  return 0;
}
STATIC int
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
STATIC int
oset_gold(obj)
struct objS* obj;
{
  return obj->tval == TV_GOLD;
}
STATIC int
oset_obj(obj)
struct objS* obj;
{
  return obj->id != 0;
}
STATIC int
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
STATIC int
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
STATIC int
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
STATIC int
ring_slot()
{
  int slot = INVEN_RING;
  for (int it = 0; it < 2; ++it, ++slot) {
    if (invenD[slot] == 0) return slot;
  }
  return -1;
}
STATIC void
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
STATIC int
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
STATIC int
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
STATIC void
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
STATIC void
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
STATIC void
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
STATIC int
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
STATIC int
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
STATIC int
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
STATIC int
place_win_monster()
{
  int cidx, fy, fx, y, x, k;
  struct creatureS* cr_ptr;

  k = randint(MAX_WIN_MON);
  cidx = k + m_level[MAX_MON_LEVEL];
  cr_ptr = &creatureD[cidx];
  y = uD.y;
  x = uD.x;

  if (k == MAX_WIN_MON && !uD.total_winner) {
    msg_print("You hear a low rumble echo through the caverns.");
    do {
      fy = randint(MAX_HEIGHT - 2);
      fx = randint(MAX_WIDTH - 2);
    } while ((caveD[fy][fx].fval >= MIN_CLOSED_SPACE) || (caveD[fy][fx].midx) ||
             (caveD[fy][fx].oidx) || (distance(fy, fx, y, x) <= MAX_SIGHT));

    return place_monster(fy, fx, cidx, 1);
  }

  return 0;
}
STATIC int
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
STATIC int
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
STATIC void
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
STATIC void
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
STATIC void
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
STATIC void
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
STATIC int
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
  return obj->id != 0;
}
STATIC int
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
  return obj->id != 0;
}
STATIC void
alloc_obj(ftyp, otyp, num)
{
  for (int it = 0; it < num; it++) {
    int y, x;
    do {
      y = randint(MAX_HEIGHT) - 1;
      x = randint(MAX_WIDTH) - 1;
    }
    /* don't put an object beneath the player, this could cause problems
       if player is standing under rubble, or on a trap */
    while (!check_fval_type(caveD[y][x].fval, ftyp) ||
           (caveD[y][x].oidx != 0) || (y == uD.y && x == uD.x));
    switch (otyp) {
      case 1:
        place_trap(y, x, randint(MAX_TRAP) - 1);
        break;
      case 2:
        place_rubble(y, x);
        break;
      case 3:
        place_gold(y, x);
        break;
      case 4:
        place_object(y, x, FALSE);
        break;
    }
  }
}
STATIC void
build_vault(y, x)
{
  if (!TEST_CAVEGEN) {
    for (int i = -1; i <= 1; ++i) {
      for (int j = -1; j <= 1; ++j) {
        if (i != 0 || j != 0) caveD[y + i][x + j].fval = MAGMA_WALL;
      }
    }
  }
}
STATIC void
build_pillar(y, x)
{
  build_vault(y, x);
  caveD[y][x].fval = MAGMA_WALL;
}
STATIC void
build_chamber(y, x, h, w)
{
  for (int j = -w; j <= w; ++j) {
    caveD[y + h][x + j].fval = MAGMA_WALL;
    caveD[y - h][x + j].fval = MAGMA_WALL;
  }
  caveD[y][x - w].fval = MAGMA_WALL;
  caveD[y][x + w].fval = MAGMA_WALL;
  caveD[y][x].fval = MAGMA_WALL;
}
STATIC int
chunk_obj(ychunk, xchunk, obj_count, func)
fn func;
{
  int xmin = xchunk * CHUNK_WIDTH - 1;
  int ymin = ychunk * CHUNK_HEIGHT - 1;
  int count = 0;
  for (int it = 0; it < obj_count; ++it) {
    for (int retry = 0; retry < RETRY; ++retry) {
      int x = xmin + randint(CHUNK_WIDTH);
      int y = ymin + randint(CHUNK_HEIGHT);
      struct caveS* c_ptr = &caveD[y][x];
      if (c_ptr->fval > 0 && c_ptr->fval <= MAX_OPEN_SPACE &&
          c_ptr->oidx == 0) {
        count += func(y, x, 0);
        retry = RETRY;
      }
    }
  }
  return count;
}
STATIC void
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
// fills a rectangular area with walls and floor tiles
STATIC void fill_rectangle(xmin, xmax, ymin, ymax, rflag, floor,
                           setup_func) fn setup_func;
{
  struct caveS* c_ptr;

  for (int i = ymin; i <= ymax; ++i) {
    for (int j = xmin; j <= xmax; ++j) {
      c_ptr = &caveD[i][j];
      c_ptr->cflag |= rflag;

      if (i == ymin || i == ymax || j == xmin || j == xmax) {
        c_ptr->fval = GRANITE_WALL;
      } else {
        if (setup_func)
          setup_func(c_ptr, i, j, floor);
        else
          c_ptr->fval = floor;
      }
    }
  }
}
STATIC int
checkered_setup(struct caveS* c_ptr, int i, int j, int floor)
{
  if ((i ^ j) & 0x1) {
    c_ptr->fval = MAGMA_WALL;
  } else {
    c_ptr->fval = floor;
  }
}
STATIC int
quadrant_setup(struct caveS* c_ptr, int i, int j, int floor)
{
  if (i % CHUNK_HEIGHT == (CHUNK_HEIGHT - 1) / 2)
    c_ptr->fval = MAGMA_WALL;
  else if (j % CHUNK_WIDTH == (CHUNK_WIDTH - 1) / 2)
    c_ptr->fval = MAGMA_WALL;
  else
    c_ptr->fval = floor;
}
STATIC int
preserve_setup(struct caveS* c_ptr, int i, int j, int floor)
{
  if (c_ptr->fval == 0) {
    c_ptr->fval = floor;
  }
}
STATIC void build_type2(ychunk, xchunk, ycenter, xcenter, type1,
                        type2) int* ycenter;
int* xcenter;
{
  int floor, ydoor;
  int xmin, xmax, ymin, ymax;
  int y, x;
  int obj_count = 0;
  int trap_count = 0;

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
      fill_rectangle(xmin, xmax, ymin, ymax, CF_ROOM, floor, 0);
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
      fill_rectangle(xmin, xmax, ymin, ymax, CF_ROOM, floor, preserve_setup);
      break;
    case 3:
      xmax -= 1;
      ymax -= 1;
      fill_rectangle(xmin, xmax, ymin, ymax, CF_ROOM, floor, 0);

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
      fill_rectangle(xmin, xmax, ymin, ymax, CF_ROOM | CF_UNUSUAL, floor,
                     checkered_setup);
      // Maze
      trap_count = 2;
      obj_count = randint(3);
      room_monster(y, x - 5, 3);
      room_monster(y, x + 5, 3);
      break;
    case 5:
      ymax -= 1;
      fill_rectangle(xmin, xmax, ymin, ymax, CF_ROOM | CF_UNUSUAL, floor,
                     quadrant_setup);

      ydoor = ymin + randint(ymax - ymin);
      place_secret_door(ydoor, x);
      if (ydoor != y) {
        place_secret_door(y, xmin + randint(CHUNK_WIDTH / 2 - 2));
        place_secret_door(y, x + randint(CHUNK_WIDTH / 2 - 2));
      }

      obj_count = 2 + randint(2);
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
      fill_rectangle(xmin, xmax, ymin, ymax, CF_ROOM, floor, 0);

      ymin = ychunk * CHUNK_HEIGHT;
      ymax = ymin + CHUNK_HEIGHT - 1;
      xmin = x - 2;
      xmax = x + 2;
      fill_rectangle(xmin, xmax, ymin, ymax, CF_ROOM, floor, 0);

      ymin = y - 1;
      ymax = y + 2;
      xmin = x - 2;
      xmax = x + 2;
      for (int i = ymin; i <= ymax; ++i) {
        for (int j = xmin; j <= xmax; ++j) {
          if (i != ymin && i != ymax) caveD[i][j].fval = floor;
          if (j != xmin && j != xmax) caveD[i][j].fval = floor;
        }
      }

      if (type2 & 0x1) {
        caveD[y][x].fval = MAGMA_WALL;
        caveD[y + 1][x].fval = MAGMA_WALL;
      }
      if (type2 > 4) {
        caveD[y - 1][x - 1].fval = MAGMA_WALL;
        caveD[y - 1][x + 1].fval = MAGMA_WALL;
        caveD[y + 2][x - 1].fval = MAGMA_WALL;
        caveD[y + 2][x + 1].fval = MAGMA_WALL;
      }
      trap_count = 1 + randint(2);
      break;
  }

  if (obj_count > 0) chunk_obj(ychunk, xchunk, obj_count, place_object);
  if (trap_count > 0) chunk_obj(ychunk, xchunk, trap_count, place_trap);

  *ycenter = y;
  *xcenter = x;
}
STATIC int
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
STATIC void
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
STATIC int
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
  for (i = 0; i < AL(room_map); i++) {
    for (j = 0; j < AL(room_map[0]); j++) {
      if (room_map[i][j]) {
        if (dun_level > randint(DUN_UNUSUAL)) {
          pick2 += 1;
          build_type2(i, j, &yloc[k], &xloc[k], randint(6), randint(8));
        } else {
          // TBD: type1 rooms do not play well with protect_floor
          // if (room_map[i][j] == 1) {
          build_room(i, j, &yloc[k], &xloc[k]);
          //} else {
          //  pick1 += 1;
          //  build_type1(i, j, &yloc[k], &xloc[k]);
          //}
        }
        k++;
      }
    }
  }

  for (j = 0; j < k; j++) {
    pick1 = randint(k) - 1;
    pick2 = randint(k) - 1;

    // swap
    y1 = yloc[pick1];
    x1 = xloc[pick1];
    yloc[pick1] = yloc[pick2];
    xloc[pick1] = xloc[pick2];
    yloc[pick2] = y1;
    xloc[pick2] = x1;
  }
  /* move zero entry to k, so that can call build_corridor all k times */
  yloc[k] = yloc[0];
  xloc[k] = xloc[0];
  for (j = 0; j < k; j++) {
    y1 = yloc[j];
    x1 = xloc[j];
    y2 = yloc[j + 1];
    x2 = xloc[j + 1];
    // connect each room to another
    build_corridor(y2, x2, y1, x1);
  }

  granite_cave();
  place_boundary();
  for (i = 0; i < DUN_STR_MAG; i++) place_streamer(MAGMA_WALL, DUN_STR_MC);
  for (i = 0; i < DUN_STR_QUA; i++) place_streamer(QUARTZ_WALL, DUN_STR_QC);

  /* Corridor intersection doors  */
  for (i = 0; i < doorindex; i++) {
    try_door(doorstk[i].y, doorstk[i].x - 1);
    try_door(doorstk[i].y, doorstk[i].x + 1);
    try_door(doorstk[i].y - 1, doorstk[i].x);
    try_door(doorstk[i].y + 1, doorstk[i].x);
  }
  // Exits
  place_stairs(TV_DOWN_STAIR, randint(2));
  place_stairs(TV_UP_STAIR, 1);
  // Character co-ords; used by alloc_monster, place_win_monster
  new_spot(&uD.y, &uD.x);

  if (dun_level >= 7) alloc_obj(FT_CORR, 2, randint(alloc_level));
  alloc_obj(FT_ROOM, 4, randnor(TREAS_ROOM_MEAN, 3));
  alloc_obj(FT_ANY, 4, randnor(TREAS_ANY_ALLOC, 3));
  alloc_obj(FT_ANY, 3, randnor(TREAS_GOLD_ALLOC, 3));
  alloc_obj(FT_ANY, 1, randint(alloc_level));

  alloc_mon((randint(RND_MALLOC_LEVEL) + MIN_MALLOC_LEVEL + alloc_level), 0,
            TRUE);
  if (dun_level >= WIN_MON_APPEAR) place_win_monster();
  return 0;
}
STATIC int
town_night()
{
  return ((turnD / (1 << 12)) & 0x1);
}
STATIC int
town_gen()
{
  int i, j, k;
  struct caveS* c_ptr;
  int room[MAX_STORE];
  int room_used;

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

  int tmp = randint(2);
  side_entrance(tmp, '0');
  if (uD.exp == 0)
    side_entrance(!tmp, '9');
  else if (uvow(VOW_STAT_FEE))
    side_entrance(!tmp, '8');

  do {
    i = 1 + randint(SYMMAP_HEIGHT - 3);
    j = 1 + randint(SYMMAP_WIDTH - 3);
    c_ptr = &caveD[i][j];
  } while (c_ptr->fval >= MIN_CLOSED_SPACE || (c_ptr->oidx != 0) ||
           (c_ptr->midx != 0));
  place_stair_tval(i, j, TV_DOWN_STAIR);
  caveD[i][j].cflag |= CF_FIELDMARK;
  return 0;
}
STATIC int
py_intown()
{
  int i, j;
  struct caveS* c_ptr;

  do {
    i = randint(SYMMAP_HEIGHT - 2);
    j = randint(SYMMAP_WIDTH - 2);
    c_ptr = &caveD[i][j];
  } while (c_ptr->fval >= MIN_CLOSED_SPACE || (c_ptr->oidx != 0) ||
           (c_ptr->midx != 0));
  uD.y = i;
  uD.x = j;
}
STATIC int
hard_reset()
{
  // Clear game state
  memset(__start_game, 0, __stop_game - __start_game);

  // Message history
  AC(msglen_cqD);
  AC(msg_cqD);
  death_descD[0] = 0;
  msg_writeD = 0;

  // Replay
  replay_flag = 0;

  // Reset overlay modes
  overlay_submodeD = 0;
  screen_submodeD = 0;

  return 0;
}
STATIC void
panel_bounds(struct panelS* panel)
{
  int panel_row = panel->panel_row;
  int panel_col = panel->panel_col;
  panel->panel_row_min = panel_row * (SYMMAP_HEIGHT / 2);
  panel->panel_row_max = panel->panel_row_min + SYMMAP_HEIGHT;
  panel->panel_col_min = panel_col * (SYMMAP_WIDTH / 2);
  panel->panel_col_max = panel->panel_col_min + SYMMAP_WIDTH;
}

STATIC void
panel_update(struct panelS* panel, int y, int x, int force)
{
  if (dun_level != 0) {
    int yd = (y < panel->panel_row_min + 2 || y > panel->panel_row_max - 3);
    if (force || yd) {
      int prow = (y - SYMMAP_HEIGHT / 4) / (SYMMAP_HEIGHT / 2);
      panel->panel_row = CLAMP(prow, 0, MAX_ROW - 2);
    }

    int xd = (x < panel->panel_col_min + 2 || x > panel->panel_col_max - 3);
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
STATIC void get_moves(midx, mm) int* mm;
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
STATIC int
see_wall(dir, y, x)
{
  if (!mmove(dir, &y, &x)) /* check to see if movement there possible */
    return TRUE;
  else if (caveD[y][x].fval >= MIN_WALL && CF_VIZ & caveD[y][x].cflag)
    return TRUE;
  else
    return FALSE;
}
STATIC void find_init(dir, y_ptr, x_ptr) int *y_ptr, *x_ptr;
{
  if (!mmove(dir, y_ptr, x_ptr))
    find_flagD = FALSE;
  else {
    find_directionD = dir;
    find_flagD = TRUE;
    find_breakrightD = find_breakleftD = FALSE;
    find_prevdirD = dir;
    if (py_affect(MA_BLIND) == 0) {
      int i = chome[dir];
      int deepleft = 0;
      int deepright = 0;
      int shortleft = 0;
      int shortright = 0;
      if (see_wall(cycle[i + 1], uD.y, uD.x)) {
        find_breakleftD = TRUE;
        shortleft = TRUE;
      } else if (see_wall(cycle[i + 1], *y_ptr, *x_ptr)) {
        find_breakleftD = TRUE;
        deepleft = TRUE;
      }
      if (see_wall(cycle[i - 1], uD.y, uD.x)) {
        find_breakrightD = TRUE;
        shortright = TRUE;
      } else if (see_wall(cycle[i - 1], *y_ptr, *x_ptr)) {
        find_breakrightD = TRUE;
        deepright = TRUE;
      }
      if (find_breakleftD && find_breakrightD) {
        find_openareaD = FALSE;
        if (dir & 1) { /* a hack to allow angled corridor entry */
          if (deepleft && !deepright)
            find_prevdirD = cycle[i - 1];
          else if (deepright && !deepleft)
            find_prevdirD = cycle[i + 1];
        }
        /* else if there is a wall two spaces ahead and seem to be in a
           corridor, then force a turn into the side corridor, must
           be moving straight into a corridor here */
        else if (see_wall(cycle[i], *y_ptr, *x_ptr)) {
          if (shortleft && !shortright)
            find_prevdirD = cycle[i - 2];
          else if (shortright && !shortleft)
            find_prevdirD = cycle[i + 2];
        }
      } else
        find_openareaD = TRUE;
    }
  }
}
STATIC int
see_nothing(dir, y, x)
{
  if (!mmove(dir, &y, &x)) /* check to see if movement there possible */
    return FALSE;
  else if ((CF_VIZ & caveD[y][x].cflag) == 0)
    return TRUE;
  else
    return FALSE;
}
STATIC int
find_event(y, x)
{
  int dir, newdir, t, check_dir, row, col;
  int i, max, option, option2;
  struct caveS* c_ptr;

  option = 0;
  option2 = 0;
  check_dir = 0;
  dir = find_prevdirD;
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
      if (mon_lit(c_ptr->midx)) return 2;

      if (c_ptr->fval <= MAX_OPEN_SPACE) {
        if (find_openareaD) {
          if (i < 0) {
            if (find_breakrightD) {
              return 3;
            }
          } else if (i > 0) {
            if (find_breakleftD) {
              return 4;
            }
          }
        } else if (option == 0)
          option = newdir; /* The first new direction. */
        else if (option2 != 0) {
          return 5; /* Three new directions. STOP. */
        } else if (option != cycle[chome[dir] + i - 1]) {
          return 6; /* If not adjacent to prev, STOP */
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
      } else if (find_openareaD) {
        /* We see an obstacle. In open area, STOP if on a side
           previously open. */
        if (i < 0) {
          if (find_breakleftD) {
            return 7;
          }
          find_breakrightD = TRUE;
        } else if (i > 0) {
          if (find_breakrightD) {
            return 8;
          }
          find_breakleftD = TRUE;
        }
      }
    }
  }

  if (find_openareaD == FALSE) { /* choose a direction. */
    if (option2 == 0) {
      /* There is only one option, or if two, then we always examine
         potential corners and never cur known corners, so you step
         into the straight option. */
      if (option != 0) find_directionD = option;
      if (option2 == 0)
        find_prevdirD = option;
      else
        find_prevdirD = option2;
    } else {
      /* Two options! */
      row = y;
      col = x;
      mmove(option, &row, &col);
      if (!see_wall(option, row, col) || !see_wall(check_dir, row, col)) {
        if (find_threatD) {
          /* STOP: the player can make a choice */
          return 9;
        } else {
          /* Don't see that it is closed off.  This could be a
             potential corner or an intersection. */
          if (see_nothing(option, row, col) && see_nothing(option2, row, col))
          /* Can not see anything ahead and in the direction we are
             turning, assume that it is a potential corner. */
          {
            find_directionD = option;
            find_prevdirD = option2;
          } else {
            /* STOP: we are next to an intersection or a room */
            return 10;
          }
        }
      } else {
        /* This corner is seen to be enclosed; we cut the corner. */
        find_directionD = option2;
        find_prevdirD = option2;
      }
    }
  }

  return 0;
}
STATIC int
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
// Player may use detection flags to light creature up
STATIC int
detect_by_mflag(mflag, cridx)
{
  struct creatureS* cr_ptr = &creatureD[cridx];
  int dflags = 0;
  if (CD_EVIL & cr_ptr->cdefense) {
    dflags |= (1 << MA_DETECT_EVIL);
    recallD[cridx].r_cdefense |= CD_EVIL;
  }

  if ((CM_INVISIBLE & cr_ptr->cmove)) {
    dflags |= (1 << MA_DETECT_INVIS);
    recallD[cridx].r_cmove |= CM_INVISIBLE;
  } else {
    dflags |= (1 << MA_DETECT_MON);
  }

  return (mflag & dflags) != 0;
}
STATIC int
detect_mon(ma_type, known)
{
  int flag = 0;
  int mflag = 1 << ma_type;
  FOR_EACH(mon, {
    if (detect_by_mflag(mflag, mon->cidx)) flag = 1;
  });

  if (flag) {
    msg_print("Your senses tingle!");
  } else if (known) {
    msg_print("You detect nothing further.");
  }

  return flag;
}
STATIC void
move_rec(y1, x1, y2, x2)
{
  int tmp = caveD[y1][x1].midx;
  caveD[y1][x1].midx = 0;
  caveD[y2][x2].midx = tmp;
}
STATIC int
perceive_creature(cr_ptr)
struct creatureS* cr_ptr;
{
  if ((CM_INVISIBLE & cr_ptr->cmove) == 0)
    return TRUE;
  else if (py_tr(TR_SEE_INVIS))
    return TRUE;
  else
    return FALSE;
}
// 2) Additional check for player blindness
STATIC int
perceive_creature2(cr_ptr)
struct creatureS* cr_ptr;
{
  if (py_affect(MA_BLIND)) return 0;
  return perceive_creature(cr_ptr);
}
// dependency on mflag requires ma_tick() to have run
STATIC int
mon_lit(midx)
{
  int fy, fx, cdis;
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;
  MUSE(u, y);
  MUSE(u, x);
  MUSE(u, mflag);
  MUSE(u, infra);
  int lit = 0;

  if (midx) {
    m_ptr = &entity_monD[midx];

    if (detect_by_mflag(mflag, m_ptr->cidx)) {
      lit = 1;
    } else if (maD[MA_BLIND] == 0)
    // direct check of MA_BLIND is because sight can change during creatures()
    {
      fy = m_ptr->fy;
      fx = m_ptr->fx;
      cdis = distance(y, x, fy, fx);
      if (cdis <= MAX_SIGHT && los(y, x, fy, fx)) {
        c_ptr = &caveD[fy][fx];
        cr_ptr = &creatureD[m_ptr->cidx];
        if ((CD_INFRA & cr_ptr->cdefense) && (cdis <= infra)) {
          lit = 1;
          recallD[m_ptr->cidx].r_cdefense |= CD_INFRA;
        } else if (CF_LIT & c_ptr->cflag) {
          lit = perceive_creature(cr_ptr);
          if (lit && (CM_INVISIBLE & cr_ptr->cmove))
            recallD[m_ptr->cidx].r_cmove |= CM_INVISIBLE;
        }
      }
    }
  }

  return lit;
}
STATIC int
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
            recallD[mon->cidx].r_cmove |= CM_EATS_OTHER;
            mon_unuse(&entity_monD[c_ptr->midx]);
            c_ptr->midx = 0;
          }
        }
        if (c_ptr->midx == 0) {
          midx = place_monster(j, k, mon->cidx, FALSE);
          return midx;
        }
      }
    }
    i++;
  } while (i <= 18);

  recallD[mon->cidx].r_cmove |= CM_MULTIPLY;

  return FALSE;
}
STATIC void
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
STATIC int
illuminate(y, x)
{
  if (caveD[y][x].cflag & CF_ROOM) light_room(y, x);
  for (int row = y - 1; row <= y + 1; ++row) {
    for (int col = x - 1; col <= x + 1; ++col) {
      caveD[row][col].cflag |= (CF_PERM_LIGHT | CF_SEEN);
    }
  }

  see_print("You are surrounded by a white light.");

  return py_affect(MA_BLIND) == 0;
}
STATIC void
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
STATIC int
unlight_area(y, x)
{
  int known;

  known = FALSE;
  if (caveD[y][x].cflag & CF_LIT_ROOM) {
    unlight_room(y, x);
    known = TRUE;
  }
  for (int row = y - 1; row <= y + 1; ++row) {
    for (int col = x - 1; col <= x + 1; ++col) {
      if (caveD[row][col].fval < MAX_FLOOR &&
          caveD[row][col].cflag & CF_PERM_LIGHT) {
        caveD[row][col].cflag &= ~CF_PERM_LIGHT;
        known = TRUE;
      }
    }
  }

  if (known) see_print("Darkness surrounds you.");

  return known;
}
STATIC void
py_light_off(y, x)
{
  int row, col;
  for (row = y - 1; row <= y + 1; ++row) {
    for (col = x - 1; col <= x + 1; ++col) {
      caveD[row][col].cflag &= ~CF_TEMP_LIGHT;
    }
  }
}
STATIC void
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
STATIC void
py_check_view()
{
  if (py_affect(MA_BLIND)) {
    py_light_off(uD.y, uD.x);
  } else {
    py_light_on(uD.y, uD.x);
  }
}
STATIC int
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
STATIC int
tohit_by_weight(weight)
{
  int use_weight = statD.use_stat[A_STR] * 15;

  if (use_weight < weight) return use_weight - weight;
  return 0;
}
STATIC int
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
STATIC int
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
STATIC char*
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
STATIC int
sustain_stat(sidx)
{
  uint32_t val = sidx;
  return ((1 << val) | TR_SUST_STAT);
}
STATIC int
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
STATIC int
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
STATIC int
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
STATIC int
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
STATIC int
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
STATIC int
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
STATIC int
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
STATIC int
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
STATIC int
usave()
{
  return uD.save + think_adj(A_WIS) +
         (level_adj[uD.clidx][LA_SAVE] * uD.lev / 3);
}
STATIC int
udevice()
{
  int xdev = uD.save + think_adj(A_INT) +
             (level_adj[uD.clidx][LA_DEVICE] * uD.lev / 3);

  if (countD.confusion) xdev /= 2;
  return xdev;
}
STATIC int
udisarm()
{
  int xdis = uD.disarm + 2 * todis_adj() + think_adj(A_INT) +
             level_adj[uD.clidx][LA_DISARM] * uD.lev / 3;
  if (py_affect(MA_BLIND)) xdis /= 8;
  if (countD.confusion) xdis /= 8;
  return xdis;
}
STATIC int
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
STATIC int
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
STATIC int
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
STATIC int
moriap_fmt_obj(fmt, flen, fflag, obj)
char* fmt;
struct objS* obj;
{
  char* iter = fmt;
  while (fflag != 0 && flen > 0) {
    uint32_t flag = fflag & -fflag;
    int used = -1;
    *iter++ = ' ';
    flen -= 1;
    switch (flag) {
      case FMT_HITDAM:
        used = snprintf(iter, flen, "(%+d,%+d)", obj->tohit, obj->todam);
        break;
      case FMT_DICE:
        used = snprintf(iter, flen, "(%dd%d)", obj->damage[0], obj->damage[1]);
        break;
      case FMT_DAMMULT:
        used = snprintf(iter, flen, "x%d",
                        obj->tval != TV_LAUNCHER ? attack_blows(obj->weight)
                                                 : obj->damage[1]);
        break;
      case FMT_AC:
        used = snprintf(iter, flen, "[%d AC]", obj->ac);
        break;
      case FMT_ACC:
        used = snprintf(iter, flen, "[%d%+d AC]", obj->ac, obj->toac);
        break;
      case FMT_CHARGES:
        used = snprintf(iter, flen, "(%d charges)", obj->p1);
        break;
      case FMT_WEIGHT: {
        int stack_weight = obj->number * obj->weight;
        used = snprintf(iter, flen, "%2d.%01dlb", stack_weight / 10,
                        stack_weight % 10);
      } break;
      case FMT_PAWN:
      case FMT_SHOP: {
        int sidx = obj_store_index(obj);
        used = snprintf(iter, flen, "%5dgp",
                        store_value(sidx, obj_value(obj), flag == FMT_PAWN));
      } break;
    }
    if (used < 0 || used >= flen) break;
    fflag ^= flag;
    flen -= used;
    iter += used;
  }
  if (flen > 0) *iter = 0;
  return iter - fmt;
}
STATIC int
obj_detail(obj, fmt)
struct objS* obj;
{
  int reveal = (obj->idflag & ID_REVEAL);
  if (reveal && oset_tohitdam(obj)) fmt |= FMT_HITDAM;
  if (reveal && oset_zap(obj)) fmt |= FMT_CHARGES;
  // if (reveal && obj->toac) fmt |= FMT_ACC;
  if (oset_armor(obj)) fmt |= reveal ? FMT_ACC : FMT_AC;
  if (oset_dice(obj)) fmt |= FMT_DICE;
  if (oset_dammult(obj)) fmt |= FMT_DAMMULT;

  return moriap_fmt_obj(AP(detailD), fmt, obj);
}
//  copy src to dst with moria formatting spec
STATIC int64_t
moria_ocat_num(dst, dstlen, objname, num)
char* dst;
char* objname;
{
  char* end = dst + dstlen;
  char* iter = dst;
  char c = 0;
  while (*iter) {
    c = *iter++;
  }

  if (oprefixD && iter == dst) {
    int wr = 0;
    if (num > 1) wr = apcati(iter, dstlen, num);
    if (num < 1) wr = apcopy(iter, dstlen, S2("no more"));
    if (num == 1) {
      if (is_a_vowel(objname[0]))
        wr = apcopy(iter, dstlen, S2("an"));
      else
        wr = apcopy(iter, dstlen, S2("a"));
    }
    iter += wr;
  }

  if (iter < end && iter > dst)
    if (iter[-1] != ' ') *iter++ = ' ';

  while ((c = *objname++)) {
    if (c == '~') {
      if (num == 1) continue;
      c = 's';
    }
    if (iter == end) break;
    *iter++ = c;
  }
  if (iter < end) *iter = 0;
  return iter - dst;
}
// p1: range [-9, 9]
STATIC int
moriap_bonus(dst, dstlen, p1)
char* dst;
{
  int used = apcati(dst + 2, dstlen - 2, p1);
  if (used + 4 <= dstlen) {
    dst[0] = '(';
    dst[1] = p1 >= 0 ? '+' : '-';
    dst[used + 2] = ')';
    dst[used + 3] = 0;
  }
  return used + 3;
}
STATIC char*
store_name(tchar)
{
  switch (tchar) {
    case '0':
      return "pawn shop";
    case '1':
      return "general store";
    case '2':
      return "armory";
    case '3':
      return "weaponsmith";
    case '4':
      return "temple";
    case '5':
      return "alchemy shop";
    case '6':
      return "magic shop";
    case '8':
      return "sauna";
    case '9':
      return "player vow";
  }
  return "";
}
#define desc(str) moria_ocat_num(AP(descD), str, number)
STATIC void obj_desc(obj, number) struct objS* obj;
{
  char* prefix = 0;
  char* name = 0;
  char* suffix = 0;
  char* sample = "";
  char bonus[8];
  int unknown = 0;

  apclear(AP(descD));
  apclear(AP(bonus));
  oprefixD = !(obj->tval == TV_SOFT_ARMOR || obj->tval == TV_HARD_ARMOR);
  if (obj->tidx) {
    struct treasureS* tr_ptr = &treasureD[obj->tidx];
    name = tr_ptr->name;

    if (obj->idflag & ID_REVEAL) {
      suffix = special_nameD[obj->sn];
      if (obj->p1) moriap_bonus(AP(bonus), obj->p1);
    } else {
      tr_unknown_sample(tr_ptr, &unknown, &sample);
    }
  }

  int indexx = mask_subval(obj->subval);
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
    case TV_FLASK:
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
      if (unknown)
        prefix = amulets[indexx];
      else
        suffix = name;
      name = "Amulet";
      break;
    case TV_RING:
      if (unknown)
        prefix = rocks[indexx];
      else
        suffix = name;
      name = "Ring";
      break;
    case TV_STAFF:
      if (unknown)
        prefix = woods[indexx];
      else
        suffix = name;
      name = "Staff";
      *bonus = 0;
      break;
    case TV_WAND:
      if (unknown)
        prefix = metals[indexx];
      else
        suffix = name;
      name = "Wand";
      *bonus = 0;
      break;
    case TV_SCROLL1:
    case TV_SCROLL2:
      if (unknown) {
        desc("Scroll~ titled");
        desc(titleD[indexx]);
        name = 0;
      } else {
        suffix = name;
        name = "Scroll~";
      }
      break;
    case TV_POTION1:
    case TV_POTION2:
      if (unknown)
        prefix = colors[indexx];
      else
        suffix = name;
      name = "Potion~";
      *bonus = 0;
      break;
    case TV_FOOD:
      *bonus = 0;
      if (indexx <= 20) {
        if (unknown)
          prefix = mushrooms[indexx];
        else
          suffix = name;
        if (indexx >= 16)
          name = "Mold~";
        else
          name = "Mushroom~";
      }
      break;
    case TV_MAGIC_BOOK:
    case TV_PRAYER_BOOK:
      suffix = name;
      name = "Book~";
      break;
    case TV_GOLD:
      oprefixD = 0;
      name = gold_nameD[indexx];
      break;
    case TV_OPEN_DOOR:
      name = "open door";
      break;
    case TV_CLOSED_DOOR:
      name = "closed door";
      break;
    case TV_RUBBLE:
      name = "rubble";
      break;
    case TV_SECRET_DOOR:
      // name = "secret door";
      return;
    case TV_UP_STAIR:
      name = "staircase up";
      break;
    case TV_DOWN_STAIR:
      name = "staircase down";
      break;
    case TV_INVIS_TRAP:
    case TV_VIS_TRAP:
    case TV_GLYPH:
      break;
    case TV_STORE_DOOR:
      prefix = store_name(obj->tchar);
      name = "entrance";
      break;
    default:
      name = "unknown object";
      break;
  }
  if (prefix) {
    desc(prefix);
    desc(name);
    if (*sample) desc(sample);
  } else if (suffix) {
    desc(name);
    desc("of");
    desc(suffix);
    if (*bonus) desc(bonus);
  } else if (name) {
    desc(name);
  }
}
#undef desc
STATIC int
mon_desc(midx)
{
  struct monS* mon = &entity_monD[midx];
  struct creatureS* cre = &creatureD[mon->cidx];
  int lit = mon_lit(midx);

  if (lit)
    snprintf(descD, AL(descD), "The %s", cre->name);
  else
    strcpy(descD, "It");

  death_creD = mon->cidx;
  return lit;
}
STATIC void
death_desc(char* special)
{
  death_creD = 0;
  strcpy(death_descD, special);
}
STATIC void
death_obj(struct objS* obj)
{
  struct treasureS* tr_ptr = &treasureD[obj->tidx];
  char* ptr = "";
  if (tr_ptr->name) ptr = tr_ptr->name;
  death_desc(ptr);
}
STATIC char*
death_text()
{
  if (death_creD) {
    struct creatureS* cre = &creatureD[death_creD];
    if (is_a_vowel(cre->name[0]))
      snprintf(death_descD, AL(death_descD), "an %s", cre->name);
    else
      snprintf(death_descD, AL(death_descD), "a %s", cre->name);
  }
  return death_descD;
}
STATIC int
summon_object(y, x, num, typ)
{
  int i, j, k;
  int py, px;
  struct caveS* c_ptr;
  int real_typ;
  int seen = 0;

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
          if (CF_VIZ & caveD[j][k].cflag) seen += real_typ;

          i = 20;
        }
      }
      i++;
    } while (i <= 20);
    num--;
  } while (num != 0);

  return seen;
}
STATIC void
memory_of_kill(cidx, ogcount)
{
  ST_INC(recallD[cidx].r_kill);

  if (ogcount) {
    int obj = ogcount % 256;
    int gc = ogcount / 256;

    uint32_t rcflag = 0;
    if (obj) rcflag |= (CM_CARRY_OBJ | CM_SMALL_OBJ);
    if (gc) rcflag |= CM_CARRY_GOLD;

    ST_MAX(recallD[cidx].r_treasure, obj + gc);
    if (rcflag) recallD[cidx].r_cmove |= rcflag;
  }
}
STATIC int
mon_death(y, x, flags)
{
  int i = 0;
  if (flags & CM_CARRY_OBJ) i += 1;
  if (flags & CM_CARRY_GOLD) i += 2;
  if (flags & CM_SMALL_OBJ) i += 4;

  int number = 0;
  if ((flags & CM_60_RANDOM) && (randint(100) < 60)) number++;
  if ((flags & CM_90_RANDOM) && (randint(100) < 90)) number++;
  if (flags & CM_1D2_OBJ) number += randint(2);
  if (flags & CM_2D2_OBJ) number += damroll(2, 2);
  if (flags & CM_4D2_OBJ) number += damroll(4, 2);

  int ogcount = 0;
  if (number > 0) ogcount = summon_object(y, x, number, i);
  return ogcount;
}
STATIC int
mon_take_hit(midx, dam)
{
  struct monS* mon = &entity_monD[midx];
  struct creatureS* cre = &creatureD[mon->cidx];
  int death_blow;

  mon->msleep = 0;
  mon->hp -= MAX(dam, 1);
  death_blow = mon->hp < 0;

  if (death_blow) {
    int ulev = uD.lev;
    div_t dxp = div(cre->mexp * cre->level, ulev);
    int frac = (dxp.rem * 32) / ulev + uD.exp_frac;

    uD.exp_frac = (frac % 32);
    uD.exp += dxp.quot + (frac >= 32);

    caveD[mon->fy][mon->fx].midx = 0;
    int ogcount = mon_death(mon->fy, mon->fx, cre->cmove);
    memory_of_kill(mon->cidx, ogcount);
    mon_unuse(mon);

    if (cre->cmove & CM_WIN) {
      // Player may be dead; check for level transition
      uD.total_winner = (uD.new_level_flag == 0);
    }
  }

  return death_blow;
}
STATIC void
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
  if (py_tr(TR_REGEN)) hitpoints += PLAYER_REGEN_HPBONUS;
  if (HACK) hitpoints += 1000;

  // Scale current hp to the new maximum
  int value = ((uD.chp << 16) + uD.chp_frac) / uD.mhp * hitpoints;
  uD.chp = value >> 16;
  uD.chp_frac = value & 0xFFFF;
  uD.mhp = hitpoints;
}
STATIC void
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
STATIC void
calc_bonuses()
{
  int tflag;
  int ac, toac;
  int tohit, todam;
  int hide_tohit, hide_todam, hide_toac;

  tohit = tohit_adj();
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

  int calc_exp = (tflag ^ cbD.tflag) == TR_EZXP;
  cbD.tflag = tflag;

  if (calc_exp) expmult_update();
}
// uD.mflags && maD are NOT up-to-date at the time of this call
STATIC void
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
STATIC void
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
STATIC void
ma_duration(maidx, nturn)
{
  add_ma_count(maidx, nturn * 2);
}
// Combat affects result in an odd tick count
// Such that the player recovers from an affect before their turn
STATIC void
ma_combat(maidx, nturn)
{
  int tick_count;
  tick_count = maD[maidx];
  add_ma_count(maidx, nturn * 2 + (tick_count % 2 == 0));
}
STATIC int
ma_clear(maidx)
{
  int turn;
  turn = maD[maidx];
  maD[maidx] = 0;
  return turn != 0;
}
STATIC int8_t
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
STATIC void
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
STATIC void py_bonuses(obj, factor) struct objS* obj;
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
STATIC int
equip_swap_into(iidx, into_slot)
{
  struct objS* obj;
  obj = obj_get(invenD[iidx]);
  if (obj->flags & TR_CURSED) {
    MSG("The item you are %s is cursed, you can't take it off.",
        describe_use(iidx));
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
STATIC void
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
        obj_detail(obj, 0);
        MSG("You drop %s%s.", descD, detailD);
        turn_flag = TRUE;
      }
    } else {
      msg_print("There are too many objects on the ground here.");
    }
  }
}
STATIC int
player_saves()
{
  return (randint(100) <= usave());
}
STATIC int
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
STATIC int
equip_random()
{
  int slot[] = {
      INVEN_BODY, INVEN_ARM, INVEN_OUTER, INVEN_HANDS, INVEN_HEAD, INVEN_FEET,
  };
  int k;

  k = slot[randint(AL(slot) - 1)];
  return invenD[k] ? k : -1;
}
STATIC int
equip_enchant(iidx, amount)
{
  struct objS* i_ptr;
  int affect;

  if (iidx >= 0) {
    i_ptr = obj_get(invenD[iidx]);
    if (may_enchant_ac(i_ptr->tval)) {
      obj_desc(i_ptr, 1);
      descD[0] &= ~0x20;
      affect = 0;
      for (int it = 0; it < amount; ++it) {
        affect += (enchant(&i_ptr->toac, 10));
      }

      if (affect) {
        MSG("%s glows %s!", descD, affect > 1 ? "brightly" : "faintly");
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
STATIC int
equip_curse()
{
  struct objS* i_ptr;
  int l;

  l = equip_random();
  if (l >= 0) {
    i_ptr = obj_get(invenD[l]);
    obj_desc(i_ptr, 1);
    descD[0] &= ~0x20;
    MSG("%s glows black, fades.", descD);
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
STATIC int
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
STATIC int
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
STATIC int
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
STATIC int
inven_slot()
{
  for (int it = 0; it < INVEN_EQUIP; ++it) {
    if (!invenD[it]) return it;
  }
  return -1;
}
STATIC void inven_used_obj(obj) struct objS* obj;
{
  obj->number -= 1;
  if (obj->number < 1) {
    for (int it = 0; it < INVEN_EQUIP; ++it) {
      if (invenD[it] == obj->id) invenD[it] = 0;
    }
    obj_unuse(obj);
  }
}
STATIC int
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
STATIC int
inven_food()
{
  for (int it = 0; it < INVEN_EQUIP; ++it) {
    struct objS* obj = obj_get(invenD[it]);
    if (obj->tval == TV_FOOD) return it;
  }
  return -1;
}
STATIC int
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
    descD[0] &= ~0x20;
    obj_detail(obj, 0);
    if (iidx >= INVEN_EQUIP) {
      calc_bonuses();
    }
    MSG("%s%s.", descD, detailD);
  }
  return used;
}
STATIC int
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
      descD[0] &= ~0x20;
      MSG("%s glows %s!", descD, affect > 1 ? "brightly" : "faintly");
      i_ptr->flags &= ~TR_CURSED;
      calc_bonuses();
    } else
      msg_print("The enchantment fails.");
  }

  return limit != 0;
}
STATIC int
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
        descD[0] &= ~0x20;
        MSG("%s glows brightly.", descD);

        return TRUE;
      }
    }
  }

  return FALSE;
}
STATIC int
rest_affect()
{
  return py_affect(MA_BLIND) + countD.confusion + py_affect(MA_FEAR) +
         py_affect(MA_RECALL);
}
STATIC void
py_rest()
{
  if (uD.chp < uD.mhp || rest_affect())
    countD.rest = 9999;
  else
    countD.rest = -9999;
  turn_flag = TRUE;
}
STATIC int
py_take_hit(damage)
{
  int death_blow;

  death_blow = (uD.chp - damage < 0);
  uD.chp -= damage;

  if (death_blow) uD.new_level_flag = NL_DEATH;
  return death_blow;
}
STATIC void py_stats(stats, len) int8_t* stats;
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
STATIC int
heroname_init()
{
  int i, j, k;
  descD[0] = 0;
  k = randint(2) + 1;
  for (i = 0; i < k; i++) {
    for (j = 1 + randint(2); j > 0; j--)
      strcat(descD, gutteralD[randint(AL(gutteralD)) - 1]);
    if (i < k - 1) strcat(descD, " ");
  }
  descD[0] &= ~0x20;
  for (i = 1; i < 12; ++i) {
    if (descD[i] == ' ') descD[i + 1] &= ~0x20;
  }
  for (i = 12; i < AL(heronameD) - 1; ++i)
    if (descD[i] == ' ') break;
  descD[i] = 0;
  strcpy(heronameD, descD);
}
STATIC int
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
STATIC int
value_by_stat(stat)
{
  return 5 * (stat - 10);
}
STATIC void
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
STATIC int
clamp(val, min, max)
{
  if (val < min) return min;
  if (val > max) return max;
  return val;
}
STATIC void
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

  fixed_seed_func(town_seed, heroname_init);

  int sc = fixed_seed_func(town_seed, social_bonus);
  uD.sc = clamp(randint(4) + sc, 1, 100);

  py_gold_init();
  calc_bonuses();
}
STATIC void
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
STATIC void sort(array, len) void* array;
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
STATIC int
magic_init()
{
  int i, j, k, t;
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
  memset(titleD, 0, sizeof(titleD));
  for (t = 0; t < AL(titleD); t++) {
    int64_t len = 0;
    apcat(AP(titleD[t]), "\"");
    for (i = randint(2) + 1; i > 0; i--) {
      for (j = randint(2); j > 0; j--) {
        int s = randint(AL(syllableD)) - 1;
        int l = apcat(AP(titleD[t]), syllableD[s]);
        if (l < MAX_TITLE - 1) len = l;
      }
      if (i > 1) apcat(AP(titleD[t]), " ");
    }
    titleD[t][len] = 0;
    apcat(AP(titleD[t]), "\"");
  }
  return 0;
}
STATIC int
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
STATIC int
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
STATIC void
lose_stat(sidx)
{
  uint32_t sustain = sustain_stat(sidx);
  if (py_tr(sustain)) {
    MSG("%s for a moment, it passes.", stat_lossD[sidx]);
  } else {
    dec_stat(sidx);
    MSG("%s.", stat_lossD[sidx]);
  }
}
STATIC int
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
STATIC int
low_stat(uint32_t bf)
{
  for (int it = 0; it < MAX_A; ++it) {
    int check = ((1 << it) & bf) != 0;

    if (check && statD.max_stat[it] != statD.cur_stat[it]) return 1;
  }
  return 0;
}
STATIC void
py_where_on_map()
{
  int restore_zoom;
  int dir;

  restore_zoom = globalD.zoom_factor;
  globalD.zoom_factor = 0;
  while (get_dir("Where on Map | Scroll which direction?", &dir)) {
    mmove(dir, &panelD.panel_row, &panelD.panel_col);
    if (panelD.panel_row > MAX_ROW - 2) panelD.panel_row = MAX_ROW - 2;
    if (panelD.panel_col > MAX_COL - 2) panelD.panel_col = MAX_COL - 2;
    panel_bounds(&panelD);
  }
  panel_update(&panelD, uD.y, uD.x, TRUE);
  globalD.zoom_factor = restore_zoom;
}
STATIC void
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
STATIC int
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
STATIC int
py_expmult()
{
  int mult_exp = uD.mult_exp;
  if (py_tr(TR_EZXP)) mult_exp -= (mult_exp - 100) / 2;
  return mult_exp;
}
STATIC int
lev_exp(lev)
{
  return player_exp[lev - 1] * py_expmult() / 100;
}
STATIC int
expmult_update()
{
  int exp = uD.exp;
  int lev = 1;
  while (lev_exp(lev) <= exp) lev++;

  int change_lev = (lev != uD.lev);

  if (change_lev) {
    MSG("Welcome to level %d.", lev);
    uD.lev = lev;
    calc_hitpoints();
    calc_mana();
  }
  return change_lev;
}
STATIC void
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
STATIC int
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
STATIC int
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
STATIC void
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
  py_check_view();
}
// TBD: We may loop infinitely with the added restriction of oidx != 0
// Phase door (short range) teleport runs a higher risk
STATIC void py_teleport(dis, uy, ux) int *uy, *ux;
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
STATIC void
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
}
STATIC int
is_obj_vulnmelee(obj, vulnmelee)
struct objS* obj;
{
  switch (vulnmelee) {
    case GF_LIGHTNING:
      return vuln_lightning(obj);
    case GF_POISON_GAS:
      return vuln_gas(obj);
    case GF_ACID:
      return vuln_acid(obj);
    case GF_FROST:
      return vuln_frost(obj);
    case GF_FIRE:
      return vuln_fire(obj);
  }
  return 0;
}
STATIC int
is_obj_vulntype(obj, vulntyp)
struct objS* obj;
{
  switch (vulntyp) {
    case GF_LIGHTNING:
      return vuln_lightning(obj);
    case GF_ACID:
      // Distinct from vuln_acid (acid_dam attack)
      return vuln_acid_breath(obj);
    case GF_FROST:
      return vuln_frost(obj);
    case GF_FIRE:
      // Distinct from vuln_fire (fire_dam attack)
      return vuln_fire_breath(obj);
  }
  return 0;
}
STATIC void
get_flags(int typ, uint32_t* weapon_type, int* harm_type)
{
  switch (typ) {
    default:
    case GF_MAGIC_MISSILE:
      *weapon_type = 0;
      *harm_type = 0;
      break;
    case GF_LIGHTNING:
      *weapon_type = CS_BR_LIGHT;
      *harm_type = CD_LIGHT;
      break;
    case GF_POISON_GAS:
      *weapon_type = CS_BR_GAS;
      *harm_type = CD_POISON;
      break;
    case GF_ACID:
      *weapon_type = CS_BR_ACID;
      *harm_type = CD_ACID;
      break;
    case GF_FROST:
      *weapon_type = CS_BR_FROST;
      *harm_type = CD_FROST;
      break;
    case GF_FIRE:
      *weapon_type = CS_BR_FIRE;
      *harm_type = CD_FIRE;
      break;
    case GF_HOLY_ORB:
      *weapon_type = 0;
      *harm_type = CD_EVIL;
      break;
  }
}
STATIC void
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
        if ((c_ptr->cflag & CF_ROOM) != 0) light_room(y, x);
      }
      c_ptr->cflag |= (CF_PERM_LIGHT | CF_SEEN);

      if (c_ptr->midx) {
        m_ptr = &entity_monD[c_ptr->midx];
        cr_ptr = &creatureD[m_ptr->cidx];
        mon_desc(c_ptr->midx);
        if (CD_LIGHT & cr_ptr->cdefense) {
          recallD[m_ptr->cidx].r_cdefense |= CD_LIGHT;
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
STATIC void magic_bolt(typ, dir, y, x, dam, bolt_typ) char* bolt_typ;
{
  int dist, flag;
  uint32_t weapon_type;
  int harm_type;
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cre;

  flag = FALSE;
  get_flags(typ, &weapon_type, &harm_type);
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
        int mod_light = ((c_ptr->cflag & CF_TEMP_LIGHT) == 0);
        if (mod_light) c_ptr->cflag ^= CF_TEMP_LIGHT;

        mon_desc(c_ptr->midx);
        descD[0] = descD[0] | 0x20;
        MSG("The %s strikes %s.", bolt_typ, descD);

        if (!replay_playback()) {
          viz_hookD = viz_magick;
          magick_distD = 0;
          magick_locD = (point_t){x, y};
          magick_hituD = 0;
        }

        // draw with temporary visibility
        msg_pause();
        if (mod_light) c_ptr->cflag ^= CF_TEMP_LIGHT;

        if (harm_type & cre->cdefense) {
          recallD[m_ptr->cidx].r_cdefense |= harm_type;
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
STATIC void fire_ball(typ, dir, y, x, dam_hp, descrip) char* descrip;
{
  int i, j;
  int dam, thit, tkill;
  int oldy, oldx, dist, flag, harm_type;
  uint32_t weapon_type;
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;

  thit = 0;
  tkill = 0;
  get_flags(typ, &weapon_type, &harm_type);
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

        if (!replay_playback()) {
          viz_hookD = viz_magick;
          magick_distD = FIRE_DIST;
          magick_locD = (point_t){x, y};
          magick_hituD = 0;
        }

        /* The ball hits and explodes.  	     */
        /* The explosion.  		     */
        for (i = y - FIRE_DIST; i <= y + FIRE_DIST; i++)
          for (j = x - FIRE_DIST; j <= x + FIRE_DIST; j++)
            if (in_bounds(i, j) && (distance(y, x, i, j) <= FIRE_DIST) &&
                los(y, x, i, j)) {
              c_ptr = &caveD[i][j];
              if ((c_ptr->oidx) &&
                  is_obj_vulntype(&entity_objD[c_ptr->oidx], typ)) {
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
                    recallD[m_ptr->cidx].r_cdefense |= harm_type;
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
STATIC int
twall(y, x)
{
  int i, j;
  struct caveS* c_ptr;
  int res, room;

  c_ptr = &caveD[y][x];
  res = FALSE;
  room = FALSE;
  if (c_ptr->cflag & CF_ROOM) {
    // room: LIGHT_FLOOR or DARK_FLOOR
    for (i = y - 1; i <= y + 1; i++)
      for (j = x - 1; j <= x + 1; j++)
        if (caveD[i][j].fval < FLOOR_THRESHOLD) {
          c_ptr->fval = caveD[i][j].fval;
          c_ptr->cflag |= (CF_PERM_LIGHT & caveD[i][j].cflag);
          room = TRUE;
          break;
        }
  }

  if (!room) c_ptr->fval = FLOOR_CORR;

  if (CF_LIT & c_ptr->cflag && c_ptr->oidx) msg_print("You find something!");
  res = TRUE;

  return (res);
}
STATIC int
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
        recallD[m_ptr->cidx].r_cdefense |= CD_STONE;
      }
    }
  } while (!flag);

  if (known) MSG("The %s turns into mud.", known);

  return known != 0;
}
STATIC int
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
        if (mon_lit(midx)) poly = TRUE;
      } else {
        mon_desc(c_ptr->midx);
        MSG("%s is unaffected.", descD);
      }
    }
  } while (!flag);
  return (poly);
}
STATIC int
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
STATIC int
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
      int mlit = mon_desc(c_ptr->midx);
      flag = TRUE;
      /* Monsters with innate resistence ignore the attack.
         Monsters which resisted the attack should wake up.  */
      if (CD_NO_SLEEP & cr_ptr->cdefense) {
        if (mlit) recallD[m_ptr->cidx].r_cdefense |= CD_NO_SLEEP;
        MSG("%s is unaffected.", descD);
      } else if (randint(MAX_MON_LEVEL) < cr_ptr->level) {
        MSG("%s sounds disoriented, only for a moment.", descD);
      } else {
        if (m_ptr->mconfused)
          m_ptr->mconfused += 3;
        else
          m_ptr->mconfused = 2 + randint(16);
        confuse = TRUE;
        MSG("%s sounds confused.", descD);
      }
      m_ptr->msleep = 0;
    }
  } while (!flag);
  return (confuse);
}
STATIC int
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

      int sflag = CD_NO_SLEEP & cr_ptr->cdefense;
      sleep = ((sflag) == 0) && (randint(MAX_MON_LEVEL) >= cr_ptr->level);

      if (sleep) m_ptr->msleep = 500;

      int mlit = mon_desc(c_ptr->midx);
      if (mlit) {
        if (sleep) {
          seen += 1;
          MSG("%s falls asleep.", descD);
        } else {
          if (sflag) recallD[m_ptr->cidx].r_cdefense |= sflag;
          MSG("%s is unaffected.", descD);
        }
      }
    }
  } while (!flag);
  return seen != 0;
}
STATIC int
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

    int sflag = CD_NO_SLEEP & cr_ptr->cdefense;
    sleep = ((sflag) == 0) && (randint(MAX_MON_LEVEL) >= cr_ptr->level);

    if (sleep) mon->msleep = 500;

    if (mon_lit(it_index)) {
      mon_desc(it_index);
      if (sleep) {
        seen += 1;
        MSG("%s falls asleep.", descD);
      } else {
        if (sflag) recallD[mon->cidx].r_cdefense |= sflag;
        MSG("%s is unaffected.", descD);
      }
    }
  });
  return seen != 0;
}
STATIC int
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
STATIC int
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
        recallD[m_ptr->cidx].r_cdefense |= CD_UNDEAD;
      }
    }
  } while (!flag);
  return (drain);
}
STATIC int
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
STATIC int
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
          if (mon_lit(c_ptr->midx)) MSG("%s grows larger.", descD);
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
STATIC int
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
STATIC int
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
STATIC int
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
STATIC int
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
      mon->msilenced = TRUE;
      mon_desc(it_index);
      if (mon_take_hit(it_index, randint(damage))) {
        MSG("%s dissolves!", descD);
        py_experience();
      } else {
        MSG("%s shudders.", descD);
      }
      recallD[mon->cidx].r_cdefense |= cflag;
    }
  });
  msg_pause();
  return (dispel);
}
STATIC int
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
STATIC int
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
STATIC int
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
STATIC int
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
        int sflag = CD_NO_SLEEP & cr_ptr->cdefense;
        if ((randint(MAX_MON_LEVEL) < cr_ptr->level) || (sflag) != 0) {
          if (sflag) recallD[m_ptr->cidx].r_cdefense |= sflag;
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
STATIC int
swap_2i(int* left, int* right)
{
  int val = *left ^ *right;
  *left ^= val;
  *right ^= val;
}
STATIC int
create_food(y, x, known)
{
  int dir_list[9];
  for (int it = 0; it < AL(dir_list); ++it) {
    dir_list[it] = it + 1;
  }
  for (int it = 0; it < AL(dir_list); ++it) {
    swap_2i(&dir_list[it], &dir_list[randint(AL(dir_list)) - 1]);
  }

  for (int it = 0; it < AL(dir_list); ++it) {
    int dir = dir_list[it];
    int i = dir_y(dir);
    int j = dir_x(dir);
    struct caveS* c_ptr = &caveD[y + i][x + j];
    if (c_ptr->fval <= MAX_OPEN_SPACE && c_ptr->oidx == 0) {
      struct objS* obj = obj_use();
      if (obj->id) {
        c_ptr->oidx = obj_index(obj);
        tr_obj_copy(OBJ_MUSH_TIDX, obj);
        obj->fy = y;
        obj->fx = x;
        msg_print("Food rolls on the ground.");
        return 1;
      }
    }
  }

  if (known) msg_print("There are too many objects nearby.");

  return 0;
}
STATIC int
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

      int mlit = mon_desc(it_index);
      if (mlit) {
        if (success) {
          MSG("%s runs frantically!", descD);
          recallD[mon->cidx].r_cdefense |= CD_UNDEAD;
        } else {
          MSG("%s is unaffected.", descD);
        }
      }
    }
  });
  return (turn_und);
}
STATIC void
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
STATIC int
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
STATIC int
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
STATIC int
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
  if (count) msg_print("You hear a stirring in the distance!");
  return (count > 0);
}
STATIC int
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
STATIC int
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
      int mlit = mon_lit(it_index);
      mon_desc(it_index);
      cr_ptr = &creatureD[mon->cidx];
      if (spd > 0) {
        mon->mspeed += spd;
        if (mlit) {
          see_count += 1;
          MSG("%s starts moving faster.", descD);
        }
      } else if (randint(MAX_MON_LEVEL) > cr_ptr->level) {
        mon->mspeed += spd;
        if (mlit) {
          MSG("%s starts moving slower.", descD);
          see_count += 1;
        }
      } else if (mlit) {
        MSG("%s resists the affects.", descD);
      }
    }
  });
  return (see_count);
}
// TBD: very similar bolt()
STATIC int
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
STATIC void
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
STATIC void
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
            if (mon_lit(c_ptr->midx)) MSG("%s grows larger.", descD);
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
STATIC void
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
STATIC void
starlite(y, x)
{
  see_print("The end of the staff bursts into a blue shimmering light.");
  for (int it = 1; it <= 9; it++)
    if (it != 5) light_line(it, y, x);
}
STATIC int
invenobj_cmp(j, i)
{
  int aid, bid, astack, bstack, ar, br, at, bt, ak, bk, known;
  struct objS* a = obj_get(invenD[j]);
  struct objS* b = obj_get(invenD[i]);

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
STATIC int
inven_sort()
{
  for (int i = 0; i < INVEN_EQUIP; ++i) {
    for (int j = i + 1; j < INVEN_EQUIP; ++j) {
      int swap = invenD[j] ^ invenD[i];
      if (swap && invenobj_cmp(j, i) > 0) {
        invenD[j] ^= swap;
        invenD[i] ^= swap;
      }
    }
  }
  return 1;
}
STATIC int
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
STATIC int
inven_overlay(begin, end, show_weight)
{
  USE(overlay_width);
  int line, count;
  int limitw = MIN(overlay_width, 80);
  int descw = 4;

  apspace(AB(overlayD));
  line = count = 0;
  overlay_submodeD = begin == 0 ? 'i' : 'e';
  for (int it = begin; it < end; ++it) {
    int len = 1;
    int obj_id = invenD[it];

    if (obj_id) {
      overlayD[line][0] = '(';
      overlayD[line][1] = 'a' + it - begin;
      overlayD[line][2] = ')';
      overlayD[line][3] = ' ';

      struct objS* obj = obj_get(obj_id);
      obj_desc(obj, obj->number);
      int dlen = obj_detail(obj, show_weight ? FMT_WEIGHT : 0);

      memcpy(overlayD[line] + descw, AP(descD));
      memcpy(overlayD[line] + limitw - dlen - 1, detailD, dlen);

      len = limitw;
      count += 1;
    }

    overlay_usedD[line] = len;
    line += 1;
  }
  return count;
}
STATIC int
weapon_curse()
{
  struct objS* i_ptr = obj_get(invenD[INVEN_WIELD]);
  if (i_ptr->tval != TV_NOTHING) {
    obj_desc(i_ptr, 1);
    descD[0] &= ~0x20;
    MSG("%s pulses black.", descD);
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
STATIC int
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
          ident |= res_stat(A_STR);
          break;
        case 17:
          ident |= res_stat(A_CON);
          break;
        case 18:
          ident |= res_stat(A_INT);
          break;
        case 19:
          ident |= res_stat(A_WIS);
          break;
        case 20:
          ident |= res_stat(A_DEX);
          break;
        case 21:
          ident |= res_stat(A_CHR);
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
          death_desc("poisonous food");
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
STATIC int
obj_tabil(obj, truth)
struct objS* obj;
{
  int tabil;
  tabil = -1;
  int tunnel = (obj->tval == TV_DIGGING);
  if (obj->idflag & ID_REVEAL) truth = 1;
  if (tunnel || may_equip(obj->tval) == INVEN_WIELD) {
    tabil = HACK ? 9000 : 0;
    int mod = tunnel ? obj->p1 * 50 : (obj->tohit + obj->todam) / 2;
    if (truth) tabil += mod;

    int base = tunnel ? 25 : obj->damage[0] * obj->damage[1];
    tabil += base;
  }
  return tabil;
}
STATIC int
obj_tbonus(obj)
struct objS* obj;
{
  int tb = statD.use_stat[A_STR] / 4;
  if (obj->tval != TV_DIGGING) tb /= 4;
  return tb;
}
STATIC int
strip_tail(line)
{
  char* text = screenD[line];
  int end = screen_usedD[line];
  int it;
  for (it = end; it > 0; --it) {
    if (text[it] == ' ') break;
  }
  screen_usedD[line] = it;
  return 0;
}
STATIC int
screen_scry_obj(sflag, obj)
struct objS* obj;
{
  int line = 0;
  for (; line < AL(screenD); ++line) {
    if (!sflag) break;
    uint32_t flag = sflag & -sflag;
    int used = -1;
    switch (flag) {
      case SCRY_WEIGHT:
        used = snprintf(AP(screenD[line]), "Weight %d.%01d Lbs (per)",
                        obj->weight / 10, obj->weight % 10);
        break;
      case SCRY_STACK: {
        int stacklimit = stacklimit_by_max_weight(ustackweight(), obj->weight);
        used = snprintf(AP(screenD[line]), "Stacking %d/%d", obj->number,
                        stacklimit);
      } break;
      case SCRY_HIT:
        used =
            snprintf(AP(screenD[line]), "Modifies Hit Chance %+d", obj->tohit);
        break;
      case SCRY_DAM:
        used = snprintf(AP(screenD[line]), "Modifies Damage %+d", obj->todam);
        break;
      case SCRY_AC:
        used = snprintf(AP(screenD[line]), "Modifies Armor %+d [%d%+d]",
                        obj->toac + obj->ac, obj->ac, obj->toac);
        break;
      case SCRY_PLAUNCHER:
        used = snprintf(AP(screenD[line]), "%s uses %s",
                        launcher_nameD[obj->p1], projectile_nameD[obj->p1]);
        break;
      case SCRY_WEAPON: {
        int lo = obj->damage[0];
        int hi = obj->damage[0] * obj->damage[1];
        int td = obj->todam;
        used = snprintf(AP(screenD[line]), "Damage per Blow [ %d - %d ] %+d",
                        lo, hi, td);
      } break;
      case SCRY_BLOWS: {
        int bl = attack_blows(obj->weight);
        int lo = obj->damage[0];
        int hi = obj->damage[0] * obj->damage[1];
        int td = obj->todam;
        lo *= bl;
        hi *= bl;
        td *= bl;
        used = snprintf(AP(screenD[line]), "x%d Blows [ %d - %d ] %+d", bl, lo,
                        hi, td);
      } break;
      case SCRY_DIGGING:
        used = snprintf(AP(screenD[line]), "Modifies digging %+d",
                        obj_tabil(obj, tr_is_known(&treasureD[obj->tidx])));
        break;
      case SCRY_ZAP:
        if (tr_is_known(&treasureD[obj->tidx])) {
          int diff = udevice() - obj->level - ((obj->tval == TV_STAFF) * 5);
          used = snprintf(AP(screenD[line]), "Device Level %d (%+d Chance)",
                          obj->level, diff);
        }
        break;
      case SCRY_HEAVY:
        used = snprintf(AP(screenD[line]), "Too heavy for proper use");
        break;
    }
    if (used < 1) break;
    screen_usedD[line] = used;
    sflag ^= flag;
  }
  return line;
}
STATIC int
obj_study(obj, town_store)
struct objS* obj;
{
  if (HACK) obj->idflag = ID_REVEAL;

  int reveal = obj->idflag & ID_REVEAL;
  int known = obj->tidx && tr_is_known(&treasureD[obj->tidx]);

  apspace(AB(screenD));
  int sflag = SCRY_WEIGHT;
  if (!town_store && (STACK_ANY & obj->subval)) sflag |= SCRY_STACK;
  if (obj->tval == TV_LAUNCHER || obj->tval == TV_PROJECTILE)
    sflag |= SCRY_PLAUNCHER;
  if (oset_melee(obj)) sflag |= (SCRY_WEAPON | SCRY_BLOWS);
  if (oset_tohitdam(obj) && obj->tohit) sflag |= SCRY_HIT;
  if (oset_tohitdam(obj) && obj->todam) sflag |= SCRY_DAM;
  if (obj->tval == TV_DIGGING) sflag |= SCRY_DIGGING;
  if (obj->tval == TV_WAND || obj->tval == TV_STAFF) sflag |= SCRY_ZAP;
  if (obj->ac || obj->toac) sflag |= SCRY_AC;
  if (oset_weightcheck(obj) && tohit_by_weight(obj->weight))
    sflag |= SCRY_HEAVY;

  int line = screen_scry_obj(sflag, obj);

  if (oset_equip(obj) && reveal) {
    int i = obj->flags;
    while (i != 0) {
      uint32_t flag = i & -i;
      i ^= flag;
      char* text = 0;
      switch (flag) {
        case TR_STR:
          text = "Modifies Strength";
          break;
        case TR_INT:
          text = "Modifies Intelligence";
          break;
        case TR_WIS:
          text = "Modifies Wisdom";
          break;
        case TR_DEX:
          text = "Modifies Dexterity";
          break;
        case TR_CON:
          text = "Modifies Constitution";
          break;
        case TR_CHR:
          text = "Modifies Charisma";
          break;
        case TR_SEARCH:
          text = "Improves Searching and Perception";
          break;
        case TR_SLOW_DIGEST:
          text = "Slows food consumption";
          break;
        case TR_STEALTH:
          text = "Improves Stealth";
          break;
        case TR_AGGRAVATE:
          text = "Aggravates monsters";
          break;
        case TR_TELEPORT:
          text = "Causes random teleportation";
          break;
        case TR_REGEN:
          text = "Speeds regeneration and increases health";
          break;
        case TR_SPEED:
          text = "Increases player speed";
          break;
        case TR_SLAY_DRAGON:
          text = "Effective against dragons (400%)";
          break;
        case TR_SLAY_ANIMAL:
          text = "Effective against animals (200%)";
          break;
        case TR_SLAY_EVIL:
          text = "Effective against evil creatures (200%)";
          break;
        case TR_SLAY_UNDEAD:
          text = "Effective against undead (300%) and Protection from XP drain";
          break;
        case TR_FROST_BRAND:
          text = "Does cold damage (150% vs fire)";
          break;
        case TR_FLAME_TONGUE:
          text = "Does fire damage (150% vs cold)";
          break;
        case TR_RES_FIRE:
          text = "Provides fire resistance (50%)";
          break;
        case TR_RES_ACID:
          text = "Provides acid resistance (50%)";
          break;
        case TR_RES_COLD:
          text = "Provides cold resistance (50%)";
          break;
        case TR_SUST_STAT:
          text = "Sustains modified stats from reduction";
          break;
        case TR_FREE_ACT:
          text = "Provides free action (immune to paralysis)";
          break;
        case TR_SEE_INVIS:
          text = "Allows seeing invisible creatures";
          break;
        case TR_RES_LIGHT:
          text = "Provides light resistance (50%)";
          break;
        case TR_FFALL:
          text = "Prevents falling through trap doors";
          break;
        case TR_SEEING:
          text = "Provides enhanced sight (immune to blind)";
          break;
        case TR_HERO:
          text = "Grants heroism (immune to fear)";
          break;
        case TR_EZXP:
          text = "Increases level gain per experience";
          break;
        case TR_SLOWNESS:
          text = "Reduces player speed";
          break;
        case TR_CURSED:
          text = "Item is cursed (cannot remove)";
          break;
      }
      BufMsg(screen, "  %s (%+d)", text, obj->p1);
      if ((flag & TR_P1) == 0) strip_tail(line - 1);
    }

    if (obj->sn == SN_LORDLINESS)
      BufFixed(screen, "  Increases success with wands & staves");
    if (obj->sn == SN_INFRAVISION) BufFixed(screen, "  Provides Infra-vision");
  }

  obj_desc(obj, obj->number);
  blipD = 1;
  screen_submodeD = 1;
  return CLOBBER_MSG("You study %s.", descD);
}
// Precondition: mode_list is at least two characters (counting nullterm)
// drop_safe (mode_len > 2) implies all modes (enabled drop_mode)
// alt_mode (mode_len > 1) toggles between inven/equip
STATIC int
inven_choice(char* prompt, char* mode_list)
{
  char c;
  char subprompt[80];
  int drop_mode = 0;

  char hint[2][80];
  int hint_used[2];
  if (INVEN_HINT) {
    int using_selection = (platformD.selection != noop);
    int mode_len = 0;
    for (char* iter = mode_list; *iter; ++iter) mode_len += 1;
    int drop_safe = (mode_len > 2);
    int alt_mode = (mode_len > 1);

    for (int it = 0; it < 2; ++it) {
      int len = 0;
      if (using_selection) {
        if (alt_mode) {
          switch (mode_list[it]) {
            case '*':
              len = snprintf(hint[it], AL(hint[it]), "(%sRIGHT: equipment)",
                             drop_safe ? "LEFT: drop toggle | " : "");
              break;
            case '/':
              len = snprintf(hint[it], AL(hint[it]), "(LEFT: inventory%s)",
                             drop_safe ? "| RIGHT: drop toggle" : "");
              break;
          }
        }
      } else {
        char* other = "";
        if (alt_mode)
          other = (mode_list[it] == '*') ? "/ equip, " : "* inven, ";
        len = snprintf(hint[it], AL(hint[it]), "(%s%s- sort, SHIFT: study)",
                       other, drop_safe ? "0 drop, " : "");
      }
      hint_used[it] = len;
    }
  }

  int mode = mode_list[0];
  while (mode) {
    msg_pause();
    int begin = 0;
    int end = INVEN_EQUIP;
    char* prefix = "Inventory";
    if (mode == '/') {
      begin = INVEN_WIELD;
      end = MAX_INVEN;
      prefix = "Equipment";
    }

    if (INVEN_HINT) {
      for (int it = 0; it < 2; ++it) {
        if (mode_list[it] == mode) {
          if (hint_used[it]) msg_hint(hint[it], hint_used[it]);
        }
      }
    }

    snprintf(subprompt, AL(subprompt), "%s: %s", prefix,
             drop_mode ? "Drop which item?" : prompt);
    inven_overlay(begin, end, drop_mode || prompt[0] == 'D');
    if (!in_subcommand(subprompt, &c)) return -1;

    if (is_lower(c)) {
      uint8_t iidx = c - 'a';
      iidx += begin;
      if (iidx < end && invenD[iidx]) {
        if (!drop_mode) return iidx;
        inven_drop(iidx);
      }
    } else if (is_upper(c)) {
      uint8_t iidx = c - 'A';
      iidx += begin;
      if (iidx < end && invenD[iidx]) {
        obj_study(obj_get(invenD[iidx]), 0);
      }
    } else if (c == '-') {
      inven_sort();
      replay_hack();
    } else {
      int next_mode = mode;
      for (char* iter = mode_list; *iter; ++iter) {
        if (*iter == c) next_mode = c;
      }
      if (next_mode == '0') {
        drop_mode = !drop_mode;
      } else if (next_mode != mode) {
        drop_mode = 0;
        mode = next_mode;
      }
    }
  }

  return -1;
}
STATIC int
inven_quaff(iidx)
{
  int ident;
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
STATIC int
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
            if (detect_mon(MA_DETECT_INVIS, known)) ident |= TRUE;
            ma_duration(MA_DETECT_INVIS, 1 + uD.lev / 5);
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
            ident |= create_food(uD.y, uD.x, known);
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
STATIC int
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
      msg_print("You fail to use the wand properly.");
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
STATIC void
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
STATIC int
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
STATIC int
spell_prompt(spidx, low_mana)
{
  int target;
  char tmp[STRLEN_MSG + 1];

  if (low_mana) msg_hint(AP("(LOW MANA)"));

  target = 0;
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
      if (!get_dir(tmp, &target)) target = -1;
    } break;
    case 21:
      target = inven_choice("Which item do you wish identified?", "*/");
      break;
    case 18:
    case 26:
      target = inven_choice("Recharge which item?", "*");
      break;
    default:
      if (!PC) {
        if (low_mana) {
          char c = CLOBBER_MSG("Are you sure you want to cast %s?",
                               spell_nameD[spidx]);
          if (c == ESCAPE || c == 'a') target = -1;
        }
      }
      break;
  }

  return target;
}
STATIC void spell_dir_target(spidx, y_ptr, x_ptr, target) int* y_ptr;
int* x_ptr;
{
  switch (spidx + 1) {
    case 1:
      magic_bolt(GF_MAGIC_MISSILE, target, uD.y, uD.x, damroll(2, 6),
                 spell_nameD[0]);
      break;
    case 2:
      detect_mon(MA_DETECT_MON, TRUE);
      ma_duration(MA_DETECT_MON, 1 + uD.lev / 5);
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
      fire_ball(GF_POISON_GAS, target, uD.y, uD.x, 12, spell_nameD[6]);
      break;
    case 8:
      confuse_monster(target, uD.y, uD.x);
      break;
    case 9:
      magic_bolt(GF_LIGHTNING, target, uD.y, uD.x, damroll(4, 8),
                 spell_nameD[8]);
      break;
    case 10:
      td_destroy(uD.y, uD.x);
      break;
    case 11:
      sleep_monster(target, uD.y, uD.x);
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
      magic_bolt(GF_FROST, target, uD.y, uD.x, damroll(6, 8), spell_nameD[14]);
      break;
    case 16:
      wall_to_mud(target, uD.y, uD.x);
      break;
    case 17:
      create_food(uD.y, uD.x, 1);
      break;
    case 18:
      inven_recharge(target, 0);
      break;
    case 19:
      sleep_monster_aoe(1);
      break;
    case 20:
      poly_monster(target, uD.y, uD.x);
      break;
    case 21:
      inven_ident(target);
      break;
    case 22:
      sleep_monster_aoe(MAX_SIGHT);
      break;
    case 23:
      magic_bolt(GF_FIRE, target, uD.y, uD.x, damroll(9, 8), spell_nameD[22]);
      break;
    case 24:
      speed_monster(target, uD.y, uD.x, -1);
      break;
    case 25:
      fire_ball(GF_FROST, target, uD.y, uD.x, 48, spell_nameD[24]);
      break;
    case 26:
      inven_recharge(target, 1);
      break;
    case 27:
      teleport_monster(target, uD.y, uD.x);
      break;
    case 28:
      ma_duration(MA_FAST, randint(20) + uD.lev);
      break;
    case 29:
      fire_ball(GF_FIRE, target, uD.y, uD.x, 72, spell_nameD[28]);
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
STATIC int
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
STATIC void py_magic(iidx, y_ptr, x_ptr) int* y_ptr;
int* x_ptr;
{
  struct objS* obj;
  uint32_t flags, first_spell;
  int cmana, sptype, spmask, spidx, target;
  struct spellS* spelltable;

  if (statD.cur_stat[A_CON] == 3) {
    msg_print("Your constitution is too weak for spell casting.");
    return;
  }

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
      } else {
        spidx = last_castD - 1;
      }

      if (spidx < 0) return;

      if ((1 << spidx) & spmask) {
        target = spell_prompt(spidx, cmana < spelltable[spidx].spmana);
        if (target < 0) return;

        // Spell Committed
        last_castD = spidx + 1;
        turn_flag = TRUE;
        if (randint(100) < spell_chanceD[spidx]) {
          msg_print("You fail to get the spell off!");
        } else {
          spell_dir_target(spidx, y_ptr, x_ptr, target);

          if ((uD.spell_worked & (1 << spidx)) == 0) {
            uD.spell_worked |= (1 << spidx);
            uD.exp += spelltable[spidx].spexp * SP_EXP_MULT;
            py_experience();
          }
        }

        if (cmana < spelltable[spidx].spmana) {
          uD.cmana = 0;
          uD.cmana_frac = 0;
          dec_stat(A_CON);
          msg_print("Your low mana damages your health!");
        } else {
          uD.cmana -= spelltable[spidx].spmana;
        }
      } else {
        turn_flag = TRUE;
        msg_print("You read the magical runes.");
        if (spelltable[spidx].splevel > uD.lev || !gain_spell(spidx)) {
          MSG("You are unable to retain the knowledge%s.",
              spelltable[spidx].splevel != 99 ? " at this time" : "");
        } else {
          MSG("You learn the spell of %s!", spell_nameD[spidx]);
        }
      }
    }
  }
}
STATIC int
prayer_prompt(pridx, low_mana)
{
  int target;
  char tmp[STRLEN_MSG + 1];

  if (low_mana) msg_hint(AP("(LOW MANA)"));

  target = 0;
  switch (pridx + 1) {
    case 9:
    case 18:
      snprintf(tmp, AL(tmp), "Which direction will you incant %s?",
               prayer_nameD[pridx]);
      if (!get_dir(tmp, &target)) target = -1;
      break;
    default:
      if (!PC) {
        if (low_mana) {
          char c = CLOBBER_MSG("Are you sure you want to incant %s?",
                               prayer_nameD[pridx]);
          if (c == ESCAPE || c == 'a') target = -1;
        }
      }
      break;
  }
  return target;
}
STATIC int
prayer_dir_target(pridx, y_ptr, x_ptr, dir)
int* y_ptr;
int* x_ptr;
{
  switch (pridx + 1) {
    case 1:
      detect_mon(MA_DETECT_EVIL, TRUE);
      ma_duration(MA_DETECT_EVIL, 1 + uD.lev / 5);
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
      create_food(uD.y, uD.x, 1);
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
STATIC void py_prayer(iidx, y_ptr, x_ptr) int* y_ptr;
int* x_ptr;
{
  struct objS* obj;
  uint32_t flags, first_spell;
  int cmana, sptype, spmask, spidx, target;
  struct spellS* spelltable;

  if (statD.cur_stat[A_CON] == 3) {
    msg_print("Your constitution is too weak for prayer.");
    return;
  }

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
      } else {
        spidx = last_castD - 1;
      }

      if (spidx < 0) return;

      if ((1 << spidx) & spmask) {
        target = prayer_prompt(spidx, cmana < spelltable[spidx].spmana);
        if (target < 0) return;

        // prayer committed
        last_castD = spidx + 1;
        turn_flag = TRUE;
        if (randint(100) < spell_chanceD[spidx]) {
          msg_print("You lost your concentration!");
        } else {
          prayer_dir_target(spidx, y_ptr, x_ptr, target);

          if ((uD.spell_worked & (1 << spidx)) == 0) {
            uD.spell_worked |= (1 << spidx);
            uD.exp += spelltable[spidx].spexp * SP_EXP_MULT;
            py_experience();
          }
        }

        if (cmana < spelltable[spidx].spmana) {
          uD.cmana = 0;
          uD.cmana_frac = 0;
          dec_stat(A_CON);
          msg_print("Your low mana damages your health!");
        } else {
          uD.cmana -= spelltable[spidx].spmana;
        }
      } else {
        MSG("You have no belief in the prayer of %s.", prayer_nameD[spidx]);
      }
    }
  }
}
STATIC int
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
      msg_print("You fail to use the staff properly.");
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
            if (detect_mon(MA_DETECT_INVIS, known)) ident |= TRUE;
            ma_duration(MA_DETECT_INVIS, 1 + uD.lev / 5);
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
            if (detect_mon(MA_DETECT_EVIL, known)) ident |= TRUE;
            ma_duration(MA_DETECT_EVIL, 1 + uD.lev / 5);
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
STATIC int
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
    msg_print("You renew the lantern's light by pouring oil from a flask.");
    turn_flag = TRUE;
    return TRUE;
  }
  return FALSE;
}
STATIC int
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
STATIC int
weight_limit()
{
  int weight_cap;

  weight_cap = statD.use_stat[A_STR] * 130 + uD.wt;
  if (weight_cap > 3000) weight_cap = 3000;
  return (weight_cap);
}
STATIC void
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
STATIC int
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
STATIC void
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
STATIC int
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
STATIC int
obj_sense(obj)
struct objS* obj;
{
  if (!oset_enchant(obj)) return FALSE;
  if (obj->idflag & (ID_REVEAL | ID_RARE | ID_MAGIK | ID_PLAIN | ID_DAMD))
    return FALSE;
  return TRUE;
}
STATIC int
obj_rare(obj)
struct objS* obj;
{
  if ((obj->flags & TR_CURSED) == 0) {
    return obj->sn != 0;
  }
  return FALSE;
}
STATIC int
obj_magik(obj)
struct objS* obj;
{
  if ((obj->flags & TR_CURSED) == 0) {
    if (obj->tohit > 0 || obj->todam > 0 || obj->toac > 0) return TRUE;
    if ((TR_P1 & obj->flags) && obj->p1 > 0) return TRUE;
    if (0x23fff880L & obj->flags) return TRUE;
  }

  return FALSE;
}
STATIC void
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
STATIC int
bow_by_projectile(pidx)
{
  struct objS *obj, *objp;
  objp = obj_get(invenD[pidx]);
  obj = obj_get(invenD[INVEN_LAUNCHER]);

  return (obj->p1 == objp->p1) ? invenD[INVEN_LAUNCHER] : 0;
}
STATIC int
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
STATIC int
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
STATIC int
mon_surprise(m_ptr, mlit)
struct monS* m_ptr;
{
  // +200 tohit is +20% crit chance
  int ret = 200;
  // Add specific limitations to the crit/hit bonus here
  if (!mlit) ret /= 2;
  return ret;
}
STATIC void
inven_throw_dir(iidx, dir)
{
  int tbth, tpth, tdam, adj, tweight, surprise;
  int wtohit, wtodam, wheavy;
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
  wheavy = 0;
  if (bowid) {
    obj = obj_get(bowid);
    obj_desc(obj, 1);
    obj_detail(obj, 0);

    wtohit += obj->tohit;
    wtodam += obj->todam;
    // str vs weight check
    wheavy = tohit_by_weight(obj->weight);

    MSG("You %s %s%s.", wheavy ? "struggle to aim" : "grasp", descD, detailD);
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

        int mlit = mon_lit(c_ptr->midx);
        adj = uD.lev * level_adj[uD.clidx][LA_BTHB];
        if (bowid) {
          tbth = uD.bowth;
        } else {
          tbth = uD.bowth * 24 / 32;
        }
        tpth = cbD.ptohit + obj->tohit + wtohit;
        if (mlit == 0) {
          if (tpth > 0) tpth /= 2;
          tbth /= 2;
          adj /= 2;
        }
        tpth += wheavy;
        tbth = tbth - cdis;

        surprise = 0;
        if (m_ptr->msleep && perceive_creature2(cr_ptr)) {
          surprise = test_hit(tbth, adj, tpth, cr_ptr->ac);
          if (surprise) surprise = mon_surprise(m_ptr, mlit);
        }

        if (surprise || test_hit(tbth, adj, tpth, cr_ptr->ac)) {
          strcpy(tname, descD);
          mon_desc(c_ptr->midx);
          descD[0] |= 0x20;
          MSG("You hear a cry as the %s strikes %s.", tname, descD);

          tdam = pdamroll(obj->damage) + obj->todam + wtodam;
          if (bowid) tdam *= obj_get(bowid)->damage[1];
          tweight = obj->weight;
          if (bowid) tweight += obj_get(bowid)->weight;

          // TBD: named projectile weapons with damage multipliers?
          // tdam = tot_dam(obj, tdam, i);
          tdam = critical_blow(tweight, tpth + surprise, adj, tdam);
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
STATIC void
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
STATIC void
py_offhand()
{
  struct objS* obj;
  int tmp, swap;
  tmp = invenD[INVEN_WIELD] ^ invenD[INVEN_AUX];
  swap = (tmp != 0);
  if (invenD[INVEN_WIELD]) {
    obj = obj_get(invenD[INVEN_WIELD]);
    if (obj->flags & TR_CURSED) {
      MSG("The item you are %s is cursed, you can't take it off.",
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
STATIC int
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
STATIC int
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
STATIC int
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
STATIC void
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
STATIC int
show_version()
{
  int line;

  screen_submodeD = 1;
  line = 0;
  BufMsg(screen, "Git Hash: %s", git_hashD);
  line += 1;
  BufMsg(screen, "License");
  BufMsg(screen, "Source from Umoria (GPLv3)");
  BufMsg(screen, "  Robert Alan Koeneke (1958-2022)");
  BufMsg(screen, "  David J. Grabiner");
  BufMsg(screen, "  James E. Wilson");
  BufMsg(screen, "Source from puff.c (Zlib)");
  BufMsg(screen, "  Mark Adler");
  BufMsg(screen, "Modified Source from SDL2 (Zlib)");
  BufMsg(screen, "  Sam Lantinga");
  BufMsg(screen, "PC builds by Cosmopolitan Toolchain (ISC)");
  BufMsg(screen, "  Justine Tunney");
  line += 1;
  BufMsg(screen, "Programming: %s", "Alan Newton");
  BufMsg(screen, "Art: %s", "Nathan Miller");

  return CLOBBER_MSG("Version %s", versionD);
}
STATIC char*
usex()
{
  if (uD.male)
    return "Male";
  else
    return "Female";
}
STATIC int
show_character()
{
  int xbth, xbowth;
  int sptype;
  int col[2] = {20, 44};
  int line = 0;

  screen_submodeD = 1;

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
  BufMsg(screen, "%s: %-6.06s", "Sex", usex());
  BufMsg(screen, "%s: %3d", "Exp Mult", py_expmult());

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

  blipD = 1;
  return CLOBBER_MSG("Name: %-16.16s", heronameD);
}

// Bounds checks on variables that may cause memory corruption
STATIC int
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
STATIC void
summary_update(struct summaryS* summary)
{
  summary->invalid = !saveslot_validation();
  summary->slevel = uD.lev;
  summary->sclass = uD.clidx;
  summary->srace = uD.ridx;
  summary->sdepth = MAX(uD.max_dlv, dun_level) * 50;
}
STATIC int
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
STATIC int
summary_saveslot_deletion(struct summaryS* summary, int saveslot, int external)
{
  char* media = external ? "External" : "Internal";
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
      c = CLOBBER_MSG("%s Media Archive: Delete level %d %s?", media,
                      summary->slevel, classD[summary->sclass].name);
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
STATIC int
saveslot_race_reroll(saveslot, race)
{
  char c = 'o';
  int using_selection = (platformD.selection != noop);
  do {
    if (KEYBOARD && c == ' ') c = 'o';
    if (c == 'o') py_race_class_seed_init(race, saveslot, platformD.seed());
    if (PC)
      using_selection ? msg_hint(AP("(LBUTTON: accept | RBUTTON: reroll)"))
                      : msg_hint(AP("(SPACEBAR: reroll, ESCAPE: accept)"));
    else
      msg_hint(AP("(LBUTTON: reroll | RBUTTON: accept)"));
    c = show_character();
    if (c == CTRL('c')) break;
  } while (c != ESCAPE);
  return 1;
}
STATIC int
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
    c = CLOBBER_MSG("Play a %s of which race?", classD[saveslot].name);
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
STATIC int
py_notes_menu()
{
  int line = 0;
  // clang-format off
BufMsg(overlay, "* Victory Screen");
BufMsg(overlay, "  Extra Feature: balrog reset (balrog spawning is otherwise disabled)");
BufMsg(overlay, "* gp suffix for shop/pawn price (previously g)");
BufMsg(overlay, "* digging tunnel rewrite");
BufMsg(overlay, "  str bonus added per turn of digging tool (up to 5/4)");
BufMsg(overlay, "  str bonus added per turn of non-digging tool (up to 5/16)");
BufMsg(overlay, "  heavy weapon penalty removed (it's hard enough to carry tools when weak)");
BufMsg(overlay, "  digging tools provide base 25; non-digging tools provide max dice roll");
BufMsg(overlay, "  base/dice requirement: 80 granite, 16 magma, 10 quartz, 0 rubble");
BufMsg(overlay, "");
BufMsg(overlay, "## Mobile features");
BufMsg(overlay, "* orientation lock option moves to Extra Features submenu");
BufMsg(overlay, "  orientation lock supports toggle at runtime");
BufMsg(overlay, "* new run/walk button with 2 icons");
BufMsg(overlay, "  some users play exclusive with one hand");
BufMsg(overlay, "  run mode is equivalent to holding one finger against the touch screen");
  // clang-format on

  CLOBBER_MSG("~2025~ Patch Q2");
  return 0;
}
STATIC int
py_saveslot_select()
{
  struct summaryS in_summary[AL(classD)];
  struct summaryS ex_summary[AL(classD)];
  int line;
  char c;
  int save_count;
  int extern_count = 0;
  uint8_t iidx;
  int has_external = (platformD.saveex != noop);
  int using_external = 0;
  int using_selection = (platformD.selection != noop);
  int testex = -1;

  // No active class
  globalD.saveslot_class = -1;

  // Clear delta visualization
  last_turnD = 0;

  save_count = py_archive_read(in_summary, 0);
  // Assist with import on re-installation
  if (save_count == 0 && has_external) {
    extern_count = py_archive_read(ex_summary, 1);
  }

  do {
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

    if (PRIVACY) {
      line = 'p' - 'a';
      BufMsg(overlay, "p) Privacy policy (open in browser)");
    }
    if (MOBILE) {
      line = 'r' - 'a';
      BufMsg(overlay, "r) Release notes");
    }

    if (has_external) {
      if (!using_external) {
        line = 's' - 'a';
        BufMsg(overlay, "s) Save all to external media");
      } else if (extern_count) {
        line = 'i' - 'a';
        BufMsg(overlay, "i) Import all");
      }

      line = 't' - 'a';
      if (using_external) {
        exportpathD[exportpath_usedD] = '/';
        exportpathD[exportpath_usedD + 1] = 0;
        if (testex == -1) {
          BufMsg(overlay, "t) Test %s", exportpathD);
        } else if (testex == 7) {
          BufMsg(overlay, "t) Test Pass");
        } else {
          BufMsg(overlay, "t) Test Failed (%d)", testex);
        }
      }

      line = 'v' - 'a';
      BufMsg(overlay, "v) View %s media", other_media);
    }

    if (PC)
      using_selection ? msg_hint(AP("(LBUTTON: play | RBUTTON: delete)"))
                      : msg_hint(AP("(SHIFT: delete character)"));
    else
      msg_hint(AP("(LBUTTON: delete | RBUTTON: play)"));

    c = CLOBBER_MSG("Play which class?");
    // Privacy policy
    if (c == 'p') {
      if (PRIVACY) SDL_OpenURL("https://rufe.org/moria/privacy.html");
      continue;
    }
    if (c == 'r') {
      py_notes_menu();
      continue;
    }
    // Deletion
    if (c == ESCAPE) {
      int srow, scol;
      if (using_selection) {
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
        int count = platformD.saveex(1);
        line = 0;
        BufMsg(overlay, "Loaded characters (x%d)", count);
        CLOBBER_MSG("%s Media Update", other_media);
        // view swap
        c = 'v';
      }
    }
    // Export
    if (c == 's') {
      if (has_external) {
        int count = platformD.saveex(0);
        line = 0;
        BufMsg(overlay, "Saved characters (x%d)", count);
        CLOBBER_MSG("%s Media Update", other_media);
        // view swap
        extern_count = 0;
        c = 'v';
      }
    }
    if (c == 't') {
      if (using_external) {
        testex = platformD.testex();
      }
    }
    if (c == 'v') {
      testex = -1;
      using_external = !using_external;
      if (using_external && extern_count == 0)
        extern_count = py_archive_read(ex_summary, 1);
    }

    if (is_lower(c)) {
      iidx = c - 'a';
      if (iidx < AL(classD)) {
        if (summary[iidx].invalid) continue;

        if (summary[iidx].slevel == 0) {
          if (!saveslot_creation(iidx)) continue;

          py_inven_init();
          inven_sort();

          // save_on_ready: Initial character save
          save_on_readyD = 1;
          return 1;
        }

        if (summary[iidx].slevel) {
          int ret = platformD.load(iidx, using_external);
          // save_on_ready: Transfer to internal storage
          if (using_external) save_on_readyD = 1;
          return ret;
        }
      }
    } else if (PC && is_upper(c))
    // PC Deletion
    {
      iidx = c - 'A';
      if (!summary[iidx].invalid) {
        if (summary_saveslot_deletion(&summary[iidx], iidx, using_external))
          extern_count -= (using_external);
      }
    }
  } while (c != CTRL('c'));

  death_desc(quit_stringD);

  return 0;
}

STATIC int
py_king()
{
  char tmp[STRLEN_MSG + 1];
  screen_submodeD = 0;
  int xleft = 19;
  int xcenter = 19 + 12;
  int col = 0;
  USE(msg_width);
  apclear(AB(screenD));
  for (int it = 0; it < AL(screenD); ++it) screen_usedD[it] = msg_width;

  int line = 0;
  TOMB("All Hail the Might King!");
  TOMB("%s", heronameD);
  TOMB("%s %s", raceD[uD.ridx].name, classD[uD.clidx].name);

  line += 1;
  for (int it = 0; it < AL(crown); ++it) {
    if (crown[it] == '\n') {
      screen_usedD[line] = col + xleft;
      line += 1;
      col = 0;
    } else {
      screenD[line][col + xleft] = crown[it];
      col += 1;
    }
  }
  line += 1;

  TOMB("Level : %d", uD.lev);
  TOMB("Exp : %d", uD.exp);
  TOMB("Gold : %d", uD.gold);
  TOMB("Depth : %d", dun_level * 50);
  TOMB("Killed by Ripe Old Age.");

  if (PC) msg_hint(AP("(CTRL-z) (c/o/p/ESC)"));
  return CLOBBER_MSG("                       Veni, Vidi, Vici!");
}
STATIC int
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

  int xcenter = 25;
  char tmp[STRLEN_MSG + 1];
  int line = 3;
  TOMB("RIP");
  TOMB("%s", heronameD);
  line += 1;
  TOMB("%s %s", raceD[uD.ridx].name, classD[uD.clidx].name);
  TOMB("Level : %d", uD.lev);
  TOMB("Exp : %d", uD.exp);
  TOMB("Gold : %d", uD.gold);
  TOMB("Depth : %d", dun_level * 50);
  line += 1;
  TOMB("Killed by");
  TOMB("%s.", death_text());
  line += 1;
  TOMB("Undo Levels: %d", countD.pundo);
  TOMB("Deaths: %d", countD.pdeath);

  if (PC) msg_hint(AP("(CTRL-z) (c/o/p/ESC)"));
  // Centering text; portrait mode+hand_swap will show message history top left
  return CLOBBER_MSG("                You are dead, sorry!");
}
STATIC int
can_undo(offset)
{
  if (replay_flag) {
    int memory_ok = 1;
    if (offset) {
      // One command may be written ahead (e.g. py_look)
      // invalidate the replay at one less than the limit
      memory_ok = replayD->input_record_writeD < AL(replayD->input_recordD) &&
                  replayD->input_action_usedD < AL(replayD->input_actionD);
    }
    int vow_permit = 1;
    if (uvow(VOW_UNDO_LIMIT)) {
      vow_permit = replayD->input_mutationD != 0 || countD.pundo < 3;
    }
    if (uvow(VOW_DEATH) && uD.new_level_flag == NL_DEATH) vow_permit = 0;
    if (uvow(VOW_UNDO_THREAT) && find_threatD) vow_permit = 0;
    return vow_permit && memory_ok;
  }
  return 0;
}
STATIC int
can_reset(death)
{
  int ok = 1;
  if (death && uvow(VOW_DEATH)) ok = 0;
  if (uvow(VOW_UNDO_THREAT) && find_threatD) ok = 0;
  return ok;
}
STATIC void
py_undo()
{
  if (can_undo(1)) {
    replayD->input_action_usedD -= 1;
    replayD->input_mutationD = 1;
    longjmp(restartD, 1);
  }
}
STATIC void
dungeon_reset(death)
{
  if (can_reset(death)) {
    if (RESEED) {
      seed_changeD = 1;
      save_on_readyD = 1;
    }
    if (replay_flag) {
      // Disable midpoint resume explicitly
      replayD->input_action_usedD = 0;
      // Record history mutation
      replayD->input_mutationD = 1;
    }
    longjmp(restartD, 1);
  }
}
STATIC int
py_help()
{
  int line = 1;

  screen_submodeD = 1;
  BufMsg(screen, "a: actuate inventory item");
  BufMsg(screen, "c: character screen");
  BufMsg(screen, "d: drop inventory or equipment");
  BufMsg(screen, "m: map dungeon");
  BufMsg(screen, "p: message history");
  BufMsg(screen, "v: version info");
  line += 1;
  BufMsg(screen, "CTRL-c: save and exit");
  BufMsg(screen, "CTRL-z: undo");
  line += 1;
  BufMsg(screen, "ESC: Game Menu");
  BufMsg(screen, "-: observer zoom adjustment");
  BufMsg(screen, "!: repeat last spell/item");

  line += 1;
  BufMsg(screen, "SHIFT: non-combat run");
  BufMsg(screen, "hljk yubn: movement & combat");

  BufPad(screen, AL(screenD), 34);

  line = 1;
  BufMsg(screen, ".: auto-command. One of the following:");
  BufMsg(screen, "  <: up stairs");
  BufMsg(screen, "  >: down stairs");
  BufMsg(screen, "  ,: pickup object");
  BufMsg(screen, "  s: search for traps/doors");

  if (KEYBOARD) {
    line += 1;
    BufMsg(screen, "KEYBOARD EXTRAS");
    // deprecate or add mobile UI?
    // BufMsg(screen, "f: force/bash chest/door/monster");
    BufMsg(screen, "  i/e: inventory/equipment actuate");
    BufMsg(screen, "  w: weapon swap with offhand");  // handy
    BufMsg(screen, "  x: examine objects/monsters");  // show look frame?
    BufMsg(screen, "  M: Map scan mode");             // useful?
    BufMsg(screen, "  R: Rest until healed");         // handy
    line += 1;
    line += 1;
    BufMsg(screen, "NUMPAD (Numlock OFF)");
    BufMsg(screen, "  2468 1379: movement & combat");
    BufMsg(screen, "  0: alias to minimap");
    BufMsg(screen, "  .: alias to auto-command");
    BufMsg(screen, "  -: alias to zoom");
  }

  return CLOBBER_MSG("keyboard help");
}
STATIC int
py_menu()
{
  char c = 0;
  char* prompt = "Advanced Game Actions";
  int death = (uD.new_level_flag == NL_DEATH);

  if (death) prompt = "You are dead.";

  while (1) {
    overlay_submodeD = 0;
    int line = 0;
    if (death) {
      char* msg = uvow(VOW_DEATH) ? "A vow of death is permanent"
                                  : "Ahh, death comes to us all";
      BufMsg(overlay, "a) %s", msg);
    } else {
      BufMsg(overlay, "a) Await event (health, malady, or recall)");
    }

    BufMsg(overlay, "b) Back / Gameplay Rewind: ");
    if (can_undo(1)) {
      if (uvow(VOW_UNDO_LIMIT)) {
        if (replayD->input_mutationD != 0) {
          BufLineAppend(overlay, line - 1, "Undo Active");
        } else {
          BufLineAppend(overlay, line - 1, "%d Level Limit",
                        MAX_UNDO_LEV - countD.pundo);
        }
      } else {
        BufLineAppend(overlay, line - 1, "Enabled");
      }
    } else {
      BufLineAppend(overlay, line - 1, "Disabled");
    }

    if (HACK && replay_flag) {
      BufLineAppend(overlay, line - 1, " %d/%d action/input %d/%d",
                    replayD->input_action_usedD, replayD->input_record_writeD,
                    (int)AL(replayD->input_actionD),
                    (int)AL(replayD->input_recordD));
    }

    BufMsg(overlay, "-");
    BufMsg(overlay, can_reset(death) ? "d) Dungeon reset" : "-");
    BufMsg(overlay, "e) Extra features");
    BufMsg(overlay, "-");
    BufMsg(overlay, "g) Game reset");
    if (PC) BufMsg(overlay, "h) Help");
    if (PC) {
      line = 'q' - 'a';
      BufMsg(overlay, "q) quit");
    }

    if (uD.vow_flag) {
      line = 'v' - 'a';
      BufMsg(overlay, "v) vow display");
    }

    if (!in_subcommand(prompt, &c)) break;

    switch (c) {
      case 'a':
        if (death) continue;
        py_rest();
        return 0;

      case 'b':
        py_undo();
        return 0;

      case 'd':
        dungeon_reset(death);
        break;

      case 'e':
        if (GFX) feature_menu();
        break;

      case 'g':
        // TBD: improve handling of game reset with vow of death
        if (death) ST_INC(recallD[death_creD].r_death);
        if (!death) platformD.savemidpoint();
        globalD.saveslot_class = -1;
        longjmp(restartD, 1);
        break;

      case 'h':
        if (PC) py_help();
        break;
      case 'q':
        quitD = 1;
        break;
      case 'v':
        vow_display();
        c = CLOBBER_MSG("Viewing your vows:");
        break;
    }
  }
  return c;
}
STATIC int
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
STATIC int
wizard_prompt(yptr, xptr)
int* yptr;
int* xptr;
{
  int x = 0, y = 0;
  msg_hint(AP("adefhlmtosw<>"));
  char c = CLOBBER_MSG("Enter wizard command:");
  turn_flag = TRUE;
  switch (c) {
    case 'a': {
      uD.exp += 1000000;
      py_experience();
      dun_level = 0;
      uD.new_level_flag = NL_DOWN_STAIR;
      uD.gold = 10000;
    } break;
    case 'b':
      uD.total_winner = 1;
      FOR_EACH(mon, {
        if (mon->cidx == MAX_WIN_MON + m_level[MAX_MON_LEVEL])
          mon_take_hit(mon->id, 10000);
      });
      break;
    case 'd':
      c = CLOBBER_MSG("Detect monster (e/i/m):");
      int ma_type = 0;
      if (c == 'e') {
        ma_type = MA_DETECT_EVIL;
      } else if (c == 'i') {
        ma_type = MA_DETECT_INVIS;
      } else if (c == 'm') {
        ma_type = MA_DETECT_MON;
      }
      detect_mon(ma_type, 1);
      ma_duration(ma_type, 1 + uD.lev / 5);
      break;
    case 'e': {
      earthquake();
    } break;
    case 'f': {
      viz_hookD = viz_magick;
      magick_distD = 2;
      magick_locD = (point_t){uD.x, uD.y};
      magick_hituD = 0;
    } break;
    case 'h':
      uD.chp = uD.mhp;
      msg_print("You are healed.");
      break;
    case 'l': {
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
    case 'm':
      map_area();
      // Reveal traps too
      for (int row = 1; row < MAX_HEIGHT; ++row) {
        for (int col = 1; col < MAX_WIDTH; ++col) {
          struct caveS* c_ptr = &caveD[row][col];
          struct objS* obj = &entity_objD[c_ptr->oidx];
          if (obj->tval == TV_INVIS_TRAP) {
            obj->tval = TV_VIS_TRAP;
            obj->tchar = '^';
            c_ptr->cflag |= CF_FIELDMARK;
          }
        }
      }
      break;
    case 't':
      msg_print("teleport");
      do {
        x = randint(MAX_WIDTH - 2);
        y = randint(MAX_HEIGHT - 2);
      } while (caveD[y][x].fval >= MIN_CLOSED_SPACE || caveD[y][x].midx != 0);
      break;
    case 'o': {
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
    case 's': {
      msg_print("Store maintenance.");
      store_maint();
    } break;
    case 'w': {
      msg_print("The air about you becomes charged.");
      ma_duration(MA_RECALL, 1);
    } break;
    case '<':
      dun_level -= 1;
      uD.new_level_flag = NL_UP_STAIR;
      break;
    case '>':
      dun_level += 1;
      uD.new_level_flag = NL_DOWN_STAIR;
      break;
  }
  if (x) *xptr = x;
  if (y) *yptr = y;
  return 0;
}
STATIC int
confirm_transition(prev, trans)
char* trans;
{
  char c = CLOBBER_MSG("Press key twice for %s.", trans);
  return (c == prev) ? prev : 0;
}
STATIC int
py_endgame(fn endtype)
{
  char c = 0;
  do {
    if (c == 'p') {
      c = show_history();
    } else if (c == CTRL('z')) {
      py_undo();
      c = ESCAPE;
    } else if (c == 'c') {
      c = show_character();
    } else if (c == 'o') {
      // Observe game state
      c = draw(DRAW_WAIT);
    } else {
      c = endtype();
    }

    if (c == CTRL('c')) break;
  } while (c != ESCAPE);
  return c;
}
STATIC int
inven_damage(typ, perc)
{
  int it, j;

  j = 0;
  for (it = 0; it < INVEN_EQUIP; it++) {
    struct objS* obj = obj_get(invenD[it]);
    if (is_obj_vulnmelee(obj, typ) && (randint(100) < perc)) {
      inven_destroy_num(it, 1);
      j++;
    }
  }
  return (j);
}
STATIC int
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
    descD[0] &= ~0x20;
    if (obj->flags & TR_RES_ACID) {
      MSG("%s resists damage.", descD);
      minus = TRUE;
    } else if ((obj->ac + obj->toac) > 0) {
      MSG("%s is damaged.", descD);
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
STATIC int
poison_gas(dam)
{
  py_take_hit(dam / 2);
  countD.poison += 12 + dam / 2;
  return dam;
}
STATIC int
fire_dam(dam)
{
  int absfire, resfire;

  absfire = py_affect(MA_AFIRE);
  resfire = py_tr(TR_RES_FIRE);

  if (absfire) dam = dam / 2;
  if (resfire) dam = dam / 2;
  py_take_hit(dam);
  if (!absfire && inven_damage(GF_FIRE, 3) > 0)
    msg_print("There is smoke coming from your pack!");
  return dam;
}
STATIC int
acid_dam(dam, verbose)
{
  if (minus_ac(verbose)) dam = dam / 2;
  if (py_tr(TR_RES_ACID)) dam = dam / 2;
  py_take_hit(dam);
  if (inven_damage(GF_ACID, 3) > 0)
    msg_print("There is an acrid smell coming from your pack!");
  return dam;
}
STATIC int
frost_dam(dam)
{
  int abscold, rescold;

  abscold = py_affect(MA_AFROST);
  rescold = py_tr(TR_RES_COLD);
  if (abscold) dam = dam / 2;
  if (rescold) dam = dam / 2;
  py_take_hit(dam);
  if (!abscold && inven_damage(GF_FROST, 5) > 0)
    msg_print("Something shatters inside your pack!");
  return dam;
}
STATIC int
light_dam(dam)
{
  if (py_tr(TR_RES_LIGHT)) dam = dam / 2;
  py_take_hit(dam);
  if (inven_damage(GF_LIGHTNING, 3) > 0)
    msg_print("There are sparks coming from your pack!");
  return dam;
}
STATIC void
corrode_gas(verbose)
{
  if (!minus_ac(verbose)) py_take_hit(randint(8));
  if (inven_damage(GF_POISON_GAS, 5) > 0)
    msg_print("There is an acrid smell coming from your pack.");
}
STATIC void
py_attack(y, x)
{
  int k, blows, surprise, hit_count;
  int base_tohit, lev_adj, tohit, todam, tval, tweight, wheavy, creature_ac;

  int midx = caveD[y][x].midx;
  struct monS* mon = &entity_monD[midx];
  struct creatureS* cre = &creatureD[mon->cidx];
  struct objS* obj = obj_get(invenD[INVEN_WIELD]);

  int mlit = mon_lit(midx);
  turn_flag = TRUE;
  tval = obj->tval;
  creature_ac = cre->ac;
  if (py_affect(MA_FEAR)) {
    msg_print("You are too afraid!");
  } else {
    // Barehand
    blows = 2;
    tweight = 1;
    wheavy = 0;
    // Weapon
    if (tval) {
      blows = attack_blows(obj->weight);
      tweight = obj->weight;
      wheavy = tohit_by_weight(obj->weight);
    }

    tohit = cbD.ptohit;
    todam = cbD.ptodam;
    base_tohit = uD.bth;
    lev_adj = uD.lev * level_adj[uD.clidx][LA_BTH];
    // reduce hit if monster not lit
    if (mlit == 0) {
      if (tohit > 0) tohit /= 2;
      base_tohit /= 2;
      lev_adj /= 2;
    }

    // Barehand after other penalties
    if (!tval) tohit -= 3;
    // Add penalty for weapon too heavy
    tohit += wheavy;

    surprise = 0;
    if (mon->msleep && perceive_creature2(cre)) {
      // Effectively x^2 chance to hit a sleeping monster
      // This preserves early game difficulty, invis penalties, weapon too_heavy
      surprise = test_hit(base_tohit, lev_adj, tohit, creature_ac);
      if (surprise) surprise = mon_surprise(mon, mlit);
    }

    // You make a lot of noise
    mon->msleep = 0;

    mon_desc(midx);
    descD[0] = descD[0] | 0x20;
    /* Loop for number of blows,  trying to hit the critter.	  */
    hit_count = 0;
    for (int it = 0; it < blows; ++it) {
      // Only the first blow may surprise the beast
      if (it > 0) surprise = 0;

      if (surprise || test_hit(base_tohit, lev_adj, tohit, creature_ac)) {
        hit_count += 1;
        MSG("You hit %s.", descD);
        k = 1;
        if (tval) {
          k = pdamroll(obj->damage);
          k = tot_dam(obj, k, mon->cidx);
        }

        k = critical_blow(tweight, tohit + surprise, lev_adj, k);
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

    if (wheavy && !hit_count)
      msg_print("You have trouble wielding such a heavy weapon.");
  }
}
STATIC void
mon_attack(midx)
{
  int bth, flag;
  struct monS* mon = &entity_monD[midx];
  struct creatureS* cre = &creatureD[mon->cidx];

  int adj = cre->level * CRE_LEV_ADJ;
  for (int it = 0; it < AL(cre->attack_list); ++it) {
    if (uD.new_level_flag) break;
    if (!cre->attack_list[it]) break;
    int mlit = mon_desc(midx);
    struct attackS* attack = &attackD[cre->attack_list[it]];

    int notice = mlit;
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
          } else {
            notice = 0;
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
              notice = 0;
            }
          } else {
            notice = 0;
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
            notice = 0;
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
          msg_print("You are chilled with frost!");
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
            notice = 0;
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
          } else {
            notice = 0;
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
          if (!equip_disenchant()) notice = 0;
          break;
        case 22: /*Eat food     */
        {
          int l = inven_food();
          if (l >= 0) {
            inven_destroy_num(l, 1);
            msg_print("It got at your rations!");
          } else {
            notice = 0;
          }
        } break;
        case 23: /*Eat light     */
        {
          struct objS* obj = obj_get(invenD[INVEN_LIGHT]);
          if (obj->p1 > 0) {
            obj->p1 = MAX(obj->p1 - 250 + randint(250), 1);
            see_print("Your light dims.");
          } else {
            notice = 0;
          }
        } break;
        case 24: /*Eat charges    */
        {
          int iidx = inven_random();
          if (iidx >= 0) {
            struct objS* obj = obj_get(invenD[iidx]);
            if (oset_zap(obj) && obj->p1 > 0) {
              int gain = cre->level * obj->p1;
              // Overflow check (balrog, mainly)
              if (mon->hp + gain > mon->hp) mon->hp += gain;
              obj->p1 = 0;
              obj->idflag |= ID_EMPTY;
              msg_print("Energy drains from your pack!");
            } else {
              notice = 0;
            }
          }
        } break;
        case 99: {
          notice = 0;
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

      if (notice) ST_INC(recallD[mon->cidx].r_attack[it]);
    } else {
      MSG("%s misses you.", descD);
    }
  }
}
STATIC void
chest_trap(y, x)
{
  int i;
  struct objS* obj;

  obj = &entity_objD[caveD[y][x].oidx];
  if (CH_LOSE_STR & obj->flags) {
    msg_print("A small needle pricks you!");
    death_desc("a poison needle");
    py_take_hit(damroll(1, 4));
    lose_stat(A_STR);
  }
  if (CH_POISON & obj->flags) {
    msg_print("A small needle pricks you!");
    death_desc("a poison needle");
    py_take_hit(damroll(1, 6));
    countD.poison += 10 + randint(20);
  }
  if (CH_PARALYSED & obj->flags) {
    msg_print("A puff of yellow gas surrounds you!");
    if (py_tr(TR_FREE_ACT))
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
    death_desc("an exploding chest");
    py_take_hit(damroll(5, 8));
  }
}
STATIC void
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
      MSG("You disarm %s.", descD);
      uD.exp += obj->p1;
      delete_object(y, x);
      py_experience();
    } else {
      MSG("You fail to disarm %s.", descD);
    }
  }
}
STATIC void
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
      msg_print("You disarm the chest.");
      obj->idflag = ID_REVEAL;
      uD.exp += obj->level;
      py_experience();
    } else {
      msg_print("You fail to disarm the chest.");
    }
  }
}
STATIC void
door_bash(y, x)
{
  int tmp;
  struct caveS* c_ptr;
  struct objS* obj;

  c_ptr = &caveD[y][x];
  obj = &entity_objD[c_ptr->oidx];

  if (c_ptr->midx == 0 && obj->tval == TV_CLOSED_DOOR) {
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
    } else if (randint(150) > statD.use_stat[A_DEX]) {
      msg_print("You are off-balance.");
      countD.paralysis = 1 + randint(2);
    } else
      msg_print("The door holds firm.");
  }
}
STATIC void
py_drop()
{
  int iidx;
  iidx = inven_choice("Drop one item?", "*/");

  if (iidx >= 0) inven_drop(iidx);
}
STATIC void
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
          msg_print("You pick the lock.");
          uD.exp += 1;
          py_experience();
          obj->p1 = 0;
        } else
          msg_print("You fail to pick the lock.");
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
          msg_print("You pick the lock.");
          flag = TRUE;
          uD.exp += obj->level;
          py_experience();
        } else
          msg_print("You fail to pick the lock.");
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
        mon_death(y, x, obj->flags);
        obj->flags = 0;
      }
    }
  } else {
    msg_print("You do not see anything you can open there.");
  }
}
STATIC void
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
          msg_print("You find a secret door.");
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
            MSG("You find %s.", descD);
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
STATIC void
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
    MSG("You find %d gold pieces worth of %s.", obj->cost,
        gold_nameD[obj->subval]);
    delete_object(y, x);
    turn_flag = TRUE;
  } else if (obj->tval <= TV_MAX_PICK_UP) {
    locn = -1;
    merge = inven_merge(obj->id, &locn);
    if (!merge && pickup) locn = inven_carry(obj->id);

    obj_desc(obj, obj->number);
    obj_detail(obj, 0);
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
STATIC int
py_monlook_dir(dir)
{
  int y = uD.y;
  int x = uD.x;
  int ly = dir_y(dir);
  int lx = dir_x(dir);

  rect_t zr;
  zoom_rect(&zr);

  int rownum = zr.y + zr.h;
  int colnum = zr.x + zr.w;
  int mon_list[AL(entity_monD)];
  int mon_count = 0;
  for (int row = zr.y; row < rownum; ++row) {
    for (int col = zr.x; col < colnum; ++col) {
      struct caveS* c_ptr = &caveD[row][col];
      if (mon_lit(c_ptr->midx)) {
        int fy = row;
        int fx = col;
        int oy = (ly != 0) * (-((fy - y) < 0) + ((fy - y) > 0));
        int ox = (lx != 0) * (-((fx - x) < 0) + ((fx - x) > 0));
        if ((oy == ly) && (ox == lx) && los(y, x, fy, fx)) {
          mon_list[mon_count] = c_ptr->midx;
          mon_count += 1;
        }
      }
    }
  }

  for (int i = 0; i < mon_count; ++i) {
    for (int j = i + 1; j < mon_count; ++j) {
      struct monS* lptr = &entity_monD[mon_list[i]];
      struct monS* rptr = &entity_monD[mon_list[j]];
      if (distance(y, x, rptr->fy, rptr->fx) <
          distance(y, x, lptr->fy, lptr->fx))
        SWAP(mon_list[i], mon_list[j]);
    }
  }

  for (int it = 0; it < mon_count; ++it) {
    int midx = mon_list[it];
    struct monS* mon = &entity_monD[midx];
    struct creatureS* cre = &creatureD[mon->cidx];
    ylookD = mon->fy;
    xlookD = mon->fx;
    msg_hint(AP("(b: bestiary, ESC)"));
    char c =
        CLOBBER_MSG("You see %s %s.%s", is_a_vowel(cre->name[0]) ? "a" : "an",
                    cre->name, mon->msleep ? " (sleeping)" : "");
    if (c == ESCAPE) it = mon_count;
    if (c == 'b') mon_bestiary(mon, cre);
  }
  return mon_count;
}
STATIC int
py_objlook_dir(dir)
{
  int y = uD.y;
  int x = uD.x;
  int ly = dir_y(dir);
  int lx = dir_x(dir);

  rect_t zr;
  zoom_rect(&zr);

  int rownum = zr.y + zr.h;
  int colnum = zr.x + zr.w;
  int obj_list[AL(entity_objD)];
  int obj_count = 0;
  for (int row = zr.y; row < rownum; ++row) {
    for (int col = zr.x; col < colnum; ++col) {
      struct caveS* c_ptr = &caveD[row][col];
      if (c_ptr->oidx) {
        int fy = row;
        int fx = col;
        int oy = (ly != 0) * (-((fy - y) < 0) + ((fy - y) > 0));
        int ox = (lx != 0) * (-((fx - x) < 0) + ((fx - x) > 0));
        if (oy == ly && ox == lx && (CF_VIZ & caveD[fy][fx].cflag)) {
          obj_list[obj_count] = c_ptr->oidx;
          obj_count += 1;
        }
      }
    }
  }

  for (int i = 0; i < obj_count; ++i) {
    for (int j = i + 1; j < obj_count; ++j) {
      struct objS* lptr = &entity_objD[obj_list[i]];
      struct objS* rptr = &entity_objD[obj_list[j]];
      if (distance(y, x, rptr->fy, rptr->fx) <
          distance(y, x, lptr->fy, lptr->fx))
        SWAP(obj_list[i], obj_list[j]);
    }
  }

  for (int it = 0; it < obj_count; ++it) {
    int oidx = obj_list[it];
    struct objS* obj = &entity_objD[oidx];
    if (obj->tval == TV_INVIS_TRAP || obj->tval == TV_SECRET_DOOR) continue;
    ylookD = obj->fy;
    xlookD = obj->fx;
    obj_desc(obj, obj->number);
    msg_hint(AP("(ESC)"));
    char c = CLOBBER_MSG("You see %s.", descD);
    if (c == ESCAPE) it = obj_count;
  }

  return obj_count;
}
STATIC void
py_examine()
{
  int dir;

  if (py_affect(MA_BLIND))
    msg_print("You can't see a thing!");
  else {
    if (get_dir("Which direction will you look?", &dir)) {
      int seen = 0;
      msg_moreD = 1;
      seen += py_monlook_dir(dir);
      seen += py_objlook_dir(dir);
      msg_moreD = 0;

      if (seen)
        msg_print("That's all you see in that direction");
      else
        msg_print("You see nothing in that direction.");
    }
  }
}
int
roff_recall(mon_num, reveal)
{
  int i, k, j;
  char temp[128];
  struct recallS* recall = &recallD[mon_num];
  struct creatureS* cr_ptr = &creatureD[mon_num];

  uint32_t rspells = recall->r_spells & cr_ptr->spells & ~CS_FREQ;
  uint32_t rcmove = recall->r_cmove & cr_ptr->cmove;
  uint16_t rcdefense = recall->r_cdefense & cr_ptr->cdefense;
  int ulev = uD.lev;

  // Quylthulgs
  if (cr_ptr->cmove & CM_ONLY_MAGIC && recall->r_attack[0] > 20)
    rcmove |= CM_ONLY_MAGIC;

  if (reveal) {
    recall->r_kill = 255;
    rcmove = cr_ptr->cmove;
    rcdefense = cr_ptr->cdefense;
    for (int it = 0; it < 4; ++it)
      recall->r_attack[it] = cr_ptr->attack_list[it];
  }
  if (reveal > 1) {
    rcmove = -1;
    rcdefense = -1;
  }

  if (recall->r_death) {
    snprintf(AP(temp), "%d of the contributors to your monster memory %s",
             recall->r_death, ((recall->r_death) == 1 ? "has" : "have"));
    roff(temp);
    roff(" been killed by this creature, and ");
    if (recall->r_kill == 0)
      roff("it is not ever known to have been defeated.");
    else {
      snprintf(AP(temp), "at least %d of the beasts %s been exterminated.",
               recall->r_kill, ((recall->r_kill) == 1 ? "has" : "have"));
      roff(temp);
    }
  } else if (recall->r_kill) {
    snprintf(AP(temp), "At least %d of these creatures %s", recall->r_kill,
             ((recall->r_kill) == 1 ? "has" : "have"));
    roff(temp);
    roff(" been killed by contributors to your monster memory.");
  } else
    roff("No known battles to the death are recalled.");

  k = FALSE;
  if (cr_ptr->level == 0) {
    roff(" It lives in the town");
    k = TRUE;
  } else if (recall->r_kill) {
    // The Balrog is a level 100 monster, but appears at 50 feet.
    i = cr_ptr->level;
    if (i > WIN_MON_APPEAR) i = WIN_MON_APPEAR;
    snprintf(AP(temp), " It is normally found at depths of %d feet", i * 50);
    roff(temp);
    k = TRUE;
  }

  // the c_list speed value is 10 greater, so that it can be a int8u
  int mspeed = cr_ptr->speed - 10;
  if (rcmove & CM_ALL_MV_FLAGS) {
    if (k)
      roff(", and");
    else {
      roff(" It");
      k = TRUE;
    }
    roff(" moves");
    if (rcmove & CM_RANDOM_MOVE) {
      int hm = (rcmove & CM_RANDOM_MOVE) >> 3;
      roff(desc_howmuch[hm]);
      roff(" erratically");
    }
    if (mspeed == 1)
      roff(" at normal speed");
    else {
      if (rcmove & CM_RANDOM_MOVE) roff(", and");
      if (mspeed <= 0) {
        if (mspeed == -1)
          roff(" very");
        else if (mspeed < -1)
          roff(" incredibly");
        roff(" slowly");
      } else {
        if (mspeed == 3)
          roff(" very");
        else if (mspeed > 3)
          roff(" unbelievably");
        roff(" quickly");
      }
    }
  }
  if (rcmove & CM_ATTACK_ONLY) {
    if (k)
      roff(", but");
    else {
      roff(" It");
      k = TRUE;
    }
    roff(" does not deign to chase intruders");
  }
  if (rcmove & CM_ONLY_MAGIC) {
    if (k)
      roff(", but");
    else {
      roff(" It");
      k = TRUE;
    }
    roff(" always moves and attacks by using magic");
  }
  if (k) roff(".");

  // Kill it once to know experience, and quality (evil, undead, monsterous).
  if (recall->r_kill) {
    roff(" A kill of this");
    if (cr_ptr->cdefense & CD_ANIMAL) roff(" natural");
    if (cr_ptr->cdefense & CD_EVIL) roff(" evil");
    if (cr_ptr->cdefense & CD_UNDEAD) roff(" undead");
    int tempxp = cr_ptr->mexp * cr_ptr->level / ulev;
    // calculate the fractional exp part scaled by 100
    j = ((cr_ptr->mexp * cr_ptr->level % ulev) * 1000 / ulev + 5) / 10;
    snprintf(AP(temp), " creature is worth %d.%02d point%s", tempxp, j,
             (tempxp == 1 && j == 0 ? "" : "s"));
    roff(temp);
    char* p = "";
    if (ulev / 10 == 1)
      p = "th";
    else {
      i = ulev % 10;
      if (i == 1)
        p = "st";
      else if (i == 2)
        p = "nd";
      else if (i == 3)
        p = "rd";
      else
        p = "th";
    }
    char* q = "";
    if (ulev == 8 || ulev == 11 || ulev == 18) q = "n";
    snprintf(AP(temp), " for a%s %d%s level character.", q, ulev, p);
    roff(temp);
  }

  // Spells known, if have been used against us.
  // Breath weapons or resistance
  k = TRUE;
  j = rspells;
  for (i = 0; j & CS_BREATHE; i++) {
    if (j & (CS_BR_LIGHT << i)) {
      j &= ~(CS_BR_LIGHT << i);
      if (k) {
        if (recall->r_spells & CS_FREQ)
          roff(" It can breathe ");
        else
          roff(" It is resistant to ");
        k = FALSE;
      } else if (j & CS_BREATHE)
        roff(", ");
      else
        roff(" and ");
      roff(desc_breath[i]);
    }
  }
  k = TRUE;
  for (i = 0; j & CS_SPELLS; i++) {
    if (j & (CS_TEL_SHORT << i)) {
      j &= ~(CS_TEL_SHORT << i);
      if (k) {
        if (rspells & CS_BREATHE)
          roff(", and is also");
        else
          roff(" It is");
        roff(" magical, casting spells which ");
        k = FALSE;
      } else if (j & CS_SPELLS)
        roff(", ");
      else
        roff(" or ");
      roff(mon_spell_nameD[i]);
    }
  }
  if (rspells & (CS_BREATHE | CS_SPELLS)) {
    if ((recall->r_spells & CS_FREQ) > 5) {
      snprintf(AP(temp), "; 1 time in %lu", cr_ptr->spells & CS_FREQ);
      roff(temp);
    }
    roff(".");
  }

  // Do we know how hard they are to kill? Armor class, hit die.
  if (((recall->r_kill) > 304 / (4 + (cr_ptr->level)))) {
    snprintf(AP(temp), " It has an armor rating of %d", cr_ptr->ac);
    roff(temp);
    snprintf(AP(temp), " and a%s life rating of %dd%d.",
             ((cr_ptr->cdefense & CD_MAX_HP) ? " maximized" : ""),
             cr_ptr->hd[0], cr_ptr->hd[1]);
    roff(temp);
  }

  // Do we know how clever they are? Special abilities.
  k = TRUE;
  j = rcmove;
  for (i = 0; j & CM_SPECIAL; i++) {
    if (j & (CM_INVISIBLE << i)) {
      j &= ~(CM_INVISIBLE << i);
      if (k) {
        roff(" It can ");
        k = FALSE;
      } else if (j & CM_SPECIAL)
        roff(", ");
      else
        roff(" and ");
      roff(desc_move[i]);
    }
  }
  if (!k) roff(".");
  // Do we know its special weaknesses? Most cdefense flags.
  k = TRUE;
  j = rcdefense;
  for (i = 0; j & CD_WEAKNESS; i++) {
    if (j & (CD_FROST << i)) {
      j &= ~(CD_FROST << i);
      if (k) {
        roff(" It is susceptible to ");
        k = FALSE;
      } else if (j & CD_WEAKNESS)
        roff(", ");
      else
        roff(" and ");
      roff(desc_weakness[i]);
    }
  }
  if (!k) roff(".");
  if (rcdefense & CD_INFRA) roff(" It is warm blooded");
  if (rcdefense & CD_NO_SLEEP) {
    if (rcdefense & CD_INFRA)
      roff(", and");
    else
      roff(" It");
    roff(" cannot be charmed or slept");
  }
  if (rcdefense & (CD_NO_SLEEP | CD_INFRA)) roff(".");

  // Do we know how aware it is?
  if (((recall->r_wake * recall->r_wake) > cr_ptr->sleep) ||
      (cr_ptr->sleep == 0 && recall->r_kill >= 10)) {
    roff(" It ");
    if (cr_ptr->sleep > 200)
      roff("prefers to ignore");
    else if (cr_ptr->sleep > 95)
      roff("pays very little attention to");
    else if (cr_ptr->sleep > 75)
      roff("pays little attention to");
    else if (cr_ptr->sleep > 45)
      roff("tends to overlook");
    else if (cr_ptr->sleep > 25)
      roff("takes quite a while to see");
    else if (cr_ptr->sleep > 10)
      roff("takes a while to see");
    else if (cr_ptr->sleep > 5)
      roff("is fairly observant of");
    else if (cr_ptr->sleep > 3)
      roff("is observant of");
    else if (cr_ptr->sleep > 1)
      roff("is very observant of");
    else if (cr_ptr->sleep != 0)
      roff("is vigilant for");
    else
      roff("is ever vigilant for");
    snprintf(AP(temp), " intruders, which it may notice from %d feet.",
             10 * cr_ptr->aaf);
    roff(temp);
  }
  // Do we know what it might carry?
  if (rcmove & (CM_CARRY_OBJ | CM_CARRY_GOLD)) {
    roff(" It may");
    int trcount = recall->r_treasure;
    if (trcount == 1) {
      if ((cr_ptr->cmove & CM_TREASURE) == CM_60_RANDOM)
        roff(" sometimes");
      else
        roff(" often");
    } else if ((trcount == 2) &&
               ((cr_ptr->cmove & CM_TREASURE) == (CM_60_RANDOM | CM_90_RANDOM)))
      roff(" often");
    roff(" carry");
    char* p = "";
    if (rcmove & CM_SMALL_OBJ)
      p = " small objects";
    else
      p = " objects";
    if (trcount == 1) {
      if (rcmove & CM_SMALL_OBJ)
        p = " a small object";
      else
        p = " an object";
    } else if (trcount == 2)
      roff(" one or two");
    else {
      snprintf(AP(temp), " up to %d", trcount);
      roff(temp);
    }
    if (rcmove & CM_CARRY_OBJ) {
      roff(p);
      if (rcmove & CM_CARRY_GOLD) {
        roff(" or treasure");
        if (trcount > 1) roff("s");
      }
      roff(".");
    } else if (trcount != 1)
      roff(" treasures.");
    else
      roff(" treasure.");
  }

  // We know about attacks it has used on us, and maybe the damage they do.
  // k is the total number of known attacks, used for punctuation
  k = 0;
  for (j = 0; j < 4; j++)
    if (recall->r_attack[j]) k++;
  uint8_t* pu = cr_ptr->attack_list;
  // j counts the attacks as printed, used for punctuation
  j = 0;
  for (i = 0; *pu != 0 && i < 4; pu++, i++) {
    int att_type, att_how, d1, d2;
    // don't print out unknown attacks
    if (!recall->r_attack[i]) continue;
    att_type = attackD[*pu].attack_type;
    att_how = attackD[*pu].attack_desc;
    d1 = attackD[*pu].attack_dice;
    d2 = attackD[*pu].attack_sides;
    j++;
    if (j == 1)
      roff(" It can ");
    else if (j == k)
      roff(", and ");
    else
      roff(", ");
    if (att_how > 19) att_how = 0;
    roff(desc_amethod[att_how]);
    if (att_type != 1 || d1 > 0 && d2 > 0) {
      roff(" to ");
      if (att_type > 24) att_type = 0;
      roff(desc_atype[att_type]);
      if (d1 && d2) {
        if (((4 + (cr_ptr->level)) * (recall->r_attack[i]) > 80 * (d1 * d2))) {
          if (att_type == 19)  // Loss of experience
            roff(" by");
          else
            roff(" with damage");
          snprintf(AP(temp), " %dd%d", d1, d2);
          roff(temp);
        }
      }
    }
  }
  if (j)
    roff(".");
  else if (k > 0 && recall->r_attack[0] >= 10)
    roff(" It has no physical attacks.");
  else
    roff(" Nothing is known about its attack.");
  // Always know the win creature.
  if (cr_ptr->cmove & CM_WIN) roff(" Killing one of these wins the game!");
  roff(" ");
}
int
roff(char* msg)
{
  apcat(AP(monmemD), msg);
}
STATIC int
mon_bestiary(struct monS* mon, struct creatureS* cre)
{
  screen_submodeD = 1;

  AC(monmemD);
  roff_recall(mon->cidx, 0);

  enum { LINE = 64 };
  int offset = 0;
  for (int line = 0; line < AL(screenD); ++line) {
    int next_br = 0;
    for (int it = LINE - 1; it > 0; --it) {
      if (monmemD[offset + it] == ' ') {
        next_br = it;
        break;
      }
    }
    memcpy(&screenD[line][0], &monmemD[offset], next_br);
    screen_usedD[line] = next_br;

    offset += next_br;
    offset += 1;
  }
  return CLOBBER_MSG("'%c' The %s:", cre->cchar, cre->name);
}
STATIC void
mon_look(midx)
{
  struct monS* mon = &entity_monD[midx];
  struct creatureS* cre = &creatureD[mon->cidx];
  msg_hint(AP("(tap for beastiary)"));
  char c =
      CLOBBER_MSG("You see %s %s.%s", is_a_vowel(cre->name[0]) ? "a" : "an",
                  cre->name, mon->msleep ? " (sleeping)" : "");
  if (c == 'O') mon_bestiary(mon, cre);
}
STATIC void
py_look(y, x)
{
  struct caveS* c_ptr;
  struct objS* obj;
  struct monS* mon;

  if (MOBILE) {
    c_ptr = &caveD[y][x];
    if (py_affect(MA_BLIND)) {
      MSG("You can't see a thing!");
    } else if (mon_lit(c_ptr->midx)) {
      mon_look(c_ptr->midx);
    } else if (c_ptr->oidx && (CF_VIZ & c_ptr->cflag)) {
      obj = &entity_objD[c_ptr->oidx];
      if (obj->tval == TV_INVIS_TRAP) {
        MSG("You see the dungeon floor.");
      } else if (obj->tval == TV_SECRET_DOOR) {
        MSG("You see a %s.", walls[0]);
      } else {
        obj_desc(obj, obj->number);
        MSG("You see %s.", descD);
      }
    } else if (y == uD.y && x == uD.x) {
      MSG("Looking good, hero.");
    } else if (c_ptr->cflag & CF_SEEN) {
      if (c_ptr->fval >= MIN_WALL) {
        int wall_idx = c_ptr->fval - MIN_WALL;
        if (wall_idx < AL(walls)) MSG("You see a %s.", walls[wall_idx]);
      } else {
        MSG("You see the dungeon floor.");
      }
    } else {
      MSG("You don't see anything.");
    }
  }
}
STATIC int
objdig_wall_plus(obj, wall_chance, wall_min)
struct objS* obj;
int wall_chance;
int wall_min;
{
  int tabil = obj_tabil(obj, TRUE);
  int tbonus = obj_tbonus(obj);
  for (int it = 0; it < MAX_TUNNEL_TURN; ++it) {
    int treq = randint(wall_chance) + wall_min;
    if (tabil > treq) return it;
    tabil += tbonus;
  }
  return MAX_TUNNEL_TURN;
}
STATIC int
tunnel_tool(y, x, iidx)
{
  struct caveS* c_ptr;
  struct objS* obj;
  int c_tval;
  int turn_count = 0;

  c_ptr = &caveD[y][x];
  c_tval = entity_objD[c_ptr->oidx].tval;
  obj = obj_get(invenD[iidx]);
  obj_desc(obj, 1);
  if (c_tval == TV_CLOSED_DOOR) {
    msg_print("You can't tunnel through a door!");
  } else if (c_tval == TV_SECRET_DOOR) {
    MSG("You tunnel into the granite wall using %s.", descD);
    do {
      turn_count += 1;
      py_search(uD.y, uD.x);
      if (entity_objD[c_ptr->oidx].tval == TV_CLOSED_DOOR) break;
    } while (turn_count < MAX_TUNNEL_TURN);
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
    int wall_chance[] = {1200, 600, 400, 180};
    int wall_min[] = {80, 16, 10, 0};
    char* wall_name[] = {
        "granite",
        "magma",
        "quartz",
        "rubble",
    };
    unsigned wall_idx = c_ptr->fval - MIN_WALL;
    if (wall_idx > 2) wall_idx = 3;
    MSG("You dig into the %s using %s.", wall_name[wall_idx], descD);
    turn_count =
        objdig_wall_plus(obj, wall_chance[wall_idx], wall_min[wall_idx]);
    if (wall_idx < 3) {
      if (turn_count < MAX_TUNNEL_TURN) {
        twall(y, x);
        msg_print("You finish the tunnel.");
      }
    } else {
      if (turn_count < MAX_TUNNEL_TURN) {
        c_ptr->fval = FLOOR_CORR;
        delete_object(y, x);
        int new_obj = (randint(10) == 1);
        if (new_obj) place_object(y, x, FALSE);
        if (new_obj && (CF_LIT & c_ptr->cflag)) {
          msg_print("You find something in the rubble!");
        } else {
          msg_print("You remove the rubble.");
        }
      }
    }
    if (turn_count < MAX_TUNNEL_TURN) turn_count += 1;
  }

  if (turn_count) {
    turn_flag = TRUE;
    // TBD: unique counter for mining?
    countD.paralysis += turn_count;
  }

  return turn_count;
}
STATIC int
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
STATIC void
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
STATIC void make_move(midx, mm, rc_ptr) int* mm;
uint32_t* rc_ptr;
{
  int fy, fx, newy, newx, do_move;
  int py, px;
  int cmove;
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;
  struct objS* obj;

  do_move = FALSE;
  m_ptr = &entity_monD[midx];
  cr_ptr = &creatureD[m_ptr->cidx];
  cmove = cr_ptr->cmove;
  fy = m_ptr->fy;
  fx = m_ptr->fx;
  py = uD.y;
  px = uD.x;

  for (int i = 0; i < 5; ++i) {
    newy = fy + dir_y(mm[i]);
    newx = fx + dir_x(mm[i]);
    c_ptr = &caveD[newy][newx];
    obj = &entity_objD[c_ptr->oidx];

    if (c_ptr->fval == BOUNDARY_WALL)
      continue;
    else if (cmove & CM_PHASE)
      do_move = TRUE;
    else if (c_ptr->fval == FLOOR_OBST) {
      if (obj->tval == TV_CLOSED_DOOR || obj->tval == TV_SECRET_DOOR) {
        do_move = FALSE;
        if ((cmove & CM_OPEN_DOOR) != 0 && obj->p1 == 0) {
          obj->tval = TV_OPEN_DOOR;
          obj->tchar = '\'';
          if (c_ptr->cflag & CF_LIT) msg_print("A door creaks open.");
          if (c_ptr->cflag & CF_LIT) *rc_ptr |= CM_OPEN_DOOR;
        } else if ((cmove & CM_OPEN_DOOR) != 0 && obj->p1 > 0) {
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
        if (!do_move) break;  // do_turn
      } else {
        // permit attack-only against a player
        do_move = (newy == py && newx == px);
      }
    } else if (c_ptr->fval <= MAX_OPEN_SPACE)
      do_move = TRUE;

    if (do_move && obj->tval == TV_GLYPH) {
      if (randint(obj->p1) < cr_ptr->level) {
        msg_print("The glyph of protection is broken!");
        delete_object(newy, newx);
      } else {
        do_move = FALSE;
        if (cmove & CM_ATTACK_ONLY) break;  // do_turn
      }
    }
    if (do_move) {
      // Creature has attempted to move on player?
      if (newy == py && newx == px) {
        mon_attack(midx);
        do_move = FALSE;
        break;  // do_turn
      }
      // Creature is attempting to move on other creature?
      else if (c_ptr->midx && c_ptr->midx != midx) {
        if ((cmove & CM_EATS_OTHER) &&
            cr_ptr->mexp >= creatureD[c_ptr->midx].mexp) {
          *rc_ptr |= CM_EATS_OTHER;
          mon_unuse(&entity_monD[c_ptr->midx]);
          c_ptr->midx = 0;
        } else
          do_move = FALSE;
      }
    }
    // Creature has been allowed move.
    if (do_move) {
      if ((cmove & CM_PICKS_UP) != 0 && mpickup_obj(obj)) {
        int lit = 0;
        if (los(py, px, newy, newx)) lit = mon_desc(midx);
        if (lit) {
          MSG("%s picks up an object.", descD);
          *rc_ptr |= CM_PICKS_UP;
        }
        delete_object(newy, newx);
      }
      // Move creature record
      move_rec(fy, fx, newy, newx);
      m_ptr->fy = newy;
      m_ptr->fx = newx;
      break;  // do_turn
    }
  }
}
STATIC void
mon_breath_msg(breath)
{
  char* name = "";
  switch (breath) {
    case GF_LIGHTNING:
      name = "lightning";
      break;
    case GF_POISON_GAS:
      name = "gas";
      break;
    case GF_ACID:
      name = "acid";
      break;
    case GF_FROST:
      name = "frost";
      break;
    case GF_FIRE:
      name = "fire";
      break;
  }
  MSG("%s breathes %s.", descD, name);
}
STATIC void
mon_breath_dam(midx, fy, fx, breath, breath_maxdam)
{
  int harm_type;
  uint32_t weapon_type;
  struct caveS* c_ptr;
  struct monS* m_ptr;
  struct creatureS* cr_ptr;

  mon_breath_msg(breath);

  int y = uD.y;
  int x = uD.x;

  if (!replay_playback()) {
    viz_hookD = viz_magick;
    magick_distD = 2;
    magick_locD = (point_t){x, y};
    magick_hituD = 1;
  }

  if (!uvow(VOW_DBREATH)) {
    uint32_t cdis = distance(y, x, fy, fx);
    int reduce = 0;
    while (cdis) reduce = bit_pos(&cdis);
    reduce += 1;
    breath_maxdam /= reduce;
  }
  breath_maxdam += (breath_maxdam == 0);

  get_flags(breath, &weapon_type, &harm_type);
  for (int i = y - 2; i <= y + 2; i++) {
    for (int j = x - 2; j <= x + 2; j++) {
      if (in_bounds(i, j) && distance(y, x, i, j) <= 2 && los(y, x, i, j)) {
        c_ptr = &caveD[i][j];
        if ((c_ptr->oidx != 0) &&
            is_obj_vulntype(&entity_objD[c_ptr->oidx], breath)) {
          if (c_ptr->fval == FLOOR_OBST) c_ptr->fval = FLOOR_CORR;
          delete_object(i, j);
        }
        if (c_ptr->fval <= MAX_OPEN_SPACE) {
          if (c_ptr->midx != midx) {
            m_ptr = &entity_monD[c_ptr->midx];
            cr_ptr = &creatureD[m_ptr->cidx];
            int blast_dam = breath_maxdam;
            if (harm_type & cr_ptr->cdefense)
              blast_dam *= 2;
            else if (weapon_type & cr_ptr->spells)
              blast_dam /= 4;
            m_ptr->hp = m_ptr->hp - blast_dam;

            // Player does not get credit for the kill
            // - no loot drops
            // - no exp
            // - cannot win the game this way
            m_ptr->msleep = 0;
            if (m_ptr->hp < 0) {
              mon_unuse(m_ptr);
              c_ptr->midx = 0;
            }
          }
        }
      }
    }
  }

  int dam = 0;
  switch (breath) {
    case GF_LIGHTNING:
      dam = light_dam(breath_maxdam);
      break;
    case GF_POISON_GAS:
      dam = poison_gas(breath_maxdam);
      break;
    case GF_ACID:
      dam = acid_dam(breath_maxdam, TRUE);
      break;
    case GF_FROST:
      dam = frost_dam(breath_maxdam);
      break;
    case GF_FIRE:
      dam = fire_dam(breath_maxdam);
      break;
  }
  if (HACK) MSG("Breath Damage (-%d)", dam);
}
STATIC void mon_try_multiply(mon) struct monS* mon;
{
  int i, j, k;

  k = 0;
  for (i = mon->fy - 1; i <= mon->fy + 1; i++)
    for (j = mon->fx - 1; j <= mon->fx + 1; j++)
      if (caveD[i][j].midx) k++;

  if ((k < 4) && (randint((k + 1) * MON_MULT_ADJ) == 1)) mon_multiply(mon);
}
STATIC int
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

      // Insane
      thrown_spell = spell_choice[randint(k) - 1];
      spell_index = thrown_spell - 4;
      ++thrown_spell;

      int mlit = mon_desc(midx);

      // -1: no message for drain mana
      if (spell_index < AL(mon_spell_nameD) - 1) {
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
          summon_monster(uD.y, uD.x);
          break;
        case 15: /*Summon Undead*/
          summon_undead(uD.y, uD.x);
          break;
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
            if (mlit) {
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
          mon_breath_dam(midx, mon->fy, mon->fx, GF_LIGHTNING,
                         (mon->hp >> (1 + maxlev)));
          break;
        case 21: /*Breath Gas   */
          if (HACK) MSG("[%d]", mon->hp);
          mon_breath_dam(midx, mon->fy, mon->fx, GF_POISON_GAS,
                         (mon->hp >> (1 + maxlev)));
          break;
        case 22: /*Breath Acid   */
          if (HACK) MSG("[%d]", mon->hp);
          mon_breath_dam(midx, mon->fy, mon->fx, GF_ACID,
                         (mon->hp >> (1 + maxlev)));
          break;
        case 23: /*Breath Frost */
          if (HACK) MSG("[%d]", mon->hp);
          mon_breath_dam(midx, mon->fy, mon->fx, GF_FROST,
                         (mon->hp >> (1 + maxlev)));
          break;
        case 24: /*Breath Fire   */
          if (HACK) MSG("[%d]", mon->hp);
          mon_breath_dam(midx, mon->fy, mon->fx, GF_FIRE,
                         (mon->hp >> (1 + maxlev)));
          break;
        default:
          MSG("%s cast unknown spell.", descD);
      }
      if (mlit) {
        int csflag = chance | (1 << (thrown_spell - 1));
        recallD[mon->cidx].r_spells |= csflag;
      }
    }
  }
  return took_turn;
}
// Returns true if make_move() is attempted
STATIC int
mon_move(midx)
{
  struct monS* m_ptr;
  struct creatureS* cr_ptr;
  struct caveS* c_ptr;
  int mm[9];
  int took_turn, random, flee;
  int cdis;
  uint32_t rcmove = 0;

  m_ptr = &entity_monD[midx];
  cr_ptr = &creatureD[m_ptr->cidx];
  c_ptr = &caveD[m_ptr->fy][m_ptr->fx];
  if (c_ptr->fval >= MIN_WALL) {
    if ((cr_ptr->cmove & CM_PHASE) == 0) {
      if (mon_take_hit(midx, damroll(8, 8))) {
        msg_print("You hear a scream muffled by rock!");
        py_experience();
      } else {
        msg_print("A creature digs itself out from the rock!");
        twall(m_ptr->fy, m_ptr->fx);
      }
      return 1;
    }
    rcmove |= CM_PHASE;
  }

  AC(mm);
  took_turn = FALSE;
  random = FALSE;
  flee = FALSE;
  cdis = distance(uD.y, uD.x, m_ptr->fy, m_ptr->fx);
  if ((cr_ptr->cmove & CM_MULTIPLY) && cdis <= cr_ptr->aaf)
    mon_try_multiply(m_ptr);
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
    } else if ((cr_ptr->cmove & CM_MOVE_NORMAL)) {
      rcmove |= CM_MOVE_NORMAL;
      if (randint(200) == 1) random = TRUE;
    }

    if (flee) {
      get_moves(midx, mm);
      mm[0] = 10 - mm[0];
      mm[1] = 10 - mm[1];
      mm[2] = 10 - mm[2];
      mm[3] = randint(9); /* May attack only if cornered */
      mm[4] = randint(9);
    } else if (random) {
      rcmove |= (cr_ptr->cmove & CM_RANDOM_MOVE);
      for (int it = 0; it < 5; ++it) {
        mm[it] = randint(9);
      }
    } else if (cdis < 2 || (cr_ptr->cmove & CM_ATTACK_ONLY) == 0) {
      if (cr_ptr->cmove & CM_ONLY_MAGIC)
        ST_INC(recallD[m_ptr->cidx].r_attack[0]);
      get_moves(midx, mm);
    } else {
      rcmove |= (cr_ptr->cmove & CM_ATTACK_ONLY);
    }

    if (mm[0]) {
      make_move(midx, mm, &rcmove);
      return 1;
    }
  }
  if (rcmove) recallD[m_ptr->cidx].r_cmove |= rcmove;
  return 0;
}
STATIC int
movement_rate(speed)
{
  if (speed <= 0) {
    return ((turnD % (2 - speed)) == 0);
  }

  return speed;
}
STATIC int
creature_movement(int* l_act)
{
  FOR_EACH(mon, {
    if (TEST_CREATURE && l_act[it_index] >= 0) {
      struct creatureS* cr_ptr = &creatureD[mon->cidx];
      printf("%d: %s mon_move %d | msleep %d\n", it_index, cr_ptr->name,
             l_act[it_index], mon->msleep);
    }

    for (int act = l_act[it_index]; act > 0; --act) {
      mon_move(it_index);
    }
  });
}
STATIC int
creature_threat(int* l_act)
{
  int threat = 0;
  // Tag relevant threats after movement is complete
  FOR_EACH(mon, {
    if (l_act[it_index] >= 0) {
      threat += mon_lit(it_index);
    }
  });

  return threat;
}
STATIC int
creatures()
{
  // Default threat to ON for draw() calls inside mon_move()
  int seen_threat = 1;
  int l_act[AL(monD)] = {0};
  int l_wake[AL(monD)] = {0};
  if (TEST_CREATURE && !replay_playback()) printf("----turn----\n");

  // We calculate monster move_count & msleep prior to mon_move
  // This enables storing player state into registers
  {
    int adj_speed = py_speed() + pack_heavy;
    MUSE(u, y);
    MUSE(u, x);
    MUSE(u, stealth);
    int aggr = py_tr(TR_AGGRAVATE);

    FOR_EACH(mon, {
      int move_count = movement_rate(mon->mspeed + adj_speed);
      int msleep = mon->msleep;
      if (msleep) {
        struct creatureS* cr_ptr = &creatureD[mon->cidx];
        int mlit = mon_lit(it_index);
        int cdis = distance(y, x, mon->fy, mon->fx);

        // Monster area of affect
        if (mlit || cdis <= cr_ptr->aaf) {
          if (aggr) msleep = 0;
          // Chance to wake per move
          while (msleep && move_count) {
            uint32_t notice = randint(1024);
            if (notice * notice * notice <= (1 << (29 - stealth))) {
              msleep = MAX(msleep - (100 / cdis), 0);
            }
            if (msleep) --move_count;
          }
        }
        if (TEST_CREATURE && !replay_playback() && cdis < MAX_SIGHT)
          printf("local %s #%d | %d->%d msleep | %d mlit", cr_ptr->name,
                 it_index, mon->msleep, msleep, mlit);
        if (TEST_CREATURE && msleep == 0) l_wake[it_index] = 1;
        if (msleep == 0) ST_INC(recallD[mon->cidx].r_wake);
        mon->msleep = msleep;
      }

      // Dance monster dance!
      l_act[it_index] = msleep ? -1 : move_count;
    });
  }

  if (TEST_CREATURE) {
    FOR_EACH(mon, {
      if (l_wake[it_index]) {
        struct creatureS* cr_ptr = &creatureD[mon->cidx];
        printf("%d: %s wakes with %d/%d move_count\n", it_index, cr_ptr->name,
               l_act[it_index],
               movement_rate(mon->mspeed + py_speed() + pack_heavy));
      }
    });
  }

  creature_movement(l_act);
  seen_threat = creature_threat(l_act);

  // only interrupt run on changes in threat
  // this allows the player use run away from a monster
  if (seen_threat > find_threatD) find_flagD = 0;
  if (seen_threat) countD.rest = 0;
  find_threatD = seen_threat;

  return seen_threat;
}
STATIC void hit_trap(y, x, uy, ux) int *uy, *ux;
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
  find_flagD = FALSE;

  dam = obj->damage[1] ? pdamroll(obj->damage) : 0;

  death_obj(obj);
  switch (obj->subval) {
    case 1: /* Open pit*/
      msg_print("You fall into a pit!");
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
      msg_print("You fall into a covered pit.");
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
      break;
  }
}
STATIC int
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
STATIC int
store_value(sidx, obj_value, pawn)
{
  int cadj, radj, iadj;
  struct ownerS* owner;

  owner = &ownerD[storeD[sidx]];

  cadj = chr_adj();
  radj = rgold_adjD[owner->owner_race][uD.ridx];
  iadj = owner->min_inflate;

  if (pawn) {
    cadj = (200 - chr_adj());
    radj = (200 - radj);
    iadj = MAX(200 - iadj, 1);
  }

  // Use a 64-bit range when scaling; narrow to int on return
  return CLAMP((int64_t)obj_value * cadj * radj * iadj / (int)1e6, 0LL, 99999);
}
STATIC int
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
  // TV_NONE (0)
  // TV_MISC
  // TV_CHEST
  return -1;
}
STATIC void
inven_pawn(iidx)
{
  struct objS* obj;
  struct treasureS* tr_ptr;
  int sidx, count, cost;

  obj = obj_get(invenD[iidx]);
  tr_ptr = &treasureD[obj->tidx];
  sidx = obj_store_index(obj);
  if (sidx >= 0) {
    cost = store_value(sidx, obj_value(obj), 1);
    count = (cost == 0 || STACK_PROJECTILE & obj->subval) ? obj->number : 1;
    tr_make_known(tr_ptr);
    obj->idflag = ID_REVEAL;
    obj_desc(obj, count);
    inven_destroy_num(iidx, count);
    if (cost == 0) {
      MSG("You donate %s.", descD);
    } else {
      uD.gold += cost;
      MSG("You sell %s for %d gold.", descD, cost);
    }
    msg_pause();
  }
}
STATIC void
pawn_display()
{
  USE(overlay_width);
  struct objS* obj;
  int sidx;
  int limitw = MIN(overlay_width, 80);
  int descw = 4;

  apspace(AB(overlayD));
  int line = 0;
  for (int it = 0; it < INVEN_EQUIP; ++it) {
    int len = 1;
    obj = obj_get(invenD[it]);
    sidx = obj_store_index(obj);
    if (sidx >= 0) {
      len = limitw;
      obj_desc(obj, obj->number);
      int dlen = obj_detail(obj, FMT_PAWN);

      overlayD[line][0] = '(';
      overlayD[line][1] = 'a' + it;
      overlayD[line][2] = ')';
      overlayD[line][3] = ' ';
      memcpy(overlayD[line] + descw, AP(descD));
      memcpy(overlayD[line] + limitw - dlen - 1, detailD, dlen);
    }

    overlay_usedD[line] = len;
    line += 1;
  }
}
STATIC void
store_display(sidx)
{
  USE(overlay_width);
  int limitw = MIN(overlay_width, 80);
  int descw = 4;

  apspace(AB(overlayD));
  int line = 0;
  for (int it = 0; it < AL(store_objD[0]); ++it) {
    int len = 1;
    struct objS* obj = &store_objD[sidx][it];
    if (obj->tidx) {
      len = limitw;
      obj_desc(obj, obj->subval & STACK_PROJECTILE ? obj->number : 1);
      int dlen = obj_detail(obj, FMT_SHOP);

      overlayD[line][0] = '(';
      overlayD[line][1] = 'a' + it;
      overlayD[line][2] = ')';
      overlayD[line][3] = ' ';
      memcpy(overlayD[line] + descw, AP(descD));
      memcpy(overlayD[line] + limitw - dlen - 1, detailD, dlen);
    }

    overlay_usedD[line] = len;
    line += 1;
  }
}
STATIC void
store_item_purchase(sidx, item)
{
  int iidx, count, cost, flag;
  struct objS* obj;
  struct treasureS* tr_ptr;

  flag = FALSE;
  obj = &store_objD[sidx][item];
  if (obj->tidx) {
    count = obj->subval & STACK_PROJECTILE ? obj->number : 1;
    cost = store_value(sidx, obj_value(obj), 0);
    if (HACK) cost = 0;
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
      MSG("You buy %s for %d gold (%c).", descD, cost, iidx + 'a');
      if (obj->number != count) MSG("You have %d.", obj->number);
      store_item_destroy(sidx, item, count);
    }
    msg_pause();
  }
}
STATIC void
pawn_entrance()
{
  char c;

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
}
STATIC void
sauna_display()
{
  USE(overlay_width);

  apspace(AB(overlayD));
  apclear(AB(overlay_usedD));
  int line = 0;
  for (int it = 0; it < MAX_A; ++it) {
    int used = 0;
    overlayD[line][used++] = '(';
    overlayD[line][used++] = 'a' + it;
    overlayD[line][used++] = ')';
    overlayD[line][used++] = ' ';

    if (low_stat(1 << it)) {
      char* stat = stat_nameD[it];
      while (*stat && used < overlay_width) overlayD[line][used++] = *stat++;
    }
    overlay_usedD[line] = used;
    line += 1;
  }
}
STATIC void
sauna_entrance()
{
  int cost = 550 * chr_adj() / 100;
  char c;
  char tmp[128];

  while (1) {
    sauna_display();

    snprintf(AP(tmp), "What stat would you restore? [%d gp]", cost);
    if (!in_subcommand(tmp, &c)) {
      break;
    }

    if (is_lower(c)) {
      uint8_t item = c - 'a';
      int r = 0;
      if (item < MAX_A) {
        if (res_stat(item)) r = 1;
      }
      if (r) {
        msg_pause();
        uD.gold -= cost;
      }
    }
  }
}
STATIC void
vow_display()
{
  USE(overlay_width);

  apspace(AB(overlayD));
  apclear(AB(overlay_usedD));
  int line = 0;
  for (int it = 0; it < AL(vowD); ++it) {
    int flag = 1 << it;
    int on = flag & uD.vow_flag;
    int used = 0;
    overlayD[line][used++] = '(';
    overlayD[line][used++] = 'a' + it;
    overlayD[line][used++] = ')';
    overlayD[line][used++] = ' ';
    overlayD[line][used++] = '[';
    overlayD[line][used++] = 'o';
    if (on) {
      overlayD[line][used++] = 'n';
    } else {
      overlayD[line][used++] = 'f';
      overlayD[line][used++] = 'f';
    }
    overlayD[line][used++] = ']';
    overlayD[line][used++] = ' ';
    memcpy(overlayD[line] + used, vowD[it], overlay_width - used);
    overlay_usedD[line] = overlay_width;
    line += 1;
  }
}
STATIC void
vow_entrance()
{
  char c;

  while (1) {
    vow_display();

    if (!in_subcommand("Newcomer, what vows do you keep?", &c)) {
      break;
    }

    if (is_lower(c)) {
      uint8_t item = c - 'a';
      if (item < 8) uD.vow_flag ^= (1 << item);
    }
  }
}
STATIC void
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
}
STATIC void
town_entrance(tval)
{
  overlay_submodeD = tval;

  switch (tval) {
    case '0':
      pawn_entrance();
      break;
    case '8':
      if (low_stat(-1))
        sauna_entrance();
      else
        msg_print("The sauna is closed.");
      break;
    case '9':
      vow_entrance();
      break;
    default:
      store_entrance(tval - '1');
      break;
  }

  // this eliminates replay_hack() calls on sort, purchase, and sell
  turn_flag = 1;
}
STATIC void yx_autoinven(y_ptr, x_ptr, iidx) int *y_ptr, *x_ptr;
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
STATIC void py_reactuate(y_ptr, x_ptr, obj_id) int *y_ptr, *x_ptr;
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
STATIC void py_actuate(y_ptr, x_ptr, submode) int *y_ptr, *x_ptr;
{
  USE(last_actuate);
  USE(last_cast);

  overlay_submodeD = submode;
  while (!turn_flag) {
    last_actuateD = last_actuate;
    last_castD = last_cast;
    int iidx = inven_choice("Use which item?",
                            overlay_submodeD == 'e' ? "/*0" : "*/0");
    if (iidx == -1) break;

    last_actuateD = invenD[iidx];
    last_castD = 0;
    yx_autoinven(y_ptr, x_ptr, iidx);
  }
}
STATIC void
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
STATIC void
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
STATIC void
player_maint()
{
  inven_sort();
  if (!uvow(VOW_FOREGO_ID)) {
    if (inven_reveal())
      msg_print("Town inhabitants share knowledge of items you gathered.");
  }

  if (!uvow(VOW_STAT_FEE)) {
    if (low_stat(-1)) {
      msg_print("A wind from the misty mountains renews your being.");
      for (int it = 0; it < MAX_A; ++it) res_stat(it);
    }
  }

  for (int sidx = 0; sidx < MAX_STORE; ++sidx) {
    for (int it = 0; it < MAX_STORE_INVEN; ++it) {
      if (store_objD[sidx][it].sn) {
        MSG("Rumor has it, a rare item is for sale by (%d) %s.", sidx + 1,
            ownerD[storeD[sidx]].name);
        break;
      }
    }
  }
}
STATIC void
ma_tick()
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
    if (MA_VIEW & delta) py_check_view();
  }
}
STATIC void
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
    death_desc("starvation");
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
      death_desc("poison");
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
STATIC int
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
STATIC void
fail(char* text)
{
  puts(text);
  exit(0);
}
STATIC void
dungeon()
{
  int y, x;
  int town;
  int check_replay;
  int teleport = 0;

  town = (dun_level == 0);
  uD.max_dlv = MAX(uD.max_dlv, dun_level);

  if (town)
    snprintf(dun_descD, AL(dun_descD), "%s", "town square");
  else
    snprintf(dun_descD, AL(dun_descD), "%d feet", dun_level * 50);

  switch (uD.new_level_flag) {
    default:
      msg_print("A golden sun rises above the misty mountains.");
      break;
    case NL_DOWN_STAIR:
      msg_print("You pass through a maze of down staircases.");
      break;
    case NL_UP_STAIR:
      msg_print("You pass through a maze of up staircases.");
      break;
    case NL_RECALL:
      if (town) {
        msg_print("You are yanked upwards!");
      } else {
        msg_print("You are yanked downwards!");
      }
      break;
    case NL_TRAP:
      msg_print("You fall, landing hard on the ground!");
      break;
    case NL_MIDPOINT_LOST:
      if (mplostD & 1)
        msg_print("Game version updated, dungeon reset.");
      else
        MSG("Midpoint resume flags: 0x%x"
            " send a screenshot to moria@rufe.org",
            mplostD);
      mplostD = 0;
      break;
  }
  uD.new_level_flag = 0;
  if (town) player_maint();

  do {
    int last_action = 0;
    if (replay_flag && replayD->input_action_usedD > 0)
      last_action = AS(replayD->input_actionD, replayD->input_action_usedD - 1);
    inven_check_weight();
    inven_check_light();

    if (TEST_REPLAY) check_replay = rnd_seed;
    turn_flag = (countD.rest != 0) || (countD.paralysis != 0);
    if (teleport) turn_flag = 0;
    while (!turn_flag) {
      int winner = (uD.total_winner == 1);  // once
      uD.total_winner += winner;

      if (!replay_playback()) {
        if (winner) py_endgame(py_king);
        if (TEST_REPLAY) replay_seedcmp(check_replay);
        if (replay_recording())
          replayD->input_record_readD = replayD->input_record_writeD =
              last_action;
      }
      if (replay_resumed()) replay_flag = REPLAY_RECORD;
      draw(DRAW_NOW);

      y = uD.y;
      x = uD.x;
      if (teleport) {
        if (equip_vibrate(TR_TELEPORT)) py_teleport(40, &y, &x);
      } else if (find_flagD) {
        mmove(find_directionD, &y, &x);
      } else {
        // not running, not paralysed, not resting; what do we DO?
        char c = game_input();
        if (!replay_playback() && (c != CTRL('d') && c != ' '))
          last_turnD = turnD;
        if (TEST_REPLAY && !is_ctrl(c)) Log("execute (%c:%d)\n", c, c);

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
              town_entrance(tchar);
              break;
            default:
              c = 's';
              break;
          }
        }

        // (jhklnbyuJHKLNBYU)
        int dir = map_roguedir(c);
        if (dir > 0 && dir != 5) {
          // 75% random movement
          if (countD.confusion && randint(4) > 1) {
            dir = dir_by_confusion();
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
          if (KEYBOARD) {
            switch (c) {
              case ESCAPE:
                if (confirm_transition(c, "Advanced Game Actions")) c = '@';
                break;
              case '?':
                py_help();
                break;
              case 'd':
                py_drop();
                break;
              case 'e':
                py_actuate(&y, &x, 'e');
                break;
              case 'w':
                py_offhand();
                break;
              case '0':
              case 'x':
                py_examine();
                break;
              case 'i':
                py_actuate(&y, &x, 'i');
                break;
              case 'v':
                show_version();
                break;
              case 'M':
                if (dun_level) py_where_on_map();
                break;
              case 'R':
                py_rest();
                break;
            }
          }

          switch (c) {
            case '-':
              globalD.zoom_factor = (globalD.zoom_factor - 1) % MAX_ZOOM;
              break;
            case '!':
              py_reactuate(&y, &x, last_actuateD);
              break;
            case '@':
              py_menu();
              break;
            case ',':
              py_pickup(y, x, TRUE);
              break;
            case 's':
              msg_print("You search the area.");
              py_search(y, x);
              break;
            case '<':
              go_up();
              break;
            case '>':
              go_down();
              break;
            case 'a':
              // Generalized inventory interaction
              py_actuate(&y, &x, 'i');
              break;
            case 'c':
              show_character();
              break;
            case 'm':
              if (maD[MA_BLIND] == 0) {
                blipD = 1;
                minimap_enlargeD = TRUE;
                CLOBBER_MSG("You check your dungeon map.");
                minimap_enlargeD = FALSE;
              }
              break;
            case 'O':
              py_look(ylookD, xlookD);
              break;
            case CTRL('c'):
              death_desc(quit_stringD);
              uD.new_level_flag = NL_DEATH;
              return;  // Interrupt game
            case 'p':
              show_history();
              break;
            case CTRL('w'):
              if (HACK)
                wizard_prompt(&y, &x);
              else
                py_menu();
              break;
            case CTRL('x'):
              if (HACK) {
                death_desc(quit_stringD);
                uD.new_level_flag = NL_DEATH;
                return;  // Interrupt game
              }
              break;
            case CTRL('z'):
              py_undo();
              break;
          }
        }
      }

      if (uD.y != y || uD.x != x) {
        struct caveS* c_ptr = &caveD[y][x];
        struct monS* mon = &entity_monD[c_ptr->midx];
        struct objS* obj = &entity_objD[c_ptr->oidx];

        if (find_flagD && mon_lit(c_ptr->midx)) {
          // Run is non-combat movement
          find_flagD = FALSE;
        } else {
          // doors known to be jammed are bashed prior to movement
          if (obj->tval == TV_CLOSED_DOOR) {
            if (obj->p1 < 0 && (obj->idflag & ID_REVEAL)) {
              // may mutate cave
              door_bash(y, x);
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
            if (py_affect(MA_BLIND) == 0 && find_flagD) {
              int evcode = find_event(y, x);
              if (evcode) {
                if (TEST_REPLAY) Log("find_event %d", evcode);
                find_flagD = FALSE;
              }
            }

            if (obj->tval == TV_CHEST && obj->sn != SN_EMPTY) {
              open_object(y, x);
            } else if (obj->tval == TV_STORE_DOOR) {
              town_entrance(obj->tchar);
            } else if (oset_pickup(obj)) {
              py_pickup(y, x, FALSE);
            }
          } else if (obj->tval == TV_CLOSED_DOOR) {
            open_object(y, x);
          } else if (py_affect(MA_BLIND) == 0 && obj->tval == TV_RUBBLE) {
            tunnel(y, x);
          } else {
            if (TEST_REPLAY) Log("find_flagD = FALSE");
            find_flagD = FALSE;
          }
          panel_update(&panelD, uD.y, uD.x, FALSE);
        }
      }
    }

    if (replay_flag) {
      if (replayD->input_action_usedD < AL(replayD->input_actionD) &&
          last_action != replayD->input_record_readD) {
        AS(replayD->input_actionD, replayD->input_action_usedD++) =
            replayD->input_record_readD;
        // show(replayD->input_action_usedD);
      }
    }

    ma_tick();  // rising
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
    ma_tick();  // falling
    tick();     // uD.new_level_flag may change (player dies from poison)
    turnD += 1;
    if (TEST_REPLAY) replay_memcmp();
  } while (!uD.new_level_flag);
  msg_pause();
}
STATIC void
mon_level_init()
{
  int i, k;

  memset(m_level, 0, sizeof(m_level));

  k = AL(creatureD) - MAX_WIN_MON;
  for (i = 1; i < k; i++) m_level[creatureD[i].level]++;

  for (i = 1; i <= MAX_MON_LEVEL; i++) m_level[i] += m_level[i - 1];
}
STATIC void
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
STATIC int
ftable_clear(void* ftable, int size)
{
  fn* func = ftable;
  for (int it = 0; it < size; ++it) func[it] = noop;
  return 0;
}
// try to be FUN on the platform with defaults
STATIC int
global_init(int argc, char** argv)
{
  ftable_clear(&platformD, sizeof(platformD) / sizeof(fn));
  globalD.saveslot_class = -1;
  globalD.zoom_factor = PC ? 0 : 2;
  globalD.vsync = 0;
  globalD.sprite = 1;
  globalD.dpad_sensitivity = 75;
  globalD.dpad_color = 1;
  globalD.small_text = 0;
  globalD.use_joystick = PC ? 1 : 0;
  globalD.power_mode = 0;
  globalD.gpu_bypass = 0;

  globalD.ghash = djb2(DJB2, bptr(&globalD) + sizeof(globalD.ghash),
                       sizeof(globalD) - sizeof(globalD.ghash));
  msg_widthD = overlay_widthD = 80;
}
#include "platform/platform.c"
int
main(int argc, char** argv)
{
  global_init(argc, argv);
  platform_init();

  if (platformD.pregame() == 0) {
    mon_level_init();
    obj_level_init();

    if (TEST_CHECKLEN) return obj_checklen();

    setjmp(restartD);
    hard_reset();

    int ready = platformD.load(globalD.saveslot_class, 0);
    if (!ready) ready = py_saveslot_select();

    if (TEST_CAVEGEN) ready = test_cavegen();

    if (ready) {
      globalD.saveslot_class = uD.clidx;
      platformD.mmap_replay(&replayD);
      // show(replayD->input_action_usedD);
      if (RESEED && seed_changeD) {
        rnd_seed += 1;
        seed_changeD = 0;
      }
      if (save_on_readyD) {
        save_on_readyD = 0;
        platformD.save(globalD.saveslot_class);
      }

      // Per-Player initialization
      fixed_seed_func(obj_seed, magic_init);
      fixed_seed_func(town_seed, heroname_init);
      // recreate history text
      fixed_seed_func(town_seed, social_bonus);

      // Replay state reset
      replay_start();

      // Input reset (values are initialized by ui_stateD on first interaction)
      modeD = submodeD = 0;
      finger_rowD = finger_colD = 0;

      // may generate messages in calc_mana()->gain_prayer()
      for (int it = 0; it < MAX_A; ++it) {
        // Perform calculations
        set_use_stat(it);
      }

      if (!platformD.monster_memory(AB(recallD), 0)) AC(recallD);

      // Release objects in the cave
      FOR_EACH(obj, {
        if (obj->tval > TV_MAX_PICK_UP || obj->fx || obj->fy) {
          obj_unuse(obj);
        }
      });
      memset(&entity_objD[0], 0, sizeof(entity_objD[0]));

      // a fresh cave!
      if (dun_level != 0) {
        cave_gen();
      } else {
        // Store rotation
        store_maint();
        // Generate town
        fixed_seed_func(town_seed, town_gen);
        // Random town monsters
        alloc_townmon(randint(RND_MALLOC_LEVEL) + MIN_MALLOC_TOWN);
        // Player random placement
        py_intown();
      }

      panel_update(&panelD, uD.y, uD.x, TRUE);
      py_check_view();
      dungeon();

      if (uD.new_level_flag != NL_DEATH) {
        platformD.monster_memory(AB(recallD), 1);
        countD.pundo += (replayD->input_mutationD != 0);
        if (platformD.save(globalD.saveslot_class)) {
          longjmp(restartD, 1);
        } else {
          death_desc("Device I/O Error");
        }
      }
    } else if (!death_descD[0]) {
      death_desc("Initialization Error");
    }

    if (memcmp(death_descD, AP(quit_stringD)) != 0) {
      replay_end();
      countD.pdeath += 1;
      while (1) {
        py_endgame(py_grave);
        if (py_menu() == CTRL('c')) break;
      }
    }
  }

  return platformD.postgame(1);
}
