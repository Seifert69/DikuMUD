/* ************************************************************************
*  file: spec_assign.c , Special module.                  Part of DIKUMUD *
*  Usage: Procedures assigning function pointers.                         *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include "structs.h"
#include "db.h"

extern struct room_data *world;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
void boot_the_shops();
void assign_the_shopkeepers();

/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
	int cityguard(struct char_data *ch, int cmd, char *arg);
	int receptionist(struct char_data *ch, int cmd, char *arg);
	int guild(struct char_data *ch, int cmd, char *arg);
	int guild_guard(struct char_data *ch, int cmd, char *arg);
	int puff(struct char_data *ch, int cmd, char *arg);
	int fido(struct char_data *ch, int cmd, char *arg);
	int janitor(struct char_data *ch, int cmd, char *arg);
	int mayor(struct char_data *ch, int cmd, char *arg);
	int snake(struct char_data *ch, int cmd, char *arg);
	int thief(struct char_data *ch, int cmd, char *arg);
	int magic_user(struct char_data *ch, int cmd, char *arg);
	int bat_red(struct char_data *ch, int cmd, char *arg);
	int bat_blue(struct char_data *ch, int cmd, char *arg);
	int bat_green(struct char_data *ch, int cmd, char *arg);
	int bat_black(struct char_data *ch, int cmd, char *arg);
	int bat_white(struct char_data *ch, int cmd, char *arg);


	mob_index[real_mobile(1)].func = puff;

	mob_index[real_mobile(3060)].func = cityguard;
	mob_index[real_mobile(3067)].func = cityguard;
	mob_index[real_mobile(3061)].func = janitor;
	mob_index[real_mobile(3062)].func = fido;
	mob_index[real_mobile(3066)].func = fido;

	mob_index[real_mobile(3005)].func = receptionist;
	mob_index[real_mobile(5007)].func = receptionist;

	mob_index[real_mobile(3020)].func = guild;
	mob_index[real_mobile(3021)].func = guild;
	mob_index[real_mobile(3022)].func = guild;
	mob_index[real_mobile(3023)].func = guild;

	mob_index[real_mobile(3024)].func = guild_guard;
	mob_index[real_mobile(3025)].func = guild_guard;
	mob_index[real_mobile(3026)].func = guild_guard;
	mob_index[real_mobile(3027)].func = guild_guard;

	mob_index[real_mobile(3143)].func = mayor;

	/* MORIA */
	mob_index[real_mobile(4000)].func = snake;
	mob_index[real_mobile(4001)].func = snake;

	mob_index[real_mobile(4053)].func = snake;

	mob_index[real_mobile(4103)].func = thief;
	mob_index[real_mobile(4100)].func = magic_user;
	mob_index[real_mobile(4102)].func = snake;

	mob_index[real_mobile(4150)].func = bat_blue;
	mob_index[real_mobile(4151)].func = bat_red;
	mob_index[real_mobile(4152)].func = bat_green;
	mob_index[real_mobile(4153)].func = bat_white;
	mob_index[real_mobile(4154)].func = bat_black;
	mob_index[real_mobile(4155)].func = thief;
	mob_index[real_mobile(4157)].func = magic_user;
	mob_index[real_mobile(4158)].func = snake;


	/* SEWERS */
	mob_index[real_mobile(7006)].func = snake;
	mob_index[real_mobile(7040)].func = bat_red;

	/* FOREST */
	mob_index[real_mobile(6113)].func = snake;
	mob_index[real_mobile(6114)].func = snake;

	mob_index[real_mobile(6910)].func = magic_user;

	boot_the_shops();
	assign_the_shopkeepers();
}



/* assign special procedures to objects */
void assign_objects(void)
{
	int board(struct char_data *ch, int cmd, char *arg);

	obj_index[real_object(3099)].func = board;
}



/* assign special procedures to rooms */
void assign_rooms(void)
{
	int dump(struct char_data *ch, int cmd, char *arg);
	int chalice(struct char_data *ch, int cmd, char *arg);
	int kings_hall(struct char_data *ch, int cmd, char *arg);
	int pet_shops(struct char_data *ch, int cmd, char *arg);
	int mar_gate(struct char_data *ch, int cmd, char *arg);
	int pray_for_items(struct char_data *ch, int cmd, char *arg);
	int worm_ritual(struct char_data *ch, int cmd, char *arg);

	world[real_room(5039)].funct = worm_ritual;
	world[real_room(3030)].funct = dump;
	world[real_room(3054)].funct = pray_for_items;

/*
	world[real_room(704)].funct = chalice;
	world[real_room(2518)].funct = kings_hall;
*/

	world[real_room(3031)].funct = pet_shops;
	world[real_room(8001)].funct = mar_gate;
}
