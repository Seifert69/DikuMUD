/* ************************************************************************
*  file: structs.h , Structures        .                  Part of DIKUMUD *
*  Usage: Declarations of central data structures                         *
************************************************************************* */

#include <sys/types.h>
#include <crypt.h>

typedef signed char sbyte;
typedef unsigned char ubyte;
typedef signed short int sh_int;
typedef unsigned short int ush_int;
typedef char bool;
typedef char byte;

#define PULSE_ZONE     240
#define PULSE_MOBILE    40
#define PULSE_VIOLENCE  12
#define WAIT_SEC       4
#define WAIT_ROUND     4

#define MAX_STRING_LENGTH   4096
#define MAX_INPUT_LENGTH      80
#define MAX_MESSAGES          60
#define MAX_ITEMS            153

#define MESS_ATTACKER 1
#define MESS_VICTIM   2
#define MESS_ROOM     3

#define SECS_PER_REAL_MIN  60
#define SECS_PER_REAL_HOUR (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY  (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR (365*SECS_PER_REAL_DAY)

#define SECS_PER_MUD_HOUR  75
#define SECS_PER_MUD_DAY   (24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH (35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR  (17*SECS_PER_MUD_MONTH)

/* The following defs are for obj_data  */

/* For 'type_flag' */

#define ITEM_LIGHT      1
#define ITEM_SCROLL     2
#define ITEM_WAND       3
#define ITEM_STAFF      4
#define ITEM_WEAPON     5
#define ITEM_FIREWEAPON 6
#define ITEM_MISSILE    7
#define ITEM_TREASURE   8
#define ITEM_ARMOR      9
#define ITEM_POTION    10 
#define ITEM_WORN      11
#define ITEM_OTHER     12
#define ITEM_TRASH     13
#define ITEM_TRAP      14
#define ITEM_CONTAINER 15
#define ITEM_NOTE      16
#define ITEM_DRINKCON  17
#define ITEM_KEY       18
#define ITEM_FOOD      19
#define ITEM_MONEY     20
#define ITEM_PEN       21
#define ITEM_BOAT      22

/* Bitvector For 'wear_flags' */

#define ITEM_TAKE              1 
#define ITEM_WEAR_FINGER       2
#define ITEM_WEAR_NECK         4
#define ITEM_WEAR_BODY         8
#define ITEM_WEAR_HEAD        16
#define ITEM_WEAR_LEGS        32
#define ITEM_WEAR_FEET        64
#define ITEM_WEAR_HANDS      128 
#define ITEM_WEAR_ARMS       256
#define ITEM_WEAR_SHIELD     512
#define ITEM_WEAR_ABOUT     1024 
#define ITEM_WEAR_WAISTE    2048
#define ITEM_WEAR_WRIST     4096
#define ITEM_WIELD          8192
#define ITEM_HOLD          16384
#define ITEM_THROW         32768
/* UNUSED, CHECKS ONLY FOR ITEM_LIGHT #define ITEM_LIGHT_SOURCE  65536 */

/* Bitvector for 'extra_flags' */

#define ITEM_GLOW            1
#define ITEM_HUM             2
#define ITEM_DARK            4
#define ITEM_LOCK            8
#define ITEM_EVIL           16
#define ITEM_INVISIBLE      32
#define ITEM_MAGIC          64
#define ITEM_NODROP        128
#define ITEM_BLESS         256
#define ITEM_ANTI_GOOD     512 /* not usable by good people    */
#define ITEM_ANTI_EVIL    1024 /* not usable by evil people    */
#define ITEM_ANTI_NEUTRAL 2048 /* not usable by neutral people */


/* Some different kind of liquids */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_COKE       15

/* for containers  - value[1] */

#define CONT_CLOSEABLE      1
#define CONT_PICKPROOF      2
#define CONT_CLOSED         4
#define CONT_LOCKED         8

struct extra_descr_data
{
	char *keyword;                 /* Keyword in look/examine          */
	char *description;             /* What to see                      */
	struct extra_descr_data *next; /* Next in list                     */
};

#define MAX_OBJ_AFFECT 2         /* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
#define OBJ_NOTIMER    -7000000

struct obj_flag_data
{
	int value[4];       /* Values of the item (see list)    */
	byte type_flag;     /* Type of item                     */
	int wear_flags;     /* Where you can wear it            */
	int extra_flags;    /* If it hums,glows etc             */
	int weight;         /* Weigt what else                  */
	int cost;           /* Value when sold (gp.)            */
	int cost_per_day;   /* Cost to keep pr. real day        */
	int timer;          /* Timer for object                 */
	long bitvector;     /* To set chars bits                */
};

/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
struct obj_affected_type {
	byte location;      /* Which ability to change (APPLY_XXX) */
	sbyte modifier;     /* How much it changes by              */
};

/* ======================== Structure for object ========================= */
struct obj_data
{
	sh_int item_number;            /* Where in data-base               */
	sh_int in_room;                /* In what room -1 when conta/carr  */ 
	struct obj_flag_data obj_flags;/* Object information               */
	struct obj_affected_type
	    affected[MAX_OBJ_AFFECT];  /* Which abilities in PC to change  */

	char *name;                    /* Title of object :get etc.        */
	char *description ;            /* When in room                     */
	char *short_description;       /* when worn/carry/in cont.         */
 	char *action_description;      /* What to write when used          */
 	struct extra_descr_data *ex_description; /* extra descriptions     */
	struct char_data *carried_by;  /* Carried by :NULL in room/conta   */

	struct obj_data *in_obj;       /* In what object NULL when none    */
	struct obj_data *contains;     /* Contains objects                 */

	struct obj_data *next_content; /* For 'contains' lists             */
	struct obj_data *next;         /* For the object list              */
};
/* ======================================================================= */

/* The following defs are for room_data  */

#define NOWHERE    -1    /* nil reference for room-database    */

/* Bitvector For 'room_flags' */

#define DARK           1
#define DEATH          2
#define NO_MOB         4
#define INDOORS        8
#define LAWFULL       16
#define NEUTRAL       32
#define CHAOTIC       64
#define NO_MAGIC     128
#define TUNNEL       256
#define PRIVATE      512

/* For 'dir_option' */

#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5

#define EX_ISDOOR		1
#define EX_CLOSED		2
#define EX_LOCKED		4
#define EX_RSCLOSED	8
#define EX_RSLOCKED	16
#define EX_PICKPROOF 32

/* For 'Sector types' */

#define SECT_INSIDE          0
#define SECT_CITY            1
#define SECT_FIELD           2
#define SECT_FOREST          3
#define SECT_HILLS           4
#define SECT_MOUNTAIN        5
#define SECT_WATER_SWIM      6
#define SECT_WATER_NOSWIM    7

struct room_direction_data
{
	char *general_description;       /* When look DIR.                  */ 
	char *keyword;                   /* for open/close                  */	
	sh_int exit_info;                /* Exit info                       */
	sh_int key;		                   /* Key's number (-1 for no key)    */
	sh_int to_room;                  /* Where direction leeds (NOWHERE) */
};

/* ========================= Structure for room ========================== */
struct room_data
{
	sh_int number;               /* Rooms number                       */
	sh_int zone;                 /* Room zone (for resetting)          */
	int sector_type;             /* sector type (move/hide)            */
	char *name;                  /* Rooms name 'You are ...'           */
	char *description;           /* Shown when entered                 */
	struct extra_descr_data *ex_description; /* for examine/look       */
	struct room_direction_data *dir_option[6]; /* Directions           */
	sh_int room_flags;           /* DEATH,DARK ... etc                 */ 
	byte light;                  /* Number of lightsources in room     */
	int (*funct)();              /* special procedure                  */
         
	struct obj_data *contents;   /* List of items in room              */
	struct char_data *people;    /* List of NPC / PC in room           */
};
/* ======================================================================== */

/* The following defs and structures are related to char_data   */

/* For 'equipment' */

#define WEAR_LIGHT      0
#define WEAR_FINGER_R   1
#define WEAR_FINGER_L   2
#define WEAR_NECK_1     3
#define WEAR_NECK_2     4
#define WEAR_BODY       5
#define WEAR_HEAD       6
#define WEAR_LEGS       7
#define WEAR_FEET       8
#define WEAR_HANDS      9
#define WEAR_ARMS      10
#define WEAR_SHIELD    11
#define WEAR_ABOUT     12
#define WEAR_WAISTE    13
#define WEAR_WRIST_R   14
#define WEAR_WRIST_L   15
#define WIELD          16
#define HOLD           17


/* For 'char_payer_data' */

#define MAX_TOUNGE  3     /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_SKILLS  53    /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_WEAR    18
#define MAX_AFFECT  25    /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */

/* Predifined  conditions */
#define DRUNK        0
#define FULL         1
#define THIRST       2

/* Bitvector for 'affected_by' */
#define AFF_BLIND             1
#define AFF_INVISIBLE         2
#define AFF_DETECT_EVIL       4
#define AFF_DETECT_INVISIBLE  8
#define AFF_DETECT_MAGIC      16
#define AFF_SENSE_LIFE        32
#define AFF_HOLD              64
#define AFF_SANCTUARY         128
#define AFF_GROUP             256
#define AFF_CURSE             1024
#define AFF_FLAMING           2048
#define AFF_POISON            4096
#define AFF_PROTECT_EVIL      8192
#define AFF_PARALYSIS         16384
#define AFF_MORDEN_SWORD      32768
#define AFF_FLAMING_SWORD     65536
#define AFF_SLEEP             131072
#define AFF_DODGE             262144
#define AFF_SNEAK             524288
#define AFF_HIDE              1048576
#define AFF_FEAR              2097152
#define AFF_CHARM             4194304
#define AFF_FOLLOW            8388608

/* modifiers to char's abilities */

#define APPLY_NONE              0
#define APPLY_STR               1
#define APPLY_DEX               2
#define APPLY_INT               3
#define APPLY_WIS               4
#define APPLY_CON               5
#define APPLY_SEX               6
#define APPLY_CLASS             7
#define APPLY_LEVEL             8
#define APPLY_AGE               9
#define APPLY_CHAR_WEIGHT      10
#define APPLY_CHAR_HEIGHT      11
#define APPLY_MANA             12
#define APPLY_HIT              13
#define APPLY_MOVE             14
#define APPLY_GOLD             15
#define APPLY_EXP              16
#define APPLY_AC               17
#define APPLY_ARMOR            17
#define APPLY_HITROLL          18
#define APPLY_DAMROLL          19
#define APPLY_SAVING_PARA      20
#define APPLY_SAVING_ROD       21
#define APPLY_SAVING_PETRI     22
#define APPLY_SAVING_BREATH    23
#define APPLY_SAVING_SPELL     24


/* 'class' for PC's */
#define CLASS_MAGIC_USER  1
#define CLASS_CLERIC      2
#define CLASS_THIEF       3
#define CLASS_WARRIOR     4

/* 'class' for NPC's */
#define CLASS_OTHER       0 /* These are not yet used!   */
#define CLASS_UNDEAD      1 /* But may soon be so        */
#define CLASS_HUMANOID    2 /* Idea is to use general    */
#define CLASS_ANIMAL      3 /* monster classes           */
#define CLASS_DRAGON      4 /* Then for example a weapon */
#define CLASS_GIANT       5 /* of dragon slaying is pos. */

/* sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/* positions */
#define POSITION_DEAD       0
#define POSITION_MORTALLYW  1
#define POSITION_INCAP      2
#define POSITION_STUNNED    3
#define POSITION_SLEEPING   4
#define POSITION_RESTING    5
#define POSITION_SITTING    6
#define POSITION_FIGHTING   7
#define POSITION_STANDING   8


/* for mobile actions: specials.act */
#define ACT_SPEC         1     /* special routine to be called if exist   */
#define ACT_SENTINEL     2     /* this mobile not to be moved             */
#define ACT_SCAVENGER    4     /* pick up stuff lying around              */
#define ACT_ISNPC        8     /* This bit is set for use with IS_NPC()   */
#define ACT_NICE_THIEF  16     /* Set if a thief should NOT be killed     */
#define ACT_AGGRESSIVE  32     /* Set if automatic attack on NPC's        */
#define ACT_STAY_ZONE   64     /* MOB Must stay inside its own zone       */
#define ACT_WIMPY      128     /* MOB Will flee when injured, and if      */
                               /* aggressive only attack sleeping players */

/* For players : specials.act */
#define PLR_BRIEF        1
#define PLR_NOSHOUT      2
#define PLR_COMPACT      4
#define PLR_DONTSET      8 /* Dont EVER set */
#define PLR_NOTELL      16
#define PLR_NOEMOTE     32
#define PLR_LOG         64 /* log activities of this player. */
#define PLR_FREEZE     128 /* No commands available */

/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data
{
	byte hours, day, month;
	sh_int year;
};

/* These data contain information about a players time data */
struct time_data
{
  time_t birth;    /* This represents the characters age                */
  time_t logon;    /* Time of the last logon (used to calculate played) */
  int played;      /* This is the total accumulated time played in secs */
};

struct char_player_data
{
	char *name;    	    /* PC / NPC s name (kill ...  )         */
	char *short_descr;  /* for 'actions'                        */
	char *long_descr;   /* for 'look'.. Only here for testing   */
	char *description;  /* Extra descriptions                   */
	char *title;        /* PC / NPC s title                     */
	byte sex;           /* PC / NPC s sex                       */
	byte class;         /* PC s class or NPC alignment          */
	byte level;         /* PC / NPC s level                     */
	int hometown;       /* PC s Hometown (zone)                 */
	bool talks[MAX_TOUNGE]; /* PC s Tounges 0 for NPC           */
 	struct time_data time; /* PC s AGE in days                  */
	struct time_data tmptime; /* PC s AGE in days, modified     */
	ubyte weight;       /* PC / NPC s weight                    */
	ubyte height;       /* PC / NPC s height                    */
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_ability_data
{
	sbyte str; 
	sbyte str_add;      /* 000 - 100 if strength 18             */
	sbyte intel;
	sbyte wis; 
	sbyte dex; 
	sbyte con; 
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_point_data
{
	sh_int mana;         
	sh_int max_mana;     /* Not useable may be erased upon player file renewal */
	sh_int hit;   
	sh_int max_hit;      /* Max hit for NPC                         */
	sh_int move;  
	sh_int max_move;     /* Max move for NPC                        */

	sh_int armor;        /* Internal -100..100, external -10..10 AC */
	int gold;            /* Money carried                           */
	int exp;             /* The experience of the player            */

	sbyte hitroll;       /* Any bonus or penalty to the hit roll    */
	sbyte damroll;       /* Any bonus or penalty to the damage roll */
};


struct char_special_data
{
	struct char_data *fighting; /* Opponent                             */
	struct char_data *hunting;  /* Hunting person..                     */

	long affected_by;        /* Bitvector for spells/skills affected by */ 

	byte position;           /* Standing or ...                         */
	byte default_pos;        /* Default position for NPC                */
	ubyte act;               /* flags for NPC behavior                  */

	byte spells_to_learn;    /* How many can you learn yet this level   */

	int carry_weight;        /* Carried weight                          */
	byte carry_items;        /* Number of items carried                 */
	int timer;               /* Timer for update                        */
	sh_int was_in_room;      /* storage of location for linkdead people */
	sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)             */
	sbyte conditions[3];      /* Drunk full etc.                        */

	byte damnodice;           /* The number of damage dice's            */
	byte damsizedice;         /* The size of the damage dice's          */
	byte last_direction;      /* The last direction the monster went    */
	int attack_type;          /* The Attack Type Bitvector for NPC's    */
	int alignment;            /* +-1000 for alignments                  */
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_skill_data
{
	byte learned;           /* % chance for success 0 = not learned   */
	bool recognise;         /* If you can recognise the scroll etc.   */
};



/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct affected_type
{
	sbyte type;           /* The type of spell that caused this      */
	sh_int duration;      /* For how long its effects will last      */
	sbyte modifier;       /* This is added to apropriate ability     */
	byte location;        /* Tells which ability to change(APPLY_XXX)*/
	long bitvector;       /* Tells which bits to set (AFF_XXX)       */

	struct affected_type *next;
};

struct follow_type
{
	struct char_data *follower;
	struct follow_type *next;
};

/* ================== Structure for player/non-player ===================== */
struct char_data
{
	sh_int nr;                            /* monster nr (pos in file)      */
	sh_int in_room;                       /* Location                      */

	struct char_player_data player;       /* Normal data                   */
	struct char_ability_data abilities;   /* Abilities                     */
	struct char_ability_data tmpabilities;/* The abilities we will use     */
	struct char_point_data points;        /* Points                        */
	struct char_special_data specials;    /* Special plaing constants      */
	struct char_skill_data skills[MAX_SKILLS]; /* Skills                   */

	struct affected_type *affected;       /* affected by what spells       */
	struct obj_data *equipment[MAX_WEAR]; /* Equipment array               */

	struct obj_data *carrying;            /* Head of list                  */
	struct descriptor_data *desc;         /* NULL for mobiles              */

	struct char_data *next_in_room;     /* For room->people - list         */
	struct char_data *next;             /* For either monster or ppl-list  */
	struct char_data *next_fighting;    /* For fighting list               */

	struct follow_type *followers;        /* List of chars followers       */
	struct char_data *master;             /* Who is char following?        */
};


/* ======================================================================== */

/* How much light is in the land ? */

#define SUN_DARK	0
#define SUN_RISE	1
#define SUN_LIGHT	2
#define SUN_SET		3

/* And how is the sky ? */

#define SKY_CLOUDLESS	0
#define SKY_CLOUDY	1
#define SKY_RAINING	2
#define SKY_LIGHTNING	3

struct weather_data
{
	int pressure;	/* How is the pressure ( Mb ) */
	int change;	/* How fast and what way does it change. */
	int sky;	/* How is the sky. */
	int sunlight;	/* And how much sun. */
};


/* ***********************************************************************
*  file element for player file. BEWARE: Changing it will ruin the file  *
*********************************************************************** */


struct char_file_u
{
	byte sex;
	byte class;
	byte level;
	time_t birth;  /* Time of birth of character     */
  int played;    /* Number of secs played in total */

	ubyte weight;
	ubyte height;

	char title[80];
	sh_int hometown;
	char description[240];
	bool talks[MAX_TOUNGE];

	sh_int load_room;            /* Which room to place char in           */

	struct char_ability_data abilities;

	struct char_point_data points;

	struct char_skill_data skills[MAX_SKILLS];

	struct affected_type affected[MAX_AFFECT];

	/* specials */

	byte spells_to_learn;  
	int alignment;     

	time_t last_logon;  /* Time (in secs) of last logon */
	ubyte act;          /* ACT Flags                    */

	/* char data */
	char name[20];
	char pwd[CRYPT_OUTPUT_SIZE+1];
	sh_int apply_saving_throw[5];
	int conditions[3];
};



/* ***********************************************************************
*  file element for object file. BEWARE: Changing it will ruin the file  *
*********************************************************************** */

#define MAX_OBJ_SAVE 15 /* Used in OBJ_FILE_U *DO*NOT*CHANGE* */


struct obj_file_elem 
{
	sh_int item_number;

	int value[4];
	int extra_flags;
	int weight;
	int timer;
	long bitvector;
	struct obj_affected_type affected[MAX_OBJ_AFFECT];
};

struct obj_file_u
{
	char owner[20];    /* Name of player                     */
	int gold_left;     /* Number of goldcoins left at owner  */
	int total_cost;    /* The cost for all items, per day    */
	long last_update;  /* Time in seconds, when last updated */

	struct obj_file_elem objects[MAX_OBJ_SAVE];
};

/* ***********************************************************
*  The following structures are related to descriptor_data   *
*********************************************************** */



struct txt_block
{
	char *text;
	struct txt_block *next;
};

struct txt_q
{
	struct txt_block *head;
	struct txt_block *tail;
};



/* modes of connectedness */

#define CON_PLYNG   0
#define CON_NME	    1
#define CON_NMECNF  2
#define CON_PWDNRM  3
#define CON_PWDGET  4
#define CON_PWDCNF  5
#define CON_QSEX    6
#define CON_RMOTD   7
#define CON_SLCT    8
#define CON_EXDSCR  9
#define CON_QCLASS  10
#define CON_LDEAD   11
#define CON_PWDNEW  12
#define CON_PWDNCNF 13

struct snoop_data
{
	struct char_data *snooping;	
		/* Who is this char snooping */
	struct char_data *snoop_by;
		/* And who is snooping on this char */
};

struct descriptor_data
{
	int descriptor;	            /* file descriptor for socket */
	char host[50];                /* hostname                   */
	char pwd[CRYPT_OUTPUT_SIZE+1];/* password                   */
	int pos;                      /* position in player-file    */
	int connected;                /* mode of 'connectedness'    */
	int wait;                     /* wait for how many loops    */
	char *showstr_head;				/* for paging through texts	*/
	char *showstr_point;				/*       -                    */
	char **str;                   /* for the modify-str system  */
	int max_str;                  /* -                          */
	int prompt_mode;              /* control of prompt-printing */
	char buf[MAX_STRING_LENGTH];  /* buffer for raw input       */
	char last_input[MAX_INPUT_LENGTH];/* the last input         */
	struct txt_q output;          /* q of strings to send       */
	struct txt_q input;           /* q of unprocessed input     */
	struct char_data *character;  /* linked to char             */
   struct char_data *original;   /* original char              */
	struct snoop_data snoop;      /* to snoop people.	         */
	struct descriptor_data *next; /* link to next descriptor    */
};

struct msg_type 
{
	char *attacker_msg;  /* message to attacker */
	char *victim_msg;    /* message to victim   */
	char *room_msg;      /* message to room     */
};

struct message_type
{
	struct msg_type die_msg;      /* messages when death            */
	struct msg_type miss_msg;     /* messages when miss             */
	struct msg_type hit_msg;      /* messages when hit              */
	struct msg_type sanctuary_msg;/* messages when hit on sanctuary */
	struct msg_type god_msg;      /* messages when hit on god       */
	struct message_type *next;/* to next messages of this kind.*/
};

struct message_list
{
	int a_type;               /* Attack type 						 */
	int number_of_attacks;	  /* How many attack messages to chose from. */
	struct message_type *msg; /* List of messages.				 */
};

struct dex_skill_type
{
	sh_int p_pocket;
	sh_int p_locks;
	sh_int traps;
	sh_int sneak;
	sh_int hide;
};

struct dex_app_type
{
	sh_int reaction;
	sh_int miss_att;
	sh_int defensive;
};

struct str_app_type
{
	sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
	sh_int todam;    /* Damage Bonus/Penalty                */
	sh_int carry_w;  /* Maximum weight that can be carrried */
	sh_int wield_w;  /* Maximum weight that can be wielded  */
};

struct wis_app_type
{
	byte bonus;       /* how many bonus skills a player can */
	                  /* practice pr. level                 */
};

struct int_app_type
{
	byte learn;       /* how many % a player learns a spell/skill */
};

struct con_app_type
{
	sh_int hitp;
	sh_int shock;
};
