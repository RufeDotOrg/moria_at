
// Visual info
DATA struct vizS vizD[SYMMAP_HEIGHT][SYMMAP_WIDTH];
DATA int vitalD[8];
DATA int vital_statD[6];
DATA int vital_usedD;
DATA int active_affectD[15];

// Full-screen & overlay (choice)
DATA char screenD[22][98];
DATA int screen_usedD[AL(screenD)];
DATA int screen_submodeD;
DATA char overlayD[22][80];
DATA int overlay_usedD[AL(overlayD)];
DATA int overlay_submodeD;
DATA uint8_t minimapD[MAX_HEIGHT][MAX_WIDTH];
DATA int minimap_enlargeD;
DATA uint32_t zoom_factorD;

// Gameplay messages write to a circular queue
DATA char msg_cqD[MAX_MSG][98];
DATA char msglen_cqD[MAX_MSG];
DATA uint32_t msg_writeD;
DATA uint32_t msg_moreD;

// Magic affect counters & names
DATA int32_t maD[MA_COUNT];
DATA uint8_t spell_orderD[32];
DATA uint8_t spell_chanceD[32];

DATA uint32_t player_exp[MAX_PLAYER_LEVEL] = {
    10,      25,       45,       70,       100,      140,     200,
    280,     380,      500,      650,      850,      1100,    1400,
    1800,    2300,     2900,     3600,     4400,     5400,    6800,
    8400,    10200,    12500,    17500,    25000,    35000L,  50000L,
    75000L,  100000L,  150000L,  200000L,  300000L,  400000L, 500000L,
    750000L, 1500000L, 2500000L, 5000000L, 10000000L};

/* this table is used to generate a psuedo-normal distribution.   See the
   function randnor() in misc1.c, this is much faster than calling
   transcendental function to calculate a true normal distribution */
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
DATA uint32_t rnd_seed;
DATA uint32_t obj_seed;
DATA uint32_t town_seed;
DATA int turnD;
DATA int player_hpD[AL(player_exp)];
DATA int death;
DATA int total_winner;
DATA int save_exit_flag;
DATA int dun_level;
DATA char dun_descD[16];
DATA int turn_flag;
DATA int new_level_flag;
DATA int pack_heavy;
DATA char descD[98];
DATA char detailD[98];
DATA char death_descD[98];
DATA int ylookD;
DATA int xlookD;

ARR_REUSE(obj, 256);
ARR_REUSE(mon, 256);

// Known refers to stackable treasures that are instanced
// Distinct from identification which is PER object
DATA uint8_t knownD[7][MAX_SUBVAL];
// Inventory of object IDs; obj_get(id)
// Zero is an available or empty slot
// [INVEN_WIELD, INVEN_AUX] are equipment
DATA int invenD[MAX_INVEN];
DATA int storeD[MAX_STORE];
DATA struct objS store_objD[MAX_STORE][MAX_STORE_INVEN];
DATA char versionD[] = "XXXX.YYYY.ZZZZ";
DATA char git_hashD[] = "AbCdEfGhIjKlMnO";

// Optional UI state
DATA uint8_t ui_stateD[256];

// load/save
DATA struct bufS save_bufD[] = {
    BUF(countD),        BUF(dun_level),   ABUF(entity_objD), ABUF(invenD),
    ABUF(knownD),       ABUF(maD),        ABUF(objD),        BUF(obj_usedD),
    ABUF(player_hpD),   BUF(rnd_seed),    BUF(town_seed),    BUF(obj_seed),
    BUF(statD),         ABUF(store_objD), BUF(turnD),        BUF(uD),
    ABUF(spell_orderD), ABUF(ui_stateD),
};
DATA int savechar_v000[AL(save_bufD)] = {
    28, 4, 10240, 136, 448, 56, 1024, 4, 160, 4, 4, 4, 24, 3840, 4, 100, 0, 0,
};
#define SAVESUM000 16080
DATA int savechar_v001[] = {
    28, 4, 10240, 136, 448, 76, 1024, 4, 160, 4, 4, 4, 24, 3840, 4, 116, 32, 0,
};
#define SAVESUM001 16148
DATA int savechar_v002[] = {
    28, 4, 10240, 140, 448, 76, 1024, 4, 160, 4, 4, 4, 24, 3840, 4, 128, 32, 0,
};
#define SAVESUM002 16164
DATA int savechar_v003[] = {
    28, 4, 10240, 140, 448,  76, 1024, 4,  160,
    4,  4, 4,     24,  3840, 4,  128,  32, 256,
};
#define SAVESUM003 16420
DATA int savesumD[] = {SAVESUM000, SAVESUM001, SAVESUM002, SAVESUM003};
DATA int *savefieldD[] = {
    savechar_v000,
    savechar_v001,
    savechar_v002,
    savechar_v003,
};
