# *****************************************************
# Parameters to control Makefile operation

CC = gcc
CFLAGS = -Wall -D_FILE_OFFSET_BITS=64 -w

# ****************************************************

all: ulib.o staticMaxHeap.o diskTimeSim.o
	$(CC) $(CFLAGS) fsmem18.c -o query ulib.o staticMaxHeap.o diskTimeSim.o -lm

diskTimeSim.o: diskTimeSim.c
	$(CC) -c diskTimeSim.c

staticMaxHeap.o: staticMaxHeap.c staticMaxHeap.h
	$(CC) -c staticMaxHeap.c

ulib.o: ulib.c ulib.h
	$(CC) -c ulib.c

clean:
	rm -f *.o
	rm -f query

distclean:
	rm -f *.o
	rm -f query
	rm -f log.txt
	rm -f result.txt
	rm -f stats.txt
