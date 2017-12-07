CFLAGS=-fPIC -fpic -g -Wall
.PHONY: clean all

all: libnbn.so test

libnbn.so: nbn.o
	$(CC) -shared $(CFLAGS) -o $@ $<

test:CFLAGS+=-L. -lnbn
test: test.o
	$(CC) $(CFLAGS) -o $@ $<

clean:
	-$(RM) test.o nbn.o libnbn.so test

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
