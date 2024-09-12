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
XX uint8_t*
bptr(void* ptr)
{
  return ptr;
}
XX int*
iptr(void* ptr)
{
  return ptr;
}
XX void*
ptr_xor(void* a, void* b)
{
  return (void*)XOR(a, b);
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
