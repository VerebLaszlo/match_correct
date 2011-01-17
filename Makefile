CC=gcc
CFLAGS=-c -Wall
#
CC=colorgcc
#

all: test

test: main_Test.c complex_Test.o complex.o windows.o
	$(CC) -o test main_Test.c complex_Test.o complex.o -lfftw3 -lm

complex_Test.o: complex_Test.c complex_Test.h complex.o
	$(CC) $(CFLAGS) -c complex_Test.c complex.o

complex.o: complex.c complex.h variables.h
	$(CC) $(CFLAGS) -c complex.c

windows.o: windows.c windows.h util_math.h
	$(CC) $(CFLAGS) -c windows.c

clean:
	rm -rf *o test

run: test
	./test
