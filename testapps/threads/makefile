CFLAGS+=-g -Wall -O0
LDFLAGS+=-g -lpthread

all: test

test: test.o
	$(CC) -o $@ $^ $(LDFLAGS)


clean:
	rm -f test
	rm -f *.o

