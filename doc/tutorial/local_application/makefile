OUTPUT=local_application.html

all: $(OUTPUT)

%.html: %.rst
	rst2html $^ $@

%.odt: %.rst
	rst2odt -g --add-syntax-highlight  $^ $@

clean:
	rm -f $(OUTPUT)


