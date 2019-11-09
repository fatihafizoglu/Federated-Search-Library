# *****************************************************
# Parameters to control Makefile operation

CC = gcc
CFLAGS = -Wall -D_FILE_OFFSET_BITS=64
DEBUG = -g -DDEBUG

# RELATIVE_ALLOCATOR_PATH
RAP = ../../topic_based/c/
# ****************************************************

all: main.c diversify.c diversify.h $(RAP)Allocator.c
	$(CC) $(CFLAGS) main.c diversify.c $(RAP)Allocator.c $(RAP)constants.c $(RAP)dictionary.c -lm -o diversify

debug: CFLAGS += $(DEBUG)
debug: all

cdiv: CFLAGS += -DCDIV
cdiv: all

debugcdiv: CFLAGS += -DCDIV $(DEBUG)
debugcdiv: all

# tester: tester.c main.c diversify.c diversify.h $(RAP)Allocator.c
# 	$(CC) $(CFLAGS) -DTESTER -DDEBUG -g tester.c main.c diversify.c $(RAP)Allocator.c $(RAP)constants.c $(RAP)dictionary.c -lm -o tester


clean:
	rm -f *.o
	rm -f diversify
