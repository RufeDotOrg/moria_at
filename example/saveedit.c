#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define main _game_main
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
  printf("hash 0x%jx\n", djb2(DJB2, save_bufD, AL(save_bufD)));
}
static void print_savechar(filename) char* filename;
{
  input_resumeD = -1;
  path_load(filename);

  printf("level %d ", uD.lev);
  printf("%s %s", raceD[uD.ridx].name, classD[uD.clidx].name);
  printf("\n");

  // statD.max_stat[A_CON] = 10;
  // statD.cur_stat[A_CON] = 10;

  // struct objS* obj = obj_use();
  // tr_obj_copy(221, obj);
  // printf("obj %d\n", obj->tidx);
  // obj->number = 4;

  // invenD[INVEN_EQUIP - 1] = obj->id;
  // path_save(filename);
}
int
main(int argc, char** argv)
{
  if (argc <= 1) dump_layout(AL(savefieldD));

  if (argc > 1) {
    for (int it = 1; it < argc; ++it) {
      print_savechar(argv[it]);
    }
  }
  return 0;
}
