#define main _game_main
#include "moria_at.c"
#undef main

int
read_number(char* prompt, int* number)
{
  char c;
  char input[18] = {'0'};
  int used = 0;
  int line;
  do {
    line = 0;
    BufMsg(screen, "%s", input);
    DRAWMSG("%s", prompt);
    c = platformD.readansi();
    switch (c) {
      case CTRL('c'):
      case ESCAPE:
        return c;
    }
    input[used++] = c;
    input[used] = 0;
  } while (used < AL(input) - 1 && c != RETURN);
  *number = strtoll(input, 0, 10);
  return c;
}

int
custom_editor()
{
  char* name_list[] = {
      "Base to hit?",
      "Bow to hit?",
      "Search?",
      "Frequency of search?",
      "Disarm?",
      "Stealth?",
      "Save",
      "Infra",
      "Level Exp Multiplier",
      "Weight",
  };
  int* ptr_list[] = {&uD.bth,      &uD.bowth,   &uD.search, &uD.fos,
                     &uD.disarm,   &uD.stealth, &uD.save,   &uD.infra,
                     &uD.mult_exp, &uD.wt};
  char c;
  for (int it = 0; it < AL(name_list); ++it) {
    c = read_number(name_list[it], ptr_list[it]);
    if (c == CTRL('c')) break;
  }
  if (c == AL(name_list)) show_character();
  return c;
}

int
hp_reroll()
{
  struct raceS* r_ptr = &raceD[uD.ridx];
  struct classS* cl_ptr = &classD[uD.clidx];
  int hitdie = r_ptr->bhitdie;
  hitdie += cl_ptr->adj_hd;
  player_hpD[0] = hitdie;

  int min_value = (MAX_PLAYER_LEVEL * 3 / 8 * (hitdie - 1)) + MAX_PLAYER_LEVEL;
  int max_value = (MAX_PLAYER_LEVEL * 5 / 8 * (hitdie - 1)) + MAX_PLAYER_LEVEL;
  do {
    for (int it = 1; it < MAX_PLAYER_LEVEL; it++) {
      player_hpD[it] = randint(hitdie);
      player_hpD[it] += player_hpD[it - 1];
    }
  } while ((player_hpD[MAX_PLAYER_LEVEL - 1] < min_value) ||
           (player_hpD[MAX_PLAYER_LEVEL - 1] > max_value));
  return 0;
}

enum { HP_COL = 2 };
int
hp_editor()
{
  int line;
  int step = MAX_PLAYER_LEVEL / HP_COL;
  char c;
  do {
    line = 0;
    BufMsg(screen, "%d MAX HP = %d base + %d con bonus",
           player_hpD[uD.lev - 1] + con_adj() * uD.lev, player_hpD[uD.lev - 1],
           con_adj() * uD.lev);
    BufMsg(screen, "Constitution bonus: %d/lev", con_adj());
    for (int it = 0; it < MAX_PLAYER_LEVEL / HP_COL; ++it) {
      BufMsg(screen, "level %2d) %3d\t\tlevel %2d) %3d", it + 1, player_hpD[it],
             it + step + 1, player_hpD[it + step]);
    }
    DRAWMSG("HP EDITOR (level %d): SPACEBAR to reroll; ESCAPE to exit", uD.lev);
    c = platformD.readansi();

    if (c == ESCAPE) return c;
    if (c == ' ') {
      int seed = platformD.seed();
      seed_init(seed);
      hp_reroll();
    }
  } while (c != CTRL('c'));
  return c;
}

int
godmode()
{
  for (int it = 0; it < AL(player_hpD); ++it) {
    player_hpD[it] = 1000;
  }
  for (int it = 0; it < MAX_A; ++it) {
    statD.cur_stat[it] = statD.max_stat[it] = 118;
    set_use_stat(it);
  }
  uD.max_exp = uD.exp = MAX_EXP;
  py_experience();
  uD.gold = 9999999;

  uD.bth = 999;
  uD.bowth = 999;
}

int
main()
{
  mon_level_init();
  obj_level_init();

  platform_init();
  platformD.pregame();

  setjmp(restartD);
  hard_reset();

  if (platformD.load("savechar")) {
    char c;
    int line;
    do {
      line = 0;
      BufMsg(screen, "%s %s level %d", raceD[uD.ridx].name,
             classD[uD.clidx].name, uD.lev);
      BufMsg(screen, "a) HP Editor");
      BufMsg(screen, "b) Attribute Editor (str, int, wis, dex, con, chr)");
      BufMsg(screen, "c) Custom Modifiers Editor");
      BufMsg(screen, "d) Dungeon Level Depth");
      BufMsg(screen, "e) Experience");
      BufMsg(screen, "");
      BufMsg(screen, "g) Gold");
      line = 20;
      BufMsg(screen, "w) Wizard Mode to disk");
      BufMsg(screen, "x) Exit writing changes to disk");

      DRAWMSG("Modify Character?");
      c = platformD.readansi();
      switch (c) {
        case 'a':
          c = hp_editor();
          break;
        case 'b': {
          for (int it = 0; it < MAX_A; ++it) {
            int val = 0;
            c = read_number(stat_abbrD[it], &val);
            if (c == CTRL('c')) it = MAX_A;
            if (val != 0) statD.cur_stat[it] = statD.max_stat[it] = val;
          }
        } break;
        case 'c': {
          c = custom_editor();
        } break;
        case 'd': {
          int feet = uD.max_dlv * 50;
          c = read_number("Max dungeon level in feet?", &feet);
          if (feet != uD.max_dlv * 50) {
            uD.max_dlv = feet / 50;
            dun_level = 0;
          }
        } break;
        case 'e': {
          int exp = uD.exp;
          c = read_number("How much experience?", &exp);
          if (exp != uD.exp) uD.exp = uD.max_exp = exp;
          py_experience();
        } break;
        case 'g':
          c = read_number("How much gold?", &uD.gold);
          break;
        case 'w':
          godmode();
        case 'x':
          platformD.save("savechar");
          c = CTRL('c');
          break;
      }
    } while (c != CTRL('c'));

  } else {
    printf("savechar: file not found\n");
  }

  platformD.postgame();

  return 0;
}
