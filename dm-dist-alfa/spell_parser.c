/* ************************************************************************
*  file: spell_parser.c , Basic routines and parsing      Part of DIKUMUD *
*  Usage : Interpreter of spells                                          *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h" 
#include "spells.h"
#include "handler.h"

#define MANA_MU 1
#define MANA_CL 1

#define SPELLO(nr, beat, pos, mlev, clev, mana, tar, func) { \
	spell_info[nr].spell_pointer = (func);    \
	spell_info[nr].beats = (beat);            \
	spell_info[nr].minimum_position = (pos);  \
	spell_info[nr].min_usesmana = (mana);     \
	spell_info[nr].min_level_cleric = (clev); \
	spell_info[nr].min_level_magic = (mlev);  \
	spell_info[nr].targets = (tar);           \
}

#define SPELL_LEVEL(ch, sn)               \
  ( (GET_CLASS(ch) == CLASS_CLERIC) ?     \
  spell_info[sn].min_level_cleric : spell_info[sn].min_level_magic)


/* 100 is the MAX_MANA for a character */
#define USE_MANA(ch, sn)                            \
  MAX(spell_info[sn].min_usesmana, 100/(2+GET_LEVEL(ch)-SPELL_LEVEL(ch,sn)))

/* Global data */

extern struct room_data *world;
extern struct char_data *character_list;
extern char *spell_wear_off_msg[];


/* Extern procedures */
void cast_armor( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_teleport( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_bless( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_blindness( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_burning_hands( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_call_lightning( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_charm_person( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_chill_touch( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_shocking_grasp( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_clone( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_colour_spray( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_control_weather( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_food( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_water( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_blind( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_critic( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_light( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_curse( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_evil( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_invisibility( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_magic( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_poison( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_evil( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_earthquake( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_enchant_weapon( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_energy_drain( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_fireball( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_harm( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_heal( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_invisibility( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_lightning_bolt( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_locate_object( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_magic_missile( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_poison( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_protection_from_evil( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_remove_curse( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sanctuary( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sleep( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_strength( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_summon( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_ventriloquate( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_word_of_recall( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_remove_poison( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sense_life( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_identify( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);



struct spell_info_type spell_info[MAX_SPL_LIST];

char *spells[]=
{
   "armor",               /* 1 */
   "teleport",
   "bless",
   "blindness",
   "burning hands",
   "call lightning",
   "charm person",
   "chill touch",
   "clone",
   "colour spray",
   "control weather",     /* 11 */
   "create food",
   "create water",
   "cure blind",
   "cure critic",
   "cure light",
   "curse",
   "detect evil",
   "detect invisibility",
   "detect magic",
   "detect poison",       /* 21 */
   "dispel evil",
   "earthquake",
   "enchant weapon",
   "energy drain",
   "fireball",
   "harm",
   "heal",
   "invisibility",
   "lightning bolt",
   "locate object",      /* 31 */
   "magic missile",
   "poison",
   "protection from evil",
   "remove curse",
   "sanctuary",
   "shocking grasp",
   "zzzzz",
   "strength",
   "summon",
   "ventriloquate",      /* 41 */
   "word of recall",
   "remove poison",
   "sense life",         /* 44 */

   /* RESERVED SKILLS */
   "SKILL_SNEAK",        /* 45 */
   "SKILL_HIDE",
   "SKILL_STEAL",
   "SKILL_BACKSTAB",
   "SKILL_PICK_LOCK",
   "SKILL_KICK",         /* 50 */
   "SKILL_BASH",
   "SKILL_RESCUE",
   /* NON-CASTABLE SPELLS (Scrolls/potions/wands/staffs) */

   "identify",           /* 53 */
   "\n"
};


const byte saving_throws[4][5][25] = {
{
  {16,14,14,14,14,14,13,13,13,13,13,11,11,11,11,11,10,10,10,10,10, 8, 6, 4, 0},
  {13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1, 0},
  {15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2, 0},
  {17,15,15,15,15,15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 5, 3, 0},
  {14,12,12,12,12,12,10,10,10,10,10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 0}
}, {
  {11,10,10,10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 0},
  {16,14,14,14,13,13,13,11,11,11,10,10,10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 0},
  {15,13,13,13,12,12,12,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 0},
  {18,16,16,16,15,15,15,13,13,13,12,12,12,11,11,11,10,10,10, 8, 8, 7, 6, 5, 0},
  {17,15,15,15,14,14,14,12,12,12,11,11,11,10,10,10, 9, 9, 9, 7, 7, 6, 5, 4, 0}
}, {
  {15,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 7, 6, 0},
  {16,14,14,14,14,12,12,12,12,10,10,10,10, 8, 8, 8, 8, 6, 6, 6, 6, 4, 3, 2, 0},
  {14,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 5, 3, 0},
  {18,16,16,16,16,15,15,15,15,14,14,14,14,13,13,13,13,12,12,12,12,11, 9, 5, 0},
  {17,15,15,15,15,13,13,13,13,11,11,11,11, 9, 9, 9, 9, 7, 7, 7, 7, 5, 3, 1, 0}
}, {
  {16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 0},
  {18,16,16,15,15,13,13,12,12,10,10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 0},
  {17,15,15,14,14,12,12,11,11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 0},
  {20,17,17,16,16,13,13,12,12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 0},
  {19,17,17,16,16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 0}
}
};



void affect_update( void )
{
	static struct affected_type *af, *next_af_dude;
	static struct char_data *i;

	for (i = character_list; i; i = i->next)
		for (af = i->affected; af; af = next_af_dude) {
			next_af_dude = af->next;
			if (af->duration >= 1)
				af->duration--;
			else {
				if ((af->type > 0) && (af->type <= 52)) /* It must be a spell */
					if (!af->next || (af->next->type != af->type) ||
					    (af->next->duration > 0))
						if (*spell_wear_off_msg[af->type]) {
							send_to_char(spell_wear_off_msg[af->type], i);
							send_to_char("\n\r", i);
						}

				affect_remove(i, af);
			}
		}
}


void clone_char(struct char_data *ch)
{
	extern struct index_data *mob_index;
	struct char_data *clone;
	struct affected_type *af;
	int i;

	CREATE(clone, struct char_data, 1);


	clear_char(clone);       /* Clear EVERYTHING! (ASSUMES CORRECT) */

	clone->player    = ch->player;
	clone->abilities = ch->abilities;

	for (i=0; i<5; i++)
		clone->specials.apply_saving_throw[i] = ch->specials.apply_saving_throw[i];

	for (af=ch->affected; af; af = af->next)
		affect_to_char(clone, af);

	for (i=0; i<3; i++)
		GET_COND(clone,i) = GET_COND(ch, i);

	clone->points = ch->points;

	for (i=0; i<MAX_SKILLS; i++)
		clone->skills[i] = ch->skills[i];

	clone->specials = ch->specials;
	clone->specials.fighting = 0;

	GET_NAME(clone) = strdup(GET_NAME(ch));

	clone->player.short_descr = strdup(ch->player.short_descr);

	clone->player.long_descr = strdup(ch->player.long_descr);

	clone->player.description = 0;
	/* REMEMBER EXTRA DESCRIPTIONS */

	GET_TITLE(clone) = strdup(GET_TITLE(ch));

	clone->nr = ch->nr;

	if (IS_NPC(clone))
		mob_index[clone->nr].number++;
	else { /* Make PC's into NPC's */
		clone->nr = -1;
		SET_BIT(clone->specials.act, ACT_ISNPC);
	}

	clone->desc = 0;
	clone->followers = 0;
	clone->master = 0;

	clone->next = character_list;
	character_list = clone;

	char_to_room(clone, ch->in_room);
}



void clone_obj(struct obj_data *obj)
{
	struct obj_data *clone;
	struct extra_descr_data *ed, temp;


	CREATE(clone, struct obj_data, 1);

	*clone = *obj;

	clone->name               = strdup(obj->name);
	clone->description        = strdup(obj->description);
	clone->short_description  = strdup(obj->short_description);
	clone->action_description = strdup(obj->action_description);
	clone->ex_description     = 0;

	/* REMEMBER EXTRA DESCRIPTIONS */
	clone->carried_by         = 0;
	clone->in_obj             = 0;
	clone->contains           = 0;
	clone->next_content       = 0;
	clone->next               = 0;

	/* VIRKER IKKE ENDNU */
}



/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(struct char_data *ch, struct char_data *victim)
{
	struct char_data *k;

	for(k=victim; k; k=k->master) {
		if (k == ch)
			return(TRUE);
	}

	return(FALSE);
}



/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data *ch)
{
	struct follow_type *j, *k;

	assert(ch->master);

	if (IS_AFFECTED(ch, AFF_CHARM)) {
		act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
		act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
		act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
		if (affected_by_spell(ch, SPELL_CHARM_PERSON))
			affect_from_char(ch, SPELL_CHARM_PERSON);
	} else {
		act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
		act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
		act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
	}

	if (ch->master->followers->follower == ch) { /* Head of follower-list? */
		k = ch->master->followers;
		ch->master->followers = k->next;
		free(k);
	} else { /* locate follower who is not head of list */
		for(k = ch->master->followers; k->next->follower!=ch; k=k->next)  ;

		j = k->next;
		k->next = j->next;
		free(j);
	}

	ch->master = 0;
	REMOVE_BIT(ch->specials.affected_by, AFF_CHARM | AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void die_follower(struct char_data *ch)
{
	struct follow_type *j, *k;

	if (ch->master)
		stop_follower(ch);

	for (k=ch->followers; k; k=j) {
		j = k->next;
		stop_follower(k->follower);
	}
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data *ch, struct char_data *leader)
{
	struct follow_type *k;

	assert(!ch->master);

	ch->master = leader;

	CREATE(k, struct follow_type, 1);

	k->follower = ch;
	k->next = leader->followers;
	leader->followers = k;

	act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
	act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
	act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
}



void say_spell( struct char_data *ch, int si )
{
	char buf[MAX_STRING_LENGTH], splwd[MAX_BUF_LENGTH];
	char buf2[MAX_STRING_LENGTH+23];

	int j, offs;
	struct char_data *temp_char;


	struct syllable {
		char org[10];
		char new[10];
	};

	struct syllable syls[] = {
	{ " ", " " },
	{ "ar", "abra"   },
	{ "au", "kada"    },
	{ "bless", "fido" },
  { "blind", "nose" },
  { "bur", "mosa" },
	{ "cu", "judi" },
	{ "de", "oculo"},
	{ "en", "unso" },
	{ "light", "dies" },
	{ "lo", "hi" },
	{ "mor", "zak" },
	{ "move", "sido" },
  { "ness", "lacri" },
  { "ning", "illa" },
	{ "per", "duda" },
	{ "ra", "gru"   },
  { "re", "candus" },
	{ "son", "sabru" },
  { "tect", "infra" },
	{ "tri", "cula" },
	{ "ven", "nofo" },
	{"a", "a"},{"b","b"},{"c","q"},{"d","e"},{"e","z"},{"f","y"},{"g","o"},
	{"h", "p"},{"i","u"},{"j","y"},{"k","t"},{"l","r"},{"m","w"},{"n","i"},
	{"o", "a"},{"p","s"},{"q","d"},{"r","f"},{"s","g"},{"t","h"},{"u","j"},
	{"v", "z"},{"w","x"},{"x","n"},{"y","l"},{"z","k"}, {"",""}
	};



	strcpy(buf, "");
	strcpy(splwd, spells[si-1]);

	offs = 0;

	while(*(splwd+offs)) {
		for(j=0; *(syls[j].org); j++)
			if (strncmp(syls[j].org, splwd+offs, strlen(syls[j].org))==0) {
				strcat(buf, syls[j].new);
				if (strlen(syls[j].org))
					offs+=strlen(syls[j].org);
				else
					++offs;
			}
	}


	sprintf(buf2,"$n utters the words, '%s'", buf);
	sprintf(buf, "$n utters the words, '%s'", spells[si-1]);

	for(temp_char = world[ch->in_room].people;
		temp_char;
		temp_char = temp_char->next_in_room)
		if(temp_char != ch) {
			if (GET_CLASS(ch) == GET_CLASS(temp_char))
				act(buf, FALSE, ch, 0, temp_char, TO_VICT);
			else
				act(buf2, FALSE, ch, 0, temp_char, TO_VICT);

		}

}



bool saves_spell(struct char_data *ch, sh_int save_type)
{
	int save;

	/* Negative apply_saving_throw makes saving throw better! */

	save = ch->specials.apply_saving_throw[save_type];

	if (!IS_NPC(ch)) {
		save += saving_throws[GET_CLASS(ch)-1][save_type][GET_LEVEL(ch)];
		if (GET_LEVEL(ch) > 20)
			return(TRUE);
	}

	return(MAX(1,save) < number(1,20));
}



char *skip_spaces(char *string)
{
	for(;*string && (*string)==' ';string++);

	return(string);
}



/* Assumes that *argument does start with first letter of chopped string */

void do_cast(struct char_data *ch, char *argument, int cmd)
{
	struct obj_data *tar_obj;
	struct char_data *tar_char;
	char name[MAX_STRING_LENGTH];
	int qend, spl, i;
	bool target_ok;

	if (IS_NPC(ch))
		return;

	if (GET_LEVEL(ch) < 21) {
		if (GET_CLASS(ch) == CLASS_WARRIOR) {
				send_to_char("Think you had better stick to fighting...\n\r", ch);
				return;
		}	else if (GET_CLASS(ch) == CLASS_THIEF) {
				send_to_char("Think you should stick to robbing and killing...\n\r", ch);
				return;
		}
	}

	argument = skip_spaces(argument);

	/* If there is no chars in argument */
	if (!(*argument)) {
		send_to_char("Cast which what where?\n\r", ch);
		return;
	}

	if (*argument != '\'') {
		send_to_char("Magic must always be enclosed by the holy magic symbols : '\n\r",ch);
		return;
	}

	/* Locate the last quote && lowercase the magic words (if any) */

	for (qend=1; *(argument+qend) && (*(argument+qend) != '\'') ; qend++)
		*(argument+qend) = LOWER(*(argument+qend));

	if (*(argument+qend) != '\'') {
		send_to_char("Magic must always be enclosed by the holy magic symbols : '\n\r",ch);
		return;
	}

	spl = old_search_block(argument, 1, qend-1,spells, 0);

	if (!spl) {
		send_to_char("Your lips do not move, no magic appears.\n\r",ch);
		return;
	}

	if ((spl > 0) && (spl <= 44) && spell_info[spl].spell_pointer) {
		if (GET_POS(ch) < spell_info[spl].minimum_position) {
			switch(GET_POS(ch)) {
				case POSITION_SLEEPING :
					send_to_char("You dream about great magical powers.\n\r", ch);
					break;
				case POSITION_RESTING :
					send_to_char("You can't concentrate enough while resting.\n\r",ch);
					break;
				case POSITION_SITTING :
					send_to_char("You can't do this sitting!\n\r", ch);
					break;
				case POSITION_FIGHTING :
					send_to_char("Impossible! You can't concentrate enough!.\n\r", ch);
					break;
				default:
					send_to_char("It seems like you're in a pretty bad shape!\n\r",ch);
					break;
			} /* Switch */
		}	else {

			if (GET_LEVEL(ch) < 21) {
				if ((GET_CLASS(ch) == CLASS_MAGIC_USER) &&
				   (spell_info[spl].min_level_magic > GET_LEVEL(ch))) {
					send_to_char("Sorry, you can't do that.\n\r", ch);
					return;
				}
				if ((GET_CLASS(ch) == CLASS_CLERIC) &&
				   (spell_info[spl].min_level_cleric > GET_LEVEL(ch))) {
					send_to_char("Sorry, you can't do that.\n\r", ch);
					return;
				}
			}

			argument+=qend+1;	/* Point to the last ' */
			for(;*argument == ' '; argument++);

			/* **************** Locate targets **************** */

			target_ok = FALSE;
			tar_char = 0;
			tar_obj = 0;

			if (!IS_SET(spell_info[spl].targets, TAR_IGNORE)) {

				argument = one_argument(argument, name);

				if (*name) {
					if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
						if (tar_char = get_char_room_vis(ch, name))
							target_ok = TRUE;

					if (!target_ok && IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
						if (tar_char = get_char_vis(ch, name))
							target_ok = TRUE;

					if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
						if (tar_obj = get_obj_in_list_vis(ch, name, ch->carrying))
							target_ok = TRUE;

					if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
						if (tar_obj = get_obj_in_list_vis(ch, name, world[ch->in_room].contents))
							target_ok = TRUE;

					if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
						if (tar_obj = get_obj_vis(ch, name))
							target_ok = TRUE;

					if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP)) {
						for(i=0; i<MAX_WEAR && !target_ok; i++)
							if (ch->equipment[i] && strcasecmp(name, ch->equipment[i]->name) == 0) {
								tar_obj = ch->equipment[i];
								target_ok = TRUE;
							}
					}

					if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY))
						if (strcasecmp(GET_NAME(ch), name) == 0) {
							tar_char = ch;
							target_ok = TRUE;
						}

				} else { /* No argument was typed */

					if (IS_SET(spell_info[spl].targets, TAR_FIGHT_SELF))
						if (ch->specials.fighting) {
							tar_char = ch;
							target_ok = TRUE;
						}

					if (!target_ok && IS_SET(spell_info[spl].targets, TAR_FIGHT_VICT))
						if (ch->specials.fighting) {
							/* WARNING, MAKE INTO POINTER */
							tar_char = ch->specials.fighting;
							target_ok = TRUE;
						}

					if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
						tar_char = ch;
						target_ok = TRUE;
					}

				}

			} else {
				target_ok = TRUE; /* No target, is a good target */
			}

			if (!target_ok) {
				if (*name) {
					if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
						send_to_char("Nobody here by that name.\n\r", ch);
					else if (IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
						send_to_char("Nobody playing by that name.\n\r", ch);
					else if (IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
						send_to_char("You are not carrying anything like that.\n\r", ch);
					else if (IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
						send_to_char("Nothing here by that name.\n\r", ch);
					else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
						send_to_char("Nothing at all by that name.\n\r", ch);
					else if (IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP))
						send_to_char("You are not wearing anything like that.\n\r", ch);
					else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
						send_to_char("Nothing at all by that name.\n\r", ch);

				} else { /* Nothing was given as argument */
					if (spell_info[spl].targets < TAR_OBJ_INV)
						send_to_char("Who should the spell be cast upon?\n\r", ch);
					else
						send_to_char("What should the spell be cast upon?\n\r", ch);
				}
				return;
			} else { /* TARGET IS OK */
				if ((tar_char == ch) && IS_SET(spell_info[spl].targets, TAR_SELF_NONO)) {
					send_to_char("You can not cast this spell upon yourself.\n\r", ch);
					return;
				}
				else if ((tar_char != ch) && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
					send_to_char("You can only cast this spell upon yourself.\n\r", ch);
					return;
				} else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tar_char)) {
					send_to_char("You are afraid that it could harm your master.\n\r", ch);
					return;
				}
			}

			if (GET_LEVEL(ch) < 21) {
				if (GET_MANA(ch) < USE_MANA(ch, spl)) {
					send_to_char("You can't summon enough energy to cast the spell.\n\r", ch);
					return;
				}
			}

			if (spl != SPELL_VENTRILOQUATE)  /* :-) */
				say_spell(ch, spl);

			WAIT_STATE(ch, spell_info[spl].beats);

			if ((spell_info[spl].spell_pointer == 0) && spl>0)
				send_to_char("Sorry, this magic has not yet been implemented :(\n\r", ch);
			else {
				if (number(1,101) > ch->skills[spl].learned) { /* 101% is failure */
					send_to_char("You lost your concentration!\n\r", ch);
					GET_MANA(ch) -= (USE_MANA(ch, spl)>>1);
					return;
				}
				send_to_char("Ok.\n\r",ch);
				((*spell_info[spl].spell_pointer) (GET_LEVEL(ch), ch, argument, SPELL_TYPE_SPELL, tar_char, tar_obj));
				GET_MANA(ch) -= (USE_MANA(ch, spl));
			}

		}	/* if GET_POS < min_pos */

		return;
	}

	switch (number(1,5)){
		case 1: send_to_char("Bylle Grylle Grop Gryf???\n\r", ch); break;
		case 2: send_to_char("Olle Bolle Snop Snyf?\n\r",ch); break;
		case 3: send_to_char("Olle Grylle Bolle Bylle?!?\n\r",ch); break;
		case 4: send_to_char("Gryffe Olle Gnyffe Snop???\n\r",ch); break;
	  default: send_to_char("Bolle Snylle Gryf Bylle?!!?\n\r",ch); break;
	}
}


void assign_spell_pointers(void)
{
	int i;

	for(i=0; i<MAX_SPL_LIST; i++)
		spell_info[i].spell_pointer = 0;


	/* From spells1.c */

	SPELLO(32,12,POSITION_FIGHTING, 1, 21, 15,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_magic_missile);

	SPELLO( 8,12,POSITION_FIGHTING, 3, 21, 15,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_chill_touch);

	SPELLO( 5,12,POSITION_FIGHTING, 5, 21, 15,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_burning_hands);

	SPELLO(37,12,POSITION_FIGHTING, 7, 21, 15,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_shocking_grasp);

	SPELLO(30,12,POSITION_FIGHTING, 9, 21, 15,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_lightning_bolt);

	SPELLO(10,12,POSITION_FIGHTING, 11,21, 15,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_colour_spray);

	SPELLO(25,12,POSITION_FIGHTING, 13,21, 35,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_energy_drain);

	SPELLO(26,12,POSITION_FIGHTING, 15,21, 15,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_fireball);

	SPELLO(23,12,POSITION_FIGHTING, 21, 7, 15,
	 TAR_IGNORE, cast_earthquake);

	SPELLO(22,12,POSITION_FIGHTING, 21, 10, 15,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_dispel_evil);

	SPELLO( 6,12,POSITION_FIGHTING, 21, 12, 15,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_call_lightning);

	SPELLO(27,12,POSITION_FIGHTING, 21, 15, 35,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_harm);



	/* Spells2.c */

	SPELLO( 1,12,POSITION_STANDING, 5,  1, 5,
	 TAR_CHAR_ROOM, cast_armor);

	SPELLO( 2,12,POSITION_FIGHTING, 8, 21, 35,
	 TAR_SELF_ONLY, cast_teleport);

	SPELLO( 3,12,POSITION_STANDING,21,  5, 5,
	 TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_ROOM, cast_bless);

	SPELLO( 4,12,POSITION_STANDING, 8,  6, 5,
	 TAR_CHAR_ROOM, cast_blindness);

	SPELLO(7,12,POSITION_STANDING, 14, 21, 5,
	 TAR_CHAR_ROOM | TAR_SELF_NONO, cast_charm_person);

	SPELLO( 9,12,POSITION_STANDING,15, 21, 40,
	 TAR_CHAR_ROOM, cast_clone);

	SPELLO(11,12,POSITION_STANDING,10, 13, 25,
	 TAR_IGNORE, cast_control_weather);

	SPELLO(12,12,POSITION_STANDING,21,  3, 5,
	 TAR_IGNORE, cast_create_food);

	SPELLO(13,12,POSITION_STANDING,21,  2, 5,
	 TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_create_water);

	SPELLO(14,12,POSITION_STANDING,21,  4, 5,
	 TAR_CHAR_ROOM, cast_cure_blind);

	SPELLO(15,12,POSITION_FIGHTING,21,  9, 20,
	 TAR_CHAR_ROOM, cast_cure_critic);

	SPELLO(16,12,POSITION_FIGHTING,21,  1, 15,
	 TAR_CHAR_ROOM, cast_cure_light);

	SPELLO(17,12,POSITION_STANDING,12, 21, 20,
	 TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_curse);

	SPELLO(18,12,POSITION_STANDING,21,  4, 5,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_detect_evil);

	SPELLO(19,12,POSITION_STANDING, 2,  5, 5,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_detect_invisibility);

	SPELLO(20,12,POSITION_STANDING, 2,  3, 5,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_detect_magic);

	SPELLO(21,12,POSITION_STANDING,21,  2, 5,
	 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_detect_poison);

	SPELLO(24,12,POSITION_STANDING,12, 21, 100,
	 TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_enchant_weapon);

	SPELLO(28,12,POSITION_FIGHTING,21, 14, 50,
	 TAR_CHAR_ROOM, cast_heal);

	SPELLO(29,12,POSITION_STANDING, 4, 21, 5,
	 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP, cast_invisibility);

	SPELLO(31,12,POSITION_STANDING, 6, 10, 20,
	 TAR_OBJ_WORLD, cast_locate_object);

	SPELLO(33,12,POSITION_STANDING,21,  8, 10,
	 TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_poison);

	SPELLO(34,12,POSITION_STANDING,21,  6, 5,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_protection_from_evil);

	SPELLO(35,12,POSITION_STANDING,21, 12, 5,
	 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM, cast_remove_curse);

	SPELLO(36,12,POSITION_STANDING,21, 13, 75,
	 TAR_CHAR_ROOM, cast_sanctuary);

	SPELLO(38,12,POSITION_STANDING,14, 21, 15,
	 TAR_CHAR_ROOM, cast_sleep);

	SPELLO(39,12,POSITION_STANDING, 7, 21, 20,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_strength);

	SPELLO(40,12,POSITION_STANDING,21,  8, 50,
	 TAR_CHAR_WORLD, cast_summon);

	SPELLO(41,12,POSITION_STANDING, 1, 21, 5,
	 TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_SELF_NONO, cast_ventriloquate);

	SPELLO(42,12,POSITION_STANDING,21, 11, 5,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_word_of_recall);

	SPELLO(43,12,POSITION_STANDING,21,  9, 5,
	 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, cast_remove_poison);

	SPELLO(44,12,POSITION_STANDING,21,  7, 5,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_sense_life);

	SPELLO(45,0,POSITION_STANDING,25,25,200, TAR_IGNORE, 0);
	SPELLO(46,0,POSITION_STANDING,25,25,200, TAR_IGNORE, 0);
	SPELLO(47,0,POSITION_STANDING,25,25,200, TAR_IGNORE, 0);
	SPELLO(48,0,POSITION_STANDING,25,25,200, TAR_IGNORE, 0);
	SPELLO(49,0,POSITION_STANDING,25,25,200, TAR_IGNORE, 0);
	SPELLO(50,0,POSITION_STANDING,25,25,200, TAR_IGNORE, 0);
	SPELLO(51,0,POSITION_STANDING,25,25,200, TAR_IGNORE, 0);
	SPELLO(52,0,POSITION_STANDING,25,25,200, TAR_IGNORE, 0);

	SPELLO(53,1,POSITION_STANDING,25,25, 100, TAR_IGNORE, cast_identify);

}
