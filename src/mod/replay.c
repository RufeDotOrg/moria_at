// Rufe.org LLC 2022-2025: GPLv3 License
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

static void* replaydatD;
static int64_t replay_sizeD;
// nm bin/moria_at | grep 'start_game'
static void* ccoffset = (void*)0x000000000049d880;  // 0x000000000069510;

static void
replay_diverge_size()
{
  USE(replay_size);
  int contig = 0;
  int* lhs = vptr(replaydatD);
  int* rhs = vptr(__start_game);
  for (int it = 0; it < replay_size / sizeof(int); ++it) {
    if (lhs[it] != rhs[it]) {
      if (contig) printf("  ");
      int64_t offset = it * sizeof(int);
      printf("%p (cc %p): 0x%x 0x%x replay v. game\n", (void*)0 + offset,
             ccoffset + offset, lhs[it], rhs[it]);
      if (__start_game + offset > entity_monD &&
          __start_game + offset < AE(entity_monD)) {
        printf("  ^ monster desync\n");
      }
      if (__start_game + offset > entity_objD &&
          __start_game + offset < AE(entity_objD)) {
        printf("  ^ object desync\n");
      }
      if (__start_game + offset > caveD && __start_game + offset < AE(caveD)) {
        printf("  ^ cave desync\n");
      }
      contig += 1;
    } else {
      contig = 0;
    }
  }
  printf("\nCompiler ccoffset: %p\n", ccoffset);
  printf("Start game: %p\n", __start_game);
  printf("Monster range: %p %p\n", entity_monD, AE(entity_monD));
  printf("Object range: %p %p\n", entity_objD, AE(entity_objD));
}

static void
replay_seedcmp(check_seed)
{
  printf("dropped %d inputs: ", replayD->input_record_writeD - last_action);
  for (int it = last_action; it < replayD->input_record_writeD; ++it) {
    char c = replayD->input_recordD[it];
    if (char_visible(c)) printf("(%c)", c);
    printf("%d ", c);
  }
  printf("\n");
  if (check_seed != rnd_seed) fail("seed moved; turn did not\n");
}
static void
replay_memcmp()
{
  if (!replaydatD) {
    replay_sizeD = __stop_game - __start_game;
    replaydatD = malloc(replay_sizeD);
  }
  USE(replay_size);

  char name[64];
  int len =
      snprintf(AP(name), "record/replay_lev%02d_turn%07d", dun_level, turnD);

  int f = open(name, O_RDONLY, 0777);
  if (f > 0) {
    int64_t count = read(f, replaydatD, replay_size);
    if (count == replay_size) {
      int r = memcmp(replaydatD, __start_game, replay_size);
      if (r != 0) {
        replay_desync = 1;
        printf("DESYNC DETAILS:\n");
        replay_diverge_size();
        puts(name);
        // Halt the replay to see what is happening
        printf("input %d %d read/write\n", replayD->input_record_readD,
               replayD->input_record_writeD);
        replayD->input_record_writeD = replayD->input_record_readD;
        if (replayD->input_action_usedD > 0) {
          int begin =
              AS(replayD->input_actionD, replayD->input_action_usedD - 2);
          int end = replayD->input_record_readD;
          printf("Action range (%d->%d):", begin, end);
          for (int it = begin; it < end; ++it) {
            char c = AS(replayD->input_recordD, it);
            printf(" (%d:%c)", c, c);
          }
        }
        printf("\n");
      }
    }
  } else {
    f = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (f > 0) write(f, __start_game, replay_size);
  }
  if (f > 0) close(f);
}

#define TEST_REPLAY 1
