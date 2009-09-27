CC = gcc $(DEFS) $(LIBS) $(INCLUDES) $(CFLAGS)
CFLAGS = -O0 -g -ggdb -pipe -Wall -Wextra
LIBS = -lconfuse -ldl
INCLUDES = 
FILES = src/radroach.c
PFILES = src/plugins/basic_actions.c
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

basic_actions: $(PFILES)
	$(CC) -fPIC -c $(PFILES)
	$(CC) -shared -o $@.so $@.o
