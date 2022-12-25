#include <sys/auxv.h>

int
platform_auxval_random()
{
  int *p = (void *)getauxval(AT_RANDOM);
  return *p;
}
