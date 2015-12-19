# *****************************************************
# Parameters to control Makefile operation

CC = gcc
CFLAGS = -Wall -D_FILE_OFFSET_BITS=64
DEBUG = -g -DDEBUG

# ****************************************************

all: main.o Allocator.o constants.o
	$(CC) $(CFLAGS) -o main main.o Allocator.o constants.o

debug: CFLAGS += $(DEBUG)
debug: all

main.o: main.c Allocator.h
	$(CC) $(CFLAGS) -c main.c

Allocator.o: Allocator.c Allocator.h
	$(CC) $(CFLAGS) -c Allocator.c

constants.o: constants.c constants.h
	$(CC) $(CFLAGS) -c constants.c

clean:
	rm *.o
	rm main
