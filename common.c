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
 */

#include <stdlib.h>
#include <string.h>
#include "common.h"

char ** parse_arg(char *s){
	int size=4;
	int argi=0;
	char *str=strdup(s);
	char *t;
	char **argv=malloc(sizeof(char *)*4);

	while((t=strsep(&str, ";")) != NULL){
		if(*t==0)
			continue;
		if(argi>=size){
			size+=2;
			argv=realloc(argv, sizeof(char *)*size);
		}
		argv[argi]=t;
		argi+=1;
	}
	argv[argi]=NULL;
	if(argv[0]==NULL)
		free(str);
	return argv;
}

void free_arg(char **argv){
	if(argv[0])
		free(argv[0]);
	free(argv);
}
