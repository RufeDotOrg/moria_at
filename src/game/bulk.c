// Rufe.org LLC 2022-2025: ISC License
// const.c
enum { DJB2 = 5381 };

// macro.c
#include "macro.h"

// type.c
typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __UINT64_TYPE__ uint64_t;
typedef __INT8_TYPE__ int8_t;
typedef __INT16_TYPE__ int16_t;
typedef __INT32_TYPE__ int32_t;
typedef __INT64_TYPE__ int64_t;
typedef __PTRDIFF_TYPE__ ptrsize;
typedef ptrsize (*fn)();
typedef int BOOL;
struct bufS {
  void* mem;
  uint64_t mem_size;
};
struct globalS {
  uint32_t ghash;
  int32_t saveslot_class;
  uint32_t zoom_factor;
  uint8_t orientation_lock;
  uint8_t vsync;
  uint8_t sprite;
  uint8_t hand_swap;
  uint16_t dpad_sensitivity;
  uint8_t dpad_color;
  uint8_t small_text;
  uint8_t label_button_order;
  uint8_t use_joystick;
  char pc_renderer[16];
};
static struct globalS globalD;
struct platformS {
  fn pregame;
  fn postgame;
  fn predraw;
  fn draw;
  fn orientation;
  fn vsync;
  fn seed;
  fn input;
  fn selection;
  fn save;
  fn erase;
  fn load;
  fn savemidpoint;
  fn saveex;
  fn testex;
  fn dpad;
};
static struct platformS platformD;
struct __attribute__((aligned(4 * 1024))) padS {
  char fourcc[4];
  int width;
  int height;
};
typedef struct rectS {
  int x;
  int y;
  int w;
  int h;
} rect_t;
typedef struct pointS {
  int x;
  int y;
} point_t;

// decl.c
char* strchr(const char* s, int c);
_Static_assert(sizeof(struct globalS) <= 64, "keep global data small!");
_Static_assert(sizeof(ptrsize) == sizeof(void*), "ptrsize check");
static __attribute__((pure)) uint8_t* bptr(void* ptr)
    __attribute__((alias("vptr")));
static __attribute__((pure)) int8_t* cptr(void* ptr)
    __attribute__((alias("vptr")));
static __attribute__((pure)) int* iptr(void* ptr)
    __attribute__((alias("vptr")));
static __attribute__((pure)) float* fptr(void* ptr)
    __attribute__((alias("vptr")));
static __attribute__((pure)) fn fnptr(void* ptr) __attribute__((alias("vptr")));

// var.c
static int8_t xdirD[16] = {
    [1] = -1, [4] = -1, [7] = -1, [3] = 1, [6] = 1, [9] = 1};
static int8_t ydirD[16] = {
    [1] = 1, [2] = 1, [3] = 1, [7] = -1, [8] = -1, [9] = -1};
static char kdirD[16] = {[1] = 'b', [2] = 'j', [3] = 'n', [4] = 'h', [5] = ' ',
                         [6] = 'l', [7] = 'y', [8] = 'k', [9] = 'u'};
char versionD[] = "XXXX.YYYY.ZZZZ";
char git_hashD[] = "AbCdEfGhIjKlMnO";

// fn.c
inline static __attribute__((always_inline)) int
dir_x(dir)
{
  return xdirD[dir];
}
inline static __attribute__((always_inline)) int
dir_y(dir)
{
  return ydirD[dir];
}
inline static __attribute__((always_inline)) int
key_dir(dir)
{
  return kdirD[dir];
}
inline static __attribute__((always_inline)) int
dir_key(key)
{
  char* iter = strchr(kdirD + 1, key);
  if (iter) return iter - kdirD;
  return 0;
}
static int
point_in_rect(void* in_point, void* in_rect)
{
  point_t* p = in_point;
  rect_t* r = in_rect;
  uint32_t xdelta = p->x - r->x;
  uint32_t ydelta = p->y - r->y;
  return (xdelta < r->w && ydelta < r->h);
}
static ptrsize
noop()
{
  return 0;
}
static int
apclear(dst, dlen)
char* dst;
{
  memset(dst, 0, dlen);
}
static int
apspace(dst, dlen)
char* dst;
{
  memset(dst, 0x20202020, dlen);
}
static int
apcopy(dst, dlen, src, slen)
char* dst;
char* src;
{
  if (slen <= dlen) memcpy(dst, src, slen);
  return (slen <= dlen) * slen;
}
static int
apcat(dst, dlen, src)
char* dst;
char* src;
{
  char c;
  char* start = dst;
  char* iter = dst;
  while (dlen > 0 && *iter) {
    ++iter;
    --dlen;
  }
  while ((c = *src++)) {
    if (dlen) {
      *iter++ = c;
      --dlen;
    }
  }
  if (dlen) *iter = 0;
  return iter - start;
}
static int
apcati(dst, dlen, num)
char* dst;
{
  char* iter = dst;
  for (; *iter; ++iter) --dlen;
  if (num < 0) num = -num;
  char* start = iter;
  for (int it = 0; it < dlen; ++it) {
    if (num == 0) break;
    *iter++ = '0' + (num % 10);
    num /= 10;
  }
  if (num == 0) {
    char* last = iter - 1;
    while (start < last) {
      char temp = *start;
      *start++ = *last;
      *last-- = temp;
    }
  } else {
    iter = start;
  }
  *iter = '\0';
  return iter - dst;
}
static __attribute__((pure)) int
char_visible(c)
{
  uint8_t vis = c - 0x21;
  return vis < 0x7f - 0x21;
}
static __attribute__((pure)) int
char_alpha(c)
{
  return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}
static __attribute__((pure)) int
char_digit(c)
{
  return ('0' <= c && c <= '9');
}
static __attribute__((pure)) int
char_bit(c)
{
  int b = c - 'a';
  if (b <= 'z' - 'a') return (1u << b);
  return 0;
}
static __attribute__((pure)) int
is_ctrl(c)
{
  return c <= 0x1f;
}
static __attribute__((pure)) int
is_lower(c)
{
  uint8_t iidx = c - 'a';
  return iidx <= 'z' - 'a';
}
static __attribute__((pure)) int
is_upper(c)
{
  uint8_t iidx = c - 'A';
  return iidx <= 'Z' - 'A';
}
static __attribute__((pure)) int
distance(y1, x1, y2, x2)
{
  int dy, dx;
  dy = ((y1 - y2) > 0 ? (y1 - y2) : -(y1 - y2));
  dx = ((x1 - x2) > 0 ? (x1 - x2) : -(x1 - x2));
  return ((((dy + dx) << 1) - (dy > dx ? dx : dy)) >> 1);
}
static __attribute__((pure)) void*
vptr(void* ptr)
{
  return ptr;
}
static __attribute__((pure)) void*
ptr_xor(void* a, void* b)
{
  return (void*)((ptrsize)a ^ (ptrsize)b);
}
static __attribute__((pure)) uint64_t
djb2(uint64_t value, const void* buffer, uint64_t bytes)
{
  const uint8_t* iter = buffer;
  for (int i = 0; i < bytes; ++i) {
    uint8_t c = iter[i];
    value = (value << 5) + value ^ c;
  }
  return value;
}
static __attribute__((pure)) int
parse_num(char* str)
{
  uint8_t digit[2];
  for (int it = 0; it < (sizeof(digit) / sizeof(digit[0])); ++it)
    digit[it] = str[it] - '0';
  int ret = 0;
  for (int it = 0; it < (sizeof(digit) / sizeof(digit[0])); ++it) {
    int d = digit[it];
    if (d < 10) {
      ret *= 10;
      ret += d;
    }
  }
  return ret;
}

