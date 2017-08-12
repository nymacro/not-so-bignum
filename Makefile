CFLAGS=-g -Wall
.PHONY: clean

nbn: nbn.o

clean:
	-$(RM) nbn.o

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
