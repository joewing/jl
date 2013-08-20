
CC=gcc
CFLAGS=-O2 -Wall -Werror -g
LDFLAGS=-g

JLOBJS=src/jl.o src/jl-context.o src/jl-func.o src/jl-scope.o src/jl-value.o
REPLOBJS=src/repl.o jl.a

all: repl jl.a

repl: $(REPLOBJS)
	$(CC) $(LDFLAGS) $(REPLOBJS) -o repl

jl.a: $(JLOBJS)
	ar r jl.a $(JLOBJS)

.c.o: $*.c *.h
	$(CC) $(CFLAGS) -c $*.c -o $*.o

clean:
	rm -f repl jl.a src/*.o

