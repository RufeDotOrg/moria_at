// Rufe.org LLC 2022-2025: GPLv3 License

// Visual info
DATA struct vizS vizD[SYMMAP_HEIGHT][SYMMAP_WIDTH];
DATA int vitalD[8];
DATA int vital_statD[6];
DATA int vital_usedD;
DATA int active_affectD[15];

// Full-screen & overlay (choice)
// display buffers are NOT null terminated
DATA char screenD[22][98];
GAME int screen_usedD[AL(screenD)];
DATA int screen_submodeD;
DATA char overlayD[22][98];
GAME int overlay_usedD[AL(overlayD)];
DATA int overlay_submodeD;
DATA char stattextD[14][28];

DATA int minimapD[MAX_HEIGHT][MAX_WIDTH];
DATA int minimap_enlargeD;
DATA int save_on_readyD;
DATA int seed_changeD;

// Savechar folder
DATA char savepathD[1024];
DATA int savepath_usedD;
DATA char exportpathD[1024];
DATA int exportpath_usedD;

// Gameplay messages write to a circular queue
DATA char msg_cqD[MAX_MSG][98];
DATA char msglen_cqD[MAX_MSG];
DATA uint32_t msg_writeD;
DATA uint32_t msg_moreD;
DATA int overlay_widthD;
DATA int msg_widthD;
DATA int msg_turnD;
DATA int last_turnD;

// Character social class history text
DATA char historyD[256];

// Monster memory
DATA char monmemD[1024];
DATA struct recallS recallD[MAX_CREATURE];

// Magic affect counters & names
GAME int32_t maD[MA_COUNT];
GAME uint8_t spell_orderD[32];
GAME uint8_t spell_chanceD[32];

DATA uint32_t player_exp[MAX_PLAYER_LEVEL] = {
    10,      25,       45,       70,       100,      140,     200,
    280,     380,      500,      650,      850,      1100,    1400,
    1800,    2300,     2900,     3600,     4400,     5400,    6800,
    8400,    10200,    12500,    17500,    25000,    35000L,  50000L,
    75000L,  100000L,  150000L,  200000L,  300000L,  400000L, 500000L,
    750000L, 1500000L, 2500000L, 5000000L, 10000000L};

// this table is used to generate a psuedo-normal distribution.
DATA int16_t normal_table[] = {
    206,   613,   1022,  1430,  1838,  2245,  2652,  3058,  3463,  3867,  4271,
    4673,  5075,  5475,  5874,  6271,  6667,  7061,  7454,  7845,  8234,  8621,
    9006,  9389,  9770,  10148, 10524, 10898, 11269, 11638, 12004, 12367, 12727,
    13085, 13440, 13792, 14140, 14486, 14828, 15168, 15504, 15836, 16166, 16492,
    16814, 17133, 17449, 17761, 18069, 18374, 18675, 18972, 19266, 19556, 19842,
    20124, 20403, 20678, 20949, 21216, 21479, 21738, 21994, 22245, 22493, 22737,
    22977, 23213, 23446, 23674, 23899, 24120, 24336, 24550, 24759, 24965, 25166,
    25365, 25559, 25750, 25937, 26120, 26300, 26476, 26649, 26818, 26983, 27146,
    27304, 27460, 27612, 27760, 27906, 28048, 28187, 28323, 28455, 28585, 28711,
    28835, 28955, 29073, 29188, 29299, 29409, 29515, 29619, 29720, 29818, 29914,
    30007, 30098, 30186, 30272, 30356, 30437, 30516, 30593, 30668, 30740, 30810,
    30879, 30945, 31010, 31072, 31133, 31192, 31249, 31304, 31358, 31410, 31460,
    31509, 31556, 31601, 31646, 31688, 31730, 31770, 31808, 31846, 31882, 31917,
    31950, 31983, 32014, 32044, 32074, 32102, 32129, 32155, 32180, 32205, 32228,
    32251, 32273, 32294, 32314, 32333, 32352, 32370, 32387, 32404, 32420, 32435,
    32450, 32464, 32477, 32490, 32503, 32515, 32526, 32537, 32548, 32558, 32568,
    32577, 32586, 32595, 32603, 32611, 32618, 32625, 32632, 32639, 32645, 32651,
    32657, 32662, 32667, 32672, 32677, 32682, 32686, 32690, 32694, 32698, 32702,
    32705, 32708, 32711, 32714, 32717, 32720, 32722, 32725, 32727, 32729, 32731,
    32733, 32735, 32737, 32739, 32740, 32742, 32743, 32745, 32746, 32747, 32748,
    32749, 32750, 32751, 32752, 32753, 32754, 32755, 32756, 32757, 32757, 32758,
    32758, 32759, 32760, 32760, 32761, 32761, 32761, 32762, 32762, 32763, 32763,
    32763, 32764, 32764, 32764, 32764, 32765, 32765, 32765, 32765, 32766, 32766,
    32766, 32766, 32766,
};

DATA uint8_t blows_table[][6] = {
    /* STR/W: 9 18 67 107 117 118   : DEX */
    /* <2 */ {1, 1, 1, 1, 1, 1},
    /* <3 */ {1, 1, 1, 1, 2, 2},
    /* <4 */ {1, 1, 1, 2, 2, 3},
    /* <5 */ {1, 1, 2, 2, 3, 3},
    /* <7 */ {1, 2, 2, 3, 3, 4},
    /* <9 */ {1, 2, 2, 3, 4, 4},
    /* >9 */ {2, 2, 3, 3, 4, 4},
};

// Game
GAME uint32_t rnd_seed;
GAME uint32_t obj_seed;
GAME uint32_t town_seed;
GAME int turnD;
GAME int player_hpD[AL(player_exp)];
GAME int dun_level;
GAME int turn_flag;
GAME int pack_heavy;
GAME int last_actuateD;
GAME int last_castD;
GAME int death_creD;

// null terminated
DATA char descD[80 - 4];
DATA char detailD[80 - 4];
DATA char death_descD[80];
DATA char dun_descD[16];

DATA int ylookD;
DATA int xlookD;

// Rendering / visualization extensions
DATA fn viz_hookD;

// Known refers to stackable treasures that are instanced
// Distinct from identification which is PER object
GAME uint8_t knownD[7][MAX_SUBVAL];
// Inventory of object IDs; obj_get(id)
// Zero is an available or empty slot
// [INVEN_WIELD, INVEN_AUX] are equipment
GAME int invenD[MAX_INVEN];
GAME int storeD[MAX_STORE];
GAME struct objS store_objD[MAX_STORE][MAX_STORE_INVEN];

DATA char quit_stringD[] = "quitting";

// Optional UI state
GAME uint8_t ui_stateD[256];
DATA int modeD;
DATA int submodeD;
DATA int blipD;
DATA uint8_t finger_rowD;
DATA uint8_t finger_colD;

// Replay
DATA int mplostD;
DATA int replay_flag;
DATA int replay_desync;
DATA char input_recordD[4 * 1024];
DATA uint32_t input_record_writeD;
DATA uint32_t input_record_readD;
DATA int input_resumeD;
// Could optimize to uint8_t using record count
DATA int16_t input_actionD[2 * 1024];
DATA int input_action_usedD;

// Run
GAME int find_threatD;
GAME int find_flagD;
// Cavegen
GAME point_t doorstk[100];
GAME int doorindex;

// Misc
DATA int cycle[] = {1, 2, 3, 6, 9, 8, 7, 4, 1, 2, 3, 6, 9, 8, 7, 4, 1};
DATA int chome[] = {-1, 8, 9, 10, 7, -1, 11, 6, 5, 4};
DATA int find_directionD;
DATA int find_openareaD;
DATA int find_breakrightD, find_breakleftD;
DATA int find_prevdirD;
DATA int magick_distD;
DATA point_t magick_locD;
DATA int magick_hituD;
DATA char vtypeD[STRLEN_MSG + 1];
DATA int oprefixD = 0;
DATA char* mon_spell_nameD[] = {
    "phase door",    "teleport",  "summon",     "wounds", "serious wounds",
    "paralysis",     "blindness", "confusion",  "fear",   "summon monster",
    "summon undead", "slow",      "drain mana",
};
