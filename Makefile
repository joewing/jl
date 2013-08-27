
CC=clang
CFLAGS=-O2 -Wall -Werror -I. -fpic -fvisibility=hidden
LDFLAGS=

JLOBJS=src/jl.o src/jl-context.o src/jl-func.o src/jl-scope.o src/jl-value.o
REPLOBJS=src/jli.o jl.a

all: jli jl.a libjl.so

jli: $(REPLOBJS)
	$(CC) $(LDFLAGS) $(REPLOBJS) -o jli
	strip jli

libjl.so: $(JLOBJS)
	$(CC) $(LDFLAGS) -shared $(JLOBJS) -o libjl.so

jl.a: $(JLOBJS)
	ar r jl.a $(JLOBJS)

.c.o: $*.c *.h
	$(CC) $(CFLAGS) -c $*.c -o $*.o

clean:
	rm -f jli jl.a libjl.so src/*.o

