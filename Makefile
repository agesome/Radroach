CC = gcc $(DEFS) $(LIBS) $(INCLUDES) $(CFLAGS)
CFLAGS = -O1 -g -ggdb -pipe -Wall -Wextra -rdynamic
LIBS = -lconfuse -ldl
INCLUDES = -Isrc/ -Isrc/plugins/
FILES = src/radroach.c
COMMIT = $(shell ./getcommit.sh)
DEFS = -DCOMMIT=\"${COMMIT}\"
SHELL = /bin/sh

all: clean radroach

radroach: radroach.o

radroach.o: $(FILES)
	$(CC) $(FILES) -c

radroach: $(radroach)
	$(CC) -o $@ *.o

clean:
	rm -f *.o
