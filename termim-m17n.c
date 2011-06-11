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
 * This library draw on tty specified by $slave
 * with specified offset and deal with window size
 *
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>

#include <m17n.h>

#include "keymap.h"
#include "utf8.h"

int out;
struct winsize win;
static MConverter *utf8conv = NULL;

void
winch(int sig){
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
}

void m17n_interpret(MInputContext *ic, char c){
	char buf[64];
	unsigned char *obuf;
	int bufsiz;
	MSymbol ms;
	MText *text;

	sprintf(buf, "%c", c);
	ms=msymbol(buf);
	if(minput_filter(ic, ms, NULL))
		return;

	text=mtext();
	minput_lookup(ic, ms, NULL, text);
	mconv_reset_converter(utf8conv);
	bufsiz=(mtext_len(text)+1)*6;
	obuf=malloc(bufsiz);
	mconv_rebind_buffer(utf8conv, obuf, bufsiz);
	mconv_encode(utf8conv, text);
	write(out, obuf, utf8conv->nbytes);
	m17n_object_unref(text);
}

int main(int argc, char *argv[]){
	char *eargv[]={"termim-next", NULL};
	char *s;
	unsigned char ibuf[BUFSIZ];
	char escape_buf[128];
	int escape_i=0;
	int escape=0;
	int i;
	int n;
	MInputMethod *im;
	MInputContext *ic;
	fd_set rfd;

	if((s=getenv("TERMIM"))!=NULL)
		out=strtol(s, NULL, 10);
	else
		out=STDOUT_FILENO;

	signal(SIGWINCH, &winch);
	winch(0);

	if(argc!=3){
		exit(1);
	}

	M17N_INIT();
	im=minput_open_im(msymbol(argv[1]), msymbol(argv[2]), NULL);
	ic=minput_create_ic(im, NULL);

	utf8conv=mconv_buffer_converter(Mcoding_utf_8, NULL, 0);

	printf("\033[H\033[44m\033[?25l");
	printf("\033[K");
	printf("M17N: %s-%s\n", argv[1], argv[2]);
	printf("\033[K");

	FD_ZERO(&rfd);
	while(1){
		FD_SET(STDIN_FILENO, &rfd);
		n = select(STDIN_FILENO+1, &rfd, 0, 0, NULL);
		if (n < 0 && errno != EINTR)
			break;
		if (n > 0 && FD_ISSET(STDIN_FILENO, &rfd)){
			n=read(STDIN_FILENO, ibuf, BUFSIZ);
			if(n<=0){
			}else{
				if(n==1){
					switch((unsigned char)*ibuf){
						case CTRL_SHIFT:
							execvp(eargv[0], eargv);
							break;
					}
				}
				for(i=0;i<n;++i){
					switch((unsigned char)(ibuf[i])){
						case '\x1b':
							if(escape)
								write(out, escape_buf, escape_i);
							escape_i=0;
							escape=1;
							break;
					}
					if(escape){
						escape_buf[escape_i]=ibuf[i];
						escape_i+=1;
						if(escape_i==2 && escape_buf[1]!='['){
							escape=0;						
							switch(escape_buf[1]){
								case '3':
									execvp(eargv[0], eargv);
									break;
								default:
									write(out, escape_buf, escape_i);
							}
						}
						if((ibuf[i]>='a' && ibuf[i]<='z') || (ibuf[i]>='A' && ibuf[i]<='N') || (ibuf[i]>='P' && ibuf[i]<='Z') || ibuf[i]=='~'){
							escape=0;
							write(out, escape_buf, escape_i);
						}
					}else{
						if((ibuf[i] & 0xFF00)==0 && isprint(ibuf[i]))
							m17n_interpret(ic, ibuf[i]);
						else{
							write(out, ibuf+i, 1);
						}
					}
				}
			}
		}
	}

	return 0;
}
