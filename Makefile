CFLAGS+=-g -Wall -I/usr/local/include
LDFLAGS+=-lutil -L/usr/local/lib

all:
	$(CC) ${CFLAGS} ${LDFLAGS} termim.c -o termim

clean:
	rm termim
