#include "cbot.h"

#define BUFSZ 4096

static char inbuf[BUFSZ + 1];
static int sock, setup_done = 0;
static settings *conf = NULL;

static void
err(char *str)
{
  fprintf(stderr, "Error: %s\n", str);
}

static void
logstr(char *str)
{
  printf("%s", str);
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
    {
      err("Socket creation failed.");
      goto err;
    }
  logstr("Created.\nResolving hostname...\n");
  if(  getaddrinfo(host, "ircd", &hints, &server) )
    {
      err("Failed to resolve hostname.");
      goto err;
    }
      logstr("Resolved.\nConecting...\n");
  if( connect(*sock, server->ai_addr, server->ai_addrlen) == -1)
    {
      err("Connection to server failed.");
      goto err;
    }
  logstr("Connected.\n");
  freeaddrinfo(server);
  return;
  
 err:
  freeaddrinfo(server);
  exit(EXIT_FAILURE);
}

static void
raw(int sock, char *str)
{
  logstr(str);
  write(sock, str, strlen(str));
}

static int
sread(int sock)
{
  int nread = 0;
  memset(inbuf, 0, BUFSZ);
  nread = (int) read(sock, inbuf, BUFSZ);
  return nread;
}
     
static void
setup(int sock)
{
  char *l;
  if(conf->name)
    {
      l = malloc(strlen("USER  localhost  :\n") + strlen(conf->name) + strlen(conf->nick) + strlen(conf->host));
      sprintf(l, "USER %s localhost %s :%s\n", conf->nick, conf->host, conf->name);
      raw(sock, l);
      free(l);
    }
  else
    {
      l = malloc(strlen("USER   localhost   :CBot - a bot in C, by age\n") + strlen(conf->nick) + strlen(conf->host));
      sprintf(l, "USER %s localhost %s :CBot - a bot in C, by age\n", conf->nick, conf->host);
      raw(sock, l);
      free(l);
    }
}

/* parse a string, return message containing that string's data */
static message *
parsemsg (const char *str)
{
  message *result;
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
      result = (message *) malloc (sizeof (message));
      
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
      free(regex);
      return NULL;
    }
  regfree(regex);
  free(regex);
  return result;
}

char mod = '`';

static command *
parsecmd(char *str)
{
  command *result;
  char msg_regex[] = "^ ([^ ]+) (.*)";
  enum act {RAW, ACT, PARAM };
  regex_t *regex = (regex_t *) malloc (sizeof (regex_t));
  regmatch_t matches[3];

  /* put out trigger char in */
  msg_regex[1] = mod;
  
  regcomp (regex, msg_regex, REG_ICASE | REG_EXTENDED);
  if( regexec (regex, str, 3, matches, 0) == 0)
    {
      result = (command *) malloc (sizeof (command));
      
      result->action = (char *) malloc(matches[ACT].rm_eo - matches[ACT].rm_so + 1);
      memcpy(result->action, &str[matches[ACT].rm_so], matches[ACT].rm_eo - matches[ACT].rm_so);
      result->action[matches[ACT].rm_eo - matches[ACT].rm_so] = '\0';
      
      result->params = (char *) malloc(matches[PARAM].rm_eo - matches[PARAM].rm_so + 1);
      memcpy(result->params, &str[matches[PARAM].rm_so], matches[PARAM].rm_eo - matches[PARAM].rm_so);
      result->params[matches[PARAM].rm_eo - matches[PARAM].rm_so] = '\0';
    }
  else
    {
      regfree(regex);
      free(regex);
      return NULL;
    }
  regfree(regex);
  free(regex);
  
  return result;
}

void
cmdfree(command *cmd)
{
  free(cmd->action);
  free(cmd->params);
  free(cmd);
  cmd = NULL;
}

void
msgfree(message *msg)
{
  free(msg->sender);
  free(msg->ident);
  free(msg->host);
  free(msg->dest);
  free(msg->msg);  
  free(msg);
  msg = NULL;
}

static int
p_response(char *l)
{
  if(strstr(l, "PING ") != NULL)
    {
      puts(l);
      memcpy(l, "PONG", 4);
      raw(sock, l);
      if( setup_done == 0)
	{
	  sleep(1);
	  setup(sock);
	  setup_done = 1;
	}
      return 1;
    }
  return 0;
}

static void
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
  free(s);
}

static action acts[] =
  {
    { "say", "Say something to comamnd source", NULL, &f_say },
    { NULL, NULL, NULL, NULL }	/* so we can detect end of array */
  };

static void
p_help(void)
{
  printf("Usage: [-h] -c confile\n");
}

static settings *
configure(char *cfile)
{
  cfg_t *cfg = NULL;
  settings *conf = (settings *) malloc(sizeof(settings));
  int status;

  cfg_opt_t opts[] = {
    CFG_SIMPLE_STR("nick", &conf->nick),
    CFG_SIMPLE_STR("name", &conf->name),
    CFG_SIMPLE_STR("host", &conf->host),
    CFG_SIMPLE_STR("trusted", &conf->trusted),
    CFG_SIMPLE_STR("password", &conf->password),
    CFG_END()
  };

  /* default values */
  conf->nick = NULL;
  conf->name = NULL;
  conf->host = NULL;
  conf->trusted = NULL;
  conf->password = NULL;

  cfg = cfg_init(opts, 0);
  status = cfg_parse(cfg, cfile);
  /* check for necessary settings */
  if ( status = CFG_SUCCESS && conf->nick != NULL && conf->host != NULL)
    return conf;
  return NULL;
}

int
main(int argc, char *argv[])
{
  int lastread = 1, i, opt;
  char *l, *cfile = NULL;
  message *cmsg = NULL;
  command *ccmd = NULL;

  while ((opt = getopt(argc, argv, "hc:")) != -1)
    {
      switch(opt)
	{
	case 'h':
	  p_help();
	  break;
	case 'c':
	  cfile = optarg;
	  break;
	default:
	  p_help();
	  exit(EXIT_FAILURE);
	}
    }

  if(cfile == NULL)
    {
      fprintf(stderr, "No config file specified.\n");
      exit(EXIT_FAILURE);
    }
  
  printf("Configuration file selected: %s\n", cfile);

  if( (conf = configure(cfile)) == NULL)
    {
      fprintf(stderr, "You did not specify nickname and host or parsing configuration failed :(\n");
      exit(EXIT_FAILURE);
    }

  s_connect(conf->host, &sock);
  sleep(2);
  /* inbuf is not used yet, so it becomes a temporary buffer */
  sprintf(inbuf, "NICK %s\n", conf->nick);
  raw(sock, inbuf);
  
  while(lastread != 0)
    {
      sleep(1);
      lastread = sread(sock);
      l = strtok(inbuf, "\n");
      while(l)
	{
	  if( !p_response(l) && (cmsg = parsemsg(l)) != NULL )
	    {
	      if( (ccmd = parsecmd(cmsg->msg)) != NULL)
		{
		  for (i = 0; acts[i].name != NULL; ++i)
		    if(strstr(acts[i].name, ccmd->action))
		      {
			acts[i].exec(cmsg, ccmd);
			printf("Command '%s' executed with parameters: '%s'\n", acts[i].name, ccmd->params);
		      }
		    else if(acts[i + 1].name == NULL)
		      {
			printf("No handler found for this command: '%s' (supplied parameters: '%s')\n", ccmd->action, ccmd->params);
			break;
		      }
		  cmdfree(ccmd);
		}
	      msgfree(cmsg);
	    }
	  l = strtok(NULL, "\n");
	}
    }
  return EXIT_SUCCESS;
}
