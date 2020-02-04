#include <stdio.h>
#include <time.h>

#include "structs.h"

#define bytemask 0xFF


struct old_char_point_data
{
  sh_int mana;
  sh_int max_mana;     /* Not useable may be erased upon player file renewal */
  sh_int hit;
  sh_int max_hit;      /* Max hit for NPC                         */
  sh_int move;
  sh_int max_move;     /* Max move for NPC                        */

  byte armor;          /* Internal -100..100, external -10..10 AC */
  int gold;            /* Money carried                           */
  int exp;             /* The experience of the player            */

  sbyte hitroll;       /* Any bonus or penalty to the hit roll    */
  sbyte damroll;       /* Any bonus or penalty to the damage roll */
};



struct char_file_old_u
{
  byte sex;
  byte class;
  byte level;
  time_t birth;  /* Time of birth of character     */
  int played;    /* Number of secs played in total */

  ubyte weight;
  ubyte height;

  char title[80];
  sh_int hometown;
  char description[240];
  bool talks[MAX_TOUNGE];

  sh_int load_room;            /* Which room to place char in           */

  struct char_ability_data abilities;

  struct old_char_point_data points;

  struct char_skill_data skills[MAX_SKILLS];

  struct affected_type affected[MAX_AFFECT];

  /* specials */

  byte spells_to_learn;
  int alignment;

  time_t last_logon;  /* Time (in secs) of last logon */
  ubyte act;          /* ACT Flags                    */

  /* char data */
  char name[20];
  char pwd[11];
  sh_int apply_saving_throw[5];
  int conditions[3];
};


void do_it(FILE *src, FILE *trg);

main(int argc, char **argv)
{
	FILE *src, *trg;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s source target\n", argv[0]);
		exit (0);
	} else if (!(src = fopen(argv[1], "rb")))
		fprintf(stderr, "%s: Could not open.\n", argv[1]);
	else if (!(trg = fopen(argv[2], "wb")))
		fprintf(stderr, "%s: Could not open.\n", argv[2]);
	else
		do_it(src, trg);

	fclose(src);
	fclose(trg);

}


void do_it(FILE *src, FILE *trg)
{
	struct char_file_u buf;
	struct char_file_old_u oldbuf;
	int a,b,c,d, n;

	srand((int) time(0));

	n=1;

	for (;;)
	{
		fread(&oldbuf, sizeof(oldbuf), 1, src);
		if (feof(src))
			break;
		/* do something */

		buf.sex = oldbuf.sex;
		buf.class = oldbuf.class;
		buf.level = oldbuf.level;
		buf.birth = oldbuf.birth;      /* Time of birth of character     */
    buf.played = oldbuf.played;    /* Number of secs played in total */

		buf.weight = oldbuf.weight;
		buf.height = oldbuf.height;

		strcpy(buf.title, oldbuf.title);
		buf.hometown = oldbuf.hometown;
		strcpy(buf.description, oldbuf.description);
		buf.talks[0] = oldbuf.talks[0];
		buf.talks[1] = oldbuf.talks[1];
		buf.talks[2] = oldbuf.talks[2];

		buf.load_room = oldbuf.load_room;

		buf.abilities = oldbuf.abilities;

    buf.points.mana = oldbuf.points.mana;
    buf.points.max_mana = oldbuf.points.max_mana;
    buf.points.hit = oldbuf.points.hit;
    buf.points.max_hit = oldbuf.points.max_hit;
    buf.points.move = oldbuf.points.move;
    buf.points.max_move = oldbuf.points.max_move;

    buf.points.armor = 100;
    buf.points.gold = oldbuf.points.gold;
    buf.points.exp = oldbuf.points.exp;

    buf.points.hitroll = 0;
    buf.points.damroll = 0;

		for(n=0; n < MAX_SKILLS; n++)
			buf.skills[n] = oldbuf.skills[n];

		for(n=0; n < MAX_AFFECT; n++)
			buf.affected[n] = oldbuf.affected[n];

		buf.spells_to_learn = oldbuf.spells_to_learn;
		buf.alignment = oldbuf.alignment;

		buf.last_logon = oldbuf.last_logon;
		buf.act = oldbuf.act;

		strcpy(buf.name,oldbuf.name);
		strcpy(buf.pwd, oldbuf.pwd);

		buf.apply_saving_throw[0] = oldbuf.apply_saving_throw[0];
		buf.apply_saving_throw[1] = oldbuf.apply_saving_throw[1];
		buf.apply_saving_throw[2] = oldbuf.apply_saving_throw[2];
		buf.apply_saving_throw[3] = oldbuf.apply_saving_throw[3];
		buf.apply_saving_throw[4] = oldbuf.apply_saving_throw[4];

		buf.conditions[0] = oldbuf.conditions[0];
		buf.conditions[1] = oldbuf.conditions[1];
		buf.conditions[2] = oldbuf.conditions[2];

		/* do something else */
		fwrite(&buf, sizeof(buf), 1, trg);
	}

	printf("Size of buf is %d\n\rSize of oldbuf is %d\n\r",
	  sizeof(buf),
		sizeof(oldbuf));
}
