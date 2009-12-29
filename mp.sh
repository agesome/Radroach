#!/bin/sh

for i in `ls src/plugins/ | grep ".c$" | sed -s 's/.c//g'`
do
gcc -Isrc/ -O1 -g -ggdb -pipe -Wall -Wextra -rdynamic -fPIC src/plugins/$i.c -c
gcc -O1 -g -ggdb -pipe -Wall -Wextra -rdynamic -shared -o plugins/$i.so $i.o
rm $i.o
done