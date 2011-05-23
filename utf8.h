#include <stdint.h>

#define bb00000000 0x0
#define bb00000011 0x3
#define bb00000111 0x7
#define bb00001111 0xF
#define bb00011100 0x1C
#define bb00111111 0x3F
#define bb10000000 0x80
#define bb11000000 0xC0
#define bb11100000 0xE0
#define bb11110000 0xF0
#define bb11111000 0xF8

#define HALF 1
#define FULL 2
#define AMBI -1

struct width_interval {
	int beg;
	int end;
	int width;
};

int ustrlen(const char *s, int l);
int ustrwidth(const char *s, int l);
const char * unext(const char **s, int *l);
uint32_t unicode(const char *s);
