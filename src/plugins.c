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
#include <dirent.h>

extern settings_t *settings;


typedef struct plugin
{
  char *name, *path;
  void *location;
  void (*execute) (message_t *, command_t *);
  struct plugin *next;
} plugin_t;

plugin_t *p_root = NULL;

plugin_t *
lastp (void)
{
  plugin_t *p = p_root;

  while (p != NULL && p->next != NULL)
    p = p->next;
  return p;
}

/* find plugin under name `name`, if `ispath` is set - compare with plugin's
   path, not it's name; if `retnext` is set, return plugin that is right before
   the one we're looking for */
plugin_t *
plugin_find (char *name, int ispath, int retnext)
{
  plugin_t *p = p_root, *l = NULL;
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

/* load a plugin from `path` */
int
plugin_load (char *path)
{
  plugin_t *p;
  void *l;
  void (*execute) (message_t *, command_t *);
  int namelen;

  if (plugin_find (path, 1, 0) != NULL)
    return 0;
  l = dlopen (path, RTLD_NOW);
  if (l == NULL)
    return 0;
  execute = dlsym(l, "execute");
  if (execute == NULL)
    return 0;
  p = lastp ();
  if (p == NULL)
    {
      p = malloc (sizeof (plugin_t));
      p_root = p;
    }
  else
    {
      p->next = malloc (sizeof (plugin_t));
      p = p->next;
    }
  /* strip off the path and .so extension */
  namelen = strrchr (path, '.') - strrchr (path, '/');
  p->name = malloc (namelen);
  strncpy (p->name, strrchr (path, '/') + 1, namelen - 1);
  p->name[namelen - 1] = '\0';
  p->location = l;
  p->execute = execute;
  p->next = NULL;
  printf ("%s: Loaded plugin '%s'\n", settings->execname, p->name);
  
  return 1;
}

void
plugins_init (char *plugindir)
{
  DIR *pd;
  struct dirent *file;
  char *p;
  int pathlen;

  pd = opendir (plugindir);

  while ( (file = readdir (pd)) )
    {
      if (strstr (file->d_name, ".so") != NULL)
	{
	  pathlen = strlen (plugindir) + strlen (file->d_name) + 1;
	  p = malloc (pathlen);
	  memset (p, 0, pathlen);
	  strcat (p, plugindir);
	  strcat (p, file->d_name);
	  plugin_load (p);
	}
    }  
}

/* int */
/* plugin_unload (char *name) */
/* { */
/*   plugin *p, *l; */

/*   p = plugin_find (name, 0, 1); */
/*   if (p != NULL) */
/*     { */
/*       l = p->next; */
/*       printf ("%s: Unloading plugin '%s' with following actions: ", */
/* 	      conf->execname, l->name); */
/*       l->unload (&action_delete); */
/*       printf ("...done\n"); */
/*       dlclose (l->loc); */
/*       p->next = l->next; */
/*       return 1; */
/*     } */
/*   p = plugin_find (name, 0, 0); */
/*   if (p == NULL) */
/*     return 0; */
/*   printf ("%s: Unloading plugin '%s' with following actions: ", */
/* 	  conf->execname, p->name); */
/*   p->unload (&action_delete); */
/*   printf ("...done\n"); */
/*   dlclose (p->loc); */
/*   p_root = p->next; */
/*   return 1; */
/* } */
