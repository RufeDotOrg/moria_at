#if defined(__FATCOSMOCC__)
#define XX inline
#else
#define XX static
#endif
XX int
char_visible(char c)
{
  uint8_t vis = c - 0x21;
  return vis < 0x7f - 0x21;
}
XX int
char_alpha(char c)
{
  return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
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
dir_x(dir)
{
  switch (dir) {
    case 1:
    case 4:
    case 7:
      return -1;
    case 3:
    case 6:
    case 9:
      return 1;
  }
  return 0;
}
XX int
dir_y(dir)
{
  switch (dir) {
    case 1:
    case 2:
    case 3:
      return 1;
    case 7:
    case 8:
    case 9:
      return -1;
  }
  return 0;
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
