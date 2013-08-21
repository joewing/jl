
CC=gcc
CFLAGS=-O2 -Wall -Werror -I. -g
LDFLAGS=-g

JLOBJS=src/jl.o src/jl-context.o src/jl-func.o src/jl-scope.o src/jl-value.o
REPLOBJS=src/jli.o jl.a

all: jli jl.a

jli: $(REPLOBJS)
	$(CC) $(LDFLAGS) $(REPLOBJS) -o jli

jl.a: $(JLOBJS)
	ar r jl.a $(JLOBJS)

.c.o: $*.c *.h
	$(CC) $(CFLAGS) -c $*.c -o $*.o

clean:
	rm -f jli jl.a src/*.o

