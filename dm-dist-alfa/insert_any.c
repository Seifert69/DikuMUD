/**************************************************************************
*  File: insert_any.c                                     Part of DikuMud *
*  Usage: Merges DikuMud wld/obj/mob/zon files                            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_LINE  256

/*************************************************************************
*  Merge routines                                                        *
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

void merge(FILE *fl1, FILE *fl2)
{
	int antal;
	int eof1, eof2;
	int num1, num2;
	char buf1[MAX_LINE], buf2[MAX_LINE];
	char tbuf1[MAX_LINE], tbuf2[MAX_LINE];

	eof1 = 0;
	eof2 = 0;

	fgets(buf1, MAX_LINE, fl1);
	antal = sscanf(buf1, " #%d ", &num1);
	assume(antal, 1, 0, "No #xxxx found next (old)");

	fgets(buf2, MAX_LINE, fl2);
	antal = sscanf(buf2, " #%d ", &num2);
	assume(antal, 1, 0, "No #xxxx found next (new)");


	fgets(buf1, MAX_LINE, fl1);
	eof1 = (buf1[0] == '$');

	fgets(buf2, MAX_LINE, fl2);
	eof2 = (buf2[0] == '$');

	for (;;) {

		if (eof1) {
			/* Purge file 2 */

			printf("#%d\n", num2);
			printf("%s", buf2);
			while (fgets(buf2, MAX_LINE, fl2))
				printf("%s", buf2);
			return;
		}

		if (eof2) {
			/* Purge file 1 */
			printf("#%d\n", num1);
			printf("%s", buf1);
			while (fgets(buf1, MAX_LINE, fl1))
				printf("%s", buf1);
			return;
		}
			

		/* Merge the rooms */

		if (num1 < num2) {
			printf("#%d\n", num1);
			printf("%s", buf1);
			do {
				fgets(buf1, MAX_LINE, fl1);
				antal = sscanf(buf1, " #%d ", &num1);
				if (antal!=1)
					printf("%s", buf1);
			} while (antal != 1);

			fgets(buf1, MAX_LINE, fl1);
			eof1 = (buf1[0] == '$');

		} else if (num1 == num2) {    /* Replace the room */

			printf("#%d\n", num1);
			printf("%s", buf1);
			do { /* Print from "new" file */
				fgets(buf1, MAX_LINE, fl1);
				antal = sscanf(buf1, " #%d ", &num1);
				if (antal!=1)
					printf("%s", buf1);
			} while (antal != 1);

			do { /* Skip the "old" file */
				fgets(buf2, MAX_LINE, fl2);
				antal = sscanf(buf2, " #%d ", &num2);
			} while (antal != 1);

			fgets(buf1, MAX_LINE, fl1);
			eof1 = (buf1[0] == '$');

			fgets(buf2, MAX_LINE, fl2);
			eof2 = (buf2[0] == '$');

		} else { /* Print a room from "old" file */

			printf("#%d\n", num2);
			printf("%s", buf2);
			do {
				fgets(buf2, MAX_LINE, fl2);
				antal = sscanf(buf2, " #%d ", &num2);
				if (antal!=1)
					printf("%s", buf2);
			} while (antal != 1);

			fgets(buf2, MAX_LINE, fl2);
			eof2 = (buf2[0] == '$');

		}

	}
}


int main(int argc, char *argv[])
{
	FILE *fl_m1, *fl_m2;

  if (argc != 3) {
		printf("Usage : insert_any <New Merge File> <Old Merge File>\n\r");
		printf("Both files must use # numbering system, and terminate with $~\n\r");
		exit(0);
	}

	if (!(fl_m1 = fopen(argv[1], "r")))
	{
		printf("Could not open the builders file.\n\r");
		exit();
	}

	if (!(fl_m2 = fopen(argv[2], "r")))
	{
		printf("Could not open 'old' file.\n\r");
		exit();
	}

	merge(fl_m1, fl_m2);

	fclose(fl_m1);
	fclose(fl_m2);
}
