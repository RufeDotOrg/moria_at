
#include "game.c"

#include "platform/platform.c"

int main()
{
  platform_init();
  platform_pregame();

  return platform_postgame(0);
}
