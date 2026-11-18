/* ************************************************************************
*  file: act.informative.c , Implementation of commands.  Part of DIKUMUD *
*  Usage : Informative commands.                                          *
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

/* extern variables */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern char credits[MAX_STRING_LENGTH];
extern char news[MAX_STRING_LENGTH];
extern char info[MAX_STRING_LENGTH];
extern char wizlist[MAX_STRING_LENGTH];
extern char *dirs[]; 
extern char *where[];
extern char *color_liquid[];
extern char *fullness[];

/* extern functions */

struct time_info_data age(struct char_data *ch);
void page_string(struct descriptor_data *d, char *str, int keep_internal);

/* intern functions */

void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode,
	bool show);


/* Procedures related to 'look' */

void argument_split_2(char *argument, char *first_arg, char *second_arg) {
	int look_at, found, begin;
	found = begin = 0;

	/* Find first non blank */
	for ( ;*(argument + begin ) == ' ' ; begin++);

	/* Find length of first word */
	for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)

	/* Make all letters lower case, AND copy them to first_arg */
	*(first_arg + look_at) = LOWER(*(argument + begin + look_at));
	*(first_arg + look_at) = '\0';
	begin += look_at;

	/* Find first non blank */
	for ( ;*(argument + begin ) == ' ' ; begin++);

	/* Find length of second word */
	for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

	/* Make all letters lower case, AND copy them to second_arg */
	*(second_arg + look_at) = LOWER(*(argument + begin + look_at));
	*(second_arg + look_at)='\0';
	begin += look_at;
}

struct obj_data *get_object_in_equip_vis(struct char_data *ch,
	char *arg, struct obj_data *equipment[], int *j) {

	for ((*j) = 0; (*j) < MAX_WEAR ; (*j)++)
		if (equipment[(*j)])
			if (CAN_SEE_OBJ(ch,equipment[(*j)]))
				if (isname(arg, equipment[(*j)]->name))
					return(equipment[(*j)]);

	return (0);
}

char *find_ex_description(char *word, struct extra_descr_data *list)
{
	struct extra_descr_data *i;

	for (i = list; i; i = i->next)
		if (isname(word,i->keyword))
			return(i->description);

	return(0);
}

void show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode)
{
	char buffer[MAX_STRING_LENGTH] = "\0";
	char *temp_desc;
	struct obj_data *i;
	int temp;
	bool found;

	if ((mode == 0) && object->description)
		strcpy(buffer,object->description);
	else 	if (object->short_description && ((mode == 1) ||
	      (mode == 2) || (mode==3) || (mode == 4))) 
		strcpy(buffer,object->short_description);
	else if (mode == 5) {
		if (object->obj_flags.type_flag == ITEM_NOTE)
		{
			if (object->action_description)
			{
				strcpy(buffer, "There is something written upon it:\n\r\n\r");
				strcat(buffer, object->action_description);
				page_string(ch->desc, buffer, 1);
			}
			else
				act("It's blank.", FALSE, ch,0,0,TO_CHAR);
			return;
		}
		else if((object->obj_flags.type_flag != ITEM_DRINKCON))
		{
			strcpy(buffer,"You see nothing special..");
		}
		else /* ITEM_TYPE == ITEM_DRINKCON */
		{
			strcpy(buffer, "It looks like a drink container.");
		}
	}

	if (mode != 3) { 
		found = FALSE;
		if (IS_OBJ_STAT(object,ITEM_INVISIBLE)) {
			 strcat(buffer,"(invisible)");
			 found = TRUE;
		}
		if (IS_OBJ_STAT(object,ITEM_EVIL) && IS_AFFECTED(ch,AFF_DETECT_EVIL)) {
			 strcat(buffer,"..It glows red!");
			 found = TRUE;
		}
		if (IS_OBJ_STAT(object,ITEM_MAGIC) && IS_AFFECTED(ch,AFF_DETECT_MAGIC)) {
			 strcat(buffer,"..It glows blue!");
			 found = TRUE;
		}
		if (IS_OBJ_STAT(object,ITEM_GLOW)) {
			strcat(buffer,"..It has a soft glowing aura!");
			found = TRUE;
		}
		if (IS_OBJ_STAT(object,ITEM_HUM)) {
			strcat(buffer,"..It emits a faint humming sound!");
			found = TRUE;
		}
	}

	strcat(buffer, "\n\r");
	page_string(ch->desc, buffer, 1);

/*
	if (((mode == 2) || (mode == 4)) && (GET_ITEM_TYPE(object) == 
		ITEM_CONTAINER)) {
		strcpy(buffer,"The ");
		strcat(buffer,fname(object->name));
		strcat(buffer," contains:\n\r");
		send_to_char(buffer, ch);
		if (mode == 2) list_obj_to_char(object->contains, ch, 1,TRUE);
		if (mode == 4) list_obj_to_char(object->contains, ch, 3,TRUE);
	}
*/
}

void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode, 
	bool show) {
	struct obj_data *i;
	bool found;

	found = FALSE;
	for ( i = list ; i ; i = i->next_content ) { 
		if (CAN_SEE_OBJ(ch,i)) {
			show_obj_to_char(i, ch, mode);
			found = TRUE;
		}    
	}  
	if ((! found) && (show)) send_to_char("Nothing\n\r", ch);
}



void show_char_to_char(struct char_data *i, struct char_data *ch, int mode)
{
	char buffer[MAX_STRING_LENGTH];
	int j, found, percent;
	struct obj_data *tmp_obj;

	if (mode == 0) {

		if (IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch,i)) {
			if (IS_AFFECTED(ch, AFF_SENSE_LIFE))
				send_to_char("You sense a hidden life form in the room.\n\r", ch);
			return;
		}

		if (!(i->player.long_descr)||(GET_POS(i) != i->specials.default_pos)){
			/* A player char or a mobile without long descr, or not in default pos. */
			if (!IS_NPC(i)) {	
				strcpy(buffer,GET_NAME(i));
				strcat(buffer," ");
				strcat(buffer,GET_TITLE(i));
			} else {
				strcpy(buffer, i->player.short_descr);
				CAP(buffer);
			}

			if ( IS_AFFECTED(i,AFF_INVISIBLE))
			   strcat(buffer," (invisible)");

			switch(GET_POS(i)) {
				case POSITION_STUNNED  : 
					strcat(buffer," is lying here, stunned."); break;
				case POSITION_INCAP    : 
					strcat(buffer," is lying here, incapacitated."); break;
				case POSITION_MORTALLYW: 
					strcat(buffer," is lying here, mortally wounded."); break;
				case POSITION_DEAD     : 
					strcat(buffer," is lying here, dead."); break;
				case POSITION_STANDING : 
					strcat(buffer," is standing here."); break;
				case POSITION_SITTING  : 
					strcat(buffer," is sitting here.");  break;
				case POSITION_RESTING  : 
					strcat(buffer," is resting here.");  break;
				case POSITION_SLEEPING : 
					strcat(buffer," is sleeping here."); break;
				case POSITION_FIGHTING :
					if (i->specials.fighting) {

						strcat(buffer," is here, fighting ");
						if (i->specials.fighting == ch)
							strcat(buffer," YOU!");
						else {
							if (i->in_room == i->specials.fighting->in_room)
								if (IS_NPC(i->specials.fighting))
									strcat(buffer, i->specials.fighting->player.short_descr);
								else
									strcat(buffer, GET_NAME(i->specials.fighting));
							else
								strcat(buffer, "someone who has already left.");
						}
					} else /* NIL fighting pointer */
							strcat(buffer," is here struggling with thin air.");
					break;
				default : strcat(buffer," is floating here."); break;
			}
			if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
				if (IS_EVIL(i))
					strcat(buffer, " (Red Aura)");
			}

			strcat(buffer,"\n\r");
			send_to_char(buffer, ch);
		}
		else  /* npc with long */
		{
			if (IS_AFFECTED(i,AFF_INVISIBLE))
				strcpy(buffer,"*");
			else
				*buffer = '\0';

			if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
				if (IS_EVIL(i))
					strcat(buffer, " (Red Aura)");
			}

			strcat(buffer, i->player.long_descr);

			send_to_char(buffer, ch);
		}
							
		if (IS_AFFECTED(i,AFF_SANCTUARY))
			act("$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);

	} else if (mode == 1) {

		if (i->player.description)
			send_to_char(i->player.description, ch);
		else {
			act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
		}

		/* Show a character to another */

		if (GET_MAX_HIT(i) > 0)
			percent = (100*GET_HIT(i))/GET_MAX_HIT(i);
		else
			percent = -1; /* How could MAX_HIT be < 1?? */

		if (IS_NPC(i))
			strcpy(buffer, i->player.short_descr);
		else
			strcpy(buffer, GET_NAME(i));

		if (percent >= 100)
			strcat(buffer, " is in an excellent condition.\n\r");
		else if (percent >= 90)
			strcat(buffer, " has a few scratches.\n\r");
		else if (percent >= 75)
			strcat(buffer, " has some small wounds and bruises.\n\r");
		else if (percent >= 50)
			strcat(buffer, " has quite a few wounds.\n\r");
		else if (percent >= 30)
			strcat(buffer, " has some big nasty wounds and scratches.\n\r");
		else if (percent >= 15)
			strcat(buffer, " looks pretty hurt.\n\r");
		else if (percent >= 0)
			strcat(buffer, " is in an awful condition.\n\r");
		else
			strcat(buffer, " is bleeding awfully from big wounds.\n\r");

		send_to_char(buffer, ch);

		found = FALSE;
		for (j=0; j< MAX_WEAR; j++) {
			if (i->equipment[j]) {
				if (CAN_SEE_OBJ(ch,i->equipment[j])) {
					found = TRUE;
				}
			}
		}
		if (found) {
			act("\n\r$n is using:", FALSE, i, 0, ch, TO_VICT);
			for (j=0; j< MAX_WEAR; j++) {
				if (i->equipment[j]) {
					if (CAN_SEE_OBJ(ch,i->equipment[j])) {
						send_to_char(where[j],ch);
						show_obj_to_char(i->equipment[j],ch,1);
					}
				}
			}
		}
		if ((GET_CLASS(ch) == CLASS_THIEF) && (ch != i)) {
			found = FALSE;
			send_to_char("\n\rYou attempt to peek at the inventory:\n\r", ch);
			for(tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
				if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0,20) < GET_LEVEL(ch))) {
					show_obj_to_char(tmp_obj, ch, 1);
					found = TRUE;
				}
			}
			if (!found)
				send_to_char("You can't see anything.\n\r", ch);
		}

	} else if (mode == 2) {

		/* Lists inventory */
		act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
		list_obj_to_char(i->carrying,ch,1,TRUE);
	}
}



void list_char_to_char(struct char_data *list, struct char_data *ch, 
	int mode) {
	struct char_data *i;

	for (i = list; i ; i = i->next_in_room) {
		if ( (ch!=i) && (IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
		     (CAN_SEE(ch,i) && !IS_AFFECTED(i, AFF_HIDE))) )
			show_char_to_char(i,ch,0); 
	} 
}



void do_look(struct char_data *ch, char *argument, int cmd)
{
	char buffer[MAX_STRING_LENGTH];
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	int keyword_no;
	int j, bits, temp;
	bool found;
	struct obj_data *tmp_object, *found_object;
	struct char_data *tmp_char;
	char *tmp_desc;
	static char *keywords[]= { 
		"north",
		"east",
		"south",
		"west",
		"up",
		"down",
		"in",
		"at",
		"",  /* Look at '' case */
		"\n" };

	if (!ch->desc)
		return;

	if (GET_POS(ch) < POSITION_SLEEPING)
		send_to_char("You can't see anything but stars!\n\r", ch);
	else if (GET_POS(ch) == POSITION_SLEEPING)
		send_to_char("You can't see anything, you're sleeping!\n\r", ch);
	else if ( IS_AFFECTED(ch, AFF_BLIND) )
		send_to_char("You can't see a damn thing, you're blinded!\n\r", ch);
	else if ( IS_DARK(ch->in_room) )
		send_to_char("It is pitch black...\n\r", ch);
	else {
		argument_split_2(argument,arg1,arg2);
		keyword_no = search_block(arg1, keywords, FALSE); /* Partiel Match */

		if ((keyword_no == -1) && *arg1) {
			keyword_no = 7;
			strcpy(arg2, arg1); /* Let arg2 become the target object (arg1) */
		}

		found = FALSE;
		tmp_object = 0;
		tmp_char	 = 0;
		tmp_desc	 = 0;

		switch(keyword_no) {
			/* look <dir> */
			case 0 :
			case 1 :
			case 2 : 
			case 3 : 
			case 4 :
			case 5 : {   

				if (EXIT(ch, keyword_no)) {

					if (EXIT(ch, keyword_no)->general_description) {
						send_to_char(EXIT(ch, keyword_no)->
							general_description, ch);
					} else {
						send_to_char("You see nothing special.\n\r", ch);
					}

					if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_CLOSED) && 
						(EXIT(ch, keyword_no)->keyword)) {
							sprintf(buffer, "The %s is closed.\n\r",
								fname(EXIT(ch, keyword_no)->keyword));
							send_to_char(buffer, ch);
					}	else {
						if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_ISDOOR) &&
						    EXIT(ch, keyword_no)->keyword) {
							sprintf(buffer, "The %s is open.\n\r",
								fname(EXIT(ch, keyword_no)->keyword));
							send_to_char(buffer, ch);
						}
					}
				} else {
						send_to_char("Nothing special there...\n\r", ch);
				}
			}
			break;

			/* look 'in'	*/
			case 6: {
				if (*arg2) {
					/* Item carried */

					bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
					         FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

					if (bits) { /* Found something */
						if (GET_ITEM_TYPE(tmp_object)== ITEM_DRINKCON)
						{
							if (tmp_object->obj_flags.value[1] <= 0) {
								act("It is empty.", FALSE, ch, 0, 0, TO_CHAR);
							} else {
								temp=((tmp_object->obj_flags.value[1]*3)/tmp_object->obj_flags.value[0]);
								sprintf(buffer,"It's %sfull of a %s liquid.\n\r",
								fullness[temp],color_liquid[tmp_object->obj_flags.value[2]]);
								send_to_char(buffer, ch);
							}
						} else if (GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER) {
							if (!IS_SET(tmp_object->obj_flags.value[1],CONT_CLOSED)) {
								send_to_char(fname(tmp_object->name), ch);
								switch (bits) {
									case FIND_OBJ_INV :
										send_to_char(" (carried) : \n\r", ch);
										break;
									case FIND_OBJ_ROOM :
										send_to_char(" (here) : \n\r", ch);
										break;
									case FIND_OBJ_EQUIP :
										send_to_char(" (used) : \n\r", ch);
										break;
								}
								list_obj_to_char(tmp_object->contains, ch, 2, TRUE);
							}
							else
								send_to_char("It is closed.\n\r", ch);
						} else {
							send_to_char("That is not a container.\n\r", ch);
						}
					} else { /* wrong argument */
						send_to_char("You do not see that item here.\n\r", ch);
					}
				} else { /* no argument */
					send_to_char("Look in what?!\n\r", ch);
				}
			}
			break;

			/* look 'at'	*/
			case 7 : {


				if (*arg2) {

					bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
					       FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &tmp_char, &found_object);

					if (tmp_char) {
						show_char_to_char(tmp_char, ch, 1);
						if (ch != tmp_char) {
							act("$n looks at you.", TRUE, ch, 0, tmp_char, TO_VICT);
							act("$n looks at $N.", TRUE, ch, 0, tmp_char, TO_NOTVICT);
						}
						return;
					}


					/* Search for Extra Descriptions in room and items */

					/* Extra description in room?? */
					if (!found) {
						tmp_desc = find_ex_description(arg2, 
							world[ch->in_room].ex_description);
						if (tmp_desc) {
							page_string(ch->desc, tmp_desc, 0);
							return; /* RETURN SINCE IT WAS A ROOM DESCRIPTION */
							/* Old system was: found = TRUE; */
						}
					}

					/* Search for extra descriptions in items */

					/* Equipment Used */

					if (!found) {
						for (j = 0; j< MAX_WEAR && !found; j++) {
							if (ch->equipment[j]) {
								if (CAN_SEE_OBJ(ch,ch->equipment[j])) {
									tmp_desc = find_ex_description(arg2, 
										ch->equipment[j]->ex_description);
									if (tmp_desc) {
										page_string(ch->desc, tmp_desc, 1);
										found = TRUE;
									}
								}
							}
						}
					}

					/* In inventory */

					if (!found) {
						for(tmp_object = ch->carrying; 
							tmp_object && !found; 
							tmp_object = tmp_object->next_content) {
							if CAN_SEE_OBJ(ch, tmp_object) {
								tmp_desc = find_ex_description(arg2, 
									tmp_object->ex_description);
								if (tmp_desc) {
									page_string(ch->desc, tmp_desc, 1);
									found = TRUE;
								}
							}
						}
					}

					/* Object In room */

					if (!found) {
						for(tmp_object = world[ch->in_room].contents; 
							tmp_object && !found; 
							tmp_object = tmp_object->next_content) {
							if CAN_SEE_OBJ(ch, tmp_object) {
								tmp_desc = find_ex_description(arg2, 
									tmp_object->ex_description);
								if (tmp_desc) {
									page_string(ch->desc, tmp_desc, 1);
									found = TRUE;
								}
							}
						}
					}
					/* wrong argument */

					if (bits) { /* If an object was found */
						if (!found)
							show_obj_to_char(found_object, ch, 5); /* Show no-description */
						else
							show_obj_to_char(found_object, ch, 6); /* Find hum, glow etc */
					} else if (!found) {
						send_to_char("You do not see that here.\n\r", ch);
					}
				} else {
					/* no argument */

					send_to_char("Look at what?\n\r", ch);
				}
			}
			break;


			/* look ''		*/ 
			case 8 : {

				send_to_char(world[ch->in_room].name, ch);
				send_to_char("\n\r", ch);

				if (!IS_SET(ch->specials.act, PLR_BRIEF))
					send_to_char(world[ch->in_room].description, ch);

				list_obj_to_char(world[ch->in_room].contents, ch, 0,FALSE);

				list_char_to_char(world[ch->in_room].people, ch, 0);
			}
			break;

			/* wrong arg	*/
			case -1 : 
				send_to_char("Sorry, I didn't understand that!\n\r", ch);
				break;
		}
	}
}

/* end of look */




void do_read(struct char_data *ch, char *argument, int cmd)
{
	char buf[100];

	/* This is just for now - To be changed later.! */
	sprintf(buf,"at %s",argument);
	do_look(ch,buf,15);
}



void do_examine(struct char_data *ch, char *argument, int cmd)
{
	char name[100], buf[100];
	int bits;
	struct char_data *tmp_char;
	struct obj_data *tmp_object;

	sprintf(buf,"at %s",argument);
	do_look(ch,buf,15);

	one_argument(argument, name);

	if (!*name)
	{
		send_to_char("Examine what?\n\r", ch);
		return;
	}

	bits = generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM |
	       FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

	if (tmp_object) {
		if ((GET_ITEM_TYPE(tmp_object)==ITEM_DRINKCON) ||
		    (GET_ITEM_TYPE(tmp_object)==ITEM_CONTAINER)) {
			send_to_char("When you look inside, you see:\n\r", ch);
			sprintf(buf,"in %s",argument);
			do_look(ch,buf,15);
		}
	}
}



void do_exits(struct char_data *ch, char *argument, int cmd)
{
	int door;
	char buf[MAX_STRING_LENGTH];
	char *exits[] =
	{	
		"North",
		"East ",
		"South",
		"West ",
		"Up   ",
		"Down "
	};

	*buf = '\0';

	for (door = 0; door <= 5; door++)
		if (EXIT(ch, door))
			if (EXIT(ch, door)->to_room != NOWHERE &&
			    !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
				if (IS_DARK(EXIT(ch, door)->to_room))
					sprintf(buf + strlen(buf), "%s - Too dark to tell\n\r", exits[door]);
				else
					sprintf(buf + strlen(buf), "%s - %s\n\r", exits[door],
						world[EXIT(ch, door)->to_room].name);

	send_to_char("Obvious exits:\n\r", ch);

	if (*buf)
		send_to_char(buf, ch);
	else
		send_to_char("None.\n\r", ch);
}


void do_score(struct char_data *ch, char *argument, int cmd)
{
	struct time_info_data playing_time;
	static char buf[100];

	struct time_info_data real_time_passed(time_t t2, time_t t1);

	sprintf(buf, "You are %d years old.", GET_AGE(ch));

	if ((age(ch).month == 0) && (age(ch).day == 0))
		strcat(buf," It's your birthday today.\n\r");
	else
		strcat(buf,"\n\r");
	send_to_char(buf, ch);

	if (GET_COND(ch,DRUNK)>10)
		send_to_char("You are intoxicated.\n\r", ch);
	if (!GET_COND(ch,THIRST))
		send_to_char("You are thirsty.\n\r", ch);
	if (!GET_COND(ch,FULL))
		send_to_char("You are hungry.\n\r", ch);

	sprintf(buf, 
		"You have %d(%d) hit, %d(%d) mana and %d(%d) movement points.\n\r",
		GET_HIT(ch),GET_MAX_HIT(ch),
		GET_MANA(ch),GET_MAX_MANA(ch),
		GET_MOVE(ch),GET_MAX_MOVE(ch));
	send_to_char(buf,ch);

	sprintf(buf,"You have scored %d exp, and have %d gold coins.\n\r",
		GET_EXP(ch),GET_GOLD(ch));
	send_to_char(buf,ch);

	playing_time = real_time_passed((time(0)-ch->player.time.logon) +
	   ch->player.time.played, 0);
	sprintf(buf,"You have been playing for %d days and %d hours.\n\r",
		playing_time.day,
		playing_time.hours);		
	send_to_char(buf, ch);		

	sprintf(buf,"This ranks you as %s %s (level %d).\n\r",
		GET_NAME(ch),
		GET_TITLE(ch), GET_LEVEL(ch) );
	send_to_char(buf,ch);

	switch(GET_POS(ch)) {
		case POSITION_DEAD : 
			send_to_char("You are DEAD!\n\r", ch); break;
		case POSITION_MORTALLYW :
			send_to_char("You are mortally wounded!, you should seek help!\n\r", ch); break;
		case POSITION_INCAP : 
			send_to_char("You are incapacitated, slowly fading away\n\r", ch); break;
		case POSITION_STUNNED : 
			send_to_char("You are stunned! You can't move\n\r", ch); break;
		case POSITION_SLEEPING : 
			send_to_char("You are sleeping.\n\r",ch); break;
		case POSITION_RESTING  : 
			send_to_char("You are resting.\n\r",ch); break;
		case POSITION_SITTING  : 
			send_to_char("You are sitting.\n\r",ch); break;
		case POSITION_FIGHTING :
			if (ch->specials.fighting)
				act("You are fighting $N.\n\r", FALSE, ch, 0,
				     ch->specials.fighting, TO_CHAR);
			else
				send_to_char("You are fighting thin air.\n\r", ch);
			break;
		case POSITION_STANDING : 
			send_to_char("You are standing.\n\r",ch); break;
		default :
			send_to_char("You are floating.\n\r",ch); break;
	}

}


void do_time(struct char_data *ch, char *argument, int cmd)
{
	char buf[100], *suf;
	int weekday, day;
	extern struct time_info_data time_info;
	extern const char *weekdays[];
	extern const char *month_name[];

	sprintf(buf, "It is %d o'clock %s, on ",
		((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
		((time_info.hours >= 12) ? "pm" : "am") );

	weekday = ((35*time_info.month)+time_info.day+1) % 7;/* 35 days in a month */

	strcat(buf,weekdays[weekday]);
	strcat(buf,"\n\r");
	send_to_char(buf,ch);

	day = time_info.day + 1;   /* day in [1..35] */

	if (day == 1)
		suf = "st";
	else if (day == 2)
		suf = "nd";
	else if (day == 3)
		suf = "rd";
	else if (day < 20)
		suf = "th";
	else if ((day % 10) == 1)
		suf = "st";
	else if ((day % 10) == 2)
		suf = "nd";
	else if ((day % 10) == 3)
		suf = "rd";
	else
		suf = "th";

	sprintf(buf, "The %d%s Day of the %s, Year %d.\n\r",
		day,
		suf,
		month_name[time_info.month],
		time_info.year);

	send_to_char(buf,ch);
}


void do_weather(struct char_data *ch, char *argument, int cmd)
{
	extern struct weather_data weather_info;
	static char buf[100];
	char static *sky_look[4]= {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"};

	if (OUTSIDE(ch)) {
		sprintf(buf, 
		"The sky is %s and %s.\n\r",
			sky_look[weather_info.sky],
			(weather_info.change >=0 ? "you feel a warm wind from south" :
	     "your foot tells you bad weather is due"));
		send_to_char(buf,ch);
	} else
		send_to_char("You have no feeling about the weather at all.\n\r", ch);
}


void do_help(struct char_data *ch, char *argument, int cmd)
{
	extern char *spells[];   /* The list of spells (spells.c)         */
	extern int top_of_helpt;
	extern struct help_index_element *help_index;
	extern FILE *help_fl;
	extern char help[MAX_STRING_LENGTH];

	int i, no, chk, bot, top, mid, minlen;
	char buf[MAX_STRING_LENGTH], buffer[MAX_STRING_LENGTH];


	if (!ch->desc)
		return;

	for(;isspace(*argument); argument++)  ;


	if (*argument)
	{
		if (!help_index)
		{
			send_to_char("No help available.\n\r", ch);
			return;
		}
		bot = 0;
		top = top_of_helpt;

		for (;;)
		{
			mid = (bot + top) / 2;
			minlen = strlen(argument);

			if (!(chk = strncmp(argument, help_index[mid].keyword, minlen)))
			{
				fseek(help_fl, help_index[mid].pos, 0);
				*buffer = '\0';
				for (;;)
				{
					fgets(buf, 80, help_fl);
					if (*buf == '#')
						break;
					strcat(buffer, buf);
					strcat(buffer, "\r");
				}
				page_string(ch->desc, buffer, 1);
				return;
			}
			else if (bot >= top)
			{
				send_to_char("There is no help on that word.\n\r", ch);
				return;
			}
			else if (chk > 0)
				bot = ++mid;
			else
				top = --mid;
		}
		return;
	}


	send_to_char(help, ch);

}





void do_wizhelp(struct char_data *ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	int no, i;
	extern char *command[];	 /* The list of commands (interpreter.c)	*/
	                         /* First command is command[0]           */
	extern struct command_info cmd_info[];
	                         /* cmd_info[1] ~~ commando[0]            */

	if (IS_NPC(ch))
		return;

	send_to_char("The following privileged comands are available:\n\r\n\r", ch);

	*buf = '\0';

	for (no = 1, i = 0; *command[i] != '\n'; i++)
		if ((GET_LEVEL(ch) >= cmd_info[i+1].minimum_level) &&
			(cmd_info[i+1].minimum_level >= 21) && (i != 217))
		{

			sprintf(buf + strlen(buf), "%-10s", command[i]);
			if (!(no % 7))
				strcat(buf, "\n\r");
			no++;
		}
	strcat(buf, "\n\r");
	page_string(ch->desc, buf, 1);
}




void do_who(struct char_data *ch, char *argument, int cmd)
{
	struct descriptor_data *d;
	char buf[256];

	send_to_char("Players\n\r-------\n\r", ch);
	for (d = descriptor_list; d; d = d->next)
	{
		if ((!d->connected) &&
		    (CAN_SEE(ch, d->character) || (GET_LEVEL(ch) >= 23)))
		{

			if(d->original) /* If switched */
				sprintf(buf, "%s %s\n\r", 
				   GET_NAME(d->original),
				  	d->original->player.title);
			else
				sprintf(buf, "%s %s\n\r", 
				   GET_NAME(d->character),
				   d->character->player.title);

		send_to_char(buf, ch);
		}
	}
}




void do_users(struct char_data *ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH], line[200];

	struct descriptor_data *d;

	strcpy(buf, "Connections:\n\r------------\n\r");
	
	for (d = descriptor_list; d; d = d->next)
	{
		if (d->character && d->character->player.name)
		{
		if(d->original)
			sprintf(line, "%-16s: ", d->original->player.name);
		else
			sprintf(line, "%-16s: ", d->character->player.name);
		}
		else
			strcpy(line, "UNDEFINED       : ");
		if (*(d->host))
			sprintf(line + strlen(line), "[%s]\n\r", d->host);
		else
			strcat(line, "[Hostname unknown]\n\r");

		strcat(buf, line);
	}
	send_to_char(buf, ch);
}



void do_inventory(struct char_data *ch, char *argument, int cmd) {

	send_to_char("You are carrying:\n\r", ch);
	list_obj_to_char(ch->carrying, ch, 1, TRUE);
}


void do_equipment(struct char_data *ch, char *argument, int cmd) {
int j;
bool found;

	send_to_char("You are using:\n\r", ch);
	found = FALSE;
	for (j=0; j< MAX_WEAR; j++) {
		if (ch->equipment[j]) {
			if (CAN_SEE_OBJ(ch,ch->equipment[j])) {
				send_to_char(where[j],ch);
				show_obj_to_char(ch->equipment[j],ch,1);
				found = TRUE;
			} else {
				send_to_char(where[j],ch);
				send_to_char("Something.\n\r",ch);
				found = TRUE;
			}
		}
	}
	if(!found) {
		send_to_char(" Nothing.\n\r", ch);
	}
}


void do_credits(struct char_data *ch, char *argument, int cmd) {

	page_string(ch->desc, credits, 0);
}


void do_news(struct char_data *ch, char *argument, int cmd) {

	page_string(ch->desc, news, 0);
}


void do_info(struct char_data *ch, char *argument, int cmd) {

	page_string(ch->desc, info, 0);
}


void do_wizlist(struct char_data *ch, char *argument, int cmd) {

	page_string(ch->desc, wizlist, 0);
}



void do_where(struct char_data *ch, char *argument, int cmd)
{
	char name[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH], buf2[256];
	register struct char_data *i;
	register struct obj_data *k;
	struct descriptor_data *d;

	one_argument(argument, name);

	if (!*name) {
		if (GET_LEVEL(ch) < 21)
		{
			send_to_char("What are you looking for?\n\r", ch);
			return;
		}
		else
		{
			strcpy(buf, "Players:\n\r--------\n\r");
		
			for (d = descriptor_list; d; d = d->next) {
				if (d->character && (d->connected == CON_PLYNG) && (d->character->in_room != NOWHERE)) {
					if (d->original)   /* If switched */
						sprintf(buf, "%-20s - %s [%d] In body of %s\n\r",
						  d->original->player.name,
						  world[d->character->in_room].name,
						  world[d->character->in_room].number,
						  fname(d->character->player.name));
					else
						sprintf(buf, "%-20s - %s [%d]\n\r",
						  d->character->player.name,
						  world[d->character->in_room].name,
						  world[d->character->in_room].number);
						 
					send_to_char(buf, ch);
				}
			}
			return;
		}
	}

	*buf = '\0';

	for (i = character_list; i; i = i->next)
		if (isname(name, i->player.name) && CAN_SEE(ch, i) )
		{
			if ((i->in_room != NOWHERE) && ((GET_LEVEL(ch)>20) ||
			    (world[i->in_room].zone == world[ch->in_room].zone))) {

				if (IS_NPC(i))
					sprintf(buf, "%-30s- %s ", i->player.short_descr,
						world[i->in_room].name);
				else
					sprintf(buf, "%-30s- %s ", i->player.name,
						world[i->in_room].name);

				if (GET_LEVEL(ch) >= 21)
					sprintf(buf2,"[%d]\n\r", world[i->in_room].number);
				else
					strcpy(buf2, "\n\r");

				strcat(buf, buf2);
				send_to_char(buf, ch);

				if (GET_LEVEL(ch) < 21)
					break;
			}
		}

	if (GET_LEVEL(ch) > 20) {
		for (k = object_list; k; k = k->next)
			if (isname(name, k->name) && CAN_SEE_OBJ(ch, k) && 
				(k->in_room != NOWHERE)) {
					sprintf(buf, "%-30s- %s [%d]\n\r",
						k->short_description,
						world[k->in_room].name,
						world[k->in_room].number);
						send_to_char(buf, ch);
				}
	}

	if (!*buf)
		send_to_char("Couldn't find any such thing.\n\r", ch);
}




void do_levels(struct char_data *ch, char *argument, int cmd)
{
	int i;
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int class;

	extern const struct title_type titles[4][25];

	if (IS_NPC(ch))
	{
		send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
		return;
	}
	class = GET_CLASS(ch);

	one_argument(argument,arg);

	if (*arg) {
		if (!strcasecmp(arg,"magic")) class = 1;
		else if (!strcasecmp(arg,"cleric")) class =  2;
		else if (!strcasecmp(arg,"thief")) class =  3;
		else if (!strcasecmp(arg,"fighter")) class = 4;
	}

	*buf = '\0';
	
	for (i = 1; i < 21; i++)
	{
		sprintf(buf + strlen(buf), "%7d-%-7d : ",
			titles[class - 1][i].exp,
			titles[class - 1][i + 1].exp);
		switch(GET_SEX(ch))
		{
			case SEX_MALE:
				strcat(buf, titles[class - 1][i].title_m); break;
			case SEX_FEMALE:
				strcat(buf, titles[class - 1][i].title_f); break;
			default:
				send_to_char("Oh dear.\n\r", ch); break;
		}
		strcat(buf, "\n\r");
	}
	send_to_char(buf, ch);
}



void do_consider(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim;
	char name[256], buf[256];
	int diff;

	one_argument(argument, name);

	if (!(victim = get_char_room_vis(ch, name))) {
		send_to_char("Consider killing who?\n\r", ch);
		return;
	}

	if (victim == ch) {
		send_to_char("Easy! Very easy indeed!\n\r", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		send_to_char("Would you like to borrow a cross and a shovel?\n\r", ch);
		return;
	}

	diff = (GET_LEVEL(victim)-GET_LEVEL(ch));

	if (diff <= -10)
		send_to_char("Now where did that chicken go?\n\r", ch);
	else if (diff <= -5)
		send_to_char("You could do it with a needle!\n\r", ch);
	else if (diff <= -2)
		send_to_char("Easy.\n\r", ch);
	else if (diff <= -1)
		send_to_char("Fairly easy.\n\r", ch);
	else if (diff == 0)
		send_to_char("The perfect match!\n\r", ch);
	else if (diff <= 1)
		send_to_char("You would need some luck!\n\r", ch);
	else if (diff <= 2)
		send_to_char("You would need a lot of luck!\n\r", ch);
	else if (diff <= 3)
		send_to_char("You would need a lot of luck and great equipment!\n\r", ch);
	else if (diff <= 5)
		send_to_char("Do you feel lucky, punk?\n\r", ch);
	else if (diff <= 10)
		send_to_char("Are you mad!?\n\r", ch);
	else if (diff <= 100)
		send_to_char("You ARE mad!\n\r", ch);

}
