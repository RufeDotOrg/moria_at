
static int checkD[MAX_HEIGHT][MAX_WIDTH];
static void
dfs(y, x)
{
  if (in_bounds(y, x)) {
    if (caveD[y][x].fval <= MAX_FLOOR && !checkD[y][x]) {
      checkD[y][x] = 1;
      for (int row = y - 1; row <= y + 1; ++row) {
        for (int col = x - 1; col <= x + 1; ++col) {
          dfs(row, col);
        }
      }
    }
  }
}

static int
cave_check(y, x)
{
  printf("cave_check %d %d xy\n", x, y);
  memset(checkD, 0, sizeof(checkD));
  dfs(y, x);
  int count = 0;
  int fail = 0;
  point_t first;
  for (int row = 0; row < MAX_HEIGHT; ++row) {
    for (int col = 0; col < MAX_WIDTH; ++col) {
      count += caveD[row][col].fval <= MAX_FLOOR;
      struct caveS* c_ptr = &caveD[row][col];
      if (c_ptr->fval <= MAX_FLOOR && !checkD[row][col]) {
        if (fail == 0) first.x = col;
        if (fail == 0) first.y = row;
        fail += 1;
      }
    }
  }

  printf("cave_check dfs fail_count? %d - %d floor tiles %d %d\n", fail, count,
         V2i(&first));
  return fail == 0;
}
// 0 OK; quit
// 1 fail; load game
int
test_cavegen()
{
  int dlev = 1;
  for (; dlev <= 50; ++dlev) {
    int seed_count = 64 * 1024;
    int pass = 0;
    for (uint32_t it = 0; it < seed_count; ++it) {
      hard_reset();
      dun_level = dlev;
      rnd_seed = it;
      cave_gen();
      pass = cave_check(uD.y, uD.x);
      if (!pass) {
        // Proceed to play for debug
        hard_reset();
        platformD.load(globalD.saveslot_class, 0);
        dun_level = dlev;
        rnd_seed = it;
        input_resumeD = -1;
        dlev = 99;
        return 1;
      }
    }
    printf("test result (%s): dlev %d | seed %d | seed_count %d\n", pass ? "ok" : "fail",
           dun_level, rnd_seed, seed_count);
  }

  if (dlev < 99) printf("ALL dlev OK\n");
  memcpy(death_descD, AP(quit_stringD));

  return 0;
}

#define TEST_CAVEGEN 1
