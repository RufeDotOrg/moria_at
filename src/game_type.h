
struct uS {
  int x;
  int y;
  int exp;
  int lev;
  int chp;
  int mhp;
  int ac;
  int toac;
};
static struct uS uD;

struct panelS {
  int panel_row;
  int panel_row_min;
  int panel_row_max;
  int panel_col;
  int panel_col_min;
  int panel_col_max;
};
static struct panelS panelD;

struct caveS {
  uint8_t midx;  // monster
  uint8_t oidx;  // object
  uint8_t fval;  // floor
  uint8_t lr : 1;
};
static struct caveS caveD[MAX_HEIGHT][MAX_WIDTH];

struct objS {
  int id;
  uint16_t tidx;  // treasure
  uint8_t tval;
  uint8_t tchar;
  uint8_t subval;
  uint8_t number;
  int16_t p1;
};

struct monS {
  int id;
  int16_t hp;
  uint16_t cidx;  // creature
  uint8_t fy;
  uint8_t fx;
  uint8_t cdis;
};
