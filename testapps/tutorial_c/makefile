CFLAGS+=-m64 -g -O0 -Wall
LDFLAGS+=-m64

OBJ=func.o main.o

test: func.o  main.o
	$(CC) -o $@ $^ $(LDFLAGS)


clean:
	rm -f test
	rm -f $(OBJ)

