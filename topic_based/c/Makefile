# *****************************************************
# Parameters to control Makefile operation

CC = gcc
CFLAGS = -Wall -D_FILE_OFFSET_BITS=64
DEBUG = -g -DDEBUG

# ****************************************************

all: main.o Allocator.o constants.o dictionary.o
	$(CC) $(CFLAGS) -o allocator main.o Allocator.o constants.o dictionary.o -lm

debug: CFLAGS += $(DEBUG)
debug: all

main.o: main.c Allocator.h
	$(CC) $(CFLAGS) -c main.c

Allocator.o: Allocator.c Allocator.h
	$(CC) $(CFLAGS) -c Allocator.c

constants.o: constants.c constants.h
	$(CC) $(CFLAGS) -c constants.c

dictionary.o: dictionary.c dictionary.h
	$(CC) $(CFLAGS) -c dictionary.c

readclusters: readclusters.c
	$(CC) $(CFLAGS) -o readclusters readclusters.c

indexclusters: indexclusters.c
	$(CC) $(CFLAGS) -o indexclusters indexclusters.c

clean:
	rm -f *.o
	rm -f allocator
	rm -f indexclusters
	rm -f readclusters
