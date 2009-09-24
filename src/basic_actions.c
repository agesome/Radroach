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

/* very basic actions */

void
f_reply (message * msg, command * cmd)
{
  char *s;
  if (strstr (msg->dest, conf->nick) != NULL)
    {
      s =
	(char *) malloc (strlen ("PRIVMSG  :\n") + strlen (msg->sender) +
			 strlen (cmd->params));
      sprintf (s, "PRIVMSG %s :%s\n", msg->sender, cmd->params);
      raw (s);
    }
  else
    {
      s =
	(char *) malloc (strlen ("PRIVMSG  :\n") + strlen (msg->dest) +
			 strlen (cmd->params));
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
  s = (char *) malloc (strlen ("JOIN \n") + strlen (cmd->params));
  sprintf (s, "JOIN %s\n", cmd->params);
  raw (s);
  free (s);
}

void
f_part (message * msg, command * cmd)
{
  cmd = NULL;
  if (strstr (msg->dest, "CBot") == NULL)
    {
      char *s;
      s = (char *) malloc (strlen ("PART \n") + strlen (msg->dest));
      sprintf (s, "PART %s\n", msg->dest);
      raw (s);
      free (s);
    }
}

void
f_me (message * msg, command * cmd)
{
  char *s;
  if (strstr (msg->dest, "CBot") != NULL)
    {
      s =
	(char *) malloc (strlen ("PRIVMSG  :\001ACTION \001\n") +
			 strlen (msg->sender) + strlen (cmd->params));
      sprintf (s, "PRIVMSG %s :\001ACTION %s\001\n", msg->sender,
	       cmd->params);
      raw (s);
    }
  else
    {
      s =
	(char *) malloc (strlen ("PRIVMSG  :\001ACTION \001\n") +
			 strlen (msg->dest) + strlen (cmd->params));
      sprintf (s, "PRIVMSG %s :\001ACTION %s\001\n", msg->dest, cmd->params);
      raw (s);
    }
  free (s);
}

void
f_say (message * msg, command * cmd)
{
  /* end of target - length of target parameter from the beginning of parameters string */
  int eot = 0;
  /* target for the actual message, do not confuse with message->dest */
  char *target = NULL, *tmessage = NULL, *s = NULL;

  msg = NULL;

  while (cmd->params[eot] != ' ')
    eot++;
  target = malloc (eot - 1);
  memcpy (target, cmd->params, eot);
  tmessage = malloc (strlen (cmd->params) - eot + 1);
  memcpy (tmessage, &cmd->params[eot + 1], strlen (cmd->params) - 1);

  printf ("%s %s\n", target, tmessage);
  s = malloc (strlen ("PRIVMSG  :\n") + strlen (target) + strlen (tmessage));
  sprintf (s, "PRIVMSG %s :%s\n", target, tmessage);
  raw (s);

  free (target);
  free (tmessage);
  free (s);
}

action acts[] = {
  {"reply", "Say something to comamnd source", NULL, &f_reply}
  ,
  {"say", "Format is \"<target> <message>\". Bot says <message> to <target>.",
   NULL, &f_say}
  ,
  {"join", "Join a channel", NULL, &f_join}
  ,
  {"part", "Leave a channel", NULL, &f_part}
  ,
  {"you", "The bot will act like he does something", NULL, &f_me}
  ,
  {NULL, NULL, NULL, NULL}	/* so we can detect end of array */
};
