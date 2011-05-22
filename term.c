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
#include "term.h"
#include "common.h"

struct term * term_create(){
	struct term *ret=malloc(sizeof(struct term));
	ret->cur_row=1;
	ret->cur_col=1;
	ret->escape=0;
	ret->display=strdup("\033[m");
	ret->display_len=3;
	return ret;
}

void term_destroy(struct term *term){
	char buf[128];
	write(term->out, buf, sprintf(buf, "\033[r\033[m\033[2J\033[H"));
	free(term->display);
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

void term_put_cursor(struct term *term){
	char buf[128];
	write(term->out, buf, sprintf(buf, "\033[%d;%dH", term->cur_row+term->off_row, term->cur_col+term->off_col));
}

#define POST_WRITE() \
if(r<0){ \
	return r; \
} \
ret+=r;


ssize_t term_write(struct term *term, const char *ibuf, size_t len){
	int i, j, r=0, ret=0;
	int argi;
	char **argv;
	char buf[128];
	int ia[8]={0};

	write(term->out, buf, sprintf(buf, "\033[%d;%dr", term->scr_beg+term->off_row, term->scr_end+term->off_row));
	term_put_cursor(term);
	write(term->out, term->display, term->display_len);
	for(j=i=0;i<len;++i){
		switch(ibuf[i]){
			case '\x1b':
				term->i=0;
				term->escape=1;
				r=write(term->out, ibuf+j, i-j);
				//XXX Unicode
				term->cur_col+=r;
				if(term->cur_col > term->siz_col){
					term->cur_row += term->cur_col / term->siz_col;
					term->cur_col /= term->siz_col;
				}
				break;
			case '\n':
				write(term->out, ibuf+j, i-j+1);
				j=i+1;
				term->cur_row+=1;
				if(term->cur_row > term->siz_row)
					term->cur_row=term->siz_row;
				term->cur_col=1+term->off_col;
				term_put_cursor(term);
				break;
		}
		if(term->escape){
			term->buf[term->i]=ibuf[i];
			term->i+=1;
			if((ibuf[i]>='a' && ibuf[i]<='z') || (ibuf[i]>='A' && ibuf[i]<='Z')){
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
								for(argi=0;argv[argi]!=NULL && argi<8;++argi){
									ia[argi]=strtol(argv[argi], NULL, 10);
								}
								term->cur_row=ia[0];
								term->cur_col=ia[1];
								ia[0]+=term->off_row;
								ia[1]+=term->off_col;
								r=write(term->out, buf, sprintf(buf, "\033[%d;%dH", ia[0], ia[1]));
								POST_WRITE();
								free(argv[0]);
								break;
							case 'm':
								term->display=malloc(term->i);
								memcpy(term->display, term->buf, term->i);
								term->display_len=term->i;
								r=write(term->out, term->buf, term->i);
								POST_WRITE();
								break;
							case 'r':
								ia[0]=1;
								ia[1]=term->siz_row;
								term->buf[term->i-1]=0;
								argv=parse_arg(&term->buf[2]);
								for(argi=0;argv[argi]!=NULL && argi<8;++argi){
									ia[argi]=strtol(argv[argi], NULL, 10);
								}
								ia[0]+=term->off_row;
								ia[1]+=term->off_row;
								r=write(term->out, buf, sprintf(buf, "\033[%d;%dr", ia[0], ia[1]));
								POST_WRITE();
								free(argv[0]);
								break;
							default:
								r=write(term->out, term->buf, term->i);
								POST_WRITE();
								break;
						}
						break;
					default:
						r=write(term->out, term->buf, term->i);
						POST_WRITE();
						break;
				}
			}
		}
	}
	if(i-j>0){
		r=write(term->out, ibuf+j, i-j);
		//XXX Unicode
		term->cur_col+=r;
		if(term->cur_col > term->siz_col){
			term->cur_row += term->cur_col / term->siz_col;
			term->cur_col /= term->siz_col;
		}
		POST_WRITE();
	}

	return ret;
}
