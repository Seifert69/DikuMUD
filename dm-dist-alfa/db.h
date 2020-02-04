/* ************************************************************************
*  file: db.h , Database module.                          Part of DIKUMUD *
*  Usage: Loading/Saving chars booting world.                             *
************************************************************************* */


/* data files used by the game system */

#define DFLT_DIR          "lib"           /* default data directory     */

#define WORLD_FILE        "tinyworld.wld" /* room definitions           */
#define MOB_FILE          "tinyworld.mob" /* monster prototypes         */
#define OBJ_FILE          "tinyworld.obj" /* object prototypes          */
#define ZONE_FILE         "tinyworld.zon" /* zone defs & command tables */
#define CREDITS_FILE      "credits"       /* for the 'credits' command  */
#define NEWS_FILE         "news"          /* for the 'news' command     */
#define MOTD_FILE         "motd"          /* messages of today          */
#define PLAYER_FILE       "players"       /* the player database        */
#define TIME_FILE         "time"          /* game calendar information  */
#define IDEA_FILE         "ideas"         /* for the 'idea'-command     */
#define TYPO_FILE         "typos"         /*         'typo'             */
#define BUG_FILE          "bugs"          /*         'bug'              */
#define MESS_FILE         "messages"      /* damage message             */
#define SOCMESS_FILE      "actions"       /* messgs for social acts     */
#define HELP_KWRD_FILE    "help_table"    /* for HELP <keywrd>          */
#define HELP_PAGE_FILE    "help"          /* for HELP <CR>              */
#define INFO_FILE         "info"          /* for INFO                   */
#define WIZLIST_FILE      "wizlist"       /* for WIZLIST                */
#define POSEMESS_FILE     "poses"         /* for 'pose'-command         */

/* public procedures in db.c */

void boot_db(void);
void save_char(struct char_data *ch, sh_int load_room);
int create_entry(char *name);
void zone_update(void);
void init_char(struct char_data *ch);
void clear_char(struct char_data *ch);
void clear_object(struct obj_data *obj);
void reset_char(struct char_data *ch);
void free_char(struct char_data *ch);
int real_room(int virtual);
char *fread_string(FILE *fl);
int real_object(int virtual);
int real_mobile(int virtual);

#define REAL 0
#define VIRTUAL 1

struct obj_data *read_object(int nr, int type);
struct char_data *read_mobile(int nr, int type);

#define MENU         \
"\n\rWelcome to DikuMUD\n\r\n\
0) Exit from DikuMud.\n\r\
1) Enter the game.\n\r\
2) Enter description.\n\r\
3) Read the background story\n\r\
4) Change password.\n\r\n\r\
   Make your choice: "


#define GREETINGS \
"\n\r\n\r  \
                           DikuMUD I (GAMMA 0.0)\n\r\n\r \
                              Created by\n\r \
                  Hans Henrik Staerfeldt, Katja Nyboe,\n\r \
           Tom Madsen, Michael Seifert, and Sebastian Hammer\n\r\n\r"

#define WELC_MESSG \
"\n\rWelcome to the land of DikuMUD. May your visit here be... Interesting.\
\n\r\n\r"


#define STORY     \
"This will soon be the background story of DIKU-MUD.\n\r\n\r"


/* structure for the reset commands */
struct reset_com
{
	char command;   /* current command                      */ 
	bool if_flag;   /* if TRUE: exe only if preceding exe'd */
	int arg1;       /*                                      */
	int arg2;       /* Arguments to the command             */
	int arg3;       /*                                      */

	/* 
	*  Commands:              *
	*  'M': Read a mobile     *
	*  'O': Read an object    *
	*  'G': Give obj to mob   *
	*  'P': Put obj in obj    *
	*  'G': Obj to char       *
	*  'E': Obj to char equip *
	*  'D': Set state of door *
	*/
};



/* zone definition structure. for the 'zone-table'   */
struct zone_data
{
	char *name;             /* name of this zone                  */
	int lifespan;           /* how long between resets (minutes)  */
	int age;                /* current age of this zone (minutes) */
	int top;                /* upper limit for rooms in this zone */

	int reset_mode;         /* conditions for reset (see below)   */
	struct reset_com *cmd;  /* command table for reset	           */

	/*
	*  Reset mode:                              *
	*  0: Don't reset, and don't update age.    *
	*  1: Reset if no PC's are located in zone. *
	*  2: Just reset.                           *
	*/
};




/* element in monster and object index-tables   */
struct index_data
{
	int virtual;    /* virtual number of this mob/obj           */
	long pos;       /* file position of this field              */
	int number;     /* number of existing units of this mob/obj	*/
	int (*func)();  /* special procedure for this mob/obj       */
};




/* for queueing zones for update   */
struct reset_q_element
{
	int zone_to_reset;            /* ref to zone_data */
	struct reset_q_element *next;	
};



/* structure for the update queue     */
struct reset_q_type
{
	struct reset_q_element *head;
	struct reset_q_element *tail;
} reset_q;



struct player_index_element
{
	char *name;
	int nr;
};


struct help_index_element
{
	char *keyword;
	long pos;
};
