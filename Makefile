PREFIX?=/usr/local
LOCALBASE?=${PREFIX}
CFLAGS+=-Wall -O2 -I${PREFIX}/include -DPREFIX='"${PREFIX}"'
LDFLAGS+=-L${LOCALBASE}/lib

all: termim termim-next libtermim

termim: utf8.c term.c termim.c
	$(CC) ${CFLAGS} utf8.c term.c termim.c -o termim -lutil ${LDFLAGS}

libtermim: termim.h libtermim.c
	$(CC) ${CFLAGS} libtermim.c -fPIC -shared -o libtermim.so

install:
	install -s -m 555 termim ${PREFIX}/bin
	install -s -m 555 termim-next ${PREFIX}/bin
	install -s -m 555 libtermim.so ${PREFIX}/lib
	install -m 555 termim.h ${PREFIX}/include

clean:
	rm -f termim termim-next libtermim.so
