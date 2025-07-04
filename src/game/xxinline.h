// Rufe.org LLC 2022-2024: ISC License
#if defined(__FATCOSMOCC__)
#define XX static __attribute__((pure)) inline __attribute__((always_inline))
#else
#define XX static __attribute__((pure))
#endif
XX int
char_visible(c)
{
  uint8_t vis = c - 0x21;
  return vis < 0x7f - 0x21;
}
XX int
char_alpha(c)
{
  return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}
XX int
char_digit(c)
{
  return ('0' <= c && c <= '9');
}
XX int
char_bit(c)
{
  int b = c - 'a';
  if (b <= 'z' - 'a') return (1u << b);
  return 0;
}
XX int
is_ctrl(c)
{
  return c <= 0x1f;
}
XX int
is_lower(c)
{
  uint8_t iidx = c - 'a';
  return iidx <= 'z' - 'a';
}
XX int
is_upper(c)
{
  uint8_t iidx = c - 'A';
  return iidx <= 'Z' - 'A';
}
XX int
distance(y1, x1, y2, x2)
{
  int dy, dx;

  dy = ABS(y1 - y2);
  dx = ABS(x1 - x2);
  return ((((dy + dx) << 1) - (dy > dx ? dx : dy)) >> 1);
}
XX void*
vptr(void* ptr)
{
  return ptr;
}
XX uint8_t* bptr(void* ptr) __attribute__((alias("vptr")));
XX int8_t* cptr(void* ptr) __attribute__((alias("vptr")));
XX int* iptr(void* ptr) __attribute__((alias("vptr")));
XX float* fptr(void* ptr) __attribute__((alias("vptr")));
XX fn fnptr(void* ptr) __attribute__((alias("vptr")));
XX void*
ptr_xor(void* a, void* b)
{
  return (void*)((ptrsize)a ^ (ptrsize)b);
}
enum { DJB2 = 5381 };
XX uint64_t
djb2(uint64_t value, const void* buffer, uint64_t bytes)
{
  const uint8_t* iter = buffer;
  for (int i = 0; i < bytes; ++i) {
    uint8_t c = iter[i];
    value = (value << 5) + value ^ c;
  }
  return value;
}
XX int
parse_num(char* str)
{
  uint8_t digit[2];

  for (int it = 0; it < AL(digit); ++it) digit[it] = str[it] - '0';

  int ret = 0;
  for (int it = 0; it < AL(digit); ++it) {
    int d = digit[it];
    if (d < 10) {
      ret *= 10;
      ret += d;
    }
  }
  return ret;
}
