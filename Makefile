CC = gcc $(CFLAGS) $(LIBS) $(INCLUDES) $(DEFS)
CFLAGS = -O0 -g -pipe -pedantic -Wall -Wextra
LIBS = 
INCLUDES = -I.
FILES =  *.c
DEFS =

all: clean cbot

cbot: cbot.o

cbot.o: $(FILES)
	$(CC) $(FILES) -c

cbot: $(cbot)
	$(CC) -o $@ *.o

clean:
	rm -f *.o cbot