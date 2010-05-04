#include "plugins.h"

void
execute (message_t *msg, command_t *cmd, uint8_t is_targeted)
{
  if (is_targeted)
    {
      char *path = strdup ( (plugin_find(cmd->params))->path );
      
      plugin_unload (cmd->params);
      plugin_load (path);
      free (path);
    }
}
