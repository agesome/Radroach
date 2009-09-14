LANG="en_US.UTF-8"
CC = gcc $(CFLAGS) $(DEFS) $(LIBS) $(INCLUDES)
CFLAGS = -O0 -g -pipe -Wall -Wextra
LIBS = -lconfuse
INCLUDES = -I.
FILES =  cbot.c
# VER = `git log | head -1 | awk '{print $2}'`
DEFS = #-D_REV=git log | head -1 | awk '{print $2}'`

all: clean cbot

cbot: cbot.o

cbot.o: $(FILES)
	$(CC) $(FILES) -c

cbot: $(cbot)
	$(CC) -o $@ *.o

clean:
	rm -f *.o cbot