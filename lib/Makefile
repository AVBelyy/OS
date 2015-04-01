CC = gcc
CC_OPTS = -Wall -std=c99 -fpic -I.
LD_OPTS = 

all: libbufio.so libhelpers.so

clean:
	rm -f libbufio.so
	rm -f libhelpers.so
	rm -f bufio.o
	rm -f helpers.o

libbufio.so: bufio.o
	$(CC) -o $@ -shared $< $(LD_OPTS)

bufio.o: bufio.c
	$(CC) -o $@ -c $< $(CC_OPTS)

libhelpers.so: helpers.o
	$(CC) -o $@ -shared $< $(LD_OPTS)

helpers.o: helpers.c
	$(CC) -o $@ -c $< $(CC_OPTS)
