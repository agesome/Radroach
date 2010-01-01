#include "plugins.h"

void
execute (message_t * msg, command_t * cmd)
{
  reply (msg, cmd->params);
  
}
