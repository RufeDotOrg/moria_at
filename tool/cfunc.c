
#include <stdint.h>
#include <stdio.h>

#define AL(x) (sizeof(x) / sizeof(x[0]))
char bufD[4 * 1024 * 1024];

int
char_whitespace(char c)
{
  switch (c) {
    case '\t':
    case ' ':
    case '\r':
    case '\n':
      return 1;
  }
  return 0;
}

static char* keywordD[] = {
    "if", "switch", "for", "while", "do",
};
int
is_keyword(char* token, int len)
{
  for (int it = 0; it < AL(keywordD); ++it) {
    if (strncmp(keywordD[it], token, len) == 0) return 1;
  }
  return 0;
}

void
check_prior_token(char* func_paren)
{
  int ws = 0;
  for (char* it = func_paren - 1; char_whitespace(*it); --it) {
    ws += 1;
  }

  int wc = 0;
  for (char* it = func_paren - ws - 1; !char_whitespace(*it); --it) {
    wc += 1;
  }

  char* token = func_paren - ws - wc;
  if (is_keyword(token, wc)) return;
  for (int it = 0; it < wc; ++it) {
    putchar(token[it]);
  }
  putchar('\n');
}

void
maybe_func(char* code)
{
  int depth;
  depth = 0;
  for (char* it = code - 1; it > bufD; --it) {
    char c = *it;
    switch (c) {
      case ')':
        depth -= 1;
        break;
      case '(':
        depth += 1;
        if (depth == 0) {
          check_prior_token(it);
          return;
        }
        break;
      case ';':
        if (depth != 0) return;
        break;
      case '{':
      case '}':
      case '=':
      case '"':
        return;
    }
  }
}

void
exit_with_usage()
{
  printf("cfunc <file>\n");
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

  for (char* it = bufD; *it != 0; ++it) {
    char c = *it;
    if (c == '{') maybe_func(it);
  }

  return 0;
}
