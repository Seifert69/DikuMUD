/* ************************************************************************
*  file: comm.c , Communication module.                   Part of DIKUMUD *
*  Usage: Communication, central game loop.                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
*  All Rights Reserved                                                    *
*  Using *any* part of DikuMud without having read license.doc is         *
*  violating our copyright.
************************************************************************* */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"

#define DFLT_PORT 4000        /* default port */
#define MAX_NAME_LENGTH 15
#define MAX_HOSTNAME   256
#define OPT_USEC 250000       /* time delay corresponding to 4 passes/sec */



extern int errno;    /* Why isn't this done in errno.h on alfa??? */

/* externs */

/* extern struct char_data *character_list; */

extern struct room_data *world;	  /* In db.c */
extern int top_of_world;            /* In db.c */
extern struct time_info_data time_info;  /* In db.c */
extern char help[];
extern bool wizlock;

/* local globals */

struct descriptor_data *descriptor_list, *next_to_process;

int lawful = 0;		/* work like the game regulator */
int slow_death = 0;  /* Shut her down, Martha, she's sucking mud */
int shutting_down = 0;    /* clean shutdown */
int reboot = 0;      /* reboot the game after a shutdown */
int no_specials = 0; /* Suppress ass. of special routines */

int maxdesc, avail_descs;
int tics = 0;        /* for extern checkpointing */

int get_from_q(struct txt_q *queue, char *dest);
/* write_to_q is in comm.h for the macro */
int run_the_game(int port);
int game_loop(int s);
int init_socket(int port);
int new_connection(int s);
int new_descriptor(int s);
int process_output(struct descriptor_data *t);
int process_input(struct descriptor_data *t);
void close_sockets(int s);
void close_socket(struct descriptor_data *d);
struct timeval timediff(struct timeval *a, struct timeval *b);
void flush_queues(struct descriptor_data *d);
void nonblock(int s);
void parse_name(struct descriptor_data *desc, char *arg);


/* extern fcnts */

struct char_data *make_char(char *name, struct descriptor_data *desc);
void boot_db(void);
void zone_update(void);
void affect_update( void ); /* In spells.c */
void point_update( void );  /* In limits.c */
void free_char(struct char_data *ch);
void slog(char *str);
void mobile_activity(void);
void string_add(struct descriptor_data *d, char *str);
void perform_violence(void);
void stop_fighting(struct char_data *ch);
void show_string(struct descriptor_data *d, char *input);
void gr(int s);

void check_reboot(void);

/* *********************************************************************
*  main game loop and related stuff				       *
********************************************************************* */




int main(int argc, char **argv)
{
	int port;
	char buf[512];
	int pos = 1;
	char *dir;

	port = DFLT_PORT;
	dir = DFLT_DIR;

	while ((pos < argc) && (*(argv[pos]) == '-'))
	{
		switch (*(argv[pos] + 1))
		{
			case 'l':
				lawful = 1;
				slog("Lawful mode selected.");
			break;
			case 'd':
				if (*(argv[pos] + 2))
					dir = argv[pos] + 2;
				else if (++pos < argc)
					dir = argv[pos];
				else
				{
					slog("Directory arg expected after option -d.");
					exit(0);
				}
			break;
			case 's':
				no_specials = 1;
				slog("Suppressing assignment of special routines.");
			break;
			default:
				sprintf(buf, "Unknown option -% in argument string.",
					*(argv[pos] + 1));
				slog(buf);
			break;
		}
		pos++;
	}
	
	if (pos < argc)
		if (!isdigit(*argv[pos]))
		{
			fprintf(stderr, "Usage: %s [-l] [-s] [-d pathname] [ port # ]\n", 
				argv[0]);
			exit(0);
		}
		else if ((port = atoi(argv[pos])) <= 1024)
		{
			printf("Illegal port #\n");
			exit(0);
		}

	sprintf(buf, "Running game on port %d.", port);
	slog(buf);

	if (chdir(dir) < 0)
	{
		perror("chdir");
		exit(0);
	}

	sprintf(buf, "Using %s as data directory.", dir);
	slog(buf);

	srandom(time(0));
	run_the_game(port);
	return(0);
}





#define PROFILE(x)


/* Init sockets, run game, and cleanup sockets */
int run_the_game(int port)
{
	int s; 
	PROFILE(extern etext();)

	void signal_setup(void);
	int load(void);
	void coma(int);

	PROFILE(monstartup((int) 2, etext);)

	descriptor_list = NULL;

	slog("Signal trapping.");
	signal_setup();

	slog("Opening mother connection.");
	s = init_socket(port);

	if (lawful && load() >= 6)
	{
		slog("System load too high at startup.");
		coma(s);
	}

	boot_db();

	slog("Entering game loop.");

	game_loop(s);

	close_sockets(s); 

	PROFILE(monitor(0);)

	if (reboot)
	{
		slog("Rebooting.");
		exit(52);            /* what's so great about HHGTTG, anyhow? */
	}

	slog("Normal termination of game.");
}






/* Accept new connects, relay commands, and call 'heartbeat-functs' */
int game_loop(int s)
{
	int tmp_room, old_len;
	fd_set input_set, output_set, exc_set;
	struct timeval last_time, now, timespent, timeout, null_time;
	static struct timeval opt_time;
	char comm[MAX_INPUT_LENGTH];
	struct descriptor_data *t, *point, *next_point;
	int pulse = 0, mask;

	null_time.tv_sec = 0;
	null_time.tv_usec = 0;

	opt_time.tv_usec = OPT_USEC;  /* Init time values */
	opt_time.tv_sec = 0;
	gettimeofday(&last_time, (struct timeval *) 0);

	maxdesc = s;
	avail_descs = getdtablesize() - 2; /* !! Change if more needed !! */

	mask = sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
		sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
		sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP) |
		sigmask(SIGVTALRM);

	/* Main loop */
	while (!shutting_down)
	{
		/* Check what's happening out there */
		FD_ZERO(&input_set);
		FD_ZERO(&output_set);
		FD_ZERO(&exc_set);
		FD_SET(s, &input_set);
		for (point = descriptor_list; point; point = point->next)
		{
			FD_SET(point->descriptor, &input_set);
			FD_SET(point->descriptor, &exc_set);
			FD_SET(point->descriptor, &output_set);
		}

		/* check out the time */
		gettimeofday(&now, (struct timeval *) 0);
		timespent = timediff(&now, &last_time);
		timeout = timediff(&opt_time, &timespent);
		last_time.tv_sec = now.tv_sec + timeout.tv_sec;
		last_time.tv_usec = now.tv_usec + timeout.tv_usec;
		if (last_time.tv_usec >= 1000000)
		{
			last_time.tv_usec -= 1000000;
			last_time.tv_sec++;
		}

		sigsetmask(mask);

		if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) 
			< 0)
		{
			perror("Select poll");
			return(-1);
		}

		if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0)
		{
			perror("Select sleep");
			exit(1);
		}

		sigsetmask(0);

		/* Respond to whatever might be happening */
		
		/* New connection? */
	 	if (FD_ISSET(s, &input_set))
			if (new_descriptor(s) < 0)
				perror("New connection");

		/* kick out the freaky folks */
		for (point = descriptor_list; point; point = next_point)
		{
			next_point = point->next;   
			if (FD_ISSET(point->descriptor, &exc_set))
			{
				FD_CLR(point->descriptor, &input_set);
				FD_CLR(point->descriptor, &output_set);
				close_socket(point);
			}
		}

		for (point = descriptor_list; point; point = next_point) 
		{
			next_point = point->next;
			if (FD_ISSET(point->descriptor, &input_set))
	 			if (process_input(point) < 0) 
					close_socket(point);
 		}

		/* process_commands; */
		for (point = descriptor_list; point; point = next_to_process)
		{
			next_to_process = point->next;

			if ((--(point->wait) <= 0) && get_from_q(&point->input, comm))
			{
				if (point->character && point->connected == CON_PLYNG &&
					point->character->specials.was_in_room !=	NOWHERE)
				{
					if (point->character->in_room != NOWHERE)
						char_from_room(point->character);
					char_to_room(point->character, 
						point->character->specials.was_in_room);
					point->character->specials.was_in_room = NOWHERE;
					act("$n has returned.",	TRUE, point->character, 0, 0, TO_ROOM);
					affect_total(point->character);
				}

				point->wait = 1;
				if (point->character)
					point->character->specials.timer = 0;
				point->prompt_mode = 1;

				if (point->str)
					string_add(point, comm);
				else if (!point->connected) 
					if (point->showstr_point)
						show_string(point, comm);
					else
						command_interpreter(point->character, comm);
				else 
					nanny(point, comm); 
			}
		}


		for (point = descriptor_list; point; point = next_point) 
		{
			next_point = point->next;
			if (FD_ISSET(point->descriptor, &output_set) && point->output.head)
				if (process_output(point) < 0)
					close_socket(point);
				else
					point->prompt_mode = 1;
 		}

		/* give the people some prompts */
		for (point = descriptor_list; point; point = point->next)
			if (point->prompt_mode)
			{
				if (point->str)
					write_to_descriptor(point->descriptor, "] ");
				else if (!point->connected)
					if (point->showstr_point)
						write_to_descriptor(point->descriptor,
							"*** Press return ***");
					else					
						write_to_descriptor(point->descriptor, "> ");
				point->prompt_mode = 0;
			}



		/* handle heartbeat stuff */
		/* Note: pulse now changes every 1/4 sec  */

		pulse++;

		if (!(pulse % PULSE_ZONE))
		{
			zone_update();
			if (lawful)
				gr(s);
		}


		if (!(pulse % PULSE_MOBILE))
			mobile_activity();

		if (!(pulse % PULSE_VIOLENCE))
			perform_violence();

		if (!(pulse % (SECS_PER_MUD_HOUR*4))){
			weather_and_time(1);
			affect_update();
			point_update();
			if ( time_info.hours == 1 )
				update_time();
		}

		if (pulse >= 2400) {
			pulse = 0;
			if (lawful)
				night_watchman();
			check_reboot();
		}

		tics++;        /* tics since last checkpoint signal */
	}
}






/* ******************************************************************
*  general utility stuff (for local use)									 *
****************************************************************** */




int get_from_q(struct txt_q *queue, char *dest)
{
	struct txt_block *tmp;

 	/* Q empty? */
	if (!queue->head)
		return(0);

	tmp = queue->head;
	strcpy(dest, queue->head->text);
	queue->head = queue->head->next;

	free(tmp->text);
	free(tmp);

	return(1);
}




void write_to_q(char *txt, struct txt_q *queue)
{
	struct txt_block *new;

	CREATE(new, struct txt_block, 1);
	CREATE(new->text, char, strlen(txt) + 1);

	strcpy(new->text, txt);

	/* Q empty? */
	if (!queue->head)
	{
		new->next = NULL;
		queue->head = queue->tail = new;
	}
	else
	{
		queue->tail->next = new;
		queue->tail = new;
		new->next = NULL;
	}
}
		






struct timeval timediff(struct timeval *a, struct timeval *b)
{
	struct timeval rslt, tmp;

	tmp = *a;

	if ((rslt.tv_usec = tmp.tv_usec - b->tv_usec) < 0)
	{
		rslt.tv_usec += 1000000;
		--(tmp.tv_sec);
	}
	if ((rslt.tv_sec = tmp.tv_sec - b->tv_sec) < 0)
	{
		rslt.tv_usec = 0;
		rslt.tv_sec =0;
	}
	return(rslt);
}






/* Empty the queues before closing connection */
void flush_queues(struct descriptor_data *d)
{
	char dummy[MAX_STRING_LENGTH];

	while (get_from_q(&d->output, dummy));
	while (get_from_q(&d->input, dummy));
}






/* ******************************************************************
*  socket handling							 *
****************************************************************** */




int init_socket(int port)
{
	int s;
	char hostname[MAX_HOSTNAME+1];
	struct sockaddr_in sa;
	struct hostent *hp;
	struct linger ld;

	bzero(&sa, sizeof(struct sockaddr_in));
	gethostname(hostname, MAX_HOSTNAME);
	hp = gethostbyname(hostname);
	if (hp == NULL)
	{
		perror("gethostbyname");
		exit(1);
	}
	sa.sin_family = hp->h_addrtype;
	sa.sin_port	= htons(port);
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) 
	{
		perror("Init-socket");
		exit(1);
 	}
	int enable = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	{
		perror ("setsockopt REUSEADDR");
		exit (1);
	}

	ld.l_onoff = 1;
	ld.l_linger = 1000;
	if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ld, sizeof(ld)) < 0)
	{
		perror("setsockopt LINGER");
		exit(1);
	}
	if (bind(s, (struct sockaddr *) &sa, sizeof(sa)) < 0)
	{
		perror("bind");
		close(s);
		exit(1);
	}
	listen(s, 3);
	return(s);
}





int new_connection(int s)
{
	struct sockaddr_in isa;
	/* struct sockaddr peer; */
	int i;
	int t;
	char buf[100];

	i = sizeof(isa);
	getsockname(s, (struct sockaddr *) &isa, &i);


	if ((t = accept(s, (struct sockaddr *) &isa, &i)) < 0)
	{
		perror("Accept");
		return(-1);
	}
	nonblock(t);

	/*

	i = sizeof(peer);
	if (!getpeername(t, &peer, &i))
	{
		*(peer.sa_data + 49) = '\0';
		sprintf(buf, "New connection from addr %s.\n", peer.sa_data);
		slog(buf);
	}

	*/

	return(t);
}





int new_descriptor(int s)
{
	int desc;
	struct descriptor_data *newd;
	int size;
	struct sockaddr_in sock;
	struct hostent *from;
	char buf[10];

	if ((desc = new_connection(s)) < 0)
		return (-1);

	if (wizlock)
	{
		write_to_descriptor(desc, "The game is wizlocked...");
		close(desc);
		return(0);
	}

	if ((desc + 1) >= avail_descs)
	{
		write_to_descriptor(desc, "Sorry.. The game is full...\n\r");
		close(desc);
		return(0);
	}
	else
		if (desc > maxdesc)
			maxdesc = desc;

	CREATE(newd, struct descriptor_data, 1);

	/* find info */
	size = sizeof(sock);
	if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0)
	{
		perror("getpeername");
		*newd->host = '\0';
	}
	else if (!(from = gethostbyaddr((char*)&sock.sin_addr,
		sizeof(sock.sin_addr), AF_INET)))
	{
		strcpy(newd->host, inet_ntoa(sock.sin_addr));
	}
	else
	{
		strncpy(newd->host, from->h_name, 49);
		*(newd->host + 49) = '\0';
	}
		
	
	/* init desc data */
	newd->descriptor = desc;
	newd->connected  = 1;
	newd->wait = 1;
	newd->prompt_mode = 0;
	*newd->buf = '\0';
	newd->str = 0;
	newd->showstr_head = 0;
	newd->showstr_point = 0;
	*newd->last_input= '\0';
	newd->output.head = NULL;
	newd->input.head = NULL;
	newd->next = descriptor_list;
	newd->character = 0;
	newd->original = 0;
	newd->snoop.snooping = 0;
	newd->snoop.snoop_by = 0;

	/* prepend to list */

	descriptor_list = newd;

	SEND_TO_Q(GREETINGS, newd);
	SEND_TO_Q("By what name do you wish to be known? ", newd);

	return(0);
}
	




int process_output(struct descriptor_data *t)
{
	char i[MAX_STRING_LENGTH + 1];

	if (!t->prompt_mode && !t->connected)
		if (write_to_descriptor(t->descriptor, "\n\r") < 0)
			return(-1);


	/* Cycle thru output queue */
	while (get_from_q(&t->output, i))
	{  
		if(t->snoop.snoop_by)
		{
			write_to_q("% ",&t->snoop.snoop_by->desc->output);
			write_to_q(i,&t->snoop.snoop_by->desc->output);
		}
		if (write_to_descriptor(t->descriptor, i))
			return(-1);
	}
	
	if (!t->connected && !(t->character && !IS_NPC(t->character) && 
								  IS_SET(t->character->specials.act, PLR_COMPACT)))
		if (write_to_descriptor(t->descriptor, "\n\r") < 0)
			return(-1);

	return(1);
}


int write_to_descriptor(int desc, char *txt)
{
	int sofar, thisround, total;

	total = strlen(txt);
	sofar = 0;

	do
	{
		thisround = write(desc, txt + sofar, total - sofar);
		if (thisround < 0)
		{
			perror("Write to socket");
			return(-1);
		}
		sofar += thisround;
	} 
	while (sofar < total);

	return(0);
}





int process_input(struct descriptor_data *t)
{
	int sofar, thisround, begin, squelch, i, k, flag;
	char tmp[MAX_INPUT_LENGTH+2], buffer[MAX_INPUT_LENGTH + 60];

	sofar = 0;
	flag = 0;
	begin = strlen(t->buf);

	/* Read in some stuff */
	do
	{
		if ((thisround = read(t->descriptor, t->buf + begin + sofar, 
			MAX_STRING_LENGTH - (begin + sofar) - 1)) > 0)
			sofar += thisround;		
		else
			if (thisround < 0)
				if(errno != EWOULDBLOCK)
 					{
					perror("Read1 - ERROR");
					return(-1);
				}
				else
					break;
			else
			{
				slog("EOF encountered on socket read.");
				return(-1);
			}
	}
	while (!ISNEWL(*(t->buf + begin + sofar - 1)));	

	*(t->buf + begin + sofar) = 0;

	/* if no newline is contained in input, return without proc'ing */
	for (i = begin; !ISNEWL(*(t->buf + i)); i++)
		if (!*(t->buf + i))
			return(0);

	/* input contains 1 or more newlines; process the stuff */
	for (i = 0, k = 0; *(t->buf + i);)
	{
		if (!ISNEWL(*(t->buf + i)) && !(flag = (k >= (MAX_INPUT_LENGTH - 2))))
			if(*(t->buf + i) == '\b')	 /* backspace */
				if (k)  /* more than one char ? */
				{
					if (*(tmp + --k) == '$')
						k--;				
					i++;
		  		}
				else
	 				i++;  /* no or just one char.. Skip backsp */
			else
				if (isascii(*(t->buf + i)) && isprint(*(t->buf + i)))
				{
					/* trans char, double for '$' (printf)	*/
					if ((*(tmp + k) = *(t->buf + i)) == '$')
						*(tmp + ++k) = '$';
					k++;
					i++;
		  		}
				else
	 				i++;
		else
		{
			*(tmp + k) = 0;
			if(*tmp == '!')
				strcpy(tmp,t->last_input);
			else
				strcpy(t->last_input,tmp);

			write_to_q(tmp, &t->input);

			if(t->snoop.snoop_by)
				{
					write_to_q("% ",&t->snoop.snoop_by->desc->output);
					write_to_q(tmp,&t->snoop.snoop_by->desc->output);
					write_to_q("\n\r",&t->snoop.snoop_by->desc->output);
				}

			if (flag)
			{
				sprintf(buffer, 
					"Line too long. Truncated to:\n\r%s\n\r", tmp);
				if (write_to_descriptor(t->descriptor, buffer) < 0)
					return(-1);

				/* skip the rest of the line */
				for (; !ISNEWL(*(t->buf + i)); i++);
			}

			/* find end of entry */
			for (; ISNEWL(*(t->buf + i)); i++);

			/* squelch the entry from the buffer */
			for (squelch = 0;; squelch++)
				if ((*(t->buf + squelch) = 
					*(t->buf + i + squelch)) == '\0')
		  			break;
			k = 0;
			i = 0;
		}
	}
	return(1);
}




void close_sockets(int s)
{
	slog("Closing all sockets.");

	while (descriptor_list)
		close_socket(descriptor_list);

	close(s);
}





void close_socket(struct descriptor_data *d)
{
	struct affected_type *af;
	struct descriptor_data *tmp;
	char buf[100];

	close(d->descriptor);
	flush_queues(d);
	if (d->descriptor == maxdesc)
		--maxdesc;

	/* Forget snooping */
	if (d->snoop.snooping)
		d->snoop.snooping->desc->snoop.snoop_by = 0;

	if (d->snoop.snoop_by)
		{
			send_to_char("Your victim is no longer among us.\n\r",d->snoop.snoop_by);
			d->snoop.snoop_by->desc->snoop.snooping = 0;
		}

	if (d->character)
		if (d->connected == CON_PLYNG)
		{
			save_char(d->character, NOWHERE);
			act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);
			sprintf(buf, "Closing link to: %s.", GET_NAME(d->character));
			slog(buf);
			d->character->desc = 0;
		}
		else
		{
			sprintf(buf, "Losing player: %s.", GET_NAME(d->character));
			slog(buf);

			free_char(d->character);
		}
	else
		slog("Losing descriptor without char.");
		

	if (next_to_process == d)		/* to avoid crashing the process loop */
		next_to_process = next_to_process->next;   

	if (d == descriptor_list) /* this is the head of the list */
		descriptor_list = descriptor_list->next;
	else  /* This is somewhere inside the list */
	{
		/* Locate the previous element */
		for (tmp = descriptor_list; (tmp->next != d) && tmp; 
			tmp = tmp->next);
		
		tmp->next = d->next;
	}
	if (d->showstr_head)
		free(d->showstr_head);
	free(d);
}





void nonblock(int s)
{
	if (fcntl(s, F_SETFL, FNDELAY) == -1)
	{
		perror("Noblock");
		exit(1);
	}
}




#define COMA_SIGN \
"\n\r\
DikuMUD is currently inactive due to excessive load on the host machine.\n\r\
Please try again later.\n\r\n\
\n\r\
   Sadly,\n\r\n\
\n\r\
    the DikuMUD system operators\n\r\n\r"


/* sleep while the load is too high */
void coma(int s)
{
	fd_set input_set;
	static struct timeval timeout =
	{
		60, 
		0
	};
	int conn;

	int workhours(void);
	int load(void);

	slog("Entering comatose state.");

	sigsetmask(sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
		sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
		sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP) |
		sigmask(SIGVTALRM));

	while (descriptor_list)
		close_socket(descriptor_list);

	FD_ZERO(&input_set);
	do
	{
		FD_SET(s, &input_set);
		if (select(64, &input_set, 0, 0, &timeout) < 0)
		{
			perror("coma select");
			exit(1);
		}
		if (FD_ISSET(s, &input_set))
		{
			if (load() < 6)
			{
				slog("Leaving coma with visitor.");
				sigsetmask(0);
				return;
			}
			if ((conn = new_connection(s)) >= 0)
			{
				write_to_descriptor(conn, COMA_SIGN);
				sleep(2);
				close(conn);
			}
		}			

		tics = 1;
		if (workhours())
		{
			slog("Working hours collision during coma. Exit.");
			exit(0);
		}
	}
	while (load() >= 6);

	slog("Leaving coma.");
	sigsetmask(0);
}



/* ****************************************************************
*	Public routines for system-to-player-communication				*
**************************************************************** */



void send_to_char(char *messg, struct char_data *ch)
{
		
	if (ch->desc && messg)
		write_to_q(messg, &ch->desc->output);
}




void send_to_all(char *messg)
{
	struct descriptor_data *i;

	if (messg)
		for (i = descriptor_list; i; i = i->next)
			if (!i->connected)
				write_to_q(messg, &i->output);
}


void send_to_outdoor(char *messg)
{
	struct descriptor_data *i;

	if (messg)
		for (i = descriptor_list; i; i = i->next)
			if (!i->connected)
				if (OUTSIDE(i->character))
					write_to_q(messg, &i->output);
}


void send_to_except(char *messg, struct char_data *ch)
{
	struct descriptor_data *i;

	if (messg)
		for (i = descriptor_list; i; i = i->next)
			if (ch->desc != i && !i->connected)
				write_to_q(messg, &i->output);
}



void send_to_room(char *messg, int room)
{
	struct char_data *i;

	if (messg)
		for (i = world[room].people; i; i = i->next_in_room)
			if (i->desc)
				write_to_q(messg, &i->desc->output);
}




void send_to_room_except(char *messg, int room, struct char_data *ch)
{
	struct char_data *i;

	if (messg)
		for (i = world[room].people; i; i = i->next_in_room)
			if (i != ch && i->desc)
				write_to_q(messg, &i->desc->output);
}

void send_to_room_except_two
	(char *messg, int room, struct char_data *ch1, struct char_data *ch2)
{
		  struct char_data *i;

			if (messg)
			  for (i = world[room].people; i; i = i->next_in_room)
				  if (i != ch1 && i != ch2 && i->desc)
					  write_to_q(messg, &i->desc->output);
}



/* higher-level communication */


void act(char *str, int hide_invisible, struct char_data *ch,
	struct obj_data *obj, void *vict_obj, int type)
{
	register char *strp, *point, *i = NULL;
	struct char_data *to;
	char buf[MAX_STRING_LENGTH];

	if (!str)
		return;
	if (!*str)
		return;

	if (type == TO_VICT)
		to = (struct char_data *) vict_obj;
	else if (type == TO_CHAR)
		to = ch;
	else
		to = world[ch->in_room].people;

	for (; to; to = to->next_in_room)
	{
		if (to->desc && ((to != ch) || (type == TO_CHAR)) &&  
			(CAN_SEE(to, ch) || !hide_invisible) && AWAKE(to) &&
			!((type == TO_NOTVICT) && (to == (struct char_data *) vict_obj)))
		{
			for (strp = str, point = buf;;)
				if (*strp == '$')
				{
					switch (*(++strp))
					{
						case 'n': i = PERS(ch, to); break;
						case 'N': i = PERS((struct char_data *) vict_obj, to); break;
						case 'm': i = HMHR(ch); break;
						case 'M': i = HMHR((struct char_data *) vict_obj); break;
						case 's': i = HSHR(ch); break;
						case 'S': i = HSHR((struct char_data *) vict_obj); break;
						case 'e': i = HSSH(ch); break;
						case 'E': i = HSSH((struct char_data *) vict_obj); break;
						case 'o': i = OBJN(obj, to); break;
						case 'O': i = OBJN((struct obj_data *) vict_obj, to); break;
						case 'p': i = OBJS(obj, to); break;
						case 'P': i = OBJS((struct obj_data *) vict_obj, to); break;
						case 'a': i = SANA(obj); break;
						case 'A': i = SANA((struct obj_data *) vict_obj); break;
						case 'T': i = (char *) vict_obj; break;
						case 'F': i = fname((char *) vict_obj); break;
						case '$': i = "$"; break;
						default:
							slog("Illegal $-code to act():");
							slog(str);
							break;
					}
					while (*point = *(i++))
						++point;
					++strp;
				}
				else if (!(*(point++) = *(strp++)))
					break;

			*(--point) = '\n';
			*(++point) = '\r';
			*(++point) = '\0';

			write_to_q(CAP(buf), &to->desc->output);
		}
		if ((type == TO_VICT) || (type == TO_CHAR))
			return;
	}
}
