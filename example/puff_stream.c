
#define main _alt_main
#include "src/moria_at.c"
#undef main

int exec_decode(buf, len) void* buf;
{
  int offset = 0;
  while (len > 0) {
    int chunk = MIN(len, 32 * 1024);
    printf(">> 0x%jx buf hash %d chunk %d image_count\n",
           djb2(DJB2, &buf[offset], chunk), chunk, chunk / 256);
    len -= chunk;
    offset += chunk;
  }
}
int rewrite_io(source, srclen, chunk) void* source;
{
  printf("----\n");
  unsigned char* out = malloc(chunk);
  uint64_t sourcelen = srclen;
  uint64_t destlen = chunk;  // Reset output buffer size
  printf("%jd %jd destlen sourcelen\n", destlen, sourcelen);
  int rc = puff_stream_decode(out, &destlen, source, &sourcelen, exec_decode);
  printf("%d %jd %jd rc destlen sourcelen\n", rc, destlen, sourcelen);
  free(out);
  return 1;
}

int
main(int argc, char** argv)
{
  int test[] = {64 * 1024, 256 * 1024};
  for (int it = 0; it < AL(test); ++it) {
    // int r1 = rewrite_io(AP(artZ), test[it]);
    // printf("1) rewrite_io %d\n", r1);
    // int r2 = rewrite_io(AP(wallZ), test[it]);
    // printf("2) rewrite_io %d\n", r2);
    // int r3 = rewrite_io(AP(treasureZ), test[it]);
    // printf("3) rewrite_io %d\n", r3);
    int r4 = rewrite_io(AP(playerZ), test[it]);
    printf("4) rewrite_io %d\n", r4);
  }

  return 0;
}
