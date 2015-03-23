CC = gcc
CC_OPTS = -Wall -O2 -std=c99 -I../lib
LD_OPTS = -L../lib -lhelpers

PROGRAM = lenwords

all: $(PROGRAM)

clean:
	rm -f $(PROGRAM)
	rm -f $(PROGRAM).o

$(PROGRAM): $(PROGRAM).o ../lib/libhelpers.so
	$(CC) -o $@ $< $(LD_OPTS)

../lib/libhelpers.so:
	$(MAKE) -C $(dir $@) $(notdir $@)

$(PROGRAM).o: $(PROGRAM).c
	$(CC) -o $@ -c $< $(CC_OPTS)
