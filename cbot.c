#include "cbot.h"

#define BUFSZ 4096

static char inbuf[BUFSZ + 1];
int sock;

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
	(char *) malloc (matches[IRC_MESSAGE].rm_eo - matches[IRC_MESSAGE].rm_so);
      memcpy (result->msg, &str[matches[IRC_MESSAGE].rm_so],
	      matches[IRC_MESSAGE].rm_eo - matches[IRC_MESSAGE].rm_so);
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

char mod = '`';

command *
parsecmd(char *str)
{
  command *result = (command *) malloc (sizeof (command));
  char msg_regex[] = "^ ([^ ]+) (.*)";
  enum act {RAW, ACT, PARAM };
  regex_t *regex = (regex_t *) malloc (sizeof (regex_t));
  regmatch_t matches[3];

  /* put out trigger char in */
  msg_regex[1] = mod;
  regcomp (regex, msg_regex, REG_ICASE | REG_EXTENDED);
  if( regexec (regex, str, 3, matches, 0) == 0)
    {
      result->action = (char *) malloc(matches[ACT].rm_eo - matches[ACT].rm_so + 1);
      memcpy(result->action, &str[matches[ACT].rm_so], matches[ACT].rm_eo - matches[ACT].rm_so);
      result->action[matches[ACT].rm_eo - matches[ACT].rm_so] = '\0';
      
      result->params = (char *) malloc(matches[PARAM].rm_eo - matches[PARAM].rm_so);
      memcpy(result->params, &str[matches[PARAM].rm_so], matches[PARAM].rm_eo - matches[PARAM].rm_so);
      result->params[matches[PARAM].rm_eo - matches[PARAM].rm_so] = '\0';
    }
  else
    {
      regfree(regex);
      return NULL;
    }
  regfree(regex);
  return result;
}

action *say;

void
f_say(message *msg, command *cmd)
{
  char *s;
  if( strstr(msg->dest, "CBot") != NULL )
    {
      s = (char *) malloc(12 + strlen(msg->sender) + strlen(cmd->params));
      sprintf(s, "PRIVMSG %s :%s\n", msg->sender, cmd->params);
      raw(sock, s);
    }
  else
    {
      s = (char *) malloc(12 + strlen(msg->dest) + strlen(cmd->params));
      sprintf(s, "PRIVMSG %s :%s\n", msg->dest, cmd->params);
      raw(sock, s);
    }
  free((void *)s);
}

void
fadd(void)
{
/* say */
say = (action *) malloc(sizeof(action));
say->name = "say";
say->desc = "Say something to comamnd source";
say->help = NULL;
say->exec = &f_say;
}

int
main(void)
{
  int setup_done = 0, lastread = 1;
  char *l;
  message *cmsg;
  command *ccmd;

  fadd();
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
		{
		  printf("Sender: '%s', ident: '%s', host: '%s', target: '%s', message: '%s'\n",
			 cmsg->sender, cmsg->ident, cmsg->host, cmsg->dest, cmsg->msg);
		  if( (ccmd = parsecmd(cmsg->msg)) != NULL)
		    {
		      printf("Command found: '%s', with parameters '%s'\n", ccmd->action, ccmd->params);
		      if ( strstr(ccmd->action, say->name) != NULL)
			say->exec(cmsg, ccmd);
		    }
		  else
		    printf("Not a command or parsing failed\n");
		}
	    }
	  l = strtok(NULL, "\n");
	}
    }
  return EXIT_SUCCESS;
}
