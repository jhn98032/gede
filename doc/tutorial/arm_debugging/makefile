OUTPUT=arm_debugging.html

all: $(OUTPUT)

%.html: %.rst
	rst2html $^ $@

%.odt: %.rst
	rst2odt -g --add-syntax-highlight  $^ $@

clean:
	rm -f $(OUTPUT)


