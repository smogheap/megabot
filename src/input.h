#ifndef INPUT_H
#define INPUT_H


#include "SDL.h"


#define INPUT_REPEAT       20
#define INPUT_REPEAT_DELAY 20


typedef struct input_t {
	int up;
	int down;
	int left;
	int right;
	int jump;
	int shoot;
	int ok;
	int cancel;
	int volup;
	int voldown;
	SDLKey any;
	Uint16 mod;
	int esc;
	int quit;
	struct state {
		struct repeat {
			Uint32 shoot;
		} repeat;
	} state;
} input_t;


int input_read(input_t *input);
void input_clear(input_t *input);


#endif
