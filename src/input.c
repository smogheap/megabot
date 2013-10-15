#include "input.h"
#include "gp2x.h"
#include "SDL.h"


void input_clear(input_t *input) {
	input->up = 0;
	input->down = 0;
	input->left = 0;
	input->right = 0;
	input->jump = 0;
	input->shoot = 0;
	input->ok = 0;
	input->cancel = 0;
	//input->volup = 0;
	//input->voldown = 0;
	input->any = SDLK_UNKNOWN;
	input->mod = 0;
	input->esc = 0;
	input->quit = 0;

	return;
}

int input_read(input_t *input) {
	SDL_Event event;
	int done = 0;
	int not_any = 0;
	Uint32 now = SDL_GetTicks();

	while(SDL_PollEvent(&event)) {
		not_any = 0;

		switch(event.type) {
		case SDL_QUIT:
			done = 1;
			break;
		case SDL_KEYDOWN:
			if(event.key.keysym.sym == SDLK_ESCAPE) {
				input->esc = 1;
			} else if(event.key.keysym.sym == SDLK_UP) {
				input->up = 1;
			} else if(event.key.keysym.sym == SDLK_LEFT) {
				input->left = 1;
			} else if(event.key.keysym.sym == SDLK_RIGHT) {
				input->right = 1;
			} else if(event.key.keysym.sym == SDLK_DOWN) {
				input->down = 1;
			} else if(event.key.keysym.sym == SDLK_RETURN) {
				input->ok = 1;
			} else if(event.key.keysym.sym == SDLK_SPACE) {
				input->ok = 1;
				input->jump = 1;
			} else if(event.key.keysym.sym == 'q') {
				done = 1;
			}
			if(!not_any) {
				input->mod = event.key.keysym.mod;
				input->any = event.key.keysym.sym;
			}
			break;
		case SDL_KEYUP:
			if(event.key.keysym.sym == SDLK_ESCAPE) {
				input->esc = 0;
			} else if(event.key.keysym.sym == SDLK_UP) {
				input->up = 0;
			} else if(event.key.keysym.sym == SDLK_LEFT) {
				input->left = 0;
			} else if(event.key.keysym.sym == SDLK_RIGHT) {
				input->right = 0;
			} else if(event.key.keysym.sym == SDLK_DOWN) {
				input->down = 0;
			} else if(event.key.keysym.sym == SDLK_RETURN) {
				input->ok = 0;
			} else if(event.key.keysym.sym == SDLK_SPACE) {
				input->ok = 0;
				input->jump = 0;
			} else if(event.key.keysym.sym == 'q') {
				done = 0;
			}
			input->mod = 0;
			input->any = SDLK_UNKNOWN;
			break;
		case SDL_JOYAXISMOTION:
			switch(event.jaxis.axis) {
			case 1: //up/down
				if(event.jaxis.value < -3200) {
					input->up = 1;
					input->down = 0;
				} else if(event.jaxis.value > 3200) {
					input->down = 1;
					input->up = 0;
				} else {
					input->up = 0;
					input->down = 0;
				}
				break;
			case 2: //left/right
				if(event.jaxis.value < -3200) {
					input->left = 1;
					input->right = 0;
				} else if(event.jaxis.value > 3200) {
					input->right = 1;
					input->left = 0;
				} else {
					input->left = 0;
					input->right = 0;
				}
				break;
			}
			if(!input->any) {
				input->any = SDLK_RETURN;
			}
			break;
		case SDL_JOYBUTTONDOWN:
			switch(event.jbutton.button) {
#ifdef GP2X
			case GP2X_BUTTON_UP:
				input->up = 1;
				break;
			case GP2X_BUTTON_DOWN:
				input->down = 1;
				break;
			case GP2X_BUTTON_LEFT:
				input->left = 1;
				break;
			case GP2X_BUTTON_RIGHT:
				input->right = 1;
				break;
			case GP2X_BUTTON_A:
				break;
			case GP2X_BUTTON_B:
				input->ok = 1;
				input->jump = 1;
				break;
			case GP2X_BUTTON_X:
				input->cancel = 1;
				input->shoot = 1;
				break;
			case GP2X_BUTTON_Y:
				return 1;
				input->shoot = 1;
				input->state.repeat.shoot = now + INPUT_REPEAT_DELAY;
				break;
			case GP2X_BUTTON_START:
			case GP2X_BUTTON_SELECT:
				input->esc = 1;
				break;
			case GP2X_BUTTON_L:
				break;
			case GP2X_BUTTON_R:
				break;
			case GP2X_BUTTON_VOLUP:
				input->volup = 1;
				not_any = 1;
				break;
			case GP2X_BUTTON_VOLDOWN:
				input->voldown = 1;
				not_any = 1;
				break;
			case GP2X_BUTTON_UPLEFT:
				input->up = 1;
				input->left = 1;
				break;
			case GP2X_BUTTON_UPRIGHT:
				input->up = 1;
				input->right = 1;
				break;
			case GP2X_BUTTON_DOWNLEFT:
				input->down = 1;
				input->left = 1;
				break;
			case GP2X_BUTTON_DOWNRIGHT:
				input->down = 1;
				input->right = 1;
				break;
#endif
			default:
				if(event.jbutton.button % 2) {
					input->shoot = 1;
					input->cancel = 1;
				} else {
					input->ok = 1;
					input->jump = 1;
				}
				break;
			}
			if(!input->any) {
				input->any = SDLK_RETURN;
			}
			break;
		case SDL_JOYBUTTONUP:
			switch(event.jbutton.button) {
#ifdef GP2X
			case GP2X_BUTTON_UP:
				input->up = 0;
				break;
			case GP2X_BUTTON_DOWN:
				input->down = 0;
				break;
			case GP2X_BUTTON_LEFT:
				input->left = 0;
				break;
			case GP2X_BUTTON_RIGHT:
				input->right = 0;
				break;
			case GP2X_BUTTON_A:
				break;
			case GP2X_BUTTON_B:
				input->ok = 0;
				input->jump = 0;
				break;
			case GP2X_BUTTON_X:
				input->cancel = 0;
				input->shoot = 0;
				break;
			case GP2X_BUTTON_Y:
				input->shoot = 0;
				input->state.repeat.shoot = 0;
				break;
			case GP2X_BUTTON_START:
			case GP2X_BUTTON_SELECT:
				input->esc = 0;
				break;
			case GP2X_BUTTON_L:
				break;
			case GP2X_BUTTON_R:
				break;
			case GP2X_BUTTON_VOLUP:
				input->volup = 0;
				not_any = 0;
				break;
			case GP2X_BUTTON_VOLDOWN:
				input->voldown = 0;
				not_any = 0;
				break;
			case GP2X_BUTTON_UPLEFT:
				input->up = 0;
				input->left = 0;
				break;
			case GP2X_BUTTON_UPRIGHT:
				input->up = 0;
				input->right = 0;
				break;
			case GP2X_BUTTON_DOWNLEFT:
				input->down = 0;
				input->left = 0;
				break;
			case GP2X_BUTTON_DOWNRIGHT:
				input->down = 0;
				input->right = 0;
				break;
#endif
			default:
				if(event.jbutton.button % 2) {
					input->shoot = 0;
					input->cancel = 0;
				} else {
					input->ok = 0;
					input->jump = 0;
				}
				break;
			}
			break;
		}
	}
	if(input->state.repeat.shoot && input->state.repeat.shoot < now) {
		input->shoot = 1;
		input->state.repeat.shoot += INPUT_REPEAT;
	}
	return done;
}
