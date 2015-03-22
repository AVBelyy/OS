CC = gcc
CC_OPTS = -Wall -std=c99 -fpic -I.
LD_OPTS = 

all: libhelpers.so

clean:
	rm -f libhelpers.so
	rm -f helpers.o

libhelpers.so: helpers.o
	$(CC) -o $@ -shared $< $(LD_OPTS)

helpers.o: helpers.c
	$(CC) -o $@ -c $< $(CC_OPTS)
