#include "plugins.h"

void
execute (message_t * msg, command_t * cmd)
{
  cmd = NULL;
  if (strstr (msg->dest, settings->nick) == NULL)
    {
      char *s;
      s = malloc (strlen ("PART \n") + strlen (msg->dest) + 1);
      sprintf (s, "PART %s\n", msg->dest);
      raw (s);
      free (s);
    }
}
