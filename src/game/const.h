
#define MAX_WIDTH 256
#define MAX_HEIGHT 128
#define SCREEN_HEIGHT 8
#define SCREEN_WIDTH 16
#define STATUS_WIDTH 13
#define MAX_COL (MAX_WIDTH / SCREEN_WIDTH * 2)
#define MAX_ROW (MAX_HEIGHT / SCREEN_HEIGHT * 2)

#define MODE_DFLT 0
#define MODE_MAP 1

#define FLOOR_NULL 0
#define FLOOR_LIGHT 1
#define FLOOR_DARK 2
#define FLOOR_CORR 3
#define MAX_OPEN_SPACE 3
#define MIN_CLOSED_SPACE 4
#define FLOOR_OBST 4 /* a corridor space with cl/st/se door or rubble */
#define MAX_FLOOR 4
#define TMP1_WALL 8
#define TMP2_WALL 9
#define MIN_WALL 12
#define GRANITE_WALL 12
#define BOUNDARY_WALL 15

#define DUN_TUN_RND 9  /* 1/Chance of Random direction	       */
#define DUN_TUN_CHG 70 /* Chance of changing direction (99 max) */
#define DUN_TUN_CON 15 /* Chance of extra tunneling	       */
#define DUN_TUN_PEN 25 /* % chance of room doors		       */
#define DUN_TUN_JCT 15 /* % chance of doors at tunnel junctions */
#define DUN_ROOM_MEAN 64 // TBD: retune - originally 32. map dimensions are modified
#define TREAS_ROOM_MEAN 14 // TBD: retune
#define TRUE 1
#define FALSE 0

/* Treasure constants */
#define TV_NOTHING	0
#define TV_MISC		1
#define TV_CHEST	2
/* min tval for wearable items, all items between TV_MIN_WEAR and TV_MAX_WEAR
   use the same flag bits, see the TR_* defines */
#define TV_MIN_WEAR	10
/* items tested for enchantments, i.e. the MAGIK inscription, see the
   enchanted() procedure */
#define TV_MIN_ENCHANT	10
#define TV_SLING_AMMO	10
#define TV_BOLT		11
#define TV_ARROW	12
#define TV_SPIKE	13
#define TV_LIGHT	15
#define TV_BOW		20
#define TV_HAFTED	21
#define TV_POLEARM	22
#define TV_SWORD	23
#define TV_DIGGING	25
#define TV_BOOTS	30
#define TV_GLOVES	31
#define TV_CLOAK	32
#define TV_HELM		33
#define TV_SHIELD	34
#define TV_HARD_ARMOR	35
#define TV_SOFT_ARMOR	36
/* max tval that uses the TR_* flags */
#define TV_MAX_ENCHANT	39
#define TV_AMULET	40
#define TV_RING		45
/* max tval for wearable items */
#define TV_MAX_WEAR	50
#define TV_STAFF	55
#define TV_WAND		65
#define TV_SCROLL1	70
#define TV_SCROLL2	71
#define TV_POTION1	75
#define TV_POTION2	76
#define TV_FLASK	77
#define TV_FOOD 	80
#define TV_MAGIC_BOOK	90
#define TV_PRAYER_BOOK	91
/* objects with tval above this are never picked up by monsters */
#define TV_MAX_OBJECT	99
#define TV_GOLD		100
// Items above cannot be lifted by the player
#define TV_MAX_PICK_UP 100
#define TV_INVIS_TRAP 101
#define TV_VIS_TRAP 102
#define TV_RUBBLE 103
#define TV_MIN_DOORS 104
#define TV_OPEN_DOOR 104
#define TV_CLOSED_DOOR 105
#define TV_UP_STAIR 107
#define TV_DOWN_STAIR 108
#define TV_SECRET_DOOR 109

#define MAX_TRAP 3
#define MAX_OBJ_LEVEL 50

#define OBJ_GREAT 12 // 1/x treasure is a big surprise!

/* Creature constants						*/
#define MIN_MALLOC_LEVEL 14 /* Minimum number of monsters/level      */
#define MAX_MON_LEVEL 40
#define MON_NASTY 50 // 1/x monsters are a big baddie
#define MAX_WIN_MON 2

/* Player constants */
#define MAX_PLAYER_LEVEL 40
#define MAX_EXP 999999
#define BTH_PLUS_ADJ 3  // base-to-hit per plus-to-hit

// Input
#define ESCAPE '\033'
