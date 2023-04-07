
#include "src/game.c"

char bufferD[64 * 1024];
int
main(int argc, char** argv)
{
  FILE* f = fopen(argv[1], "rb");
  if (!f) return 1;

  if (fread(bufferD, sizeof(bufferD), 1, f) <= 0) return 2;

  char* found[128];
  int found_used = 0;
  char* iter = bufferD;
  do {
    iter = strstr(iter, "<table");

    found[found_used] = iter;
    found_used += 1;
    iter += (iter != 0);
  } while (iter && found_used < AL(found));

  char* term[128];
  int term_used = 0;
  iter = bufferD;
  do {
    iter = strstr(iter, "</table>");

    term[term_used] = iter;
    term_used += 1;
    iter += (iter != 0);
  } while (iter && term_used < AL(term));

  printf("found %d term %d", found_used, term_used);

  if (argc > 2) {
    FILE* of = fopen(argv[2], "wb+");
    for (int it = 0; it < MIN(found_used, term_used); ++it) {
      fwrite(found[it], term[it] - found[it], 1, of);
    }
  }

  return 0;
}
