
#define GAME static __attribute__((section("game")))

static char global_nameD[16] = "fredrick";
GAME char py_nameD[16] = {0};

extern char __start_game;
extern char __stop_game;
int main()
{
  printf("global name: %s\n", global_nameD);
  printf("name: %s\n", py_nameD);
  printf("%p __start_game\n",  &__start_game);
  printf("%p __stop_game\n",  &__stop_game);
  printf("%p game var\n", &py_nameD);
  return 0;
}
