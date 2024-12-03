// Rufe.org LLC 2022-2024: ISC License
// reference to buffer of known length in bytes
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
  uint8_t stable_button_order;
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
