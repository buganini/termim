/*
 * Copyright (c) 2011 buganini@gmail.com . All rights reserved.
 * Copyright (c) 1980, 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
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

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/kbio.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <libutil.h>
#include <paths.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "keymap.h"
#include "term.h"

extern int ambi_width;

struct tty *tty, *tty2;
struct term *term, *term2;
static int master, slave;
static int master2, slave2;
static int child, child2;
static int qflg, ttyflg;
int tube[2];

static struct termios tt;

static void done(int) __dead2;
static void doshell(char **);
static void dodock();
static void fail(void);
static void finish(void);
static void usage(void);
void sigchild(int);
void sigforwarder(int);
void winchforwarder(int);

int
main(int argc, char *argv[])
{
	int cc;
	struct termios rtt;
	struct winsize win;
	int ch, n;
	struct timeval tv, *tvp;
	time_t tvec, start;
	char obuf[BUFSIZ];
	char ibuf[BUFSIZ];
	keymap_t kmap;
	
	fd_set rfd;
	int flushtime = 30;
	int nfds=0;

	while ((ch = getopt(argc, argv, "hnw")) != -1)
		switch(ch) {
		case 'n':
			ambi_width=1;
			break;
		case 'w':
			ambi_width=2;
			break;
		case 'h':
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if(pipe(tube)==-1)
		err(1, "pipe");

	sprintf(obuf, "%d", tube[1]);
	setenv("TERMIM", obuf, 1);

	if ((ttyflg = isatty(STDIN_FILENO)) != 0) {
		if (tcgetattr(STDIN_FILENO, &tt) == -1)
			err(1, "tcgetattr");
		if (ioctl(STDIN_FILENO, TIOCGWINSZ, &win) == -1)
			err(1, "ioctl");
		win.ws_row-=2;
		term=term_create();
		term_assoc_output(term, STDOUT_FILENO);
		term_set_size(term, win.ws_row, win.ws_col);
		term_set_offset(term, 0, 0);
		if (openpty(&master, &slave, NULL, &tt, &win) == -1)
			err(1, "openpty");
		term2=term_create();
		term_assoc_output(term2, STDOUT_FILENO);
		term_set_size(term2, 2, win.ws_col);
		term_set_offset(term2, win.ws_row, 0);
		win.ws_row=2;
		if (openpty(&master2, &slave2, NULL, &tt, &win) == -1)
			err(1, "openpty");
	} else {
		if (openpty(&master, &slave, NULL, NULL, NULL) == -1)
			err(1, "openpty");
		if (openpty(&master2, &slave2, NULL, NULL, NULL) == -1)
			err(1, "openpty");
	}

	if (!qflg) {
		tvec = time(NULL);
	}
	if (ttyflg) {
		rtt = tt;
		cfmakeraw(&rtt);
		rtt.c_lflag &= ~ECHO;
		(void)tcsetattr(STDIN_FILENO, TCSAFLUSH, &rtt);
	}

	tcgetattr(master2, &rtt);
	cfmakeraw(&rtt);
	rtt.c_lflag &= ~ECHO;
	tcsetattr(master2, TCSAFLUSH, &rtt);

	signal(SIGCHLD, &sigchild);

	child = fork();
	if (child < 0) {
		warn("fork");
		done(1);
	}
	if (child == 0)
		doshell(argv);
	close(slave);

	child2 = fork();
	if (child2 < 0) {
		warn("fork");
		done(1);
	}
	if (child2 == 0)
		dodock();
	close(tube[1]);

	fcntl(tube[0], F_SETFL, O_NONBLOCK);

	signal(SIGINT, &sigforwarder);
	signal(SIGQUIT, &sigforwarder);
	signal(SIGPIPE, &sigforwarder);
	signal(SIGINFO, &sigforwarder);
	signal(SIGUSR1, &sigforwarder);
	signal(SIGUSR2, &sigforwarder);
	signal(SIGWINCH, &winchforwarder);

	ioctl(STDIN_FILENO, GIO_KEYMAP, &kmap);
	kmap.key[42].map[2] = CTRL_SHIFT;
	kmap.key[54].map[2] = CTRL_SHIFT;
	kmap.key[57].map[2] = CTRL_SPACE;
	kmap.key[57].map[1] = SHIFT_SPACE;
	ioctl(STDIN_FILENO, PIO_KEYMAP, &kmap);

	if (flushtime > 0)
		tvp = &tv;
	else
		tvp = NULL;

#define RESET "\033[m\033[2J\033[H"
	term_write(term, RESET, sizeof(RESET));
	term_write(term2, RESET, sizeof(RESET));

	start = time(0);
	FD_ZERO(&rfd);
	if(master2 > tube[0]) nfds = tube[0];
	if(master > nfds) nfds = master;
	if(master2 > nfds) nfds = master2;
	if(STDIN_FILENO > nfds) nfds = STDIN_FILENO;
	nfds+=1;
	for (;;) {
		FD_SET(tube[0], &rfd);
		FD_SET(master, &rfd);
		FD_SET(master2, &rfd);
		FD_SET(STDIN_FILENO, &rfd);
		if (flushtime > 0) {
			tv.tv_sec = flushtime;
			tv.tv_usec = 0;
		}
		n = select(nfds, &rfd, 0, 0, tvp);
		if (n < 0 && errno != EINTR)
			break;
		if (n > 0 && FD_ISSET(STDIN_FILENO, &rfd)) {
			cc = read(STDIN_FILENO, ibuf, sizeof (ibuf));
			if (cc < 0)
				break;
			write(master2, ibuf, cc);
		}
		if (n > 0 && FD_ISSET(master, &rfd)) {
			cc = read(master, obuf, sizeof (obuf));
			if (cc <= 0)
				break;
			term_write(term, obuf, cc);
		}
		if (n > 0 && FD_ISSET(master2, &rfd)) {
			cc = read(master2, obuf, sizeof (obuf));
			if (cc <= 0)
				break;
			term_write(term2, obuf, cc);
		}
		if (n > 0 && FD_ISSET(tube[0], &rfd)) {
			cc = read(tube[0], ibuf, sizeof (ibuf));
			if(cc < 0 && errno!=EAGAIN)
				break;
			write(master, ibuf, cc);
		}
		tvec = time(0);
		if (tvec - start >= flushtime) {
			start = tvec;
		}
	}
	finish();
	done(0);
}

void
sigchild(int sig)
{
	done(1);
}

void
sigforwarder(int sig)
{
	kill(child, sig);
}

void
winchforwarder(int sig)
{
	struct winsize win;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &win);

	win.ws_row-=2;
	term_set_size(term, win.ws_row, win.ws_col);
	term_set_offset(term, 0, 0);
	ioctl(master, TIOCSWINSZ, &win);

	term_set_size(term2, 2, win.ws_col);
	term_set_offset(term2, win.ws_row, 0);
	win.ws_row=2;
	ioctl(master2, TIOCSWINSZ, &win);
}


static void
usage(void)
{
	(void)fprintf(stderr,
	    "usage: termim [-nw]\n"
	    "\t -n\tYour terminal display ambiguous as narrow (default)\n"
	    "\t -w\tYour terminal display ambiguous as wide\n"
	);
	exit(1);
}

static void
finish(void)
{
	int e, status;

	if (waitpid(child, &status, 0) == child) {
		if (WIFEXITED(status))
			e = WEXITSTATUS(status);
		else if (WIFSIGNALED(status))
			e = WTERMSIG(status);
		else /* can't happen */
			e = 1;
		done(e);
	}
	if (waitpid(child2, &status, 0) == child2) {
		if (WIFEXITED(status))
			e = WEXITSTATUS(status);
		else if (WIFSIGNALED(status))
			e = WTERMSIG(status);
		else /* can't happen */
			e = 1;
		done(e);
	}
}

static void
doshell(char **av)
{
	const char *shell;

	shell = getenv("SHELL");
	if (shell == NULL)
		shell = _PATH_BSHELL;

	(void)close(master);
	login_tty(slave);
	if (av[0]) {
		execvp(av[0], av);
		warn("%s", av[0]);
	} else {
		execl(shell, shell, "-i", (char *)NULL);
		warn("%s", shell);
	}
	fail();
}

static void
dodock()
{
	char *argv[]={"termim-next", NULL};
	(void)close(tube[0]);
	(void)close(master2);
	login_tty(slave2);
	execvp(argv[0], argv);
	warn("%s", "ime");
	fail();
}

static void
fail(void)
{
	(void)kill(0, SIGTERM);
	done(1);
}

static void
done(int eno)
{
	time_t tvec;

	if (ttyflg)
		(void)tcsetattr(STDIN_FILENO, TCSAFLUSH, &tt);
	tvec = time(NULL);
	term_destroy(term);
	term_destroy(term2);
	(void)close(master);
	(void)close(master2);
	exit(eno);
}
