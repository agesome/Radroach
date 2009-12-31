#include "plugins.h"
#include <libguile.h>

int eerror = 0;
SCM scm, scm_result;
char *result;
message_t *glob_msg;

SCM
scm_reply (SCM str)
{
  reply (glob_msg, scm_to_locale_string (scm_object_to_string (str, SCM_UNDEFINED)));
  
  return SCM_BOOL_T;
}

void
exec_error (message_t *msg)
{  
  eerror = 1;
  reply (msg, "Error :(");
  
  return;
}

void
execute (message_t *msg, command_t *cmd)
{
  glob_msg = msg;
  
  scm = scm_internal_catch (SCM_BOOL_T, (scm_t_catch_body) scm_c_eval_string,
			    (void *) cmd->params, (scm_t_catch_handler) exec_error, (void *) msg);
  if (eerror)
    {
      eerror = 0;
      return;
    }
  scm_result = scm_object_to_string (scm, SCM_UNDEFINED);
  result = scm_to_locale_string (scm_result);
  
  reply (msg, result);
  free (result);
}


__attribute__((constructor)) void
s_eval_initialize (void)
{
  scm_init_guile ();

  scm_primitive_load (scm_from_locale_string ("./src/plugins/s-eval.scm"));

  gh_new_procedure ("reply", scm_reply, 1, 0, 0);
}
