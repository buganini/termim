/*
 * Copyright (c) 2011 Kuan-Chung Chiu <buganini@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * This library read from tty and doing some process
 * and write to another tty
 *
 */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/kbio.h>
#include "tty.h"
#include "common.h"
#include "keymap.h"

volatile int lock=0;

void siginfo(int sig){
	lock-=1;
}

struct tty * tty_create(){
	struct tty *ret=malloc(sizeof(struct tty));
	ret->escape=0;
	signal(SIGINFO, &siginfo);
	return ret;
}

void tty_destroy(struct tty *tty){
	free(tty);
}

void tty_init(void){
	keymap_t kmap;

	ioctl(STDIN_FILENO, GIO_KEYMAP, &kmap);
	kmap.key[42].map[2] = CTRL_SHIFT;
	kmap.key[54].map[2] = CTRL_SHIFT;
	kmap.key[57].map[2] = CTRL_SPACE;
	kmap.key[57].map[1] = SHIFT_SPACE;
	ioctl(STDIN_FILENO, PIO_KEYMAP, &kmap);
}

void tty_assoc_output(struct tty *tty, int out){
	tty->out=out;
}

#define WRITE(X,Y,Z) do{ \
r=write((X),(Y),(Z)); \
if(r<0){ \
	return r; \
} \
ret+=r; \
}while(0);

ssize_t tty_writer(struct tty *tty, char *ibuf, size_t len){
	int r=0, ret=0;
	char buf[128];

//	bypass processing:
//	return write(tty->out, ibuf, len);
	for(r=0;r<len;++r)
	if(len==1){
		switch((unsigned char)(*ibuf)){
			case UP:
				WRITE(tty->out, buf, sprintf(buf, "\033[A"));
				break;
			case DOWN:
				WRITE(tty->out, buf, sprintf(buf, "\033[B"));
				break;
			case LEFT:
				WRITE(tty->out, buf, sprintf(buf, "\033[D"));
				break;
			case RIGHT:
				WRITE(tty->out, buf, sprintf(buf, "\033[C"));
				break;
			default:
				WRITE(tty->out, ibuf, 1);
				break;
		}
	}else{
		WRITE(tty->out, ibuf, len);
	}
	return ret;
}

#define ALONE_WRITE(X,Y,Z) do{ \
lock+=1; \
r=write((X),(Y),(Z)); \
while(lock!=0); \
if(r<0){ \
	return r; \
} \
ret+=r; \
}while(0);

ssize_t tty_writev(struct tty *tty, char *ibuf, size_t len){
	int i, j, r=0, ret=0;
	char buf[128];

//	bypass processing:
//	return write(tty->out, ibuf, len);

	for(j=i=0;i<len;++i){
		switch((unsigned char)(ibuf[i])){
			case '\x1b':
				tty->i=0;
				tty->escape=1;
				if(i-j>0)
					ALONE_WRITE(tty->out, ibuf+j, i-j);
				break;
			case CTRL_SPACE:
				ALONE_WRITE(tty->out, buf, sprintf(buf, "%c", CTRL_PRESS));
				ALONE_WRITE(tty->out, " ", 1);
				ALONE_WRITE(tty->out, buf, sprintf(buf, "%c", CTRL_RELEASE));
				j=i+1;
				break;			
			case SHIFT_SPACE:
				ALONE_WRITE(tty->out, buf, sprintf(buf, "%c", SHIFT_PRESS));
				ALONE_WRITE(tty->out, " ", 1);
				ALONE_WRITE(tty->out, buf, sprintf(buf, "%c", SHIFT_RELEASE));
				j=i+1;
				break;			
		}
		if(tty->escape){
			tty->buf[tty->i]=ibuf[i];
			tty->i+=1;
			if(tty->i==2 && tty->buf[1]!='['){
				tty->escape=0;
				j=i+1;
				if((ibuf[1]>='0' && ibuf[1]<='9') || (ibuf[1]>='a' && ibuf[1]<='z') || (ibuf[1]>='A' && ibuf[1]<='Z')){
					ALONE_WRITE(tty->out, buf, sprintf(buf, "%c", ALT_PRESS));
					ALONE_WRITE(tty->out, &tty->buf[1], 1);
					ALONE_WRITE(tty->out, buf, sprintf(buf, "%c", ALT_RELEASE));
				}else{
					ALONE_WRITE(tty->out, tty->buf, 2);
				}
				continue;
			}
			if((ibuf[i]>='a' && ibuf[i]<='z') || (ibuf[i]>='A' && ibuf[i]<='Z') || ibuf[i]=='~'){
				tty->escape=0;
				j=i+1;
				switch(tty->buf[1]){
					case '[':
						switch(ibuf[i]){
							case 'A':
								if(tty->i==3){
									ALONE_WRITE(tty->out, buf, sprintf(buf, "%c", UP));
								}else{
									ALONE_WRITE(tty->out, tty->buf, tty->i);
								}
								break;
							case 'B':
								if(tty->i==3){
									ALONE_WRITE(tty->out, buf, sprintf(buf, "%c", DOWN));
								}else{
									ALONE_WRITE(tty->out, tty->buf, tty->i);
								}
								break;
							case 'C':
								if(tty->i==3){
									ALONE_WRITE(tty->out, buf, sprintf(buf, "%c", RIGHT));
								}else{
									ALONE_WRITE(tty->out, tty->buf, tty->i);
								}
								break;
							case 'D':
								if(tty->i==3){
									ALONE_WRITE(tty->out, buf, sprintf(buf, "%c", LEFT));
								}else{
									ALONE_WRITE(tty->out, tty->buf, tty->i);
								}
								break;
							default:
								ALONE_WRITE(tty->out, tty->buf, tty->i);
								break;
						}
						break;
					default:
						ALONE_WRITE(tty->out, tty->buf, tty->i);
						break;
				}
			}
		}
	}
	if(tty->escape==0 && i-j>0){
		ALONE_WRITE(tty->out, ibuf+j, i-j);
	}

	return ret;
}
