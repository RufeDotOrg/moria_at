
struct uS {
  int y;
  int x;
  int exp;
  int lev;
  int chp;
  int mhp;
  int gold;
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

struct treasureS {
  char *name;        /* Object name		*/
  uint32_t flags;    /* Special flags	*/
  uint8_t tval;      /* Category number	*/
  uint8_t tchar;     /* Character representation*/
  int16_t p1;        /* Misc. use variable	*/
  int32_t cost;      /* Cost of item		*/
  uint8_t subval;    /* Sub-category number	*/
  uint8_t number;    /* Number of items	*/
  uint16_t weight;   /* Weight		*/
  int16_t tohit;     /* Plusses to hit	*/
  int16_t todam;     /* Plusses to damage	*/
  int16_t ac;        /* Normal AC		*/
  int16_t toac;      /* Plusses to AC	*/
  uint8_t damage[2]; /* Damage when hits	*/
  uint8_t level;     /* Level item first found */
};

struct objS {
  int id;
  uint16_t tidx;  // treasure definition
  uint8_t tval;
  uint8_t tchar;
  int16_t p1;
  int32_t cost;
  uint8_t subval;
  uint8_t number;
  uint8_t dam[2];
  uint8_t level;
};

struct creatureS {
  char *name;        /* Descrip of creature	*/
  uint32_t cmove;    /* Bit field		*/
  uint32_t spells;   /* Creature spells	*/
  uint16_t cdefense; /* Bit field		*/
  uint16_t mexp;     /* Exp value for kill	*/
  uint8_t sleep;     /* Inactive counter/10	*/
  uint8_t aaf;       /* Area affect radius	*/
  uint8_t ac;        /* AC			*/
  uint8_t speed;     /* Movement speed+10	*/
  uint8_t cchar;     /* Character rep.	*/
  uint8_t hd[2];     /* Creatures hit die	*/
  uint8_t damage[4]; /* Type attack and damage*/
  uint8_t level;     /* Level of creature	*/
};

struct attackS {
  uint8_t attack_type;
  uint8_t attack_desc;
  uint8_t attack_dice;
  uint8_t attack_sides;
};

struct monS {
  int id;
  uint8_t fy;
  uint8_t fx;
  uint16_t cidx;  // creature definition
  int16_t hp;
  uint8_t cdis;
};
