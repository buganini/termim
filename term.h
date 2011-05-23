struct term {
	/* output terminal's fd*/
	int out;

	/* buffer for holding ansi escape sequence */
	char buf[256];
	int i;
	int escape;

	/* window size */
	int siz_row;
	int siz_col;

	/* cursor attributes */
	int cur_row;
	int cur_col;
	int cur_visible;
	int cur_blink;

	/* window offset */
	int off_row;
	int off_col;

	/* scroll region */
	int scr_beg;
	int scr_end;

	/* display attributes */
	int bold;
	int underline;
	int blink;
	int reverse;
	int invisible;
	int fg;
	int bg;
};

struct term * term_create();
void term_destroy(struct term *term);
void term_set_size(struct term *term, int siz_row, int siz_col);
void term_set_offset(struct term *term, int off_row, int off_col);
void term_assoc_output(struct term *term, int out);
ssize_t term_read(struct term *term, void *buf, size_t len);
ssize_t term_write(struct term *term, const char *ibuf, size_t len);
char ** parse_arg(char *s);
