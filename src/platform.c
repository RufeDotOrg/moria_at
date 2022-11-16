#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "tty.c"

char platform_readansi() {
  char c = 0;
  int read_count = read(0, &c, 1);
  if (read_count == -1) {
    if (errno != EAGAIN) return -1;
  }
  return c;
}

void platform_init() {
  ioctl(STDIN_FILENO, TCGETA, &save_termD);

  struct termios tbuf;
  tcgetattr(STDIN_FILENO, &tbuf);
  tbuf.c_iflag &= ~(ICRNL | IXON);
  tbuf.c_oflag &= ~(OPOST);
  tbuf.c_lflag &= ~(ECHO | ICANON | ISIG);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &tbuf);

  write(STDOUT_FILENO, tc_hide_cursorD, sizeof(tc_hide_cursorD));
}

void platform_reset() {
  write(STDOUT_FILENO, tc_clearD, sizeof(tc_clearD));
  write(STDOUT_FILENO, tc_show_cursorD, sizeof(tc_show_cursorD));

  ioctl(STDIN_FILENO, TCSETA, &save_termD);
}
