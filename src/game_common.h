#include <stdbool.h>
#include <stdint.h>

#define CLAMP(x, min, max) (x < min ? min : x > max ? max : x)
#define AL(x) (sizeof(x) / sizeof(x[0]))
#define AP(x) x, AL(x)
