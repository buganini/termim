#include <sys/types.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/kbio.h>
#include "tty.h"
#include "common.h"
#include "keymap.h"
/*
 * This library draw on tty specified by $slave
 * with specified offset and deal with window size
 *
 */

struct tty * tty_create(){
	struct tty *ret=malloc(sizeof(struct tty));
	ret->escape=0;
	return ret;
}

void tty_destroy(struct tty *tty){
	free(tty);
}

void tty_assoc_input(struct tty *tty, int in){
	keymap_t kmap;

	tty->in=in;

	ioctl(in, GIO_KEYMAP, &kmap);
	kmap.key[14].map[0] = BACKSPACE;
	kmap.key[42].map[2] = CTRL_SHIFT;
	kmap.key[54].map[2] = CTRL_SHIFT;
	kmap.key[57].map[2] = CTRL_SPACE;
	ioctl(in, PIO_KEYMAP, &kmap);
}

void tty_assoc_output(struct tty *tty, int out){
	tty->out=out;
}

#define POST_WRITE() \
if(r<0){ \
	return r; \
} \
ret+=r;

ssize_t tty_read_write(struct tty *tty, char *ibuf, size_t len){
	int i, j, r=0, ret=0;
	char **argv;
	char buf[128];

	len=read(tty->in, ibuf, len);
	for(j=i=0;i<len;++i){
		switch(ibuf[i]){
			case '\x1b':
				tty->i=0;
				tty->escape=1;
				write(tty->out, ibuf+j, i-j);
				break;				
		}
		if(tty->escape){
			tty->buf[tty->i]=ibuf[i];
			tty->i+=1;
			if((ibuf[i]>='a' && ibuf[i]<='z') || (ibuf[i]>='A' && ibuf[i]<='Z')){
				tty->escape=0;
				j=i+1;
				switch(tty->buf[1]){
					case '[':
						switch(ibuf[i]){
							case 'A':
								argv=parse_arg(&tty->buf[2]);
								if(*argv==NULL){
									write(tty->out, buf, sprintf(buf, "%c", UP));
								}else{
									r=write(tty->out, tty->buf, tty->i);
									POST_WRITE();
								}
								break;
							case 'B':
								argv=parse_arg(&tty->buf[2]);
								if(*argv==NULL){
									write(tty->out, buf, sprintf(buf, "%c", DOWN));
								}else{
									r=write(tty->out, tty->buf, tty->i);
									POST_WRITE();
								}
								break;
							case 'C':
								argv=parse_arg(&tty->buf[2]);
								if(*argv==NULL){
									write(tty->out, buf, sprintf(buf, "%c", RIGHT));
								}else{
									r=write(tty->out, tty->buf, tty->i);
									POST_WRITE();
								}
								break;
							case 'D':
								argv=parse_arg(&tty->buf[2]);
								if(*argv==NULL){
									write(tty->out, buf, sprintf(buf, "%c", LEFT));
								}else{
									r=write(tty->out, tty->buf, tty->i);
									POST_WRITE();
								}
								break;
							default:
								r=write(tty->out, tty->buf, tty->i);
								POST_WRITE();
								break;
						}
						break;
					default:
						r=write(tty->out, tty->buf, tty->i);
						POST_WRITE();
						break;
				}
			}
		}
	}
	if(i-j>0){
		r=write(tty->out, ibuf+j, i-j);
		POST_WRITE();
	}

	return ret;
}
