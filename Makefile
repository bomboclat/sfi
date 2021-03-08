# libsfi
# (c) 2021, 2022 Andrea Galletti

VERSION = 1.0
# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
# includes and libs
INCS = -I. -I/usr/include
LIBS = -L/usr/lib
# flags
CFLAGS = -O0 -Wall -Werror -ansi ${INCS} -DVERSION=\"${VERSION}\"
LDFLAGS = ${LIBS}
# srcs / objs
SRC    = sfi.c
OBJ    = ${SRC:.c=.o}

# Main targets
all: sfi

sfi: ${OBJ}
	@echo LD $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

# Targets deps
.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

# Helper targets
clean:
	@echo cleaning
	@rm -f sfi ${OBJ} ${LIBOBJ} sfi-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p sfi-${VERSION}
	@cp -R LICENSE Makefile sfi.1 ${SRC} sfi-${VERSION}
	@tar -zcf sfi-${VERSION}.tar.gz sfi-${VERSION}
	@rm -rf sfi-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f sfi ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/sfi
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < sfi.1 > ${DESTDIR}${MANPREFIX}/man1/sfi.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/sfi.1

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/sfi
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/sfi.1

.PHONY: all clean dist install uninstall
