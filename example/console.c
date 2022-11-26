#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "game.c"

static struct termios save_termio;

void
init_raw()
{
  struct termios tbuf;
  tcgetattr(STDIN_FILENO, &tbuf);
  tbuf.c_iflag &= ~(ICRNL | IXON);
  tbuf.c_oflag &= ~(OPOST);
  // ignore_signals: bg/fg changes the terminal settings underneath you
  tbuf.c_lflag &= ~(ECHO | ICANON | ISIG);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &tbuf);
}

int
main()
{
  ioctl(0, TCGETA, &save_termio);

  init_raw();

  fcntl(0, F_SETFL, O_NONBLOCK);
  char prev = 0;
  while (1) {
    char text[8] = {0};
    int len = 0;
    len = read(STDIN_FILENO, AP(text));

    if (len <= 0) {
      nanosleep(&(struct timespec){0, 8e6}, 0);
      continue;
    }
    if (iscntrl(text[0])) {
      printf("code: ");
      for (int it = 0; it < len; ++it) {
        if (text[it]) printf("%x ", text[it]);
      }
      printf("\r\n");
    } else {
      printf("%x ('%c')\r\n", text[0], text[0]);
    }
    if (len > 1) {
      printf("tty_translate %c\r\n", tty_translate(text, len));
    }

    if (text[0] == CTRL('c') && prev == CTRL('c')) break;
    prev = text[0];
  }

  ioctl(0, TCSETA, &save_termio);
  return 0;
}
