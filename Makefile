
CC=gcc
CFLAGS=-O2 -Wall -Werror -g
LDFLAGS=-g

all: *.c *.h
	$(CC) $(CFLAGS) $(LDFLAGS) jl.c repl.c -o repl

clean:
	rm -f repl

