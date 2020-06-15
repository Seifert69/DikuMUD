/* ************************************************************************
*  File: fight.c , Combat module.                         Part of DIKUMUD *
*  Usage: Combat system and messages.                                     *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "spells.h"

/* Structures */

struct char_data *combat_list = 0;	    /* head of l-list of fighting chars	*/
struct char_data *combat_next_dude = 0; /* Next dude global trick           */


/* External structures */

extern struct room_data *world;
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data  *object_list;

/* External procedures */
extern void slog(char *msg);

char *fread_string(FILE *f1);
void stop_follower(struct char_data *ch);
void do_flee(struct char_data *ch, char *argument, int cmd);
void hit(struct char_data *ch, struct char_data *victim, int type);


/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
  {"hit",   "hits"},             /* TYPE_HIT      */
  {"pound", "pounds"},           /* TYPE_BLUDGEON */
  {"pierce", "pierces"},         /* TYPE_PIERCE   */
  {"slash", "slashes"},          /* TYPE_SLASH    */
  {"whip", "whips"},             /* TYPE_WHIP     */
  {"claw", "claws"},             /* TYPE_CLAW     */
  {"bite", "bites"},             /* TYPE_BITE     */
  {"sting", "stings"},           /* TYPE_STING    */
  {"crush", "crushes"}           /* TYPE_CRUSH    */
};




/* The Fight related routines */


void appear(struct char_data *ch)
{
  act("$n slowly fade into existence.", FALSE, ch,0,0,TO_ROOM);

  if (affected_by_spell(ch, SPELL_INVISIBLE))
    affect_from_char(ch, SPELL_INVISIBLE);

  REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
}



void load_messages(void)
{
	FILE *f1;
	int i,type;
	struct message_type *messages;
	char chk[100];

	if (!(f1 = fopen(MESS_FILE, "r"))){
		perror("read messages");
		exit(0);
	}

	for (i = 0; i < MAX_MESSAGES; i++)
	{ 
		fight_messages[i].a_type = 0;
		fight_messages[i].number_of_attacks=0;
		fight_messages[i].msg = 0;
	}

	fscanf(f1, " %s \n", chk);

	while(*chk == 'M')
	{
		fscanf(f1," %d\n", &type);
		for (i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type!=type) &&
			(fight_messages[i].a_type); i++);
		if(i>=MAX_MESSAGES){
			slog("Too many combat messages.");
			exit(0);
		}

		CREATE(messages,struct message_type,1);
		fight_messages[i].number_of_attacks++;
		fight_messages[i].a_type=type;
		messages->next=fight_messages[i].msg;
		fight_messages[i].msg=messages;

		messages->die_msg.attacker_msg      = fread_string(f1);
		messages->die_msg.victim_msg        = fread_string(f1);
		messages->die_msg.room_msg          = fread_string(f1);
		messages->miss_msg.attacker_msg     = fread_string(f1);
		messages->miss_msg.victim_msg       = fread_string(f1);
		messages->miss_msg.room_msg         = fread_string(f1);
		messages->hit_msg.attacker_msg      = fread_string(f1);
		messages->hit_msg.victim_msg        = fread_string(f1);
		messages->hit_msg.room_msg          = fread_string(f1);
		messages->god_msg.attacker_msg      = fread_string(f1);
		messages->god_msg.victim_msg        = fread_string(f1);
		messages->god_msg.room_msg          = fread_string(f1);
		fscanf(f1, " %s \n", chk);
	}

	fclose(f1);
}


void update_pos( struct char_data *victim )
{

	if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POSITION_STUNNED)) return;
	else if (GET_HIT(victim) > 0 ) GET_POS(victim) = POSITION_STANDING;
	else if (GET_HIT(victim) <= -11) GET_POS(victim) = POSITION_DEAD;
	else if (GET_HIT(victim) <= -6) GET_POS(victim) = POSITION_MORTALLYW;
	else if (GET_HIT(victim) <= -3) GET_POS(victim) = POSITION_INCAP;
	else GET_POS(victim) = POSITION_STUNNED;

}


/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data *ch, struct char_data *vict)
{
	assert(!ch->specials.fighting);

	ch->next_fighting = combat_list;
	combat_list = ch;

	if(IS_AFFECTED(ch,AFF_SLEEP))
		affect_from_char(ch,SPELL_SLEEP);

	ch->specials.fighting = vict;
	GET_POS(ch) = POSITION_FIGHTING;
}



/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data *ch)
{
	struct char_data *tmp;

	assert(ch->specials.fighting);

	if (ch == combat_next_dude)
		combat_next_dude = ch->next_fighting;

	if (combat_list == ch)
	   combat_list = ch->next_fighting;
	else
	{
		for (tmp = combat_list; tmp && (tmp->next_fighting != ch); 
			tmp = tmp->next_fighting);
		if (!tmp) {
			slog("Char fighting not found Error (fight.c, stop_fighting)");
			abort();
		}
		tmp->next_fighting = ch->next_fighting;
	}

	ch->next_fighting = 0;
	ch->specials.fighting = 0;
	GET_POS(ch) = POSITION_STANDING;
	update_pos(ch);
}



#define MAX_NPC_CORPSE_TIME 5
#define MAX_PC_CORPSE_TIME 10

void make_corpse(struct char_data *ch)
{
	struct obj_data *corpse, *o;
	struct obj_data *money;	
	char buf[MAX_STRING_LENGTH];
	int i;

	struct obj_data *create_money( int amount );

	CREATE(corpse, struct obj_data, 1);
	clear_object(corpse);

	
	corpse->item_number = NOWHERE;
	corpse->in_room = NOWHERE;
	corpse->name = strdup("corpse");

	sprintf(buf, "Corpse of %s is lying here.", 
	  (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
	corpse->description = strdup(buf);

	sprintf(buf, "Corpse of %s",
	  (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
	corpse->short_description = strdup(buf);

	corpse->contains = ch->carrying;
	if ( (GET_GOLD(ch)>0) &&
        ( IS_NPC(ch) || (ch->desc) ) )
	{
		money = create_money(GET_GOLD(ch));
		GET_GOLD(ch)=0;
		obj_to_obj(money,corpse);
	}

	corpse->obj_flags.type_flag = ITEM_CONTAINER;
	corpse->obj_flags.wear_flags = ITEM_TAKE;
	corpse->obj_flags.value[0] = 0; /* You can't store stuff in a corpse */
	corpse->obj_flags.value[3] = 1; /* corpse identifyer */
	corpse->obj_flags.weight = GET_WEIGHT(ch)+IS_CARRYING_W(ch);
	corpse->obj_flags.cost_per_day = 100000;
	if (IS_NPC(ch))
		corpse->obj_flags.timer = MAX_NPC_CORPSE_TIME;
	else
		corpse->obj_flags.timer = MAX_PC_CORPSE_TIME;

	for (i=0; i<MAX_WEAR; i++)
		if (ch->equipment[i])
			obj_to_obj(unequip_char(ch, i), corpse);

	ch->carrying = 0;
	IS_CARRYING_N(ch) = 0;
	IS_CARRYING_W(ch) = 0;

	corpse->next = object_list;
	object_list = corpse;

	for(o = corpse->contains; o; o->in_obj = corpse, o = o->next_content);
	object_list_new_owner(corpse, 0);

	obj_to_room(corpse, ch->in_room);
}


/* When ch kills victim */
void change_alignment(struct char_data *ch, struct char_data *victim)
{
	int align;

	if ((align = GET_ALIGNMENT(ch)-GET_ALIGNMENT(victim)) > 0) {
		if (align > 650)
			GET_ALIGNMENT(ch) = MIN(1000,GET_ALIGNMENT(ch) + ((align-650) / 4));
		else
			GET_ALIGNMENT(ch) /= 2;
	} else {
		if (align < -650)
			GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) + ((align+650) / 4));
		else
			GET_ALIGNMENT(ch) /= 2;
	}
}



void death_cry(struct char_data *ch)
{
	struct char_data *victim;
	int door, was_in;

	act("Your blood freezes as you hear $ns death cry.", FALSE, ch,0,0,TO_ROOM);
	was_in = ch->in_room;

	for (door = 0; door <= 5; door++) {
		if (CAN_GO(ch, door))	{
			ch->in_room = world[was_in].dir_option[door]->to_room;
			act("Your blood freezes as you hear someones death cry.",FALSE,ch,0,0,TO_ROOM);
			ch->in_room = was_in;
		}
	}
}



void raw_kill(struct char_data *ch)
{
	if (ch->specials.fighting)
		stop_fighting(ch);

	death_cry(ch);

	make_corpse(ch);
	affect_total(ch); 
	extract_char(ch);
}



void die(struct char_data *ch)
{
	gain_exp(ch, -(GET_EXP(ch)/2));
	raw_kill(ch);
}



void group_gain(struct char_data *ch, struct char_data *victim)
{
	char buf[256];
	int no_members, share;
	struct char_data *k;
	struct follow_type *f;

	if (!(k=ch->master))
		k = ch;


	if (IS_AFFECTED(k, AFF_GROUP) &&
	   (k->in_room == ch->in_room))
		no_members = 1;
	else
		no_members = 0;

	for (f=k->followers; f; f=f->next)
		if (IS_AFFECTED(f->follower, AFF_GROUP) &&
		   (f->follower->in_room == ch->in_room))
			no_members++;

	if (no_members >= 1)
		share = MIN(450000/no_members, (GET_EXP(victim)/3)/no_members);
	else
		share = 0;

	if (IS_AFFECTED(k, AFF_GROUP) &&
	   (k->in_room == ch->in_room)) {
		act("You receive your share of experience.", FALSE, k, 0, 0, TO_CHAR);
		gain_exp(k, share);
		change_alignment(k, victim);
	}

	for (f=k->followers; f; f=f->next) {
		if (IS_AFFECTED(f->follower, AFF_GROUP) &&
		   (f->follower->in_room == ch->in_room)) {
			act("You receive your share of experience.", FALSE, f->follower,0,0,TO_CHAR);
			gain_exp(f->follower, share);
			change_alignment(f->follower, victim);
		}
	}
}

char *replace_string(char *str, char *weapon)
{
	static char buf[256];
	char *cp;

	cp = buf;

	for (; *str; str++) {
		if (*str == '#') {
			switch(*(++str)) {
				case 'W' : 
					for (; *weapon; *(cp++) = *(weapon++));
					break;
				default :
					*(cp++) = '#';
					break;
			}
		} else {
			*(cp++) = *str;
		}

		*cp = 0;
	} /* For */

	return(buf);
}



void dam_message(int dam, struct char_data *ch, struct char_data *victim,
                 int w_type)
{
	struct obj_data *wield;
	char *buf;

	static struct dam_weapon_type {
		char *to_room;
		char *to_char;
		char *to_victim;
	} dam_weapons[] = {

	 {"$n misses $N with $s #W.",                           /*    0    */
	  "You miss $N with your #W.",
	  "$n miss you with $s #W." },

   {"$n tickles $N with $s #W.",                          /*  1.. 2  */
    "You tickle $N as you #W $M.",
    "$n tickle you as $e #W you." },

   {"$n barely #W $N.",                                   /*  3.. 4  */
    "You barely #W $N.",
    "$n barely #W you."},

	 {"$n #W $N.",                                          /*  5.. 6  */
    "You #W $N.",
    "$n #W you."}, 

	 {"$n #W $N hard.",                                     /*  7..10  */
	  "You #W $N hard.",
    "$n #W you hard."},

	 {"$n #W $N very hard.",                                /* 11..14  */
	  "You #W $N very hard.",
	  "$n #W you very hard."},

	 {"$n #W $N extremely hard.",                          /* 15..20  */
	  "You #W $N extremely hard.",
	  "$n #W you extremely hard."},

	 {"$n massacre $N to small fragments with $s #W.",     /* > 20    */
	  "You massacre $N to small fragments with your #W.",
	  "$n massacre you to small fragments with $s #W."}
	};

	w_type -= TYPE_HIT;   /* Change to base of table with text */

	wield = ch->equipment[WIELD];

	if (dam == 0) {
		buf = replace_string(dam_weapons[0].to_room, attack_hit_text[w_type].singular);
		act(buf, FALSE, ch, wield, victim, TO_NOTVICT);
		buf = replace_string(dam_weapons[0].to_char, attack_hit_text[w_type].singular);
		act(buf, FALSE, ch, wield, victim, TO_CHAR);
		buf = replace_string(dam_weapons[0].to_victim, attack_hit_text[w_type].singular);
		act(buf, FALSE, ch, wield, victim, TO_VICT);
	} else if (dam <= 2) {
		buf = replace_string(dam_weapons[1].to_room, attack_hit_text[w_type].singular);
		act(buf, FALSE, ch, wield, victim, TO_NOTVICT);
		buf = replace_string(dam_weapons[1].to_char, attack_hit_text[w_type].singular);
		act(buf, FALSE, ch, wield, victim, TO_CHAR);
		buf = replace_string(dam_weapons[1].to_victim, attack_hit_text[w_type].plural);
		act(buf, FALSE, ch, wield, victim, TO_VICT);
	} else if (dam <= 4) {
		buf = replace_string(dam_weapons[2].to_room, attack_hit_text[w_type].singular);
		act(buf, FALSE, ch, wield, victim, TO_NOTVICT);
		buf = replace_string(dam_weapons[2].to_char, attack_hit_text[w_type].singular);
		act(buf, FALSE, ch, wield, victim, TO_CHAR);
		buf = replace_string(dam_weapons[2].to_victim, attack_hit_text[w_type].plural);
		act(buf, FALSE, ch, wield, victim, TO_VICT);
	} else if (dam <= 6) {
		buf = replace_string(dam_weapons[3].to_room, attack_hit_text[w_type].plural);
		act(buf, FALSE, ch, wield, victim, TO_NOTVICT);
		buf = replace_string(dam_weapons[3].to_char, attack_hit_text[w_type].singular);
		act(buf, FALSE, ch, wield, victim, TO_CHAR);
		buf = replace_string(dam_weapons[3].to_victim, attack_hit_text[w_type].plural);
		act(buf, FALSE, ch, wield, victim, TO_VICT);
	} else if (dam <= 10) {
		buf = replace_string(dam_weapons[4].to_room, attack_hit_text[w_type].plural);
		act(buf, FALSE, ch, wield, victim, TO_NOTVICT);
		buf = replace_string(dam_weapons[4].to_char, attack_hit_text[w_type].singular);
		act(buf, FALSE, ch, wield, victim, TO_CHAR);
		buf = replace_string(dam_weapons[4].to_victim, attack_hit_text[w_type].singular);
		act(buf, FALSE, ch, wield, victim, TO_VICT);
	} else if (dam <= 15) {
		buf = replace_string(dam_weapons[5].to_room, attack_hit_text[w_type].plural);
		act(buf, FALSE, ch, wield, victim, TO_NOTVICT);
		buf = replace_string(dam_weapons[5].to_char, attack_hit_text[w_type].singular);
		act(buf, FALSE, ch, wield, victim, TO_CHAR);
		buf = replace_string(dam_weapons[5].to_victim, attack_hit_text[w_type].plural);
		act(buf, FALSE, ch, wield, victim, TO_VICT);
	} else if (dam <= 20) {
		buf = replace_string(dam_weapons[6].to_room, attack_hit_text[w_type].singular);
		act(buf, FALSE, ch, wield, victim, TO_NOTVICT);
		buf = replace_string(dam_weapons[6].to_char, attack_hit_text[w_type].singular);
		act(buf, FALSE, ch, wield, victim, TO_CHAR);
		buf = replace_string(dam_weapons[6].to_victim, attack_hit_text[w_type].singular);
		act(buf, FALSE, ch, wield, victim, TO_VICT);
	} else {
		buf = replace_string(dam_weapons[7].to_room, attack_hit_text[w_type].singular);
		act(buf, FALSE, ch, wield, victim, TO_NOTVICT);
		buf = replace_string(dam_weapons[7].to_char, attack_hit_text[w_type].singular);
		act(buf, FALSE, ch, wield, victim, TO_CHAR);
		buf = replace_string(dam_weapons[7].to_victim, attack_hit_text[w_type].singular);
		act(buf, FALSE, ch, wield, victim, TO_VICT);
	}
}



void damage(struct char_data *ch, struct char_data *victim,
            int dam, int attacktype)
{
	char buf[MAX_STRING_LENGTH];
	struct message_type *messages;
	int i,j,nr,max_hit,exp;

	int hit_limit(struct char_data *ch);

	assert(GET_POS(victim) > POSITION_DEAD);

	if ((GET_LEVEL(victim)>20) && !IS_NPC(victim)) /* You can't damage an immortal! */
		dam=0;
		
	if (victim != ch) {
		if (GET_POS(victim) > POSITION_STUNNED) {
			if (!(victim->specials.fighting))
				set_fighting(victim, ch);
			GET_POS(victim) = POSITION_FIGHTING;
		}

		if (GET_POS(ch) > POSITION_STUNNED) {
			if (!(ch->specials.fighting))
				set_fighting(ch, victim);

			if (IS_NPC(ch) && IS_NPC(victim) &&
          victim->master &&
			    !number(0,10) && IS_AFFECTED(victim, AFF_CHARM) &&
			    (victim->master->in_room == ch->in_room)) {
				if (ch->specials.fighting)
					stop_fighting(ch);
				hit(ch, victim->master, TYPE_UNDEFINED);
				return;
			}
		}
	}

	if (victim->master == ch)
		stop_follower(victim);
			
	if (IS_AFFECTED(ch, AFF_INVISIBLE))
		appear(ch);

	if (IS_AFFECTED(victim, AFF_SANCTUARY))
		dam = MIN(dam, 18);  /* Max 18 damage when sanctuary */

	dam=MIN(dam,100);

	dam=MAX(dam,0);

	GET_HIT(victim)-=dam;

	if (ch != victim)
		gain_exp(ch,GET_LEVEL(victim)*dam);

	update_pos(victim);


	if ((attacktype >= TYPE_HIT) && (attacktype <= TYPE_SLASH)) {
		if (!ch->equipment[WIELD]) {
			dam_message(dam, ch, victim, TYPE_HIT);
		} else {
			dam_message(dam, ch, victim, attacktype);
		}
	} else {

	for(i = 0; i < MAX_MESSAGES; i++) {
		if (fight_messages[i].a_type == attacktype) {
			nr=dice(1,fight_messages[i].number_of_attacks);
			for(j=1,messages=fight_messages[i].msg;(j<nr)&&(messages);j++)
				messages=messages->next;

			if (!IS_NPC(victim) && (GET_LEVEL(victim) > 20)) {
				act(messages->god_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
				act(messages->god_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
				act(messages->god_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
			} else if (dam != 0) {
				if (GET_POS(victim) == POSITION_DEAD) {
					act(messages->die_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
					act(messages->die_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
					act(messages->die_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
				} else {
					act(messages->hit_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
					act(messages->hit_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
					act(messages->hit_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
				}
			} else { /* Dam == 0 */
				act(messages->miss_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
				act(messages->miss_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
				act(messages->miss_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
			}
		}
	}
	}
	switch (GET_POS(victim)) {
		case POSITION_MORTALLYW:
			act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
			act("You are mortally wounded, and will die soon, if not aided.", FALSE, victim, 0, 0, TO_CHAR);
			break;
		case POSITION_INCAP:
			act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
			act("You are incapacitated an will slowly die, if not aided.", FALSE, victim, 0, 0, TO_CHAR);
			break;
		case POSITION_STUNNED:
			act("$n is stunned, but will probably regain conscience again.", TRUE, victim, 0, 0, TO_ROOM);
			act("You're stunned, but will probably regain conscience again.", FALSE, victim, 0, 0, TO_CHAR);
			break;
		case POSITION_DEAD:
			act("$n is dead! R.I.P.", TRUE, victim, 0, 0, TO_ROOM);
			act("You are dead!  Sorry...", FALSE, victim, 0, 0, TO_CHAR);
			break;

		default:  /* >= POSITION SLEEPING */

			max_hit=hit_limit(victim);

			if (dam > (max_hit/5))
				act("That Really did HURT!",FALSE, victim, 0, 0, TO_CHAR);

			if (GET_HIT(victim) < (max_hit/5)) {

				act("You wish that your wounds would stop BLEEDING that much!",FALSE,victim,0,0,TO_CHAR);
				if (IS_NPC(victim))
					if (IS_SET(victim->specials.act, ACT_WIMPY))
						do_flee(victim, "", 0);
			}
			break;		
	}

	if (!IS_NPC(victim) && !(victim->desc)) {
		do_flee(victim, "", 0);
		if (!victim->specials.fighting) {
			act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
			victim->specials.was_in_room = victim->in_room;
			char_from_room(victim);
			char_to_room(victim, 0);
		}
	}

	if (GET_POS(victim) < POSITION_STUNNED)
		if (ch->specials.fighting == victim)
			stop_fighting(ch);

	if (!AWAKE(victim))
		if (victim->specials.fighting)
			stop_fighting(victim);

	if (GET_POS(victim) == POSITION_DEAD) {
		if (IS_NPC(victim) || victim->desc)
			if (IS_AFFECTED(ch, AFF_GROUP)) {
					group_gain(ch, victim);
			} else {
				/* Calculate level-difference bonus */
				exp = GET_EXP(victim)/3;
				if (IS_NPC(ch))
					exp += (exp*MIN(4, (GET_LEVEL(victim) - GET_LEVEL(ch))))>>3;
				else
					exp += (exp*MIN(8, (GET_LEVEL(victim) - GET_LEVEL(ch))))>>3;
				exp = MAX(exp, 1);
				gain_exp(ch, exp);
				change_alignment(ch, victim);
			}
		if (!IS_NPC(victim)) {
			sprintf(buf, "%s killed by %s at %s",
				GET_NAME(victim),
				(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
				world[victim->in_room].name);
			slog(buf);
		}
		die(victim);
	}
}



void hit(struct char_data *ch, struct char_data *victim, int type)
{

	struct obj_data *wielded = 0;
	struct obj_data *held = 0;
	int w_type;
	int victim_ac, calc_thaco;
	int dam;
	byte diceroll;

	extern int thaco[4][25];
	extern byte backstab_mult[];
	extern struct str_app_type str_app[];
	extern struct dex_app_type dex_app[];

	if (ch->in_room != victim->in_room) {
		slog("NOT SAME ROOM WHEN FIGHTING!");
		return;
	}

	if (ch->equipment[HOLD])
		held = ch->equipment[HOLD];

	if (ch->equipment[WIELD] &&
	   (ch->equipment[WIELD]->obj_flags.type_flag == ITEM_WEAPON)) {
		wielded = ch->equipment[WIELD];
		switch (wielded->obj_flags.value[3]) {
			case 0  :
			case 1  :
			case 2  : w_type = TYPE_WHIP; break;
			case 3  : w_type = TYPE_SLASH; break;
			case 4  :
			case 5  :
			case 6  : w_type = TYPE_CRUSH; break;
			case 7  : w_type = TYPE_BLUDGEON; break;
			case 8  :
			case 9  :
			case 10 :
			case 11 : w_type = TYPE_PIERCE; break;

			default : w_type = TYPE_HIT; break;
		}
	}	else {
		if (IS_NPC(ch) && (ch->specials.attack_type >= TYPE_HIT))
			w_type = ch->specials.attack_type;
		else
			w_type = TYPE_HIT;
	}

	/* Calculate the raw armor including magic armor */
	/* The lower AC, the better                      */

	if (!IS_NPC(ch))
		calc_thaco  = thaco[GET_CLASS(ch)-1][GET_LEVEL(ch)];
	else
		/* THAC0 for monsters is set in the HitRoll */
		calc_thaco = 20;

	calc_thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
	calc_thaco -= GET_HITROLL(ch);

	diceroll = number(1,20);

	victim_ac  = GET_AC(victim)/10;

	if (AWAKE(victim))
		victim_ac += dex_app[GET_DEX(victim)].defensive;

	victim_ac = MAX(-10, victim_ac);  /* -10 is lowest */

	if ((diceroll < 20) && AWAKE(victim) &&
       ((diceroll==1) || ((calc_thaco-diceroll) > victim_ac))) {
		if (type == SKILL_BACKSTAB)
			damage(ch, victim, 0, SKILL_BACKSTAB);
		else
			damage(ch, victim, 0, w_type);
	} else {

		dam  = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
		dam += GET_DAMROLL(ch);

		if (!wielded) {
			if (IS_NPC(ch))
				dam += dice(ch->specials.damnodice, ch->specials.damsizedice);
			else
				dam += number(0,2);  /* Max. 2 dam with bare hands */
		} else {
			dam += dice(wielded->obj_flags.value[1], wielded->obj_flags.value[2]);
		}

		if (GET_POS(victim) < POSITION_FIGHTING)
			dam *= 1+(POSITION_FIGHTING-GET_POS(victim))/3;
		/* Position  sitting  x 1.33 */
		/* Position  resting  x 1.66 */
		/* Position  sleeping x 2.00 */
		/* Position  stunned  x 2.33 */
		/* Position  incap    x 2.66 */
		/* Position  mortally x 3.00 */

		dam = MAX(1, dam);  /* Not less than 0 damage */

		if (type == SKILL_BACKSTAB) {
			dam *= backstab_mult[GET_LEVEL(ch)];
			damage(ch, victim, dam, SKILL_BACKSTAB);
		} else
			damage(ch, victim, dam, w_type);
	}
}



/* control the fights going on */
void perform_violence(void)
{
	struct char_data *ch;

	for (ch = combat_list; ch; ch=combat_next_dude)
	{
		combat_next_dude = ch->next_fighting;
		assert(ch->specials.fighting);

		if (AWAKE(ch) && (ch->in_room==ch->specials.fighting->in_room)) {
			hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
		} else { /* Not in same room */
			stop_fighting(ch);
		}
	}
}
