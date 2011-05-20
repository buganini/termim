CFLAGS+=-g -Wall -I/usr/local/include
LDFLAGS+=-lutil -L/usr/local/lib

all:
	$(CC) ${CFLAGS} ${LDFLAGS} termim.c -o termim

install:
	install -s -m 555 termim /usr/local/bin
	install -m 555 termim-ibus /usr/local/libexec

clean:
	rm termim
