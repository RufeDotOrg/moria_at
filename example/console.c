#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

static struct termios save_termio;

void init_raw() {
  struct termios tbuf;
  tcgetattr(STDIN_FILENO, &tbuf);
  tbuf.c_iflag &= ~(ICRNL | IXON);
  tbuf.c_oflag &= ~(OPOST);
  // ignore_signals: ISIG
  tbuf.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &tbuf);
}

int main() {
  ioctl(0, TCGETA, &save_termio);

  init_raw();

  char c;
  while (1) {
    int read_count = read(0, &c, 1);
    if (read_count == -1) {
      if (errno != EAGAIN)
        break;
      continue;
    }

    if (iscntrl(c)) {
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
  }

  ioctl(0, TCSETA, &save_termio);
  return 0;
}
