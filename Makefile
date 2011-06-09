PREFIX?=/usr/local
CFLAGS+=-g -Wall -I/usr/local/include -DPREFIX='"${PREFIX}"'
LDFLAGS+=-L/usr/local/lib

all: termim termim-next termim-chewing termim-kmfl

termim: utf8.c term.c termim.c
	$(CC) ${CFLAGS} ${LDFLAGS} -lutil utf8.c term.c termim.c -o termim

termim-chewing: utf8.c termim-chewing.c
	$(CC) ${CFLAGS} ${LDFLAGS} -lchewing utf8.c termim-chewing.c -o termim-chewing

termim-kmfl: utf8.c termim-kmfl.c
	$(CC) ${CFLAGS} ${LDFLAGS} -lkmfl utf8.c termim-kmfl.c -o termim-kmfl

install: install-termim install-termim-chewing install-termim-kmfl

install-termim:
	install -s -m 555 termim ${PREFIX}/bin
	install -s -m 555 termim-next ${PREFIX}/bin

install-termim-chewing:
	install -s -m 555 termim-chewing ${PREFIX}/bin

install-termim-kmfl:
	install -s -m 555 termim-kmfl ${PREFIX}/bin

clean:
	rm -f termim
	rm -f termim-chewing
	rm -f termim-kmfl
