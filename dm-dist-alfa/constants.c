/* ************************************************************************
*  file: constants.c                                      Part of DIKUMUD *
*  Usage: For constants used by the game.                                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include "structs.h"
#include "limits.h"


const char *spell_wear_off_msg[] = {
  "RESERVED DB.C",
  "You feel less proctected.",
  "!Teleport!",
  "You feel less righteous.",
  "You feel a cloak of blindness disolve.",
  "!Burning Hands!",
  "!Call Lightning",
  "You feel more self-confident.",
  "!Chill Touch!",
  "!Clone!",
  "!Color Spray!",
  "!Control Weather!",
  "!Create Food!",
  "!Create Water!",
  "!Cure Blind!",
  "!Cure Critic!",
  "!Cure Light!",
  "You feel better.",
  "You sense the red in your vision disappear.",
  "The detect invisible wears off.",
  "The detect magic wears off.",
  "The detect poison wears off.",
  "!Dispel Evil!",
  "!Earthquake!",
  "!Enchant Weapon!",
  "!Energy Drain!",
  "!Fireball!",
  "!Harm!",
  "!Heal",
  "You feel yourself exposed.",
  "!Lightning Bolt!",
  "!Locate object!",
  "!Magic Missile!",
  "You feel less sick.",
  "You feel less protected.",
  "!Remove Curse!",
  "The white aura around your body fades.",
  "!Shocking Grasp!",
  "You feel les tired.",
  "You feel weaker.",
  "!Summon!",
  "!Ventriloquate!",
  "!Word of Recall!",
  "!Remove Poison!",
  "You feel less aware of your suroundings.",
  "",  /* NO MESSAGE FOR SNEAK*/
  "!Hide!",
  "!Steal!",
  "!Backstab!",
  "!Pick Lock!",
  "!Kick!",
  "!Bash!",
  "!Rescue!",
  "!UNUSED!"
};


const int rev_dir[] = 
{
	2,
	3,
	0,
	1,	
	5,
	4
}; 

const int movement_loss[]=
{
	1,	/* Inside     */
	2,  /* City       */
	2,  /* Field      */
	3,  /* Forest     */
	4,  /* Hills      */
	6,  /* Mountains  */
  4,  /* Swimming   */
  1   /* Unswimable */
};

const char *dirs[] = 
{
	"north",
	"east",
	"south",
	"west",
	"up",
	"down",
	"\n"
};

const char *weekdays[7] = { 
	"the Day of the Moon",
	"the Day of the Bull",
	"the Day of the Deception",
	"the Day of Thunder",
	"the Day of Freedom",
	"the day of the Great Gods",
	"the Day of the Sun" };

const char *month_name[17] = {
	"Month of Winter",           /* 0 */
	"Month of the Winter Wolf",
	"Month of the Frost Giant",
	"Month of the Old Forces",
	"Month of the Grand Struggle",
	"Month of the Spring",
	"Month of Nature",
	"Month of Futility",
	"Month of the Dragon",
	"Month of the Sun",
	"Month of the Heat",
	"Month of the Battle",
	"Month of the Dark Shades",
	"Month of the Shadows",
	"Month of the Long Shadows",
	"Month of the Ancient Darkness",
	"Month of the Great Evil"
};

const int sharp[] = {
   0,
   0,
   0,
   1,    /* Slashing */
   0,
   0,
   0,
   0,    /* Bludgeon */
   0,
   0,
   0,
   0 };  /* Pierce   */

const char *where[] = {
	"<used as light>      ",
	"<worn on finger>     ",
	"<worn on finger>     ",
	"<worn around neck>   ",
	"<worn around neck>   ",
	"<worn on body>       ",
	"<worn on head>       ",
	"<worn on legs>       ",
	"<worn on feet>       ",
	"<worn on hands>      ",
	"<worn on arms>       ",
	"<worn as shield>     ",
	"<worn about body>    ",
	"<worn about waist>   ",
	"<worn around wrist>  ",
	"<worn around wrist>  ",
	"<wielded>            ",
	"<held>               " 
}; 

const char *drinks[]=
{
	"water",
	"beer",
	"wine",
	"ale",
	"dark ale",
	"whisky",
	"lemonade",
	"firebreather",
	"local speciality",
	"slime mold juice",
	"milk",
	"tea",
	"coffee",
	"blood",
	"salt water",
	"coca cola"
};

const char *drinknames[]=
{
	"water",
	"beer",
	"wine",
	"ale",
	"ale",
	"whisky",
	"lemonade",
	"firebreather",
	"local",
	"juice",
	"milk",
	"tea",
	"coffee",
	"blood",
	"salt",
	"cola"
};

const int drink_aff[][3] = {
	{ 0,1,10 },  /* Water    */
	{ 3,2,5 },   /* beer     */
	{ 5,2,5 },   /* wine     */
	{ 2,2,5 },   /* ale      */
	{ 1,2,5 },   /* ale      */
	{ 6,1,4 },   /* Whiskey  */
	{ 0,1,8 },   /* lemonade */
	{ 10,0,0 },  /* firebr   */
	{ 3,3,3 },   /* local    */
	{ 0,4,-8 },  /* juice    */
	{ 0,3,6 },
	{ 0,1,6 },
	{ 0,1,6 },
	{ 0,2,-1 },
	{ 0,1,-2 },
	{ 0,1,5 }
};

const char *color_liquid[]=
{
	"clear",
	"brown",
	"clear",
	"brown",
	"dark",
	"golden",
	"red",
	"green",
	"clear",
	"light green",
	"white",
	"brown",
	"black",
	"red",
	"clear",
	"black"
};

const char *fullness[] =
{
	"less than half ",
	"about half ",
	"more than half ",
	""
};

const struct title_type titles[4][25] = {
{ {"the Man","the Woman",0},
  {"the Apprentice of Magic","the Apprentice of Magic",1},
  {"the Spell Student","the Spell Student",2500},
  {"the Scholar of Magic","the Scholar of Magic",5000},
  {"the Delver in Spells","the Delveress in Spells",10000},
  {"the Medium of Magic","the Medium of Magic",20000},
  {"the Scribe of Magic","the Scribess of Magic",40000},
  {"the Seer","the Seeress",60000},
  {"the Sage","the Sage",90000},
  {"the Illusionist","the Illusionist",135000},
  {"the Abjurer","the Abjuress",250000},
  {"the Invoker","the Invoker",375000},
  {"the Enchanter","the Enchantress",750000},
  {"the Conjurer","the Conjuress",1125000},
  {"the Magician","the Witch",1500000},
  {"the Creator","the Creator",1875000},
  {"the Savant","the Savant",2250000},
  {"the Magus","the Craftess",2625000},
  {"the Wizard","the Wizard",3000000},
  {"the Warlock","the War Witch",3375000},
  {"the Sorcerer","the Sorceress",3750000},
  {"the Immortal Warlock","the Immortal Enchantress",4000000},
  {"the Avatar of Magic","the Empress of Magic",5000000},
  {"the God of magic","the Goddess of magic",6000000},
  {"the Implementator","the Implementress",7000000} },

{ {"the Man","the Woman",0},
  {"the Believer","the Believer",1},
  {"the Attendant","the Attendant",1500},
  {"the Acolyte","the Acolyte",3000},
  {"the Novice","the Novice",6000},
  {"the Missionary","the Missionary",13000},
  {"the Adept","the Adept",27500},
  {"the Deacon","the Deaconess",55000},
  {"the Vicar","the Vicaress",110000},
  {"the Priest","the Priestess",225000},
  {"the Minister","the Lady Minister",450000},
  {"the Canon","the Canon",675000},
  {"the Levite","the Levitess",900000},
  {"the Curate","the Curess",1125000},
  {"the Monk","the Nunne",1350000},
  {"the Healer","the Healess",1575000},
  {"the Chaplain","the Chaplain",1800000},
  {"the Expositor","the Expositress",2025000},
  {"the Bishop","the Bishop",2250000},
  {"the Arch Bishop","the Arch Lady of the Church",2475000},
  {"the Patriarch","the Matriarch",2700000},
  {"the Immortal Cardinal","the Immortal Priestess",3000000},
  {"the Inquisitor","the Inquisitress",5000000},
  {"the God of good and evil","the Goddess of good and evil",6000000},
  {"the Implementator","the Implementress",7000000} },

{ {"the Man","the Woman",0},
  {"the Pilferer","the Pilferess",1},
  {"the Footpad","the Footpad",1250},
  {"the Filcher","the Filcheress",2500},
  {"the Pick-Pocket","the Pick-Pocket",5000},
  {"the Sneak","the Sneak",10000},
  {"the Pincher","the Pincheress",20000},
  {"the Cut-Purse","the Cut-Purse",30000},
  {"the Snatcher","the Snatcheress",70000},
  {"the Sharper","the Sharpress",110000},
  {"the Rogue","the Rogue",160000},
  {"the Robber","the Robber",220000},
  {"the Magsman","the Magswoman",440000},
  {"the Highwayman","the Highwaywoman",660000},
  {"the Burglar","the Burglaress",880000},
  {"the Thief","the Thief",1100000},
  {"the Knifer","the Knifer",1320000},
  {"the Quick-Blade","the Quick-Blade",1540000},
  {"the Killer","the Murderess",1760000},
  {"the Brigand","the Brigand",1980000},
  {"the Cut-Throat","the Cut-Throat",2200000},
  {"the Immortal Assasin","the Immortal Assasin",2500000},
  {"the Demi God of thieves","the Demi Goddess of thieves",5000000},
  {"the God of thieves and tradesmen","the Goddess of thieves and tradesmen",6000000},
  {"the Implementator","the Implementress",7000000} },

{ {"the Man","the Woman",0},
  {"the Swordpupil","the Swordpupil",1},
  {"the Recruit","the Recruit",2000},
  {"the Sentry","the Sentress",4000},
  {"the Fighter","the Fighter",8000},
  {"the Soldier","the Soldier",16000},
  {"the Warrior","the Warrior",32000},
  {"the Veteran","the Veteran",64000},
  {"the Swordsman","the Swordswoman",125000},
  {"the Fencer","the Fenceress",250000},
  {"the Combatant","the Combatess",500000},
  {"the Hero","the Heroine",750000},
  {"the Myrmidon","the Myrmidon",1000000},
  {"the Swashbuckler","the Swashbuckleress",1250000},
  {"the Mercenary","the Mercenaress",1500000},
  {"the Swordmaster","the Swordmistress",1750000},
  {"the Lieutenant","the Lieutenant",2000000},
  {"the Champion","the Lady Champion",2250000},
  {"the Dragoon","the Lady Dragoon",2500000},
  {"the Cavalier","the Cavalier",2750000},
  {"the Knight","the Lady Knight",3000000},
  {"the Immortal Warlord","the Immortal Lady of War",3250000},
  {"the Extirpator","the Queen of Destruction",5000000},
  {"the God of war","the Goddess of war",6000000},
  {"the Implementator","the Implementress",7000000} }
};

const char *item_types[] = {
	"UNDEFINED",
	"LIGHT",
	"SCROLL",
	"WAND",
	"STAFF",
	"WEAPON",
	"FIRE WEAPON",
	"MISSILE",
	"TREASURE",
	"ARMOR",
	"POTION",
	"WORN",
	"OTHER",
	"TRASH",
	"TRAP",
	"CONTAINER",
	"NOTE",
	"LIQUID CONTAINER",
	"KEY",
	"FOOD",
	"MONEY",
	"PEN",
	"BOAT",
	"\n"
};

const char *wear_bits[] = {
	"TAKE",
	"FINGER",
	"NECK",
	"BODY",
	"HEAD",
	"LEGS",
	"FEET",
	"HANDS",
	"ARMS",
	"SHIELD",
	"ABOUT",
	"WAISTE",
	"WRIST",
	"WIELD",
	"HOLD",
	"THROW",
	"LIGHT-SOURCE",
	"\n"
};

const char *extra_bits[] = {
	"GLOW",
	"HUM",
	"DARK",
	"LOCK",
	"EVIL",
	"INVISIBLE",
	"MAGIC",
	"NODROP",
	"BLESS",
	"ANTI-GOOD",
	"ANTI-EVIL",
	"ANTI-NEUTRAL",
	"\n"
};

const char *room_bits[] = {
	"DARK",
	"DEATH",
	"NO_MOB",
	"INDOORS",
	"LAWFULL",
	"NEUTRAL",
	"CHAOTOC",
	"NO_MAGIC",
	"TUNNEL",
	"PRIVATE",
	"\n"
};

const char *exit_bits[] = {
	"IS-DOOR",
	"CLOSED",
	"LOCKED",
	"\n"
};

const char *sector_types[] = {
	"Inside",
	"City",
	"Field",
	"Forest",
	"Hills",
	"Mountains",
	"Water Swim",
	"Water NoSwim",
	"\n"
};

const char *equipment_types[] = {
	"Special",
	"Worn on right finger",
	"Worn on left finger",
	"First worn around Neck",
	"Second worn around Neck",
	"Worn on body",
	"Worn on head",
	"Worn on legs",
	"Worn on feet",
	"Worn on hands",
	"Worn on arms",
	"Worn as shield",
	"Worn about body",
	"Worn around waiste",
	"Worn around right wrist",
	"Worn around left wrist",
	"Wielded",
	"Held",
	"\n"
};
	
const char *affected_bits[] = 
{	"BLIND",
	"INVISIBLE",
	"DETECT-EVIL",
	"DETECT-INVISIBLE",
	"DETECT-MAGIC",
	"SENCE-LIFE",
	"HOLD",
	"SANCTUARY",
	"GROUP",
	"UNUSED",
	"CURSE",
	"FLAMING-HANDS",
	"POISON",
	"PROTECT-EVIL",
	"PARALYSIS",
	"MORDENS-SWORD",
	"FLAMING-SWORD",
	"SLEEP",
	"DODGE",
	"SNEAK",
	"HIDE",
	"FEAR",
	"CHARM",
	"FOLLOW",
	"SAVED_OBJECTS",
	"\n"
};

const char *apply_types[] = {
	"NONE",
	"STR",
	"DEX",
	"INT",
	"WIS",
	"CON",
	"SEX",
	"CLASS",
	"LEVEL",
	"AGE",
	"CHAR_WEIGHT",
	"CHAR_HEIGHT",
	"MANA",
	"HIT",
	"MOVE",
	"GOLD",
	"EXP",
	"ARMOR",
	"HITROLL",
	"DAMROLL",
	"SAVING_PARA",
	"SAVING_ROD",
	"SAVING_PETRI",
	"SAVING_BREATH",
	"SAVING_SPELL",
	"\n"
};

const char *pc_class_types[] = {
	"UNDEFINED",
	"Magic User",
	"Cleric",
	"Thief",
	"Warrior",
	"\n"
};

const char *npc_class_types[] = {
	"Normal",
	"Undead",
	"\n"
};

const char *action_bits[] = {
	"SPEC",
	"SENTINEL",
	"SCAVENGER",
	"ISNPC",
	"NICE-THIEF",
	"AGGRESSIVE",
	"STAY-ZONE",
	"WIMPY",
	"\n"
};


const char *player_bits[] = {
	"BRIEF",
	"NOSHOUT",
	"COMPACT",
	"DONTSET",
	"NOTELL",
	"NOEMOTE",
	"",
	"FREEZE",
	"\n"
};


const char *position_types[] = {
	"Dead",
	"Mortally wounded",
	"Incapasitated",
	"Stunned",
	"Sleeping",
	"Resting",
	"Sitting",
	"Fighting",
	"Standing",
	"\n"
};

const char *connected_types[]	=	{
	"Playing",
	"Get name",
	"Confirm name",
	"Read Password",
	"Get new password",
	"Confirm new password",
	"Get sex",
	"Read messages of today",
	"Read Menu",
	"Get extra description",
	"Get class",
	"\n"
};

/* [class], [level] (all) */
const int thaco[4][25] = {
	 { 100,20,20,20,19,19,19,18,18,18,17,17,17,16,16,16,15,15,15,14,14,14,13,13,13},
   { 100,20,20,20,18,18,18,16,16,16,14,14,14,12,12,12,10,10,10, 8, 8, 8, 6, 6, 6},
   { 100,20,20,19,19,18,18,17,17,16,16,15,15,14,13,13,12,12,11,11,10,10, 9, 9, 8},
   { 100,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 1, 1, 1, 1}
};

/* [ch] strength apply (all) */
const struct str_app_type str_app[31] = {
	{ -5,-4,   0,  0 },  /* 0  */
	{ -5,-4,   3,  1 },  /* 1  */
	{ -3,-2,   3,  2 },
	{ -3,-1,  10,  3 },  /* 3  */
	{ -2,-1,  25,  4 },
	{ -2,-1,  55,  5 },  /* 5  */
	{ -1, 0,  80,  6 },
	{ -1, 0,  90,  7 },
	{  0, 0, 100,  8 },
	{  0, 0, 100,  9 },
	{  0, 0, 115, 10 }, /* 10  */
	{  0, 0, 115, 11 },
	{  0, 0, 140, 12 },
	{  0, 0, 140, 13 },
	{  0, 0, 170, 14 },
	{  0, 0, 170, 15 }, /* 15  */
	{  0, 1, 195, 16 },
	{  1, 1, 220, 18 },
	{  1, 2, 255, 20 }, /* 18  */
	{  3, 7, 640, 40 },
	{  3, 8, 700, 40 }, /* 20  */
	{  4, 9, 810, 40 },
	{  4,10, 970, 40 },
	{  5,11,1130, 40 },
	{  6,12,1440, 40 },
	{  7,14,1750, 40 }, /* 25            */
	{  1, 3, 280, 22 }, /* 18/01-50      */
	{  2, 3, 305, 24 }, /* 18/51-75      */
	{  2, 4, 330, 26 }, /* 18/76-90      */
	{  2, 5, 380, 28 }, /* 18/91-99      */
	{  3, 6, 480, 30 }  /* 18/100   (30) */
};

/* [dex] skill apply (thieves only) */
const struct dex_skill_type dex_app_skill[26] = {
	{-99,-99,-90,-99,-60},   /* 0 */
	{-90,-90,-60,-90,-50},   /* 1 */
	{-80,-80,-40,-80,-45},
	{-70,-70,-30,-70,-40},
	{-60,-60,-30,-60,-35},
	{-50,-50,-20,-50,-30},   /* 5 */
	{-40,-40,-20,-40,-25},
	{-30,-30,-15,-30,-20},
	{-20,-20,-15,-20,-15},
	{-15,-10,-10,-20,-10},
	{-10, -5,-10,-15, -5},   /* 10 */
	{ -5,  0, -5,-10,  0},
	{  0,  0,  0, -5,  0},
	{  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0},   /* 15 */
	{  0,  5,  0,  0,  0},
	{  5, 10,  0,  5,  5},
	{ 10, 15,  5, 10, 10},
	{ 15, 20, 10, 15, 15},
	{ 15, 20, 10, 15, 15},   /* 20 */
	{ 20, 25, 10, 15, 20},
	{ 20, 25, 15, 20, 20},
	{ 25, 25, 15, 20, 20},
	{ 25, 30, 15, 25, 25},
	{ 25, 30, 15, 25, 25}    /* 25 */
};

/* [level] backstab multiplyer (thieves only) */
const byte backstab_mult[25] = {
	1,   /* 0 */
	2,   /* 1 */
	2,
	2,
	2,
	3,   /* 5 */
	3,
	3,
	3,
	4,
	4,   /* 10 */
	4,
	4,
	4,
	5,
	5,   /* 15 */
	5,
	5,
	5,
	5,
	5,   /* 20 */
	5,
	5,
	5,
	5    /* 25 */
};

/* [dex] apply (all) */
struct dex_app_type dex_app[26] = {
	{-7,-7, 6},   /* 0 */
	{-6,-6, 5},   /* 1 */
	{-4,-4, 5},
	{-3,-3, 4},
	{-2,-2, 3},
	{-1,-1, 2},   /* 5 */
	{ 0, 0, 1},
	{ 0, 0, 0},
	{ 0, 0, 0},
	{ 0, 0, 0},
	{ 0, 0, 0},   /* 10 */
	{ 0, 0, 0},
	{ 0, 0, 0},
	{ 0, 0, 0},
	{ 0, 0, 0},
	{ 0, 0,-1},   /* 15 */
	{ 1, 1,-2},
	{ 2, 2,-3},
	{ 2, 2,-4},
	{ 3, 3,-4},
	{ 3, 3,-4},   /* 20 */
	{ 4, 4,-5},
	{ 4, 4,-5},
	{ 4, 4,-5},
	{ 5, 5,-6},
	{ 5, 5,-6}    /* 25 */
};

/* [con] apply (all) */
struct con_app_type con_app[26] = {
	{-4,20},   /* 0 */
	{-3,25},   /* 1 */
	{-2,30},
	{-2,35},
	{-1,40},
	{-1,45},   /* 5 */
	{-1,50},
	{ 0,55},
	{ 0,60},
	{ 0,65},
	{ 0,70},   /* 10 */
	{ 0,75},
	{ 0,80},
	{ 0,85},
	{ 0,88},
	{ 1,90},   /* 15 */
	{ 2,95},
	{ 2,97},
	{ 3,99},
	{ 3,99},
	{ 4,99},   /* 20 */
	{ 5,99},
	{ 5,99},
	{ 5,99},
	{ 6,99},
	{ 7,100}   /* 25 */
};

/* [int] apply (all) */
struct int_app_type int_app[26] = {
	3,
	5,    /* 1 */
	7,
	8,
	9,
	10,   /* 5 */
	11,
	12,
	13,
	15,
	17,   /* 10 */
	19,
	22,
	25,
	30,
	35,   /* 15 */
	40,
	45,
	50,
	53,
	55,   /* 20 */
	56,
	60,
	70,
	80,
	99    /* 25 */
};

/* [wis] apply (all) */
struct wis_app_type wis_app[26] = {
	0,   /* 0 */
	0,   /* 1 */
	0,
	0,
	0,
	0,   /* 5 */
	0,
	0,
	0,
	0,
	0,   /* 10 */
	0,
	2,
	2,
	3,
	3,   /* 15 */
	3,
	4,
	5,   /* 18 */
	6,
	6,   /* 20 */
	6,
	6,
	6,
	6,
	6   /* 25 */
};
