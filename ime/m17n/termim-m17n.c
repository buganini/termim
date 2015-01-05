/*
 * Copyright (c) 2011, 2015 Kuan-Chung Chiu <buganini@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
#ifdef __linux
#include <pty.h>
#include <time.h>
#include <utmp.h>
#else
#include <libutil.h>
#endif

#include <m17n.h>
#include <termim.h>
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
	MInputMethod *im;
	MInputContext *ic;

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


	while(1){
		struct termim_event *event = termim_read_input();
		if(event->type == TERMIM_EVENT_EOF){
			break;
		}else if(event->type == TERMIM_EVENT_RAW){
			if(event->raw)
				write(out, event->raw, event->raw_length);
		}else if(event->type == TERMIM_EVENT_KEY){
			if(event->modifiers == TERMIM_MOD_ALT){
				switch(event->code){
					case TERMIM_KEY_3:
						M17N_FINI();
						execvp(eargv[0], eargv);
						break;
					default:
						if(event->raw)
							write(out, event->raw, event->raw_length);
						break;
				}
			}else if(event->modifiers == 0){
				if(isprint(event->code)){
					m17n_interpret(ic, event->code);
				}else{
					if(event->raw)
						write(out, event->raw, event->raw_length);
				}
			}
		}
		if(event->raw){
			free(event->raw);
		}
		free(event);
	}
	return 0;
}
