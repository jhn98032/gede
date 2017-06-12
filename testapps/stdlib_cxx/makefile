CXXFLAGS+=-g -Wall -O0

all: test

test: test.o
	$(CXX) -o $@ $^


clean:
	rm -f test
	rm -f *.o

distclean:
	@(MAKE) clean
	chmod a-x *.cpp
	chmod a-x makefile
	#rm -f *~
	chmod a+x start_gede.sh

