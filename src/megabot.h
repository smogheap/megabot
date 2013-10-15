#ifndef MEGABOT_H


#include "SDL.h"


#define MEGABOT_SCREEN_W  320
#define MEGABOT_SCREEN_H  240
#define MEGABOT_TEXT_SIZE   8
#define MEGABOT_FRAMETIME (1000 / 60)


typedef struct {
	SDL_Surface *black;
	SDL_Surface *title;
	SDL_Surface *text;
	SDL_Surface *arrows;
	SDL_Surface *stageselect;
	struct {
		SDL_Surface **data;
		int *idle;
		int *shoot;
	} player;
} gpx_t;


#endif
