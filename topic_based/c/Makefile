# *****************************************************
# Parameters to control Makefile operation

CC = gcc
CFLAGS = -Wall -D_FILE_OFFSET_BITS=64
DEBUG = -g -DDEBUG

# ****************************************************

all: main.o Allocator.o constants.o dictionary.o
	$(CC) $(CFLAGS) -o main main.o Allocator.o constants.o dictionary.o -lm

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


clean:
	rm *.o
	rm main
