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

#include "radroach.h"
#define BUFSZ 10

char inbuf[BUFSZ];
/*! \brief Non-negative value indicates that setup is already done. ( setup() called) */
int setup_done = 0;
/*! \brief Refers to global configuration structure. */
settings *conf = NULL;

#include "util.c"
#include "plugins.c"

/*! \brief Print short usage instructions. */
void
p_help (void)
{
  printf ("Usage: %s [-h] -c confile\n", conf->execname);
}

/*! \brief Returns a line from irc server.
  @param s socket to read from
  @return a line recieved from server, without the '\\r\\n' in the end */
/* FIXME: store everything in a buffer, give out lines */
char *
sogetline (int s)
{
  char c = 0, *buf, *rebuf;
  int r, i = 0, bsz = BUFSZ;

  buf = malloc (BUFSZ);
  while ((r = read (s, &c, 1)) != -1)
    {
      /* ordinary read */
      if (r)
	buf[i++] = c;
      /* all done, reallocate and return */
      /* string is less than our buffer, reallocate. */
      if (c == '\n' && i < bsz)
	{
	  buf[i] = '\0';
	  rebuf = malloc (strlen (buf) + 1);
	  strcpy (rebuf, buf);
	  free (buf);
	  return rebuf;
	}
      else if (c == '\n' && i == bsz)
	{
	  buf[i - 1] = '\0';
	  return buf;
	}
      /* we've reached buffer limit, reallocate with more space */
      if (i >= bsz)
	{
	  rebuf = malloc (i + BUFSZ);
	  memcpy (rebuf, buf, i);
	  free (buf);
	  buf = rebuf;
	  bsz += BUFSZ;
	}
    }
  /* something failed and we did not return a sting earlier */
  return NULL;

}

/*! \brief free(), but for message struct.
 @param msg pointer to message to be freed */
void
msgfree (message * msg)
{
  free (msg->raw);
  free (msg);
}

/*! \brief Perform connection to server.
  @param host the host to connect to */
void
sconnect (char *host)
{
  struct addrinfo *server = NULL, hints;
  int st, sock;

  memset (&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  if ((sock = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      error (0, 0, "Socket creation failed.\n");
      goto err;
    }
  logstr ("Socket created.\n");
  if ((st = getaddrinfo (host, "ircd", &hints, &server)) != 0)
    {
      error (0, 0, "Failed to resolve hostname: %s", gai_strerror (st));
      goto err;
    }
  logstr ("Hostname resolved.\n");
  if (connect (sock, server->ai_addr, server->ai_addrlen) == -1)
    {
      error (0, 0, "Connection to server failed: %s", gai_strerror (errno));
      goto err;
    }
  logstr ("Connected.\n");
  conf->sock = sock;
  freeaddrinfo (server);

  /* inbuf is not used yet, don't scream. */
  sprintf (inbuf, "NICK %s\n", conf->nick);
  raw (inbuf);

  return;

err:
  if (server)
    freeaddrinfo (server);
  exit (EXIT_FAILURE);
}

/*! \brief Performs after-connection actions. */
void
setup (void)
{
  char *l = NULL;
  if (conf->name)
    {
      l =
	malloc (strlen ("USER  localhost  :\n") + strlen (conf->name) +
		strlen (conf->nick) + strlen (conf->host) + 1);
      sprintf (l, "USER %s localhost %s :%s\n", conf->nick, conf->host,
	       conf->name);
      raw (l);
    }
  else
    {
      l =
	malloc (strlen ("USER  localhost  :CBot - a bot in C, by age\n") +
		strlen (conf->nick) + strlen (conf->host) + 1);
      sprintf (l, "USER %s localhost %s :CBot - a bot in C, by age\n",
	       conf->nick, conf->host);
      raw (l);
    }
  free (l);
}

/*! \brief Parses a string, returns message containing that string's data.
 @param l the string to be parsed
 @return message struct with all the data that input string contained */
message *
parsemsg (char *l)
{
  /* we only want to parse private messages, which may contain instructions */
  if (strstr (l, "PRIVMSG") != NULL && l[0] == ':')
    {
      message *result = malloc (sizeof (message));
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

/*! \brief Parses a string, returns command containing string's data.
 @param l string to be parsed
 @return struct with strting's data */
command *
parsecmd (char *l)
{
  if (l[0] == conf->action_trigger && l[1] != ' ')
    {
      command *result = malloc (sizeof (command));
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

/*! \brief Checks if there's a ping request, replies if there is. Additionally calls setup().
  @param l the string to be checked
  @return 1 if there was a ping request, 0 if there wasn't */
int
p_response (char *l)
{
  if (strstr (l, "PING ") != NULL)
    {
      memcpy (l, "PONG", 4);
      raw (l);
      free (l);
      if (setup_done == 0)
	{
	  setup ();
	  setup_done = 1;
	}
      return 1;
    }
  return 0;
}

/*! \brief Parses a configuration file.
  @param cfile path to the file
  @return positive on succesful parse */
int
parsecfg (char *cfile)
{
  cfg_t *cfg = NULL;
  int status;

  cfg_opt_t opts[] = {
    CFG_SIMPLE_STR ("nick", &conf->nick),
    CFG_SIMPLE_STR ("name", &conf->name),
    CFG_SIMPLE_STR ("host", &conf->host),
    CFG_SIMPLE_STR ("trusted", &conf->trusted),
    CFG_SIMPLE_STR ("password", &conf->password),
    CFG_END ()
  };

  /* default values */
  conf->nick = NULL;
  conf->name = NULL;
  conf->host = NULL;
  conf->trusted = NULL;
  conf->password = NULL;

  cfg = cfg_init (opts, 0);
  status = cfg_parse (cfg, cfile);
  printf ("%s: Trusted users are: %s\n", conf->execname, conf->trusted);
  /* printf("%s %s %s %s\n", conf->nick, conf->name, conf->host, conf->password); */
  /* check for necessary settings */
  if (status == CFG_SUCCESS && conf->nick != NULL && conf->host != NULL)
    return 1;
  return 0;
}

/*! \brief Checks if message is from a trusted source.
  @param msg the message to check
  @return positive if the source is trusted, zero otherwise */
int
checkrights (message * msg)
{
  char *l, *s;

  s = malloc (2 + strlen (msg->ident) + strlen (msg->host));
  sprintf (s, "%s@%s", msg->ident, msg->host);
  for (l = strtok (conf->trusted, " "); l != NULL; l = strtok (NULL, " "))
    if (strstr (s, l))
      {
	free (s);
	return 1;
      }
  free (s);
  return 0;
}

/*! \brief Executes apropriate function as parameters specify.
 @param msg message with request data
 @param cmd command with command name and parameters */
void
execute (message * msg, command * cmd)
{
  action *a;

  a = finda (cmd->action, 0);
  if (checkrights (msg) && a != NULL)
    {
      printf ("%s: Accepted command from %s (%s@%s)\n", conf->execname,
	      msg->sender, msg->ident, msg->host);
      printf ("%s: Executing command '%s' with parameters: '%s'\n",
	      conf->execname, a->name, cmd->params);
      a->exec (msg, cmd);
    }
  else if (a == NULL)
    {
      printf
	("%s: No handler found for this command: '%s' (supplied parameters: '%s')\n",
	 conf->execname, cmd->action, cmd->params);
    }
  else
    printf
      ("%s: Untrusted user %s (%s@%s) tried to execute command %s, ignored\n",
       conf->execname, msg->sender, msg->ident, msg->host, cmd->action);

  free (cmd);
  msgfree (msg);
}

/*! \brief Parses commandline arguments.
  @return positive on successful parse */
int
configure (int argc, char *argv[])
{
  char *cfile = NULL;
  int opt;
  conf = (settings *) malloc (sizeof (settings));

  if (argc < 2)
    {
      p_help ();
      return 0;
    }

  conf->execname = argv[0];

  while ((opt = getopt (argc, argv, "hc:")) != -1)
    {
      switch (opt)
	{
	case 'h':
	  {
	    p_help ();
	    continue;
	  }
	case 'c':
	  {
	    cfile = optarg;
	    continue;
	  }
	case '?':
	default:
	  continue;
	}
    }

  if (cfile == NULL)
    {
      error (0, 0, "No configuration file specified.");
      return 0;
    }

  printf ("%s: Configuration file selected: %s\n", conf->execname, cfile);

  if (!parsecfg (cfile))
    {
      error (0, 0,
	     "You did not specify nickname and host or parsing configuration failed.");
      return 0;
    }
  return 1;
}

int
main (int argc, char *argv[])
{
  char *l = NULL;
  message *cmsg = NULL;
  command *ccmd = NULL;

  printf ("%s: This is Radroach commit %s\n", argv[0], COMMIT);
  if (!configure (argc, argv))
    exit (EXIT_FAILURE);
  conf->action_trigger = '=';	/* default trigger char */
  plugins_init ();
  sconnect (conf->host);

  while ((l = sogetline (conf->sock)) != NULL)
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
    }
  return EXIT_SUCCESS;
}
