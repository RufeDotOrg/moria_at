// Rufe.org LLC 2022-2024: ISC License
#pragma once

#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

#if __APPLE__
#undef snprintf
#endif

DATA FILE* fileoutD;
#define printf(x, ...) fprintf(fileoutD, x, ##__VA_ARGS__)
#define Log(x, ...) fprintf(fileoutD, x "\n", ##__VA_ARGS__)
DATA int quitD;

enum { KEYBOARD = 1 };
enum { PC = 1 };
enum { GFX = 0 };
// Full terminal: No post processing / text output to stdout
enum { FULLTERM = 1 };

#include "tty.c"

#ifndef TCGETS
#define TCGETS TIOCGETA
#endif
#ifndef TCSETS
#define TCSETS TIOCSETA
#endif

int
platform_readansi()
{
  if (quitD) return CTRL('c');
  while (1) {
    char text[8] = {0};
    int len = 0;
    len = read(STDIN_FILENO, AP(text));
    if (len <= 0) {
      if (errno == EINTR || errno == EAGAIN) {
        nanosleep(&(struct timespec){0, 8e6}, 0);
        continue;
      }
      return CTRL('x');
    } else if (len == 1) {
      return text[0];
    } else {
      return tty_translate(text, len);
    }
  }
  return 0;
}

int
platform_pregame()
{
  if (!fileoutD) fileoutD = fopen("log.txt", "wb");

  if (save_termD[1] == 0) {
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    ioctl(STDIN_FILENO, TCGETS, save_termD);

    struct termios tbuf;
    tcgetattr(STDIN_FILENO, &tbuf);
    tbuf.c_iflag &= ~(ICRNL | IXON);
    if (FULLTERM) tbuf.c_oflag &= ~(OPOST);
    tbuf.c_lflag &= ~(ECHO | ICANON | ISIG);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &tbuf);

    write(STDOUT_FILENO, tc_hide_cursorD, sizeof(tc_hide_cursorD));
  }

  Log("pregame complete");

  return 0;
}

int
platform_postgame()
{
  if (fileoutD) {
    fclose(fileoutD);
    fileoutD = 0;
  }

  if (save_termD[1]) {
    if (FULLTERM) write(STDOUT_FILENO, tc_clearD, sizeof(tc_clearD));
    write(STDOUT_FILENO, tc_show_cursorD, sizeof(tc_show_cursorD));

    ioctl(STDIN_FILENO, TCSETS, save_termD);
  }

  return 0;
}

int
platform_init()
{
  platformD.input = platform_readansi;
  platformD.draw = platform_draw;
  platformD.predraw = platform_predraw;

  platformD.pregame = platform_pregame;
  platformD.postgame = platform_postgame;
  platformD.draw = platform_draw;
  platformD.input = platform_readansi;
  if (platformD.seed == noop) platformD.seed = platform_random;
  platformD.load = platform_load;
  platformD.save = platform_save;
  platformD.erase = platform_erase;
}

int
zoom_rect(rect_t* po)
{
  *po = (rect_t){
      panelD.panel_row_min,
      panelD.panel_col_min,
      SYMMAP_WIDTH,
      SYMMAP_HEIGHT,
  };
}
