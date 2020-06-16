/* ************************************************************************
*  file: act.offensive.c , Implementation of commands.    Part of DIKUMUD *
*  Usage : Offensive commands.                                            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"

/* extern variables */

extern struct descriptor_data *descriptor_list;
extern struct room_data *world;


void raw_kill(struct char_data *ch);
void stop_follower(struct char_data *ch);

void do_hit(struct char_data *ch, char *argument, int cmd)
{
	char arg[MAX_STRING_LENGTH];
	char buffer[MAX_STRING_LENGTH];
	struct char_data *victim;

	one_argument(argument, arg);

	if (*arg) {
		victim = get_char_room_vis(ch, arg);
		if (victim) {
			if (victim == ch) {
				send_to_char("You hit yourself..OUCH!.\n\r", ch);
				act("$n hits $mself, and says OUCH!", FALSE, ch, 0, victim, TO_ROOM);
			} else {
				if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
					act("$N is just such a good friend, you simply can't hit $M.",
					  FALSE, ch,0,victim,TO_CHAR);
					return;
				}
				if ((GET_POS(ch)==POSITION_STANDING) &&
				    (victim != ch->specials.fighting)) {
					hit(ch, victim, TYPE_UNDEFINED);
					WAIT_STATE(ch, PULSE_VIOLENCE+2); /* HVORFOR DET?? */
				} else {
					send_to_char("You do the best you can!\n\r",ch);
				}
			}
		} else {
			send_to_char("They aren't here.\n\r", ch);
		}
	} else {
		send_to_char("Hit who?\n\r", ch);
	}
}



void do_kill(struct char_data *ch, char *argument, int cmd)
{
	static char arg[MAX_STRING_LENGTH];
	char buf[70];
	struct char_data *victim;

	if ((GET_LEVEL(ch) < 24) || IS_NPC(ch)) {
		do_hit(ch, argument, 0);
    return;
	}

	one_argument(argument, arg);

	if (!*arg)
	{
		send_to_char("Kill who?\n\r", ch);
	}
	else
	{
		if (!(victim = get_char_room_vis(ch, arg)))
   		send_to_char("They aren't here.\n\r", ch);
		else
   		if (ch == victim)
      	send_to_char("Your mother would be so sad.. :(\n\r", ch);
   		else {
				act("You chop $M to pieces! Ah! The blood!", FALSE, ch, 0, victim, TO_CHAR);
				act("$N chops you to pieces!", FALSE, victim, 0, ch, TO_CHAR);
				act("$n brutally slays $N", FALSE, ch, 0, victim, TO_NOTVICT);
				raw_kill(victim);
			}
	}
}



void do_backstab(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim;
	char name[256];
	byte percent;

	one_argument(argument, name);

	if (!(victim = get_char_room_vis(ch, name))) {
		send_to_char("Backstab who?\n\r", ch);
		return;
	}

	if (victim == ch) {
		send_to_char("How can you sneak up on yourself?\n\r", ch);
		return;
	}

	if (!ch->equipment[WIELD]) {
		send_to_char("You need to wield a weapon, to make it a succes.\n\r",ch);
		return;
	}

	if (ch->equipment[WIELD]->obj_flags.value[3] != 11) {
		send_to_char("Only piercing weapons can be used for backstabbing.\n\r",ch);
		return;
	}

	if (victim->specials.fighting) {
		send_to_char("You can't backstab a fighting person, too alert!\n\r", ch);
		return;
	}

	percent=number(1,101); /* 101% is a complete failure */

	if (AWAKE(victim) && (percent > ch->skills[SKILL_BACKSTAB].learned))
		damage(ch, victim, 0, SKILL_BACKSTAB);
	else
		hit(ch,victim,SKILL_BACKSTAB);
}



void do_order(struct char_data *ch, char *argument, int cmd)
{
  char name[100], message[256];
	char buf[256+24];
	bool found = FALSE;
	int org_room;
	struct char_data *victim;
	struct follow_type *k;

  half_chop(argument, name, message);

	if (!*name || !*message)
		send_to_char("Order who to do what?\n\r", ch);
	else if (!(victim = get_char_room_vis(ch, name)) &&
	         strcasecmp("follower", name) && strcasecmp("followers", name))
			send_to_char("That person isn't here.\n\r", ch);
	else if (ch == victim)
		send_to_char("You obviously suffer from skitzofrenia.\n\r", ch);

	else {
		if (IS_AFFECTED(ch, AFF_CHARM)) {
			send_to_char("Your superior would not aprove of you giving orders.\n\r",ch);
			return;
		}

		if (victim) {
			sprintf(buf, "$N orders you to '%s'", message);
			act(buf, FALSE, victim, 0, ch, TO_CHAR);
			act("$n gives $N an order.", FALSE, ch, 0, victim, TO_ROOM);

			if ( (victim->master!=ch) || !IS_AFFECTED(victim, AFF_CHARM) )
				act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
			else {
				if (saves_spell(victim,SAVING_SPELL)) {
					if (saves_spell(victim,SAVING_SPELL)&&
						 saves_spell(victim,SAVING_SPELL)&&
						 saves_spell(victim,SAVING_SPELL)) {
						/* make it easyer to use charmn */
						stop_follower(victim);
						if (IS_NPC(victim))
							hit(victim,ch);
					} else {
						act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
					}
				}
				send_to_char("Ok.\n\r", ch);
				command_interpreter(victim, message);
			}
		} else {  /* This is order "followers" */
			sprintf(buf, "$n issues the order '%s'.", message);
			act(buf, FALSE, ch, 0, victim, TO_ROOM);

			org_room = ch->in_room;

			for (k = ch->followers; k; k = k->next) {
				if (org_room == k->follower->in_room)
					if (IS_AFFECTED(k->follower, AFF_CHARM)) {
						found = TRUE;
						if (saves_spell(victim,SAVING_SPELL)) {
							if (saves_spell(victim,SAVING_SPELL)&&
							    saves_spell(victim,SAVING_SPELL)&&
							    saves_spell(victim,SAVING_SPELL)) {
								/* make it easyer to use charmn */
								stop_follower(victim);
								if (IS_NPC(victim))
								hit(victim,ch);
								} else {
									act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
								}
						}
						command_interpreter(k->follower, message);
					}
			}
			if (found)
				send_to_char("Ok.\n\r", ch);
			else
				send_to_char("Nobody here are loyal subjects of yours!\n\r", ch);
		}
	}
}



void do_flee(struct char_data *ch, char *argument, int cmd)
{
	int i, attempt, loose, die;

	void gain_exp(struct char_data *ch, int gain);
	int special(struct char_data *ch, int cmd, char *arg);

	if (!(ch->specials.fighting)) {
		for(i=0; i<6; i++) {
			attempt = number(0, 5);  /* Select a random direction */
			if (CAN_GO(ch, attempt) &&
			!IS_SET(world[EXIT(ch, attempt)->to_room].room_flags, DEATH)) {
				act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);
				if ((die = do_simple_move(ch, attempt, FALSE))== 1) {
					/* The escape has succeded */
					send_to_char("You flee head over heels.\n\r", ch);
					return;
				} else {
					if (!die) act("$n tries to flee, but is too exhausted!", TRUE, ch, 0, 0, TO_ROOM);
					return;
				}
			}
		} /* for */
		/* No exits was found */
		send_to_char("PANIC! You couldn't escape!\n\r", ch);
		return;
	}

	for(i=0; i<6; i++) {
		attempt = number(0, 5);  /* Select a random direction */
		if (CAN_GO(ch, attempt) &&
		   !IS_SET(world[EXIT(ch, attempt)->to_room].room_flags, DEATH)) {
			act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);
			if ((die = do_simple_move(ch, attempt, FALSE))== 1) { 
              /* The escape has succeded */
				loose = GET_MAX_HIT(ch->specials.fighting)-GET_HIT(ch->specials.fighting);
				loose *= GET_LEVEL(ch->specials.fighting);

				if (!IS_NPC(ch))
					gain_exp(ch, -loose);

				send_to_char("You flee head over heels.\n\r", ch);

				/* Insert later when using huntig system        */
				/* ch->specials.fighting->specials.hunting = ch */

				if (ch->specials.fighting->specials.fighting == ch)
					stop_fighting(ch->specials.fighting);
				stop_fighting(ch);
				return;
			} else {
				if (!die) act("$n tries to flee, but is too exhausted!", TRUE, ch, 0, 0, TO_ROOM);
				return;
			}
		}
	} /* for */

	/* No exits was found */
	send_to_char("PANIC! You couldn't escape!\n\r", ch);
}



void do_bash(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim;
	char name[256], buf[256];
	byte percent;

	one_argument(argument, name);

	if (GET_CLASS(ch) != CLASS_WARRIOR) {
		send_to_char("You better leave all the martial arts to fighters.\n\r", ch);
		return;
	}

	if (!(victim = get_char_room_vis(ch, name))) {
		if (ch->specials.fighting) {
			victim = ch->specials.fighting;
		} else {
			send_to_char("Bash who?\n\r", ch);
			return;
		}
	}


	if (victim == ch) {
		send_to_char("Aren't we funny today...\n\r", ch);
		return;
	}

	if (!ch->equipment[WIELD]) {
		send_to_char("You need to wield a weapon, to make it a success.\n\r",ch);
		return;
	}

	percent=number(1,101); /* 101% is a complete failure */

	if (percent > ch->skills[SKILL_BASH].learned) {
		damage(ch, victim, 0, SKILL_BASH);
		GET_POS(ch) = POSITION_SITTING;
	} else {
		damage(ch, victim, 1, SKILL_BASH);
		GET_POS(victim) = POSITION_SITTING;
		WAIT_STATE(victim, PULSE_VIOLENCE*2);
	}
	WAIT_STATE(ch, PULSE_VIOLENCE*2);
}



void do_rescue(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim, *tmp_ch;
	int percent;
	char victim_name[240];
	char buf[240];

	one_argument(argument, victim_name);

	if (!(victim = get_char_room_vis(ch, victim_name))) {
		send_to_char("Who do you want to rescue?\n\r", ch);
		return;
	}

	if (victim == ch) {
		send_to_char("What about fleeing instead?\n\r", ch);
		return;
	}

	if (ch->specials.fighting == victim) {
		send_to_char("How can you rescue someone you are trying to kill?\n\r",ch);
		return;
	}

	for (tmp_ch=world[ch->in_room].people; tmp_ch &&
	    (tmp_ch->specials.fighting != victim); tmp_ch=tmp_ch->next_in_room)  ;

	if (!tmp_ch) {
		act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);
		return;
	}


	if (GET_CLASS(ch) != CLASS_WARRIOR)
		send_to_char("But only true warriors can do this!", ch);
	else {
		percent=number(1,101); /* 101% is a complete failure */
		if (percent > ch->skills[SKILL_RESCUE].learned) {
			send_to_char("You fail the rescue.\n\r", ch);
			return;
		}

		send_to_char("Banzai! To the rescue...\n\r", ch);
		act("You are rescued by $N, you are confused!", FALSE, victim, 0, ch, TO_CHAR);
		act("$n heroically rescues $N.", FALSE, ch, 0, victim, TO_NOTVICT);

		if (victim->specials.fighting == tmp_ch)
			stop_fighting(victim);
		if (tmp_ch->specials.fighting)
			stop_fighting(tmp_ch);
		if (ch->specials.fighting)
			stop_fighting(ch);

		set_fighting(ch, tmp_ch);
		set_fighting(tmp_ch, ch);

		WAIT_STATE(victim, 2*PULSE_VIOLENCE);
	}

}



void do_kick(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim;
	char name[256], buf[256];
	byte percent;


	if (GET_CLASS(ch) != CLASS_WARRIOR) {
		send_to_char("You better leave all the martial arts to fighters.\n\r", ch);
		return;
	}

	one_argument(argument, name);

	if (!(victim = get_char_room_vis(ch, name))) {
		if (ch->specials.fighting) {
			victim = ch->specials.fighting;
		} else {
			send_to_char("Kick who?\n\r", ch);
			return;
		}
	}

	if (victim == ch) {
		send_to_char("Aren't we funny today...\n\r", ch);
		return;
	}

	percent=((10-(GET_AC(victim)/10))<<1) + number(1,101); /* 101% is a complete failure */

	if (percent > ch->skills[SKILL_KICK].learned) {
		damage(ch, victim, 0, SKILL_KICK);
	} else {
		damage(ch, victim, GET_LEVEL(ch)>>1, SKILL_KICK);
	}
	WAIT_STATE(ch, PULSE_VIOLENCE*3);
}

