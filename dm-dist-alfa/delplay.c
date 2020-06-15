#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "structs.h"


#define TOLOWER(c)  (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))


void del(char *filename, int name)
{
	char confirm[80];
	FILE *fl;
	struct char_file_u player;
	int pos, num;
	long end;

	if (!(fl = fopen(filename, "r+")))
	{
		perror("list");
		exit(1);
	}

	puts("Searching for player:");

	for (num = 1, pos = 0;; pos++, num++)
	{
		fread(&player, sizeof(player), 1, fl);
		if (feof(fl))
		{
			fprintf(stderr, "delplay: could not locate %d.\n", name);
			exit(1);
		}

		if (num == name) {
			printf("Confirm deletion of [%s] by typeing Yes: ", player.name);
			scanf("%s", confirm);
			if (strcasecmp("Yes", confirm)) {
				printf("Aborted delete.\n");
				exit(1);
			} else {
				break;
			}
		}

	}

	/* read the last player */
	fseek(fl, -sizeof(player), 2);
	fread(&player, sizeof(player), 1, fl);
	fseek(fl, pos*sizeof(player), 0);
	fwrite(&player, sizeof(player), 1, fl);
	fseek(fl, 0, 2);
	end = ftell(fl);
	fclose(fl);

	if (truncate(filename, end-sizeof(player)))
		perror("truncate");
}

	
int main(int argc, char **argv)
{
	if (argc != 3)
		puts("Usage: delplay <DikuMUD player filename> <Player Number>");
	else {
		if (atoi(argv[2]) < 1)
			puts("Illegal player number, must be >= 1");
		else
			del(argv[1], atoi(argv[2]));
	}
	return 0;
}
