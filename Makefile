PREFIX ?= /usr/local
BINDIR ?= /bin
MANDIR ?= /man

build:
	$(MAKE) -C src

install: src/ebread
	install -v -d $(DESTDIR)$(PREFIX)$(BINDIR)
	install -v -m 755 src/ebread $(DESTDIR)$(PREFIX)$(BINDIR)
	install -v -d $(DESTDIR)$(PREFIX)$(MANDIR)/man1
	install -v -m 644 man/ebread.1 $(DESTDIR)$(PREFIX)$(MANDIR)/man1

clean:
	$(MAKE) clean -C src

.PHONY: build install clean
