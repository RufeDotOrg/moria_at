
#define MAX_WIDTH 256
#define MAX_HEIGHT 128
#define SCREEN_HEIGHT 8
#define SCREEN_WIDTH 16
#define STATUS_WIDTH 13
#define MAX_COL (MAX_WIDTH / SCREEN_WIDTH * 2 - 2)
#define MAX_ROW (MAX_HEIGHT / SCREEN_HEIGHT * 2 - 2)

#define FLOOR_NULL 0
#define FLOOR_LIGHT 1
#define FLOOR_DARK 2
#define FLOOR_CORR 3
#define MAX_OPEN_SPACE 3
#define MIN_CLOSED_SPACE 4
#define FLOOR_OBST 4 /* a corridor space with cl/st/se door or rubble */
#define TMP1_WALL	8
#define TMP2_WALL	9
#define MIN_WALL 12
#define GRANITE_WALL 12
#define BOUNDARY_WALL	15

#define DUN_TUN_RND	  9   /* 1/Chance of Random direction	       */
#define DUN_TUN_CHG	 70   /* Chance of changing direction (99 max) */
#define DUN_TUN_CON	 15   /* Chance of extra tunneling	       */
#define DUN_TUN_PEN	 25   /* % chance of room doors		       */
#define DUN_TUN_JCT	 15   /* % chance of doors at tunnel junctions */
#define DUN_ROOM_MEAN 64
#define TRUE 1
#define FALSE 0

#define TV_MIN_DOORS	104
#define TV_OPEN_DOOR	104
#define TV_CLOSED_DOOR	105
#define TV_UP_STAIR	107
#define TV_DOWN_STAIR	108
#define TV_SECRET_DOOR	109

#define MODE_DFLT 0
#define MODE_MAP 1

/* Creature constants						*/
#define MIN_MALLOC_LEVEL   14 /* Minimum number of monsters/level      */
#define MAX_MON_NATTACK 1

#define BTH_PLUS_ADJ 3 // base-to-hit per plus-to-hit
