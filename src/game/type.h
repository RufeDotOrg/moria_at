
// reference to buffer of known length in bytes
struct bufS {
  void* mem;
  uint64_t mem_size;
};
// Buffer
#define BUF(x)    \
  (struct bufS)   \
  {               \
    &x, sizeof(x) \
  }
// Array Buffer
#define ABUF(x)  \
  (struct bufS)  \
  {              \
    x, sizeof(x) \
  }

// Summary of savechar
struct summaryS {
  int invalid;
  int slevel;
  int sclass;
  int srace;
  int sdepth;
};

// Platform visualization info
struct vizS {
  char sym;
  uint8_t floor;
  uint8_t light;
  uint8_t dim;
  uint8_t tr;
  uint16_t cr;
  uint8_t fade;
  uint8_t look;
};

// U the player
struct uS {
  int32_t y;
  int32_t x;
  int32_t exp;
  int32_t max_exp;
  int32_t lev;
  int32_t max_dlv;
  int32_t chp;
  int32_t chp_frac;
  int32_t mhp;
  int32_t gold;
  uint8_t melee_confuse;
  uint8_t melee_genocide;
  uint8_t melee_unused1;
  uint8_t melee_unused2;
  // Stateful affects from magic spells
  int32_t ma_ac;
  int32_t mflag;
  // Status
  int32_t food;
  int32_t food_digest;
  uint8_t new_level_flag;
  uint8_t clidx;  // class
  uint8_t ridx;   // race
  uint8_t male;
  // initialized by class/race
  int32_t bth;
  int32_t search;
  int32_t fos;  // Frequency of search
  int32_t disarm;
  int32_t stealth;
  int32_t save;
  int32_t infra;
  int32_t mult_exp;  // multiplier for exp-per-level
  int32_t wt;
  // Spell state
  int32_t cmana;
  int32_t cmana_frac;
  int32_t mmana;
  uint32_t spell_worked;
  // Ranged attack
  int32_t bowth;
  // Social Class
  uint16_t ht;
  uint8_t age;
  uint8_t sc;
  int32_t unused;
};
GAME struct uS uD;

// Calculated bonus; see calc_bonuses()
struct cbS {
  int32_t ptohit;
  int32_t ptodam;
  int32_t pac;
  int32_t ptoac;
  int32_t hide_toac;
  int32_t hide_tohit;
  int32_t hide_todam;
  // Treasure flags from equipment
  int32_t tflag;
  // Other flags
  // int32_t prev_weapon;
  // uint8_t weapon_heavy;
};
GAME struct cbS cbD;

struct countS {
  // Player ailment counters
  int32_t poison;
  int32_t unused_count1;
  int32_t confusion;
  int32_t imagine;
  int32_t rest;
  int32_t paralysis;
  // Player protection counters
  int32_t life_prot;
};
GAME struct countS countD;

struct raceS {
  char* name;          /* Type of race			*/
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
  char* name;              /* type of class		*/
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

struct backgroundS {
  char* info;    /* History information    */
  uint8_t roll;  /* Die roll needed for history  */
  uint8_t chart; /* Table number     */
  uint8_t next;  /* Pointer to next table  */
  uint8_t bonus; /* Bonus to the Social Class+50 */
};

// Only the player has stats
struct statS {
  int8_t max_stat[MAX_A]; /* What is restored */
  int8_t cur_stat[MAX_A]; /* What is natural */
  int8_t mod_stat[MAX_A]; /* Boosted artificially (by wearing something) */
  int8_t use_stat[MAX_A]; /* Play value, see set_use_stat() */
};
GAME struct statS statD;

// Player spell
struct spellS {
  uint8_t splevel;
  uint8_t spmana;
  uint8_t spfail;
  uint8_t spexp;
};

struct panelS {
  int32_t panel_row;
  int32_t panel_row_min;
  int32_t panel_row_max;
  int32_t panel_col;
  int32_t panel_col_min;
  int32_t panel_col_max;
};
DATA struct panelS panelD;

struct caveS {
  uint8_t midx;   // monster
  uint8_t oidx;   // object
  uint8_t fval;   // floor
  uint8_t cflag;  // cave flag (CF_)
};
GAME struct caveS caveD[MAX_HEIGHT][MAX_WIDTH];

struct treasureS {
  char* name;        /* Object name		*/
  uint32_t flags;    /* Special flags	*/
  uint8_t tval;      /* Category number	*/
  uint8_t tchar;     /* Character representation*/
  int16_t p1;        /* Misc. use variable	*/
  int32_t cost;      /* Cost of item		*/
  uint16_t weight;   /* Weight		*/
  int16_t tohit;     /* Plusses to hit	*/
  int16_t todam;     /* Plusses to damage	*/
  int16_t ac;        /* Normal AC		*/
  int16_t toac;      /* Plusses to AC	*/
  uint8_t subval;    /* Sub-category number	*/
  uint8_t damage[2]; /* Damage when hits	*/
  uint8_t level;     /* Level item first found */
};

struct objS {
  int32_t id;
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
  char* name;             /* Descrip of creature	*/
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
  int32_t id;
  uint16_t cidx;  // creature definition
  uint16_t msleep;
  int32_t mspeed;
  int32_t hp;
  uint8_t fy;
  uint8_t fx;
  uint8_t mlit;
  uint8_t mstunned;
  uint8_t mconfused;
  uint8_t msilenced;
};

struct ownerS {
  char* name;
  int16_t max_cost;
  uint8_t max_inflate;
  uint8_t min_inflate;
  uint8_t haggle_per;
  uint8_t owner_race;
  uint8_t insult_max;
};

// Global options
struct globalS {
  int32_t saveslot_class;
  uint32_t zoom_factor;
  uint32_t orientation_lock;
  char pc_renderer[16];
};
// Global data may be cached to disk
// Does not mutate on a game reset
DATA struct globalS globalD;
