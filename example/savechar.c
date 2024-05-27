#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "src/game.c"

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
version_by_savesum(sum)
{
  for (int it = 0; it < AL(savesumD); ++it)
    if (savesumD[it] == sum) return it;
  return -1;
}
static void show_character(filename) char* filename;
{
  char buf[16 * 1024];
  int fd = open(filename, O_RDONLY);

  if (fd) {
    int save_size;
    read(fd, &save_size, sizeof(save_size));
    int version = version_by_savesum(save_size);
    printf("%s: version %d - ", filename, version);
    int* savefield = savefieldD[version];
    for (int it = 0; it < 16; ++it) {
      int size = read(fd, buf, savefield[it]);
      // printf("read size %d\n", size);
    }
    struct uS* u = vptr(buf);
    printf("level %d ", u->lev);
    printf("%s %s", raceD[uD.ridx].name, classD[uD.clidx].name);
    printf("\n");
    close(fd);
  }
}
int
main(int argc, char** argv)
{
  if (argc <= 1) dump_layout(AL(savefieldD));

  if (argc > 1) {
    for (int it = 1; it < argc; ++it) {
      show_character(argv[it]);
    }
  }
  return 0;
}
