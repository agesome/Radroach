#include "plugins.h"

void
execute (message_t *msg, command_t *cmd)
{
  char *path = strdup ( (plugin_find(cmd->params))->path );

  plugin_unload (cmd->params);
  plugin_load (path);
  free (path);
}
