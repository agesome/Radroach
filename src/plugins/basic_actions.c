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

/* very basic actions, look for function descriptions at the end of file */
#include "../radroach.h"

settings *conf = NULL;

#include "../util.c"

void
f_reply (message * msg, command * cmd)
{
  char *s;
  if (strstr (msg->dest, conf->nick) != NULL)
    {
      s = malloc (strlen ("PRIVMSG  :\n") + strlen (msg->sender) +
		  strlen (cmd->params) + 2);
      sprintf (s, "PRIVMSG %s :%s\n", msg->sender, cmd->params);
      raw (s);
    }
  else
    {
      s = malloc (strlen ("PRIVMSG  :\n") + strlen (msg->dest) +
		  strlen (cmd->params) + 2);
      sprintf (s, "PRIVMSG %s :%s\n", msg->dest, cmd->params);
      raw (s);
    }
  free (s);
}

void
f_join (message * msg, command * cmd)
{
  msg = NULL;
  char *s;
  s = malloc (strlen ("JOIN \n") + strlen (cmd->params) + 1);
  sprintf (s, "JOIN %s\n", cmd->params);
  raw (s);
  free (s);
}

void
f_part (message * msg, command * cmd)
{
  cmd = NULL;
  if (strstr (msg->dest, conf->nick) == NULL)
    {
      char *s;
      s = malloc (strlen ("PART \n") + strlen (msg->dest) + 1);
      sprintf (s, "PART %s\n", msg->dest);
      raw (s);
      free (s);
    }
}

void
f_me (message * msg, command * cmd)
{
  char *s;
  if (strstr (msg->dest, conf->nick) != NULL)
    {
      s = malloc (strlen ("PRIVMSG  :\001ACTION \001\n") +
		  strlen (msg->sender) + strlen (cmd->params) + 1);
      sprintf (s, "PRIVMSG %s :\001ACTION %s\001\n", msg->sender,
	       cmd->params);
      raw (s);
    }
  else
    {
      s = malloc (strlen ("PRIVMSG  :\001ACTION \001\n") +
		  strlen (msg->dest) + strlen (cmd->params) + 1);
      sprintf (s, "PRIVMSG %s :\001ACTION %s\001\n", msg->dest, cmd->params);
      raw (s);
    }
  free (s);
}

void
f_say (message * msg, command * cmd)
{
  /* target for the actual message, do not confuse with message->dest */
  char *target = NULL, *tmessage = NULL, *s = NULL;
  msg = NULL;

  target = cmd->params;
  tmessage = strchr (cmd->params, ' ') + 1;
  *(tmessage - 1) = '\0';

  s =
    malloc (strlen ("PRIVMSG  :\n") + strlen (target) + strlen (tmessage) +
	    1);
  sprintf (s, "PRIVMSG %s :%s\n", target, tmessage);
  raw (s);

  free (s);
}

void
f_trigger (message * msg, command * cmd)
{
  conf->action_trigger = cmd->params[0];

  char *s;
  if (strstr (msg->dest, conf->nick) != NULL)
    {
      s =
	malloc (strlen ("PRIVMSG  :Trigger character is now ' '\n") +
		strlen (msg->sender) + strlen (cmd->params) + 1);
      sprintf (s, "PRIVMSG %s :Trigger character is now '%c'\n", msg->sender,
	       cmd->params[0]);
      raw (s);
    }
  else
    {
      s =
	malloc (strlen ("PRIVMSG  :Trigger character is now ' '\n") +
		strlen (msg->sender) + strlen (cmd->params) + 1);
      sprintf (s, "PRIVMSG %s :Trigger character is now '%c'\n", msg->dest,
	       cmd->params[0]);
      raw (s);
    }
  free (s);
}

/* called on plugin load, you should manually add all actions here */
void
load (int (*action_add) (char *, void (*)(message *, command *)))
{
  action_add ("join", &f_join);
  action_add ("you", &f_me);
  action_add ("leave", &f_part);
  action_add ("say", &f_say);
  action_add ("trigger", &f_trigger);
  action_add ("reply", &f_reply);
}

/* called on unload, remove actions here */
void
unload (int (*action_delete) (char *))
{
  action_delete ("join");
  action_delete ("you");
  action_delete ("leave");
  action_delete ("say");
  action_delete ("trigger");
  action_delete ("reply");
}

/* called on load with global bot's configuration variable as parameter */
void
setconf (settings * c)
{
  conf = c;
}
