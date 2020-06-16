/* ************************************************************************
*  file: reception.c, Special module for Inn's.           Part of DIKUMUD *
*  Usage: Procedures handling saving/loading of player objects            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <sys/time.h>
#include <string.h>

#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "spells.h"

#define OBJ_SAVE_FILE "pcobjs.obj"
#define OBJ_FILE_FREE "\0\0\0"

extern struct room_data *world;
extern struct index_data *mob_index;
extern struct index_data *obj_index;

/* Extern functions */
extern void slog(char *msg);

void store_to_char(struct char_file_u *st, struct char_data *ch);
void do_tell(struct char_data *ch, char *argument, int cmd);
void clear_char(struct char_data *ch);


struct obj_cost {
	int total_cost;
	int no_carried;
	bool ok;
};


/* ************************************************************************
* Routines used for the "Offer"                                           *
************************************************************************* */

void add_obj_cost(struct char_data *ch, struct char_data *re,
                  struct obj_data *obj, struct obj_cost *cost)
{
	char buf[MAX_STRING_LENGTH];

	/* Add cost for an item and it's conents, and next->contents */

	if (obj) {
		if ((obj->item_number > -1) && (cost->ok)) {
			cost->total_cost += MAX(0, obj->obj_flags.cost_per_day);
			cost->no_carried++;
			add_obj_cost(ch, re, obj->contains, cost);
			add_obj_cost(ch, re, obj->next_content, cost);
		} else
			if (cost->ok) {
				act("$n tells you 'I refuse storing $p'",FALSE,re,obj,ch,TO_VICT);
				cost->ok = FALSE;
			}

	}
}


bool recep_offer(struct char_data *ch,	struct char_data *receptionist,
	struct obj_cost *cost)
{
	int i;
	char buf[MAX_STRING_LENGTH];

	cost->total_cost = 100; /* Minimum cost */
	cost->no_carried = 0;
	cost->ok = TRUE; /* Use if any "-1" objects */

	add_obj_cost(ch, receptionist, ch->carrying, cost);

	for(i = 0; i<MAX_WEAR; i++)
		add_obj_cost(ch, receptionist, ch->equipment[i], cost);

	if (!cost->ok)
		return(FALSE);
	
	if (cost->no_carried == 0) {
		act("$n tells you 'But you are not carrying anything?'",FALSE,receptionist,0,ch,TO_VICT);
		return(FALSE);
	}

	if (cost->no_carried > MAX_OBJ_SAVE) {
		sprintf(buf,"$n tells you 'Sorry, but I can't store any more than %d items.",
			MAX_OBJ_SAVE);
		act(buf,FALSE,receptionist,0,ch,TO_VICT);
		return(FALSE);
	}

	sprintf(buf, "$n tells you 'It will cost you %d coins per day'", cost->total_cost);
	act(buf,FALSE,receptionist,0,ch,TO_VICT);

	if (cost->total_cost > GET_GOLD(ch)) {
		if (GET_LEVEL(ch) < 21)
			act("$n tells you 'Which I can see you can't afford'",
			  FALSE,receptionist,0,ch,TO_VICT);
		else {
			act("$n tells you 'Well, since you're a God, I guess it's okay'",
			  FALSE,receptionist,0,ch,TO_VICT);
			cost->total_cost = 0;
		}
	}

	if ( cost->total_cost > GET_GOLD(ch) )
		return(FALSE);
	else
		return(TRUE);
}


/* ************************************************************************
* General save/load routines                                              *
************************************************************************* */

/* pos == 0 is first position in file! */
void update_file(FILE *fl, int pos, struct obj_file_u *st)
{
	if (fseek(fl, sizeof(struct obj_file_u)*(pos), 0)) {
		perror("Seeking in file update (update_file, reception.c)");
		exit(1);
	}
	if (fwrite(st, sizeof(struct obj_file_u), 1, fl) < 1) {
		perror("Error updating file (update_file, reception.c).");
		exit(1);
	}

	/* WHY OH WHY?!??!? I can't tell, but I doesn't work without */
  /* Something with update mode and reads after writes         */
	if (fseek(fl, sizeof(struct obj_file_u)*(pos+1), 0)) {
		perror("Seeking in file update (update_file, reception.c)");
		exit(1);
	}

}


/* ************************************************************************
* Routines used to load a characters equipment from disk                  *
************************************************************************* */

void obj_store_to_char(struct char_data *ch, struct obj_file_u *st)
{
	struct obj_data *obj;
	int i, j;

	void obj_to_char(struct obj_data *object, struct char_data *ch);

	for(i=0; i<MAX_OBJ_SAVE; i++) {
		if (st->objects[i].item_number > -1) {
      if (real_object(st->objects[i].item_number) > -1) {
				obj = read_object(st->objects[i].item_number, VIRTUAL);

				obj->obj_flags.value[0] = st->objects[i].value[0];
				obj->obj_flags.value[1] = st->objects[i].value[1];
				obj->obj_flags.value[2] = st->objects[i].value[2];
				obj->obj_flags.value[3] = st->objects[i].value[3];

				obj->obj_flags.extra_flags = st->objects[i].extra_flags;
				obj->obj_flags.weight      = st->objects[i].weight;
				obj->obj_flags.timer       = st->objects[i].timer;
				obj->obj_flags.bitvector   = st->objects[i].bitvector;

				for(j=0; j<MAX_OBJ_AFFECT; j++)
					obj->affected[j] = st->objects[i].affected[j];

				obj_to_char(obj, ch);
			}
		}
	}
}


void load_char_objs(struct char_data *ch)
{
	FILE *fl;
	int pos, i, j;
	bool found = FALSE;
	float timegold;
	struct obj_file_u st;


	/* r+b is for Binary Reading/Writing */
	if (!(fl = fopen(OBJ_SAVE_FILE, "r+b")))
	{
		perror("Opening object file for Loading PC's objects");
		exit(1);
	}

	pos = 0;

	while (!feof(fl) && !found) {
		pos += fread(&st, sizeof(struct obj_file_u), 1, fl);
		found = !strcasecmp(st.owner, GET_NAME(ch));
	}

	if (found) {

		obj_store_to_char(ch, &st);

		/* To avoid overflow of int */

		timegold =
       (unsigned) (((float) st.total_cost*(float) (time(0) - st.last_update))
                  / (float) (SECS_PER_REAL_DAY));

		GET_GOLD(ch) -= timegold;

		if (GET_GOLD(ch) < 0)
			GET_GOLD(ch) = 0;


		if (fseek(fl, sizeof(struct obj_file_u)*(pos-1), 0)) {
			perror("seeking in PC objects file.");
			exit(1);
		}
		strcpy(st.owner, OBJ_FILE_FREE);
		if (fwrite(&st, sizeof(struct obj_file_u), 1, fl) < 1) {
			slog("Error updating name to be set as unused.");
			exit(1);
		}

	} else {
		slog("Char has no data in file!");
	}

	fclose(fl);

	/* Save char, to avoid strange data if crashing */
	save_char(ch, NOWHERE);

}


/* ************************************************************************
* Routines used to save a characters equipment from disk                  *
************************************************************************* */

/* Puts object in store, at first item which has no -1 */
void put_obj_in_store(struct obj_data *obj, struct obj_file_u *st)
{
	int i, j;
	bool found = FALSE;

	for (i=0; (i<MAX_OBJ_SAVE) && !found; i++)
		if (st->objects[i].item_number == -1) {
			st->objects[i].item_number = obj_index[obj->item_number].virtual;
			st->objects[i].value[0] = obj->obj_flags.value[0];
			st->objects[i].value[1] = obj->obj_flags.value[1];
			st->objects[i].value[2] = obj->obj_flags.value[2];
			st->objects[i].value[3] = obj->obj_flags.value[3];

			st->objects[i].extra_flags = obj->obj_flags.extra_flags;
			st->objects[i].weight  = obj->obj_flags.weight;
			st->objects[i].timer  = obj->obj_flags.timer;
			st->objects[i].bitvector  = obj->obj_flags.bitvector;
			for(j=0; j<MAX_OBJ_AFFECT; j++)
				st->objects[i].affected[j] = obj->affected[j];
			found = TRUE;
		}

	if (!found) {
		slog("No empty space to store object. (put_obj_in_store, reception.c)");
		exit(1);
	}
}



/* Destroy inventory after transferring it to "store inventory" */
void obj_to_store(struct obj_data *obj, struct obj_file_u *st,
                  struct char_data * ch)
{
	static char buf[240];

	if (obj) {
		obj_to_store(obj->contains, st, ch);
		obj_to_store(obj->next_content, st, ch);

		if ((obj->obj_flags.timer < 0) && (obj->obj_flags.timer != OBJ_NOTIMER)) {
			sprintf(buf, "You're told: 'The %s is just old junk, I'll throw it away for you.'\n\r",
			  fname(obj->name));
			send_to_char(buf, ch);
		} else {
			put_obj_in_store(obj, st);
			if (obj->in_obj)
				obj_from_obj(obj);
			extract_obj(obj);
		}
	}
}



/* write the vital data of a player to the player file */
void save_obj(struct char_data *ch, struct obj_cost *cost)
{
	struct obj_file_u st, dummy;
	FILE *fl;
	int pos, i, j;
	bool found = FALSE;


	for(j=0; j<MAX_OBJ_SAVE; j++) {
		st.objects[j].item_number = -1; /* Set as not used */
		st.objects[j].value[0] = 0;
		st.objects[j].value[1] = 0;
		st.objects[j].value[2] = 0;
		st.objects[j].value[3] = 0;
		st.objects[j].extra_flags = 0;
		st.objects[j].weight = 0;
		st.objects[j].timer = 0;

		st.objects[j].bitvector = 0;
		for (i=0; i<MAX_OBJ_AFFECT; i++) {
			st.objects[j].affected[i].location = 0;
			st.objects[j].affected[i].modifier = 0;
		}

	}


	strcpy(st.owner, GET_NAME(ch));
	st.gold_left = GET_GOLD(ch);
	st.total_cost = cost->total_cost;
	st.last_update = time(0);

	obj_to_store(ch->carrying, &st, ch);
	ch->carrying = 0;

	for(i=0; i<MAX_WEAR; i++)
		if (ch->equipment[i]) {
			obj_to_store(ch->equipment[i], &st, ch);
			unequip_char(ch, i);
		}

	if (!(fl = fopen(OBJ_SAVE_FILE, "a+b")))
	{
		perror("saving PC's objects");
		exit(1);
	}


	if (fseek(fl, 0, 0)) /* Move to beginning of file */
	{
		perror("seeking to start of PC objects file.");
		exit(1);
	}

	pos = 0;

	while (!feof(fl) && !found) {
		pos+=fread(&dummy, sizeof(struct obj_file_u), 1, fl);
		found = (dummy.owner[0] == '\0');
	}

	if (feof(fl))
		update_file(fl, pos, &st);
	else {
		if (!found) {
			perror("Really strange...\n\r");
			exit(1);
		}

		update_file(fl, pos-1, &st);
	}

	fclose(fl);
}



/* ************************************************************************
* Routines used to update object file, upon boot time                     *
************************************************************************* */

void update_obj_file(void)
{
	FILE *fl, *char_file;
	struct obj_file_u st;
	struct char_file_u ch_st;
	struct char_data tmp_char;
	int pos, no_read, player_i;
	long days_passed, secs_lost;
	char buf[MAX_STRING_LENGTH];

	int find_name(char *name);
	extern struct player_index_element *player_table;

	pos = 0;

	if (((char_file = fopen(PLAYER_FILE, "r+")) != NULL) && ((fl = fopen(OBJ_SAVE_FILE, "r+b")) != NULL))
	{

		while (!feof(fl)) {
			no_read = fread(&st, sizeof(struct obj_file_u), 1, fl);
			pos += no_read;

			if ((!feof(fl)) && (no_read > 0) && st.owner[0]) {
				sprintf(buf, "   Processing %s[%d].", st.owner, pos);
				slog(buf);
				days_passed = ((time(0) - st.last_update) / SECS_PER_REAL_DAY);
				secs_lost = ((time(0) - st.last_update) % SECS_PER_REAL_DAY);

				if (days_passed > 0) {
					if ((st.total_cost*days_passed) > st.gold_left) {

						if ((player_i = find_name(st.owner)) < 0) {
							perror("   Character not in list. (update_obj_file)");
							exit(1);
						}

						fseek(char_file, (long) (player_table[player_i].nr *
							sizeof(struct char_file_u)), 0);

						fread(&ch_st, sizeof(struct char_file_u), 1, char_file);

						sprintf(buf, "   Dumping %s from object file.", ch_st.name);
						slog(buf);

						ch_st.points.gold = 0;
						ch_st.load_room = NOWHERE;
						fseek(char_file, (long) (player_table[player_i].nr *
							sizeof(struct char_file_u)), 0);
						fwrite(&ch_st, sizeof(struct char_file_u), 1, char_file);

						strcpy(st.owner, OBJ_FILE_FREE);
						update_file(fl, pos-1, &st);

					} else {

						sprintf(buf, "   Updating %s", st.owner);
						slog(buf);
						st.gold_left  -= (st.total_cost*days_passed);
						st.last_update = time(0)-secs_lost;
						update_file(fl, pos-1, &st);
					}
				}
			}
		}

		fclose(fl);
		fclose(char_file);
	}
}


/* ************************************************************************
* Routine Receptionist                                                    *
************************************************************************* */



int receptionist(struct char_data *ch, int cmd, char *arg)
{
	char buf[240];
	struct obj_cost cost;
	struct char_data *recep = 0;
	struct char_data *temp_char;
	sh_int save_room;
	sh_int action_tabel[9] = {23,24,36,105,106,109,111,142,147};

	void do_action(struct char_data *ch, char *argument, int cmd);
	int number(int from, int to);

	if (!ch->desc)
		return(FALSE); /* You've forgot FALSE - NPC couldn't leave */

	for (temp_char = world[ch->in_room].people; (temp_char) && (!recep);
		temp_char = temp_char->next_in_room)
		if (IS_MOB(temp_char))
			if (mob_index[temp_char->nr].func == receptionist)
				recep = temp_char;

	if (!recep) {
		slog("Ingen receptionist.\n\r");
		exit(1);
	}

	if (IS_NPC(ch))
		return(FALSE);

	if ((cmd != 92) && (cmd != 93)) {
		if (!number(0, 30))
			do_action(recep, "", action_tabel[number(0,8)]);
		return(FALSE);
	}

	if (!AWAKE(recep)) {
		act("$e isn't able to talk to you...", FALSE, recep, 0, ch, TO_VICT);
		return(TRUE);
	}

	if (!CAN_SEE(recep, ch)) 
	{
		act("$n says, 'I don't deal with people I can't see!'", FALSE, recep, 0, 0, TO_ROOM);
		return(TRUE);
	}

	if (cmd == 92) { /* Rent  */
		if (recep_offer(ch, recep, &cost)) {

			act("$n stores your stuff in the safe, and helps you into your chamber.",
				FALSE, recep, 0, ch, TO_VICT);
			act("$n helps $N into $S private chamber.",FALSE, recep,0,ch,TO_NOTVICT);

			save_obj(ch, &cost);
			save_room = ch->in_room;
			extract_char(ch);
			ch->in_room = world[save_room].number;
			save_char(ch, ch->in_room);
		}
		
	} else {         /* Offer */
		recep_offer(ch, recep, &cost);
		act("$N gives $n an offer.", FALSE, ch, 0, recep, TO_ROOM);
	}
	
	return(TRUE);
}
