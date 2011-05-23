CFLAGS+=-g -Wall -I/usr/local/include
LDFLAGS+=-L/usr/local/lib

all:
	$(CC) ${CFLAGS} ${LDFLAGS} -lutil utf8.c common.c tty.c term.c termim.c -o termim
	$(CC) ${CFLAGS} ${LDFLAGS} -lchewing utf8.c termim-chewing.c -o termim-chewing

install:
	install -s -m 555 termim /usr/local/bin
	install -m 555 termim-chewing /usr/local/bin

clean:
	rm -f termim
	rm -f termim-chewing
