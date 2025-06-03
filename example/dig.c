
#define main _alt_main
#include "src/moria_at.c"
#undef main

char* wall_name[] = {
    "granite",
    "magma",
    "quartz",
    "rubble",
};
int wall_chance[] = {1200, 600, 400, 180};
int wall_min[] = {80, 10, 10, 0};
// original:
// 118 str bonus
// 50 per p1
// 25 if tunneling
//
// current:
// no str bonus
// 50 per p1
// 25 if tunneling
// minwall bugs
int
main()
{
  for (int it = 0; it < AL(treasureD); ++it) {
    if (treasureD[it].tval == TV_DIGGING)
      printf("%s %dlbs (%dd%d) (+%d)\n", treasureD[it].name,
             treasureD[it].weight / 10, treasureD[it].damage[0],
             treasureD[it].damage[1], treasureD[it].p1);
  }

  for (int it = 0; it < AL(wall_chance); ++it) {
    int t = wall_chance[it];
    char* n = wall_name[it];
    printf("%s %.03f per p1 %.03f base tunneling %.03f str bonus (no impl)\n",
           n, 50.f / t, 25.f / t, 118.f / t);
  }
  return 0;
}
