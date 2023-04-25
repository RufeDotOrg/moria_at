static int
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
static int
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
static int
is_ctrl(c)
{
  return c <= 0x1f;
}
static int
is_lower(c)
{
  uint8_t iidx = c - 'a';
  return iidx <= 'z' - 'a';
}
static int
is_upper(c)
{
  uint8_t iidx = c - 'A';
  return iidx <= 'Z' - 'A';
}
