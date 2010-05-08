/* This file is part of Radroach.
   
   Radroach is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   Foobar is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with Radroach.  If not, see <http://www.gnu.org/licenses/>. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <confuse.h>
#include <dlfcn.h>

/* some type definitions */
typedef struct message
{
  char *sender, *ident, *host, *dest, *msg, *raw;
} message_t;

typedef struct command
{
  char *action, *params;
} command_t;

typedef struct settings
{
  char *nick, *name, *host, *trusted, *password, action_trigger, *execname;
  FILE *socket;
} settings_t;

typedef struct plugin
{
  char *name, *path;
  void *location;
  void (*execute) (message_t *, command_t *, uint8_t);
} plugin_t;
