#include "plugins.h"

void
execute (message_t * msg, command_t * cmd)
{
  char *s;

  msg = NULL;
  s = malloc (strlen ("JOIN \n") + strlen (cmd->params) + 1);
  sprintf (s, "JOIN %s\n", cmd->params);
  write (settings->sock, s, strlen (s));
  /* raw (s); */
  free (s);
}
