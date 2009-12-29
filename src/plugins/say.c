#include "plugins.h"

void
execute (message_t * msg, command_t * cmd)
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
