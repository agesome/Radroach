#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFSZ 4096

static char inbuf[BUFSZ];

static void
die(char *why)
{
  (void) puts(why);
  exit(EXIT_FAILURE);
}

static void
logstr(char *what)
{
  (void) puts(what);
}

static void
s_connect(char *host, int *sock)
{
  struct addrinfo *server, hints;
  
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  logstr("Creating socket...\n");
  if( (*sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    die("Socket creation failed.\n");
  logstr("Created.\nResolving hostname...\n");
  if( printf("Returned: %d\n", getaddrinfo(host, "ircd", &hints, &server)) )
    die("Failed to resolve hostname.\n");
  logstr("Resolved.\nConecting...\n");
  if( connect(*sock, server->ai_addr, server->ai_addrlen) == -1)
    die("Connection to server failed.\n");
  logstr("Connected.\n");

  freeaddrinfo(server);
}

static void
raw(int sock, char *str)
{
  (void) write(sock, str, strlen(str));
}

static int
sread(int sock)
{
  int nread = 0;
  memset(inbuf, 0, BUFSZ);
  nread = (int) read(sock, inbuf, BUFSZ);
  inbuf[nread + 1] = '\0';
  return nread;
}
     
static void
setup(int sock)
{
  raw(sock, "USER CBot localhost irc.cluenet.org :CBot - a bot in C, by age\n");
}

int
main(void)
{
  int sock, setup_done = 0, lastread = 1;
  char *l;

  s_connect("irc.cluenet.o", &sock);
  (void) sleep(2);
  raw(sock, "NICK CBot\n");
  while(lastread != 0)
    {
      (void) sleep(1);
      lastread = sread(sock);
      l = strtok(inbuf, "\n");
      while(l)
	{
	  if(strstr(l, "PING ") != NULL)
	    {
	      (void) puts(l);
	      memcpy(l, "PONG", 4);
	      raw(sock, l);
	      (void) puts(l);
	      if( setup_done == 0)
		{
		  (void) sleep(1);
		  setup(sock);
		  setup_done = 1;
		}
	    }
	  else
	    {
	      (void) puts(l);
	    }
	  l = strtok(NULL, "\n");
	}
    }
  return EXIT_SUCCESS;
}
