#include "game.c"

#include <stdio.h>

char bufD[4 * 1024 * 1024];
uint64_t bucketD[256];
void
bucket_output_for_chars(char* chars)
{
  for (char* it = chars; *it != 0; ++it) {
    char c = *it;
    printf("%c %ju\n", c, bucketD[c]);
  }
}

void
exit_with_usage()
{
  printf("charcount <file>\n");
}

int
main(int argc, char** argv)
{
  if (argc < 2) exit_with_usage();

  FILE* f = fopen(argv[1], "rb");
  if (!f) return 1;

  int byte_count = fread(bufD, 1, sizeof(bufD), f);
  if (byte_count < 0 || byte_count == sizeof(bufD)) return 2;
  bufD[byte_count] = 0;

  for (int it = 0; it < byte_count; ++it) {
    bucketD[bufD[it]] += 1;
  }

  bucket_output_for_chars("`~!@#$%^&*()-={}[]\\|;':\",./<>?");
  return 0;
}
