# *****************************************************
# Parameters to control Makefile operation

CC = gcc
CFLAGS = -Wall -D_FILE_OFFSET_BITS=64
DEBUG = -g -DDEBUG
BIN_NAME = csi_sampler

# RELATIVE_ALLOCATOR_PATH
RAP = ../../topic_based/c/
# ****************************************************

all: csi_sampler.c
	$(CC) $(CFLAGS) csi_sampler.c -lm -o $(BIN_NAME)

debug: CFLAGS += $(DEBUG)
debug: all

clean:
	rm -f *.o
	rm -f $(BIN_NAME)
