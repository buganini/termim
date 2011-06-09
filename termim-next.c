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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>

#define ARRAY_SIZE 32

int main(int argc, char **argv){
	char buf[64];
	char *imlist[ARRAY_SIZE];
	char *eargv[ARRAY_SIZE];
	char *termim_i=getenv("TERMIM_I");
	char *termim_im=getenv("TERMIM_IM");
	char *t, *im, *arg;
	int i;
	int n=0;

	if(termim_im==NULL){
		return 1;
	}

	termim_im=strdup(termim_im);
	t=termim_im;
	while((im=strsep(&t, ":"))!=NULL){
		if(n==ARRAY_SIZE)
			break;
		imlist[n]=im;
		n+=1;
	}

	if(n==0)
		return 1;

	if(termim_i==NULL){
		i=0;
	}else{
		i=strtol(termim_i, NULL, 10);
		i=(i+1)%n;
	}

	sprintf(buf, "%d", i);
	setenv("TERMIM_I", buf, 1);

	im=strdup(imlist[i]);
	i=0;
	t=im;
	while((arg=strsep(&t, " "))!=NULL){
		if(i==ARRAY_SIZE)
			break;
		eargv[i]=arg;
		i+=1;
	}
	eargv[i]=NULL;

	execvp(eargv[0], eargv);

	return errno;
}
