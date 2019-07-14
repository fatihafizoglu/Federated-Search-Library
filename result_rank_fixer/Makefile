# *****************************************************
# Parameters to control Makefile operation

CC = gcc
CFLAGS = -Wall -D_FILE_OFFSET_BITS=64
DEBUG = -g -DDEBUG

# RELATIVE_ALLOCATOR_PATH
RAP = ../topic_based/c/
# ****************************************************

all: result_rank_fixer.o Allocator.o constants.o dictionary.o
	$(CC) $(CFLAGS) -o result_rank_fixer result_rank_fixer.o Allocator.o constants.o dictionary.o -lm

debug: CFLAGS += $(DEBUG)
debug: all

result_rank_fixer.o: result_rank_fixer.c
	$(CC) $(CFLAGS) -c result_rank_fixer.c

Allocator.o: $(RAP)Allocator.c $(RAP)Allocator.h
	$(CC) $(CFLAGS) -c $(RAP)Allocator.c

constants.o: $(RAP)constants.c $(RAP)constants.h
	$(CC) $(CFLAGS) -c $(RAP)constants.c

dictionary.o: $(RAP)dictionary.c $(RAP)dictionary.h
	$(CC) $(CFLAGS) -c $(RAP)dictionary.c

clean:
	rm -f *.o
	rm -f result_rank_fixer
