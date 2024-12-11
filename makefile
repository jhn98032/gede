
all:
	./build.py --verbose

install:
	./build.py install

clean:
	./build.py clean
	chmod a+x *.py

