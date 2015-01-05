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
#define __dead2 __attribute__ ((noreturn))
#include <pty.h>
#include <time.h>
#include <utmp.h>
#else
#include <libutil.h>
#endif

#include <chewing/chewing.h>
#include "termim.h"
#include "utf8.h"

ChewingContext *ctx;
int out;

static int selKey_define[ 11 ] = {'1','2','3','4','5','6','7','8','9','0',0};

struct winsize win;

void
winch(int sig){
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
}

int chewing_is_entering(ChewingContext *ctx){
	int nul;
	char *a=chewing_buffer_String(ctx);
	char *b=chewing_zuin_String(ctx, &nul);
	return *a!=0 || *b!=0;
}

void setColor(i){
	if(i&1)
		printf("\033[2m");
	else
		printf("\033[0;44m\033[?25l");
}

void draw(){
	const char *s;
	int i;
	int j;
	int last;
	int nul;
	int n=0;
	int w;
	IntervalType itv;
	char tbuf[512];
	printf("\033[H\033[?25l");
	char *ChiEng[2]={"英數", "注音"};
	char *Shape[2]={"半形", "全形"};

	//language bar
	printf("[%s][%s] ", ChiEng[chewing_get_ChiEngMode(ctx)?1:0], Shape[chewing_get_ShapeMode(ctx)?1:0]);

	//edit buffer
	n=chewing_cursor_Current(ctx);

	s=chewing_buffer_String(ctx);
	nul=INT_MAX;
	chewing_interval_Enumerate(ctx);

	last=0;
	i=0;
	j=0;
	while(chewing_interval_hasNext(ctx)){
		chewing_interval_Get(ctx, &itv);

		setColor(j);
		for(i=0;itv.from>last;last+=1,++i){
			if(last==n)
				printf("\033[1m");
			uputchar(unext(&s,&nul));
			setColor(j);
		}
		if(i)
			j+=1;
		last=itv.to;

		setColor(j);
		for(i=itv.from;i<itv.to;++i){
			if(i==n)
				printf("\033[1m");
			uputchar(unext(&s,&nul));
		}
		setColor(j);
		j+=1;
	}
	setColor(j);
	while((s=unext(&s,&nul))!=NULL){
		if(i==n)
			printf("\033[1m");
		uputchar(unext(&s,&nul));
		setColor(j);
		i+=1;
	}

	printf("\033[0;44;4m");
	s=chewing_zuin_String(ctx, &nul);
	printf("%s", s);
	printf("\033[0;44m\033[?25l");

	printf("\033[K\n");

	//candidates
	if(chewing_is_entering(ctx)){
		chewing_cand_Enumerate(ctx);
		n=1;
		i=0;
		while(chewing_cand_hasNext(ctx)){
			s=chewing_cand_String(ctx);
			sprintf(tbuf, "%c.%s ", selKey_define[i], s);
			w=ustrwidth(tbuf, INT_MAX);
			if(n+w>=win.ws_col){
				n=1;
				printf("\n");
			}
			printf("%s", tbuf);
			n+=w;
			if(i==10){
				break;
			}
			++i;
		}
	}
	printf("\033[K");
	fflush(stdout);
}

int main(int argc, char *argv[]){
	char *eargv[]={"termim-next", NULL};
	char chbuf[100];
	char buf[128];
	char *s;

	if((s=getenv("TERMIM"))!=NULL)
		out=strtol(s, NULL, 10);
	else
		out=STDOUT_FILENO;

	signal(SIGWINCH, &winch);
	winch(0);

	chewing_Init( PREFIX "/share/libchewing3/chewing", chbuf);
	ctx = chewing_new();
	chewing_set_ChiEngMode(ctx, 0);
	chewing_set_KBType(ctx, chewing_KBStr2Num("KB_DEFAULT"));
	chewing_set_candPerPage(ctx, 10);
	chewing_set_addPhraseDirection(ctx, 1);
	chewing_set_escCleanAllBuf(ctx, 1);
	chewing_set_maxChiSymbolLen(ctx, 64);
	chewing_set_selKey(ctx, selKey_define, 10);
	chewing_set_spaceAsSelection(ctx, 1);

	printf("\033[H\033[44m\n\n");
	draw();

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
					case TERMIM_KEY_1:
						chewing_set_ChiEngMode(ctx, chewing_get_ChiEngMode(ctx)?0:1);
						s=chewing_buffer_String(ctx);
						write(out, buf, sprintf(buf, "%s", s));
						chewing_handle_Esc(ctx);
						break;
					case TERMIM_KEY_2:
						chewing_set_ShapeMode(ctx, chewing_get_ShapeMode(ctx)?0:1);
						break;
					case TERMIM_KEY_3:
						s=chewing_buffer_String(ctx);
						write(out, buf, sprintf(buf, "%s", s));
						chewing_handle_Esc(ctx);
						execvp(eargv[0], eargv);
						break;
					default:
						if(event->raw)
							write(out, event->raw, event->raw_length);
						break;
				}
			}else if(event->modifiers == 0){
				switch(event->code){
					case TERMIM_KEY_UP:
						if(chewing_is_entering(ctx)){
							chewing_handle_Up(ctx);
						}else{
							if(event->raw)
								write(out, event->raw, event->raw_length);
						}
						break;
					case TERMIM_KEY_DOWN:
						if(chewing_is_entering(ctx)){
							chewing_handle_Down(ctx);
						}else{
							if(event->raw)
								write(out, event->raw, event->raw_length);
						}
						break;
					case TERMIM_KEY_RIGHT:
						if(chewing_is_entering(ctx)){
							chewing_handle_Right(ctx);
						}else{
							if(event->raw)
								write(out, event->raw, event->raw_length);
						}
						break;
					case TERMIM_KEY_LEFT:
						if(chewing_is_entering(ctx)){
							chewing_handle_Left(ctx);
						}else{
							if(event->raw)
								write(out, event->raw, event->raw_length);
						}
						break;
					case TERMIM_KEY_ENTER:
						if(chewing_is_entering(ctx)){
							chewing_handle_Enter(ctx);
						}else{
							if(event->raw)
								write(out, event->raw, event->raw_length);
						}
						break;
					case TERMIM_KEY_BACKSPACE:
						if(chewing_is_entering(ctx)){
							chewing_handle_Backspace(ctx);
						}else{
							if(event->raw)
								write(out, event->raw, event->raw_length);
						}
						break;
					case TERMIM_KEY_SPACE:
						if(chewing_is_entering(ctx)){
							chewing_handle_Space(ctx);
						}else{
							if(event->raw)
								write(out, event->raw, event->raw_length);
						}
						break;
					default:
						if(isprint(event->code)){
							chewing_handle_Default(ctx, event->code);
						}else{
							if(event->raw)
								write(out, event->raw, event->raw_length);
						}
				}
			}
		}
		if(event->raw){
			free(event->raw);
		}
		free(event);
		if(chewing_commit_Check(ctx)){
			s=chewing_commit_String(ctx);
			write(out, s, strlen(s));
			free(s);
			chewing_handle_Esc(ctx);
		}
		draw();
	}

	chewing_delete(ctx);
	chewing_Terminate();

	return 0;
}
