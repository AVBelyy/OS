CC = gcc
CC_OPTS = -Wall -I../lib
LD_OPTS = -L../lib -lhelpers

PROGRAM = cat

all: $(PROGRAM)

clean:
	rm -f $(PROGRAM)
	rm -f $(PROGRAM).o

$(PROGRAM): $(PROGRAM).o ../lib/libhelpers.so
	$(CC) -o $@ $(LD_OPTS) $<

$(PROGRAM).o: $(PROGRAM).c
	$(CC) -o $@ -c $(CC_OPTS) $<
