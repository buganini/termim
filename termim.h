struct termim_event {
	unsigned char type;
	int code;
	int modifiers;
	char *raw;
	size_t raw_length;
};

#define TERMIM_EVENT_KEY  0xfd
#define TERMIM_EVENT_RAW  0xfe
#define TERMIM_EVENT_EOF  0xff

#define TERMIM_KEY_0 0x30
#define TERMIM_KEY_1 0x31
#define TERMIM_KEY_2 0x32
#define TERMIM_KEY_3 0x33
#define TERMIM_KEY_4 0x34
#define TERMIM_KEY_5 0x35
#define TERMIM_KEY_6 0x36
#define TERMIM_KEY_7 0x37
#define TERMIM_KEY_8 0x38
#define TERMIM_KEY_9 0x39
#define TERMIM_KEY_ENTER 0x0D
#define TERMIM_KEY_SPACE 0x20
#define TERMIM_KEY_BACKSPACE 0x7F
#define TERMIM_KEY_UP 0xff
#define TERMIM_KEY_DOWN 0xfe
#define TERMIM_KEY_LEFT 0xfd
#define TERMIM_KEY_RIGHT 0xfc

#define TERMIM_MOD_ALT 1

struct termim_event * termim_read_input();
