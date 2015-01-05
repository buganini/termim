/*
 * Copyright (c) 2015 Kuan-Chung Chiu <buganini@gmail.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <errno.h>
#include "termim.h"

struct termim_event * termim_read_input(){
	static char escape_buf[128];
	static int escape_i=0;
	static int escape=0;
	static int i = 0;
	static int n = 0;
	fd_set fds;
	unsigned char buf[BUFSIZ];

	struct termim_event  *event = malloc(sizeof(struct termim_event));
	event->type = 0;
	event->code = 0;
	event->modifiers = 0;
	event->raw = NULL;
	event->raw_length = 0;

	while(1){
		for(;i<n;++i){
			if(buf[i]==0x1b){
				if(escape){
					escape_buf[escape_i] = 0;

					event->type = TERMIM_EVENT_RAW;
					event->raw = strdup(escape_buf);
					event->raw_length = escape_i;

					escape_i=1;
					escape_buf[0]=buf[i];

					i+=1;
					return event;
				}
				escape_i=1;
				escape_buf[0]=buf[i];

				escape=1;
				continue;
			}
			if(escape){
				escape_buf[escape_i]=buf[i];
				escape_i+=1;
				escape_buf[escape_i] = 0;
				if(escape_i==2 && escape_buf[1]!='[' && escape_buf[1]>='0' && escape_buf[1]<='9'){
					escape=0;
					switch(escape_buf[1]){
						case '1':
							event->type = TERMIM_EVENT_KEY;
							event->code = TERMIM_KEY_1;
							event->modifiers = TERMIM_MOD_ALT;
							i+=1;
							return event;
						case '2':
							event->type = TERMIM_EVENT_KEY;
							event->code = TERMIM_KEY_2;
							event->modifiers = TERMIM_MOD_ALT;
							i+=1;
							return event;
						case '3':
							event->type = TERMIM_EVENT_KEY;
							event->code = TERMIM_KEY_3;
							event->modifiers = TERMIM_MOD_ALT;
							i+=1;
							return event;
						default:
							event->type = TERMIM_EVENT_RAW;
							event->raw = strdup(escape_buf);
							event->raw_length = escape_i;

							escape_i = 0;
							i+=1;
							return event;
					}
				}
				if((buf[i]>='a' && buf[i]<='z') || (buf[i]>='A' && buf[i]<='N') || (buf[i]>='P' && buf[i]<='Z') || buf[i]=='~'){
					escape=0;
					switch(escape_buf[1]){
						case '[':
							switch(escape_buf[2]){
								case 'A':
									event->type = TERMIM_EVENT_KEY;
									event->code = TERMIM_KEY_UP;
									event->raw = strdup(escape_buf);
									event->raw_length = escape_i;

									escape_i = 0;
									i+=1;
									return event;
								case 'B':
									event->type = TERMIM_EVENT_KEY;
									event->code = TERMIM_KEY_DOWN;
									event->raw = strdup(escape_buf);
									event->raw_length = escape_i;

									escape_i = 0;
									i+=1;
									return event;
								case 'C':
									event->type = TERMIM_EVENT_KEY;
									event->code = TERMIM_KEY_RIGHT;
									event->raw = strdup(escape_buf);
									event->raw_length = escape_i;

									escape_i = 0;
									i+=1;
									return event;
								case 'D':
									event->type = TERMIM_EVENT_KEY;
									event->code = TERMIM_KEY_LEFT;
									event->raw = strdup(escape_buf);
									event->raw_length = escape_i;

									escape_i = 0;
									i+=1;
									return event;
								default:
									event->type = TERMIM_EVENT_RAW;
									event->raw = strdup(escape_buf);
									event->raw_length = escape_i;

									escape_i = 0;
									i+=1;
									return event;
							}
						case 'O':
							switch(escape_buf[2]){
								case 'A':
									event->type = TERMIM_EVENT_KEY;
									event->code = TERMIM_KEY_UP;
									event->raw = strdup(escape_buf);
									event->raw_length = escape_i;

									escape_i = 0;
									i+=1;
									return event;
								case 'B':
									event->type = TERMIM_EVENT_KEY;
									event->code = TERMIM_KEY_DOWN;
									event->raw = strdup(escape_buf);
									event->raw_length = escape_i;

									escape_i = 0;
									i+=1;
									return event;
								case 'C':
									event->type = TERMIM_EVENT_KEY;
									event->code = TERMIM_KEY_RIGHT;
									event->raw = strdup(escape_buf);
									event->raw_length = escape_i;

									escape_i = 0;
									i+=1;
									return event;
								case 'D':
									event->type = TERMIM_EVENT_KEY;
									event->code = TERMIM_KEY_LEFT;
									event->raw = strdup(escape_buf);
									event->raw_length = escape_i;

									escape_i = 0;
									i+=1;
									return event;
								default:
									event->type = TERMIM_EVENT_RAW;
									event->raw = strdup(escape_buf);
									event->raw_length = escape_i;

									escape_i = 0;
									i+=1;
									return event;
							}
						default:
							event->type = TERMIM_EVENT_RAW;
							event->raw = strdup(escape_buf);
							event->raw_length = escape_i;

							escape_i = 0;
							i+=1;
							return event;
					}
				}else{
					continue;
				}
			}else{
				event->type = TERMIM_EVENT_KEY;
				event->code = buf[i];
				event->raw = malloc(1);
				event->raw[0] = buf[i];
				event->raw_length = 1;
				i+=1;
				return event;
			}
		}
		while(1){
			FD_ZERO(&fds);
			FD_SET(STDIN_FILENO, &fds);
			n = select(STDIN_FILENO+1, &fds, 0, 0, NULL);
			if (n < 0 && errno != EINTR){
				event->type = TERMIM_EVENT_EOF;
				return event;
			}
			if (n > 0 && FD_ISSET(STDIN_FILENO, &fds)){
				n=read(STDIN_FILENO, buf, sizeof(buf));
				i=0;
				break;
			}
		}
	}
}
