prefix=/usr/local
all:
	clang maclookup.c -o maclookup
install: maclookup
	clang maclookup.c -o maclookup
	install -m 0755 maclookup $(prefix)/bin

