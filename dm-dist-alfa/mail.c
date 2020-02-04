/*********************************
 ** Mail system 1.0 for DikuMUD **
 ** (C)opywrong Groo 1991-04-19 **
 *********************************
 * If you may patch it? Sure but *
 * mail your patches to me when  *
 * you do as I would like to    *
 * stay informed on what happens *
 *********************************
 * Oh yeah, I almost forgot.     *
 * Email : nv89-ogu@nada.kth.se  *
 *********************************/

#include "maildef.h"
#include <stdio.h>
#include <time.h>
#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif


struct mail_index_struct *mail_index;
struct free_list_struct *free_list;
long end_of_file;
FILE *mailfp;


void *mymalloc(size)
int size;
{
	void *the_ptr;
	if ((the_ptr = (void *)malloc(size)) == NULL) {
		fprintf(stderr, "Couldn't allocate %d bytes, exiting.\n",
			size);
	}
	return the_ptr;
}


struct free_list_struct *new_free(offset) 
long offset;
{
	struct free_list_struct *p;
	p = (struct free_list_struct *)mymalloc(FREESIZE);
	p->zero_offset = offset;
	return p;
}


struct mail_offset_struct *new_offset() {
	struct mail_offset_struct *p;
	p = (struct mail_offset_struct *)mymalloc(OFFSETSIZE);
	p->mail_header_offset = 1L;	/* 1 lr knappast bli en vanlig blocksize */
	p->my_next_mail = NULL;
	return p;
}


struct mail_index_struct *new_index(first)
struct mail_offset_struct *first;
{
	struct mail_index_struct *p;
	p = (struct mail_index_struct *)mymalloc(INDEXSIZE);
	p->next = NULL;
	p->my_first_mail = first;
	return p;
}


struct mail_index_struct *get_index_pos(name) 
char name[NAMESIZE];
{

	extern struct mail_index_struct *mail_index;
	struct mail_index_struct *cur;
	cur = mail_index;
	do {
		if (!strcmp(cur->to, name))
			return cur;
	}
	while ((cur = cur->next) != NULL);
	return NULL;
}


void add_free(offset, p)
long offset;
struct free_list_struct *p;
{

	struct free_list_struct *new = new_free(offset);

	while (p->next != NULL && offset > p->next->zero_offset)
		p = p->next;
	if (offset && offset == p->next->zero_offset) {
		fprintf(stderr, "Error. Trying to add free offset %u twice. Ignored.\n",
			offset);
		free(new);
		return;
	}
	new->next = p->next;
	p->next = new;
	return;
}


int add_free_list(offset)
long offset;
{
	extern FILE *mailfp;
	extern struct free_list_struct *free_list;
	unsigned int blocks = 0;

	char the_mean_byte = '\0';

	if (offset == 0) {
		rewind(mailfp);
		fwrite(&the_mean_byte, CHARSIZE, 1, mailfp);
		add_free(offset, free_list);
		fseek(mailfp, DATABLOCKSIZE, SEEK_CUR);
		fread(&offset, LONGSIZE, 1, mailfp);
		blocks++;
	}
	while (offset != NULL && offset != 1) {
		fseek(mailfp, offset, SEEK_SET);
		fwrite(&the_mean_byte, CHARSIZE, 1, mailfp);
		add_free(offset, free_list);
		fseek(mailfp, DATABLOCKSIZE, SEEK_CUR);
		fread(&offset, LONGSIZE, 1, mailfp);
		blocks++;
	}
	return blocks;
}


long get_free_offset() {

	extern struct free_list_struct *free_list;
	struct free_list_struct *p = free_list->next;
	long ret = 1L;

	if (p != NULL) {
		free_list->next = p->next;
		ret = p->zero_offset;
		free(p);
	}
	return ret;
}


/* The first element in mail_index only works as a pointer to the next one */

void add_mail(to, from, offset) 
char to[NAMESIZE], from[NAMESIZE];
long offset;
{

	extern struct mail_index_struct *mail_index;
	struct mail_index_struct *mi = mail_index;
	struct mail_offset_struct *mo;

	/*
	if (offset % BLOCKSIZE) {
	*/
	if (offset % BLOCKSIZE) {
		fprintf(stderr, "Offset value/Blocksize mismatch.\n");
		return;
	}
	while (strncmp(mi->to, to, NAMESIZE)) {/* Check for name in index list */
		if (mi->next == NULL) {					/* else create new index-element */
			mi = (mi->next = new_index(new_offset()));
			strncpy(mi->to, to, NAMESIZE);
			break;
		}
		mi = mi->next;
	}
	mo = mi->my_first_mail;
	if (mo->mail_header_offset != 1) {
		while (mo->my_next_mail != NULL)
			mo = mo->my_next_mail;
		mo = (mo->my_next_mail = new_offset());
	}
	strncpy(mo->from, from, NAMESIZE);
	mo->mail_header_offset = offset;
}


/* REMOVE_OFFSET */
/* Returns NULL in case of an error, 1 if the mail_offset_struct element
	was removed without remarks and 2 if the mail_index_struct element
	was also removed because there were no more mails to the player.
	However, remove_offset() won't (I hope) cause any severe errors so
	there's no immediate need to disable the mailsystem should it return
	a NULL value. */


int remove_offset(cur_ind, cur_off)

struct mail_index_struct *cur_ind;
struct mail_offset_struct *cur_off;

{
	extern struct mail_index_struct *mail_index;
	extern struct free_list_struct *free_list;
	struct mail_index_struct *pmi;
	struct mail_offset_struct *p;
	p = cur_ind->my_first_mail;
	while (p != cur_off) {
		if (p == NULL) {
			fprintf(stderr, "Element to remove not found in list.\n");
			return NULL;
		}
		p = p->my_next_mail;
	}
	if (cur_off == cur_ind->my_first_mail) {
		if (cur_off->my_next_mail == NULL) {
			pmi = mail_index;
			while (pmi->next != cur_ind) {
				if (pmi->next == NULL) {
					fprintf(stderr, "Mail index list element not found.\n");
					return NULL;
				}
				pmi = pmi->next;
			}
			add_free_list(cur_off->mail_header_offset);
			free(cur_off);
			pmi->next = cur_ind->next;
			free(cur_ind);
			return 2;					/* cur_ind removed */
		}
		else {
			cur_ind->my_first_mail = cur_off->my_next_mail;
			add_free_list(cur_off->mail_header_offset);
			free(cur_off);
			return 1;					/* cur_off removed */
		}
	}
	p = cur_ind->my_first_mail;
	while (p->my_next_mail != cur_off) {
		if (p->my_next_mail == NULL) {
			fprintf(stderr, "Unable to find element to remove in list.\n");
			return NULL;
		}
		p = p->my_next_mail;
	}
	p->my_next_mail = cur_off->my_next_mail;
	add_free_list(cur_off->mail_header_offset);
	free(cur_off);
	return 1;							/* cur_off removed (yet another time..) */
}


/* READ_DELETE */
/* read_delete returns a NULL pointer when it either can't allocate memory
	enough to hold the mail someone is trying to read, if it attempts	to read
	beyond EOF or if the header block read isn't marked as a header block.
	In the event of a NULL return it is probably best to disable the mail. */

char *read_delete(recipient)
char recipient[NAMESIZE];
{

	extern FILE *mailfp;
	struct mail_index_struct *pi;
	struct mail_offset_struct *po;
	struct dblock dbl;
	struct head_block hbl;
	static char *text;
	char block[DATABLOCKSIZE];
	long gobbledigook;

	free(text);
	text = (char *)mymalloc((gobbledigook = HEADBLOCKSIZE + NAMESIZE + 50));
	memset(text, '\0', gobbledigook);

	if ((pi = get_index_pos(recipient)) == NULL) {
		fprintf(stderr, "Probably-stupid-postoffice-creator bug.\n");
		return "This mail seems to be anonymous and with no text..?";
	}
	dbl.offset = NULL;
	if ((po = pi->my_first_mail) == NULL) {
		fprintf(stderr, "Mysterious-and-annoying-little-bug error.\n");
		return "You notice this mail has been eaten by a bug!";
	}
	if ((mailfp = fopen(MAILFILE, "rb+")) == NULL) {
		fprintf(stderr, "2 Couldn't open mailfile %s\n", MAILFILE);
		return "Isn't it frustrating when they use invisible ink..?";
	}
	fseek(mailfp, po->mail_header_offset, SEEK_SET);
	fread(&hbl, BLOCKSIZE, 1, mailfp);
	if (hbl.the_mean_byte != 1) {
		fprintf(stderr, "Wrong header block position or corrupt header (%d)\n",
			hbl.the_mean_byte);
		return NULL;
	}
	sprintf(text,
		"Mail from %s, posted %s\n", po->from, ctime(&(hbl.date)));
	strncat(text, hbl.msg, HEADBLOCKSIZE);
	if (hbl.offset != NULL) {
		fseek(mailfp, hbl.offset, SEEK_SET);
		do {
			if(dbl.offset && dbl.offset >= end_of_file) {
				fprintf(stderr, "Attempt to read past EOF: stupid Groo error..\n");
				return NULL;
			}
			dbl.offset = NULL;
			fread(&dbl, BLOCKSIZE, 1, mailfp);
			if ((text = (char *)realloc(text,
				(gobbledigook + DATABLOCKSIZE))) == NULL) {
				fprintf(stderr, "Couldn't realloc %d more bytes for mail.\n",
					DATABLOCKSIZE);
				free(text);
				return NULL;
			}
			gobbledigook += DATABLOCKSIZE;
			strncat(text, dbl.msg, DATABLOCKSIZE);
			fseek(mailfp, dbl.offset, SEEK_SET);
		}
		while (dbl.offset != NULL);
	}
	remove_offset(pi, po);
	/* Note : Initially I made the mail removal routines with the idea that
				 it all would look a more like e.g the standard unix mailsystem
				 or the official lpmud one in the way that a player could give
				 commands for reading, sending and deleting mails and choose
				 if a mail was to be saved even after it'd been read. This means
				 these routines are more intelligent than they have to be right
				 now (hence the way a call to remove_offset looks) but maybe we'd
				 like to change the system later on so I'll keep it this way.
	*/
	fclose(mailfp);
	return text;
}


#ifdef MAX_MAIL_AGE

int delete_old(base)
struct free_list_struct *base;
{

	struct free_list_struct *p = base;
	unsigned int blocks = 0;

	while (p->next != NULL) {
		base = p;
		p = p->next;
		free(base);
		blocks += add_free_list(p->zero_offset);
	}
	return blocks;
}

#endif



/* SCAN_FILE */
/* Call this function once, at game reboot, to setup the memory index lists
	etc for the mailsystem. If you know what you're doing you may call it again
	while the system is up to use another mailfile or something or maybe check
	the current mailfile for errors but then you'd have to free all the linked
	list elements of 'free_list' and 'mail_index' first.
	Scan_file currently also deletes old mails if MAX_MAIL_AGE has been
	defined. If you don't want old mails deleted, just undefine MAX_MAIL_AGE
	in maildef.h.
	If scan_file returns NULL, disable the mailsystem. */

int scan_file(file)
char *file;
{

	extern FILE *mailfp;
	char byte;
	struct head_block *tmp_hbl;
#ifdef MAX_MAIL_AGE
	unsigned int blocks_deleted = 0;
	unsigned int mails_deleted = 0;
	struct free_list_struct *delete_offsets;

#endif
	extern struct free_list_struct *free_list;
	extern struct mail_index_struct *mail_index;
	extern long end_of_file;
	unsigned int free_blocks = 0;
	unsigned int used_blocks = 0;

	long fpos, curtime, no_of_mails = 0;

	end_of_file = 0L;
	time(&curtime);
	tmp_hbl = (struct head_block *)mymalloc(sizeof(struct head_block));

	/* Allocate memory for 1:st element in linked list over free
		blocks in the mailfile. */
	if ((free_list = (struct free_list_struct *)mymalloc(FREESIZE)) == NULL)
		return NULL;
	free_list->next = NULL;

	/* Allocate memory for 1:st element in the mail_index list where each
		element points to a list with mails for a single person.
		(The latter list is really a list of file offsets for all the
		 mails to a specific player) */
	if ((mail_index =
			(struct mail_index_struct *)mymalloc(INDEXSIZE)) == NULL)
			return NULL;
	mail_index->next = NULL;

#ifdef MAX_MAIL_AGE
	delete_offsets = (struct free_list_struct *)mymalloc(FREESIZE);
	delete_offsets->next = NULL;
#endif

	/* Open the mailfile for scanning */
	if ((mailfp = fopen(file, "rb+")) == NULL) {
		fprintf(stderr, "Can't open mailfile. Creating a new one.\n");
		return 1;
	}

	/* Read the first block of the mailfile - this block is supposed
		to be marked as either empty or a header block. */
	fread(tmp_hbl, BLOCKSIZE, 1, mailfp);
	if (tmp_hbl->the_mean_byte != 1 && tmp_hbl->the_mean_byte != 0) {
		fprintf(stderr, "1:st block not a header block byt type %d\n",
			tmp_hbl->the_mean_byte);
		fprintf(stderr, "Mailfile corrupt.\n");
		/* 'One drop of poison infects the whole tun of wine' */
		return NULL;
	}

	/* Copy to, from & offset to curr_offset and mail_index element */
	if (tmp_hbl->the_mean_byte == 1) {
		used_blocks++;
		no_of_mails++;
#ifdef MAX_MAIL_AGE
		if (curtime - tmp_hbl->date > MAX_MAIL_AGE) {
			add_free(0L, delete_offsets);
			mails_deleted++;
		}
		else
#endif
			add_mail(tmp_hbl->to, tmp_hbl->from, 0L);
	}
	else {
		add_free(0L, free_list);
		free_blocks++;
	}

	while (1) {
		/* Read a byte and check if we got one - if not, we're at
			the E of the F */
		if (fread(&byte, sizeof(char), 1, mailfp) != 1) {
				end_of_file = ftell(mailfp);
				break;
		}
		if (byte == 2) {		/* Data block */
			used_blocks++;
			fseek(mailfp, BLOCKSIZE - CHARSIZE, SEEK_CUR);
		}
		else if (byte == 0) {
			/* Allocate new free_list element and
				save file offset (#bytes from the beginning). */
			free_blocks++;
			add_free(ftell(mailfp) - CHARSIZE, free_list);
			fseek(mailfp, BLOCKSIZE - CHARSIZE, SEEK_CUR);
		}
		else if (byte == 1) {		/* Header block */
			/* Read it before it's too late! (Well..) */
			fread((void *)((int)tmp_hbl + 1), BLOCKSIZE - 1, 1, mailfp);

			/*********** MAYBE REMOVE THESE LINES ************/
			tmp_hbl->to[NAMESIZE - 1] = 0;
			tmp_hbl->from[NAMESIZE - 1] = 0;
			/* Maybe better to set NAMESIZE to the actual namesize + 1 */
			/*********** MAYBE REMOVE THESE LINES ************/
			no_of_mails++;
			used_blocks++;

#ifdef MAX_MAIL_AGE
			/* If it is old - delete it */
			if (curtime - tmp_hbl->date > MAX_MAIL_AGE) {
				add_free(ftell(mailfp) - BLOCKSIZE, delete_offsets);
				mails_deleted++;
				continue;
			}
#endif
			/* Add mail to the amazing lists! */
			add_mail(tmp_hbl->to, tmp_hbl->from, ftell(mailfp) - BLOCKSIZE);
		}
		else {
			if (byte != EOF) {
				fprintf(stderr, "Wrong mean_byte error, char was '%c'[%d]\n", byte,
					byte);
				fprintf(stderr, "FPOS %u\n", ftell(mailfp));
				return NULL;
			}
		}
	}
#ifdef MAX_MAIL_AGE
	blocks_deleted = delete_old(delete_offsets);
#endif
	fprintf(stderr,
	"***************************** MAILSYS *****************************\n");
	fprintf(stderr, "Mailfile read : %s, size was %u bytes, ",
		MAILFILE, end_of_file);
	fprintf(stderr, "blocksize %d\nTotal # of unread mails %u\n", BLOCKSIZE,
		no_of_mails);

	fprintf(stderr, "Used blocks : %u (%u bytes)\n",
		used_blocks, (used_blocks * BLOCKSIZE));
	fprintf(stderr, "Free blocks : %u (%u bytes)\n",
		free_blocks, (free_blocks * BLOCKSIZE));
#ifdef MAX_MAIL_AGE
	fprintf(stderr, "Of these, %u was deleted due to old age (%u blocks)\n\n",
		mails_deleted, blocks_deleted);
#endif
	fclose(mailfp);
	return 1;
}



void writeblock(buf, nextpos) 
char *buf;
long nextpos;
{
	extern FILE *mailfp;

	memcpy(buf + BLOCKSIZE - LONGSIZE, &nextpos, LONGSIZE);
	if (fwrite(buf, BLOCKSIZE, 1, mailfp) != 1) {
		fprintf(stderr, "Fwrite at fpos %u failed.\n", ftell(mailfp));
		return;
	}
}


/************************************************************
 * This is store_mail - the current version is so much more *
 * easy and convenient to follow than the previous one that *
 * was a real meanie and also had a few very weird bugs.    *
 ************************************************************/
/* Maybe I should make this one check for errors a bit too. */

void store_mail(to, from, buf) 
char *to, *from, *buf;
{
	FILE *mailfp;
	extern long end_of_file;

	char shovel[BLOCKSIZE];
	char append = '\0';
	long fpos, next_pos, buflen, curtime, buf_index = 0L;

	printf("Storemail.\n");
	memset(shovel, '\0', BLOCKSIZE);
	shovel[0] = 1;
	printf("1..\n");
	time(&curtime);
	memcpy(shovel + CHARSIZE, &curtime, LONGSIZE);
	strncpy(shovel + BLOCKSIZE - DATABLOCKSIZE, to, NAMESIZE);
	strncpy(shovel + BLOCKSIZE - DATABLOCKSIZE + NAMESIZE, from, NAMESIZE);
	strncpy(shovel + BLOCKSIZE - (LONGSIZE + HEADBLOCKSIZE), buf,
		HEADBLOCKSIZE);
	printf("2..\n");
	if (end_of_file == 0) {    /* eof stts av scan_file */
		if ((mailfp = fopen(MAILFILE, "wb+")) == NULL) {
			fprintf(stderr, "Unable to open mailfile.\n");
			return;
		}
		append = 1;
		fpos = 0L;
	}
	else {
		if ((mailfp = fopen(MAILFILE, "rb+")) == NULL) {
			fprintf(stderr, "Unable to open mailfile!\n");
			return;
		}
		if ((fpos = get_free_offset()) == 1) {
			fpos = end_of_file;
			append = 1;
		}
	}

	printf("3..\n");
	buflen = strlen(buf);
	fseek(mailfp, fpos, SEEK_SET);
	printf("4..\n");
/*
	add_mail(to, from, fpos);
*/

	printf("5..\n");
	if (buflen <= HEADBLOCKSIZE) {
		writeblock(shovel, NULL);					/* If it's just a header block */
		if (append)
			end_of_file += BLOCKSIZE;
		return;
	}

	printf("6..\n");
	if (append) {
		next_pos = fpos + BLOCKSIZE;
		end_of_file = next_pos;
	}
	else {
		next_pos = get_free_offset();				/* Get fpos for next headblock */
		if (next_pos == 1) {
			next_pos = end_of_file;
		}
	}

	printf("7..\n");
	writeblock(shovel, next_pos);
	buf_index = HEADBLOCKSIZE;

	shovel[0] = 2;
	printf("8..\n");
	while (buf_index < buflen) {
		if (next_pos == end_of_file) {
			end_of_file += BLOCKSIZE;
			append = 1;
		}
		fpos = next_pos;
		fseek(mailfp, fpos, SEEK_SET);
		strncpy(shovel + CHARSIZE, buf + buf_index, DATABLOCKSIZE);
		buf_index += DATABLOCKSIZE;
		if (buf_index < buflen) {
			if (!append) {
				next_pos = get_free_offset();
				if (next_pos == 1) {
					next_pos = end_of_file;
				}
			}
			else
				next_pos = end_of_file;
		}
		else
			next_pos = NULL;
		writeblock(shovel, next_pos);
	}
	printf("9.\n");
	fclose(mailfp);
}



/***********************************************************************
 * On popular demand I sacrificed a lot of sweat and tears making this *
 * advanced function for finding out whether a certain player has mail *
 *	waiting or not. I expect people, especially Quinn, to be extremely  *
 *	grateful for this one! 															  *
 ***********************************************************************/

int has_mail(who)
char *who;
{
	if (get_index_pos(who) == NULL)
		return 0;
	return 1;
}




/*******************************************************
 * Known bugs :                                        *
 * ------------                             				 *
 *                                                     *
 * Hah!  None whatsoever!  (But then I know little..)  *
 *                                                     *
 *******************************************************


 *******************************************************
 * Known could-be bugs :										 *
 * ---------------------										 *
 *																		 *
 * Well there is one maybe...When I did the most		 *
 * extensive test of the program, entering about 30-40 *
 * mail messages that occupied hundreds of blocks of a *
 * pretty small size (blocksize) I may have discovered *
 * a bug. I used the read_delete function to read and  *
 * delete all the mails, one by one, and in the end    *
 * there were two datablocks left in the mailfile and  *
 * they were marked as being used although they had no *
 * header block. This didn't disturb the function of   *
 * the mail system of course but it made the mailfile  *
 * use two more blocks than needed and thus wasting    *
 * some disk space (well..if it happens many times..). *
 * I am not sure however because I had been copying,	 *
 * renaming and examining the mailfile plus that I had *
 * recompiled the mailsystem several times between the *
 * occasions I entered and read/deleted the various    *
 * mail messages. That may have caused it.             *
 * (It hasn't happened again though I've been testing) *
 * What can be done about it?   Well...				    *
 * a person knowing what he's doing could edit the     *
 * mailfile with a diskeditor and mark the lost blocks *
 * as unused and everything would be just fine. Now if *
 * this really is a bug I'll make a function that does *
 * this automatically, maybe at reboot, and then I'll  *
 * see if I can find the bug. (Tell me all about it)   *
 *  												/Groo	 			 *
 *******************************************************/

