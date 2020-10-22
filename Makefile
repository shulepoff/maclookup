DESTDIR ?= /usr/local
SRC := src/maclookup.c
CC = clang
all:
	$(CC) -o maclookup $(SRC)
install: maclookup
	install -m 0755 maclookup $(DESTDIR)/bin

