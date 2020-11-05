DESTDIR ?= /usr/local
MANDIR ?= share/man/man1
SRC := src/maclookup.c
MANS := $(wildcard docs/*.1.gz)
CC = clang

all:
	$(CC) -o maclookup $(SRC)
install: all
	install -m 0755 maclookup $(DESTDIR)/bin
	mkdir -p $(DESTDIR)/$(MANDIR)
	cp -R $(MANS) $(DESTDIR)/$(MANDIR)
uninstall: 
	rm -f $(DESTDIR)/bin/maclookup
	rm -f $(DESTDIR)/$(MANDIR)/maclookup.1.gz
