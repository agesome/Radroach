#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <regex.h>

struct message
{
  char *sender, *ident, *host, *dest, *msg;
};

typedef struct message message;

struct command
{
  char *action, *params;
};

typedef struct command command;

struct action
{
  char *name, *desc, *help;
  void (*exec)(message *, command *);
};

typedef struct action action;

struct settings
{
  char *nick, *name, *host, *trusted;
  int password;
};

typedef struct settings settings;
