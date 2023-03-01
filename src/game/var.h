
// load/save
EXTERN void **save_addr_ptrD;
EXTERN int *save_len_ptrD;
EXTERN char **save_name_ptrD;
EXTERN int save_field_countD;

// Visual info
EXTERN struct vizS vizD[SYMMAP_HEIGHT][SYMMAP_WIDTH];
EXTERN char vitalinfoD[22][13];
EXTERN int vitalinfo_usedD[AL(vitalinfoD)];
EXTERN char affectinfoD[160];
EXTERN int affectinfo_usedD;

// Full-screen & overlay (choice)
EXTERN char screenD[22][80];
EXTERN int screen_usedD[AL(screenD)];
EXTERN char overlayD[22][80];
EXTERN int overlay_usedD[AL(overlayD)];
EXTERN int overlay_submodeD;
EXTERN uint8_t minimapD[MAX_HEIGHT][MAX_WIDTH];
EXTERN int minimap_enlargeD;
EXTERN uint32_t zoom_factorD;

// Gameplay messages write to a circular queue
EXTERN char msg_cqD[MAX_MSG][80];
EXTERN char msglen_cqD[MAX_MSG];
EXTERN uint32_t msg_writeD;
EXTERN uint32_t msg_countD;

// Magic affect counters & names
static int maD[MA_COUNT];

uint32_t player_exp[MAX_PLAYER_LEVEL] = {
    10,      25,       45,       70,       100,      140,     200,
    280,     380,      500,      650,      850,      1100,    1400,
    1800,    2300,     2900,     3600,     4400,     5400,    6800,
    8400,    10200,    12500,    17500,    25000,    35000L,  50000L,
    75000L,  100000L,  150000L,  200000L,  300000L,  400000L, 500000L,
    750000L, 1500000L, 2500000L, 5000000L, 10000000L};

/* this table is used to generate a psuedo-normal distribution.   See the
   function randnor() in misc1.c, this is much faster than calling
   transcendental function to calculate a true normal distribution */
int16_t normal_table[] = {
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

uint8_t blows_table[][6] = {
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
static uint32_t rnd_seed;
static uint32_t obj_seed;
static uint32_t town_seed;
static int turnD;
static int player_hpD[AL(player_exp)];
static int death;
static int total_winner;
static int save_exit_flag;
static int dun_level;
static int turn_flag;
static int new_level_flag;
static int pack_heavy;
static int light_bonus;
static char descD[160];
static char death_descD[160];
static int miningD[2];

ARR_REUSE(obj, 256);
ARR_REUSE(mon, 256);

// Known refers to stackable treasures that are instanced
// Distinct from identification which is PER object
static uint8_t knownD[7][MAX_SUBVAL];
// Inventory of object IDs; obj_get(id)
// Zero is an available or empty slot
// [INVEN_WIELD, INVEN_AUX] are equipment
static int invenD[MAX_INVEN];
static int storeD[MAX_STORE];
EXTERN struct objS store_objD[MAX_STORE][MAX_STORE_INVEN];
static char versionD[] = "XXXX.YYYY.ZZZZ";
static char git_hashD[] = "AbCdEfGhIjKlMnO";
