#ifndef SCALE_H
#define SCALE_H


int scale(int src, double ratio);
SDL_Surface *scaleSurf(SDL_Surface *src, double ratio);
SDL_Surface *scaleSurfEx(SDL_Surface *src, double ratio, int smooth);
SDL_Surface *scaleFactor(SDL_Surface *src, int factor);


#endif
