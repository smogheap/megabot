#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "SDL.h"
#include "SDL_mixer.h"


int main(int argc, char **argv) {
	Mix_Music *music;
	int c;
	int quit = 0;

	if(argc != 2 || (argc == 1 && (!strcasecmp(argv[1], "--help") ||
								   !strcasecmp(argv[1], "-h")))) {
		printf("usage:\n");
		printf("\t%s <musicfile>\n\n", argv[0]);
		return -1;
	}

	if(SDL_Init(SDL_INIT_AUDIO
				|SDL_INIT_TIMER) < 0) {
		printf("unable to init SDL: %s\n", SDL_GetError());
		return -1;
	}
	//	Mix_Init(0);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 512);

	music = Mix_LoadMUS(argv[1]);
	if(!music) {
		printf("failed to open music: %s\n", Mix_GetError());
		return -1;
	}
	Mix_PlayMusic(music, -1);
	printf("playing '%s' forever.\npress enter to quit.\n", argv[1]);
	while(!fgetc(stdin));
	printf("...sounded good to me!  :^)\n");
	Mix_FreeMusic(music);

	Mix_CloseAudio();
	//	Mix_Quit();
	SDL_Quit();
	return 0;
}
