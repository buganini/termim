PREFIX?=/usr/local
CFLAGS+=-Wall -I${PREFIX}/include -DPREFIX='"${PREFIX}"'
LDFLAGS+=-L${PREFIX}/lib

all: termim termim-next libtermim

termim: utf8.c term.c termim.c
	$(CC) ${CFLAGS} utf8.c term.c termim.c -o termim -lutil ${LDFLAGS}

libtermim: termim.h libtermim.c
	$(CC) ${CFLAGS} libtermim.c -fPIC -shared -o libtermim.so

termim-chewing: libtermim utf8.c termim-chewing.c
	$(CC) ${CFLAGS} utf8.c termim-chewing.c -o termim-chewing -ltermim -lchewing ${LDFLAGS}

termim-kmfl: termim-kmfl.c
	$(CC) ${CFLAGS} termim-kmfl.c -o termim-kmfl -lkmfl ${LDFLAGS}

termim-m17n: termim-m17n.c
	$(CC) ${CFLAGS} termim-m17n.c -o termim-m17n -lm17n ${LDFLAGS}

install: install-termim install-termim-chewing install-termim-kmfl install-termim-m17n

install-termim:
	install -s -m 555 termim ${PREFIX}/bin
	install -s -m 555 termim-next ${PREFIX}/bin
	install -s -m 555 libtermim.so ${PREFIX}/lib

install-termim-chewing:
	install -s -m 555 termim-chewing ${PREFIX}/bin

install-termim-kmfl:
	install -s -m 555 termim-kmfl ${PREFIX}/bin

install-termim-m17n:
	install -s -m 555 termim-m17n ${PREFIX}/bin

clean:
	rm -f termim
	rm -f termim-next
	rm -f termim-chewing
	rm -f termim-kmfl
	rm -f termim-m17n
