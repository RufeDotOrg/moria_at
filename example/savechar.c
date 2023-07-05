#define main _main
#include "src/moria_at.c"
#undef main

#define nameof(x) #x
static char* save_nameD[] = {
    nameof(&countD),   nameof(&dun_level),   nameof(entity_objD),
    nameof(invenD),    nameof(knownD),       nameof(maD),
    nameof(objD),      nameof(&obj_usedD),   nameof(player_hpD),
    nameof(&rnd_seed), nameof(&town_seed),   nameof(&obj_seed),
    nameof(&statD),    nameof(store_objD),   nameof(&turnD),
    nameof(&uD),       nameof(spell_orderD),
};
static void
dump_layout(version)
{
  int sum;
  for (int it = 0; it < AL(save_nameD); ++it) {
    printf("%s ", save_nameD[it]);
  }
  printf("\n");

  printf("static int savechar_v%03d[] = {\n  ", version);
  sum = 0;
  for (int it = 0; it < AL(save_bufD); ++it) {
    struct bufS buf = save_bufD[it];
    printf("%jd, ", buf.mem_size);
    sum += buf.mem_size;
  }
  printf("\n};\n");
  printf("#define SAVESUM%03d %d\n", version, sum);
}
int
main()
{
  dump_layout(AL(savefieldD));

  return 0;
}
