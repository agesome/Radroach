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

/* various utility functions, for both Radroach and plugins */

/* used for logging, may be extended in future */
void
logstr (char *str)
{
  printf ("%s: %s", conf->execname, str);
}

/* sends raw string `str` via global socket `sock` */
void
raw (char *str)
{
  logstr (str);
  write (conf->sock, str, strlen (str));
}
