#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <regex.h>

#define BUFSZ 4096

static char inbuf[BUFSZ + 1];

struct message
{
  char *sender, *ident, *host, *dest, *msg;
};

typedef struct message message;

static void
die(char *why)
{
  puts(why);
  exit(EXIT_FAILURE);
}

static void
logstr(char *what)
{
  puts(what);
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

  logstr("Creating socket...");
  if( (*sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    die("Socket creation failed.");
  logstr("Created.\nResolving hostname...");
  if(  getaddrinfo(host, "ircd", &hints, &server) )
    die("Failed to resolve hostname.");
  logstr("Resolved.\nConecting...");
  if( connect(*sock, server->ai_addr, server->ai_addrlen) == -1)
    die("Connection to server failed.");
  logstr("Connected.");

  freeaddrinfo(server);
}

static void
raw(int sock, char *str)
{
  write(sock, str, strlen(str));
}

static int
sread(int sock)
{
  int nread = 0;
  memset(inbuf, 0, BUFSZ);
  nread = (int) read(sock, inbuf, BUFSZ);
  /* inbuf[nread + 1] = '\0'; */
  return nread;
}
     
static void
setup(int sock)
{
  raw(sock, "USER CBot localhost irc.cluenet.org :CBot - a bot in C, by age\n");
}

/* parse a string, return message containing that string's data */
static message *
parsemsg (const char *str)
{
  message *result = (message *) malloc (sizeof (message));
  enum irc_parts
  { IRC_RAW, IRC_SENDER, IRC_IDENT, IRC_HOST, IRC_DEST, IRC_MESSAGE };
  /* thanks nathan */
  char irc_regex[] = "^:([^!]+)!([^@]+)@([^ ]+) [A-Z]+ ([^ ]+) :(.*)$";
  regex_t *regex = (regex_t *) malloc (sizeof (regex_t));
  regmatch_t matches[6];
  /* regex is tested, so no error checking */
  regcomp (regex, irc_regex, REG_ICASE | REG_EXTENDED);
  if( regexec (regex, str, 6, matches, 0) == 0)
    {
      result->sender =
	(char *) malloc (matches[IRC_SENDER].rm_eo - matches[IRC_SENDER].rm_so + 1);
      memcpy (result->sender, &str[matches[IRC_SENDER].rm_so],
	      matches[IRC_SENDER].rm_eo - matches[IRC_SENDER].rm_so);
      result->sender[matches[IRC_SENDER].rm_eo - matches[IRC_SENDER].rm_so] = '\0';
      result->ident =
	(char *) malloc (matches[IRC_IDENT].rm_eo - matches[IRC_IDENT].rm_so + 1);
      memcpy (result->ident, &str[matches[IRC_IDENT].rm_so],
	      matches[IRC_IDENT].rm_eo - matches[IRC_IDENT].rm_so);
      result->ident[matches[IRC_IDENT].rm_eo - matches[IRC_IDENT].rm_so] = '\0';
      result->host =
	(char *) malloc (matches[IRC_HOST].rm_eo - matches[IRC_HOST].rm_so + 1);
      memcpy (result->host, &str[matches[IRC_HOST].rm_so],
	      matches[IRC_HOST].rm_eo - matches[IRC_HOST].rm_so);
      result->host[matches[IRC_HOST].rm_eo - matches[IRC_HOST].rm_so] = '\0';
      result->dest =
	(char *) malloc (matches[IRC_DEST].rm_eo - matches[IRC_DEST].rm_so + 1);
      memcpy (result->dest, &str[matches[IRC_DEST].rm_so],
	      matches[IRC_DEST].rm_eo - matches[IRC_DEST].rm_so);
      result->dest[matches[IRC_DEST].rm_eo - matches[IRC_DEST].rm_so] = '\0';
      result->msg =
	(char *) malloc (matches[IRC_MESSAGE].rm_eo - matches[IRC_MESSAGE].rm_so + 1);
      memcpy (result->msg, &str[matches[IRC_MESSAGE].rm_so],
	      matches[IRC_MESSAGE].rm_eo - matches[IRC_MESSAGE].rm_so);
      /* there's alro \r in the end, it fscks things up so we cut it */
      result->msg[matches[IRC_MESSAGE].rm_eo - matches[IRC_MESSAGE].rm_so - 1] = '\0';
    }
  else
    {
      regfree(regex);
      return NULL;
    }
  regfree(regex);
  return result;
}

int
main(void)
{
  int sock, setup_done = 0, lastread = 1;
  char *l;
  message *cmsg;
  
  s_connect("irc.cluenet.org", &sock);
  sleep(2);
  raw(sock, "NICK CBot\n");
  while(lastread != 0)
    {
      sleep(1);
      lastread = sread(sock);
      l = strtok(inbuf, "\n");
      while(l)
	{
	  if(strstr(l, "PING ") != NULL)
	    {
	      puts(l);
	      memcpy(l, "PONG", 4);
	      raw(sock, l);
	      puts(l);
	      if( setup_done == 0)
		{
		  sleep(1);
		  setup(sock);
		  setup_done = 1;
		}
	    }
	  else
	    {
	      puts(l);
	      if( (cmsg = parsemsg(l)) != NULL )
		printf("Sender: '%s', ident: '%s', host: '%s', target: '%s', message: '%s'\n",
		       cmsg->sender, cmsg->ident, cmsg->host, cmsg->dest, cmsg->msg);
	    }
	  l = strtok(NULL, "\n");
	}
    }
  return EXIT_SUCCESS;
}
