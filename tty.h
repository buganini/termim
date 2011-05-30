struct tty {
	/* output tty's fd*/
	int out;

	/* buffer for holding ansi escape sequence */
	char buf[256];
	int i;
	int escape;
};

struct tty * tty_create();
void tty_destroy(struct tty *tty);
void tty_assoc_output(struct tty *tty, int out);
void tty_init(void);
ssize_t tty_writev(struct tty *tty, char *ibuf, size_t len);
ssize_t tty_writer(struct tty *tty, char *ibuf, size_t len);
