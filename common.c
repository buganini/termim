#include <stdlib.h>
#include <string.h>
#include "common.h"

char ** parse_arg(char *s){
	int size=0;
	int argi=0;
	char *str=strdup(s);
	char *p=str;
	char *t;
	char **argv=NULL;

	t=NULL;
	do{
		if(argi>=size){
			size+=8;
			argv=realloc(argv, sizeof(char *)*size);
		}
		argv[argi]=t;
		argi+=1;
	}while((t=strsep(&p, ";")) != NULL);
	argv[argi]=NULL;
	return argv;
}
