CFLAGS+=-g -O0 -Wall
LDFLAGS+=-lpthread

test: subdir/subfile.o test.o
	$(CC) -o $@ $^ $(LDFLAGS)


clean:
	rm -f test
	rm -f *.o

