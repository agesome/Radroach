/* Radroach is a simple IRC bot
   Copyright (C) 2009  Evgeny Grablyk <evgeny.grablyk@gmail.com>
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

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

/* some type definitions */
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

char inbuf[BUFSZ + 1];
/* trigger character */
char action_trigger = '`';
/* points to argv[0] */
char *execname;
/* variable that indicates if setup is already done */
int setup_done = 0;
/* global variable that refers to currently open socket */
int sock;
/* global configuration */
settings *conf = NULL;

void
p_help(void);
void
logstr(char *str);
void
raw(int sock, char *str);
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
char *
sogetline(int s);
