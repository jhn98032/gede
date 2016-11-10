CFLAGS+=-g -O0 -Wall
LDFLAGS+=-lpthread
OBJ=subdir/subfile.o test.o

test: subdir/subfile.o test.o
	$(CC) -o $@ $^ $(LDFLAGS)


clean:
	rm -f test
	rm -f $(OBJ)


