CC = gcc
CC_OPTS = -Wall -I../lib
LD_OPTS = -L../lib -lhelpers

all: cat

clean:
	rm -f cat
	rm -f cat.o

cat: cat.o ../lib/libhelpers.so
	$(CC) -o $@ $(LD_OPTS) $<

cat.o: cat.c
	$(CC) -o $@ -c $(CC_OPTS) $<
