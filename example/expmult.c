
#define main _alt_main
#include "src/moria_at.c"
#undef main

int
main()
{
  int lo = INT32_MAX;
  int hi = 0;
  for (int it = 0; it < AL(raceD); ++it) {
    for (int jt = 0; jt < AL(classD); ++jt) {
      if ((1 << jt) & raceD[it].rtclass) {
        int expmult = raceD[it].b_exp + classD[jt].m_exp;
        printf("%s %s: %d\n", raceD[it].name, classD[jt].name, expmult);
        lo = MIN(lo, expmult);
        hi = MAX(hi, expmult);
      }
    }
  }
  printf("[%d,%d]\n", lo, hi);
  return 0;
}
