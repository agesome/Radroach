#include "plugins.h"

void
execute (message_t * msg, command_t * cmd, uint8_t is_targeted)
{
  cmd = NULL;
  /* send out PART only if we're in a channel */
  if (strstr (msg->dest, settings->nick) == NULL && is_targeted)
    {
      char *s;
      s = malloc (strlen ("PART \n") + strlen (msg->dest) + 1);
      sprintf (s, "PART %s\n", msg->dest);
      raw (s);
      free (s);
    }
}
