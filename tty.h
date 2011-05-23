struct tty {
	/* i/o tty's fd*/
	int in;
	int out;

	/* buffer for holding ansi escape sequence */
	char buf[256];
	int i;
	int escape;
};

struct tty * tty_create();
void tty_destroy(struct tty *tty);
void tty_assoc_input(struct tty *tty, int in);
void tty_assoc_output(struct tty *tty, int out);
ssize_t tty_readr_writev(struct tty *tty, char *ibuf, size_t len);
ssize_t tty_readv_writer(struct tty *tty, char *ibuf, size_t len);
