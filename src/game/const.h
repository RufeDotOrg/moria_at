
#define MAX_WIDTH 128
#define MAX_HEIGHT 64
#define SYMMAP_HEIGHT 16
#define SYMMAP_WIDTH 32
#define MAX_COL (MAX_WIDTH / SYMMAP_WIDTH * 2)
#define MAX_ROW (MAX_HEIGHT / SYMMAP_HEIGHT * 2)
#define STATUS_HEIGHT 22
#define STATUS_WIDTH 13
#define SCREEN_HEIGHT 23
#define MAX_MSG 16
#define MAX_ZOOM 4

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
#define MAGMA_WALL 13
#define QUARTZ_WALL 14
#define BOUNDARY_WALL 15

// Cave flags
#define CF_ROOM 0x1
#define CF_TEMP_LIGHT 0x2
#define CF_PERM_LIGHT 0x4
#define CF_FIELDMARK 0x8  // Object bypasses normal visibility rules
#define CF_LIT (CF_TEMP_LIGHT | CF_PERM_LIGHT)
#define CF_LIT_ROOM (CF_ROOM | CF_PERM_LIGHT)
#define CF_VIZ (CF_TEMP_LIGHT | CF_PERM_LIGHT | CF_FIELDMARK)

#define CHUNK_HEIGHT 16
#define CHUNK_WIDTH 32
#define CHUNK_COL (MAX_HEIGHT / CHUNK_HEIGHT)
#define CHUNK_ROW (MAX_WIDTH / CHUNK_WIDTH)
#define CHUNK_SQ (CHUNK_COL*CHUNK_ROW)

#define ROOM_HEIGHT (CHUNK_HEIGHT/2-1)
#define ROOM_WIDTH (CHUNK_WIDTH/2-1)

#define DUN_TUN_RND 9       /* 1/Chance of Random direction	       */
#define DUN_TUN_CHG 70      /* Chance of changing direction (99 max) */
#define DUN_TUN_CON 15      /* Chance of extra tunneling	       */
#define DUN_TUN_PEN 25      /* % chance of room doors		       */
#define DUN_TUN_JCT 15      /* % chance of doors at tunnel junctions */
#define DUN_STR_DEN  5      /* Density of streamers          */
#define DUN_STR_RNG  2      /* Width of streamers          */
#define DUN_STR_MAG  3      /* Number of magma streamers         */
#define DUN_STR_MC   90     /* 1/x chance of treasure per magma      */
#define DUN_STR_QUA  2      /* Number of quartz streamers        */
#define DUN_STR_QC   40     /* 1/x chance of treasure per quartz     */
#define DUN_ROOM_MEAN 32
#define MIN_MALLOC_LEVEL 14
#define RND_MALLOC_LEVEL 8
#define TREAS_ROOM_MEAN 7
#define TREAS_ANY_ALLOC 2
#define TREAS_GOLD_ALLOC 2
#define TRUE 1
#define FALSE 0

// Per turn
#define MAX_MALLOC_CHANCE 160 /* 1/x chance of new monster each round  */

#define MAX_STORE 6
#define MAX_STORE_INVEN 16
#define MAX_STORE_CHOICE 26
#define MIN_STORE_INVEN 8
#define STORE_TURN_AROUND 8

/* indexes into the special name table */
#define SN_R			1
#define SN_RA			2
#define SN_RF			3
#define SN_RC			4
#define SN_RL			5
#define SN_HA			6
#define SN_DF			7
#define SN_SA			8
#define SN_SD			9
#define SN_SE			10
#define SN_SU			11
#define SN_FT			12
#define SN_FB			13
#define SN_FREE_ACTION		14
#define SN_SLAYING		15
#define SN_CLUMSINESS		16
#define SN_WEAKNESS		17
#define SN_SLOW_DESCENT		18
#define SN_SPEED		19
#define SN_STEALTH		20
#define SN_SLOWNESS		21
#define SN_NOISE		22
#define SN_GREAT_MASS		23
#define SN_INTELLIGENCE		24
#define SN_WISDOM		25
#define SN_INFRAVISION		26
#define SN_MIGHT		27
#define SN_LORDLINESS		28
#define SN_MAGI			29
#define SN_BEAUTY		30
#define SN_SEEING		31
#define SN_REGENERATION		32
#define SN_STUPIDITY		33
#define SN_DULLNESS		34
#define SN_BLINDNESS		35
#define SN_TIMIDNESS		36
#define SN_TELEPORTATION	37
#define SN_UGLINESS		38
#define SN_PROTECTION		39
#define SN_IRRITATION		40
#define SN_VULNERABILITY	41
#define SN_ENVELOPING		42
#define SN_FIRE			43
#define SN_SLAY_EVIL		44
#define SN_DRAGON_SLAYING	45
#define SN_EMPTY		46
#define SN_LOCKED		47
#define SN_POISON_NEEDLE	48
#define SN_GAS_TRAP		49
#define SN_EXPLOSION_DEVICE	50
#define SN_SUMMONING_RUNES	51
#define SN_MULTIPLE_TRAPS	52
#define SN_DISARMED		53
#define SN_UNLOCKED		54
#define SN_SLAY_ANIMAL		55

/* Treasure constants */
#define TV_NOTHING 0
#define TV_MISC 1
#define TV_CHEST 2
/* min tval for wearable items, all items between TV_MIN_WEAR and TV_MAX_WEAR
   use the same flag bits, see the TR_* defines */
#define TV_MIN_WEAR 10
#define TV_LIGHT 15
/* items tested for enchantments, i.e. the MAGIK inscription, see the
   sense_magik() procedure */
#define TV_MIN_ENCHANT 21
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
#define TV_MON_PICK_UP 99
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
#define TV_STORE_DOOR 110
#define TV_GLYPH 111
#define TV_PAWN_DOOR 112
#define TV_MAX_VISIBLE 112

#define MAX_OBJ_LEVEL 50
#define MAX_DUNGEON_OBJ 345
#define OBJ_GREAT 12  // 1/x treasure is a big surprise!

/* Magic Treasure Generation constants				*/
/* Note: Number of special objects, and degree of enchantments	 */
/*	 can be adjusted here.					 */
#define OBJ_STD_ADJ	 125  /* Adjust STD per level * 100	       */
#define OBJ_STD_MIN	 7    /* Minimum STD			       */
#define OBJ_TOWN_LEVEL	 7    /* Town object generation level	       */
#define OBJ_BASE_MAGIC	 15   /* Base amount of magic		       */
#define OBJ_BASE_MAX	 70   /* Max amount of magic		       */
#define OBJ_DIV_SPECIAL	 6    /* magic_chance/#	 special magic	      */
#define OBJ_DIV_CURSED	 13   /* 10*magic_chance/#  cursed items	 */

#define OBJ_RUNE_TIDX 351 /* Rune of protection treasure index */
#define OBJ_MUSH_TIDX 352 /* Create food treasure index */
#define OBJ_TRAP_BEGIN 353
#define OBJ_TRAP_END 371
#define MAX_TRAP (OBJ_TRAP_END-OBJ_TRAP_BEGIN)

/* spell types used by get_flags(), breathe(), fire_bolt() and fire_ball() */
#define GF_MAGIC_MISSILE 0
#define GF_LIGHTNING	1
#define GF_POISON_GAS	2
#define GF_ACID		3
#define GF_FROST	4
#define GF_FIRE		5
#define GF_HOLY_ORB	6

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
// #define TR_BLIND 0x08000000L
// #define TR_TIMID 0x10000000L
#define TR_TUNNEL 0x20000000L
#define TR_SLOWNESS 0x40000000L
#define TR_CURSED 0x80000000L
// Set of flags dependent on p1 value
#define TR_P1 (TR_STATS | TR_SEARCH | TR_STEALTH | TR_TUNNEL)

/* definitions for chests */
#define CH_LOCKED	0x00000001L
#define CH_TRAPPED	0x000001F0L
#define CH_LOSE_STR	0x00000010L
#define CH_POISON	0x00000020L
#define CH_PARALYSED	0x00000040L
#define CH_EXPLODE	0x00000080L
#define CH_SUMMON	0x00000100L

/* magic effects */
#define MA_BLESS 0
#define MA_HERO 1
#define MA_SUPERHERO 2
#define MA_FAST 3
#define MA_SLOW 4
#define MA_AFIRE 5
#define MA_AFROST 6
#define MA_INVULN 7
#define MA_SEE_INVIS 8 // TBD: move to immediate (breaks save format)
#define MA_SEE_INFRA 9
#define MA_BLIND 10
#define MA_DETECT_MON 15
#define MA_DETECT_EVIL 16
#define MA_DETECT_INVIS 17
#define MA_RECALL 18
#define MA_COUNT 19
// Magic affects less than this value are persistent in the save file
#define MA_SAVE 14

#define OBJ_BOLT_RANGE	 18   /* Maximum range of bolts and balls      */

/* Creature constants						*/
#define MAX_MON_LEVEL 40
#define MON_NASTY 50     // 1/x monsters are a big baddie
#define MON_DRAIN_EXP 2 /* Percent of player exp drained per hit */
#define MON_SUMMON_ADJ 2 /* Adjust level of summoned creatures    */
#define MON_MULT_ADJ 7 /* High value slows multiplication       */
#define MAX_WIN_MON 2 /* Total number of win creatures */
#define WIN_MON_APPEAR 50 /* Level where winning creature begins */
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

/* creature spell definitions */
#define CS_FREQ		0x0000000FL
#define CS_SPELLS	0x0001FFF0L
#define CS_TEL_SHORT	0x00000010L
#define CS_TEL_LONG	0x00000020L
#define CS_TEL_TO	0x00000040L
#define CS_LGHT_WND	0x00000080L
#define CS_SER_WND	0x00000100L
#define CS_HOLD_PER	0x00000200L
#define CS_BLIND	0x00000400L
#define CS_CONFUSE	0x00000800L
#define CS_FEAR		0x00001000L
#define CS_SUMMON_MON	0x00002000L
#define CS_SUMMON_UND	0x00004000L
#define CS_SLOW_PER	0x00008000L
#define CS_DRAIN_MANA	0x00010000L

#define CS_BREATHE	0x00F80000L /* may also just indicate resistance */
#define CS_BR_LIGHT	0x00080000L /* if no spell frequency set */
#define CS_BR_GAS	0x00100000L
#define CS_BR_ACID	0x00200000L
#define CS_BR_FROST	0x00400000L
#define CS_BR_FIRE	0x00800000L

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
#define MAX_EXP 9999999 /* Maximum player experience */
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
#define INVEN_EQUIP_END 33
#define INVEN_AUX 33
#define MAX_INVEN 34

#define ID_MAGIK 0x1  /* obj_sense() magic */
#define ID_DAMD 0x2   /* wore something cursed and known bad */
#define ID_EMPTY 0x4  /* wand/staff known to have 0 charges */
#define ID_REVEAL 0x08 /* full object identity is known */
#define ID_CORRODED 0x10 /* armor is known to provide no protection from acid/gas damage */
#define ID_PLAIN 0x20 /* obj_sense() not magic or cursed */
#define ID_RARE 0x40 /* obj_sense() special name item */

/* tval sub type */
#define MAX_SUBVAL 64
#define MASK_SUBVAL (MAX_SUBVAL - 1)

// Potion/Scroll/Food: single slot; +/-1 per interaction
#define STACK_SINGLE 0x40
// Arrows/Bolts/Pebbles/Spike: single slot; +/- obj->number on interaction
#define STACK_BATCH 0x80
#define STACK_ANY 0xc0

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
// Output
enum { BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE, BRIGHT };
