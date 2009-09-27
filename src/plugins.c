/* Radroach is a simple IRC bot
   Copyright © 2009  Evgeny Grablyk <evgeny.grablyk@gmail.com>
   
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

/* implements support for plugins */

typedef struct
{
  char *name;
  void *location;
  void (*load)( int (*)(char *, void (*)(message * msg, command * cmd)) );
  void (*unload)( int (*)(char *name, void (*)(message * msg, command * cmd)) );
  struct plugin *next;
} plugin;

plugin *p_root = NULL;
action *a_root = NULL;

int
action_add(char *name, void (*f)(message * msg, command * cmd))
{
  action *a, *p;
  if (a_root == NULL)
    {
      a_root = malloc (sizeof (action));
      a = a_root;
      a->next = NULL;
    }
  else
    {
      for(p = a_root; p->next != NULL; p = p->next);
      p->next = a = malloc (sizeof (action));
      a->next = NULL;
    }
  a->name = name;
  a->exec = f;
  printf("%s: Added action %s\n", conf->execname, name);
  return 1;
}

/* int */
/* action_delete(char *name) */
/* { */
/* } */

/* should load `name` from filesystem, call load(), add to list */
int
plugin_load(char *name)
{
  void *loc;
  char *msg;
  plugin *p, *n;
  void (*setconf)(settings *);

  loc = dlopen(name, RTLD_NOW);
  if (!loc)
    {
      msg = dlerror();
      if(msg != NULL)
	goto onerror;
    }
  if(p_root == NULL)
    {
      p_root = malloc (sizeof (plugin));
      p = p_root;
      p->next = NULL;
    }
  else
    {
      for(n = p_root; n->next != NULL; n = (plugin *) n->next);
      n->next = (struct plugin *) malloc (sizeof (plugin));
      p = (plugin *) n->next;
      p->next = NULL;
    }
  
  p->name = name;
  p->location = loc;
  p->load = dlsym(loc, "load");
  p->unload = dlsym(loc, "unload");
  setconf = dlsym(loc, "setconf");
  msg = dlerror();
  if (msg != NULL)
    {
      free(p);
      p = NULL;
      printf("%s: Bad plugin '%s'\n", conf->execname, name);
      goto onerror;
    }
  setconf(conf);
  p->load(&action_add);
  return 1;
  
 onerror:
  dlclose(loc);
  return 0;
}

/* call unload(), remove from list, actually unload */
/* int */
/* plugin_unload(char *name) */
/* { */
/* } */
