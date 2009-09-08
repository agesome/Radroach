LANG="en_US.UTF-8"
CC = gcc $(CFLAGS) $(LIBS) $(INCLUDES) $(DEFS)
CFLAGS = -O0 -g -pipe -Wall -Wextra
LIBS = -lconfuse
INCLUDES = -I.
FILES =  cbot.c
DEFS =

all: clean cbot

cbot: cbot.o

cbot.o: $(FILES)
	$(CC) $(FILES) -c

cbot: $(cbot)
	$(CC) -o $@ *.o

clean:
	rm -f *.o cbot