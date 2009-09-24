CC = gcc $(CFLAGS) $(DEFS) $(LIBS) $(INCLUDES)
CFLAGS = -O0 -g -ggdb -pipe -Wall -Wextra
LIBS = -lconfuse
INCLUDES = 
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
	rm -f *.o radroach