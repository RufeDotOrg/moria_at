// Rufe.org LLC 2022-2024: ISC License

#define T inline static __attribute__((always_inline))

DATA int8_t xdirD[16] = {
    [1] = -1, [4] = -1, [7] = -1, [3] = 1, [6] = 1, [9] = 1};
T int
dir_x(dir)
{
  return xdirD[dir];
}
DATA int8_t ydirD[16] = {
    [1] = 1, [2] = 1, [3] = 1, [7] = -1, [8] = -1, [9] = -1};
T int
dir_y(dir)
{
  return ydirD[dir];
}
DATA char kdirD[16] = {[1] = 'b', [2] = 'j', [3] = 'n', [4] = 'h', [5] = ' ',
                       [6] = 'l', [7] = 'y', [8] = 'k', [9] = 'u'};
T int
key_dir(dir)
{
  return kdirD[dir];
}
char* strchr(const char* s, int c);
T int
dir_key(key)
{
  char* iter = strchr(kdirD + 1, key);
  if (iter) return iter - kdirD;
  return 0;
}
