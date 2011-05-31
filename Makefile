PREFIX?=/usr/local
CFLAGS+=-g -Wall -I/usr/local/include -DPREFIX='"${PREFIX}"'
LDFLAGS+=-L/usr/local/lib

all: termim termim-chewing

termim:
	$(CC) ${CFLAGS} ${LDFLAGS} -lutil utf8.c term.c termim.c -o termim

termim-chewing:
	$(CC) ${CFLAGS} ${LDFLAGS} -lchewing utf8.c termim-chewing.c -o termim-chewing

install: install-termim install-termim-chewing

install-termim:
	install -s -m 555 termim ${PREFIX}/bin

install-termim-chewing:
	install -s -m 555 termim-chewing ${PREFIX}/bin

clean:
	rm -f termim
	rm -f termim-chewing
