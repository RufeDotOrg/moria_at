
int8_t xdirD[16] = {[1] = -1, [4] = -1, [7] = -1, [3] = 1, [6] = 1, [9] = 1};
__attribute__((always_inline)) int
dir_x(dir)
{
  return xdirD[dir];
}
int8_t ydirD[16] = {[1] = 1, [2] = 1, [3] = 1, [7] = -1, [8] = -1, [9] = -1};
__attribute__((always_inline)) int
dir_y(dir)
{
  return ydirD[dir];
}
