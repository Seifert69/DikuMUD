/* ************************************************************************
*  file: mar_fiz_maz.c, Special module.                   Part of DIKUMUD *
*  Usage: Procedures handling special procedures for the world builders   *
*         Marauder (Dragon), Fizgig (Redferne) and Maze (Quifael)         *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"

/*   external vars  */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;


/* ********************************************************************
*  Special procedures for Marauder                                    *
******************************************************************** */

int mar_gate(struct char_data *ch, int cmd, char *arg)
{
	int i, j;
  struct descriptor_data *desc;
	struct obj_data *obj, *tmp_obj;
	bool punished;

	for (desc = descriptor_list; desc; desc = desc->next) {
		if ((desc->connected == CON_PLYNG) && !IS_NPC(desc->character) &&
		    (desc->character) && (world[desc->character->in_room].number < 8000)) {
			punished = FALSE;

			for(i=0; i<MAX_WEAR; i++) {
				if ((desc->character->equipment[i]) &&
					((j=obj_index[desc->character->equipment[i]->item_number].virtual)>=8500) &&
		       (j<8999) ) {
					obj=unequip_char(desc->character, i);
					GET_GOLD(desc->character) -= obj->obj_flags.cost;
					extract_obj(obj); /* Destroy it */
					punished = TRUE;

				}
			}

			for (obj = desc->character->carrying; obj; obj = tmp_obj) {
				tmp_obj = obj->next_content;
				if (((j=obj_index[obj->item_number].virtual)>=8000) &&
		       (j<8999) ) {
					obj_from_char(obj);
					GET_GOLD(desc->character) -= obj->obj_flags.cost;
					extract_obj(obj); /* Destroy it */
					punished = TRUE;
				}
			}

			if (punished) {
				send_to_char("You have been punished by the Gods!\n\r", desc->character);
				GET_GOLD(desc->character) = MAX(0, GET_GOLD(desc->character));
				GET_MOVE(desc->character) = MIN(GET_MOVE(desc->character), 10);
				GET_MANA(desc->character) = 0;
			}

		} /* if a playing player */

	}  /* for */

	return FALSE;
}
