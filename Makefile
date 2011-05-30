PREFIX?=/usr/local
CFLAGS+=-g -Wall -I/usr/local/include -DPREFIX='"${PREFIX}"'
LDFLAGS+=-L/usr/local/lib

all:
	$(CC) ${CFLAGS} ${LDFLAGS} -lutil utf8.c term.c termim.c -o termim
	$(CC) ${CFLAGS} ${LDFLAGS} -lchewing utf8.c termim-chewing.c -o termim-chewing

install:
	install -s -m 555 termim ${PREFIX}/bin
	install -s -m 555 termim-chewing ${PREFIX}/bin

clean:
	rm -f termim
	rm -f termim-chewing
