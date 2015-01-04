DESTDIR?=
PREFIX?=/usr/local
LOCALBASE?=${PREFIX}
CFLAGS+=-Wall -O2 -I${PREFIX}/include -I../.. -DPREFIX='"${PREFIX}"'
LDFLAGS+=-L${LOCALBASE}/lib -ltermim -lkmfl

all: termim-kmfl

termim-kmfl: termim-kmfl.c
	$(CC) ${CFLAGS} termim-kmfl.c -o termim-kmfl ${LDFLAGS}

install:
	install -s -m 555 termim-kmfl ${DESTDIR}${PREFIX}/bin

clean:
	rm -f termim-kmfl
