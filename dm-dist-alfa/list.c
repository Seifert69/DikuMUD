#include <stdio.h>
#include <time.h>

#include "structs.h"

#define GET_LEVEL_TEXT(l)   \
  (l == 0 ? "NEW" :         \
	(l < 4  ? "LOW" :         \
	(l < 7  ? "MED" :         \
	(l < 10 ? "EXP" :         \
	(l < 16 ? "ADV" :         \
	(l < 20 ? "HIG" :         \
	(l ==20 ? "SUP" :         \
	(l < 24 ? "IMM" : "WIZ"))))))))

#define GET_CLASS_TEXT(c)                \
	(c == CLASS_MAGIC_USER ? "<MAGIC> " :  \
	(c == CLASS_CLERIC ?     "<CLERIC>" :  \
	(c == CLASS_THIEF ?      "<THIEF>"  :  \
	(c == CLASS_WARRIOR ?    "<FIGHT>"  : "<UNDEF>"))))

void list(char *filename);

main(int argc, char **argv)
{
	if (argc != 2)
		fprintf(stderr, "Usage: %s <DikuMUD playerfile-name>\n", argv[0]);
	else
		list(argv[1]);
}


void list(char *filename)
{
	FILE *fl;
	struct char_file_u buf;
	char *point;
	int num, days;
	long played;

	if (!(fl = fopen(filename, "rb")))
	{
		perror(filename);
		exit(1);
	}


	played = 0;

	for (num=1;;num++)
	{
		fread(&buf, sizeof(buf), 1, fl);
		if (feof(fl))
			break;

		printf("[%3d] %-20s  ", num, buf.name);
		printf("<%s %2d>    ", GET_LEVEL_TEXT(buf.level), buf.level);
		printf("%-8s  ", GET_CLASS_TEXT(buf.class));

		days = (time(0)-buf.last_logon)/SECS_PER_REAL_DAY;

		if (days >= 7)
			printf("[DAY %2d]  ", days);
		else
			printf("[--- --]  ");
		printf("[%s]", buf.pwd);
		printf("\n");

		played += buf.played;
/*
		for (point = buf.name; *point; ++point)
			printf("'%c':%d ", *point, *point);
		putchar('\n');
*/			
	}

	fclose(fl);

	printf("\nGrand total time played is %d seconds\n", played);
	printf("   (First started on Sat Feb  2 19:20:55 1991)\n\n");
	printf("   Equals %d hours\n", (played/SECS_PER_REAL_HOUR));
	printf("   Equals %d days\n", (played/SECS_PER_REAL_DAY));
}
