#include <stddef.h>

#include "game.c"

#define DECL_STRUCT(S, body) \
  struct S body;             \
  static char* declD = "struct " #S " " #body ";"

char* typeD[] = {
    "uint", "int", "char",  //"float", "double"
};
int type_widthD[] = {
    32,
    32,
    8,
};

struct subtypeS {
  char* name;
  char c;
  uint8_t byte_count;
};
// Unsigned sets bit 0x20 on char 'c' (making it upper case)
struct subtypeS subtypeD[] = {
    {"char", 'c', 1},
    {"short", 'h', 2},
    {"int", 'i', 4},
    {"long", 'l', 8},  // {"float", 'f', 4}, {"double", 'd', 8},
};

struct subtypeS*
subtype_by_type_width(char* type, int width)
{
  int us;
  switch (*type) {
    case 'u':
      us = 1;
    case 'i':
    case 'c':
      for (int it = 0; it < AL(subtypeD); ++it) {
        if (subtypeD[it].byte_count * 8 == width) return &subtypeD[it];
      }
      break;
  }
  return 0;
}

int
char_numeric(char c)
{
  uint8_t uv = c - '0';
  return uv <= 9;
}
char*
gen_field(str, outiter, outsize)
char* str;
char** outiter;
int* outsize;
{
  int ws;

  ws = strspn(str, " \t\r\n");
  str += ws;
  for (int it = 0; it < AL(typeD); ++it) {
    char* match = strstr(str, typeD[it]);
    if (!match) continue;

    char* stmt = strchr(str, ';');
    if (match < stmt) {
      uint64_t width = 0;
      uint64_t count = 0;
      for (char* cit = str; cit < stmt; ++cit) {
        if (width == 0 && char_numeric(*cit)) {
          width = strtoull(cit, 0, 10);
        }
        // Pointer is assumed to be the same size as this tool
        if (*cit == '*') {
          printf("pointer! %ju\n", sizeof(void*));
          width = 8*sizeof(void*);
        }
        // Array of instances
        if (count == 0 && *cit == '[') {
          count = strtoull(cit + 1, 0, 10);
          *outiter += sprintf(*outiter, "%ju", count);
        }
      }
      width = (width != 0) ? width : type_widthD[it];
      count = (count != 0) ? count : 1;

      struct subtypeS* st = subtype_by_type_width(typeD[it], width);
      **outiter = st->c;
      //TBD: outsize padding
      *outsize += count * width;
      printf("%s [width %ju] [count %d] [byte_count %d]\n", typeD[it], width, count,
             *outsize/8);
      return stmt;
    }
  }
  **outiter = 0;
  return str;
}

int
str_pack_decl(char* str, int len, char* decl)
{
  int width;
  char *iter, *write;

  width = 0;
  if (strstr(decl, "struct")) {
    iter = strstr(decl, "{");
    write = &str[0];
    while (iter) {
      iter = gen_field(iter + 1, &write, &width);
      if (*write == 0) break;
      write += 1;
    }
  }

  return width;
}

int
main()
{
  // DECL_STRUCT(sS, {
  //   int a;
  //   uint8_t b;
  // });
  //  struct sS s = {0};
  //  printf("[ %d a ] [ %d b ]\n", s.a, s.b);

  DECL_STRUCT(treasureS, {
    char* name;        /* Object name		*/
    uint32_t flags;    /* Special flags	*/
    uint8_t tval;      /* Category number	*/
    uint8_t tchar;     /* Character representation*/
    int16_t p1;        /* Misc. use variable	*/
    int32_t cost;      /* Cost of item		*/
    uint8_t subval;    /* Sub-category number	*/
    uint8_t number;    /* Number of items	*/
    uint16_t weight;   /* Weight		*/
    int16_t tohit;     /* Plusses to hit	*/
    int16_t todam;     /* Plusses to damage	*/
    int16_t ac;        /* Normal AC		*/
    int16_t toac;      /* Plusses to AC	*/
    uint8_t damage[2]; /* Damage when hits	*/
    uint8_t level;     /* Level item first found */
  });
  struct treasureS s = {0};
  puts(declD);

  char pack[32];
  int width = str_pack_decl(pack, AL(pack), declD);
  int byte_count = width / 8;
  printf("\npack: %s byte_count: %d sizeof: %ju\n", pack, byte_count,
         sizeof(s));
  if (byte_count != sizeof(s)) puts("warning: padded structure");

  return 0;
}
