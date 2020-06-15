/**************************************************************************
*  file: db.c , Database module.                          Part of DIKUMUD *
*  Usage: Loading/Saving chars, booting world, resetting etc.             *
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

#define NEW_ZONE_SYSTEM

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

char credits[MAX_STRING_LENGTH];      /* the Credits List                */
char news[MAX_STRING_LENGTH];	        /* the news                        */
char motd[MAX_STRING_LENGTH];         /* the messages of today           */
char help[MAX_STRING_LENGTH];         /* the main help page              */
char info[MAX_STRING_LENGTH];         /* the info text                   */
char wizlist[MAX_STRING_LENGTH];      /* the wizlist                     */


FILE *mob_f,                          /* file containing mob prototypes  */
     *obj_f,                          /* obj prototypes                  */
     *help_fl;                        /* file for help texts (HELP <kwd>)*/

struct index_data *mob_index;         /* index table for mobile file     */
struct index_data *obj_index;         /* index table for object file     */
struct help_index_element *help_index = 0;

int top_of_mobt = 0;                  /* top of mobile index table       */
int top_of_objt = 0;                  /* top of object index table       */
int top_of_helpt;                     /* top of help index table         */

struct time_info_data time_info;	/* the infomation about the time   */
struct weather_data weather_info;	/* the infomation about the weather */

bool wizlock = FALSE;                 /* is the game wizlocked           */


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

/* external refs */
extern struct descriptor_data *descriptor_list;
void load_messages(void);
void weather_and_time ( int mode );
void assign_command_pointers ( void );
void assign_spell_pointers ( void );
void slog(char *str);
int dice(int number, int size);
int number(int from, int to);
void boot_social_messages(void);
void boot_pose_messages(void);
void update_obj_file(void); /* In reception.c */
struct help_index_element *build_help_index(FILE *fl, int *num);


/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */


/* body of the booting system */
void boot_db(void)
{
	int i;
	extern int no_specials;

	slog("Boot db -- BEGIN.");

	slog("Resetting the game time:");
	reset_time();

	slog("Reading newsfile, credits, help-page, info and motd.");
	file_to_string(NEWS_FILE, news);
	file_to_string(CREDITS_FILE, credits);
	file_to_string(MOTD_FILE, motd);
	file_to_string(HELP_PAGE_FILE, help);
	file_to_string(INFO_FILE, info);
	file_to_string(WIZLIST_FILE, wizlist);

	slog("Opening mobile, object and help files.");
	if (!(mob_f = fopen(MOB_FILE, "r")))
	{
		perror("boot");
		exit(0);
	}

	if (!(obj_f = fopen(OBJ_FILE, "r")))
	{
		perror("boot");
		exit(0);
	}
	if (!(help_fl = fopen(HELP_KWRD_FILE, "r")))
		slog("   Could not open help file.");
	else 
		help_index = build_help_index(help_fl, &top_of_helpt);


	slog("Loading zone table.");
	boot_zones();

	slog("Loading rooms.");
	boot_world();
	slog("Renumbering rooms.");
	renum_world();

	slog("Generating index tables for mobile and object files.");
	mob_index = generate_indices(mob_f, &top_of_mobt);
	obj_index = generate_indices(obj_f, &top_of_objt);

	slog("Renumbering zone table.");
	renum_zone_table();

	slog("Generating player index.");
	build_player_index();

	slog("Loading fight messages.");
	load_messages();

	slog("Loading social messages.");
	boot_social_messages();

	slog("Loading pose messages.");
	boot_pose_messages();

	slog("Assigning function pointers:");
	if (!no_specials)
	{
		slog("   Mobiles.");
		assign_mobiles();
		slog("   Objects.");
		assign_objects();
		slog("   Room.");
		assign_rooms();
	}

	slog("   Commands.");
	assign_command_pointers();
	slog("   Spells.");
	assign_spell_pointers();

	slog("Updating characters with saved items:");
	update_obj_file();

	for (i = 0; i <= top_of_zone_table; i++)
	{
		fprintf(stderr, "Performing boot-time reset of %s (rooms %d-%d).\n",
			zone_table[i].name,
			(i ? (zone_table[i - 1].top + 1) : 0),
			zone_table[i].top);
		reset_zone(i);
	}

	reset_q.head = reset_q.tail = 0;

	slog("Boot db -- DONE.");
}


/* reset the time in the game from file */
void reset_time(void)
{
	char buf[MAX_STRING_LENGTH];
	struct time_info_data mud_time;

	long beginning_of_time = 650336715;

	struct time_info_data mud_time_passed(time_t t2, time_t t1);

	time_info = mud_time_passed(time(0), beginning_of_time);

/*
	FILE *f1;
	long current_time;
	long last_time;
	long diff_time;
	long diff_hours;

	if (!(f1 = fopen(TIME_FILE, "r")))
	{
		perror("reset time");
		exit(0);
	}

	fscanf(f1, "#\n");

	fscanf(f1, "%ld\n", &last_time);
	fscanf(f1, "%d\n", &last_time_info.hours);
	fscanf(f1, "%d\n", &last_time_info.day);
	fscanf(f1, "%d\n", &last_time_info.month);
	fscanf(f1, "%d\n", &last_time_info.year);

	fclose(f1);

	sprintf(buf,"   Last Gametime: %dH %dD %dM %dY.",
	        last_time_info.hours, last_time_info.day,
	        last_time_info.month, last_time_info.year);
	log(buf);

	current_time = time(0);
	diff_time = current_time - last_time;

	sprintf(buf,"   Time since last shutdown: %d.", diff_time);
	log(buf);

	time_info.hours = last_time_info.hours;
	time_info.day   = last_time_info.day;
	time_info.month = last_time_info.month;
	time_info.year  = last_time_info.year;

	diff_hours = diff_time/SECS_PER_MUD_HOUR;
	diff_time = diff_time % SEC_PR_HOUR;
	
	sprintf(buf,"   Real time lack : %d sec.", diff_time);
	log(buf);

	for(;diff_hours > 0; diff_hours--) 
		weather_and_time(0);

*/

	switch(time_info.hours){
		case 0 :
		case 1 :
		case 2 :
		case 3 :
		case 4 : 
		{
			weather_info.sunlight = SUN_DARK;
			break;
		}
		case 5 :
		{
			weather_info.sunlight = SUN_RISE;
			break;
		}
		case 6 :
		case 7 :
		case 8 :
		case 9 :
		case 10 :
		case 11 :
		case 12 :
		case 13 :
		case 14 :
		case 15 :
		case 16 :
		case 17 :
		case 18 :
		case 19 :
		case 20 :
		{
			weather_info.sunlight = SUN_LIGHT;
			break;
		}
		case 21 :
		{
			weather_info.sunlight = SUN_SET;
			break;
		}
		case 22 :
		case 23 :
		default :
		{
			weather_info.sunlight = SUN_DARK;
			break;
		}
	}

	sprintf(buf,"   Current Gametime: %dH %dD %dM %dY.",
	        time_info.hours, time_info.day,
	        time_info.month, time_info.year);
	slog(buf);

	weather_info.pressure = 960;
	if ((time_info.month>=7)&&(time_info.month<=12))
		weather_info.pressure += dice(1,50);
	else
		weather_info.pressure += dice(1,80);

	weather_info.change = 0;

	if (weather_info.pressure<=980)
		weather_info.sky = SKY_LIGHTNING;
	else if (weather_info.pressure<=1000)
		weather_info.sky = SKY_RAINING;
	else if (weather_info.pressure<=1020)
		weather_info.sky = SKY_CLOUDY;
	else weather_info.sky = SKY_CLOUDLESS;
}



/* update the time file */
void update_time(void)
{
	FILE *f1;
	extern struct time_info_data time_info;
	long current_time;

	return;


	if (!(f1 = fopen(TIME_FILE, "w")))
	{
		perror("update time");
		exit(0);
	}

	current_time = time(0);
	slog("Time update.");

	fprintf(f1, "#\n");

	fprintf(f1, "%ld\n", current_time);
	fprintf(f1, "%d\n", time_info.hours);
	fprintf(f1, "%d\n", time_info.day);
	fprintf(f1, "%d\n", time_info.month);
	fprintf(f1, "%d\n", time_info.year);

	fclose(f1);
}



/* generate index table for the player file */
void build_player_index(void)
{
	int nr = -1, i;
	struct char_file_u dummy;
	FILE *fl;

	if ((fl = fopen(PLAYER_FILE, "rb+")))
	{

		for (; !feof(fl);)
		{
			fread(&dummy, sizeof(struct char_file_u), 1, fl);
			if (!feof(fl))   /* new record */
			{
				/* Create new entry in the list */
				if (nr == -1) {
					CREATE(player_table, 
					   struct player_index_element, 1);
					nr = 0;
				}	else {
					if (!(player_table = (struct player_index_element *)
					    realloc(player_table, (++nr + 1) *
					    sizeof(struct player_index_element))))
					{
						perror("generate index");
						exit(0);
					}
				}
			
				player_table[nr].nr = nr;

				CREATE(player_table[nr].name, char,
				   strlen(dummy.name) + 1);
				for (i = 0; *(player_table[nr].name + i) = 
				   LOWER(*(dummy.name + i)); i++);
			}
		}

		fclose(fl);
	}

	top_of_p_table = nr;

	top_of_p_file = top_of_p_table;
}
	





/* generate index table for object or monster file */
struct index_data *generate_indices(FILE *fl, int *top)
{
	int i = 0;
	struct index_data *index = NULL;
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
						perror("load indices");
						exit(0);
			 		}
				sscanf(buf, "#%d", &index[i].virtual);
				index[i].pos = ftell(fl);
				index[i].number = 0;
				index[i].func = 0;
				i++;
			}
			else 
				if (*buf == '$')	/* EOF */
					break;
		}
		else
		{
			perror("generate indices");
			exit(0);
		}
	}
	*top = i - 2;
	return(index);
}




/* load the rooms */
void boot_world(void)
{
	FILE *fl;
	int room_nr = 0, zone = 0, dir_nr, virtual_nr, flag, tmp;
	char *temp, chk[50];
	struct extra_descr_data *new_descr;

	world = 0;
	character_list = 0;
	object_list = 0;
	
	if (!(fl = fopen(WORLD_FILE, "r")))
	{
		perror("fopen");
		slog("boot_world: could not open world file.");
		exit(0);
	}

	do
	{
		fscanf(fl, " #%d\n", &virtual_nr);

		temp = fread_string(fl);
		if (flag = (*temp != '$'))	/* a new record to be read */
		{
			allocate_room(room_nr);
			world[room_nr].number = virtual_nr;
			world[room_nr].name = temp;
			world[room_nr].description = fread_string(fl);

			if (top_of_zone_table >= 0)
			{
				fscanf(fl, " %*d ");

				/* OBS: Assumes ordering of input rooms */

				if (world[room_nr].number <= (zone ? zone_table[zone-1].top : -1))
				{
					fprintf(stderr, "Room nr %d is below zone %d.\n",
						room_nr, zone);
					exit(0);
				}
				while (world[room_nr].number > zone_table[zone].top)
					if (++zone > top_of_zone_table)
					{
						fprintf(stderr, "Room %d is outside of any zone.\n",
							virtual_nr);
						exit(0);
					}
				world[room_nr].zone = zone;
			}
			fscanf(fl, " %d ", &tmp);
			world[room_nr].room_flags = tmp;
			fscanf(fl, " %d ", &tmp);
			world[room_nr].sector_type = tmp;

			world[room_nr].funct = 0;
			world[room_nr].contents = 0;
			world[room_nr].people = 0;
			world[room_nr].light = 0; /* Zero light sources */

			for (tmp = 0; tmp <= 5; tmp++)
				world[room_nr].dir_option[tmp] = 0;

			world[room_nr].ex_description = 0;

			for (;;)
			{
				fscanf(fl, " %s \n", chk);

				if (*chk == 'D')  /* direction field */
					setup_dir(fl, room_nr, atoi(chk + 1));
				else if (*chk == 'E')  /* extra description field */
				{
					CREATE(new_descr, struct extra_descr_data, 1);
					new_descr->keyword = fread_string(fl);
					new_descr->description = fread_string(fl);
					new_descr->next = world[room_nr].ex_description;
					world[room_nr].ex_description = new_descr;
				}
				else if (*chk == 'S')	/* end of current room */
					break;
			}
						
			room_nr++;
  		}
	}
	while (flag);

	free(temp);	/* cleanup the area containing the terminal $  */

	fclose(fl);
	top_of_world = --room_nr;
}





void allocate_room(int new_top)
{
	struct room_data *new_world;

	if (new_top)
	{ 
		if (!(new_world = (struct room_data *) 
			realloc(world, (new_top + 1) * sizeof(struct room_data))))
		{
			perror("alloc_room");
			exit(0);
		} 
	}
	else
		CREATE(new_world, struct room_data, 1);

	world = new_world;
}






/* read direction data */
void setup_dir(FILE *fl, int room, int dir)
{
	int tmp;

	CREATE(world[room].dir_option[dir], 
		struct room_direction_data, 1);

	world[room].dir_option[dir]->general_description =
		fread_string(fl);
	world[room].dir_option[dir]->keyword = fread_string(fl);

	fscanf(fl, " %d ", &tmp);
	if (tmp == 1)
		world[room].dir_option[dir]->exit_info = EX_ISDOOR;
	else if (tmp == 2)
		world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF;
	else
		world[room].dir_option[dir]->exit_info = 0;
 
	fscanf(fl, " %d ", &tmp);
	world[room].dir_option[dir]->key = tmp;

	fscanf(fl, " %d ", &tmp);
	world[room].dir_option[dir]->to_room = tmp;
}




void renum_world(void)
{
	register int room, door;

	for (room = 0; room <= top_of_world; room++)
		for (door = 0; door <= 5; door++)
			if (world[room].dir_option[door])
		 if (world[room].dir_option[door]->to_room != NOWHERE)
			 world[room].dir_option[door]->to_room =
				 real_room(world[room].dir_option[door]->to_room);
}


#ifdef NEW_ZONE_SYSTEM

void renum_zone_table(void)
{
	int zone, comm;

	for (zone = 0; zone <= top_of_zone_table; zone++)
		for (comm = 0; zone_table[zone].cmd[comm].command != 'S'; comm++)
			switch(zone_table[zone].cmd[comm].command)
			{
				case 'M':
					zone_table[zone].cmd[comm].arg1 =
						real_mobile(zone_table[zone].cmd[comm].arg1);
					zone_table[zone].cmd[comm].arg3 = 
						real_room(zone_table[zone].cmd[comm].arg3);
				break;
				case 'O':
					zone_table[zone].cmd[comm].arg1 = 
						real_object(zone_table[zone].cmd[comm].arg1);
					if (zone_table[zone].cmd[comm].arg3 != NOWHERE)
						zone_table[zone].cmd[comm].arg3 =
						real_room(zone_table[zone].cmd[comm].arg3);
				break;
				case 'G':
					zone_table[zone].cmd[comm].arg1 =
						real_object(zone_table[zone].cmd[comm].arg1);
				break;
				case 'E':
					zone_table[zone].cmd[comm].arg1 =
						real_object(zone_table[zone].cmd[comm].arg1);
				break;
				case 'P':
					zone_table[zone].cmd[comm].arg1 =
						real_object(zone_table[zone].cmd[comm].arg1);
					zone_table[zone].cmd[comm].arg3 =
						real_object(zone_table[zone].cmd[comm].arg3);
				break;					
				case 'D':
					zone_table[zone].cmd[comm].arg1 =
						real_room(zone_table[zone].cmd[comm].arg1);
				break;
			}
}


#else


void renum_zone_table(void)
{
	int zone, comm;

	for (zone = 0; zone <= top_of_zone_table; zone++)
		for (comm = 0; zone_table[zone].cmd[comm].command != 'S'; comm++)
			switch(zone_table[zone].cmd[comm].command)
			{
				case 'M':
					zone_table[zone].cmd[comm].arg1 =
						real_mobile(zone_table[zone].cmd[comm].arg1);
					zone_table[zone].cmd[comm].arg3 = 
						real_room(zone_table[zone].cmd[comm].arg3);
				break;
				case 'O':
					zone_table[zone].cmd[comm].arg1 = 
						real_object(zone_table[zone].cmd[comm].arg1);
					if (zone_table[zone].cmd[comm].arg3 != NOWHERE)
						zone_table[zone].cmd[comm].arg3 =
						real_room(zone_table[zone].cmd[comm].arg3);
				break;
				case 'G':
					zone_table[zone].cmd[comm].arg1 =
						real_object(zone_table[zone].cmd[comm].arg1);
					zone_table[zone].cmd[comm].arg2 =
						real_mobile(zone_table[zone].cmd[comm].arg2);
				break;
				case 'E':
					zone_table[zone].cmd[comm].arg1 =
						real_object(zone_table[zone].cmd[comm].arg1);
					zone_table[zone].cmd[comm].arg2 =
						real_mobile(zone_table[zone].cmd[comm].arg2);
				break;
				case 'P':
					zone_table[zone].cmd[comm].arg1 =
						real_object(zone_table[zone].cmd[comm].arg1);
					zone_table[zone].cmd[comm].arg2 =
						real_object(zone_table[zone].cmd[comm].arg2);
				break;					
				case 'D':
					zone_table[zone].cmd[comm].arg1 =
						real_room(zone_table[zone].cmd[comm].arg1);
				break;
			}
}


#endif


#ifdef NEW_ZONE_SYSTEM

/* load the zone table and command tables */
void boot_zones(void)
{
	FILE *fl;
	int zon = 0, cmd_no = 0, ch, expand, tmp;
	char *check, buf[81];

	if (!(fl = fopen(ZONE_FILE, "r")))
	{
		perror("boot_zones");
		exit(0);
	}

	for (;;)
	{
		fscanf(fl, " #%*d\n");
		check = fread_string(fl);

		if (*check == '$')
			break;		/* end of file */

		/* alloc a new zone */

		if (!zon)
			CREATE(zone_table, struct zone_data, 1);
		else
			if (!(zone_table = (struct zone_data *) realloc(zone_table,
				(zon + 1) * sizeof(struct zone_data))))
				{
					perror("boot_zones realloc");
					exit(0);
				}

		zone_table[zon].name = check;
		fscanf(fl, " %d ", &zone_table[zon].top);
		fscanf(fl, " %d ", &zone_table[zon].lifespan);
		fscanf(fl, " %d ", &zone_table[zon].reset_mode);

		/* read the command table */

		cmd_no = 0;

		for (expand = 1;;)
		{
			if (expand)
				if (!cmd_no)
					CREATE(zone_table[zon].cmd, struct reset_com, 1);
				else
					if (!(zone_table[zon].cmd =
					  (struct reset_com *) realloc(zone_table[zon].cmd, 
					  (cmd_no + 1) * sizeof(struct reset_com))))
					{
						perror("reset command load");
						exit(0);
					}

			expand = 1;

			fscanf(fl, " "); /* skip blanks */
			fscanf(fl, "%c", 
				&zone_table[zon].cmd[cmd_no].command);
			
			if (zone_table[zon].cmd[cmd_no].command == 'S')
				break;

			if (zone_table[zon].cmd[cmd_no].command == '*')
			{
				expand = 0;
				fgets(buf, 80, fl); /* skip command */
				continue;
			}

			fscanf(fl, " %d %d %d", 
				&tmp,
				&zone_table[zon].cmd[cmd_no].arg1,
				&zone_table[zon].cmd[cmd_no].arg2);

			zone_table[zon].cmd[cmd_no].if_flag = tmp;

			if (zone_table[zon].cmd[cmd_no].command == 'M' ||
				zone_table[zon].cmd[cmd_no].command == 'O' ||
				zone_table[zon].cmd[cmd_no].command == 'E' ||
				zone_table[zon].cmd[cmd_no].command == 'P' ||
				zone_table[zon].cmd[cmd_no].command == 'D')
				fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);

			fgets(buf, 80, fl);	/* read comment */

			cmd_no++;
		}
		zon++;
	}
	top_of_zone_table = --zon;
	free(check);
	fclose(fl);
}


#else


/* load the zone table and command tables */
void boot_zones(void)
{
	FILE *fl;
	int zon = 0, cmd_no = 0, ch, expand;
	char *check, buf[81];

	if (!(fl = fopen(ZONE_FILE, "r")))
	{
		perror("boot_zones");
		exit(0);
	}

	for (;;)
	{
		fscanf(fl, " #%*d\n");
		check = fread_string(fl);

		if (*check == '$')
			break;		/* end of file */

		/* alloc a new zone */

		if (!zon)
			CREATE(zone_table, struct zone_data, 1);
		else
			if (!(zone_table = (struct zone_data *) realloc(zone_table,
				(zon + 1) * sizeof(struct zone_data))))
				{
					perror("boot_zones realloc");
					exit(0);
				}

		zone_table[zon].name = check;
		fscanf(fl, " %d ", &zone_table[zon].top);
		fscanf(fl, " %d ", &zone_table[zon].lifespan);
		fscanf(fl, " %d ", &zone_table[zon].reset_mode);

		/* read the command table */

		cmd_no = 0;

		for (expand = 1;;)
		{
			if (expand)
				if (!cmd_no)
					CREATE(zone_table[zon].cmd, struct reset_com, 1);
				else
					if (!(zone_table[zon].cmd =
					  (struct reset_com *) realloc(zone_table[zon].cmd, 
					  (cmd_no + 1) * sizeof(struct reset_com))))
					{
						perror("reset command load");
						exit(0);
					}

			expand = 1;

			fscanf(fl, " "); /* skip blanks */
			fscanf(fl, "%c", 
				&zone_table[zon].cmd[cmd_no].command);
			
			if (zone_table[zon].cmd[cmd_no].command == 'S')
				break;

			if (zone_table[zon].cmd[cmd_no].command == '*')
			{
				expand = 0;
				fgets(buf, 80, fl); /* skip command */
				continue;
			}

			fscanf(fl, " %d %d %d", 
				&zone_table[zon].cmd[cmd_no].if_flag,
				&zone_table[zon].cmd[cmd_no].arg1,
				&zone_table[zon].cmd[cmd_no].arg2);

			if (zone_table[zon].cmd[cmd_no].command == 'M' ||
				zone_table[zon].cmd[cmd_no].command == 'O' ||
				zone_table[zon].cmd[cmd_no].command == 'E' ||
				zone_table[zon].cmd[cmd_no].command == 'D')
				fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);

			fgets(buf, 80, fl);	/* read comment */

			cmd_no++;
		}
		zon++;
	}
	top_of_zone_table = --zon;
	free(check);
	fclose(fl);
}


#endif

/*************************************************************************
*  procedures for resetting, both play-time and boot-time	 	 *
*********************************************************************** */


/* read a mobile from MOB_FILE */
struct char_data *read_mobile(int nr, int type)
{
	int i, skill_nr;
	long tmp, tmp2, tmp3;
	struct char_data *mob;
	char chk[10], buf[100];
	char letter;

	i = nr;
	if (type == VIRTUAL)
		if ((nr = real_mobile(nr)) < 0)
	{
		sprintf(buf, "Mobile (V) %d does not exist in database.", i);
		return(0);
	}

	fseek(mob_f, mob_index[nr].pos, 0);

	CREATE(mob, struct char_data, 1);
	clear_char(mob);

	/***** String data *** */
		
	mob->player.name = fread_string(mob_f);
	mob->player.short_descr = fread_string(mob_f);
	mob->player.long_descr = fread_string(mob_f);
	mob->player.description = fread_string(mob_f);
	mob->player.title = 0;

	/* *** Numeric data *** */

	fscanf(mob_f, "%ld ", &tmp);
	mob->specials.act = tmp;
	SET_BIT(mob->specials.act, ACT_ISNPC);

	fscanf(mob_f, " %ld ", &tmp);
	mob->specials.affected_by = tmp;

	fscanf(mob_f, " %ld ", &tmp);
	mob->specials.alignment = tmp;

	fscanf(mob_f, " %c \n", &letter);

	if (letter == 'S') {
		/* The new easy monsters */
		mob->abilities.str   = 11;
		mob->abilities.intel = 11; 
		mob->abilities.wis   = 11;
		mob->abilities.dex   = 11;
		mob->abilities.con   = 11;

		fscanf(mob_f, " %ld ", &tmp);
		GET_LEVEL(mob) = tmp;
		
		fscanf(mob_f, " %ld ", &tmp);
		mob->points.hitroll = 20-tmp;
		
		fscanf(mob_f, " %ld ", &tmp);
		mob->points.armor = 10*tmp;

		fscanf(mob_f, " %ldd%ld+%ld ", &tmp, &tmp2, &tmp3);
		mob->points.max_hit = dice(tmp, tmp2)+tmp3;
		mob->points.hit = mob->points.max_hit;

		fscanf(mob_f, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
		mob->points.damroll = tmp3;
		mob->specials.damnodice = tmp;
		mob->specials.damsizedice = tmp2;

		mob->points.mana = 10;
		mob->points.max_mana = 10;

		mob->points.move = 50;
		mob->points.max_move = 50;

		fscanf(mob_f, " %ld ", &tmp);
		mob->points.gold = tmp;

		fscanf(mob_f, " %ld \n", &tmp);
		GET_EXP(mob) = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->specials.position = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->specials.default_pos = tmp;

		fscanf(mob_f, " %ld \n", &tmp);
		mob->player.sex = tmp;

		mob->player.class = 0;

		mob->player.time.birth = time(0);
		mob->player.time.played	= 0;
		mob->player.time.logon  = time(0);
		mob->player.weight = 200;
		mob->player.height = 198;

		for (i = 0; i < 3; i++)
			GET_COND(mob, i) = -1;

		for (i = 0; i < 5; i++)
			mob->specials.apply_saving_throw[i] = MAX(20-GET_LEVEL(mob), 2);

	} else {  /* The old monsters are down below here */

		fscanf(mob_f, " %ld ", &tmp);
		mob->abilities.str = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->abilities.intel = tmp; 

		fscanf(mob_f, " %ld ", &tmp);
		mob->abilities.wis = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->abilities.dex = tmp;

		fscanf(mob_f, " %ld \n", &tmp);
		mob->abilities.con = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		fscanf(mob_f, " %ld ", &tmp2);

		mob->points.max_hit = number(tmp, tmp2);
		mob->points.hit = mob->points.max_hit;

		fscanf(mob_f, " %ld ", &tmp);
		mob->points.armor = 10*tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->points.mana = tmp;
		mob->points.max_mana = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->points.move = tmp;		
		mob->points.max_move = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->points.gold = tmp;

		fscanf(mob_f, " %ld \n", &tmp);
		GET_EXP(mob) = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->specials.position = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->specials.default_pos = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->player.sex = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->player.class = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		GET_LEVEL(mob) = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->player.time.birth = time(0);
		mob->player.time.played	= 0;
		mob->player.time.logon  = time(0);

		fscanf(mob_f, " %ld ", &tmp);
		mob->player.weight = tmp;

		fscanf(mob_f, " %ld \n", &tmp);
		mob->player.height = tmp;

		for (i = 0; i < 3; i++)
		{
			fscanf(mob_f, " %ld ", &tmp);
			GET_COND(mob, i) = tmp;
		}
		fscanf(mob_f, " \n ");

		for (i = 0; i < 5; i++)
		{
			fscanf(mob_f, " %ld ", &tmp);
			mob->specials.apply_saving_throw[i] = tmp;
		}

		fscanf(mob_f, " \n ");

		/* Set the damage as some standard 1d4 */
		mob->points.damroll = 0;
		mob->specials.damnodice = 1;
		mob->specials.damsizedice = 6;

		/* Calculate THAC0 as a formular of Level */
		mob->points.hitroll = MAX(1, GET_LEVEL(mob)-3);
	}

	mob->tmpabilities = mob->abilities;

	for (i = 0; i < MAX_WEAR; i++) /* Initialisering Ok */
		mob->equipment[i] = 0;

	mob->nr = nr;

	mob->desc = 0;


	/* insert in list */

	mob->next = character_list;
	character_list = mob;

	mob_index[nr].number++;

	return(mob);
}


/* read an object from OBJ_FILE */
struct obj_data *read_object(int nr, int type)
{
	struct obj_data *obj;
	int tmp, i;
	char chk[50], buf[100];
	struct extra_descr_data *new_descr;

	i = nr;
	if (type == VIRTUAL)
		if ((nr = real_object(nr)) < 0)
	{
		sprintf(buf, "Object (V) %d does not exist in database.", i);
		return(0);
	}

	fseek(obj_f, obj_index[nr].pos, 0);

	CREATE(obj, struct obj_data, 1);

	clear_object(obj);

	/* *** string data *** */

	obj->name = fread_string(obj_f);
	obj->short_description = fread_string(obj_f);
	obj->description = fread_string(obj_f);
	obj->action_description = fread_string(obj_f);

	/* *** numeric data *** */

	fscanf(obj_f, " %d ", &tmp);
	obj->obj_flags.type_flag = tmp;
	fscanf(obj_f, " %d ", &tmp);
	obj->obj_flags.extra_flags = tmp;
	fscanf(obj_f, " %d ", &tmp);
	obj->obj_flags.wear_flags = tmp;
	fscanf(obj_f, " %d ", &tmp);
	obj->obj_flags.value[0] = tmp;
	fscanf(obj_f, " %d ", &tmp);
	obj->obj_flags.value[1] = tmp;
	fscanf(obj_f, " %d ", &tmp);
	obj->obj_flags.value[2] = tmp;
	fscanf(obj_f, " %d ", &tmp);
	obj->obj_flags.value[3] = tmp;
	fscanf(obj_f, " %d ", &tmp);
	obj->obj_flags.weight = tmp;
	fscanf(obj_f, " %d \n", &tmp);
	obj->obj_flags.cost = tmp;
	fscanf(obj_f, " %d \n", &tmp);
	obj->obj_flags.cost_per_day = tmp;

	/* *** extra descriptions *** */

	obj->ex_description = 0;

	while (fscanf(obj_f, " %s \n", chk), *chk == 'E')
	{
		CREATE(new_descr, struct extra_descr_data, 1);

		new_descr->keyword = fread_string(obj_f);
		new_descr->description = fread_string(obj_f);

		new_descr->next = obj->ex_description;
		obj->ex_description = new_descr;
	}

	for( i = 0 ; (i < MAX_OBJ_AFFECT) && (*chk == 'A') ; i++)
	{
		fscanf(obj_f, " %d ", &tmp);
		obj->affected[i].location = tmp;
		fscanf(obj_f, " %d \n", &tmp);
		obj->affected[i].modifier = tmp;
		fscanf(obj_f, " %s \n", chk);
	}

	for (;(i < MAX_OBJ_AFFECT);i++)
	{
		obj->affected[i].location = APPLY_NONE;
		obj->affected[i].modifier = 0;
	}

	obj->in_room = NOWHERE;
	obj->next_content = 0;
	obj->carried_by = 0;
	obj->in_obj = 0;
	obj->contains = 0;
	obj->item_number = nr;	

	obj->next = object_list;
	object_list = obj;

	obj_index[nr].number++;


	return (obj);  
}




#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update(void)
{
	int i;
	struct reset_q_element *update_u, *temp;

	/* enqueue zones */

	for (i = 0; i <= top_of_zone_table; i++)
	{
		if (zone_table[i].age < zone_table[i].lifespan &&
			zone_table[i].reset_mode)
			(zone_table[i].age)++;
		else
			if (zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode)
			{
			/* enqueue zone */

			CREATE(update_u, struct reset_q_element, 1);
 
			update_u->zone_to_reset = i;
			update_u->next = 0;

			if (!reset_q.head)
				reset_q.head = reset_q.tail = update_u;
			else
			{
				reset_q.tail->next = update_u;
				reset_q.tail = update_u;
			}

			zone_table[i].age = ZO_DEAD;
			}
	}

	/* dequeue zones (if possible) and reset */

	for (update_u = reset_q.head; update_u; update_u = update_u->next) 
		if (zone_table[update_u->zone_to_reset].reset_mode == 2 ||
			is_empty(update_u->zone_to_reset))
		{
		reset_zone(update_u->zone_to_reset);

		/* dequeue */

		if (update_u == reset_q.head)
			reset_q.head = reset_q.head->next;
		else
		{
			for (temp = reset_q.head; temp->next != update_u;
				temp = temp->next);

			if (!update_u->next)
				reset_q.tail = temp;

			temp->next = update_u->next;


		}

		free(update_u);
		break;
		} 
}




#ifdef NEW_ZONE_SYSTEM

#define ZCMD zone_table[zone].cmd[cmd_no]

/* execute the reset command table of a given zone */
void reset_zone(int zone)
{
	int cmd_no, last_cmd = 1;
	char buf[256];
	struct char_data *mob = NULL;
	struct obj_data *obj, *obj_to;

	for (cmd_no = 0;;cmd_no++)
	{
		if (ZCMD.command == 'S')
			break;

		if (last_cmd || !ZCMD.if_flag)
			switch(ZCMD.command)
		{
			case 'M': /* read a mobile */
				if (mob_index[ZCMD.arg1].number < 
					ZCMD.arg2)
				{
					mob = read_mobile(ZCMD.arg1, REAL);
					char_to_room(mob, ZCMD.arg3);
					last_cmd = 1;
				}
				else
					last_cmd = 0;
			break;

			case 'O': /* read an object */
				if (obj_index[ZCMD.arg1].number < ZCMD.arg2)
				if (ZCMD.arg3 >= 0)
				{
					if (!get_obj_in_list_num(ZCMD.arg1,world[ZCMD.arg3].contents))
					{
						obj = read_object(ZCMD.arg1, REAL);
						obj_to_room(obj, ZCMD.arg3);
						last_cmd = 1;
					}
					else
						last_cmd = 0;
				}
				else
				{
					obj = read_object(ZCMD.arg1, REAL);
					obj->in_room = NOWHERE;
					last_cmd = 1;
				}
				else
					last_cmd = 0;
			break;

			case 'P': /* object to object */
				if (obj_index[ZCMD.arg1].number < ZCMD.arg2)
				{
					obj = read_object(ZCMD.arg1, REAL);
					obj_to = get_obj_num(ZCMD.arg3);
					obj_to_obj(obj, obj_to);
					last_cmd = 1;
				}
				else
					last_cmd = 0;
			break;

			case 'G': /* obj_to_char */
				if (obj_index[ZCMD.arg1].number < ZCMD.arg2)
				{		
					obj = read_object(ZCMD.arg1, REAL);
					obj_to_char(obj, mob);
					last_cmd = 1;
				}
				else
					last_cmd = 0;
			break;

			case 'E': /* object to equipment list */
				if (obj_index[ZCMD.arg1].number < ZCMD.arg2)
				{		
					obj = read_object(ZCMD.arg1, REAL);
					equip_char(mob, obj, ZCMD.arg3);
					last_cmd = 1;
				}
				else
					last_cmd = 0;
			break;

			case 'D': /* set state of door */
				switch (ZCMD.arg3)
				{
					case 0:
						REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
							EX_LOCKED);
						REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
							EX_CLOSED);
					break;
					case 1:
						SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
							EX_CLOSED);
						REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
							EX_LOCKED);
					break;
					case 2:
						SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
							EX_LOCKED);
						SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
							EX_CLOSED);
					break;
				}
				last_cmd = 1;
			break;

			default:
				sprintf(buf, "Undefd cmd in reset table; zone %d cmd %d.\n\r",
					zone, cmd_no);
				slog(buf);
				exit(0);
			break;
		}
		else
			last_cmd = 0;

	}

	zone_table[zone].age = 0;
}

#undef ZCMD

#else


#define ZCMD zone_table[zone].cmd[cmd_no]

/* execute the reset command table of a given zone */
void reset_zone(int zone)
{
	int cmd_no, last_cmd = 1;
	char buf[256];
	struct char_data *mob;
	struct obj_data *obj, *obj_to;

	for (cmd_no = 0;;cmd_no++)
	{
		if (ZCMD.command == 'S')
			break;

		if (last_cmd || !ZCMD.if_flag)
			switch(ZCMD.command)
		{
			case 'M': /* read a mobile */
				if (mob_index[ZCMD.arg1].number < 
					ZCMD.arg2)
				{
					mob = read_mobile(ZCMD.arg1, REAL);
					char_to_room(mob, ZCMD.arg3);
					last_cmd = 1;
				}
				else
					last_cmd = 0;
			break;

			case 'O': /* read an object */
				if (obj_index[ZCMD.arg1].number <
					ZCMD.arg2)
				if (ZCMD.arg3 >= 0)
				{
					if (!get_obj_in_list_num(
					  ZCMD.arg1,world[ZCMD.arg3].contents))
						{
						obj = read_object(ZCMD.arg1, REAL);
						obj_to_room(obj, ZCMD.arg3);
						last_cmd = 1;
						}
					else
						last_cmd = 0;
				}
				else
				{
					obj = read_object(ZCMD.arg1, REAL);
					obj->in_room = NOWHERE;
					last_cmd = 1;
				}
				else
					last_cmd = 0;
			break;

			case 'P': /* object to object */
				obj = get_obj_num(ZCMD.arg1);
				obj_to = get_obj_num(ZCMD.arg2);
				obj_to_obj(obj, obj_to);
				last_cmd = 1;
			break;

			case 'G': /* obj_to_char */
				obj = get_obj_num(ZCMD.arg1);
				mob = get_char_num(ZCMD.arg2);
				obj_to_char(obj, mob);
				last_cmd = 1;
			break;

			case 'E': /* object to equipment list */
				obj = get_obj_num(ZCMD.arg1);
				mob = get_char_num(ZCMD.arg2);
				equip_char(mob, obj, ZCMD.arg3);
				last_cmd = 1;
			break;

			case 'D': /* set state of door */
				switch (ZCMD.arg3)
				{
					case 0:
						REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
							EX_LOCKED);
						REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
							EX_CLOSED);
					break;
					case 1:
						SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
							EX_CLOSED);
						REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
							EX_LOCKED);
					break;
					case 2:
						SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
							EX_LOCKED);
						SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
							EX_CLOSED);
					break;
				}
			break;

			default:
				sprintf(buf, "Undefd cmd in reset table; zone %d cmd %d.\n\r",
					zone, cmd_no);
				log(buf);
				exit(0);
			break;
		}
		else
			last_cmd = 0;

	}

	zone_table[zone].age = 0;
}

#undef ZCMD

#endif

/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(int zone_nr)
{
	struct descriptor_data *i;

	for (i = descriptor_list; i; i = i->next)
		if (!i->connected)
			if (world[i->character->in_room].zone == zone_nr)
				return(0);

	return(1);
}





/*************************************************************************
*  stuff related to the save/load player system								  *
*********************************************************************** */

/* Load a char, TRUE if loaded, FALSE if not */
int load_char(char *name, struct char_file_u *char_element)
{
	FILE *fl;
	int player_i;

	int find_name(char *name);

	if ((player_i = find_name(name)) >= 0) {

		if (!(fl = fopen(PLAYER_FILE, "r"))) {
			perror("Opening player file for reading. (db.c, load_char)");
			exit(0);
		}

		fseek(fl, (long) (player_table[player_i].nr *
		sizeof(struct char_file_u)), 0);

		fread(char_element, sizeof(struct char_file_u), 1, fl);
		fclose(fl);
		return(player_i);
	} else

		return(-1);
}




/* copy data from the file structure to a char struct */	
void store_to_char(struct char_file_u *st, struct char_data *ch)
{
	int i;

	GET_SEX(ch) = st->sex;
	GET_CLASS(ch) = st->class;
	GET_LEVEL(ch) = st->level;

	ch->player.short_descr = 0;
	ch->player.long_descr = 0;

	if (*st->title)
	{
		CREATE(ch->player.title, char, strlen(st->title) + 1);
		strcpy(ch->player.title, st->title);
	}
	else
		GET_TITLE(ch) = 0;

	if (*st->description)
	{
		CREATE(ch->player.description, char, 
			strlen(st->description) + 1);
		strcpy(ch->player.description, st->description);
	}
	else
		ch->player.description = 0;

	ch->player.hometown = st->hometown;

	ch->player.time.birth = st->birth;
	ch->player.time.played = st->played;
	ch->player.time.logon  = time(0);

	for (i = 0; i <= MAX_TOUNGE - 1; i++)
		ch->player.talks[i] = st->talks[i];

	ch->player.weight = st->weight;
	ch->player.height = st->height;

	ch->abilities = st->abilities;
	ch->tmpabilities = st->abilities;
	ch->points = st->points;

	for (i = 0; i <= MAX_SKILLS - 1; i++)
		ch->skills[i] = st->skills[i];

	ch->specials.spells_to_learn = st->spells_to_learn;
	ch->specials.alignment    = st->alignment;

	ch->specials.act          = st->act;
	ch->specials.carry_weight = 0;
	ch->specials.carry_items  = 0;
	ch->points.armor          = 100;
	ch->points.hitroll        = 0;
	ch->points.damroll        = 0;

	CREATE(GET_NAME(ch), char, strlen(st->name) +1);
	strcpy(GET_NAME(ch), st->name);

	/* Not used as far as I can see (Michael) */
	for(i = 0; i <= 4; i++)
	  ch->specials.apply_saving_throw[i] = st->apply_saving_throw[i];

	for(i = 0; i <= 2; i++)
	  GET_COND(ch, i) = st->conditions[i];

	/* Add all spell effects */
	for(i=0; i < MAX_AFFECT; i++) {
		if (st->affected[i].type)
			affect_to_char(ch, &st->affected[i]);
	}
	ch->in_room = st->load_room;
	affect_total(ch);
} /* store_to_char */

	

	
/* copy vital data from a players char-structure to the file structure */
void char_to_store(struct char_data *ch, struct char_file_u *st)
{
	int i;
	struct affected_type *af;
	struct obj_data *char_eq[MAX_WEAR];

	/* Unaffect everything a character can be affected by */

	for(i=0; i<MAX_WEAR; i++) {
		if (ch->equipment[i])
			char_eq[i] = unequip_char(ch, i);
		else
			char_eq[i] = 0;
	}

	for(af = ch->affected, i = 0; i<MAX_AFFECT; i++) {
		if (af) {
			st->affected[i] = *af;
			st->affected[i].next = 0;
			/* subtract effect of the spell or the effect will be doubled */
			affect_modify( ch, st->affected[i].location,
			                   st->affected[i].modifier,
			                   st->affected[i].bitvector, FALSE);                         
			af = af->next;
		} else {
			st->affected[i].type = 0;  /* Zero signifies not used */
			st->affected[i].duration = 0;
			st->affected[i].modifier = 0;
			st->affected[i].location = 0;
			st->affected[i].bitvector = 0;
			st->affected[i].next = 0;
		}
	}

	if ((i >= MAX_AFFECT) && af && af->next)
		slog("WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");



	ch->tmpabilities = ch->abilities;

	st->birth      = ch->player.time.birth;
	st->played     = ch->player.time.played;
	st->played    += (long) (time(0) - ch->player.time.logon);
	st->last_logon = time(0);

	ch->player.time.played = st->played;
	ch->player.time.logon = time(0);

	st->hometown = ch->player.hometown;
	st->weight   = GET_WEIGHT(ch);
	st->height   = GET_HEIGHT(ch);
	st->sex      = GET_SEX(ch);
	st->class    = GET_CLASS(ch);
	st->level    = GET_LEVEL(ch);
	st->abilities = ch->abilities;
	st->points    = ch->points;
	st->alignment       = ch->specials.alignment;
	st->spells_to_learn = ch->specials.spells_to_learn;
	st->act             = ch->specials.act;

	st->points.armor   = 100;
	st->points.hitroll =  0;
	st->points.damroll =  0;

	if (GET_TITLE(ch))
		strcpy(st->title, GET_TITLE(ch));
	else
		*st->title = '\0';

	if (ch->player.description)
		strcpy(st->description, ch->player.description);
	else
		*st->description = '\0';


	for (i = 0; i <= MAX_TOUNGE - 1; i++)
		st->talks[i] = ch->player.talks[i];

	for (i = 0; i <= MAX_SKILLS - 1; i++)
		st->skills[i] = ch->skills[i];

	strcpy(st->name, GET_NAME(ch) );

	for(i = 0; i <= 4; i++)
	  st->apply_saving_throw[i] = ch->specials.apply_saving_throw[i];

	for(i = 0; i <= 2; i++)
	  st->conditions[i] = GET_COND(ch, i);

	for(af = ch->affected, i = 0; i<MAX_AFFECT; i++) {
		if (af) {
			/* Add effect of the spell or it will be lost */
			/* When saving without quitting               */
			affect_modify( ch, st->affected[i].location,
			                   st->affected[i].modifier,
			                   st->affected[i].bitvector, TRUE);
			af = af->next;
		}
	}

	for(i=0; i<MAX_WEAR; i++) {
		if (char_eq[i])
			equip_char(ch, char_eq[i], i);
	}

	affect_total(ch);
} /* Char to store */




/* create a new entry in the in-memory index table for the player file */
int create_entry(char *name)
{
	int i, pos;
	struct player_index_element tmp;

	if (top_of_p_table == -1)
	{
		CREATE(player_table, struct player_index_element, 1);
		top_of_p_table = 0;
	}
	else
		if (!(player_table = (struct player_index_element *) 
		  realloc(player_table, sizeof(struct player_index_element) * 
		  (++top_of_p_table + 1))))
		{
			perror("create entry");
			exit(1);
		}

	CREATE(player_table[top_of_p_table].name, char , strlen(name) + 1);

	/* copy lowercase equivalent of name to table field */
	for (i = 0; *(player_table[top_of_p_table].name + i) = 
			LOWER(*(name + i)); i++);

	player_table[top_of_p_table].nr = top_of_p_table;

	return (top_of_p_table);

	
}
		


/* write the vital data of a player to the player file */
void save_char(struct char_data *ch, sh_int load_room)
{
	struct char_file_u st;
	FILE *fl;
	char mode[4];
	int expand;

	bzero(&st, sizeof(struct char_file_u));

	if (IS_NPC(ch) || !ch->desc)
		return;

	if (expand = (ch->desc->pos > top_of_p_file))
	{
		strcpy(mode, "a+");
		top_of_p_file++;
	}
	else
		strcpy(mode, "r+");

	char_to_store(ch, &st);
	st.load_room = load_room;

	strcpy(st.pwd, ch->desc->pwd);

	if (!(fl = fopen(PLAYER_FILE, mode)))
	{
		perror("save char");
		exit(1);
	}

	fflush(fl);
	if (expand)
	{
		fwrite(&st, sizeof(struct char_file_u), 1, fl);
	}

	fseek(fl, ch->desc->pos * sizeof(struct char_file_u), 0);

	fwrite(&st, sizeof(struct char_file_u), 1, fl);

	fclose(fl);
}




/* for possible later use with qsort */
int compare(struct player_index_element *arg1, struct player_index_element 
	*arg2)
{
	return (strcasecmp(arg1->name, arg2->name));
}




/************************************************************************
*  procs of a (more or less) general utility nature			*
********************************************************************** */


/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE *fl)
{
	char buf[MAX_STRING_LENGTH], tmp[MAX_STRING_LENGTH];
	char *rslt;
	register char *point;
	int flag;

	bzero(buf, MAX_STRING_LENGTH);

	do
	{
		if (!fgets(tmp, MAX_STRING_LENGTH, fl))
		{
			perror("fread_str");
			exit(0);
		}

		if (strlen(tmp) + strlen(buf) > MAX_STRING_LENGTH)
		{
			slog("fread_string: string too large (db.c)");
			exit(0);
		}
		else
			strncat(buf, tmp, MAX_STRING_LENGTH-1);

		size_t strlenbuf = strlen(buf);

		/* Set point to be the first non-whitespace character */
		for (point = buf + strlenbuf - 2; point >= buf && isspace(*point); point--) ;;

		if ((flag = (*point == '~')) && (strlenbuf >= 3))
			if (*(buf + strlenbuf - 3) == '\n')
			{
				*(buf + strlenbuf - 2) = '\r';
				*(buf + strlenbuf - 1) = '\0';
			}
			else
				*(buf + strlenbuf -2) = '\0';
		else
		{
			*(buf + strlenbuf + 1) = '\0';
			*(buf + strlenbuf) = '\r';
		}
	}
	while (!flag);

	/* do the allocate boogie  */

	if (strlen(buf) > 0)
	{
		CREATE(rslt, char, strlen(buf) + 1);
		strcpy(rslt, buf);
	}
	else
		rslt = 0;
	return(rslt);
}





/* release memory allocated for a char struct */
void free_char(struct char_data *ch)
{
	struct affected_type *af;

	free(GET_NAME(ch));

  	if (ch->player.title)
		free(ch->player.title);
	if (ch->player.short_descr)
		free(ch->player.short_descr);
	if (ch->player.long_descr)
		free(ch->player.long_descr);
	if(ch->player.description)
		free(ch->player.description);

	for (af = ch->affected; af; af = af->next) 
		affect_remove(ch, af);

	free(ch);
}







/* release memory allocated for an obj struct */
void free_obj(struct obj_data *obj)
{
	struct extra_descr_data *this, *next_one;

	free(obj->name);
	if(obj->description)
		free(obj->description);
	if(obj->short_description)
		free(obj->short_description);
	if(obj->action_description)
		free(obj->action_description);

	for( this = obj->ex_description ;
		(this != 0);this = next_one )
	{
		next_one = this->next;
		if(this->keyword)
			free(this->keyword);
		if(this->description)
			free(this->description);
		free(this);
	}

	free(obj);
}






/* read contents of a text file, and place in buf */
int file_to_string(char *name, char *buf)
{
	FILE *fl;
	char tmp[100];

	*buf = '\0';

	if (!(fl = fopen(name, "r")))
	{
		perror("file-to-string");
		*buf = '\0';
		return(-1);
	}

	do
	{
		fgets(tmp, 99, fl);

		if (!feof(fl))
		{
			if (strlen(buf) + strlen(tmp) + 2 > MAX_STRING_LENGTH)
			{
				slog("fl->strng: string too big (db.c, file_to_string)");
				*buf = '\0';
				return(-1);
			}

			strcat(buf, tmp);
			*(buf + strlen(buf) + 1) = '\0';
			*(buf + strlen(buf)) = '\r';
		}
	}
	while (!feof(fl));

	fclose(fl);

	return(0);
}




/* clear some of the the working variables of a char */
void reset_char(struct char_data *ch)
{
	int i;

	for (i = 0; i < MAX_WEAR; i++) /* Initialisering */
		ch->equipment[i] = 0;

	ch->followers = 0;
	ch->master = 0;
/*	ch->in_room = NOWHERE; Used for start in room */
	ch->carrying = 0;
	ch->next = 0;
	ch->next_fighting = 0;
	ch->next_in_room = 0;
	ch->specials.fighting = 0;
	ch->specials.position = POSITION_STANDING;
	ch->specials.default_pos = POSITION_STANDING;
	ch->specials.carry_weight = 0;
	ch->specials.carry_items = 0;

	if (GET_HIT(ch) <= 0)
		GET_HIT(ch) = 1;
	if (GET_MOVE(ch) <= 0)
		GET_MOVE(ch) = 1;
	if (GET_MANA(ch) <= 0)
		GET_MANA(ch) = 1;
}



/* clear ALL the working variables of a char and do NOT free any space alloc'ed*/
void clear_char(struct char_data *ch)
{
	memset(ch, '\0', sizeof(struct char_data));

	ch->in_room = NOWHERE;
	ch->specials.was_in_room = NOWHERE;
	ch->specials.position = POSITION_STANDING;
	ch->specials.default_pos = POSITION_STANDING;
	GET_AC(ch) = 100; /* Basic Armor */
}


void clear_object(struct obj_data *obj)
{
	memset(obj, '\0', sizeof(struct obj_data));

	obj->item_number = -1;
	obj->in_room	  = NOWHERE;
}




/* initialize a new character only if class is set */
void init_char(struct char_data *ch)
{
	int i;

	/* *** if this is our first player --- he be God *** */

	if (top_of_p_table < 0)
	{
		GET_EXP(ch) = 7000000;
		GET_LEVEL(ch) = 24;
	}

	set_title(ch);

	ch->player.short_descr = 0;
	ch->player.long_descr = 0;
	ch->player.description = 0;

	ch->player.hometown = number(1,4);

	ch->player.time.birth = time(0);
	ch->player.time.played = 0;
	ch->player.time.logon = time(0);

	for (i = 0; i < MAX_TOUNGE; i++)
	 ch->player.talks[i] = 0;

	GET_STR(ch) = 9;
	GET_INT(ch) = 9;
	GET_WIS(ch) = 9;
	GET_DEX(ch) = 9;
	GET_CON(ch) = 9;

	/* make favors for sex */
	if (ch->player.sex == SEX_MALE) {
		ch->player.weight = number(120,180);
		ch->player.height = number(160,200);
	} else {
		ch->player.weight = number(100,160);
		ch->player.height = number(150,180);
	}

	ch->points.mana = GET_MAX_MANA(ch);
	ch->points.hit = GET_MAX_HIT(ch);
	ch->points.move = GET_MAX_MOVE(ch);
	ch->points.armor = 100;

	for (i = 0; i <= MAX_SKILLS - 1; i++)
	{
		if (GET_LEVEL(ch) <24) {
			ch->skills[i].learned = 0;
			ch->skills[i].recognise = FALSE;
		}	else {
			ch->skills[i].learned = 100;
			ch->skills[i].recognise = FALSE;
		}
	}

	ch->specials.affected_by = 0;
	ch->specials.spells_to_learn = 0;

	for (i = 0; i < 5; i++)
		ch->specials.apply_saving_throw[i] = 0;

	for (i = 0; i < 3; i++)
		GET_COND(ch, i) = (GET_LEVEL(ch) == 24 ? -1 : 24);
}



/* returns the real number of the room with given virtual number */
int real_room(int virtual)
{
	int bot, top, mid;

	bot = 0;
	top = top_of_world;

	/* perform binary search on world-table */
	for (;;)
	{
		mid = (bot + top) / 2;

		if ((world + mid)->number == virtual)
			return(mid);
		if (bot >= top)
		{
			fprintf(stderr, "Room %d does not exist in database\n", virtual);
			return(-1);
		}
		if ((world + mid)->number > virtual)
			top = mid - 1;
		else
			bot = mid + 1;
	}
}






/* returns the real number of the monster with given virtual number */
int real_mobile(int virtual)
{
	int bot, top, mid;

	bot = 0;
	top = top_of_mobt;

	/* perform binary search on mob-table */
	for (;;)
	{
		mid = (bot + top) / 2;

		if ((mob_index + mid)->virtual == virtual)
			return(mid);
		if (bot >= top)
			return(-1);
		if ((mob_index + mid)->virtual > virtual)
			top = mid - 1;
		else
			bot = mid + 1;
	}
}






/* returns the real number of the object with given virtual number */
int real_object(int virtual)
{
	int bot, top, mid;

	bot = 0;
	top = top_of_objt;

	/* perform binary search on obj-table */
	for (;;)
	{
		mid = (bot + top) / 2;

		if ((obj_index + mid)->virtual == virtual)
			return(mid);
		if (bot >= top)
			return(-1);
		if ((obj_index + mid)->virtual > virtual)
			top = mid - 1;
		else
			bot = mid + 1;
	}
}


