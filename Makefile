
CC=gcc
CFLAGS=-O2 -Wall -Werror -g
LDFLAGS=-g

JLOBJS=src/jl.o
REPLOBJS=src/repl.o $(JLOBJS)

all: repl

repl: $(REPLOBJS)
	$(CC) $(LDFLAGS) $(REPLOBJS) -o repl

.c.o: $*.c *.h
	$(CC) $(CFLAGS) -c $*.c -o $*.o

clean:
	rm -f repl src/*.o

