#include "plugins.h"

float sum = 0;

void
execute (message_t * msg, command_t * cmd, uint8_t is_targeted)
{
  if (is_targeted)
    {
      float sum_t;
      char *l;
  
      if (strstr (cmd->params, "reset"))
	{
	  sum = 0;
	  reply (msg, "Sum reset.");
	}
      else if (strstr (cmd->params, "show"))
	{
	  l = malloc (strlen ("Sum is: .") + 4);
	  sprintf (l, "Sum is: %2.1f\n", sum);
	  reply (msg, l);
	}
      else
	{
	  if (sscanf (cmd->params, "%f", &sum_t))
	    sum += sum_t;
	}
    }
}
