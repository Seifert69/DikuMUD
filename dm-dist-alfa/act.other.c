/* ************************************************************************
*  file: act.other.c , Implementation of commands.        Part of DIKUMUD *
*  Usage : Other commands.                                                *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"

/* extern variables */

extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct dex_skill_type dex_app_skill[];
extern struct spell_info_type spell_info[];


/* extern procedures */
extern void slog(char *str);


void hit(struct char_data *ch, struct char_data *victim, int type);
void do_shout(struct char_data *ch, char *argument, int cmd);



void do_qui(struct char_data *ch, char *argument, int cmd)
{
	send_to_char("You have to write quit - no less, to quit!\n\r",ch);
	return;
}

void do_quit(struct char_data *ch, char *argument, int cmd)
{
	void do_save(struct char_data *ch, char *argument, int cmd);
	void die(struct char_data *ch);

	if (IS_NPC(ch) || !ch->desc)
		return;

	if (GET_POS(ch) == POSITION_FIGHTING) {
		send_to_char("No way! You are fighting.\n\r", ch);
		return;
	}

	if (GET_POS(ch) < POSITION_STUNNED) {
		send_to_char("You die before your time!\n\r", ch);
		die(ch);
		return;
	}

	act("Goodbye, friend.. Come back soon!", FALSE, ch, 0, 0, TO_CHAR);
	act("$n has left the game.", TRUE, ch,0,0,TO_ROOM);
	extract_char(ch); /* Char is saved in extract char */
}



void do_save(struct char_data *ch, char *argument, int cmd)
{
	char buf[100];

	if (IS_NPC(ch) || !ch->desc)
		return;

	sprintf(buf, "Saving %s.\n\r", GET_NAME(ch));
	send_to_char(buf, ch);
	save_char(ch, NOWHERE);
}


void do_not_here(struct char_data *ch, char *argument, int cmd)
{
	send_to_char("Sorry, but you cannot do that here!\n\r",ch);
}



void do_sneak(struct char_data *ch, char *argument, int cmd)
{
	struct affected_type af;
	byte percent;

	send_to_char("Ok, you'll try to move silently for a while.\n\r", ch);
	if (IS_AFFECTED(ch, AFF_SNEAK))
		affect_from_char(ch, SKILL_SNEAK);

	percent=number(1,101); /* 101% is a complete failure */

	if (percent > ch->skills[SKILL_SNEAK].learned +
	    dex_app_skill[GET_DEX(ch)].sneak)
		return;

	af.type = SKILL_SNEAK;
	af.duration = GET_LEVEL(ch);
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_SNEAK;
	affect_to_char(ch, &af);
}



void do_hide(struct char_data *ch, char *argument, int cmd)
{
	byte percent;

	send_to_char("You attempt to hide yourself.\n\r", ch);

	if (IS_AFFECTED(ch, AFF_HIDE))
		REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

	percent=number(1,101); /* 101% is a complete failure */

	if (percent > ch->skills[SKILL_HIDE].learned +
	    dex_app_skill[GET_DEX(ch)].hide)
		return;

	SET_BIT(ch->specials.affected_by, AFF_HIDE);
}


void do_steal(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim;
	struct obj_data *obj;
	char victim_name[240];
	char obj_name[240];
	char buf[240];
	int percent, bits;
	bool equipment = FALSE;
	int gold, eq_pos;
	bool ohoh = FALSE;



	argument = one_argument(argument, obj_name);
	one_argument(argument, victim_name);


	if (!(victim = get_char_room_vis(ch, victim_name))) {
		send_to_char("Steal what from who?\n\r", ch);
		return;
	} else if (victim == ch) {
		send_to_char("Come on now, that's rather stupid!\n\r", ch);
		return;
	}

	if ((GET_EXP(ch) < 1250) && (!IS_NPC(victim))) {
	  send_to_char("Due to misuse of steal, you can't steal from other players\n\r", ch);
	  send_to_char("unless you got at least 1,250 experience points.\n\r", ch);
	  return;
	}

	WAIT_STATE(ch, 10); /* It takes TIME to steal */

	/* 101% is a complete failure */
	percent=number(1,101) - dex_app_skill[GET_DEX(ch)].p_pocket;

   percent += AWAKE(victim) ? 10 : -50;

	if (GET_POS(victim) < POSITION_SLEEPING)
		percent = -1; /* ALWAYS SUCCESS */

	if (GET_LEVEL(victim)>20) /* NO NO With Imp's and Shopkeepers! */
		percent = 101; /* Failure */

	if (strcasecmp(obj_name, "coins") && strcasecmp(obj_name,"gold")) {

		if (!(obj = get_obj_in_list_vis(victim, obj_name, victim->carrying))) {

			for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
				if (victim->equipment[eq_pos] &&
				   (isname(obj_name, victim->equipment[eq_pos]->name)) &&
						CAN_SEE_OBJ(ch,victim->equipment[eq_pos])) {
					obj = victim->equipment[eq_pos];
					break;
				}

			if (!obj) {
				act("$E has not got that item.",FALSE,ch,0,victim,TO_CHAR);
				return;
			} else { /* It is equipment */
				if ((GET_POS(victim) > POSITION_STUNNED)) {
					send_to_char("Steal the equipment now? Impossible!\n\r", ch);
					return;
				} else {
					act("You unequip $p and steal it.",FALSE, ch, obj ,0, TO_CHAR);
					act("$n steals $p from $N.",FALSE,ch,obj,victim,TO_NOTVICT);
					obj_to_char(unequip_char(victim, eq_pos), ch);
				}
			}
		} else {  /* obj found in inventory */

			percent += GET_OBJ_WEIGHT(obj); /* Make heavy harder */

			if (percent > ch->skills[SKILL_STEAL].learned) {
				ohoh = TRUE;
				act("Oops..", FALSE, ch,0,0,TO_CHAR);
				act("$n tried to steal something from you!",FALSE,ch,0,victim,TO_VICT);
				act("$n tries to steal something from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
			} else { /* Steal the item */
				if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
					if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
						obj_from_char(obj);
						obj_to_char(obj, ch);
						send_to_char("Got it!\n\r", ch);
					}
				} else
					send_to_char("You cannot carry that much.\n\r", ch);
			}
		}
	} else { /* Steal some coins */
		if (percent > ch->skills[SKILL_STEAL].learned) {
			ohoh = TRUE;
			act("Oops..", FALSE, ch,0,0,TO_CHAR);
			act("You discover that $n has $s hands in your wallet.",FALSE,ch,0,victim,TO_VICT);
			act("$n tries to steal gold from $N.",TRUE, ch, 0, victim, TO_NOTVICT);
		} else {
			/* Steal some gold coins */
			gold = (int) ((GET_GOLD(victim)*number(1,10))/100);
			gold = MIN(1782, gold);
			if (gold > 0) {
				GET_GOLD(ch) += gold;
				GET_GOLD(victim) -= gold;
				sprintf(buf, "Bingo! You got %d gold coins.\n\r", gold);
				send_to_char(buf, ch);
			} else {
				send_to_char("You couldn't get any gold...\n\r", ch);
			}
		}
	}

	if (ohoh && IS_NPC(victim) && AWAKE(victim))
		if (IS_SET(victim->specials.act, ACT_NICE_THIEF)) {
			sprintf(buf, "%s is a bloody thief.", GET_NAME(ch));
			do_shout(victim, buf, 0);
			slog(buf);
			send_to_char("Don't you ever do that again!\n\r", ch);
		} else {
			hit(victim, ch, TYPE_UNDEFINED);
		}

}


#ifdef JKL

void do_pick(struct char_data *ch, char *argument, int cmd)
{
	byte percent;

	percent=number(1,101); /* 101% is a complete failure */

	if (percent > (ch->skills[SKILL_PICK_LOCK].learned +
	    dex_app_skill[GET_DEX(ch)].p_locks)) {
		send_to_char("You failed to pick the lock.\n\r", ch);
		return;
	}
}


#endif

void do_practice(struct char_data *ch, char *arg, int cmd) {
	send_to_char("You can only practise in a guild.\n\r", ch);
}






void do_idea(struct char_data *ch, char *argument, int cmd)
{
	FILE *fl;
	char str[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
	{
		send_to_char("Monsters can't have ideas - Go away.\n\r", ch);
		return;
	}

	/* skip whites */
	for (; isspace(*argument); argument++);

	if (!*argument)
	{
		send_to_char("That doesn't sound like a good idea to me.. Sorry.\n\r",
			ch);
		return;
	}

	if (!(fl = fopen(IDEA_FILE, "a")))
	{
		perror ("do_idea");
		send_to_char("Could not open the idea-file.\n\r", ch);
		return;
	}

	sprintf(str, "**%s: %s\n", GET_NAME(ch), argument);
	fputs(str, fl);
	fclose(fl);
	send_to_char("Ok. Thanks.\n\r", ch);
}







void do_typo(struct char_data *ch, char *argument, int cmd)
{
	FILE *fl;
	char str[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
	{
		send_to_char("Monsters can't spell - leave me alone.\n\r", ch);
		return;
	}

	/* skip whites */
	for (; isspace(*argument); argument++);

	if (!*argument)
	{
		send_to_char("I beg your pardon?\n\r",
			ch);
		return;
	}

	if (!(fl = fopen(TYPO_FILE, "a")))
	{
		perror ("do_typo");
		send_to_char("Could not open the typo-file.\n\r", ch);
		return;
	}

	sprintf(str, "**%s[%d]: %s\n",
		GET_NAME(ch), world[ch->in_room].number, argument);
	fputs(str, fl);
	fclose(fl);
	send_to_char("Ok. thanks.\n\r", ch);
}





void do_bug(struct char_data *ch, char *argument, int cmd)
{
	FILE *fl;
	char str[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
	{
		send_to_char("You are a monster! Bug off!\n\r", ch);
		return;
	}

	/* skip whites */
	for (; isspace(*argument); argument++);

	if (!*argument)
	{
		send_to_char("Pardon?\n\r",
			ch);
		return;
	}

	if (!(fl = fopen(BUG_FILE, "a")))
	{
		perror ("do_bug");
		send_to_char("Could not open the bug-file.\n\r", ch);
		return;
	}

	sprintf(str, "**%s[%d]: %s\n",
		GET_NAME(ch), world[ch->in_room].number, argument);
	fputs(str, fl);
	fclose(fl);
	send_to_char("Ok.\n\r", ch);
}



void do_brief(struct char_data *ch, char *argument, int cmd)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->specials.act, PLR_BRIEF))
	{
		send_to_char("Brief mode off.\n\r", ch);
		REMOVE_BIT(ch->specials.act, PLR_BRIEF);
	}
	else
	{
		send_to_char("Brief mode on.\n\r", ch);
		SET_BIT(ch->specials.act, PLR_BRIEF);
	}
}


void do_compact(struct char_data *ch, char *argument, int cmd)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->specials.act, PLR_COMPACT))
	{
		send_to_char("You are now in the uncompacted mode.\n\r", ch);
		REMOVE_BIT(ch->specials.act, PLR_COMPACT);
	}
	else
	{
		send_to_char("You are now in compact mode.\n\r", ch);
		SET_BIT(ch->specials.act, PLR_COMPACT);
	}
}


void do_group(struct char_data *ch, char *argument, int cmd)
{
	char name[256];
	struct char_data *victim, *k;
	struct follow_type *f;
	bool found;

	one_argument(argument, name);

	if (!*name) {
		if (!IS_AFFECTED(ch, AFF_GROUP)) {
			send_to_char("But you are a member of no group?!\n\r", ch);
		} else {
			send_to_char("Your group consists of:\n\r", ch);
			if (ch->master)
				k = ch->master;
			else
				k = ch;

			if (IS_AFFECTED(k, AFF_GROUP))
				act("     $N (Head of group)",FALSE,ch, 0, k, TO_CHAR);

			for(f=k->followers; f; f=f->next)
				if (IS_AFFECTED(f->follower, AFF_GROUP))
					act("     $N",FALSE,ch, 0, f->follower, TO_CHAR);
		}

		return;
	}

	if (!(victim = get_char_room_vis(ch, name))) {
		send_to_char("No one here by that name.\n\r", ch);
	} else {

		if (ch->master) {
			act("You can not enroll group members without being head of a group.",
			   FALSE, ch, 0, 0, TO_CHAR);
			return;
		}

		found = FALSE;

		if (victim == ch)
			found = TRUE;
		else {
			for(f=ch->followers; f; f=f->next) {
				if (f->follower == victim) {
					found = TRUE;
					break;
				}
			}
		}
		
		if (found) {
			if (IS_AFFECTED(victim, AFF_GROUP)) {
				act("$n has been kicked out of the group!", FALSE, victim, 0, ch, TO_ROOM);
				act("You are no longer a member of the group!", FALSE, victim, 0, 0, TO_CHAR);
				REMOVE_BIT(victim->specials.affected_by, AFF_GROUP);
			} else {
				act("$n is now a group member.", FALSE, victim, 0, 0, TO_ROOM);
				act("You are now a group member.", FALSE, victim, 0, 0, TO_CHAR);
				SET_BIT(victim->specials.affected_by, AFF_GROUP);
			}
		} else {
			act("$N must follow you, to enter the group", FALSE, ch, 0, victim, TO_CHAR);
		}
	}
}


void do_quaff(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct obj_data *temp;
  int i;
	bool equipped;

	equipped = FALSE;

  one_argument(argument,buf);

	if (!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))) {
		temp = ch->equipment[HOLD];
		equipped = TRUE;
	  if ((temp==0) || !isname(buf, temp->name)) {
			act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
  	  return;
  	}
	}

  if (temp->obj_flags.type_flag!=ITEM_POTION)
  {
    act("You can only quaff potions.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  act("$n quaffs $p.", TRUE, ch, temp, 0, TO_ROOM);
  act("You quaff $p which dissolves.",FALSE,ch,temp,0,TO_CHAR);

  for (i=1; i<4; i++)
    if (temp->obj_flags.value[i] >= 1)
      ((*spell_info[temp->obj_flags.value[i]].spell_pointer)
        ((byte) temp->obj_flags.value[0], ch, "", SPELL_TYPE_POTION, ch, 0));

	if (equipped)
		unequip_char(ch, HOLD);

	extract_obj(temp);
}


void do_recite(struct char_data *ch, char *argument, int cmd)
{
	char buf[100];
	struct obj_data *scroll, *obj;
	struct char_data *victim;
	int i, bits;
	bool equipped;

	equipped = FALSE;
	obj = 0;
	victim = 0;

	argument = one_argument(argument,buf);

	if (!(scroll = get_obj_in_list_vis(ch,buf,ch->carrying))) {
		scroll = ch->equipment[HOLD];
		equipped = TRUE;
	  if ((scroll==0) || !isname(buf, scroll->name)) {
			act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
			return;
  	}
	}

  if (scroll->obj_flags.type_flag!=ITEM_SCROLL)
  {
    act("Recite is normally used for scroll's.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

	if (*argument) {
	  bits = generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM |
        FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &victim, &obj);
		if (bits == 0) {
			send_to_char("No such thing around to recite the scroll on.\n\r", ch);
			return;
		}
	} else {
		victim = ch;
	}

	act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
  act("You recite $p which dissolves.",FALSE,ch,scroll,0,TO_CHAR);

  for (i=1; i<4; i++)
    if (scroll->obj_flags.value[i] >= 1)
      ((*spell_info[scroll->obj_flags.value[i]].spell_pointer)
      ((byte) scroll->obj_flags.value[0], ch, "", SPELL_TYPE_SCROLL, victim, obj));

	if (equipped)
		unequip_char(ch, HOLD);

	extract_obj(scroll);
}



void do_use(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
	struct char_data *tmp_char;
  struct obj_data *tmp_object, *stick;

  int bits;

  argument = one_argument(argument,buf);

  if (ch->equipment[HOLD] == 0 ||
      !isname(buf, ch->equipment[HOLD]->name)) {
    act("You do not hold that item in your hand.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

	stick = ch->equipment[HOLD];

  if (stick->obj_flags.type_flag == ITEM_STAFF)
  {
		act("$n taps $p three times on the ground.",TRUE, ch, stick, 0,TO_ROOM);
		act("You tap $p three times on the ground.",FALSE,ch, stick, 0,TO_CHAR);

		if (stick->obj_flags.value[2] > 0) {  /* Is there any charges left? */
			stick->obj_flags.value[2]--;
			((*spell_info[stick->obj_flags.value[3]].spell_pointer)
			((byte) stick->obj_flags.value[0], ch, "", SPELL_TYPE_STAFF, 0, 0));

		} else {
			send_to_char("The staff seems powerless.\n\r", ch);
		}
	} else if (stick->obj_flags.type_flag == ITEM_WAND) {

		bits = generic_find(argument, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
		                    FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
		if (bits) {
			if (bits == FIND_CHAR_ROOM) {
				act("$n point $p at $N.", TRUE, ch, stick, tmp_char, TO_ROOM);
				act("You point $p at $N.",FALSE,ch, stick, tmp_char, TO_CHAR);
			} else {
				act("$n point $p at $P.", TRUE, ch, stick, tmp_object, TO_ROOM);
				act("You point $p at $P.",FALSE,ch, stick, tmp_object, TO_CHAR);
			}

			if (stick->obj_flags.value[2] > 0) { /* Is there any charges left? */
				stick->obj_flags.value[2]--;
				((*spell_info[stick->obj_flags.value[3]].spell_pointer)
			  ((byte) stick->obj_flags.value[0], ch, "", SPELL_TYPE_WAND, tmp_char, tmp_object));
			} else {
				send_to_char("The wand seems powerless.\n\r", ch);
			}
		} else {
			send_to_char("What should the wand be pointed at?\n\r", ch);
		}
	} else {
		send_to_char("Use is normally only for wand's and staff's.\n\r", ch);
  }
}
