#This is the makefile for the compressor executable.

CC=gcc
CFLAGS=-Isrc/ -Ibuild/
BIN=bin/
BUILD=build/
SRC=src/

all: Compression

Compression.o: src/Compression.c
	$(CC) $(CFLAGS) -c $(SRC)Compression.c  -o $(BUILD)$@

Compression: Compression.o
	$(CC) $(CFLAGS) $(BUILD)Compression.o -o $(BIN)Compressor

clean:
	rm $(BUILD)*.o $(BIN)Compressor
