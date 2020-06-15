/* ************************************************************************
*  file: changes.c , Implementation of new commands.      Part of DIKUMUD *
*  Usage : New commands while new datastructures are developed.           *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"

/*   external vars  */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct title_type titles[4][25];
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct int_app_type int_app[26];
extern struct wis_app_type wis_app[26];
extern bool wizlock;

/* external functs */
extern void slog(char *msg);

void set_title(struct char_data *ch);
char *skip_spaces(char *string);
struct time_info_data age(struct char_data *ch);
void sprinttype(int type, char *names[], char *result);
void sprintbit(long vektor, char *names[], char *result);
int mana_limit(struct char_data *ch);
int hit_limit(struct char_data *ch);
int move_limit(struct char_data *ch);
int mana_gain(struct char_data *ch);
int hit_gain(struct char_data *ch);
int move_gain(struct char_data *ch);

/* To be moved to moved to act.wizard.c */

void do_noemote(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *vict;
	struct obj_data *dummy;
	char buf[MAX_INPUT_LENGTH];

	if (IS_NPC(ch))
		return;

	one_argument(argument, buf);

	if (!*buf)
		send_to_char("NOEMOTE who?\n\t", ch);

	else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
		send_to_char("Couldn't find any such creature.\n\r", ch);
	else if (IS_NPC(vict))
		send_to_char("Can't do that to a beast.\n\r", ch);
	else if (GET_LEVEL(vict) > GET_LEVEL(ch))
		act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
	else if (IS_SET(vict->specials.act, PLR_NOEMOTE))
	{
		send_to_char("You can emote again.\n\r", vict);
		send_to_char("NOEMOTE removed.\n\r", ch);
		REMOVE_BIT(vict->specials.act, PLR_NOEMOTE);
	}
	else
	{
		send_to_char("The gods take away your ability to emote!\n\r", vict);
		send_to_char("NOEMOTE set.\n\r", ch);
		SET_BIT(vict->specials.act, PLR_NOEMOTE);
	}
}


void do_notell(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *vict;
	struct obj_data *dummy;
	char buf[MAX_INPUT_LENGTH];

	if (IS_NPC(ch))
		return;

	one_argument(argument, buf);

	if (!*buf)
		if (IS_SET(ch->specials.act, PLR_NOTELL))
		{
			send_to_char("You can now hear tells again.\n\r", ch);
			REMOVE_BIT(ch->specials.act, PLR_NOTELL);
		}
		else
		{
			send_to_char("From now on, you can't use tell.\n\r", ch);
			SET_BIT(ch->specials.act, PLR_NOTELL);
		}
	else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
		send_to_char("Couldn't find any such creature.\n\r", ch);
	else if (IS_NPC(vict))
		send_to_char("Can't do that to a beast.\n\r", ch);
	else if (GET_LEVEL(vict) > GET_LEVEL(ch))
		act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
	else if (IS_SET(vict->specials.act, PLR_NOTELL))
	{
		send_to_char("You can use telepatic communication again.\n\r", vict);
		send_to_char("NOTELL removed.\n\r", ch);
		REMOVE_BIT(vict->specials.act, PLR_NOTELL);
	}
	else
	{
		send_to_char("The gods take away your ability to use telepatic communication!\n\r", vict);
		send_to_char("NOTELL set.\n\r", ch);
		SET_BIT(vict->specials.act, PLR_NOTELL);
	}
}


void do_freeze(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *vict;
	struct obj_data *dummy;
	char buf[MAX_INPUT_LENGTH];

	if (IS_NPC(ch))
		return;

	one_argument(argument, buf);

	if (!*buf)
		send_to_char("Freeze who?\n\r", ch);

	else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
		send_to_char("Couldn't find any such creature.\n\r", ch);
	else if (IS_NPC(vict))
		send_to_char("Can't do that to a beast.\n\r", ch);
	else if (GET_LEVEL(vict) > GET_LEVEL(ch))
		act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
	else if (IS_SET(vict->specials.act, PLR_FREEZE))
	{
		send_to_char("You now can do things again.\n\r", vict);
		send_to_char("FREEZE removed.\n\r", ch);
		REMOVE_BIT(vict->specials.act, PLR_FREEZE);
	}
	else
	{
		send_to_char("The gods take away your ability to ...\n\r", vict);
		send_to_char("FREEZE set.\n\r", ch);
		SET_BIT(vict->specials.act, PLR_FREEZE);
	}
}


void do_log(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *vict;
	struct obj_data *dummy;
	char buf[MAX_INPUT_LENGTH];

	if (IS_NPC(ch))
		return;

	one_argument(argument, buf);

	if (!*buf)
		send_to_char("Log who?\n\r", ch);

	else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
		send_to_char("Couldn't find any such creature.\n\r", ch);
	else if (IS_NPC(vict))
		send_to_char("Can't do that to a beast.\n\r", ch);
	else if (GET_LEVEL(vict) >= GET_LEVEL(ch))
		act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
	else if (IS_SET(vict->specials.act, PLR_LOG))
	{
		send_to_char("LOG removed.\n\r", ch);
		REMOVE_BIT(vict->specials.act, PLR_LOG);
	}
	else
	{
		send_to_char("LOG set.\n\r", ch);
		SET_BIT(vict->specials.act, PLR_LOG);
	}
}

void do_wizlock(struct char_data *ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];

	if (wizlock = !wizlock) {
		sprintf(buf,"Game has been wizlocked by %s.",GET_NAME(ch));
		slog(buf);
		send_to_char("Game wizlocked.\n\r", ch);
	} else {
		sprintf(buf,"Game has been un-wizlocked by %s.",GET_NAME(ch));
		slog(buf);
		send_to_char("Game un-wizlocked.\n\r", ch);
	}
}




/* This routine is used by 24.level ONLY to set 
   specific char/npc variables, including skills */

void do_set(struct char_data *ch, char *argument, int cmd)
{
	extern char *skill_fields[]; /* in modify.c */
	/* from spell_parser.c */
	char *spells[]= {
		"armor","teleport","bless","blindness","burning hands","call lightning",
		"charm person","chill touch","clone","colour spray","control weather",
		"create food","create water","cure blind","cure critic","cure light",
		"curse","detect evil","detect invisibility","detect magic",
		"detect poison","dispel evil","earthquake","enchant weapon",
		"energy drain","fireball","harm","heal","invisibility",
		"lightning bolt","locate object","magic missile","poison",
		"protection from evil","remove curse","sanctuary","shocking grasp",
		"zzzzz","strength","summon","ventriloquate","word of recall",
		"remove poison","sense life",
		"sneak","hide","steal","backstab","pick lock",
		"kick","bash","rescue","\n"
	};
	char *values[] = {
		"age","sex","class","level","height","weight","str","stradd",
		"int","wis","dex","con","gold","exp","mana","hit","move",
		"sessions","alignment","thirst","drunk","full","\n"
	};
	struct char_data *vict;
	char name[100], buf2[120+1], buf[100], help[MAX_STRING_LENGTH];
	int skill, field, value, i, qend;

	argument = one_argument(argument, name);
	if (!*name) /* no arguments. print an informative text */
	{
		send_to_char("Syntax:\n\rset <name> skill '<skill>' <value> <value>\n\r", ch);
		send_to_char("or:\n\rset <name> value <field> <value>\n\r", ch);

		strcpy(help, "Skill being one of the following:\n\r");
		for (i = 1; *spells[i] != '\n'; i++)
		{
			sprintf(help + strlen(help), "%18s", spells[i]);
			if (!(i % 4))
			{
				strcat(help, "\n\r");
				send_to_char(help, ch);
				*help = '\0';
			}
		}
		if (*help)
			send_to_char(help, ch);

		strcpy(help, "\n\rField being one of the following:\n\r");
		for (i = 1; *values[i] != '\n'; i++)
		{
			sprintf(help + strlen(help), "%18s", values[i]);
			if (!(i % 4))
			{
				strcat(help, "\n\r");
				send_to_char(help, ch);
				*help = '\0';
			}
		}
		if (*help)
			send_to_char(help, ch);
		send_to_char("\n\r", ch);
		return;
	}
	if (!(vict = get_char_vis(ch, name)))
	{
		send_to_char("No living thing by that name.\n\r", ch);
		return;
	}
	argument = one_argument(argument, buf);
	if (!*buf)
	{
		send_to_char("'skill' or 'value' expected.\n\r", ch);
		return;
	}
	if (strcasecmp(buf, "skill") && strcasecmp(buf, "s") && strcasecmp(buf, "value") && strcasecmp(buf, "v"))
	{
		send_to_char("'skill' or 'value' expected.\n\r", ch);
		return;
	}
	if (!strcasecmp(buf, "skill") || !strcasecmp(buf, "s"))
	/* This is a skill */
	{

		argument = skip_spaces(argument);

		/* If there is no chars in argument */
		if (!(*argument)) {
			send_to_char("Skill name expected.\n\r", ch);
			return;
		}

		if (*argument != '\'') {
			send_to_char("Skill must be enclosed in: ''\n\r",ch);
			return;
		}

		/* Locate the last quote && lowercase the magic words (if any) */

		for (qend=1; *(argument+qend) && (*(argument+qend) != '\'') ; qend++)
			*(argument+qend) = LOWER(*(argument+qend));

		if (*(argument+qend) != '\'') {
			send_to_char("Skill must be enclosed in: ''\n\r",ch);
			return;
		}

		if ((skill = old_search_block(argument, 1, qend-1, spells, 0)) < 0)
		{
			send_to_char("Unrecognized skill.\n\r", ch);
			return;
		}
		skill--;
		argument += qend+1; /* skip to next parameter */
		argument = one_argument(argument,buf);
		if (!*buf)
		{
			send_to_char("Learned value expected.\n\r", ch);
			return;
		}
		value = atoi(buf);
		if (value < 0)
		{
			send_to_char("Minimum value for learned is 0.\n\r", ch);
			return;
		}
		if (value > 100)
		{
			send_to_char("Max value for learned is 100.\n\r", ch);
			return;
		}
		argument = one_argument(argument,buf);
		if (!*buf)
		{
			send_to_char("Learned value expected.\n\r", ch);
			return;
		}
		if (strcasecmp(buf, "y") && strcasecmp(buf, "n"))
		{
			send_to_char("Recognice value must be 'y' or 'n'", ch);
			return;
		}
		sprintf(buf2,"%s changes %s's %s to %d,%s.",GET_NAME(ch),GET_NAME(vict),
		        spells[skill],value,buf);
		vict->skills[skill].learned = value;
		vict->skills[skill].recognise = (!strcasecmp(buf, "y"));
	} else {
		/* it is another value */
		argument = one_argument(argument,buf);
		if (!*buf)
		{
			send_to_char("Field name expected.\n\r", ch);
			return;
		}
		if ((skill = old_search_block(buf, 0, strlen(buf), values, 1)) < 0)
		{
			send_to_char("No such field is known. Try 'set' for list.\n\r", ch);
			return;
		}
		skill--;
		argument = one_argument(argument,buf);
		if (!*buf)
		{
			send_to_char("Value for field expected.\n\r", ch);
			return;
		}
		sprintf(buf2,"%s sets %s's %s to %s.",GET_NAME(ch),GET_NAME(vict),values[skill],buf);
		switch (skill) {
			case 0: /* age */
			{
				value = atoi(buf);
				if ((value < 16) || (value > 79))
				{
					send_to_char("Age must be more than 16 years\n\r", ch);
					send_to_char("and less than 80 years.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set age of victim */
				vict->player.time.birth = 
					time(0) - (long)value*(long)SECS_PER_MUD_YEAR;
			};
			break;
			case 1: /* sex */
			{
				if (strcasecmp(buf, "m") && strcasecmp(buf, "f") && strcasecmp(buf, "n"))
				{
					send_to_char("Sex must be 'm','f' or 'n'.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set sex of victim */
				switch(*buf) {
					case 'm':vict->player.sex = SEX_MALE;   break;
					case 'f':vict->player.sex = SEX_FEMALE; break;
					case 'n':vict->player.sex = SEX_NEUTRAL;break;
				}
			}
			break;
			case 2: /* class */
			{
				if (strcasecmp(buf, "m") && strcasecmp(buf, "c") &&
				    strcasecmp(buf, "w") && strcasecmp(buf, "t"))
				{
					send_to_char("Class must be 'm','c','w' or 't'.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set class of victim */
				switch(*buf) {
					case 'm':vict->player.class = CLASS_MAGIC_USER; break;
					case 'c':vict->player.class = CLASS_CLERIC;     break;
					case 'w':vict->player.class = CLASS_WARRIOR;    break;
					case 't':vict->player.class = CLASS_THIEF;      break;
				}
			}
			break;
			case 3: /* level */
			{
				value = atoi(buf);
				if ((value < 0) || (value > 24))
				{
					send_to_char("Level must be more than 0\n\r", ch);
					send_to_char("and less than 25.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set level of victim */
				vict->player.level = value;
			}
			break;
			case 4: /* height */
			{
				value = atoi(buf);
				if ((value < 100) || (value > 250))
				{
					send_to_char("Height must be more than 100 cm\n\r", ch);
					send_to_char("and less than 251 cm.\n\r", ch); 
					return;
				}
				slog(buf2);
				/* set hieght of victim */
				vict->player.height = value;
			}		
			break;
			case 5: /* weight */
			{
				value = atoi(buf);
				if ((value < 100) || (value > 250))
				{
					send_to_char("Weight must be more than 100 pound\n\r", ch);
					send_to_char("and less than 250 pound.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set weight of victim */
				vict->player.weight = value;
			}
			break;
			case 6: /* str */
			{
				value = atoi(buf);
				if ((value <= 0) || (value > 18))
				{
					send_to_char("Strength must be more than 0\n\r", ch);
					send_to_char("and less than 19.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set original strength of victim */
				vict->abilities.str = value;
			}
			break;
			case 7: /* stradd */
			{
				value = atoi(buf);
				if ((value <= 0) || (value > 100))
				{
					send_to_char("Strength addition must be more\n\r", ch);
					send_to_char("than 0 and less than 101.\n\r", ch);
				}
				slog(buf2);
				/* set original strength addition of victim */
				vict->abilities.str_add = value;
			}
			break;
			case 8: /* int */
			{
				value = atoi(buf);
				if ((value <= 0) || (value > 18))
				{
					send_to_char("Inteligence must be more than 0\n\r", ch);
					send_to_char("and less than 19.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set original INT of victim */
				vict->abilities.intel = value;
			}
			break;
			case 9: /* wis */
			{
				value = atoi(buf);
				if ((value <= 0) || (value > 18))
				{
					send_to_char("Wisdom must be more than 0\n\r", ch);
					send_to_char("and less than 19.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set original WIS of victim */
				vict->abilities.wis = value;
			}
			break;
			case 10: /* dex */
			{
				value = atoi(buf);
				if ((value <= 0) || (value > 18))
				{
					send_to_char("Dexterity must be more than 0\n\r", ch);
					send_to_char("and less than 19.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set original DEX of victim */
				vict->abilities.dex = value;
			}
			break;
			case 11: /* con */
			{
				value = atoi(buf);
				if ((value <= 0) || (value > 18))
				{
					send_to_char("Constitution must be more than 0\n\r", ch);
					send_to_char("and less than 19.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set original CON of victim */
				vict->abilities.con = value;
			}
			break;
			case 12: /* gold */
			{
				value = atoi(buf);
				slog(buf2);
				/* set original gold of victim */
				vict->points.gold = value;
			}
			break;
			case 13: /* exp */
			{
				value = atoi(buf);
				if ((value <= 0) || (value > 7000000))
				{
					send_to_char("Experience-points must be more than 0\n\r", ch);
					send_to_char("and less than 7000000.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set original exp of victim */
				vict->points.exp = value;
			}
			break;
			case 14: /* mana */
			{
				value = atoi(buf);
				if ((value <= -100) || (value > 200))
				{
					send_to_char("Mana-points must be more than -100\n\r", ch);
					send_to_char("and less than 200.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set original mana of victim */
				vict->points.mana = value;
			}
			break;
			case 15: /* hit */
			{
				value = atoi(buf);
				if ((value <= -10) || (value > 30000))
				{
					send_to_char("Hit-points must be more than -10\n\r", ch);
					send_to_char("and less than 30000.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set original hit of victim */
				vict->points.hit = value;
			}
			break;
			case 16: /* move */
			{
				value = atoi(buf);
				if ((value <= -100) || (value > 200))
				{
					send_to_char("Move-points must be more than -100\n\r", ch);
					send_to_char("and less than 200.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set original move of victim */
				vict->points.move = value;
			}
			break;
			case 17: /* sessions */
			{
				value = atoi(buf);
				if ((value < 0) || (value > 100))
				{
					send_to_char("Sessions must be more than 0\n\r", ch);
					send_to_char("and less than 100.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set original sessions of victim */
				vict->specials.spells_to_learn = value;
			}
			break;
			case 18: /* alignment */
			{
				value = atoi(buf);
				if ((value < -1000) || (value > 1000))
				{
					send_to_char("Alignment must be more than -1000\n\r", ch);
					send_to_char("and less than 1000.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set original alignment of victim */
				vict->specials.alignment = value;
			}
			break;
			case 19: /* thirst */
			{
				value = atoi(buf);
				if ((value < -1) || (value > 100))
				{
					send_to_char("Thirst must be more than -2\n\r", ch);
					send_to_char("and less than 101.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set original thirst of victim */
				vict->specials.conditions[THIRST] = value;
			}
			break;
			case 20: /* drunk */
			{
				value = atoi(buf);
				if ((value < -1) || (value > 100))
				{
					send_to_char("Drunk must be more than -2\n\r", ch);
					send_to_char("and less than 101.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set original drunk of victim */
				vict->specials.conditions[DRUNK] = value;
			}
			break;
			case 21: /* full */
			{
				value = atoi(buf);
				if ((value < -1) || (value > 100))
				{
					send_to_char("Full must be more than -2\n\r", ch);
					send_to_char("and less than 101.\n\r", ch);
					return;
				}
				slog(buf2);
				/* set original full of victim */
				vict->specials.conditions[FULL] = value;
			}
			break;
		}
	}
	send_to_char("Ok.\n\r", ch);
}

void do_wiz(struct char_data *ch, char *argument, int cmd)
{
	static char buf1[MAX_STRING_LENGTH];
  struct descriptor_data *i;

	for (; *argument == ' '; argument++);

	if (!(*argument))
		send_to_char("What do you want to tell all gods and immortals?\n\r", ch);
	else
	{
		send_to_char("Ok.\n\r", ch);
		sprintf(buf1, "::$n::%s", argument);

    	for (i = descriptor_list; i; i = i->next)
      	if (i->character != ch && !i->connected && 
			    GET_LEVEL(i->character) > 20)
				act(buf1, 0, ch, 0, i->character, TO_VICT);
	}
}


