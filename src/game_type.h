
struct uS {
  int x;
  int y;
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
  uint8_t cidx;  // creature
  uint8_t tidx;  // treasure
  uint8_t fval;  // floor
};
static struct caveS caveD[MAX_HEIGHT][MAX_WIDTH];
