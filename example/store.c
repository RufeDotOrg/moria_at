
#define main _main
#include "src/animal_run.c"
#undef main

int
usage()
{
  puts("store <index>");
  return 1;
}

int
main(int argc, char** argv)
{
  if (argc != 2) return usage();

  uint8_t sidx = strtoull(argv[1], 0, 10);
  if (sidx < AL(store_choiceD)) {
    for (int it = 0; it < AL(store_choiceD[0]); ++it) {
      int tidx = store_choiceD[sidx][it];
      printf("(%03d) [%d] %s\n", tidx, store_stockD[sidx][it],
             treasureD[tidx].name);
    }
  }
  return 0;
}
