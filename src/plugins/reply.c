#include "plugins.h"

void
execute (message_t * msg, command_t * cmd, uint8_t is_targeted)
{
  if (is_targeted)
    {
      reply (msg, cmd->params);
    }
}
