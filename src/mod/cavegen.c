
DATA int max_loop_count;
enum { LOAD_GAME = 0 };
enum { DLEV_BEGIN = 1 };  // [1, 50] or just N != 1
enum { SEED_BEGIN = 0 };
enum { SEED_RANGE = 64 * 1024 };
// #define logidx 14

DATA int cave_pngD = 0;
DATA int checkD[MAX_HEIGHT][MAX_WIDTH];
DATA int imageD[MAX_HEIGHT][MAX_WIDTH];
DATA int image_usedD;
GAME point_t cave_startD;
GAME point_t cave_endD;
GAME point_t cave_checkD;
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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "third_party/stb/stb_image_write.h"
static int
image_write()
{
  char name[64];
  int len = snprintf(AP(name), "cave_image%03d.png", image_usedD++);
  int r = stbi_write_png(name, MAX_WIDTH, MAX_HEIGHT, 4, imageD,
                         MAX_WIDTH * sizeof(int));
  if (!r) printf("image_write failed\n");
}
static int
cave_image()
{
  for (int row = 0; row < MAX_HEIGHT; ++row) {
    for (int col = 0; col < MAX_WIDTH; ++col) {
      struct caveS* c_ptr = &caveD[row][col];
      int color = 0;
      if (!c_ptr->fval) {
        color = 0;
      } else if (c_ptr->fval <= MAX_FLOOR) {
        color = 50;
      } else {
        switch (c_ptr->fval) {
          case BOUNDARY_WALL:
            color = 100;
            break;
          case GRANITE_WALL:
            color = 90;
            break;
          case MAGMA_WALL:
            color = 80;
            break;
          case QUARTZ_WALL:
            color = 70;
            break;
        }
      }
      imageD[row][col] = rgb_by_labr(color);

      if (row == cave_startD.y && col == cave_startD.x)
        imageD[row][col] = 0xff0000ff;
      if (row == cave_endD.y && col == cave_endD.x)
        imageD[row][col] = 0xffff00ff;
      if (row == cave_checkD.y && col == cave_checkD.x)
        imageD[row][col] = 0xff00ffff;
    }
  }
}
static int
cave_debug()
{
  if (cave_pngD) {
    cave_image();
    image_write();
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
  point_t first = {0, 0};
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

  cave_checkD = first;
  printf("cave_check dfs fail_count? %d - %d floor tiles %d %d\n", fail, count,
         V2i(&first));
  return fail == 0;
}

int
test_cavegen()
{
  int ret = 0;
  int dlev = DLEV_BEGIN;
  int dlev_end = dlev == 1 && SEED_BEGIN == 0 ? 50 : dlev;
  for (; dlev <= dlev_end; ++dlev) {
    int pass = 0;

    int begin = SEED_BEGIN;
    int end = SEED_BEGIN == 0 ? SEED_BEGIN + SEED_RANGE : SEED_BEGIN + 1;
    for (uint32_t seed = begin; seed < end; ++seed) {
      hard_reset();
      dun_level = dlev;
      rnd_seed = seed;
      cave_gen();
      pass = cave_check(uD.y, uD.x);
      if (!pass) {
        if (!LOAD_GAME) {
          hard_reset();
          dun_level = dlev;
          rnd_seed = seed;
          cave_pngD = 1;
          cave_gen();
          cave_check(uD.y, uD.x);
          cave_debug();
          char name[64];
          int len = snprintf(AP(name), "cave_image%03d.png", image_usedD - 1);
          printf("PNG output: dlev %d rnd_seed %d last_image %s\n", dlev, seed,
                 name);

          exit(1);
        }

        // Proceed to play for debug
        hard_reset();
        platformD.load(globalD.saveslot_class, 0);
        dun_level = dlev;
        rnd_seed = seed;
        input_resumeD = -1;
        dlev = dlev_end * 2;
        ret = 1;
        break;
      }
    }
    printf("test result (%s): dlev %d | SEED_RANGE %d\n", pass ? "ok" : "fail",
           dun_level, SEED_RANGE);
  }

  if (dlev < dlev_end * 2) printf("ALL dlev OK\n");
  memcpy(death_descD, AP(quit_stringD));
  printf("max loop count %d\n", max_loop_count);

  return ret && LOAD_GAME;
}

#define TEST_CAVEGEN 1
