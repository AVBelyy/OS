CC = gcc
CC_OPTS = -Wall -O2 -std=c99 -I../lib
LD_OPTS = -L../lib -lhelpers -lbufio

PROGRAM1 = forking
PROGRAM2 = polling

all: $(PROGRAM1) $(PROGRAM2)

clean:
	rm -f $(PROGRAM1) $(PROGRAM2)
	rm -f $(PROGRAM2).o $(PROGRAM2).o

$(PROGRAM1): $(PROGRAM1).o ../lib/libhelpers.so ../lib/libbufio.so
	$(CC) -o $@ $< $(LD_OPTS)

$(PROGRAM2): $(PROGRAM2).o ../lib/libhelpers.so ../lib/libbufio.so
	$(CC) -o $@ $< $(LD_OPTS)

../lib/libhelpers.so:
	$(MAKE) -C $(dir $@) $(notdir $@)

../lib/libbufio.so:
	$(MAKE) -C $(dir $@) $(notdir $@)

$(PROGRAM1).o: $(PROGRAM1).c
	$(CC) -o $@ -c $< $(CC_OPTS)

$(PROGRAM2).o: $(PROGRAM2).c
	$(CC) -o $@ -c $< $(CC_OPTS)
