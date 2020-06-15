#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "utils.h"
#include "structs.h"
#include "comm.h"


#define MAX_MSGS 50	               /* Max number of messages.          */
#define SAVE_FILE "board.messages"	/* Name of file for saving messages */
#define BOARD_OBJECT  xxx           /* what are we?                     */
#define BOARD_ROOM    xxx           /* where are we?                    */
#define MAX_MESSAGE_LENGTH 4000     /* that should be enough            */

char *msgs[MAX_MSGS];
char *head[MAX_MSGS];
int msg_num;

void board_write_msg(struct char_data *ch, char *arg);
int board_display_msg(struct char_data *ch, char *arg);
int board_remove_msg(struct char_data *ch, char *arg);
void board_save_board();
void board_load_board();
void board_reset_board();
void error_log();
void board_fix_long_desc(int num, char *headers[MAX_MSGS]);
int board_show_board(struct char_data *ch, char *arg);

/* I have used cmd number 180-182 as the cmd numbers here. */
/* The commands would be, in order : NOTE <header>         */
/* READ <message number>, REMOVE <message number>          */
/* LOOK AT BOARD should give the long desc of the board    */
/* and that should equal a list of message numbers and     */
/* headers. This is done by calling a function that sets   */
/* the long desc in the board object. This function is     */
/* called when someone does a REMOVE or NOTE command.      */
/* I have named the function board_fix_long_desc(). In the */
/* board_write_msg() function there is a part that should  */
/* be replaced with a call to some dreadful routine used   */
/* by the STRING command to receive player input. It is    */
/* reputed to lurk somewhere within the limits of an evil  */
/* file named act.comm.c.....*/

/* saving the board after the addition of a new messg      */
/* poses a slight problem, since the text isn't actually   */
/* entered in board_write. What I'll do is to let board()  */
/* save the first time a 'look' is issued in the room..    */
/* ugh! that's ugly - gotta think of something better.     */
/* -quinn                                                  */

/* And here is the board...correct me if I'm wrong. */


int board(struct char_data *ch, int cmd, char *arg)
{
	static int has_loaded = 0;

	if (!ch->desc)
		return(FALSE); /* By MS or all NPC's will be trapped at the board */

	/* note: I'll let display and remove return 0 if the arg was non-board- */
	/* related. Thus, it'll be possible to read other things than the board */
	/* while you're in the room. Conceiveably, you could do this for write, */
	/* too, but I'm not in the mood for such hacking.                       */

	if (!has_loaded)
	{
		board_load_board();
		has_loaded = 1;
	}

	switch (cmd) {
		case 15:  /* look */
			return(board_show_board(ch, arg));
		case 149: /* write */
			board_write_msg(ch, arg);
		return 1;
		case 63: /* read */
			return(board_display_msg(ch, arg));
		case 66: /* remove */
			return(board_remove_msg(ch, arg));
		default:
			return 0;
	}
}


void board_write_msg(struct char_data *ch, char *arg) {
	if (msg_num > MAX_MSGS - 1) {
		send_to_char("The board is full already.\n\r", ch);
		return;
	}
	/* skip blanks */
	for(; isspace(*arg); arg++);
	if (!*arg) {
		send_to_char("We must have a headline!\n\r", ch);
		return;
	}
	head[msg_num] = (char *)malloc(strlen(arg) + strlen(GET_NAME(ch)) + 4);
	/* +4 is for a space and '()' around the character name. */
	if (!head[msg_num]) {
		error_log("Malloc for board header failed.\n\r");
		send_to_char("The board is malfunctioning - sorry.\n\r", ch);
		return;
	}
	strcpy(head[msg_num], arg);
	/* Is this clumsy?  - four strcat() in a row I mean..*/
	strcat(head[msg_num], " (");
	strcat(head[msg_num], GET_NAME(ch));
	strcat(head[msg_num], ")");
	msgs[msg_num] = NULL;

	send_to_char("Write your message. Terminate with a @.\n\r\n\r", ch);
	act("$n starts to write a message.", TRUE, ch, 0, 0, TO_ROOM);

	ch->desc->str = &msgs[msg_num];
	ch->desc->max_str = MAX_MESSAGE_LENGTH;

	msg_num++;
}


int board_remove_msg(struct char_data *ch, char *arg) {
	int ind, msg;
	char buf[256], number[MAX_INPUT_LENGTH];

	if (GET_LEVEL(ch) < 10) {
		send_to_char("Due to misuse of the REMOVE command, only 10th level\n\r", ch);
		send_to_char("and above can remove messages.\n\r", ch);
		return(0);
	}

	one_argument(arg, number);

	if (!*number || !isdigit(*number))
		return(0);
	if (!(msg = atoi(number))) return(0);
	if (!msg_num) {
		send_to_char("The board is empty!\n\r", ch);
		return(1);
	}
	if (msg < 1 || msg > msg_num) {
		send_to_char("That message exists only in your imagination..\n\r",
			ch);
		return(1);
	}

	ind = msg;
	free(head[--ind]);
	if (msgs[ind])
		free(msgs[ind]);
	for (; ind < msg_num -1; ind++) {
		head[ind] = head[ind + 1];
		msgs[ind] = msgs[ind + 1];
	}
	msg_num--;
	send_to_char("Message removed.\n\r", ch);
	sprintf(buf, "$n just removed message %d.", ind + 1);
	act(buf, FALSE, ch, 0, 0, TO_ROOM);
	board_save_board();

	return(1);
}

void board_save_board() {
	FILE *the_file;		
	int ind, len;
	if (!msg_num) {
		error_log("No messages to save.\n\r");
		return;
	}
	the_file = fopen(SAVE_FILE, "wb");
	if (!the_file) {
		error_log("Unable to open/create savefile..\n\r");
		return;
	}
	fwrite(&msg_num, sizeof(int), 1, the_file);
	for (ind = 0; ind < msg_num; ind++) {
		len = strlen(head[ind]) + 1;
		fwrite(&len, sizeof(int), 1, the_file);
		fwrite(head[ind], sizeof(char), len, the_file);
		len = strlen(msgs[ind]) + 1;
		fwrite(&len, sizeof(int), 1, the_file);
		fwrite(msgs[ind], sizeof(char), len, the_file);
	}
	fclose(the_file);
	board_fix_long_desc(msg_num, head);
	return;
}

void board_load_board() {
	FILE *the_file;
	int ind, len = 0;

	board_reset_board();
	the_file = fopen(SAVE_FILE, "rb");
	if (!the_file) {
		error_log("Can't open message file. Board will be empty.\n\r");
		return;
	}
	fread(&msg_num, sizeof(int), 1, the_file);
	
	if (msg_num < 1 || msg_num > MAX_MSGS || feof(the_file)) {
		error_log("Board-message file corrupt or nonexistent.\n\r");
		fclose(the_file);
		return;
	}
	for (ind = 0; ind < msg_num; ind++) {
		fread(&len, sizeof(int), 1, the_file);
		head[ind] = (char *)malloc(len + 1);
		if (!head[ind]) {
			error_log("Malloc for board header failed.\n\r");
			board_reset_board();
			fclose(the_file);
			return;
		}
		fread(head[ind], sizeof(char), len, the_file);
		fread(&len, sizeof(int), 1, the_file);
		msgs[ind] = (char *)malloc(len + 1);
		if (!msgs[ind]) {
			error_log("Malloc for board msg failed..\n\r");
			board_reset_board();
			fclose(the_file);
			return;
		}
		fread(msgs[ind], sizeof(char), len, the_file);
	}
	fclose(the_file);
	board_fix_long_desc(msg_num, head);
	return;
}

void board_reset_board() {
	int ind;
	for (ind = 0; ind < MAX_MSGS; ind++) {
		free(head[ind]);
		free(msgs[ind]);
	}
	msg_num = 0;
	board_fix_long_desc(0, 0);
	return;
}

void error_log(char *str) {	/* The original error-handling was MUCH */
	fputs("Board : ", stderr);	/* more competent than the current but  */
	fputs(str, stderr);	/* I got the advice to cut it out..;)   */
	return;
}

int board_display_msg(struct char_data *ch, char *arg) {
	char buf[512], number[MAX_INPUT_LENGTH], buffer[MAX_STRING_LENGTH];
	int msg;
	one_argument(arg, number);
	if (!*number || !isdigit(*number))
		return(0);
	if (!(msg = atoi(number))) return(0);
	if (!msg_num) {
		send_to_char("The board is empty!\n\r", ch);
		return(1);
	}
	if (msg < 1 || msg > msg_num) {
		send_to_char("That message exists only in your imagination..\n\r",
			ch);
		return(1);
	}

	/*  sprintf(buf, "$n reads message %d titled : %s.",
		msg, head[msg - 1]);
	act(buf, TRUE, ch, 0, 0, TO_ROOM);    */

	/* Can PERFORM() handle this...?  no. Sorry*/
	/* sprintf(ch, "Message %d	: %s\n\r%s", msg, head[msg - 1], msgs[msg - 1]); */
	/* Bad news */

	sprintf(buffer, "Message %d : %s\n\r\n\r%s", msg, head[msg - 1],
		msgs[msg - 1]);
	page_string(ch->desc, buffer, 1);
	return(1);
}


		
void board_fix_long_desc(int num, char *headers[MAX_MSGS]) {

	struct obj_data *ob;




	/**** Assign the right value to this pointer..how? ****/
	/**** It should point to the bulletin board object ****/
	/**** Then make ob.description point to a malloced ****/
	/**** space containing itoa(msg_num) and all the   ****/
	/**** headers. In the format :
	This is a bulletin board. Usage : READ/REMOVE <message #>, NOTE <header>
	There are 12 messages on the board.
	1	: Re : Whatever and something else too.
	2	: I don't agree with Rainbird.
	3	: Me neither.
	4 	: Groo got hungry again - bug or sabotage?

	Well...something like that..;) 			   ****/
	
	/**** It is always to contain the first line and   ****/
	/**** the second line will vary in how many notes  ****/
	/**** the board has. Then the headers and message  ****/
	/**** numbers will be listed. 			   ****/
	return;
}




int board_show_board(struct char_data *ch, char *arg)
{
	int i;
	char buf[MAX_STRING_LENGTH], tmp[MAX_INPUT_LENGTH];

	one_argument(arg, tmp);

	if (!*tmp || !isname(tmp, "board bulletin"))
		return(0);

	act("$n studies the board.", TRUE, ch, 0, 0, TO_ROOM);

	strcpy(buf,
"This is a bulletin board. Usage: READ/REMOVE <messg #>, WRITE <header>\n\r");
	if (!msg_num)
		strcat(buf, "The board is empty.\n\r");
	else
	{
		sprintf(buf + strlen(buf), "There are %d messages on the board.\n\r",
			msg_num);
		for (i = 0; i < msg_num; i++)
			sprintf(buf + strlen(buf), "%-2d : %s\n\r", i + 1, head[i]);
 	}
	page_string(ch->desc, buf, 1);

	return(1);
}
