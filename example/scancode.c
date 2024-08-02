#include "src/game.c"

#include "platform/sdl2/platform.c"

#include <stdio.h>

int
main()
{
  char mapping[256] = {0};
  // Alpha?
  for (int it = 0; it < 26; ++it) {
    mapping[SDL_SCANCODE_A + it] = 'a' + it;
  }
  // Numeric
  mapping[SDL_SCANCODE_1] = '1';

  // Minimap
  mapping[SDL_SCANCODE_KP_0] = 'm';

  // Special
  // mapping[SDL_SCANCODE_RETURN] = '\n';
  mapping[SDL_SCANCODE_ESCAPE] = ESCAPE;
  mapping[SDL_SCANCODE_SPACE] = ' ';
  mapping[SDL_SCANCODE_MINUS] = '-';
  mapping[SDL_SCANCODE_EQUALS] = '=';
  mapping[SDL_SCANCODE_COMMA] = ',';
  mapping[SDL_SCANCODE_PERIOD] = '.';

  // Movement
  char dir[] = {
      'b', 'j', 'n', 'h', ' ', 'l', 'y', 'k', 'u',
  };
  for (int it = 0; it < AL(dir); ++it) {
    mapping[SDL_SCANCODE_KP_1 + it] = dir[it];
  }

  // confusing in a few places; low mana
  // mapping[SDL_SCANCODE_KP_ENTER] = ESCAPE;

  // Misc
  mapping[SDL_SCANCODE_KP_PLUS] = '+';
  mapping[SDL_SCANCODE_KP_MINUS] = '-';
  mapping[SDL_SCANCODE_KP_PERIOD] = '.';
  mapping[SDL_SCANCODE_KP_MULTIPLY] = 'i';
  mapping[SDL_SCANCODE_KP_DIVIDE] = 'e';

  // Output

  printf("DATA char gameplay_inputD[] = {\n");
  for (int it = 0; it < 16; ++it) {
    for (int jt = 0; jt < 16; ++jt) {
      printf("%3d, ", mapping[it * 16 + jt]);
    }
    printf("\n");
  }
  printf("};\n");

  FILE* f = fopen("keyconfig", "wb");
  if (f) {
    fwrite(AP(mapping), 1, f);
    fclose(f);
  }

  enum { SPAM = 0 };
  if (SPAM) {
    for (int it = 0; it < AL(mapping); ++it) {
      if (mapping[it]) printf("%c ", mapping[it]);
    }
  }

  return 0;
}
