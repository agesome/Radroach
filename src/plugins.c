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

/* implements support for loadable plugins */

typedef struct plugin
{
  char *name, *path;
  void *loc;
  void (*unload) (int (*)(char *));
  struct plugin *next;
} plugin;

plugin *p_root = NULL;
action *a_root = NULL;

plugin *
lastp (void)
{
  plugin *p = p_root;

  while (p != NULL && p->next != NULL)
    p = p->next;
  return p;
}

action *
lasta (void)
{
  action *a = a_root;

  while (a->next != NULL)
    a = a->next;
  return a;
}

/* find plugin under name `name`, if `ispath` is set - compare with plugin's
   path, not it's name; if `retnext` is set, return plugin that is right before
   the one we're looking for */
plugin *
findp (char *name, int ispath, int retnext)
{
  plugin *p = p_root, *l;
  char *r = NULL;

  while (p != NULL)
    {
      if (retnext && p->next != NULL)
	{
	  l = p;
	  p = p->next;
	}
      else if (retnext)
	return 0;

      if (ispath)
	r = strstr (p->path, name);
      else
	r = strstr (p->name, name);
      if (r != NULL)
	return p;
      p = l->next;
    }
  return NULL;
}

/* find action `name`; if `retnext` is set, return action that is right before
   the one we're looking for */
action *
finda (char *name, int retnext)
{
  action *a = a_root;
  char *r = NULL;

  while (a != NULL)
    {
      if (a->next != NULL && retnext)
	r = strstr ((a->next)->name, name);
      else if (!retnext)
	r = strstr (a->name, name);
      if (r != NULL)
	return a;
      a = a->next;
    }
  return NULL;
}

/* add action `name`, with execution function `exec` */
int
action_add (char *name, void (*exec) (message * msg, command * cmd))
{
  action *a;

  if (finda (name, 0) != NULL)
    return 0;
  a = lasta ();
  a->next = malloc (sizeof (action));
  a = a->next;
  a->name = name;
  a->exec = exec;
  a->next = NULL;
  printf ("%s ", name);

  return 1;
}

/* load a plugin from `path` */
int
plugin_load (char *path)
{
  plugin *p;
  void *l;
  void (*load) (int (*)(char *, void (*)(message * msg, command * cmd)));
  void (*unload) (int (*)(char *));
  void (*setconf) (settings *);

  if (findp (path, 1, 0) != NULL)
    return 0;

  l = dlopen (path, RTLD_NOW);
  if (l == NULL)
    return 0;

  unload = dlsym (l, "unload");
  setconf = dlsym (l, "setconf");
  load = dlsym (l, "load");
  if (unload == NULL || setconf == NULL || load == NULL)
    {
      printf ("%s: Bad plugin '%s'\n", conf->execname, path);
      dlclose (l);
      return 0;
    }

  p = lastp ();
  if (p == NULL)
    {
      p = malloc (sizeof (plugin));
      p_root = p;
    }
  else
    {
      p->next = malloc (sizeof (plugin));
      p = p->next;
    }
  p->name = strdup (strrchr (path, '/') + 1);
  p->loc = l;
  p->unload = unload;

  setconf (conf);
  printf ("%s: Loading plugin '%s' with following actions: ", conf->execname,
	  p->name);
  load (&action_add);
  printf ("...done\n");
  p->next = NULL;
  return 1;
}

int
action_delete (char *name)
{
  action *a, *p;

  a = finda (name, 1);
  if (a == NULL)
    return 0;
  p = a->next;
  a->next = p->next;
  printf ("%s ", name);
  return 1;
}

int
plugin_unload (char *name)
{
  plugin *p, *l;

  p = findp (name, 0, 1);
  if (p != NULL)
    {
      l = p->next;
      printf ("%s: Unloading plugin '%s' with following actions: ",
	      conf->execname, l->name);
      l->unload (&action_delete);
      printf ("...done\n");
      dlclose (l->loc);
      p->next = l->next;
      return 1;
    }
  p = findp (name, 0, 0);
  if (p == NULL)
    return 0;
  printf ("%s: Unloading plugin '%s' with following actions: ",
	  conf->execname, p->name);
  p->unload (&action_delete);
  printf ("...done\n");
  dlclose (p->loc);
  p_root = p->next;
  return 1;
}

/* wrappers for plugin_load() / plugin_unload() */
void
plugload (message * msg, command * cmd)
{
  msg = NULL;
  plugin_load (cmd->params);
}

void
pluguload (message * msg, command * cmd)
{
  msg = NULL;
  plugin_unload (cmd->params);
}

void
plugins_init (void)
{
  action *a;
  a_root = malloc (sizeof (action));

  a_root->name = "plugload";
  a_root->exec = &plugload;
  a_root->next = a = malloc (sizeof (action));
  a->name = "pluguload";
  a->exec = &pluguload;
  a->next = NULL;
}
