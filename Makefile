# See LICENSE file for copyright and license details.

VERSION = 2.0beta4

PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

CPPFLAGS = -DVERSION=\"${VERSION}\"
CFLAGS   = -std=c99 -pthread -pedantic -Wall -Wextra \
	   -Wno-unused-parameter -O3 ${CPPFLAGS}
DBGFLAGS = -std=c99 -pthread -pedantic -Wall -Wextra \
           -Wno-unused-parameter -g ${CPPFLAGS}

CC = cc


all: options nissy

options:
	@echo nissy build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "CC       = ${CC}"

nissy:
	${CC} ${CFLAGS} -o nissy src/*.c

win:
	x86_64-w64-mingw32-gcc ${CFLAGS} -static -o nissy.exe src/*.c

debug:
	${CC} ${DBGFLAGS} -o nissy src/*.c

clean:
	rm -rf nissy nissy-${VERSION}.tar.gz

dist: clean
	mkdir -p nissy-${VERSION}
	cp -R LICENSE Makefile INSTALL doc src nissy-${VERSION}
	groff -Tpdf   -mandoc doc/nissy.1 > nissy-${VERSION}/doc/nissy.pdf 
	groff -Thtml  -mandoc doc/nissy.1 > nissy-${VERSION}/doc/nissy.html 
	tar -cf nissy-${VERSION}.tar nissy-${VERSION}
	gzip nissy-${VERSION}.tar
	rm -rf nissy-${VERSION}

install: nissy
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f nissy ${DESTDIR}${PREFIX}/bin/nissy
	chmod 755 ${DESTDIR}${PREFIX}/bin/nissy
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < doc/nissy.1 \
				     > ${DESTDIR}${MANPREFIX}/man1/nissy.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/nissy.1

uninstall:
	rm -rf ${DESTDIR}${PREFIX}/bin/nissy ${DESTDIR}${MANPREFIX}/man1/nissy.1
	for s in ${SCRIPTS}; do rm -rf ${DESTDIR}${PREFIX}/bin/$$s; done

.PHONY: all options nissy win debug clean dist install uninstall

