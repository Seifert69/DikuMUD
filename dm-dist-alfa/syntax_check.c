/**************************************************************************
*  file: SYNTAX_CHECKER.c , Check syntax of all files     Part of DIKUMUD *
*  Usage: QUICK AND DIRTY!!                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "limits.h"

/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */

struct room_data *world;              /* dyn alloc'ed array of rooms     */
int top_of_world = 0;                 /* ref to the top element of world */
struct obj_data  *object_list = 0;    /* the global linked list of obj's */
struct char_data *character_list = 0; /* global l-list of chars          */

struct zone_data *zone_table;         /* table of reset data             */
int top_of_zone_table = 0;
struct message_list fight_messages[MAX_MESSAGES]; /* fighting messages   */
struct player_index_element *player_table = 0; /* index to player file   */
int top_of_p_table = 0;               /* ref to top of table             */
int top_of_p_file = 0;

FILE *mob_f,                          /* file containing mob prototypes  */
     *obj_f,                          /* obj prototypes                  */
     *wld_f,                          /* World file                      */
     *zon_f;

struct index_data *mob_index;         /* index table for mobile file     */
struct index_data *obj_index;         /* index table for object file     */
struct index_data *wld_index;

struct help_index_element *help_index = 0;

int top_of_mobt = 0;                  /* top of mobile index table       */
int top_of_objt = 0;                  /* top of object index table       */
int top_of_wldt = 0;

struct time_data time_info;		/* the infomation about the time   */
struct weather_data weather_info;	/* the infomation about the weather */




/* local procedures */
void boot_zones(void);
void setup_dir(FILE *fl, int room, int dir);
void allocate_room(int new_top);
void boot_world(void);
struct index_data *generate_indices(FILE *fl, int *top);
void build_player_index(void);
void char_to_store(struct char_data *ch, struct char_file_u *st);
void store_to_char(struct char_file_u *st, struct char_data *ch);
int is_empty(int zone_nr);
void reset_zone(int zone);
int file_to_string(char *name, char *buf);
void renum_world(void);
void renum_zone_table(void);
void reset_time(void);
void clear_char(struct char_data *ch);


/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */


void assume(int faktisk, int antal, int place, char *errmsg)
{
	if (antal != faktisk) {
		printf("Error has occured at #%d.\n\r", place);
		printf("Message is : %s\n\r", errmsg);
		printf("Actual number read is %d\n\r", faktisk);
		exit();
	}
}


/* generate index table for object, monster or world file*/
struct index_data *generate_indices(FILE *fl, int *top)
{
	int i = 0, antal;
	struct index_data *index;
	long pos;
	char buf[82];

	rewind(fl);

	for (;;)
	{
		if (fgets(buf, 81, fl))
		{
			if (*buf == '#')
			{
				/* allocate new cell */
				
				if (!i)						 /* first cell */
					CREATE(index, struct index_data, 1);
				else
					if (!(index = 
						(struct index_data*) realloc(index, 
						(i + 1) * sizeof(struct index_data))))
					{
						printf("load indices");
						exit();
			 		}
				antal = sscanf(buf, "#%d", &index[i].virtual);
				assume(antal, 1, index[i].virtual, "Next string with E/A/$");

				index[i].pos = ftell(fl);
				index[i].number = index[i].virtual;
				index[i].func = 0;
				i++;
			}
			else 
				if (*buf == '$')	/* EOF */
					break;
		}
		else
		{
			printf("Error when generating index, based upon #xxxx numbers.\n\r");
			printf("   Probably error at end of file.\n\r");

			exit();
		}
	}
	index[i-1].number = -1;
	*top = i - 1;
	return(index);
}

int exist_index(struct index_data *index_list, int top, int num)
{
	int i, found;

	found = FALSE;

	for(i=0; (i<=top) && !(found); i++)
		if (index_list[i].number == num)
			found = TRUE;

	if (!found) {
		printf("Reference to non-existent number #%d\n\r", num);
	}

	return (found);
}


/* check the rooms */
void check_world(FILE *fl)
{
	int room_nr = 0, zone = 0, dir_nr, virtual_nr, flag, tmp, old_virtual;
	char *temp, chk[50];
	struct extra_descr_data *new_descr;

	int antal;
	char *temp2;

	world = 0;
	character_list = 0;
	object_list = 0;

	rewind(fl);

	old_virtual = -1;


	do
	{
		antal = fscanf(fl, " #%d\n", &virtual_nr);
		assume(antal,1, virtual_nr, "Reading #xxx");

		if (old_virtual > virtual_nr)
			assume(0,1,virtual_nr, "Error - #'s not in order.");
		old_virtual = virtual_nr;

		temp = fread_string(fl);
		if (flag = (*temp != '$'))	/* a new record to be read */
		{
			temp2 = fread_string(fl);

			antal = fscanf(fl, " %d ", &tmp);
			assume(antal,1, virtual_nr, "In room basic 3 numbers");

			antal = fscanf(fl, " %d ", &tmp);
			assume(antal,1, virtual_nr, "In room basic 3 numbers");

			antal = fscanf(fl, " %d ", &tmp);
			assume(antal,1, virtual_nr, "In room basic 3 numbers");

			for (;;)
			{
				antal = fscanf(fl, " %s \n", chk);
				assume(antal,1, virtual_nr, "Reading D/E/S string");

				if (*chk == 'D')  /* direction field */
					setup_dir(fl, virtual_nr, atoi(chk + 1));
				else if (*chk == 'E')  /* extra description field */
				{
					temp2 = fread_string(fl); /* Description */
					temp2 = fread_string(fl); /* Keywords    */
				}
				else if (*chk == 'S')	/* end of current room */
					break;
				else
					assume(FALSE, 0, virtual_nr, "MISSING D/E or S");
			}
						
		}
	}
	while (flag);
}




/* read direction data */
void setup_dir(FILE *fl, int room, int dir)
{
	int tmp, antal;
	char *temp;

	temp = fread_string(fl);
	temp = fread_string(fl);

	antal = fscanf(fl, " %d ", &tmp);
	assume(antal,1, room, "One of three Direction data");
	antal = fscanf(fl, " %d ", &tmp);
	assume(antal,1, room, "One of three Direction data");
	antal = fscanf(fl, " %d ", &tmp);
	assume(antal,1, room, "One of three Direction data");
	exist_index(wld_index, top_of_wldt, tmp);

}


#define NEW_ZONE_SYSTEM xxx

/* load the zone table and command tables */
void check_zones(FILE *fl)
{
	int line_no;
	int antal, tmp1, tmp2, tmp3, tmp4;
	int zon = 0, cmd_no = 0, ch, expand;
	char *check, buf[81];
	char cmd_type;

	rewind(fl);
	line_no=1;

	for (;;)
	{
		antal = fscanf(fl, " #%*d\n");
		assume(antal, 0, line_no++, "Zone number not found");

		check = fread_string(fl);
		line_no++;

		if (*check == '$')
			break;		/* end of file */

		/* alloc a new zone */

#ifdef NEW_ZONE_SYSTEM
		antal = fscanf(fl, " %d ", &zon);
		assume(antal, 1, line_no, "Zone Room < number not found");
#endif

		antal = fscanf(fl, " %d ", &zon);
		assume(antal, 1, line_no, "Life Span");

		antal = fscanf(fl, " %d ", &zon);
		assume(antal, 1, line_no++, "Reset Mode");


		/* read the command table */

		cmd_no = 0;

		for (expand = 1;;)
		{

			fscanf(fl, " "); /* skip blanks */
			antal = fscanf(fl, "%c", &cmd_type);
			assume(antal, 1, line_no, "Command type M/*/O/G/E/S missing");
			
			if (cmd_type == 'S')
				break;

			if (cmd_type == '*')
			{
				expand = 0;
				fgets(buf, 80, fl); /* skip command */
				line_no++;
				continue;
			}

			antal = fscanf(fl, " %d %d %d", &tmp1, &tmp2, &tmp3);
			assume(antal, 3, line_no, "Three values after command missing");


			if (cmd_type == 'M' || cmd_type == 'O' ||
			    cmd_type == 'D' || cmd_type == 'P') {
				antal = fscanf(fl, " %d", &tmp4);
				assume(antal, 1, line_no, "Fourth value after command missing");
			}

			switch (cmd_type) {
				case 'M' :
					exist_index(mob_index, top_of_mobt, tmp2);
					exist_index(wld_index, top_of_wldt, tmp4);
					break;
				case 'O' :
					exist_index(obj_index, top_of_objt, tmp2);
					exist_index(wld_index, top_of_wldt, tmp4);
					break;
				case 'G' :
					exist_index(obj_index, top_of_objt, tmp2);
					break;
				case 'E' :
					exist_index(obj_index, top_of_objt, tmp2);
					break;
				case 'P' :
					exist_index(obj_index, top_of_objt, tmp2);
					exist_index(obj_index, top_of_objt, tmp4);
					break;
				case 'D' :
					exist_index(wld_index, top_of_wldt, tmp2);
					break;
				case 'R' :
					exist_index(wld_index, top_of_wldt, tmp2);
					exist_index(obj_index, top_of_objt, tmp3);
					break;
				case '*' :
					break;
				deafult  :
					printf("Illegal command type");
					exit();
					break;
			}
					

			fgets(buf, 80, fl);	/* read comment */
			line_no++;
		}
	}
}
 




/*************************************************************************
*  procedures for resetting, both play-time and boot-time	 	 *
*********************************************************************** */


/* read a mobile from MOB_FILE */
void check_mobile(FILE *fl)
{
	int virtual_nr, old_virtual, antal, flag;
	char *temp;
	char bogst;

	int i, skill_nr;
	long tmp, tmp2, tmp3;
	struct char_data *mob;
	char chk[10];


	old_virtual = -1;

	rewind(fl);

	do {
		antal = fscanf(fl, " #%d\n", &virtual_nr);
		assume(antal,1, virtual_nr, "Reading #xxx");

		if (old_virtual > virtual_nr)
			assume(0,1,virtual_nr, "Error - #'s not in order.");

		old_virtual = virtual_nr;

		temp = fread_string(fl);  /* Namelist */
		if (flag = (*temp != '$')) {	/* a new record to be read */

			/***** String data *** */
			/* Name already read mob->player.name = fread_string(fl); */
			temp = fread_string(fl);  /* short description  */
			temp = fread_string(fl);  /*long_description    */
			temp = fread_string(fl);  /* player.description */

			/* *** Numeric data *** */

			antal = fscanf(fl, "%d ", &tmp);
			assume(antal, 1, virtual_nr, "ACT error");

			antal = fscanf(fl, " %d ", &tmp);
			assume(antal, 1, virtual_nr, "affected_by error");

			antal = fscanf(fl, " %d ", &tmp);
			assume(antal, 1, virtual_nr, "Monster Alignment Error");

			antal = fscanf(fl, " %c \n", &bogst);
			assume(antal, 1, virtual_nr, "Simple/Detailed error");

			if (bogst!='S')
				printf("%c %d\n", bogst, bogst);

			if (bogst == 'S') {
				/* The new easy monsters */

				antal = fscanf(fl, " %D ", &tmp);
				assume(antal, 1, virtual_nr, "Level error");
		
				antal = fscanf(fl, " %D ", &tmp);
				assume(antal, 1, virtual_nr, "THAC0 error");
		
				antal = fscanf(fl, " %D ", &tmp);
				assume(antal, 1, virtual_nr, "AC error");

				antal = fscanf(fl, " %Dd%D+%D ", &tmp, &tmp2, &tmp3);
				assume(antal, 3, virtual_nr, "Hitpoints");

				antal = fscanf(fl, " %Dd%D+%D \n", &tmp, &tmp2, &tmp3);
				assume(antal, 3, virtual_nr, "Damage error");

				antal = fscanf(fl, " %D ", &tmp);
				assume(antal, 1, virtual_nr, "GOLD error");

				antal = fscanf(fl, " %D \n", &tmp);
				assume(antal, 1, virtual_nr, "XP error");

				antal = fscanf(fl, " %D ", &tmp);
				assume(antal, 1, virtual_nr, "POSITION error");

				antal = fscanf(fl, " %D ", &tmp);
				assume(antal, 1, virtual_nr, "DEFAULT POS error");

				antal = fscanf(fl, " %D \n", &tmp);
				assume(antal, 1, virtual_nr, "SEXY error");

		} else {  /* The old monsters are down below here */

			printf("Detailed monsters can't be syntax-checked (yet).\n\r");
			assume(0,1,virtual_nr, "DETAIL ERROR");

			exit();
			/*   ***************************
			fscanf(fl, " %D ", &tmp);
			mob->abilities.str = tmp;

			fscanf(fl, " %D ", &tmp);
			mob->abilities.intel = tmp; 

			fscanf(fl, " %D ", &tmp);
			mob->abilities.wis = tmp;

			fscanf(fl, " %D ", &tmp);
			mob->abilities.dex = tmp;

			fscanf(fl, " %D \n", &tmp);
			mob->abilities.con = tmp;

			fscanf(fl, " %D ", &tmp);
			fscanf(fl, " %D ", &tmp2);

			mob->points.max_hit = 0;
			mob->points.hit = mob->points.max_hit;

			fscanf(fl, " %D ", &tmp);
			mob->points.armor = tmp;

			fscanf(fl, " %D ", &tmp);
			mob->points.mana = tmp;
			mob->points.max_mana = tmp;

			fscanf(fl, " %D ", &tmp);
			mob->points.move = tmp;		
			mob->points.max_move = tmp;

			fscanf(fl, " %D ", &tmp);
			mob->points.gold = tmp;

			fscanf(fl, " %D \n", &tmp);
			GET_EXP(mob) = tmp;

			fscanf(fl, " %D ", &tmp);
			mob->specials.position = tmp;

			fscanf(fl, " %D ", &tmp);
			mob->specials.default_pos = tmp;

			fscanf(fl, " %D ", &tmp);
			mob->player.sex = tmp;

			fscanf(fl, " %D ", &tmp);
			mob->player.class = tmp;

			fscanf(fl, " %D ", &tmp);
			GET_LEVEL(mob) = tmp;

			fscanf(fl, " %D ", &tmp);
			mob->player.birth.hours = time_info.hours;
			mob->player.birth.day	= time_info.day;
			mob->player.birth.month = time_info.month;
			mob->player.birth.year  = time_info.year - tmp;

			fscanf(fl, " %D ", &tmp);
			mob->player.weight = tmp;

			fscanf(fl, " %D \n", &tmp);
			mob->player.height = tmp;

			for (i = 0; i < 3; i++)
			{
				fscanf(fl, " %D ", &tmp);
				GET_COND(mob, i) = tmp;
			}
			fscanf(fl, " \n ");

			for (i = 0; i < 5; i++)
			{
				fscanf(fl, " %D ", &tmp);
				mob->specials.apply_saving_throw[i] = tmp;
			}

			fscanf(fl, " \n ");
			mob->points.damroll = 0;
			mob->specials.damnodice = 1;
			mob->specials.damsizedice = 6;

			mob->points.hitroll = 0;
			************************************* */
		}

		}
	}
	while (flag);

}


/* read an object from OBJ_FILE */
void check_objects(FILE *fl)
{
	int virtual_nr, old_virtual, antal, flag;
	char *temp;

	struct obj_data *obj;
	int tmp, i;
	char chk[256];
	struct extra_descr_data *new_descr;

	old_virtual = -1;

	rewind(fl);

	antal = fscanf(fl, " %s \n", chk);
	assume(antal, 1, virtual_nr, "First #xxx number");

	do {
		antal = sscanf(chk, " #%d\n", &virtual_nr);
		assume(antal,1, virtual_nr, "Reading #xxx");

		if (old_virtual > virtual_nr)
			assume(0,1,virtual_nr, "Error - #'s not in order.");

		old_virtual = virtual_nr;

		temp = fread_string(fl);  /* Namelist */
		if (flag = (*temp != '$')) {	/* a new record to be read */

			/* *** string data *** */

			/* temp = fread_string(fl);  name has been read above */
			temp = fread_string(fl); /* short */
			temp = fread_string(fl); /* descr */
			temp = fread_string(fl); /* action */

			/* *** numeric data *** */

			antal = fscanf(fl, " %d ", &tmp);
			assume(antal, 1, virtual_nr, "Error reading type flag");

			antal = fscanf(fl, " %d ", &tmp);
			assume(antal, 1, virtual_nr, "Extra Flag");

			antal = fscanf(fl, " %d ", &tmp);
			assume(antal, 1, virtual_nr, "wear_flags");

			antal = fscanf(fl, " %d ", &tmp);
			assume(antal, 1, virtual_nr, "value[0]");

			antal = fscanf(fl, " %d ", &tmp);
			assume(antal, 1, virtual_nr, "value[1]");

			antal = fscanf(fl, " %d ", &tmp);
			assume(antal, 1, virtual_nr, "value[2]");

			antal = fscanf(fl, " %d ", &tmp);
			assume(antal, 1, virtual_nr, "value[3]");

			antal = fscanf(fl, " %d ", &tmp);
			assume(antal, 1, virtual_nr, "Weight");

			antal = fscanf(fl, " %d \n", &tmp);
			assume(antal, 1, virtual_nr, "Cost");

			antal = fscanf(fl, " %d \n", &tmp);
			assume(antal, 1, virtual_nr, "Cost Per Day");

			/* *** extra descriptions *** */

			while (fscanf(fl, " %s \n", chk), *chk == 'E')
			{

				temp = fread_string(fl);
				temp = fread_string(fl);
			}

			for( i = 0 ; (i < MAX_OBJ_AFFECT) && (*chk == 'A') ; i++)
			{
				antal = fscanf(fl, " %d ", &tmp);
				assume(antal, 1, virtual_nr, "affected location");

				antal = fscanf(fl, " %d \n", &tmp);
				assume(antal, 1, virtual_nr, "Modifier");

				antal = fscanf(fl, " %s \n", chk);
        assume(antal, 1, virtual_nr, "Next string with E/A/$");

			}
		}
	}
	while (flag);
}




/************************************************************************
*  procs of a (more or less) general utility nature			*
********************************************************************** */


/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE *fl)
{
	static char buf[MAX_STRING_LENGTH], tmp[100];
	char *rslt;
	register char *point;
	int flag;

	bzero(buf, MAX_STRING_LENGTH);

	do
	{
		if (!fgets(tmp, MAX_STRING_LENGTH, fl))
		{
			printf("fread_str");
			exit();
		}

		if (strlen(tmp) + strlen(buf) > MAX_STRING_LENGTH)
		{
			printf("fread_string: string too large (db.c, fread_string)");
			exit();
		}
		else
			strcat(buf, tmp);

		for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point);
			point--);		
		if (flag = (*point == '~'))
			if (*(buf + strlen(buf) - 3) == '\n')
			{
				*(buf + strlen(buf) - 2) = '\r';
				*(buf + strlen(buf) - 1) = '\0';
			}
			else
				*(buf + strlen(buf) -2) = '\0';
		else
		{
			*(buf + strlen(buf) + 1) = '\0';
			*(buf + strlen(buf)) = '\r';
		}
	}
	while (!flag);

	return(buf);
}


int main(int argc, char *argv[])
{

	char name[256];

  if (argc != 2) {
		printf("Usage : syntax_check <BaseFileName>\n\r");
		exit(0);
	}

	strcpy(name, argv[1]);
	strcat(name, ".wld");

	if (!(wld_f = fopen(name, "r")))
	{
		printf("Could not open world file.\n\r");
		exit();
	}
	strcpy(name, argv[1]);
	strcat(name, ".mob");
	if (!(mob_f = fopen(name, "r")))
	{
		printf("Could not open mobile file.\n\r");
		exit();
	}
	strcpy(name, argv[1]);
	strcat(name, ".obj");
	if (!(obj_f = fopen(name, "r")))
	{
		printf("Could not open object file.\n\r");
		exit();
	}
	strcpy(name, argv[1]);
	strcat(name, ".zon");
	if (!(zon_f = fopen(name, "r")))
	{
		printf("Could not open zone file.\n\r");
		exit();
	}

	
	printf("Generating world file indexes.\n\r");
	wld_index = generate_indices(wld_f, &top_of_wldt);

	printf("Generating mobile file indexes.\n\r");
	mob_index = generate_indices(mob_f, &top_of_mobt);

	printf("Generating object file indexes.\n\r");
	obj_index = generate_indices(obj_f, &top_of_objt);

	printf("Checking World File\n\r");
	check_world(wld_f);

	printf("Checking Mobile File (only simple mobiles).\n\r");
	check_mobile(mob_f);

	printf("Checking Object File.\n\r");
	check_objects(obj_f);

	printf("Checking Zone File .\n\r");
	check_zones(zon_f);

  printf("\n\r\nCheck successfully completed without any obvious errors.\n\r");

	fclose(zon_f);
	fclose(wld_f);
	fclose(mob_f);
	fclose(obj_f);
}
