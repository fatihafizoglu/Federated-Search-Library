# *****************************************************
# Parameters to control Makefile operation

CC = gcc
CFLAGS = -Wall -D_FILE_OFFSET_BITS=64 -w
# Here we are using -w to inhibit warning messages
DEBUG = -g -DDEBUG

# ****************************************************

all: staticMaxHeap.o stopword.lst
	$(CC) $(CFLAGS) qp_bm25.c -o qp_bm25 staticMaxHeap.o -lm

debug: CFLAGS += $(DEBUG)
debug: all

staticMaxHeap.o: staticMaxHeap.c staticMaxHeap.h
	$(CC) -g -c staticMaxHeap.c

clean:
	rm -f *.o
	rm -f qp_bm25

distclean:
	rm -f *.o
	rm -f qp_bm25
# currently there is no known output
