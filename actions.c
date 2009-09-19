/* #include "cbot.h" */

void
f_reply(message *msg, command *cmd)
{
  char *s;
  if( strstr(msg->dest, conf->nick) != NULL )
    {
      s = (char *) malloc(strlen("PRIVMSG  :\n") + strlen(msg->sender) + strlen(cmd->params));
      sprintf(s, "PRIVMSG %s :%s\n", msg->sender, cmd->params);
      raw(sock, s);
    }
  else
    {
      s = (char *) malloc(strlen("PRIVMSG  :\n") + strlen(msg->dest) + strlen(cmd->params));
      sprintf(s, "PRIVMSG %s :%s\n", msg->dest, cmd->params);
      raw(sock, s);
    }
  free(s);
}

void
f_join(message *msg, command *cmd)
{
  msg = NULL;
  char *s;
  s = (char *) malloc(strlen("JOIN \n") + strlen(cmd->params));
  sprintf(s, "JOIN %s\n", cmd->params);
  raw(sock, s);
  free(s);
}

void
f_part(message *msg, command *cmd)
{
  cmd = NULL;
  if( strstr(msg->dest, "CBot") == NULL )
    {
      char *s;
      s = (char *) malloc(strlen("PART \n") + strlen(msg->dest));
      sprintf(s, "PART %s\n", msg->dest);
      raw(sock, s);
      free(s);
    }
}

void
f_me(message *msg, command *cmd)
{
  char *s;
  if( strstr(msg->dest, "CBot") != NULL )
    {
      s = (char *) malloc(strlen("PRIVMSG  :\001ACTION \001\n") + strlen(msg->sender) + strlen(cmd->params));
      sprintf(s, "PRIVMSG %s :\001ACTION %s\001\n", msg->sender, cmd->params);
      raw(sock, s);
    }
  else
    {
      s = (char *) malloc(strlen("PRIVMSG  :\001ACTION \001\n") + strlen(msg->dest) + strlen(cmd->params));
      sprintf(s, "PRIVMSG %s :\001ACTION %s\001\n", msg->dest, cmd->params);
      raw(sock, s);
    }
  free(s);
}

void
f_say(message *msg, command *cmd)
{
  /* end of target - length of target parameter from the beginning of parameters string */
  int eot = 0;
  /* target for the actual message, do not confuse with message->dest */
  char *target = NULL, *tmessage = NULL, *s = NULL;

  msg = NULL;
  
  while(cmd->params[eot] != ' ')
    eot++;
  target = malloc(eot - 1);
  memcpy(target, cmd->params, eot);
  tmessage = malloc(strlen(cmd->params) - eot + 1);
  memcpy(tmessage, &cmd->params[eot + 1], strlen(cmd->params) - 1);

  printf("%s %s\n", target, tmessage);
  s = malloc(strlen("PRIVMSG  :\n") + strlen(target) + strlen(tmessage));
  sprintf(s, "PRIVMSG %s :%s\n", target, tmessage);
  raw(sock, s);

  free(target);
  free(tmessage);
  free(s);
}
  
action acts[] =
  {
    { "reply", "Say something to comamnd source", NULL, &f_reply },
    { "say", "Format is \"<target> <message>\". Bot says <message> to <target>." , NULL, &f_say},
    { "join", "Join a channel", NULL, &f_join },
    { "part", "Leave a channel", NULL, &f_part },
    { "you", "The bot will act like he does something", NULL, &f_me },    
    { NULL, NULL, NULL, NULL }	/* so we can detect end of array */
  };
