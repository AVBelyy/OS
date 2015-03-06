CC = gcc
CC_OPTS = -Wall -I../lib
LD_OPTS = -L../lib -lhelpers

PROGRAM = cat

all: $(PROGRAM)

clean:
	rm -f $(PROGRAM)
	rm -f $(PROGRAM).o

<<<<<<< HEAD
$(PROGRAM): $(PROGRAM).o ../lib/libhelpers.so
=======
../lib/libhelpers.so:
	$(MAKE) -C $(dir $@) $(notdir $@)

cat: cat.o ../lib/libhelpers.so
>>>>>>> 56a719242f567684d7dcd54131b758e0e5fb8dc1
	$(CC) -o $@ $(LD_OPTS) $<

$(PROGRAM).o: $(PROGRAM).c
	$(CC) -o $@ -c $(CC_OPTS) $<
