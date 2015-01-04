DESTDIR?=
PREFIX?=/usr/local
LOCALBASE?=${PREFIX}
CFLAGS+=-Wall -O2 -I${PREFIX}/include -I../.. -DPREFIX='"${PREFIX}"'
LDFLAGS+=-L${LOCALBASE}/lib -ltermim -lchewing

all: termim-chewing

termim-chewing: termim-chewing.c
	$(CC) ${CFLAGS} ../../utf8.c termim-chewing.c -o termim-chewing ${LDFLAGS}

install:
	install -s -m 555 termim-chewing ${DESTDIR}${PREFIX}/bin

clean:
	rm -f termim-chewing
