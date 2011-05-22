#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>

#include <chewing/chewing.h>
#include "keymap.h"

int out;
int active=0;

static int selKey_define[ 11 ] = {'1','2','3','4','5','6','7','8','9','0',0};

int main(int argc, char *argv[]){
	ChewingContext *ctx;
	char *s;
	unsigned char ibuf[BUFSIZ];
	char buf[128];
	unsigned char c;
	int n;
	fd_set rfd;

	if(argc!=2)
		return 1;

	out=strtol(argv[1], NULL, 10);

	chewing_Init( "/usr/local/share/chewing", "/tmp" );
	ctx = chewing_new();
	chewing_set_KBType( ctx, chewing_KBStr2Num( "KB_DEFAULT" ) );
	chewing_set_candPerPage( ctx, 9 );
	chewing_set_maxChiSymbolLen( ctx, 16 );
	chewing_set_addPhraseDirection( ctx, 1 );
	chewing_set_selKey( ctx, selKey_define, 10 );
	chewing_set_spaceAsSelection( ctx, 1 );
	

	FD_ZERO(&rfd);
	while(1){
		FD_SET(STDIN_FILENO, &rfd);
		n = select(STDIN_FILENO+1, &rfd, 0, 0, NULL);
		if (n < 0 && errno != EINTR)
			break;
		if (n > 0 && FD_ISSET(STDIN_FILENO, &rfd)){
			n=read(STDIN_FILENO, ibuf, BUFSIZ);
			if(n<=0){
			}else if(n==1){
				c=*ibuf;
				if(c==CTRL_SPACE || c=='~'){
					if(active)
						active=0;
					else
						active=1;
					continue;
				}
				if(active){
					switch(c){
						case SPACE:
							printf("SPACE\n");
							chewing_handle_Space(ctx);
							break;
						case ENTER:
							printf("ENTER\n");
							chewing_handle_Enter(ctx);
							break;
						case BACKSPACE:
							printf("BACKSPACE\n");
							chewing_handle_Backspace(ctx);
							break;
						case UP:
							printf("UP\n");
							chewing_handle_Up(ctx);
							break;
						case DOWN:
							printf("DOWN\n");
							chewing_handle_Down(ctx);
							break;
						case LEFT:
							printf("LEFT\n");
							chewing_handle_Left(ctx);
							break;
						case RIGHT:
							printf("RIGHT\n");
							chewing_handle_Right(ctx);
							break;
						default:
							chewing_handle_Default(ctx, c);
					}
				}else{
					switch(c){
						case UP:
							write(out, buf, sprintf(buf, "\033[A"));
							break;
						case DOWN:
							write(out, buf, sprintf(buf, "\033[B"));
							break;
						case LEFT:
							write(out, buf, sprintf(buf, "\033[D"));
							break;
						case RIGHT:
							write(out, buf, sprintf(buf, "\033[C"));
							break;
						default:
							write(out, ibuf, 1);
					}
				}
				if(chewing_commit_Check(ctx)){
					s=chewing_commit_String(ctx);
					write(out, s, strlen(s));
					free(s);
				}
			}else{
				write(out, ibuf, n);
			}
		}
	}

	chewing_delete(ctx);
	chewing_Terminate();

	return 0;
}
