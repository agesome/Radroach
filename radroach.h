#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <regex.h>
#include <confuse.h>

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
  char *nick, *name, *host, *trusted, *password;
};

typedef struct settings settings;

#define BUFSZ 10

static char inbuf[BUFSZ + 1], *execname;
static int sock, setup_done = 0;
static settings *conf = NULL;

void
p_help(void);
void
logstr(char *str);
void
raw(int sock, char *str);
int
sread(int sock, char *buf, int nc);
void
sconnect(char *host);
void
setup(int sock);
message *
parsemsg (const char *str);
command *
parsecmd(char *str);
int
p_response(char *l);
settings *
parsecfg(char *cfile);
int
checkrights(message *msg);
void
execute(message *msg, command *cmd);
int
configure(int argc, char *argv[]);
