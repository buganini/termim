CFLAGS+=-g -Wall -I/usr/local/include
LDFLAGS+=-lutil -L/usr/local/lib

all:
	$(CC) ${CFLAGS} ${LDFLAGS} term.c termim.c -o termim

install:
	install -s -m 555 termim /usr/local/bin
	mkdir -p /usr/local/share/termim/ibus
	install -m 555 ibus/* /usr/local/share/termim/ibus

clean:
	rm termim
