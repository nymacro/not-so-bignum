CC=clang
CFLAGS=-fPIC -fpic -g -Wall # -fsanitize=memory -fno-omit-frame-pointer
LDFLAGS= -Wl,--rpath $(PWD) -L. -lnbn

.PHONY: clean all

all: libnbn.so test

libnbn.so: nbn.o
	$(CC) -shared $(CFLAGS) -o $@ nbn.o

test: test.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ test.o

clean:
	-$(RM) test.o nbn.o libnbn.so test

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

