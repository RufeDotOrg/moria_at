

static struct classS classD[] = {
    {"Warrior", 9, 25, 14, 1, 38, 70, 55, 18, {5, -2, -2, 2, 2, -1}, 0, 0, 0},
    {"Mage",
     0,
     30,
     16,
     2,
     20,
     34,
     20,
     36,
     {-5, 3, 0, 1, -2, 1},
     SP_MAGE,
     30,
     1},
    {"Priest",
     2,
     25,
     16,
     2,
     32,
     48,
     35,
     30,
     {-3, -3, 3, -1, 0, 2},
     SP_PRIEST,
     20,
     1},
    {"Rogue",
     6,
     45,
     32,
     5,
     16,
     60,
     66,
     30,
     {2, 1, -2, 3, 1, -1},
     SP_MAGE,
     0,
     5},
    {"Ranger",
     4,
     30,
     24,
     3,
     24,
     56,
     72,
     30,
     {2, 2, 0, 1, 1, 1},
     SP_MAGE,
     40,
     3},
    {"Paladin",
     6,
     20,
     12,
     1,
     38,
     68,
     40,
     24,
     {3, -3, 1, 0, 2, 2},
     SP_PRIEST,
     35,
     1},
};

// level adjustment
static int level_adj[][MAX_LA] = {
    /*         bth    bthb   device  disarm   save/misc hit  */
    /* Warrior */ {4, 4, 2, 2, 3},
    /* Mage    */ {2, 2, 4, 3, 3},
    /* Priest  */ {2, 2, 4, 3, 3},
    /* Rogue   */ {3, 4, 3, 4, 3},
    /* Ranger  */ {3, 4, 3, 3, 3},
    /* Paladin */ {3, 3, 3, 2, 3},
};

static struct raceS raceD[] = {
    {
        "Human", {0, 0, 0, 0, 0, 0},
        14,      6,
        72,      6,
        180,     25,
        66,      4,
        150,     20,
        0,       0,
        0,       0,
        0,       0,
        0,       10,
        0,       100,
        0x3F,
    },
    {
        "Half-Elf", {-1, 1, 0, 1, -1, 1},
        24,         16,
        66,         6,
        130,        15,
        62,         6,
        100,        10,
        2,          6,
        1,          -1,
        -1,         5,
        3,          9,
        2,          110,
        0x3F,
    },
    {
        "Elf", {-1, 2, 1, 1, -2, 1},
        75,    75,
        60,    4,
        100,   6,
        54,    4,
        80,    6,
        5,     8,
        1,     -2,
        -5,    15,
        6,     8,
        3,     120,
        0x1F,
    },
    {
        "Halfling", {-2, 2, 1, 3, 1, 1},
        21,         12,
        36,         3,
        60,         3,
        33,         3,
        50,         3,
        15,         12,
        4,          -5,
        -10,        20,
        18,         6,
        4,          110,
        0x0B,
    },
    {
        "Gnome", {-1, 2, 0, 2, 1, -2},
        50,      40,
        42,      3,
        90,      6,
        39,      3,
        75,      3,
        10,      6,
        3,       -3,
        -8,      12,
        12,      7,
        4,       125,
        0x0F,
    },
    {
        "Dwarf", {2, -3, 1, -2, 2, -3},
        35,      15,
        48,      3,
        150,     10,
        46,      3,
        120,     10,
        2,       7,
        -1,      0,
        15,      0,
        9,       9,
        5,       120,
        0x05,
    },
    {
        "Half-Orc", {2, -1, 0, 0, 1, -4},
        11,         4,
        66,         1,
        150,        5,
        62,         1,
        120,        5,
        -3,         0,
        -1,         3,
        12,         -5,
        -3,         10,
        3,          110,
        0x0D,
    },
    {
        "Half-Troll",
        {4, -4, -2, -4, 3, -6},
        20,
        10,
        96,
        10,
        255,
        50,
        84,
        8,
        225,
        40,
        -5,
        -1,
        -2,
        5,
        20,
        -10,
        -8,
        12,
        3,
        120,
        0x05,
    },
};

static char* spell_nameD[] = {
    /* Mage Spells */
    "Magic Missile",     "Detect Monsters",
    "Phase Door",        "Light Area",
    "Cure Light Wounds", "Find Hidden Traps/Doors",
    "Stinking Cloud",    "Confuse Monster",
    "Lightning Bolt",    "Trap/Door/Lock Destruction",
    "Sleep Bolt",        "Cure Poison",
    "Teleport Self",     "Remove Curse",
    "Frost Bolt",        "Turn Stone to Mud",
    "Create Food",       "Recharge Item (Reckless)",
    "Sleep Adjacent",    "Polymorph Other",
    "Identify",          "Sleep Area",
    "Fire Bolt",         "Slow Monster",
    "Frost Ball",        "Recharge Item (Rigorous)",
    "Teleport Other",    "Haste Self",
    "Fire Ball",         "Word of Destruction",
    "Genocide",
};
static char* prayer_nameD[] = {
    /* Priest Prayers */
    "Detect Evil",
    "Cure Light Wounds",
    "Bless",
    "Remove Fear",
    "Call Light",
    "Find Traps",
    "Detect Doors/Stairs",
    "Slow Poison",
    "Confuse Monster",
    "Phase Door",
    "Cure Medium Wounds",
    "Enduring Blessing",
    "Divine Sleep",
    "Create Food",
    "Remove Curse",
    "Resist Heat and Cold",
    "Neutralize Poison",
    "Orb of Draining",
    "Cure Serious Wounds",
    "Sense Invisible",
    "Protection from Evil",
    "Earthquake",
    "Magic Mapping",
    "Cure Critical Wounds",
    "Turn Undead",
    "Divine Blessing",
    "Dispel Undead",
    "Heal",
    "Dispel Evil",
    "Glyph of Warding",
    "Holy Word",
};
struct spellS spellD[AL(classD) - 1][SP_MAX] = {
    {/* Mage     */
     {1, 1, 22, 1},    {1, 1, 23, 1},     {1, 2, 24, 1},    {1, 2, 26, 1},
     {3, 3, 25, 2},    {3, 3, 25, 1},     {3, 3, 27, 2},    {3, 4, 30, 1},
     {5, 4, 30, 6},    {5, 4, 30, 8},     {5, 5, 30, 5},    {5, 5, 35, 6},
     {7, 6, 35, 9},    {7, 6, 50, 10},    {7, 6, 40, 12},   {9, 7, 44, 19},
     {9, 7, 45, 19},   {9, 7, 50, 22},    {9, 7, 45, 19},   {11, 7, 45, 25},
     {11, 7, 99, 19},  {13, 7, 50, 22},   {15, 9, 50, 25},  {17, 9, 50, 31},
     {19, 12, 55, 38}, {21, 12, 90, 44},  {23, 12, 60, 50}, {25, 12, 65, 63},
     {29, 18, 65, 88}, {33, 21, 80, 125}, {37, 25, 95, 200}},
    {/* Priest     */
     {1, 1, 10, 1},    {1, 2, 15, 1},     {1, 2, 20, 1},    {1, 2, 25, 1},
     {3, 2, 25, 1},    {3, 3, 27, 2},     {3, 3, 27, 2},    {3, 3, 28, 3},
     {5, 4, 29, 4},    {5, 4, 30, 5},     {5, 4, 32, 5},    {5, 5, 34, 5},
     {7, 5, 36, 6},    {7, 5, 38, 7},     {7, 6, 38, 9},    {7, 7, 38, 9},
     {9, 6, 38, 10},   {9, 7, 38, 10},    {9, 7, 40, 10},   {11, 8, 42, 10},
     {11, 8, 42, 12},  {11, 9, 55, 15},   {13, 10, 45, 15}, {13, 11, 45, 16},
     {15, 12, 50, 20}, {15, 14, 50, 22},  {17, 14, 55, 32}, {21, 16, 60, 38},
     {25, 20, 70, 75}, {33, 24, 90, 125}, {39, 32, 80, 200}},
    {/* Rogue     */
     {99, 99, 0, 0},   {5, 1, 50, 1},    {7, 2, 55, 1},  {9, 3, 60, 2},
     {11, 4, 65, 2},   {13, 5, 70, 3},   {99, 99, 0, 0}, {15, 6, 75, 3},
     {99, 99, 0, 0},   {17, 7, 80, 4},   {19, 8, 85, 5}, {21, 9, 90, 6},
     {99, 99, 0, 0},   {23, 10, 95, 7},  {99, 99, 0, 0}, {99, 99, 0, 0},
     {25, 12, 95, 9},  {27, 15, 95, 11}, {99, 99, 0, 0}, {99, 99, 0, 0},
     {29, 18, 99, 19}, {99, 99, 0, 0},   {99, 99, 0, 0}, {99, 99, 0, 0},
     {99, 99, 0, 0},   {99, 99, 0, 0},   {99, 99, 0, 0}, {99, 99, 0, 0},
     {99, 99, 0, 0},   {99, 99, 0, 0},   {99, 99, 0, 0}},
    {/* Ranger      */
     {3, 1, 30, 1},    {3, 2, 35, 2},     {3, 2, 35, 2},    {5, 3, 35, 2},
     {5, 3, 40, 2},    {5, 4, 45, 3},     {7, 5, 40, 6},    {7, 6, 40, 5},
     {9, 7, 40, 7},    {9, 8, 45, 8},     {11, 8, 40, 10},  {11, 9, 45, 10},
     {13, 10, 45, 12}, {13, 11, 55, 13},  {15, 12, 50, 15}, {15, 13, 50, 15},
     {17, 17, 55, 15}, {17, 17, 60, 17},  {21, 17, 55, 17}, {21, 19, 60, 18},
     {23, 25, 95, 20}, {23, 20, 60, 20},  {25, 20, 60, 20}, {25, 21, 65, 20},
     {27, 21, 65, 22}, {29, 23, 95, 23},  {31, 25, 70, 25}, {33, 25, 75, 38},
     {35, 25, 80, 50}, {37, 30, 95, 100}, {99, 99, 0, 0}},
    {/* Paladin     */
     {1, 1, 30, 1},    {2, 2, 35, 2},    {3, 3, 35, 3},    {5, 3, 35, 5},
     {5, 4, 35, 5},    {7, 5, 40, 6},    {7, 5, 40, 6},    {9, 7, 40, 7},
     {9, 7, 40, 8},    {9, 8, 40, 8},    {11, 9, 40, 10},  {11, 10, 45, 10},
     {11, 10, 45, 10}, {13, 10, 45, 12}, {13, 11, 45, 13}, {15, 13, 45, 15},
     {15, 15, 50, 15}, {17, 15, 50, 17}, {17, 15, 50, 18}, {19, 15, 50, 19},
     {19, 15, 50, 19}, {21, 17, 50, 20}, {23, 17, 50, 20}, {25, 20, 50, 20},
     {27, 21, 50, 22}, {29, 22, 50, 24}, {31, 24, 60, 25}, {33, 28, 60, 31},
     {35, 32, 70, 38}, {37, 36, 90, 50}, {39, 38, 90, 100}}};

static char stat_abbrD[MAX_A][5] = {
    "STR ", "INT ", "WIS ", "DEX ", "CON ", "CHR ",
};
static char vital_nameD[8][5] = {
    "LEV ", "EXP ", "MANA", "MHP ", "CHP ", "AC  ", "GOLD", "",
};
static char* stat_lossD[] = {
    "You feel weaker",      "You have trouble thinking clearly",
    "You feel very naive",  "You feel clumsy",
    "Your body is damaged", "Your skin starts to itch",
};
static char* stat_gainD[] = {
    "Wow!  What bulging muscles",
    "Aren't you brilliant",
    "You suddenly have a profound thought",
    "You feel more limber",
    "Your body feels tingly",
    "Gee, ain't you cute",
};
static char* stat_nameD[] = {
    "strength",  "intelligence", "wisdom",
    "dexterity", "constitution", "charisma",
};
static char* affectD[][3] = {
    {"Recall"},
    {"Hungry", "Starving"},
    {"Burdened"},
    {"Fast", "Fast (2)"},
    {"Slow", "Slow (2)"},
    {"Blind"},
    {"Hero", "Hero (2)", "Hero (3)"},
    {"Afraid"},
    {"Confused"},
    {"SeeInvis"},
    {"Paralyse"},
    {"Poison"},
    {"+Spell"},
    {"Imagine"},
    {""},
};
