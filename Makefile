CFLAGS+=-g -Wall -I/usr/local/include
LDFLAGS+=-L/usr/local/lib

all:
	$(CC) ${CFLAGS} ${LDFLAGS} -lutil common.c tty.c term.c termim.c -o termim
	$(CC) ${CFLAGS} ${LDFLAGS} -lchewing termim-chewing.c -o termim-chewing

install:
	install -s -m 555 termim /usr/local/bin
#	mkdir -p /usr/local/share/termim/ibus
#	install -m 555 ibus/* /usr/local/share/termim/ibus
	install -m 555 termim-chewing /usr/local/bin

clean:
	rm termim
