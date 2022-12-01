
#define MAX_WIDTH 256
#define MAX_HEIGHT 128
#define SYMMAP_HEIGHT 16
#define SYMMAP_WIDTH 32
#define MAX_COL (MAX_WIDTH / SYMMAP_WIDTH * 2)
#define MAX_ROW (MAX_HEIGHT / SYMMAP_HEIGHT * 2)
#define STATUS_HEIGHT 22
#define STATUS_WIDTH 13
#define SCREEN_HEIGHT 23

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

// Cave flags
#define CF_ROOM 0x1
#define CF_TEMP_LIGHT 0x2
#define CF_PERM_LIGHT 0x4
#define CF_FIELDMARK 0x8  // Object bypasses normal visibility rules

#define CHUNK_HEIGHT 16
#define CHUNK_WIDTH 32
#define CHUNK_COL (MAX_HEIGHT / CHUNK_HEIGHT)
#define CHUNK_ROW (MAX_WIDTH / CHUNK_WIDTH)

#define ROOM_WIDTH 11
#define ROOM_HEIGHT 4
#define DUN_TUN_RND 9       /* 1/Chance of Random direction	       */
#define DUN_TUN_CHG 70      /* Chance of changing direction (99 max) */
#define DUN_TUN_CON 15      /* Chance of extra tunneling	       */
#define DUN_TUN_PEN 25      /* % chance of room doors		       */
#define DUN_TUN_JCT 15      /* % chance of doors at tunnel junctions */
#define DUN_ROOM_MEAN 64    // TBD: retune - originally 32
#define TREAS_ROOM_MEAN 14  // TBD: retune
#define TREAS_GOLD_ALLOC 4  // TBD: retune
#define TRUE 1
#define FALSE 0

/* Treasure constants */
#define TV_NOTHING 0
#define TV_MISC 1
#define TV_CHEST 2
/* min tval for wearable items, all items between TV_MIN_WEAR and TV_MAX_WEAR
   use the same flag bits, see the TR_* defines */
#define TV_MIN_WEAR 10
/* items tested for enchantments, i.e. the MAGIK inscription, see the
   enchanted() procedure */
#define TV_MIN_ENCHANT 10
#define TV_SLING_AMMO 10
#define TV_BOLT 11
#define TV_ARROW 12
#define TV_SPIKE 13
#define TV_LIGHT 15
#define TV_BOW 20
#define TV_HAFTED 21
#define TV_POLEARM 22
#define TV_SWORD 23
#define TV_DIGGING 25
#define TV_BOOTS 30
#define TV_GLOVES 31
#define TV_CLOAK 32
#define TV_HELM 33
#define TV_SHIELD 34
#define TV_HARD_ARMOR 35
#define TV_SOFT_ARMOR 36
/* max tval that uses the TR_* flags */
#define TV_MAX_ENCHANT 39
#define TV_AMULET 40
#define TV_RING 45
/* max tval for wearable items */
#define TV_MAX_WEAR 50
#define TV_STAFF 55
#define TV_WAND 65
#define TV_SCROLL1 70
#define TV_SCROLL2 71
#define TV_POTION1 75
#define TV_POTION2 76
#define TV_FLASK 77
#define TV_FOOD 80
#define TV_MAGIC_BOOK 90
#define TV_PRAYER_BOOK 91
/* objects with tval above this are never picked up by monsters */
#define TV_MAX_OBJECT 99
#define TV_GOLD 100
// Items above cannot be lifted by the player
#define TV_MAX_PICK_UP 100
#define TV_INVIS_TRAP 101
/* objects between TV_MIN_VISIBLE and TV_MAX_VISIBLE are always visible,
   i.e. the cave fm flag is set when they are present */
#define TV_MIN_VISIBLE 102
#define TV_VIS_TRAP 102
#define TV_RUBBLE 103
#define TV_MIN_DOORS 104
#define TV_OPEN_DOOR 104
#define TV_CLOSED_DOOR 105
#define TV_UP_STAIR 107
#define TV_DOWN_STAIR 108
#define TV_SECRET_DOOR 109
#define TV_MAX_VISIBLE 110

#define MAX_TRAP 3
#define MAX_OBJ_LEVEL 50

#define OBJ_GREAT 12  // 1/x treasure is a big surprise!

// Wearable obj flags
#define TR_STATS 0x0000003FL /* the stats must be the low 6 bits */
#define TR_STR 0x00000001L
#define TR_INT 0x00000002L
#define TR_WIS 0x00000004L
#define TR_DEX 0x00000008L
#define TR_CON 0x00000010L
#define TR_CHR 0x00000020L
#define TR_SEARCH 0x00000040L
#define TR_SLOW_DIGEST 0x00000080L
#define TR_STEALTH 0x00000100L
#define TR_AGGRAVATE 0x00000200L
#define TR_TELEPORT 0x00000400L
#define TR_REGEN 0x00000800L
#define TR_SPEED 0x00001000L

#define TR_EGO_WEAPON 0x0007E000L
#define TR_SLAY_DRAGON 0x00002000L
#define TR_SLAY_ANIMAL 0x00004000L
#define TR_SLAY_EVIL 0x00008000L
#define TR_SLAY_UNDEAD 0x00010000L
#define TR_FROST_BRAND 0x00020000L
#define TR_FLAME_TONGUE 0x00040000L

#define TR_RES_FIRE 0x00080000L
#define TR_RES_ACID 0x00100000L
#define TR_RES_COLD 0x00200000L
#define TR_SUST_STAT 0x00400000L
#define TR_FREE_ACT 0x00800000L
#define TR_SEE_INVIS 0x01000000L
#define TR_RES_LIGHT 0x02000000L
#define TR_FFALL 0x04000000L
#define TR_BLIND 0x08000000L
#define TR_TIMID 0x10000000L
#define TR_TUNNEL 0x20000000L
#define TR_INFRA 0x40000000L
#define TR_CURSED 0x80000000L

/* Creature constants						*/
#define MIN_MALLOC_LEVEL 14 /* Minimum number of monsters/level      */
#define MAX_MON_LEVEL 40
#define MON_NASTY 50     // 1/x monsters are a big baddie
#define MON_DRAIN_LIFE 2 /* Percent of player exp drained per hit */
#define MON_SUMMON_ADJ	    2 /* Adjust level of summoned creatures    */
#define MAX_WIN_MON 2
#define CRE_LEV_ADJ 3
#define MAX_SIGHT 20 /* Maximum dis a creature can be seen    */

/* definitions for creatures, cmove field */
#define CM_ALL_MV_FLAGS 0x0000003FL
#define CM_ATTACK_ONLY 0x00000001L
#define CM_MOVE_NORMAL 0x00000002L
/* For Quylthulgs, which have no physical movement.  */
#define CM_ONLY_MAGIC 0x00000004L

#define CM_RANDOM_MOVE 0x00000038L
#define CM_20_RANDOM 0x00000008L
#define CM_40_RANDOM 0x00000010L
#define CM_75_RANDOM 0x00000020L

#define CM_SPECIAL 0x003F0000L
#define CM_INVISIBLE 0x00010000L
#define CM_OPEN_DOOR 0x00020000L
#define CM_PHASE 0x00040000L
#define CM_EATS_OTHER 0x00080000L
#define CM_PICKS_UP 0x00100000L
#define CM_MULTIPLY 0x00200000L

#define CM_SMALL_OBJ 0x00800000L
#define CM_CARRY_OBJ 0x01000000L
#define CM_CARRY_GOLD 0x02000000L
#define CM_TREASURE 0x7C000000L
#define CM_TR_SHIFT 26 /* used for recall of treasure */
#define CM_60_RANDOM 0x04000000L
#define CM_90_RANDOM 0x08000000L
#define CM_1D2_OBJ 0x10000000L
#define CM_2D2_OBJ 0x20000000L
#define CM_4D2_OBJ 0x40000000L
#define CM_WIN 0x80000000L

/* creature defense flags */
#define CD_DRAGON 0x0001
#define CD_ANIMAL 0x0002
#define CD_EVIL 0x0004
#define CD_UNDEAD 0x0008
#define CD_WEAKNESS 0x03F0
#define CD_FROST 0x0010
#define CD_FIRE 0x0020
#define CD_POISON 0x0040
#define CD_ACID 0x0080
#define CD_LIGHT 0x0100
#define CD_STONE 0x0200

#define CD_NO_SLEEP 0x1000
#define CD_INFRA 0x2000
#define CD_MAX_HP 0x4000

/* Player constants */
#define MAX_PLAYER_LEVEL 40
#define MAX_EXP 999999
#define USE_DEVICE	  3   /* x> Harder devices x< Easier devices   */

#define BTH_PLUS_ADJ 3  // base-to-hit per plus-to-hit

#define PLAYER_FOOD_FULL 10000   /* Getting full			       */
#define PLAYER_FOOD_MAX 15000    /* Maximum food value, beyond is wasted  */
#define PLAYER_FOOD_FAINT 300    /* Character begins fainting	       */
#define PLAYER_FOOD_WEAK 1000    /* Warn player that he is getting very low*/
#define PLAYER_FOOD_ALERT 2000   /* Warn player that he is getting low    */
#define PLAYER_REGEN_FAINT 33    /* Regen factor>>16 when fainting	 */
#define PLAYER_REGEN_WEAK 98     /* Regen factor>>16 when weak	 */
#define PLAYER_REGEN_NORMAL 197  /* Regen factor>>16 when full	 */
#define PLAYER_REGEN_HPBASE 1442 /* Min amount hp regen>>16		 */
#define PLAYER_REGEN_MNBASE 524  /* Min amount mana regen>>16	 */

/* Class spell types */
#define SP_MAGE 1
#define SP_PRIEST 2

/* Class level adjustment */
#define LA_BTH 0
#define LA_BTHB 1
#define LA_DEVICE 2
#define LA_DISARM 3
#define LA_SAVE 4
#define MAX_LA 5

/* magic numbers for players inventory array */
#define INVEN_EQUIP 22
#define INVEN_WIELD 22
#define INVEN_HEAD 23
#define INVEN_NECK 24
#define INVEN_BODY 25
#define INVEN_ARM 26
#define INVEN_HANDS 27
#define INVEN_RING 28
#define INVEN_RIGHT 28
#define INVEN_LEFT 29
#define INVEN_FEET 30
#define INVEN_OUTER 31
#define INVEN_LIGHT 32
#define INVEN_AUX 33
#define MAX_INVEN 34

#define ID_MAGIK 0x1  /* sense of magic */
#define ID_DAMD 0x2   /* cursed */
#define ID_EMPTY 0x4  /* reveal on use */
#define ID_REVEAL 0x8 /* Object ident is known */

/* inventory stacking subvals */
#define MAX_SUBVAL 64
#define MASK_SUBVAL (MAX_SUBVAL - 1)
/* these never stack */
#define ITEM_NEVER_STACK_MIN 0
#define ITEM_NEVER_STACK_MAX 63
/* these items always stack with others of same subval, always treated as
   single objects, must be power of 2 */
#define ITEM_SINGLE_STACK 64

// ITEM_GROUP removed

/* Stat indexes */
#define A_STR 0
#define A_INT 1
#define A_WIS 2
#define A_DEX 3
#define A_CON 4
#define A_CHR 5
#define MAX_A 6

// Input
#define ESCAPE '\033'
