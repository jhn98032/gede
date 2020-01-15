
all: test.adb
	gnatmake  -g  $^

clean:
	rm -f test
	rm -f *.o
	rm -f *.ali
	rm -f *.ads
	rm -f b~*.*

distclean:
	make clean
	rm -f gede_gdb_log.txt

.PHONY: all distclean clean

