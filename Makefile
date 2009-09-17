LANG="en_US.UTF-8"
CC = gcc $(CFLAGS) $(DEFS) $(LIBS) $(INCLUDES)
CFLAGS = -O0 -g -pipe -Wall -Wextra
LIBS = -lconfuse
INCLUDES = -I.
FILES = radroach.c
DEFS = 

all: clean radroach

radroach: radroach.o

radroach.o: $(FILES)
	$(CC) $(FILES) -c

radroach: $(radroach)
	$(CC) -o $@ *.o

clean:
	rm -f *.o radroach