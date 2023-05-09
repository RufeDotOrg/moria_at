#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "src/game/common.h"

#define DBG "\n"
static char bufferD[4 * 1024 * 1024];
static char* cssD =
    "<head>"
    "  <title>Rufe.org</title>"
    "  <meta name=viewport content=width=device-width,initial-scale=1>"
    "  <link rel=icon href=data:>"
    "  <style>"
    "    html { font-family: sans-serif; font-size: 32px; "
    "width:auto;max-width:1280px;min-width:960px;margin:auto; }"
    "    body {  margin: 0 0 0 1em; padding: 2em 1em 1em 0em; }"
    "  </style>"
    "</head>"
    "\n";

void
exit_with_usage()
{
  puts("tohtml <file>");
  exit(1);
}

int
firstchar_from_str(str)
char* str;
{
  for (int it = 0; str[it] != 0; ++it) {
    if (str[it] == ' ') continue;
    if (str[it] == '\n') continue;
    if (str[it] == '\t') continue;
    return it;
  }
  return -1;
}
int
str_level(str)
char* str;
{
  char c = *str;
  int level = 0;
  while (*str == c) {
    level += 1;
    ++str;
  }
  return level;
}
char*
str_skipws(str)
char* str;
{
  while (*str == ' ' || *str == '\n' || *str == '\t') ++str;
  return str;
}
int
main(int argc, char** argv)
{
  if (argc != 2) exit_with_usage();

  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) return 2;

  int byte_count = read(fd, AP(bufferD));
  if (byte_count < 0 || byte_count >= AL(bufferD)) return 3;
  bufferD[byte_count] = 0;

  printf("<html>");
  printf("%s", cssD);
  printf("<body>");
  char* iter = bufferD;
  char* end;
  int was_list = 0;
  do {
    end = strchr(iter, '\n');
    if (end) *end = 0;
    int firstchar = firstchar_from_str(iter);
    int is_list = (iter[firstchar] == '*');
    int is_heading = (iter[firstchar] == '#');
    int is_link = (iter[firstchar] == '(');
    if ((was_list ^ is_list)) {
      if (is_list)
        printf("<ul>");
      else
        printf("</ul>");
    }
    if (is_list || is_heading || is_link) {
      int level = str_level(&iter[firstchar]);
      iter = str_skipws(&iter[firstchar + level]);
    }
    if (is_list) printf("<li>");
    if (is_heading) printf("<strong>");
    if (is_link) {
      char* mid = strchr(iter, ')');
      *mid = 0;
      printf("<p><a href=\"%s\">%s</a></p>", iter, str_skipws(mid + 1));
    } else {
      printf("<p>%s</p>", iter);
    }
    if (is_list) printf("</li>");
    if (is_heading) printf("</strong>");
    if (DBG) printf("%s", DBG);
    if (end) iter = end + 1;
    was_list = is_list;
  } while (end);
  printf("</body></html>");

  return 0;
}
