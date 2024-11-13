#define main _alt_main
#include "src/moria_at.c"
#undef main

int
exit_usage()
{
  printf("spell <class> <level>\n");
  exit(1);
}

int
idx_by_classname(char* name)
{
  for (int it = 0; it < AL(classD); ++it) {
    if (strcasecmp(name, classD[it].name) == 0) return it;
  }
  return -1;
}

struct spellS*
spelltable(clidx)
{
  return spellD[clidx - 1];
}
int
attr_adj(value)
{
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
main(int argc, char** argv)
{
  if (argc < 2) exit_usage();

  int clidx = idx_by_classname(argv[1]);
  if (clidx < 0) exit_usage();
  printf("class %d: %s\n", clidx, classD[clidx].name);

  int lvl = 0;
  if (argc > 2) lvl = strtoull(argv[2], 0, 10);
  int think_adj = 0;
  if (argc > 3) {
    int int_attr = 0;
    int_attr = strtoull(argv[3], 0, 10);
    think_adj = attr_adj(int_attr);
    printf("%d int = %d attr_adj * 3 = %d\n", int_attr, think_adj,
           3 * think_adj);
  }
  for (int it = 0; it < AL(spellD[0]); ++it) {
    int splevel = spelltable(clidx)[it].splevel;
    int spfail = spelltable(clidx)[it].spfail;
    if (spfail) {
      char adjtext1[8];
      char adjtext2[8];
      memset(adjtext1, ' ', sizeof(adjtext1));
      memset(adjtext2, ' ', sizeof(adjtext2));

      int adj1 = spfail - (lvl - splevel) * 3;
      if (adj1 < spfail) snprintf(AP(adjtext1), "(%d)", adj1);
      int adj2 = adj1 - (3 * think_adj);
      if (adj1 < spfail && adj2 < adj1) snprintf(AP(adjtext2), "(%d)", adj2);
      printf("%30s: %3d %6s %6s\n", spell_nameD[it], spfail, adjtext1,
             adjtext2);
    }
  }

  return 0;
}
