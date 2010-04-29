#include "plugins.h"

void
execute (message_t * msg, command_t * cmd, uint8_t is_targeted)
{
  char *s;
	
  if (is_targeted)
    {
      msg = NULL;
      s = malloc (strlen ("JOIN \n") + strlen (cmd->params) + 1);
      sprintf (s, "JOIN %s\n", cmd->params);
      raw (s);
      free (s);
    }
}
