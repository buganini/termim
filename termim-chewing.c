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

	printf("\033[4m");
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
	char *s;
	unsigned char ibuf[BUFSIZ];
	char buf[128];
	unsigned char c;
	int n;
	fd_set rfd;

	if(argc!=2)
		return 1;

	out=strtol(argv[1], NULL, 10);

	signal(SIGWINCH, &winch);
	winch(0);

	chewing_Init( "/usr/local/share/chewing", "/tmp");
	ctx = chewing_new();
	chewing_set_ChiEngMode(ctx, 0);
	chewing_set_KBType(ctx, chewing_KBStr2Num( "KB_DEFAULT"));
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
			if(n<=0){
			}else if(n==1){
				c=*ibuf;
				switch(c){
					case CTRL_SPACE:
					case '`':
						chewing_set_ChiEngMode(ctx, chewing_get_ChiEngMode(ctx)?0:1);
						break;
					case SHIFT_SPACE:
					case '~':
						chewing_set_ShapeMode(ctx, chewing_get_ShapeMode(ctx)?0:1);
						break;
					case SPACE:
						chewing_handle_Space(ctx);
						break;
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
					case UP:
						if(chewing_is_entering(ctx))
							chewing_handle_Up(ctx);
						else
							write(out, buf, sprintf(buf, "%c", UP));
						break;
					case DOWN:
						if(chewing_is_entering(ctx))
							chewing_handle_Down(ctx);
						else
							write(out, buf, sprintf(buf, "%c", DOWN));
						break;
					case LEFT:
						if(chewing_is_entering(ctx))
							chewing_handle_Left(ctx);
						else
							write(out, buf, sprintf(buf, "%c", LEFT));
						break;
					case RIGHT:
						if(chewing_is_entering(ctx))
							chewing_handle_Right(ctx);
						else
							write(out, buf, sprintf(buf, "%c", RIGHT));
						break;
					default:
						if((c & 0xFF00)==0 && isprint(c))
							chewing_handle_Default(ctx, c);
						else
							write(out, ibuf, 1);
				}
				if(chewing_commit_Check(ctx)){
					s=chewing_commit_String(ctx);
					write(out, s, strlen(s));
					free(s);
					chewing_handle_Esc(ctx);
				}
			}else{
				write(out, ibuf, n);
			}
		}
		draw();
	}

	chewing_delete(ctx);
	chewing_Terminate();

	return 0;
}
