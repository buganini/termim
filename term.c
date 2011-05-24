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

#include <sys/types.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "term.h"
#include "utf8.h"

struct term * term_create(){
	struct term *ret=malloc(sizeof(struct term));
	ret->cur_row=1;
	ret->cur_col=1;
	ret->cur_visible=1;
	ret->cur_blink=0;
	ret->escape=0;
	return ret;
}

void term_destroy(struct term *term){
	char buf[128];
	write(term->out, buf, sprintf(buf, "\033[r\033[m\033[2J\033[H\033[?12l\033[?25h"));
	free(term);
	
}

void term_set_size(struct term *term, int siz_row, int siz_col){
	term->siz_row=siz_row;
	term->siz_col=siz_col;
	term->scr_beg=1;
	term->scr_end=siz_row;
}

void term_set_offset(struct term *term, int off_row, int off_col){
	term->cur_row-=term->off_row;
	term->cur_row+=off_row;
	term->cur_col-=term->off_col;
	term->cur_col+=off_col;

	term->off_row=off_row;
	term->off_col=off_col;
}

void term_assoc_output(struct term *term, int out){
	term->out=out;
}

ssize_t term_read(struct term *term, void *buf, size_t len){
	return read(term->out, buf, len);
}

int term_put_cursor(struct term *term){
	char buf[128];
	return write(term->out, buf, sprintf(buf, "\033[%d;%dH", term->cur_row+term->off_row, term->cur_col+term->off_col));
}

#define WRITE(X,Y,Z) do{ \
r=write((X),(Y),(Z)); \
if(r<0){ \
	return r; \
} \
ret+=r; \
}while(0);


ssize_t term_write(struct term *term, const char *ibuf, size_t len){
	int i, j, t, r=0, ret=0;
	int argi;
	char **argv;
	char buf[128];
	int ia[8]={0};
	char hl[]={'l','h'};

	WRITE(term->out, buf, sprintf(buf, "\033[%d;%dr", term->scr_beg+term->off_row, term->scr_end+term->off_row));

	term_put_cursor(term);

	WRITE(term->out, buf, sprintf(buf, "\033[m"));
	if(term->bold)
		WRITE(term->out, buf, sprintf(buf, "\033[1m"));
	if(term->underline)
		WRITE(term->out, buf, sprintf(buf, "\033[4m"));
	if(term->blink)
		WRITE(term->out, buf, sprintf(buf, "\033[5m"));
	if(term->reverse)
		WRITE(term->out, buf, sprintf(buf, "\033[7m"));
	if(term->invisible)
		WRITE(term->out, buf, sprintf(buf, "\033[8m"));
	WRITE(term->out, buf, sprintf(buf, "\033[?12%c", hl[term->cur_blink?1:0]));
	WRITE(term->out, buf, sprintf(buf, "\033[?25%c", hl[term->cur_visible?1:0]));
	WRITE(1, buf, sprintf(buf, "\033[%d;%dm", term->fg, term->bg));

	for(j=i=0;i<len;++i){
		switch(ibuf[i]){
			case '\x1b':
				term->i=0;
				term->escape=1;
				WRITE(term->out, ibuf+j, i-j);

				term->cur_col+=ustrwidth(ibuf+j, i-j);
				if(term->cur_col > term->siz_col){
					term->cur_row += term->cur_col / term->siz_col;
					term->cur_col /= term->siz_col;
				}
				term_put_cursor(term);

				break;
			case 0x8:
				WRITE(term->out, ibuf+j, i-j+1);
				j=i+1;

				term->cur_col-=1;
				if(term->cur_col<1)
					term->cur_col=1;
				term_put_cursor(term);
				break;
			case '\n':
				WRITE(term->out, ibuf+j, i-j+1 /* include \n */);
				j=i+1;

				term->cur_row+=1;
				if(term->cur_row > term->siz_row)
					term->cur_row=term->siz_row;
				term->cur_col=1;
				term_put_cursor(term);

				break;
		}
		if(term->escape){
			term->buf[term->i]=ibuf[i];
			term->i+=1;
			if(term->i==2 && term->buf[1]!='['){
				WRITE(term->out, term->buf, 2);
				term->escape=0;
				j=i+1;
				continue;
			}
			if((ibuf[i]>='a' && ibuf[i]<='z') || (ibuf[i]>='A' && ibuf[i]<='Z') || ibuf[i]=='~'){
				term->escape=0;
				j=i+1;
				switch(term->buf[1]){
					case '[':
						switch(ibuf[i]){
							case 'f':
							case 'H':
								term->buf[term->i-1]=0;
								argv=parse_arg(&term->buf[2]);
								ia[0]=1;
								ia[1]=1;
								for(argi=0;argv[argi]!=NULL && argi<2;++argi){
									ia[argi]=strtol(argv[argi], NULL, 10);
								}
								free_arg(argv);
								term->cur_row=ia[0];
								term->cur_col=ia[1];
								term_put_cursor(term);
								break;
							case 'A':
								term->buf[term->i-1]=0;
								argv=parse_arg(&term->buf[2]);
								ia[0]=1;
								for(argi=0;argv[argi]!=NULL && argi<1;++argi){
									ia[argi]=strtol(argv[argi], NULL, 10);
								}
								free_arg(argv);
								term->cur_row-=ia[0];
								if(term->cur_row<1)
									term->cur_row=1;
								term->buf[term->i-1]='A';
								WRITE(term->out, term->buf, term->i);
								break;
							case 'B':
								term->buf[term->i-1]=0;
								argv=parse_arg(&term->buf[2]);
								ia[0]=1;
								for(argi=0;argv[argi]!=NULL && argi<1;++argi){
									ia[argi]=strtol(argv[argi], NULL, 10);
								}
								free_arg(argv);
								term->cur_row+=ia[0];
								if(term->cur_row>term->cur_row)
									term->cur_row=term->cur_row;
								term->buf[term->i-1]='B';
								WRITE(term->out, term->buf, term->i);
								break;
							case 'C':
								term->buf[term->i-1]=0;
								argv=parse_arg(&term->buf[2]);
								ia[0]=1;
								for(argi=0;argv[argi]!=NULL && argi<1;++argi){
									ia[argi]=strtol(argv[argi], NULL, 10);
								}
								free_arg(argv);
								term->cur_col+=ia[0];
								if(term->cur_col>term->cur_col)
									term->cur_col=term->cur_col;
								term->buf[term->i-1]='C';
								WRITE(term->out, term->buf, term->i);
								break;
							case 'D':
								term->buf[term->i-1]=0;
								argv=parse_arg(&term->buf[2]);
								ia[0]=1;
								for(argi=0;argv[argi]!=NULL && argi<1;++argi){
									ia[argi]=strtol(argv[argi], NULL, 10);
								}
								free_arg(argv);
								term->cur_col-=ia[0];
								if(term->cur_col<1)
									term->cur_col=1;
								term->buf[term->i-1]='D';
								WRITE(term->out, term->buf, term->i);
								break;
							case 'h':
								switch(term->buf[2]){
									case '?':
										term->buf[term->i-1]=0;
										argv=parse_arg(&term->buf[3]);
										term->buf[term->i-1]='h';
										if(argv[0]){
											t=strtol(argv[0], NULL, 10);
											switch(t){
												case 12:
													term->cur_blink=1;
													break;
												case 25:
													term->cur_visible=1;
													break;
											}
										}
										free_arg(argv);
										break;
								}
								WRITE(term->out, term->buf, term->i);
								break;
							case 'l':
								switch(term->buf[2]){
									case '?':
										term->buf[term->i-1]=0;
										argv=parse_arg(&term->buf[3]);
										term->buf[term->i-1]='l';
										if(argv[0]){
											t=strtol(argv[0], NULL, 10);
											switch(t){
												case 12:
													term->cur_visible=0;
													break;
												case 25:
													term->cur_blink=0;
													break;
											}
										}
										free_arg(argv);
										break;
								}
								WRITE(term->out, term->buf, term->i);
								break;
							case 'J':
								term->buf[term->i-1]=0;
								argv=parse_arg(&term->buf[2]);
								t=0;
								if(argv[0]!=NULL)
									t=strtol(argv[0], NULL, 10);
								free_arg(argv);
								switch(t){
									case 0:
										for(t=term->cur_row;t<=term->siz_row;++t)
											WRITE(term->out, buf, sprintf(buf, "\033[%d;%dH\033[2K", t, 1));
										break;
									case 1:
										for(t=term->cur_row;t>=1;--t)
											WRITE(term->out, buf, sprintf(buf, "\033[%d;%dH\033[2K", t, 1));
										break;										
									case 2:
										for(t=1;t<=term->siz_row;++t)
											WRITE(term->out, buf, sprintf(buf, "\033[%d;%dH\033[2K", t, 1));
										break;
								}
								term_put_cursor(term);
								break;
							case 'm':
								term->buf[term->i-1]=0;
								argv=parse_arg(&term->buf[2]);
								if(argv[0]==NULL){
									term->bold=0;
									term->underline=0;
									term->blink=0;
									term->reverse=0;
									term->invisible=0;
									term->fg=0;
									term->bg=0;
								}
								for(argi=0;argv[argi]!=NULL;++argi){
									t=strtol(argv[argi], NULL, 10);
									switch(t){
										case 0:
											term->bold=0;
											term->underline=0;
											term->blink=0;
											term->reverse=0;
											term->invisible=0;
											term->fg=0;
											term->bg=0;
											break;
										case 1:
											term->bold=1;
											break;
										case 4:
											term->underline=1;
											break;
										case 5:
											term->blink=1;
											break;
										case 7:
											term->reverse=1;
											break;
										case 8:
											term->invisible=0;
											break;
										case 30:
										case 31:
										case 32:
										case 33:
										case 34:
										case 35:
										case 36:
										case 37:
										case 38:
										case 39:
											term->fg=t;
											break;
										case 40:
										case 41:
										case 42:
										case 43:
										case 44:
										case 45:
										case 46:
										case 47:
										case 48:
										case 49:
											term->bg=t;
											break;
									}
								}
								free_arg(argv);
								term->buf[term->i-1]='m';
								WRITE(term->out, term->buf, term->i);
								break;
							case 'r':
								ia[0]=1;
								ia[1]=term->siz_row;
								term->buf[term->i-1]=0;
								argv=parse_arg(&term->buf[2]);
								for(argi=0;argv[argi]!=NULL && argi<2;++argi){
									ia[argi]=strtol(argv[argi], NULL, 10);
								}
								ia[0]+=term->off_row;
								ia[1]+=term->off_row;
								WRITE(term->out, buf, sprintf(buf, "\033[%d;%dr", ia[0], ia[1]));
								free_arg(argv);
								break;
							default:
								WRITE(term->out, term->buf, term->i);
								break;
						}
						break;
					default:
						WRITE(term->out, term->buf, term->i);
						break;
				}
			}
		}
	}
	if(term->escape==0 && i-j>0){
		WRITE(term->out, ibuf+j, i-j);
		term->cur_col+=ustrwidth(ibuf+j, i-j);
		if(term->cur_col > term->siz_col){
			term->cur_row += term->cur_col / term->siz_col;
			term->cur_col /= term->siz_col;
		}
	}
	return ret;
}
