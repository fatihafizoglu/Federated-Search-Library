# *****************************************************
# Parameters to control Makefile operation

CC = gcc
CFLAGS = -Wall -D_FILE_OFFSET_BITS=64
DEBUG = -g -DDEBUG

# ****************************************************

all: main.o Allocator.o messages.o
	$(CC) $(CFLAGS) -o main main.o Allocator.o messages.o

main.o: main.c Allocator.h
	$(CC) $(CFLAGS) -c main.c

Allocator.o: Allocator.c Allocator.h
	$(CC) $(CFLAGS) -c Allocator.c
	
messages.o: messages.c messages.h
	$(CC) $(CFLAGS) -c messages.c 
	
clean:
	rm *.o
	rm main
