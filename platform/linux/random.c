#include <errno.h>
#include <sys/random.h>

int
platform_random()
{
  int i, f, r;
  int val;
  f = GRND_RANDOM | GRND_NONBLOCK;
  for (int i = 0; i < sizeof(val); i += r) {
    r = getrandom(&val, sizeof(val) - i, f);
    if (r == -1 && errno == EINTR)
      r = 0;
    else if (r == -1 && errno == EAGAIN)
      r = f = 0;
  }
  return val;
}
