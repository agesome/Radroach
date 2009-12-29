#include "plugins.h"

void
execute (message_t * msg, command_t * cmd)
{
  char *s;
  if (strstr (msg->dest, settings->nick) != NULL)
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
