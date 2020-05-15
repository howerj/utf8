VERSION = 0x000001ul
CFLAGS  =-std=c99 -Wall -Wextra -pedantic -O2 -DUTF8_VERSION="${VERSION}"
TARGET  =utf8
AR      = ar
ARFLAGS = rcs
TRACE   =
DESTDIR = install

.PHONY: all run test clean install dist


all: ${TARGET}

run: test
	${TRACE} ./${TARGET}

test: ${TARGET}
	./${TARGET}

main.o: main.c ${TARGET}.h

${TARGET}.o: ${TARGET}.c ${TARGET}.h


lib${TARGET}.a: ${TARGET}.o
	${AR} ${ARFLAGS} $@ $<

${TARGET}: main.o lib${TARGET}.a
	${CC} ${CFLAGS} $^ -o $@
	-strip ${TARGET}

${TARGET}.1: readme.md
	pandoc -s -f markdown -t man $< -o $@

install: ${TARGET} lib${TARGET}.a ${TARGET}.1 .git
	install -p -D ${TARGET} ${DESTDIR}/bin/${TARGET}
	install -p -m 644 -D lib${TARGET}.a ${DESTDIR}/lib/lib${TARGET}.a
	install -p -m 644 -D ${TARGET}.h ${DESTDIR}/include/${TARGET}.h
	-install -p -m 644 -D ${TARGET}.1 ${DESTDIR}/man/${TARGET}.1
	mkdir -p ${DESTDIR}/src
	cp -a .git ${DESTDIR}/src
	cd ${DESTDIR}/src && git reset --hard HEAD

dist: install
	tar zcf ${TARGET}-${VERSION}.tgz ${DESTDIR}

check:
	-scan-build make
	-cppcheck --enable=all *.c

clean:
	git clean -dffx

