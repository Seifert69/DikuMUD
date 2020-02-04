/* Present a message on a port */

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>

void watch(int port, char *text);
void wave(int sock, char *text);
int new_connection(int s);
int init_socket(int port);
int write_to_descriptor(int desc, char *txt);
void nonblock(int s);




main(int argc, char **argv)
{
	int port;
	char txt[2048], buf[83];
	FILE *fl;

	if (argc != 3)
	{
		fputs("Usage: sign (<filename> | - ) <port #>\n", stderr);
		exit();
	}
	if (!strcmp(argv[1], "-"))
	{
		fl = stdin;
		puts("Input text (terminate with ^D)");
	}
	else if (!(fl = fopen(argv[1], "r")))
	{
		perror(argv[1]);
		exit();
	}
	for (;;)
	{
		fgets(buf, 81, fl);
		if (feof(fl))
			break;
		strcat(buf, "\r");
		if (strlen(buf) + strlen(txt) > 2048)
		{
			fputs("String too long\n", stderr);
			exit();
		}
		strcat(txt, buf);
	}
	if ((port = atoi(argv[2])) <= 1024)
	{
		fputs("Illegal port #\n", stderr);
		exit();
	}
	watch(port, txt);
}




void watch(int port, char *text)
{
	int mother;
	fd_set input_set;

	mother = init_socket(port);

	FD_ZERO(&input_set);
	for(;;)
	{
		FD_SET(mother, &input_set);
		if (select(64, &input_set, 0, 0, 0) < 0)
		{
			perror("select");
			exit();
		}
		if (FD_ISSET(mother, &input_set))
			wave(mother, text);
	}
}



void wave(int sock, char *text)
{
	int s;

	if ((s = new_connection(sock)) < 0)
		return;

	write_to_descriptor(s, text);
	sleep(6);
	close(s);
}



int new_connection(int s)
{
	struct sockaddr_in isa;
	/* struct sockaddr peer; */
	int i;
	int t;
	char buf[100];

	i = sizeof(isa);
	getsockname(s, &isa, &i);


	if ((t = accept(s, &isa, &i)) < 0)
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
		sprintf(buf, "New connection from addr %s\n", peer.sa_data);
		log(buf);
	}

	*/

	return(t);
}






int init_socket(int port)
{
	int s;
	char *opt;
	char hostname[1024];
	struct sockaddr_in sa;
	struct hostent *hp;
	struct linger ld;

	bzero(&sa, sizeof(struct sockaddr_in));
	gethostname(hostname, 1023);
	hp = gethostbyname(hostname);
	if (hp == NULL)
	{
		perror("gethostbyname");
		exit();
	}
	sa.sin_family = hp->h_addrtype;
	sa.sin_port	= htons(port);
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) 
	{
		perror("Init-socket");
		exit();
 	}
	if (setsockopt (s, SOL_SOCKET, SO_REUSEADDR,
		(char *) &opt, sizeof (opt)) < 0) 
	{
		perror ("setsockopt REUSEADDR");
		exit ();
	}

	ld.l_onoff = 1;
	ld.l_linger = 1000;
	if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ld, sizeof(ld)) < 0)
	{
		perror("setsockopt LINGER");
		exit();
	}
	if (bind(s, &sa, sizeof(sa), 0) < 0)
	{
		perror("bind");
		close(s);
		exit();
	}
	listen(s, 5);
	return(s);
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




void nonblock(int s)
{
	if (fcntl(s, F_SETFL, FNDELAY) == -1)
	{
		perror("Noblock");
		exit();
	}
}
