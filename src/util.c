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

void raw (char *str);
extern settings_t *settings;

/* reply either to a channel or to a person */
void
reply (message_t *msg, char *reply)
{
  char *s;
  
  if (strstr (msg->dest, settings->nick) != NULL)
    {
      s = malloc (strlen ("PRIVMSG  :\n") + strlen (msg->sender) +
		  strlen (reply) + 2);
      sprintf (s, "PRIVMSG %s :%s\n", msg->sender, reply);
      raw (s);
    }
  else
    {
      s = malloc (strlen ("PRIVMSG  :\n") + strlen (msg->dest) +
		  strlen (reply) + strlen(msg->sender) + 2);
      sprintf (s, "PRIVMSG %s :%s: %s\n", msg->dest, msg->sender, reply);
      raw (s);
    }
  free (s);
}

void
logstr (char *fmt, ...)
{
  va_list args;
  
  va_start (args, fmt);
  vprintf (fmt, args);
  va_end (args);
}

void
raw (char *str)
{
  logstr (str);
  fprintf (settings->socket, str);
}

/*  Print short usage instructions. */
void
print_usage (void)
{
  logstr ("usage: %s [-h] -c <file>\n", settings->execname);
}
