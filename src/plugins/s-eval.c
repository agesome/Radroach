#include "plugins.h"
#include <libguile.h>

#define EVAL_NORVALUE scm_from_long (-1)

int eerror = 0;
SCM scm, scm_result;
char *result;
message_t *glob_msg;

SCM
scm_reply (SCM str)
{
  reply (glob_msg, scm_to_locale_string (scm_object_to_string (str, SCM_UNDEFINED)));
  
  return EVAL_NORVALUE;
}

SCM
scm_raw (SCM str)
{
  raw (scm_to_locale_string (str));
  /* raw ("\n"); */
  
  /* return SCM_BOOL_T; */
}

SCM
scm_say (SCM target, SCM str)
{
  char *dest, *msg, *l;

  /* FIXME: segfaults on free */
  /* scm_dynwind_begin (0); */
  dest = scm_to_locale_string (target);
  msg = scm_to_locale_string (scm_object_to_string (str, SCM_UNDEFINED));
  l = malloc (strlen (dest) + strlen(msg) - 2 + strlen ("PRIVMSG  :\n") + 1);
  sprintf (l, "PRIVMSG %s :%s\n", dest, msg);
  raw (l);
  free (l);
  /* scm_dynwind_free (msg); */
  /* scm_dynwind_free (dest); */
  /* scm_dynwind_end (); */

  return EVAL_NORVALUE;
}

/* error handler */
void
exec_error (message_t *msg)
{  
  eerror = 1;
  reply (msg, "Error :(");
}

void
execute (message_t *msg, command_t *cmd, uint8_t is_targeted)
{
  if (is_targeted || !strcmp(cmd->action, "scheme"))
    {
      glob_msg = msg;
      scm = scm_internal_catch (SCM_BOOL_T, (scm_t_catch_body) scm_c_eval_string,
				(void *) cmd->params, (scm_t_catch_handler) exec_error, (void *) msg);
      if (eerror)
	{
	  eerror = 0;
	  return;
	}
      if (scm != EVAL_NORVALUE)
  	{
	  scm_result = scm_object_to_string (scm, SCM_UNDEFINED);
	  result = scm_to_locale_string (scm_result);
	  
	  reply (msg, result);
	  free (result);
	}
    }
}


__attribute__((constructor)) void
s_eval_initialize (void)
{
  scm_init_guile ();

  scm_primitive_load (scm_from_locale_string ("./src/plugins/s-eval.scm"));

  scm_c_define_gsubr ("reply", 1, 0, 0, scm_reply);
  scm_c_define_gsubr ("say", 2, 0, 0, scm_say);
  scm_c_define_gsubr ("raw", 1, 0, 0, scm_raw);  
}

__attribute__((destructor)) void
s_eval_deinitialize (void)
{
  /* scm_c_use_module (""); */
  /* scm_c_eval_string ("(use-modules (boot-9))"); */
}
