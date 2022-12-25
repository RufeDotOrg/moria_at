#include "dsp/tty/tty.h"
#include "libc/calls/calls.h"
#include "libc/calls/ioctl.h"
#include "libc/calls/struct/sigaction.h"
#include "libc/calls/termios.h"
#include "libc/errno.h"
#include "libc/fmt/fmt.h"
#include "libc/log/check.h"
#include "libc/log/log.h"
#include "libc/runtime/runtime.h"
#include "libc/stdio/rand.h"
#include "libc/stdio/stdio.h"
#include "libc/str/str.h"
#include "libc/sysv/consts/exit.h"
#include "libc/sysv/consts/fileno.h"
#include "libc/sysv/consts/o.h"
#include "libc/sysv/consts/sig.h"
#include "libc/sysv/consts/termios.h"
#include "libc/x/xsigaction.h"

#include "tty.c"

static bool platform_initD;
#define CTRL(C) ((C) ^ 0b01100000)
#define WRITE(FD, SLIT) write(FD, SLIT, strlen(SLIT))
#define ENABLE_SAFE_PASTE "\e[?2004h"
#define ENABLE_MOUSE_TRACKING "\e[?1000;1002;1015;1006h"
#define DISABLE_MOUSE_TRACKING "\e[?1000;1002;1015;1006l"
#define PROBE_DISPLAY_SIZE "\e7\e[9979;9979H\e[6n\e8"

#define INT16_MAX 0x7fff
#define INT32_MAX 0x7fffffff

int
platform_readansi()
{
  while (1) {
    char text[512];
    int len = 0;
    len = readansi(STDIN_FILENO, AP(text));
    if (len <= 0) {
      break;
    } else if (len == 1) {
      return text[0];
    } else {
      return tty_translate(text, len);
    }
  }
  return -1;
}

void
platform_reset()
{
  if (platform_initD) {
    WRITE(1, DISABLE_MOUSE_TRACKING);
    ioctl(1, TCSETS, &save_termD);
    write(1, tc_clearD, sizeof(tc_clearD));
    write(1, tc_show_cursorD, sizeof(tc_show_cursorD));
  }
}

static int
_rawmode()
{
  struct termios t;
  if (!platform_initD) {
    platform_initD = 1;
    if (ioctl(1, TCGETS, &save_termD) != -1) {
      atexit(platform_reset);
    } else {
      return -1;
    }
  }
  memcpy(&t, &save_termD, sizeof(t));
  t.c_cc[VMIN] = 1;
  t.c_cc[VTIME] = 1;
  t.c_iflag &= ~(INPCK | ISTRIP | PARMRK | INLCR | IGNCR | ICRNL | IXON |
                 IGNBRK | BRKINT);
  t.c_lflag &= ~(IEXTEN | ICANON | ECHO | ECHONL | ISIG);
  t.c_cflag &= ~(CSIZE | PARENB);
  t.c_oflag &= ~OPOST;
  t.c_cflag |= CS8;
  t.c_iflag |= IUTF8;
  ioctl(1, TCSETS, &t);
  WRITE(1, ENABLE_SAFE_PASTE);
  WRITE(1, ENABLE_MOUSE_TRACKING);
  WRITE(1, PROBE_DISPLAY_SIZE);
  return 0;
}

int
platform_seed()
{
  int seed = rdseed();
  return seed;
}

void
platform_init()
{
  _rawmode();
  write(1, tc_hide_cursorD, sizeof(tc_hide_cursorD));

  platformD.seed = platform_seed;
  platformD.readansi = platform_readansi;
  platformD.draw = platform_draw;
}
