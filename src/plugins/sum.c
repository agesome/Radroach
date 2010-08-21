#include "plugins.h"

float sum = 0;

void
execute (message_t * msg, command_t * cmd, uint8_t is_targeted)
{
  if (is_targeted)
    {
      float sum_t;
      char *l;
      
      if (cmd->params == NULL)
	return;
      
      if (strstr (cmd->params, "reset"))
	{
	  sum = 0;
	  reply (msg, "sum has been reset");
	}
      else if (strstr (cmd->params, "show"))
	{
	  char buf[64];		/* insecure :( */
	  snprintf (buf, 64, "%2.1f", sum);
	  l = malloc (strlen ("sum is ") + strlen (buf) + 2);
	  sprintf (l, "sum is %2.1f", sum);
	  reply (msg, l);
	}
      else
	{
	  if (sscanf (cmd->params, "%f", &sum_t))
	    sum += sum_t;
	}
    }
}
