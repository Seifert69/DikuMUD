/* ************************************************************************
*  file: spells2.c , Implementation of magic.             Part of DIKUMUD *
*  Usage : All the non-offensive magic handling routines.                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"


/* Global data */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct spell_info_type spell_info[MAX_SPL_LIST];
extern struct obj_data  *object_list;


/* Extern procedures */
extern void slog(char *msg);

void die(struct char_data *ch);
void update_pos( struct char_data *victim );
void damage(struct char_data *ch, struct char_data *victim,
            int damage, int weapontype);
void clone_char(struct char_data *ch);
void say_spell( struct char_data *ch, int si );
bool saves_spell(struct char_data *ch, sh_int spell);
void add_follower(struct char_data *ch, struct char_data *victim);
char *strdup(char *str);


void cast_armor( byte level, struct char_data *ch, char *arg, int type,
	struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
		case SPELL_TYPE_SPELL:
			if ( affected_by_spell(tar_ch, SPELL_ARMOR) ){
				send_to_char("Nothing seems to happen.\n\r", ch);
				return;
			}
			if (ch != tar_ch)
				act("$N is protected by your deity.", FALSE, ch, 0, tar_ch, TO_CHAR);

			spell_armor(level,ch,tar_ch,0);
			break;
		case SPELL_TYPE_POTION:
			if ( affected_by_spell(ch, SPELL_ARMOR) )
				return;
			spell_armor(level,ch,ch,0);
			break;
		case SPELL_TYPE_SCROLL:
			if (tar_obj) return;
         if (!tar_ch) tar_ch = ch;
			if ( affected_by_spell(tar_ch, SPELL_ARMOR) )
				return;
			spell_armor(level,ch,ch,0);
			break;
		case SPELL_TYPE_WAND:
			if (tar_obj) return;
			if ( affected_by_spell(tar_ch, SPELL_ARMOR) )
				return;
			spell_armor(level,ch,ch,0);
			break;
      default :
	      slog("Serious screw-up in armor!");
         break;
	}
}



void cast_teleport( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
		case SPELL_TYPE_SCROLL:
		case SPELL_TYPE_POTION:
    case SPELL_TYPE_SPELL:
			if (!tar_ch)
				tar_ch = ch;
			spell_teleport(level, ch, tar_ch, 0);
			break;

		case SPELL_TYPE_WAND:
			if(!tar_ch) return;
			spell_teleport(level, ch, tar_ch, 0);
			break;

    case SPELL_TYPE_STAFF:
      for (tar_ch = world[ch->in_room].people ; 
           tar_ch ; tar_ch = tar_ch->next_in_room)
         if (tar_ch != ch) 
            spell_teleport(level, ch, tar_ch, 0);
      break;
			
    default :
	    slog("Serious screw-up in teleport!");
      break;
	}
}


void cast_bless( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
	struct affected_type af;

  switch (type) {
    case SPELL_TYPE_SPELL:
			if (tar_obj) {        /* It's an object */
				if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS) ) {
					send_to_char("Nothing seems to happen.\n\r", ch);
					return;
				}
				spell_bless(level,ch,0,tar_obj);

			} else {              /* Then it is a PC | NPC */

				if ( affected_by_spell(tar_ch, SPELL_BLESS) ||
					(GET_POS(tar_ch) == POSITION_FIGHTING)) {
					send_to_char("Nothing seems to happen.\n\r", ch);
					return;
				} 
				spell_bless(level,ch,tar_ch,0);
			}
			break;
	 case SPELL_TYPE_POTION:
   		if ( affected_by_spell(ch, SPELL_BLESS) ||
				(GET_POS(ch) == POSITION_FIGHTING))
				return;
			spell_bless(level,ch,ch,0);
         break;
    case SPELL_TYPE_SCROLL:
			if (tar_obj) {        /* It's an object */
				if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS) )
					return;
				spell_bless(level,ch,0,tar_obj);

			} else {              /* Then it is a PC | NPC */

				if (!tar_ch) tar_ch = ch;
				
				if ( affected_by_spell(tar_ch, SPELL_BLESS) ||
					(GET_POS(tar_ch) == POSITION_FIGHTING))
					return;
				spell_bless(level,ch,tar_ch,0);
			}
			break;
    case SPELL_TYPE_WAND:
			if (tar_obj) {        /* It's an object */
				if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS) )
					return;
				spell_bless(level,ch,0,tar_obj);

			} else {              /* Then it is a PC | NPC */

				if ( affected_by_spell(tar_ch, SPELL_BLESS) ||
					(GET_POS(tar_ch) == POSITION_FIGHTING))
					return;
				spell_bless(level,ch,tar_ch,0);
			}
			break;
    default :
	    slog("Serious screw-up in bless!");
         break;
	}
}



void cast_blindness( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
	struct affected_type af;

  switch (type) {
    case SPELL_TYPE_SPELL:
			if ( IS_AFFECTED(tar_ch, AFF_BLIND) ){
				send_to_char("Nothing seems to happen.\n\r", ch);
				return;
			}
			spell_blindness(level,ch,tar_ch,0);
			break;
    case SPELL_TYPE_POTION:
			if ( IS_AFFECTED(ch, AFF_BLIND) )
				return;
			spell_blindness(level,ch,ch,0);
			break;
    case SPELL_TYPE_SCROLL:
         if (tar_obj) return;
         if (!tar_ch) tar_ch = ch;
			if ( IS_AFFECTED(ch, AFF_BLIND) )
				return;
			spell_blindness(level,ch,ch,0);
			break;
    case SPELL_TYPE_WAND:
         if (tar_obj) return;
			if ( IS_AFFECTED(ch, AFF_BLIND) )
				return;
			spell_blindness(level,ch,ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               if (!(IS_AFFECTED(tar_ch, AFF_BLIND)))
                  spell_blindness(level,ch,tar_ch,0);
         break;
    default :
	    slog("Serious screw-up in blindness!");
         break;
	}
}


void cast_clone( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
	struct char_data *vict;
	char buf[MAX_STRING_LENGTH];

	send_to_char("Not *YET* implemented.", ch);
	return;

  /* clone both char and obj !!*/

/*
  switch (type) {
    case SPELL_TYPE_SPELL:
			if (tar_ch) {	
				sprintf(buf, "You create a duplicate of %s.\n\r", GET_NAME(tar_ch));
				send_to_char(buf, ch);
				sprintf(buf, "%%s creates a duplicate of %s,\n\r", GET_NAME(tar_ch));
				perform(buf, ch, FALSE);

				spell_clone(level,ch,tar_ch,0);
			} else {
				sprintf(buf, "You create a duplicate of %s %s.\n\r",SANA(tar_obj),tar_obj->short_description);
				send_to_char(buf, ch);
				sprintf(buf, "%%s creates a duplicate of %s %s,\n\r",SANA(tar_obj),tar_obj->short_description);
				perform(buf, ch, FALSE);

				spell_clone(level,ch,0,tar_obj);
			};
			break;


    default : 
         slog("Serious screw-up in clone!");
         break;
	}
*/
		 /* MISSING REST OF SWITCH -- POTION, SCROLL, WAND */
}


void cast_control_weather( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
	char buffer[MAX_STRING_LENGTH];
	extern struct weather_data weather_info;

  switch (type) {
    case SPELL_TYPE_SPELL:

			one_argument(arg,buffer);

			if (strcasecmp("better",buffer) && strcasecmp("worse",buffer))
			{
				send_to_char("Do you want it to get better or worse?\n\r",ch);
				return;
			}

			if(!strcasecmp("better",buffer))
				weather_info.change+=(dice(((level)/3),4));
			else
				weather_info.change-=(dice(((level)/3),4)); 
			break;
      default :
	      slog("Serious screw-up in control weather!");
         break;
	}
}



void cast_create_food( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
			act("$n magically creates a mushroom.",FALSE, ch, 0, 0, TO_ROOM);
         spell_create_food(level,ch,0,0);
			break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if(tar_ch) return;
         spell_create_food(level,ch,0,0);
			break;
    default :
	    slog("Serious screw-up in create food!");
         break;
	}
}



void cast_create_water( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			if (tar_obj->obj_flags.type_flag != ITEM_DRINKCON) {
				send_to_char("It is unable to hold water.\n\r", ch);
				return;
			}
			spell_create_water(level,ch,0,tar_obj);
			break;
      default :
	      slog("Serious screw-up in create water!");
         break;
	}
}



void cast_cure_blind( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_cure_blind(level,ch,tar_ch,0);
			break;
    case SPELL_TYPE_POTION:
			spell_cure_blind(level,ch,ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               spell_cure_blind(level,ch,tar_ch,0);
         break;
    default :
	    slog("Serious screw-up in cure blind!");
         break;
	}
}



void cast_cure_critic( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_cure_critic(level,ch,tar_ch,0);
			break;
    case SPELL_TYPE_POTION:
			spell_cure_critic(level,ch,ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               spell_cure_critic(level,ch,tar_ch,0);
         break;
      default :
	      slog("Serious screw-up in cure critic!");
         break;

	}
}



void cast_cure_light( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cure_light(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_POTION:
			spell_cure_light(level,ch,ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               spell_cure_light(level,ch,tar_ch,0);
         break;
    default :
	    slog("Serious screw-up in cure light!");
         break;
  }
}


void cast_curse( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			if (tar_obj)   /* It is an object */ 
				spell_curse(level,ch,0,tar_obj);
			else {              /* Then it is a PC | NPC */
				spell_curse(level,ch,tar_ch,0);
			}
			break;
    case SPELL_TYPE_POTION:
			spell_curse(level,ch,ch,0);
			break;
    case SPELL_TYPE_SCROLL:
			if (tar_obj)   /* It is an object */ 
				spell_curse(level,ch,0,tar_obj);
			else {              /* Then it is a PC | NPC */
				if (!tar_ch) tar_ch = ch;
				spell_curse(level,ch,tar_ch,0);
			}
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               spell_curse(level,ch,tar_ch,0);
         break;
    default :
	    slog("Serious screw-up in curse!");
         break;
	}
}


void cast_detect_evil( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			if ( affected_by_spell(tar_ch, SPELL_DETECT_EVIL) ){
				send_to_char("Nothing seems to happen.\n\r", tar_ch);
				return;
			}
			spell_detect_evil(level,ch,tar_ch,0);
			break;
    case SPELL_TYPE_POTION:
			if ( affected_by_spell(ch, SPELL_DETECT_EVIL) )
				return;
			spell_detect_evil(level,ch,ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               if(!(IS_AFFECTED(tar_ch, SPELL_DETECT_EVIL)))
                  spell_detect_evil(level,ch,tar_ch,0);
         break;
    default :
	    slog("Serious screw-up in detect evil!");
         break;
	}
}



void cast_detect_invisibility( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			if ( affected_by_spell(tar_ch, SPELL_DETECT_INVISIBLE) ){
				send_to_char("Nothing seems to happen.\n\r", tar_ch);
				return;
			}
			spell_detect_invisibility(level,ch,tar_ch,0);
			break;
    case SPELL_TYPE_POTION:
			if ( affected_by_spell(ch, SPELL_DETECT_INVISIBLE) )
				return;
			spell_detect_invisibility(level,ch,ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               if(!(IS_AFFECTED(tar_ch, SPELL_DETECT_INVISIBLE)))
                  spell_detect_invisibility(level,ch,tar_ch,0);
         break;
    default :
	    slog("Serious screw-up in detect invisibility!");
         break;
	}
}



void cast_detect_magic( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			if ( affected_by_spell(tar_ch, SPELL_DETECT_MAGIC) ){
				send_to_char("Nothing seems to happen.\n\r", tar_ch);
				return;
			}
			spell_detect_magic(level,ch,tar_ch,0);
			break;
    case SPELL_TYPE_POTION:
			if ( affected_by_spell(ch, SPELL_DETECT_MAGIC) )
				return;
			spell_detect_magic(level,ch,ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               if (!(IS_AFFECTED(tar_ch, SPELL_DETECT_MAGIC)))
                  spell_detect_magic(level,ch,tar_ch,0);
         break;
    default :
	    slog("Serious screw-up in detect magic!");
         break;
	}
}



void cast_detect_poison( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_detect_poison(level, ch, tar_ch,tar_obj);
			break;
    case SPELL_TYPE_POTION:
			spell_detect_poison(level, ch, ch,0);
			break;
    case SPELL_TYPE_SCROLL:
         if (tar_obj) {
				spell_detect_poison(level, ch, 0, tar_obj);
            return;
         }
         if (!tar_ch) tar_ch = ch;
			spell_detect_poison(level, ch, tar_ch, 0);
			break;
    default :
	    slog("Serious screw-up in detect poison!");
         break;
	}
}



void cast_dispel_evil( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_dispel_evil(level, ch, tar_ch,0);
			break;
    case SPELL_TYPE_POTION:
			spell_dispel_evil(level,ch,ch,0);
			break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj) return;
      if (!tar_ch) tar_ch = ch;
			spell_dispel_evil(level, ch, tar_ch,0);
			break;
    case SPELL_TYPE_WAND:
      if (tar_obj) return;
			spell_dispel_evil(level, ch, tar_ch,0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
              spell_dispel_evil(level,ch,tar_ch,0);
         break;
    default :
	    slog("Serious screw-up in dispel evil!");
         break;
	}
}


void cast_enchant_weapon( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_enchant_weapon(level, ch, 0,tar_obj);
			break;

    case SPELL_TYPE_SCROLL:
			if(!tar_obj) return;
			spell_enchant_weapon(level, ch, 0,tar_obj);
			break;
    default :
	    slog("Serious screw-up in enchant weapon!");
      break;
	}
}


void cast_heal( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			act("$n heals $N.", FALSE, ch, 0, tar_ch, TO_NOTVICT);
			act("You heal $N.", FALSE, ch, 0, tar_ch, TO_CHAR);
			spell_heal(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_POTION:
         spell_heal(level, ch, ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
              spell_heal(level,ch,tar_ch,0);
         break;
    default :
	    slog("Serious screw-up in heal!");
         break;
	}
}


void cast_invisibility( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			if (tar_obj) {
				if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE) )
					send_to_char("Nothing new seems to happen.\n\r", ch);
				else
					spell_invisibility(level, ch, 0, tar_obj);
			} else { /* tar_ch */
				if ( IS_AFFECTED(tar_ch, AFF_INVISIBLE) )
					send_to_char("Nothing new seems to happen.\n\r", ch);
				else
					spell_invisibility(level, ch, tar_ch, 0);
			}
			break;
    case SPELL_TYPE_POTION:
         if (!IS_AFFECTED(ch, AFF_INVISIBLE) )
            spell_invisibility(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
			if (tar_obj) {
				if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)) )
					spell_invisibility(level, ch, 0, tar_obj);
			} else { /* tar_ch */
            if (!tar_ch) tar_ch = ch;

				if (!( IS_AFFECTED(tar_ch, AFF_INVISIBLE)) )
					spell_invisibility(level, ch, tar_ch, 0);
			}
			break;
    case SPELL_TYPE_WAND:
			if (tar_obj) {
				if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)) )
					spell_invisibility(level, ch, 0, tar_obj);
			} else { /* tar_ch */
				if (!( IS_AFFECTED(tar_ch, AFF_INVISIBLE)) )
					spell_invisibility(level, ch, tar_ch, 0);
			}
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
               if (!( IS_AFFECTED(tar_ch, AFF_INVISIBLE)) )
                  spell_invisibility(level,ch,tar_ch,0);
         break;
    default :
	    slog("Serious screw-up in invisibility!");
         break;
	}
}




void cast_locate_object( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_locate_object(level, ch, 0, tar_obj);
			break;
      default :
	      slog("Serious screw-up in locate object!");
         break;
	}
}


void cast_poison( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_poison(level, ch, tar_ch, tar_obj);
			break;
    case SPELL_TYPE_POTION:
			spell_poison(level, ch, ch, 0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_poison(level,ch,tar_ch,0);
         break;
    default :
	    slog("Serious screw-up in poison!");
         break;
	}
}


void cast_protection_from_evil( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_protection_from_evil(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_POTION:
         spell_protection_from_evil(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if(!tar_ch) tar_ch = ch;
			spell_protection_from_evil(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_protection_from_evil(level,ch,tar_ch,0);
         break;
    default :
	    slog("Serious screw-up in protection from evil!");
         break;
	}
}


void cast_remove_curse( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_remove_curse(level, ch, tar_ch, tar_obj);
			break;
    case SPELL_TYPE_POTION:
         spell_remove_curse(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) {
				spell_remove_curse(level, ch, 0, tar_obj);
 				return;
			}
         if(!tar_ch) tar_ch = ch;
			spell_remove_curse(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_remove_curse(level,ch,tar_ch,0);
         break;
    default :
	    slog("Serious screw-up in remove curse!");
         break;
	}
}



void cast_remove_poison( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_remove_poison(level, ch, tar_ch, tar_obj);
			break;
    case SPELL_TYPE_POTION:
         spell_remove_poison(level, ch, ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_remove_poison(level,ch,tar_ch,0);
         break;
    default :
	    slog("Serious screw-up in remove poison!");
         break;
	}
}



void cast_sanctuary( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_sanctuary(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_POTION:
         spell_sanctuary(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj)
 				return;
         if(!tar_ch) tar_ch = ch;
			spell_sanctuary(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_sanctuary(level,ch,tar_ch,0);
         break;
    default :
	    slog("Serious screw-up in sanctuary!");
         break;
	}
}


void cast_sleep( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_sleep(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_POTION:
			spell_sleep(level, ch, ch, 0);
			break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if (!tar_ch) tar_ch = ch;
         spell_sleep(level, ch, tar_ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(tar_obj) return;
         spell_sleep(level, ch, tar_ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_sleep(level,ch,tar_ch,0);
         break;
    default :
	    slog("Serious screw-up in sleep!");
         break;
	}
}


void cast_strength( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_strength(level, ch, tar_ch, 0);
			break;
    case SPELL_TYPE_POTION:
			spell_strength(level, ch, ch, 0);
			break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if (!tar_ch) tar_ch = ch;
         spell_strength(level, ch, tar_ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_strength(level,ch,tar_ch,0);
         break;
    default :
	    slog("Serious screw-up in strength!");
         break;
	}
}


void cast_ventriloquate( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
	struct char_data *tmp_ch;
	char buf1[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];

	if (type != SPELL_TYPE_SPELL) {
		slog("Attempt to ventriloquate by non-cast-spell.");
		return;
	}
	for(; *arg && (*arg == ' '); arg++);
	if (tar_obj) {
		sprintf(buf1, "The %s says '%s'\n\r", fname(tar_obj->name), arg);
		sprintf(buf2, "Someone makes it sound like the %s says '%s'.\n\r",
		  fname(tar_obj->name), arg);
	}	else {
		sprintf(buf1, "%s says '%s'\n\r", GET_NAME(tar_ch), arg);
		sprintf(buf2, "Someone makes it sound like %s says '%s'\n\r",
		  GET_NAME(tar_ch), arg);
	}

	sprintf(buf3, "Someone says, '%s'\n\r", arg);

	for (tmp_ch = world[ch->in_room].people; tmp_ch;
	  tmp_ch = tmp_ch->next_in_room) {

		if ((tmp_ch != ch) && (tmp_ch != tar_ch)) {
			if ( saves_spell(tmp_ch, SAVING_SPELL) )
				send_to_char(buf2, tmp_ch);
			else
				send_to_char(buf1, tmp_ch);
		} else {
			if (tmp_ch == tar_ch)
				send_to_char(buf3, tar_ch);
		}
	}
}



void cast_word_of_recall( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_word_of_recall(level, ch, ch, 0);
			break;
    case SPELL_TYPE_POTION:
			spell_word_of_recall(level, ch, ch, 0);
			break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if (!tar_ch) tar_ch = ch;
         spell_word_of_recall(level, ch, tar_ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(tar_obj) return;
         spell_word_of_recall(level, ch, tar_ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_word_of_recall(level,ch,tar_ch,0);
         break;
    default :
	    slog("Serious screw-up in word of recall!");
         break;
	}
}



void cast_summon( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_summon(level, ch, tar_ch, 0);
			break;
      default :
	      slog("Serious screw-up in summon!");
         break;
	}
}



void cast_charm_person( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
		case SPELL_TYPE_SPELL:
			spell_charm_person(level, ch, tar_ch, 0);
			break;
      case SPELL_TYPE_SCROLL:
         if(!tar_ch) return;
         spell_charm_person(level, ch, tar_ch, 0);
         break;
      case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_charm_person(level,ch,tar_ch,0);
         break;
      default :
	      slog("Serious screw-up in charm person!");
         break;
	}
}



void cast_sense_life( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
		case SPELL_TYPE_SPELL:
			spell_sense_life(level, ch, ch, 0);
			break;
      case SPELL_TYPE_POTION:
         spell_sense_life(level, ch, ch, 0);
         break;
      case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_sense_life(level,ch,tar_ch,0);
         break;
      default :
	      slog("Serious screw-up in sense life!");
         break;
	}
}


void cast_identify( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
		case SPELL_TYPE_SCROLL:
			spell_identify(level, ch, tar_ch, tar_obj);
			break;
		default :
			slog("Serious screw-up in identify!");
			break;
	}
}


void cast_fire_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_fire_breath(level, ch, tar_ch, 0);
			break;   /* It's a spell.. But people can't cast it! */
      default :
	      slog("Serious screw-up in firebreath!");
         break;
	}
}

void cast_frost_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_frost_breath(level, ch, tar_ch, 0);
			break;   /* It's a spell.. But people can't cast it! */
      default :
	      slog("Serious screw-up in frostbreath!");
         break;
	}
}

void cast_acid_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_acid_breath(level, ch, tar_ch, 0);
			break;   /* It's a spell.. But people can't cast it! */
      default :
	      slog("Serious screw-up in acidbreath!");
         break;
	}
}

void cast_gas_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
		for (tar_ch = world[ch->in_room].people ; 
				tar_ch ; tar_ch = tar_ch->next_in_room)
			if (tar_ch != ch) 
				spell_gas_breath(level,ch,tar_ch,0);
         break;
			/* THIS ONE HURTS!! */
      default :
	      slog("Serious screw-up in gasbreath!");
         break;
	}
}

void cast_lightning_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_lightning_breath(level, ch, tar_ch, 0);
			break;   /* It's a spell.. But people can't cast it! */
      default :
	      slog("Serious screw-up in lightningbreath!");
         break;
	}
}
