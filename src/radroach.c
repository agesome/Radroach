/* Radroach is a simple IRC bot
   Copyright © 2009  Evgeny Grablyk <evgeny.grablyk@gmail.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <radroach.h>
/* Refers to global configuration structure. */
settings_t *settings = NULL;
void setup (void);
void logstr (char *, ...);

void *
malloc_wrapper (size_t size)
{
  void *mem;

  mem = malloc (size);
  if (mem == NULL)
    {
      fprintf (stderr, "%s: failed to allocate %d bytes, exiting",
          settings->execname, (unsigned int) size);
      exit (EXIT_FAILURE);
    }
  if (DEBUG)
    logstr ("allocated %d bytes at %p\n", size, mem);
  return mem;
}

void
free_wrapper (void *mem)
{
  if (DEBUG)
    logstr ("freeing memory at %p\n", mem);
  free (mem);
}

#define malloc(x) malloc_wrapper(x)
#define free(x) free_wrapper(x)
#include <util.c>
#include <plugins.c>

/* Returns a line from irc server. */
char *
sogetline ()
{
  /* irc RFC specifies maximum message length of 512 chars, plus one for \0 */
  char buffer[513], *line;
  size_t len;

  if (feof (settings->socket) || ferror (settings->socket))
    return NULL;
  fgets (buffer, 513, settings->socket);
  len = strlen (buffer);
  line = malloc (len);
  strncpy (line, buffer, len);
  line[len - 1] = '\0';
  if (settings->verbose)
    logstr ("-> %s\n", line);
  return line;
}

/* free(), but for message struct. */
void
msgfree (message_t * msg)
{
  free (msg->raw);
  free (msg);
}

/* Checks if there's a ping request, replies if there is. Additionally calls setup(). */
int
p_response (char *l)
{
  if (strstr (l, "PING ") != NULL)
    {
      memcpy (l, "PONG", 4);
      raw (l);
      free (l);
      return 1;
    }
  return 0;
}

/* Perform connection to server. */
void
server_connect (char *host)
{
  struct addrinfo *server = NULL, hints;
  int st, sock;

  memset (&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  sock = socket (AF_INET, SOCK_STREAM, 0);
  if (sock == -1)
    {
      logstr ("socket creation failed\n");
      goto err;
    }
  st = getaddrinfo (host, "6667", &hints, &server);
  if (st != 0)
    {
      logstr("failed to resolve hostname\n");
      goto err;
    }
  logstr ("hostname resolved\n");
  st = connect (sock, server->ai_addr, server->ai_addrlen);
  if (st == -1)
    {
      logstr("connection to server failed\n");
      goto err;
    }
  logstr ("connected\n");
  settings->socket = fdopen (sock, "r+");
  freeaddrinfo (server);
  
  return;
  
err:
  if (server)
    freeaddrinfo (server);
  exit (EXIT_FAILURE);
}

/* Performs after-connection actions. */
void
setup (void)
{
  char *l = NULL;

  l = malloc (strlen ("NICK \n") + strlen (settings->nick) + 1);
  sprintf (l, "NICK %s\n", settings->nick);
  raw (l);
  free (l);
  
  if (settings->name)
    {
      l =
	malloc (strlen ("USER  * * :\n") + strlen (settings->name) +
		strlen (settings->nick) + 1);
      sprintf (l, "USER %s * * :%s\n", settings->nick,
	       settings->name);
      raw (l);
    }
  else
    {
      l =
	malloc (strlen ("USER  localhost  :Radroach\n") +
		strlen (settings->nick) + strlen (settings->host) + 1);
      sprintf (l, "USER %s localhost %s :Radroach\n",
	       settings->nick, settings->host);
      raw (l);
    }
  free (l);
  while (!p_response (sogetline ())); /* wait for first ping before attempting to join */
  if (settings->aj_list != NULL)
    {
      char *msg, *chan;
      size_t mlen;

      for (chan = strtok (settings->aj_list, " "); chan != NULL; chan = strtok (NULL, " "))
  	{
  	  mlen = strlen ("JOIN \n") + strlen (chan) + 1;
  	  msg = malloc (mlen);
  	  snprintf (msg, mlen, "JOIN %s\n", chan);
  	  raw (msg);
  	  free (msg);
  	}
    }
}

/*  Parses a string, returns message containing that string's data. */
message_t *
parsemsg (char *l)
{
  /* we only want to parse private messages, which may contain instructions */
  if (strstr (l, " PRIVMSG ") != NULL && l[0] == ':')
    {
      message_t *result = malloc (sizeof (message_t));
      result->raw = l;

      result->sender = &l[1];
      l = strchr (l, '!') + 1;
      *(l - 1) = '\0';

      result->ident = &l[0];
      l = strchr (l, '@') + 1;
      *(l - 1) = '\0';

      result->host = &l[0];
      l = strchr (l, ' ') + 1;
      *(l - 1) = '\0';

      /* we need to skip "PRIVMSG" */
      l = strchr (l, ' ') + 1;
      result->dest = &l[0];
      /* skipping ":" */
      l = strchr (l, ' ') + 2;
      *(l - 2) = '\0';

      /* string ends with \r\n, we want just \0 */
      result->msg = &l[0];
      l = strchr (l, '\r');
      *l = '\0';

      return result;
    }
  return NULL;
}

/* Parses a string, returns command containing string's data. */
command_t *
parsecmd (char *l)
{
  if (l[0] != ' ' && l[1] != ' ')
    {
      command_t *result = malloc (sizeof (command_t));
      result->action = &l[1];

      l = strchr (l, ' ');	/* if there is a space after command name, we may expect parameters */
      if (l != NULL)
	{
	  *l = '\0';
	  l++;
	  result->params = (*l != '\0') ? l : NULL;
	  return result;
	}
      else			/* there are no parameters at all */
	{
	  result->params = NULL;
	  return result;
	}
    }
  return NULL;
}

/* Checks if message is from a trusted source. */
int
checkrights (message_t * msg)
{
  char *l, *s;
  /* just fix me alrady */
  return 1;
  
  s = malloc (2 + strlen (msg->ident) + strlen (msg->host));
  sprintf (s, "%s@%s", msg->ident, msg->host);
  for (l = strtok (settings->trusted, " "); l != NULL; l = strtok (NULL, " "))
    if (strstr (s, l))
      {
	free (s);
	return 1;
      }
  free (s);
  return 0;
}

/* Executes apropriate function as parameters specify. */
void
execute (message_t * msg, command_t * cmd)
{
  unsigned int i;
  
  if (checkrights (msg))
    {
      logstr ("accepted command %s from %s (%s@%s)\n", cmd->action, msg->sender, msg->ident, msg->host);
      for (i = 0; i < plugin_count; i++)
	{
          /* bad performance impact. meh. */
	  if (!strcmp(plugins[i]->name, cmd->action))
          {
            logstr ("plugin %s matched\n", plugins[i]->name);
	    plugins[i]->execute (msg, cmd, 1);
          }
	  else
	    plugins[i]->execute (msg, cmd, 0);
	}
    }
  free (cmd);
  msgfree (msg);
}

/* configures Radroach */
int
configure (int argc, char *argv[])
{
  char *cfile = NULL;
  int opt, status;
  cfg_t *cfg = NULL;
  cfg_opt_t opts[] = {
    CFG_SIMPLE_STR ("nick", &settings->nick),
    CFG_SIMPLE_STR ("name", &settings->name),
    CFG_SIMPLE_STR ("host", &settings->host),
    CFG_SIMPLE_STR ("trusted", &settings->trusted),
    CFG_SIMPLE_STR ("password", &settings->password),
    CFG_SIMPLE_STR ("join", &settings->aj_list),
    CFG_END ()
  };
  
  if (argc < 2)
    {
      print_usage ();
      return 1;
    }
  while ((opt = getopt (argc, argv, "vhc:")) != -1)
    {
      switch (opt)
	{
	case 'h':
	  {
	    print_usage ();
	    continue;
	  }
	case 'c':
	  {
	    cfile = optarg;
	    continue;
	  }
	case 'v':
	  {
	    settings->verbose = true;
	  }
	case '?':
	default:
	  continue;
	}
    }
  if (cfile == NULL)
    {
      logstr ("no confguration file specified\n");
      return 1;
    }

  logstr ("configuration file selected: %s; will now parse\n", cfile);
  
  /* default values */
  settings->nick = NULL;
  settings->name = NULL;
  settings->host = NULL;
  settings->trusted = NULL;
  settings->password = NULL;
  settings->aj_list = NULL;

  cfg = cfg_init (opts, 0);
  status = cfg_parse (cfg, cfile);
  /* check for necessary settings */
  if (status != CFG_SUCCESS || settings->nick == NULL || settings->host == NULL)
    {
      logstr("parsing configuration failed\n");
      return 1;
    }
  return 0;
}

void
sighandler (int sig)
{
  switch (sig)
    {
    case SIGABRT:
      logstr ("\tSIGABRT\n");
      break;
    case SIGBUS:
      logstr ("\tSIGBUS\n");
      break;
    case SIGHUP:
      logstr ("\tSIGHUP\n");
      break;
    case SIGINT:
      logstr ("\tSIGINT\n");
      break;
    case SIGKILL:
      logstr ("\tSIGKILL\n");
      break;
    case SIGQUIT:
      logstr ("\tSIGQUIT\n");
      break;
    case SIGTERM:
      logstr ("\tSIGTERM\n");
      break;
    case 0:
      logstr ("exiting, goodbye\n");
      break;
    default:
      break;
    }

  if (settings->socket)
    {
      fclose (settings->socket);
      settings->socket = NULL;
    }
  plugins_unload ();
  exit (EXIT_SUCCESS);
}

void
register_signals (void)
{
  struct sigaction action;

  action.sa_handler = sighandler;
  sigemptyset (&action.sa_mask);
  action.sa_flags = 0;
  
  sigaction (SIGINT, &action, NULL);
  sigaction (SIGTERM, &action, NULL);
  
  atexit ((void (*)(void)) sighandler);
}

int
main (int argc, char *argv[])
{
  char *l = NULL;
  message_t *cmsg = NULL;
  command_t *ccmd = NULL;
  settings_t global_settings;

  settings = &global_settings;
  settings->execname = argv[0];
  settings->action_trigger = '`';	/* default trigger char */
  settings->verbose = false;
  
  logstr ("Radroach here\n");
  register_signals ();
  if (configure (argc, argv))
    exit (EXIT_FAILURE);
  logstr ("trusted users are: %s\n", settings->trusted);
  if (settings->aj_list)
    logstr ("channels to join: %s\n", settings->aj_list);
  server_connect (settings->host);
  setup ();
  plugins_load ("./plugins/");

  while ((l = sogetline ()) != NULL)
    {
      if (p_response (l))
	continue;
      cmsg = parsemsg (l);
      if (cmsg != NULL)
        {
          ccmd = parsecmd (cmsg->msg);
          if (ccmd != NULL)
            execute (cmsg, ccmd);
        }
      else
	free (l);
    }
  return EXIT_SUCCESS;
}
