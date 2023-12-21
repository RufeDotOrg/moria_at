#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "tty.c"

enum { KEYBOARD = 1 };

#ifndef TCGETS
#define TCGETS TIOCGETA
#endif
#ifndef TCSETS
#define TCSETS TIOCSETA
#endif

int
platform_readansi()
{
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
  return -1;
}

int
platform_pregame()
{
  if (save_termD[1] == 0) {
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    ioctl(STDIN_FILENO, TCGETS, save_termD);

    struct termios tbuf;
    tcgetattr(STDIN_FILENO, &tbuf);
    tbuf.c_iflag &= ~(ICRNL | IXON);
    tbuf.c_oflag &= ~(OPOST);
    tbuf.c_lflag &= ~(ECHO | ICANON | ISIG);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &tbuf);

    write(STDOUT_FILENO, tc_hide_cursorD, sizeof(tc_hide_cursorD));
  }

  platformD.readansi = platform_readansi;
  platformD.draw = platform_draw;
  platformD.predraw = platform_predraw;

  return 0;
}

int
platform_postgame()
{
  if (save_termD[1]) {
    write(STDOUT_FILENO, tc_clearD, sizeof(tc_clearD));
    write(STDOUT_FILENO, tc_show_cursorD, sizeof(tc_show_cursorD));

    ioctl(STDIN_FILENO, TCSETS, save_termD);
  }

  return 0;
}
