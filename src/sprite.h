#ifndef SPRITE_H


#include "SDL.h"
#include "zzjson.h"


typedef enum {
	ACTION_IDLE = 0,
	ACTION_POSE,
	ACTION_STEP,
	ACTION_RUN,
	ACTION_SHOOT,
	ACTION_RUNSHOOT,
	ACTION_JUMP,
	ACTION_JUMPSHOOT,
	ACTION_COUNT
} action_t;

typedef struct {
	char *name;
	int framecount;
	Uint32 *dur;
	int *frame;
	SDL_Surface **gpx;
} anim_t;

typedef struct {
	action_t action;
	anim_t anim[ACTION_COUNT];
	Uint32 actionstart;
	int actioncomplete;
} sprite_t;


sprite_t *sprite_create();
sprite_t *sprite_load(char *name, ZZJSON *json);

#endif
