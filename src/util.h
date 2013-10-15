#ifndef UTIL_H


#include "zzjson.h"
#include "SDL.h"


void jsonerror(void *ehandle, const char *format, ...);
char *jsonstring(ZZJSON *obj);
int jsonint(ZZJSON *obj);
void draw_fade(Uint8 alpha, SDL_Surface *surf);
int draw_text(SDL_Surface *dest, int zoom, char *text, int x, int y);


#endif
