

struct classS classD[] = {
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
int level_adj[][MAX_LA] = {
    /*         bth    bthb   device  disarm   save/misc hit  */
    /* Warrior */ {4, 4, 2, 2, 3, 5},
    /* Mage    */ {2, 2, 4, 3, 3, 1},
    /* Priest  */ {2, 2, 4, 3, 3, 3},
    /* Rogue   */ {3, 4, 3, 4, 3, 2},
    /* Ranger  */ {3, 4, 3, 3, 3, 2},
    /* Paladin */ {3, 3, 3, 2, 3, 3},
};

struct raceS raceD[] = {
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

char* spell_nameD[] = {
    /* Mage Spells */
    "Magic Missile",
    "Detect Monsters",
    "Phase Door",
    "Light Area",
    "Cure Light Wounds",
    "Find Hidden Traps/Doors",
    "Stinking Cloud",
    "Confusion",
    "Lightning Bolt",
    "Trap/Door Destruction",
    "Sleep I",
    "Cure Poison",
    "Teleport Self",
    "Remove Curse",
    "Frost Bolt",
    "Turn Stone to Mud",
    "Create Food",
    "Recharge Item I",
    "Sleep II",
    "Polymorph Other",
    "Identify",
    "Sleep III",
    "Fire Bolt",
    "Slow Monster",
    "Frost Ball",
    "Recharge Item II",
    "Teleport Other",
    "Haste Self",
    "Fire Ball",
    "Word of Destruction",
    "Genocide",
    /* Priest Spells, start at index 31 */
    "Detect Evil",
    "Cure Light Wounds",
    "Bless",
    "Remove Fear",
    "Call Light",
    "Find Traps",
    "Detect Doors/Stairs",
    "Slow Poison",
    "Blind Creature",
    "Portal",
    "Cure Medium Wounds",
    "Chant",
    "Sanctuary",
    "Create Food",
    "Remove Curse",
    "Resist Heat and Cold",
    "Neutralize Poison",
    "Orb of Draining",
    "Cure Serious Wounds",
    "Sense Invisible",
    "Protection from Evil",
    "Earthquake",
    "Sense Surroundings",
    "Cure Critical Wounds",
    "Turn Undead",
    "Prayer",
    "Dispel Undead",
    "Heal",
    "Dispel Evil",
    "Glyph of Warding",
    "Holy Word",
};

char stat_nameD[MAX_A][5] = {
    "STR ", "INT ", "WIS ", "DEX ", "CON ", "CHR ",
};
char* stat_lossD[] = {
    "You feel weaker",        "You have trouble thinking clearly",
    "You feel very naive",    "You feel clumsy",
    "Your health is damaged", "Your skin starts to itch",
};
