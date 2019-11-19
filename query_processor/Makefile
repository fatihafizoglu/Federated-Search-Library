# *****************************************************
# Parameters to control Makefile operation

CC = gcc
CFLAGS = -Wall -D_FILE_OFFSET_BITS=64 #-w
# Here we are using -w to inhibit warning messages
DEBUG = -g -DDEBUG

# ****************************************************

all: qp_bm25.c qp_bm25.h staticMaxHeap.c staticMaxHeap.h stopword.lst
	$(CC) $(CFLAGS) qp_bm25.c staticMaxHeap.c -lm -o qp_bm25

debug: CFLAGS += $(DEBUG)
debug: all

xquad: CFLAGS += -DXQUAD
xquad: all

debugxquad: CFLAGS += -DXQUAD $(DEBUG)
debugxquad: all

tester: tester.c qp_bm25.c qp_bm25.h staticMaxHeap.c staticMaxHeap.h stopword.lst
	$(CC) $(CFLAGS) -DTESTER -DDEBUG -g tester.c qp_bm25.c staticMaxHeap.c -lm -o tester

clean:
	rm -f *.o
	rm -f qp_bm25
	rm -f tester
