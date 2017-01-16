OUTPUT=index.html

all: $(OUTPUT)

index.html: local_application.rst
	rst2html $^ $@

%.odt: %.rst
	rst2odt -g --add-syntax-highlight  $^ $@

clean:
	rm -f $(OUTPUT)


