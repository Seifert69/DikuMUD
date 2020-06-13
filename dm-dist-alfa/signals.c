/* ************************************************************************
*  file: signals.c , trapping of signals from Unix.       Part of DIKUMUD *
*  Usage : Signal Trapping.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <signal.h>
#include <stdio.h>
#include <sys/time.h>

#include "utils.h"

extern void log_message(char *msg);

int checkpointing(void);
int shutdown_request(void);
int logsig(void);
int hupsig(void);

void signal_setup(void)
{
	struct itimerval itime;
	struct timeval interval;

	signal(SIGUSR2, shutdown_request);

	/* just to be on the safe side: */

	signal(SIGHUP, hupsig);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, hupsig);
	signal(SIGALRM, logsig);
	signal(SIGTERM, hupsig);

	/* set up the deadlock-protection */

	interval.tv_sec = 900;    /* 15 minutes */
	interval.tv_usec = 0;
	itime.it_interval = interval;
	itime.it_value = interval;
	setitimer(ITIMER_VIRTUAL, &itime, 0);
	signal(SIGVTALRM, checkpointing);
}



void checkpointing(int ignored)
{
	extern int tics;
	
	if (!tics)
	{
		log_message("CHECKPOINT shutdown: tics not updated");
		abort();
	}
	else
		tics = 0;
}




void shutdown_request(int ignored)
{
	extern int shutting_down;

	log_message("Received USR2 - shutdown request");
	shutting_down = 1;
}



/* kick out players etc */
int hupsig(void)
{
	extern int shutting_down;

	log_message("Received SIGHUP, SIGINT, or SIGTERM. Shutting down");
	exit(0);   /* something more elegant should perhaps be substituted */
}



int logsig(void)
{
	log_message("Signal received. Ignoring.");
}
