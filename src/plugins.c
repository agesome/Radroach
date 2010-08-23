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
#define MAX_PLUGINS 256

extern settings_t *settings;

plugin_t *plugins[MAX_PLUGINS];
unsigned int plugin_count = 0;

/* find plugin under name `name` */
plugin_t *
plugin_find (char *name)
{
  unsigned int i;

  for (i = 0; i < plugin_count; i++)
    if (!strcmp (plugins[i]->name, name))
      return plugins[i];
  return NULL;
}

/* load a plugin from `path` */
int
plugin_load (char *path)
{
  plugin_t *p;
  void *location;
  void (*execute) (message_t *, command_t *, uint8_t);
  int namelen;
  char *name;

  /* strip off the path and .so extension */
  namelen = strrchr (path, '.') - strrchr (path, '/');
  name = malloc (namelen);
  strncpy (name, strrchr (path, '/') + 1, namelen - 1);
  name[namelen - 1] = '\0';
  
  if (plugin_find (name) != NULL)
    goto error;
  location = dlopen (path, RTLD_NOW);
  if (location == NULL)
    goto error;
  execute = dlsym(location, "execute");
  if (execute == NULL)
    {
      dlclose (location);
      goto error;
    }
  p = plugins[plugin_count] = malloc (sizeof (plugin_t));
  plugin_count++;
  p->location = location;
  p->execute = execute;
  p->name = name;
  p->path = path;
  logstr ("loaded plugin '%s'\n", p->name);
  
  return 1;

 error:
  free (name);
  return 0;    
}

int
plugin_unload (char *name)
{
  plugin_t *p;
  unsigned int i;

  p = plugin_find (name);
  if (p == NULL)
    return 0;
  for (i = 0; i < plugin_count; i++)
    if (plugins[i] == p)
      break;
  for (i = i; i < plugin_count - 1; i++)
    plugins[i] = plugins[i + 1];
  plugin_count--;
  dlclose (p->location);
  free (p);
  logstr ("unloaded plugin '%s'\n", name);
  return 1;
}

void
plugins_load (char *plugindir)
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

/* unload all loaded plugins */
void
plugins_unload (void)
{
  while (plugin_count)
    {
      plugin_unload (plugins[0]->name);
    }
}
