/*
 * Copyright (c) 2011 Kuan-Chung Chiu <buganini@gmail.com>
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
