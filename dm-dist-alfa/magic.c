/* ************************************************************************
*  file: magic.c , Implementation of spells.              Part of DIKUMUD *
*  Usage : The actual effect of magic.                                    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "limits.h"

/* Extern structures */
extern struct room_data *world;
extern struct obj_data  *object_list;
extern struct char_data *character_list;

/* Extern procedures */
extern void do_look(struct char_data *ch, char *argument, int cmd);

void damage(struct char_data *ch, struct char_data *victim,
            int damage, int weapontype);
bool saves_spell(struct char_data *ch, sh_int spell);
void weight_change_object(struct obj_data *obj, int weight);
int dice(int number, int size);


/* Offensive Spells */

void spell_magic_missile(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
	int dam_each[] = 
		{0,  3,3,4,4,5, 6,6,6,6,6, 6,6,6,6,6, 6,6,6,6,6,
		     6,6,6,6,6, 6,6,6,6,6};

  assert(victim && ch);
  assert((level >= 1) && (level <= 30)); 

  dam = number(dam_each[level]>>1, dam_each[level]<<1);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_MAGIC_MISSILE);
}



void spell_chill_touch(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  int dam;
	int dam_each[] = 
		{0,  0,0,8,7,9, 9,12,13,13,13, 13,13,13,13,13, 13,13,13,13,13,
		     13,13,13,13,13, 13,13,13,13,13};

  assert(victim && ch);
  assert((level >= 3) && (level <= 30)); 

  dam = number(dam_each[level]>>1, dam_each[level]<<1);

  if ( !saves_spell(victim, SAVING_SPELL) )
  {
    af.type      = SPELL_CHILL_TOUCH;
    af.duration  = 6;
    af.modifier  = -1;
    af.location  = APPLY_STR;
    af.bitvector = 0;
    affect_join(victim, &af, TRUE, FALSE);
  } else {
    dam >>= 1;
  }
  damage(ch, victim, dam, SPELL_CHILL_TOUCH);
}



void spell_burning_hands(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
	int dam_each[] = 
		{0,  0,0,0,0,19, 17,20,19,19,19, 19,19,19,19,19, 19,19,19,19,19,
		     19,19,19,19,19, 19,19,19,19,19};


  assert(victim && ch);
  assert((level >= 5) && (level <= 30));

  dam = number(dam_each[level]>>1, dam_each[level]<<1);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_BURNING_HANDS);
}



void spell_shocking_grasp(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
	int dam_each[] = 
		{0,  0,0,0,0,0, 0,41,33,29,27, 25,25,25,25,25, 25,25,25,25,25,
		     25,25,25,25,25, 25,25,25,25,25};

  assert(victim && ch);
  assert((level >= 7) && (level <= 30)); 

  dam = number(dam_each[level]>>1, dam_each[level]<<1);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_SHOCKING_GRASP);
}



void spell_lightning_bolt(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
	int dam_each[] = 
		{0,  0,0,0,0,0, 0,0,0,59,46, 39,35,38,36,36, 36,36,36,36,36,
		     36,36,36,36,36, 36,36,36,36,36};

  assert(victim && ch);
  assert((level >= 9) && (level <= 30)); 

  dam = number(dam_each[level]>>1, dam_each[level]<<1);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_LIGHTNING_BOLT);
}



void spell_colour_spray(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
	int dam_each[] = 
		{0,  0,0,0,0,0, 0,0,0,0,0, 79,60,57,51,47, 44,44,44,44,44,
		     44,44,44,44,44, 44,44,44,44,44};


	assert(victim && ch);
	assert((level >= 11) && (level <= 30));

  dam = number(dam_each[level]-20, dam_each[level]+20);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_COLOUR_SPRAY);
}


/* Drain XP, MANA, HP - caster gains HP and MANA */
void spell_energy_drain(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam, xp, mana;

  void set_title(struct char_data *ch);
	void gain_exp(struct char_data *ch, int gain);

  assert(victim && ch);
  assert((level >= 13) && (level <=  30));

  if ( !saves_spell(victim, SAVING_SPELL) ) {
		GET_ALIGNMENT(ch) = MIN(-1000, GET_ALIGNMENT(ch)-200);
    if (GET_LEVEL(victim) <= 2) {
      damage(ch, victim, 100, SPELL_ENERGY_DRAIN); /* Kill the sucker */
    } else {
			xp = number(level>>1,level)*1000;
			gain_exp(victim, -xp);

			dam = dice(1,10);

			mana = GET_MANA(victim)>>1;
			GET_MOVE(victim) >>= 1;
			GET_MANA(victim) = mana;

			GET_MANA(ch) += mana>>1;
			GET_HIT(ch) += dam;

      send_to_char("Your life energy is drained!\n\r", victim);

      damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
	  }
  } else {
    damage(ch, victim, 0, SPELL_ENERGY_DRAIN); /* Miss */
  }
}



void spell_fireball(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
	int dam_each[] = 
		{0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,143, 105,88,77,71,71,
		     71,71,71,71,71, 71,71,71,71,71};

  assert(victim && ch);
  assert((level >= 15) && (level <= 30)); 

  dam = number(dam_each[level]-20, dam_each[level]+20);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  damage(ch, victim, dam, SPELL_FIREBALL);
}




void spell_earthquake(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
  struct char_data *tmp_victim, *temp;

  assert(ch);
  assert((level >= 7) && (level <= 30)); 

	dam =  dice(1,8)+level;

  send_to_char("The earth trembles beneath your feet!\n\r", ch);
  act("$n makes the earth trembles and shivers\n\rYou fall, and hit yourself!",
	  FALSE, ch, 0, 0, TO_ROOM);


  for(tmp_victim = character_list; tmp_victim; tmp_victim = temp)
  {
    temp = tmp_victim->next;
    if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim) ) {
			damage(ch, tmp_victim, dam, SPELL_EARTHQUAKE);
    } else
      if (world[ch->in_room].zone == world[tmp_victim->in_room].zone)
        send_to_char("The earth trembles and shivers\n\r", tmp_victim);
  }

}



void spell_dispel_evil(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

	assert(ch && victim);
	assert((level >= 10) && (level<=30));

	if (IS_EVIL(ch))
		victim = ch;
	else
	  if (IS_GOOD(victim)) {
			act("God protects $N.", FALSE, ch, 0, victim, TO_CHAR);
  	  return;
		}

  if ((GET_LEVEL(victim) < level) || (victim == ch))
    dam = 100;
  else {
    dam = dice(level,4);

	  if ( saves_spell(victim, SAVING_SPELL) )
  	  dam >>= 1;
	}

  damage(ch, victim, dam, SPELL_DISPEL_EVIL);
}



void spell_call_lightning(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  extern struct weather_data weather_info;

  assert(victim && ch);
  assert((level >= 12) && (level <= 30));

	dam = dice(MIN(level,15), 8);

  if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {

	  if ( saves_spell(victim, SAVING_SPELL) )
  	  dam >>= 1;
  
  	damage(ch, victim, dam, SPELL_CALL_LIGHTNING);
	}
}



void spell_harm(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

	assert(victim && ch);
	assert((level >= 15) && (level <= 30));

	dam = GET_HIT(victim) - dice(1,4);

	if (dam < 0)
		dam = 0; /* Kill the suffering bastard */
	else {
	  if ( saves_spell(victim, SAVING_SPELL) )
			dam = MIN(50, dam/2);
	}

  damage(ch, victim, dam, SPELL_HARM);
}



/* spells2.c - Not directly offensive spells */

void spell_armor(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
  assert((level >= 0) && (level <= 30));

	if (!affected_by_spell(victim, SPELL_ARMOR)) {
	  af.type      = SPELL_ARMOR;
  	af.duration  = 24;
	  af.modifier  = -20;
	  af.location  = APPLY_AC;
	  af.bitvector = 0;

	  affect_to_char(victim, &af);
		send_to_char("You feel someone protecting you.\n\r", victim);
	}
}



void spell_teleport(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int to_room;
	extern int top_of_world;      /* ref to the top element of world */

	assert(ch);

	do {
		to_room = number(0, top_of_world);
	} while (IS_SET(world[to_room].room_flags, PRIVATE));

  act("$n slowly fade out of existence.", FALSE, ch,0,0,TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, to_room);
  act("$n slowly fade in to existence.", FALSE, ch,0,0,TO_ROOM);

	do_look(ch, "", 0);

	if (IS_SET(world[to_room].room_flags, DEATH) && GET_LEVEL(ch) < 21) {
    death_cry(ch);
    extract_char(ch);
  }
}



void spell_bless(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(ch && (victim || obj));
	assert((level >= 0) && (level <= 30));

  if (obj) {
    if ( (5*GET_LEVEL(ch) > GET_OBJ_WEIGHT(obj)) &&
         (GET_POS(ch) != POSITION_FIGHTING) &&
         !IS_OBJ_STAT(obj, ITEM_EVIL)) {
	    SET_BIT(obj->obj_flags.extra_flags, ITEM_BLESS);
			act("$p briefly glows.",FALSE,ch,obj,0,TO_CHAR);
		}
	} else {

		if ((GET_POS(victim) != POSITION_FIGHTING) &&
		    (!affected_by_spell(victim, SPELL_BLESS))) {

			send_to_char("You feel righteous.\n\r", victim);
			af.type      = SPELL_BLESS;
  	  af.duration  = 6;
    	af.modifier  = 1;
	    af.location  = APPLY_HITROLL;
  	  af.bitvector = 0;
    	affect_to_char(victim, &af);

	    af.location = APPLY_SAVING_SPELL;
  	  af.modifier = -1;                 /* Make better */
    	affect_to_char(victim, &af);
		}
	}
}



void spell_blindness(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(ch && victim);
  assert((level >= 0) && (level <= 30));


  if (saves_spell(victim, SAVING_SPELL) ||
	   affected_by_spell(victim, SPELL_BLINDNESS))
		return;

  act("$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM);
  send_to_char("You have been blinded!\n\r", victim);

  af.type      = SPELL_BLINDNESS;
  af.location  = APPLY_HITROLL;
  af.modifier  = -4;  /* Make hitroll worse */
  af.duration  = 1;
  af.bitvector = AFF_BLIND;
  affect_to_char(victim, &af);


  af.location = APPLY_AC;
  af.modifier = +40; /* Make AC Worse! */
  affect_to_char(victim, &af);
}



void spell_clone(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{

  assert(ch && (victim || obj));
  assert((level >= 0) && (level <= 30));

	send_to_char("Clone is not ready yet.", ch);

  if (obj) {

	} else {
		/* clone_char(victim); */
	}
}



void spell_control_weather(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   /* Control Weather is not possible here!!! */
   /* Better/Worse can not be transferred     */
}



void spell_create_food(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct obj_data *tmp_obj;

  assert(ch);
  assert((level >= 0) && (level <= 30));

  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);

  tmp_obj->name = strdup("mushroom");
  tmp_obj->short_description = strdup("A Magic Mushroom");
  tmp_obj->description = strdup("A really delicious looking magic mushroom lies here.");

  tmp_obj->obj_flags.type_flag = ITEM_FOOD;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
  tmp_obj->obj_flags.value[0] = 5+level;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 10;
  tmp_obj->obj_flags.cost_per_day = 1;

  tmp_obj->next = object_list;
  object_list = tmp_obj;

  obj_to_room(tmp_obj,ch->in_room);

  tmp_obj->item_number = -1;

	act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
	act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_CHAR);
}



void spell_create_water(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int water;

  extern struct weather_data weather_info;
	void name_to_drinkcon(struct obj_data *obj,int type);
	void name_from_drinkcon(struct obj_data *obj);

  assert(ch && obj);

	if (GET_ITEM_TYPE(obj) == ITEM_DRINKCON) {
		if ((obj->obj_flags.value[2] != LIQ_WATER)
		     && (obj->obj_flags.value[1] != 0)) {

			name_from_drinkcon(obj);
			obj->obj_flags.value[2] = LIQ_SLIME;
			name_to_drinkcon(obj, LIQ_SLIME);

		} else {

			water = 2*level * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);

			/* Calculate water it can contain, or water created */
			water = MIN(obj->obj_flags.value[0]-obj->obj_flags.value[1], water);

			if (water > 0) {
			  obj->obj_flags.value[2] = LIQ_WATER;
				obj->obj_flags.value[1] += water;

				weight_change_object(obj, water);

				name_from_drinkcon(obj);
				name_to_drinkcon(obj, LIQ_WATER);
				act("$p is filled.", FALSE, ch,obj,0,TO_CHAR);
			}
		}
	}
}



void spell_cure_blind(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  assert(victim);
  assert((level >= 0) && (level <= 30));

	if (affected_by_spell(victim, SPELL_BLINDNESS)) {
	  affect_from_char(victim, SPELL_BLINDNESS);

	  send_to_char("Your vision returns!\n\r", victim);
	}
}



void spell_cure_critic(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int healpoints;

  assert(victim);
  assert((level >= 0) && (level <= 30));

  healpoints = dice(3,8)+3;

  if ( (healpoints + GET_HIT(victim)) > hit_limit(victim) )
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += healpoints;

  send_to_char("You feel better!\n\r", victim);

  update_pos(victim);
}



void spell_cure_light(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int healpoints;

  assert(ch && victim);
  assert((level >= 0) && (level <= 30));

  healpoints = dice(1,8);

  if ( (healpoints+GET_HIT(victim)) > hit_limit(victim) )
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += healpoints;

  update_pos( victim );

  send_to_char("You feel better!\n\r", victim);
}



void spell_curse(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim || obj);
  assert((level >= 0) && (level <= 30));

  if (obj) {
    SET_BIT(obj->obj_flags.extra_flags, ITEM_EVIL);
    SET_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);

    /* LOWER ATTACK DICE BY -1 */
    if(obj->obj_flags.type_flag == ITEM_WEAPON)
      obj->obj_flags.value[2]--;
		act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
	} else {
    if ( saves_spell(victim, SAVING_SPELL) ||
		   affected_by_spell(victim, SPELL_CURSE))
      return;

    af.type      = SPELL_CURSE;
    af.duration  = 24*7;       /* 7 Days */
    af.modifier  = -1;
    af.location  = APPLY_HITROLL;
    af.bitvector = AFF_CURSE;
    affect_to_char(victim, &af);

    af.location = APPLY_SAVING_PARA;
    af.modifier = 1; /* Make worse */
    affect_to_char(victim, &af);

    act("$n briefly reveal a red aura!", FALSE, victim, 0, 0, TO_ROOM);
		act("You feel very uncomfortable.",FALSE,victim,0,0,TO_CHAR);
	}
}



void spell_detect_evil(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

	assert(victim);
	assert((level >= 0) && (level <= 30));

	if ( affected_by_spell(victim, SPELL_DETECT_EVIL) )
		return;

  af.type      = SPELL_DETECT_EVIL;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_EVIL;

  affect_to_char(victim, &af);

  send_to_char("Your eyes tingle.\n\r", victim);
}



void spell_detect_invisibility(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
  assert((level >= 0) && (level <= 30));

  if ( affected_by_spell(victim, SPELL_DETECT_INVISIBLE) )
		return;

  af.type      = SPELL_DETECT_INVISIBLE;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_INVISIBLE;

  affect_to_char(victim, &af);

  send_to_char("Your eyes tingle.\n\r", victim);
}



void spell_detect_magic(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
  assert((level >= 0) && (level <= 30));

  if ( affected_by_spell(victim, SPELL_DETECT_MAGIC) )
		return;

  af.type      = SPELL_DETECT_MAGIC;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_MAGIC;

  affect_to_char(victim, &af);
  send_to_char("Your eyes tingle.\n\r", victim);
}



void spell_detect_poison(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	assert(ch && (victim || obj));

  if (victim) {
    if (victim == ch)
      if (IS_AFFECTED(victim, AFF_POISON))
        send_to_char("You can sense poison in your blood.\n\r", ch);
      else
        send_to_char("You feel healthy.\n\r", ch);
    else
      if (IS_AFFECTED(victim, AFF_POISON)) {
        act("You sense that $E is poisoned.",FALSE,ch,0,victim,TO_CHAR);
      } else {
        act("You sense that $E is healthy.",FALSE,ch,0,victim,TO_CHAR);
      }
  } else { /* It's an object */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      if (obj->obj_flags.value[3])
        act("Poisonous fumes are revealed.",FALSE, ch, 0, 0, TO_CHAR);
      else
        send_to_char("It looks very delicious.\n\r", ch);
    }
  }
}



void spell_enchant_weapon(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int i;

	assert(ch && obj);
	assert(MAX_OBJ_AFFECT >= 2);

	if ((GET_ITEM_TYPE(obj) == ITEM_WEAPON) &&
	   !IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {

		for (i=0; i < MAX_OBJ_AFFECT; i++)
			if (obj->affected[i].location != APPLY_NONE)
				return;

		SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

		obj->affected[0].location = APPLY_HITROLL;
		obj->affected[0].modifier = 1 + (level >= 18);

		obj->affected[1].location = APPLY_DAMROLL;
		obj->affected[1].modifier = 1 + (level >= 20);

		if (IS_GOOD(ch)) {
			SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
			act("$p glows blue.",FALSE,ch,obj,0,TO_CHAR);
		} else if (IS_EVIL(ch)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
      act("$p glows red.",FALSE,ch,obj,0,TO_CHAR);
    } else {
      act("$p glows yellow.",FALSE,ch,obj,0,TO_CHAR);
		}
	}
}



void spell_heal(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	assert(victim);

	spell_cure_blind(level, ch, victim, obj);

	GET_HIT(victim) += 100;

	if (GET_HIT(victim) >= hit_limit(victim))
		GET_HIT(victim) = hit_limit(victim)-dice(1,4);

  update_pos( victim );

  send_to_char("A warm feeling fills your body.\n\r", victim);
}


void spell_invisibility(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

	assert((ch && obj) || victim);

  if (obj) {
    if ( !IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE) ) {
			act("$p turns invisible.",FALSE,ch,obj,0,TO_CHAR);
			act("$p turns invisible.",TRUE,ch,obj,0,TO_ROOM);
      SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
		}
  } else {              /* Then it is a PC | NPC */
		if (!affected_by_spell(victim, SPELL_INVISIBLE)) {

		  act("$n slowly fade out of existence.", TRUE, victim,0,0,TO_ROOM);
  	  send_to_char("You vanish.\n\r", victim);

	    af.type      = SPELL_INVISIBLE;
    	af.duration  = 24;
    	af.modifier  = -40;
  	  af.location  = APPLY_AC;
	    af.bitvector = AFF_INVISIBLE;
  	  affect_to_char(victim, &af);
  	}
	}
}


void spell_locate_object(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct obj_data *i;
  char name[256];
  char buf[MAX_STRING_LENGTH];
  int j;

	assert(ch);

	strcpy(name, fname(obj->name));

	j=level>>1;

	for (i = object_list; i && (j>0); i = i->next)
    if (isname(name, i->name)) {
      if(i->carried_by) {
        sprintf(buf,"%s carried by %s.\n\r",
          i->short_description,PERS(i->carried_by,ch));
        send_to_char(buf,ch);
      } else if (i->in_obj) {
        sprintf(buf,"%s in %s.\n\r",i->short_description,
          i->in_obj->short_description);
        send_to_char(buf,ch);
      } else {
        sprintf(buf,"%s in %s.\n\r",i->short_description,
          (i->in_room == NOWHERE ? "use, but uncertain." : world[i->in_room].name));
        send_to_char(buf,ch);
      j--;
      }
    }

  if(j==0)
    send_to_char("You are very confused.\n\r",ch);
  if(j==level>>1)
    send_to_char("No such object.\n\r",ch);
}


void spell_poison(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	assert(victim || obj);

  if (victim) {
    if(!saves_spell(victim, SAVING_PARA))
    {
      af.type = SPELL_POISON;
      af.duration = level*2;
      af.modifier = -2;
      af.location = APPLY_STR;
      af.bitvector = AFF_POISON;

      affect_join(victim, &af, FALSE, FALSE);

      send_to_char("You feel very sick.\n\r", victim);
    }

  } else { /* Object poison */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 1;
    }
  }
}


void spell_protection_from_evil(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

	assert(victim);

  if (!affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL) ) {
    af.type      = SPELL_PROTECT_FROM_EVIL;
    af.duration  = 24;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_PROTECT_EVIL;
    affect_to_char(victim, &af);
		send_to_char("You have a righteous feeling!\n\r", victim);
	}
}


void spell_remove_curse(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	assert(ch && (victim || obj));

	if (obj) {

		if ( IS_SET(obj->obj_flags.extra_flags, ITEM_EVIL) ||
		     IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
			act("$p briefly glows blue.", TRUE, ch, obj, 0, TO_CHAR);

			REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_EVIL);
			REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
		}
	} else {      /* Then it is a PC | NPC */
		if (affected_by_spell(victim, SPELL_CURSE) ) {
			act("$n briefly glows red, then blue.",FALSE,victim,0,0,TO_ROOM);
			act("You feel better.",FALSE,victim,0,0,TO_CHAR);
      affect_from_char(victim, SPELL_CURSE);
    }
	}
}


void spell_remove_poison(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{

	assert(ch && (victim || obj));

  if (victim) {
    if(affected_by_spell(victim,SPELL_POISON)) {
      affect_from_char(victim,SPELL_POISON);
      act("A warm feeling runs through your body.",FALSE,victim,0,0,TO_CHAR);
			act("$N looks better.",FALSE,ch,0,victim,TO_ROOM);
    }
	} else {
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 0;
			act("The $p steams briefly.",FALSE,ch,obj,0,TO_CHAR);
		}
	}
}



void spell_sanctuary(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if (!affected_by_spell(victim, SPELL_SANCTUARY) ) {

    act("$n is surrounded by a white aura.",TRUE,victim,0,0,TO_ROOM);
		act("You start glowing.",TRUE,victim,0,0,TO_CHAR);

    af.type      = SPELL_SANCTUARY;
    af.duration  = (level<21) ? 3 : level;
    af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_SANCTUARY;
    affect_to_char(victim, &af);
  }
}



void spell_sleep(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

	assert(victim);

  if ( !saves_spell(victim, SAVING_SPELL) )
  {
    af.type      = SPELL_SLEEP;
    af.duration  = 4+level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SLEEP;
    affect_join(victim, &af, FALSE, FALSE);

    if (GET_POS(victim)>POSITION_SLEEPING)
    {
      act("You feel very sleepy ..... zzzzzz",FALSE,victim,0,0,TO_CHAR);
      act("$n go to sleep.",TRUE,victim,0,0,TO_ROOM);
	    GET_POS(victim)=POSITION_SLEEPING;
    }

    return;
  }
}



void spell_strength(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

	assert(victim);

  act("You feel stronger.", FALSE, victim,0,0,TO_CHAR);

  af.type      = SPELL_STRENGTH;
  af.duration  = level;
  af.modifier  = 1+(level>18);

  af.location  = APPLY_STR;
  af.bitvector = 0;

  affect_join(victim, &af, TRUE, FALSE);
}



void spell_ventriloquate(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	/* Not possible!! No argument! */
}



void spell_word_of_recall(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  extern int top_of_world;
  int loc_nr,location;
  bool found = FALSE;

  void do_look(struct char_data *ch, char *argument, int cmd);

	assert(victim);

  if (IS_NPC(victim))
    return;

  /*  loc_nr = GET_HOME(ch); */

  loc_nr = 3001;
  for (location = 0; location <= top_of_world; location++)
    if (world[location].number == loc_nr) {
      found = TRUE;
      break;
    }

  if ((location == top_of_world) || !found)
  {
    send_to_char("You are completely lost.\n\r", victim);
    return;
  }

	/* a location has been found. */

  act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, location);
  act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
  do_look(victim, "",15);

}


void spell_summon(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	sh_int target;

	assert(ch && victim);

	if (GET_LEVEL(victim) > MIN(20,level+3)) {
		send_to_char("You failed.\n\r",ch);
		return;
	}

  if ((IS_NPC(victim) && saves_spell(victim, SAVING_SPELL)) ||
      IS_SET(world[victim->in_room].room_flags,PRIVATE)) {
		send_to_char("You failed.\n\r", ch);
		return;
	}

	act("$n disappears suddenly.",TRUE,victim,0,0,TO_ROOM);

	target = ch->in_room;
	char_from_room(victim);
	char_to_room(victim,target);

	act("$n arrives suddenly.",TRUE,victim,0,0,TO_ROOM);
	act("$n has summoned you!",FALSE,ch,0,victim,TO_VICT);
	do_look(victim,"",15);
}


void spell_charm_person(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  void add_follower(struct char_data *ch, struct char_data *leader);
  bool circle_follow(struct char_data *ch, struct char_data *victim);
  void stop_follower(struct char_data *ch);

	assert(ch && victim);

  /* By testing for IS_AFFECTED we avoid ei. Mordenkainens sword to be */
  /* abel to be "recharmed" with duration                              */

	if (victim == ch) {
		send_to_char("You like yourself even better!\n\r", ch);
		return;
	}

  if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM) &&
      (level >= GET_LEVEL(victim))) {
    if (circle_follow(victim, ch)) {
      send_to_char("Sorry, following in circles can not be allowed.\n\r", ch);
      return;
    }

		if (saves_spell(victim, SAVING_PARA))
			return;

    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);

    af.type      = SPELL_CHARM_PERSON;

    if (GET_INT(victim))
      af.duration  = 24*18/GET_INT(victim);
    else
      af.duration  = 24*18;

    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);

		act("Isn't $n just such a nice fellow?",FALSE,ch,0,victim,TO_VICT);
	}
}



void spell_sense_life(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

	assert(victim);

  if (!affected_by_spell(victim, SPELL_SENSE_LIFE)) {
    send_to_char("Your feel your awareness improve.\n\r", ch);

    af.type      = SPELL_SENSE_LIFE;
    af.duration  = 5*level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SENSE_LIFE;
    affect_to_char(victim, &af);
  }
}


/* ***************************************************************************
 *                     Not cast-able spells                                  *
 * ************************************************************************* */

void spell_identify(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  char buf[256+25], buf2[256];
	int i;
  bool found;

	struct time_info_data age(struct char_data *ch);

  /* Spell Names */
	extern char *spells[];

	/* For Objects */
	extern char *item_types[];
	extern char *extra_bits[];
	extern char *apply_types[];
	extern char *affected_bits[];


  assert(ch && (obj || victim));

  if (obj) {
		send_to_char("You feel informed:\n\r", ch);

		sprintf(buf, "Object '%s', Item type: ", obj->name);
		sprinttype(GET_ITEM_TYPE(obj),item_types,buf2);
		strcat(buf,buf2); strcat(buf,"\n\r");
		send_to_char(buf, ch);

		if (obj->obj_flags.bitvector) {
			send_to_char("Item will give you following abilities:  ", ch);
			sprintbit(obj->obj_flags.bitvector,affected_bits,buf);
			strcat(buf,"\n\r");
			send_to_char(buf, ch);
		}

		send_to_char("Item is: ", ch);
		sprintbit(obj->obj_flags.extra_flags,extra_bits,buf);
		strcat(buf,"\n\r");
		send_to_char(buf,ch);

		sprintf(buf,"Weight: %d, Value: %d\n\r",
		  obj->obj_flags.weight, obj->obj_flags.cost);
		send_to_char(buf, ch);

    switch (GET_ITEM_TYPE(obj)) {

			case ITEM_SCROLL : 
			case ITEM_POTION :
				sprintf(buf, "Level %d spells of:\n\r",	obj->obj_flags.value[0]);
				send_to_char(buf, ch);
				if (obj->obj_flags.value[1] >= 1) {
					sprinttype(obj->obj_flags.value[1]-1,spells,buf);
					strcat(buf,"\n\r");
					send_to_char(buf, ch);
				}
				if (obj->obj_flags.value[2] >= 1) {
					sprinttype(obj->obj_flags.value[2]-1,spells,buf);
					strcat(buf,"\n\r");
					send_to_char(buf, ch);
				}
				if (obj->obj_flags.value[3] >= 1) {
					sprinttype(obj->obj_flags.value[3]-1,spells,buf);
					strcat(buf,"\n\r");
					send_to_char(buf, ch);
				}
				break;

			case ITEM_WAND : 
			case ITEM_STAFF : 
				sprintf(buf, "Has %d chages, with %d charges left.\n\r",
				  obj->obj_flags.value[1],
				  obj->obj_flags.value[2]);
				send_to_char(buf, ch);

				sprintf(buf, "Level %d spell of:\n\r",	obj->obj_flags.value[0]);
				send_to_char(buf, ch);

				if (obj->obj_flags.value[3] >= 1) {
					sprinttype(obj->obj_flags.value[3]-1,spells,buf);
					strcat(buf,"\n\r");
					send_to_char(buf, ch);
				}
				break;

			case ITEM_WEAPON :
				sprintf(buf, "Damage Dice is '%dD%d'\n\r",
					obj->obj_flags.value[1],
	   			obj->obj_flags.value[2]);
				send_to_char(buf, ch);
				break;

			case ITEM_ARMOR :
				sprintf(buf, "AC-apply is %d\n\r",
					obj->obj_flags.value[0]);
				send_to_char(buf, ch);
				break;

    }

		found = FALSE;

		for (i=0;i<MAX_OBJ_AFFECT;i++) {
			if ((obj->affected[i].location != APPLY_NONE) &&
			   (obj->affected[i].modifier != 0)) {
				if (!found) {
					send_to_char("Can affect you as :\n\r", ch);
					found = TRUE;
				}

				sprinttype(obj->affected[i].location,apply_types,buf2);
				sprintf(buf,"    Affects : %s By %d\n\r", buf2,obj->affected[i].modifier);
				send_to_char(buf, ch);
			}
		}

  } else {       /* victim */

		if (!IS_NPC(victim)) {
			sprintf(buf,"%d Years,  %d Months,  %d Days,  %d Hours old.\n\r",
			        age(victim).year, age(victim).month,
			        age(victim).day, age(victim).hours);
			send_to_char(buf,ch);

			sprintf(buf,"Height %dcm  Weight %dpounds \n\r",
			  GET_HEIGHT(victim), GET_WEIGHT(victim));
			send_to_char(buf,ch);

/*
			sprintf(buf,"Str %d/%d,  Int %d,  Wis %d,  Dex %d,  Con %d\n\r",
				GET_STR(victim), GET_ADD(victim),
				GET_INT(victim),
				GET_WIS(victim),
				GET_DEX(victim),
				GET_CON(victim) );
			send_to_char(buf,ch);
*/

    } else {
      send_to_char("You learn nothing new.\n\r", ch);
    }
  }

}


/* ***************************************************************************
 *                     NPC spells..                                          *
 * ************************************************************************* */

void spell_fire_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	int hpch;
	struct obj_data *burn;

	assert(victim && ch);
	assert((level >= 1) && (level <= 30)); 

	hpch = GET_HIT(ch);
	if(hpch<10) hpch=10;

	dam = number(0,hpch>>2);

	if ( saves_spell(victim, SAVING_BREATH) )
		dam >>= 1;

	damage(ch, victim, dam, SPELL_FIRE_BREATH);

	/* And now for the damage on inventory */

	if(number(0,30)<GET_LEVEL(ch))
	{
		if (!saves_spell(victim, SAVING_BREATH) )
		{
			for(burn=victim->carrying ; 
				burn && !(((burn->obj_flags.type_flag==ITEM_SCROLL) || 
				(burn->obj_flags.type_flag==ITEM_WAND) ||
				(burn->obj_flags.type_flag==ITEM_STAFF) ||
				(burn->obj_flags.type_flag==ITEM_NOTE)) &&
            (number(0,2)==0)) ;
				 burn=burn->next_content);
			if(burn)
			{
				act("$o burns",0,victim,burn,0,TO_CHAR);
				extract_obj(burn);
			}
		}
	}
}


void spell_frost_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	int hpch;
	struct obj_data *frozen;

	assert(victim && ch);
	assert((level >= 1) && (level <= 30)); 

	hpch = GET_HIT(ch);
	if(hpch<10) hpch=10;

	dam = number(0,hpch>>2);

	if ( saves_spell(victim, SAVING_BREATH) )
		dam >>= 1;

	damage(ch, victim, dam, SPELL_FROST_BREATH);

	/* And now for the damage on inventory */

	if(number(0,30)<GET_LEVEL(ch))
	{
		if (!saves_spell(victim, SAVING_BREATH) )
		{
			for(frozen=victim->carrying ; 
				frozen && !(((frozen->obj_flags.type_flag==ITEM_DRINKCON) || 
				(frozen->obj_flags.type_flag==ITEM_FOOD) ||
				(frozen->obj_flags.type_flag==ITEM_POTION)) &&
            (number(0,2)==0)) ;
				 frozen=frozen->next_content); 
			if(frozen)
			{
				act("$o breaks.",0,victim,frozen,0,TO_CHAR);
				extract_obj(frozen);
			}
		}
	}
}


void spell_acid_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	int hpch;
	int damaged;

	int apply_ac(struct char_data *ch, int eq_pos);
	
	assert(victim && ch);
	assert((level >= 1) && (level <= 30)); 

	hpch = GET_HIT(ch);
	if(hpch<10) hpch=10;

	dam = number(0,hpch>>2);

	if ( saves_spell(victim, SAVING_BREATH) )
		dam >>= 1;

	damage(ch, victim, dam, SPELL_ACID_BREATH);

	/* And now for the damage on equipment */

	if(number(0,30)<GET_LEVEL(ch))
	{
		if (!saves_spell(victim, SAVING_BREATH) )
		{
			for(damaged = 0; damaged<MAX_WEAR &&
				!((victim->equipment[damaged]) &&
				(victim->equipment[damaged]->obj_flags.type_flag!=ITEM_ARMOR) &&
				(victim->equipment[damaged]->obj_flags.value[0]>0) && 
            (number(0,2)==0)) ; damaged++);  
			if(damaged<MAX_WEAR)
			{
				act("$o is damaged.",0,victim,victim->equipment[damaged],0,TO_CHAR);
				GET_AC(victim)-=apply_ac(victim,damaged);
				victim->equipment[damaged]->obj_flags.value[0]-=number(1,7);
				GET_AC(victim)+=apply_ac(victim,damaged);
				victim->equipment[damaged]->obj_flags.cost = 0;
			}
		}
	}
}


void spell_gas_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	int hpch;

	assert(victim && ch);
	assert((level >= 1) && (level <= 30)); 

	hpch = GET_HIT(ch);
	if(hpch<10) hpch=10;

	dam = number(0,hpch>>2);

	if ( saves_spell(victim, SAVING_BREATH) )
		dam >>= 1;

	damage(ch, victim, dam, SPELL_GAS_BREATH);


}


void spell_lightning_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	int hpch;

	assert(victim && ch);
	assert((level >= 1) && (level <= 30)); 

	hpch = GET_HIT(ch);
	if(hpch<10) hpch=10;

	dam = number(0,hpch>>2);

	if ( saves_spell(victim, SAVING_BREATH) )
		dam >>= 1;

	damage(ch, victim, dam, SPELL_LIGHTNING_BREATH);


}
