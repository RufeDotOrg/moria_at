
// Platform visualization info
struct vizS {
  char sym;
  uint8_t floor;
  uint8_t light;
  uint8_t dim;
  uint8_t tr;
  uint16_t cr;
  uint16_t fade;
};

// U the player
struct uS {
  int y;
  int x;
  int exp;
  int max_exp;
  int lev;
  int max_dlv;
  int chp;
  int chp_frac;
  int mhp;
  int gold;
  uint8_t melee_confuse;
  uint8_t melee_genocide;
  uint8_t melee_unused1;
  uint8_t melee_unused2;
  // Stateful affects from magic spells
  int ma_ac;
  int mflag;
  // Status
  int food;
  int food_digest;
  uint8_t char_unused1;
  uint8_t clidx;  // class
  uint8_t ridx;   // race
  uint8_t male;
  // initialized by class/race
  int bth;
  int search;
  int fos;  // Frequency of search
  int disarm;
  int stealth;
  int save;
  int infra;
  int mult_exp;  // multiplier for exp-per-level
  int wt;
};
static struct uS uD;

// Calculated bonus; see calc_bonuses()
struct cbS {
  int ptohit;
  int ptodam;
  int pac;
  int ptoac;
  int hide_toac;
  int hide_tohit;
  int hide_todam;
  int prev_weapon;
  // Treasure flags from equipment
  int tflag;
  // Other flags
  uint8_t weapon_heavy;
};
static struct cbS cbD;

struct countS {
  // Player ailment counters
  int poison;
  int blind;  // TBD: deprecate
  int confusion;
  int fear;  // TBD: deprecate
  int rest;
  int paralysis;
  // Player protection counters
  int protevil;
};
static struct countS countD;

struct raceS {
  char *name;          /* Type of race			*/
  uint8_t attr[MAX_A]; /* Attribute adjustments */
  uint8_t b_age;       /* Base age of character		*/
  uint8_t m_age;       /* Maximum age of character	*/
  uint8_t m_b_ht;      /* base height for males		*/
  uint8_t m_m_ht;      /* mod height for males		*/
  uint8_t m_b_wt;      /* base weight for males		*/
  uint8_t m_m_wt;      /* mod weight for males		*/
  uint8_t f_b_ht;      /* base height females		*/
  uint8_t f_m_ht;      /* mod height for females	*/
  uint8_t f_b_wt;      /* base weight for female	*/
  uint8_t f_m_wt;      /* mod weight for females	*/
  int8_t dis;          /* base chance to disarm		*/
  int8_t srh;          /* base chance for search	*/
  int8_t stl;          /* Stealth of character		*/
  int8_t fos;          /* frequency of auto search	*/
  int8_t bth;          /* adj base chance to hit	*/
  int8_t bthb;         /* adj base to hit with bows	*/
  int8_t bsav;         /* Race base for saving throw	*/
  uint8_t bhitdie;     /* Base hit points for race	*/
  uint8_t infra;       /* See infra-red			*/
  uint8_t b_exp;       /* Base experience factor	*/
  uint8_t rtclass;     /* Bit field for class types	*/
};

struct classS {
  char *name;              /* type of class		*/
  uint8_t adj_hd;          /* Adjust hit points		*/
  uint8_t mdis;            /* mod disarming traps		*/
  uint8_t msrh;            /* modifier to searching	*/
  uint8_t mstl;            /* modifier to stealth		*/
  uint8_t mfos;            /* modifier to freq-of-search	*/
  uint8_t mbth;            /* modifier to base to hit	*/
  uint8_t mbthb;           /* modifier to base to hit - bows*/
  uint8_t msav;            /* Class modifier to save	*/
  uint8_t mattr[MAX_A];    /* Class modifier for attributes	*/
  uint8_t spell;           /* class use mage spells	*/
  uint8_t m_exp;           /* Class experience factor	*/
  uint8_t first_spell_lev; /* First level where class can use spells. */
};

// Only the player has stats
struct statS {
  int8_t max_stat[MAX_A]; /* What is restored */
  int8_t cur_stat[MAX_A]; /* What is natural */
  int8_t mod_stat[MAX_A]; /* Boosted artificially (by wearing something) */
  int8_t use_stat[MAX_A]; /* Play value, see set_use_stat() */
};
static struct statS statD;

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
  uint8_t midx;   // monster
  uint8_t oidx;   // object
  uint8_t fval;   // floor
  uint8_t cflag;  // cave flag (CF_)
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
  uint32_t flags;
  uint8_t fy;
  uint8_t fx;
  uint8_t tval;
  uint8_t tchar;
  uint16_t tidx;  // treasure definition
  int16_t p1;
  int32_t cost;
  uint8_t subval;
  uint8_t number;
  uint16_t weight;
  int16_t tohit;
  int16_t todam;
  int16_t ac;
  int16_t toac;
  uint8_t damage[2];
  uint8_t level;
  uint8_t idflag;  // aka. ident
  uint8_t sn;
};

struct creatureS {
  char *name;             /* Descrip of creature	*/
  uint32_t cmove;         /* Bit field		*/
  uint32_t spells;        /* Creature spells	*/
  uint16_t cdefense;      /* Bit field		*/
  uint16_t mexp;          /* Exp value for kill	*/
  uint8_t sleep;          /* Inactive counter/10	*/
  uint8_t aaf;            /* Area affect radius	*/
  uint8_t ac;             /* AC			*/
  uint8_t speed;          /* Movement speed+10	*/
  uint8_t cchar;          /* Character rep.	*/
  uint8_t hd[2];          /* Creatures hit die	*/
  uint8_t attack_list[4]; /* Type attack and damage*/
  uint8_t level;          /* Level of creature	*/
};

struct attackS {
  uint8_t attack_type;
  uint8_t attack_desc;
  uint8_t attack_dice;
  uint8_t attack_sides;
};

struct monS {
  int id;
  uint16_t cidx;  // creature definition
  uint16_t msleep;
  int mspeed;
  int hp;
  uint8_t fy;
  uint8_t fx;
  uint8_t mlit;
  uint8_t mstunned;
  uint8_t mconfused;
  uint8_t msilenced;
};

struct ownerS {
  char *name;
  int16_t max_cost;
  uint8_t max_inflate;
  uint8_t min_inflate;
  uint8_t haggle_per;
  uint8_t owner_race;
  uint8_t insult_max;
};

struct platformS {
  fn seed;
  fn load;
  fn save;
  fn erase;
  fn readansi;
  fn draw;
};
EXTERN struct platformS platformD;
