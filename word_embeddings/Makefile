# *****************************************************
# Parameters to control Makefile operation

CC = gcc
CFLAGS = -Wall -D_FILE_OFFSET_BITS=64
DEBUG = -g -DDEBUG
BIN_NAME = query_expand

# RELATIVE_ALLOCATOR_PATH
RAP = ../topic_based/c/
# ****************************************************

all: query_expand.c word_embeddings.c word_embeddings.h
	$(CC) $(CFLAGS) query_expand.c word_embeddings.c -lm -o $(BIN_NAME)

debug: CFLAGS += $(DEBUG)
debug: all

clean:
	rm -f *.o
	rm -f $(BIN_NAME)
