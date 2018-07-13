CFLAGS+=-g -Wall -O0
LDFLAGS+=-g

all: test

test: other.o test.o
	$(CC) -o $@ $^ $(LDFLAGS)


clean:
	rm -f test
	rm -f *.o

