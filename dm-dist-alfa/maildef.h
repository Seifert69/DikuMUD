#define CHARSIZE sizeof(char)
#define LONGSIZE sizeof(long)


/* BLOCKSIZE must be greater than NAMESIZE * 2 + LONGSIZE * 2 + CHARSIZE */
/* 32 is the smallest BLOCKSIZE to use if NAMESIZE == 11, LONGSIZE == 4,
   and CHARSIZE == 1. */
/*	If your players write mostly short mails you may want to have a
	small blocksize and the other way around. Unfortunately it has not
        been tested what blocksize would be the most effective so I can't
        really tell. However I can imagine it'd be somewhere between 70-80
        and 150 if you're interested in making the mailfile as small as possible.
        If you're only interested in speed at game reboot and a slight increase
        in speed when writing/reading/deleting mails you could use a larger
        blocksize but the mailfile will get bigger. Well if you have the disk
	space...Try 150-500 bytes/block and see what happens. Remember that
	if you use a blocksize of 500, no mail will occupy less than 500 bytes.
	But as I said the optimal setting depends on how large your average
	mails are and also how much the mailsizes vary.
*/

#define BLOCKSIZE 114
#define NAMESIZE 11
/* NAMESIZE should perhaps be 12 instead */
#define HEADBLOCKSIZE (BLOCKSIZE-((LONGSIZE*2)+CHARSIZE+(NAMESIZE*2)))
#define DATABLOCKSIZE (BLOCKSIZE-(LONGSIZE+CHARSIZE))
#define OFFSETSIZE sizeof(struct mail_offset_struct)
#define FREESIZE sizeof(struct free_list_struct)
#define INDEXSIZE sizeof(struct mail_index_struct)

/* MAX_MAIL_AGE == 2 months (5184000 seconds) */

#define MAX_MAIL_AGE 5184000
#define MAILFILE "/usr/users/groo/mailsys/hej.x"


struct head_block {
	char the_mean_byte;
	long date;
	char to[NAMESIZE], from[NAMESIZE];
        char msg[HEADBLOCKSIZE];
        long offset;
};


struct dblock {
        char the_mean_byte;
        char msg[DATABLOCKSIZE];
        long offset;
};


struct free_list_struct {
        long zero_offset;
        struct free_list_struct *next;
};


struct mail_offset_struct {
        long mail_header_offset;
        char from[NAMESIZE];
        struct mail_offset_struct *my_next_mail;
};


struct mail_index_struct {
        char to[NAMESIZE];
        struct mail_offset_struct *my_first_mail;
        struct mail_index_struct *next;
};

