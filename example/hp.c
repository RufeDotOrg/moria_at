
#define main _alt_main
#include "src/moria_at.c"
#undef main

int
main()
{
  for (int it = 0; it < AL(classD); ++it) {
    printf("%s: %d\n", classD[it].name, classD[it].adj_hd);
  }
  for (int it = 0; it < AL(raceD); ++it) {
    printf("%s: %d\n", raceD[it].name, raceD[it].bhitdie);
  }

  return 0;
}
