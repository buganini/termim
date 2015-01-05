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

#include <kmfl/kmfl.h>
#include <kmfl/libkmfl.h>

#include <termim.h>
#include "utf8.h"

char desc[1024];
int out;
struct winsize win;

void
winch(int sig){
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
}

void output_string(void *connection, char *p){
	write(out, p, strlen(p));
};
void output_char(void *connection, BYTE q){
	write(out, &q, 1);
};
void output_beep(void *connection){};
void forward_keyevent(void *connection, UINT key, UINT state){

};
void erase_char(void *connection){
	write(out, "\b", 1);
}

int main(int argc, char *argv[]){
	char *eargv[]={"termim-next", NULL};
	char *s;
	int kbd;
	KMSI *ic;

	if((s=getenv("TERMIM"))!=NULL)
		out=strtol(s, NULL, 10);
	else
		out=STDOUT_FILENO;

	signal(SIGWINCH, &winch);
	winch(0);

	if(argc!=2){
		exit(1);
	}
	char kbd_file[256];
	sprintf(kbd_file, "%s/share/kmfl/%s.kmn", PREFIX, argv[1]);
	kbd=kmfl_load_keyboard(kbd_file);
	ic=kmfl_make_keyboard_instance(NULL);
	kmfl_attach_keyboard(ic, kbd);
	kmfl_get_header(ic, SS_NAME, desc, sizeof(desc)-1);

	printf("\033[H\033[44m\033[?25l");
	printf("\033[K");
	printf("KMFL: %s\n", desc);
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
						kmfl_delete_keyboard_instance(ic);
						kmfl_unload_keyboard(kbd);
						execvp(eargv[0], eargv);
						break;
					default:
						if(event->raw)
							write(out, event->raw, event->raw_length);
						break;
				}
			}else if(event->modifiers == 0){
				if(isprint(event->code)){
					kmfl_interpret(ic, event->code, 0);
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
