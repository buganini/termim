DESTDIR?=
PREFIX?=/usr/local
LOCALBASE?=${PREFIX}
CFLAGS+=-Wall -O2 -I${PREFIX}/include -I../.. -DPREFIX='"${PREFIX}"'
LDFLAGS+=-L${LOCALBASE}/lib -ltermim -lm17n

all: termim-m17n

termim-m17n: termim-m17n.c
	$(CC) ${CFLAGS} termim-m17n.c -o termim-m17n ${LDFLAGS}

install:
	install -s -m 555 termim-m17n ${DESTDIR}${PREFIX}/bin

clean:
	rm -f termim-m17n
