GENS=startpage.htm


all: $(GENS)

%.htm: %.rst
	rst2html --stylesheet=style.css $^ $@

%.odt: %.rst
	rst2odt $^ $@

%.pdf: %.odt
	odt2pdf $^


clean:
	rm -f $(GENS)
