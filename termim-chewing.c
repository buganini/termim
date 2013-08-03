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
#ifdef __linux
#define __dead2 __attribute__ ((noreturn))
#include <pty.h>
#include <time.h>
#include <utmp.h>
#else
#include <libutil.h>
#endif

#include <chewing/chewing.h>
#include "keymap.h"
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
	char *s;
	unsigned char ibuf[BUFSIZ];
	char buf[128];
	char escape_buf[128];
	int escape_i=0;
	int escape=0;
	int i;
	int skip;
	int n;
	fd_set rfd;
	char chbuf[100];

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

	FD_ZERO(&rfd);
	while(1){
		FD_SET(STDIN_FILENO, &rfd);
		n = select(STDIN_FILENO+1, &rfd, 0, 0, NULL);
		if (n < 0 && errno != EINTR)
			break;
		if (n > 0 && FD_ISSET(STDIN_FILENO, &rfd)){
			n=read(STDIN_FILENO, ibuf, BUFSIZ);
			skip=0;
			if(n<=0){
			}else{
				if(n==1){
					switch((unsigned char)*ibuf){
						case CTRL_SPACE:
							chewing_set_ChiEngMode(ctx, chewing_get_ChiEngMode(ctx)?0:1);
							s=chewing_buffer_String(ctx);
							write(out, buf, sprintf(buf, "%s", s));
							chewing_handle_Esc(ctx);
							skip=1;
							break;
						case SHIFT_SPACE:
							chewing_set_ShapeMode(ctx, chewing_get_ShapeMode(ctx)?0:1);
							skip=1;
							break;
						case CTRL_SHIFT:
							s=chewing_buffer_String(ctx);
							write(out, buf, sprintf(buf, "%s", s));
							chewing_handle_Esc(ctx);
							execvp(eargv[0], eargv);
							break;
					}
					if(skip){
						if(chewing_commit_Check(ctx)){
							s=chewing_commit_String(ctx);
							write(out, s, strlen(s));
							free(s);
							chewing_handle_Esc(ctx);
						}
						draw();
						continue;
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
							switch(escape_buf[i]){
								case '1':
									chewing_set_ChiEngMode(ctx, chewing_get_ChiEngMode(ctx)?0:1);
									s=chewing_buffer_String(ctx);
									write(out, buf, sprintf(buf, "%s", s));
									chewing_handle_Esc(ctx);
									skip=1;
									break;
								case '2':
									chewing_set_ShapeMode(ctx, chewing_get_ShapeMode(ctx)?0:1);
									skip=1;
									break;
								case '3':
									s=chewing_buffer_String(ctx);
									write(out, buf, sprintf(buf, "%s", s));
									chewing_handle_Esc(ctx);
									execvp(eargv[0], eargv);
									break;
								default:
									write(out, escape_buf, escape_i);
							}
						}
						if((ibuf[i]>='a' && ibuf[i]<='z') || (ibuf[i]>='A' && ibuf[i]<='N') || (ibuf[i]>='P' && ibuf[i]<='Z') || ibuf[i]=='~'){
							escape=0;
							switch(escape_buf[1]){
								case '[':
									switch(ibuf[i]){
										case 'A':
											if(escape_i==3 && chewing_is_entering(ctx)){
												chewing_handle_Up(ctx);
											}else{
												write(out, escape_buf, escape_i);
											}
											break;
										case 'B':
											if(escape_i==3 && chewing_is_entering(ctx)){
												chewing_handle_Down(ctx);
											}else{
												write(out, escape_buf, escape_i);
											}
											break;
										case 'C':
											if(escape_i==3 && chewing_is_entering(ctx)){
												chewing_handle_Right(ctx);
											}else{
												write(out, escape_buf, escape_i);
											}
											break;
										case 'D':
											if(escape_i==3 && chewing_is_entering(ctx)){
												chewing_handle_Left(ctx);
											}else{
												write(out, escape_buf, escape_i);
											}
											break;
										default:
											write(out, escape_buf, escape_i);
											break;
									}
									break;
								default:
									write(out, escape_buf, escape_i);
									break;
							}
						}
					}else{
						switch(ibuf[i]){
							case ENTER:
								if(chewing_is_entering(ctx))
									chewing_handle_Enter(ctx);
								else
									write(out, buf, sprintf(buf, "\r"));
								break;
							case BACKSPACE:
								if(chewing_is_entering(ctx))
									chewing_handle_Backspace(ctx);
								else
									write(out, buf, sprintf(buf, "%c", BACKSPACE));
								break;
							case SPACE:
								if(chewing_is_entering(ctx))
									chewing_handle_Space(ctx);
								else
									write(out, buf, sprintf(buf, "%c", SPACE));
								break;
							default:
								if((ibuf[i] & 0xFF00)==0 && isprint(ibuf[i]))
									chewing_handle_Default(ctx, ibuf[i]);
								else{
									write(out, ibuf+i, 1);
								}
						}
					}
					if(chewing_commit_Check(ctx)){
						s=chewing_commit_String(ctx);
						write(out, s, strlen(s));
						free(s);
						chewing_handle_Esc(ctx);
					}

				}
			}
		}
		draw();
	}

	chewing_delete(ctx);
	chewing_Terminate();

	return 0;
}
