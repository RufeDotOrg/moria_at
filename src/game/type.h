// Rufe.org LLC 2022-2024: ISC License
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

// Global options
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
  char pc_renderer[16];
};
_Static_assert(sizeof(struct globalS) <= 64, "keep global data small!");
// Global data may be cached to disk
// Does not mutate on a game reset
DATA struct globalS globalD;

struct platformS {
  fn pregame;
  fn postgame;
  // render
  fn predraw;
  fn draw;
  fn orientation;
  fn vsync;
  // rng
  fn seed;
  // input
  fn input;
  fn selection;
  // i/o
  fn save;
  fn erase;
  fn load;
  fn savemidpoint;
  fn saveex;
  fn testex;
  fn dpad;
};
DATA struct platformS platformD;

struct __attribute__((aligned(4 * 1024))) padS {
  char fourcc[4];
  int width;
  int height;
};
