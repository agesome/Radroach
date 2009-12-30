#include "plugins.h"
#include <libguile.h>

void
execute (message_t * msg, command_t * cmd)
{
  SCM scm, scm_result;
  char buf[4096], *result;
  
  scm_init_guile ();
  scm = scm_c_eval_string (cmd->params);
  scm_result = scm_object_to_string (scm, SCM_UNDEFINED);
  result = scm_to_locale_string (scm_result);

  sprintf (buf, "PRIVMSG %s :%s\n", msg->dest, result);
  raw (buf);
}
