#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_image.h"

#include "megabot.h"
#include "scale.h"
#include "zzjson.h"
#include "input.h"
#include "util.h"


gpx_t GPX;

struct {
	enum {
		MODE_SPLASH,
		MODE_TITLE,
		MODE_STORY,
		MODE_STAGESELECT,
		MODE_ACTION,
		MODE_CREDITS
	} mode;

	union {
		/* title */
		struct {
			char **label;
			int active;
		} options;
	} data;
} state;


int main(int argc, char **argv) {
	SDL_Surface *screen = NULL;
	int zoom = 1;

    ZZJSON_CONFIG zzconfig = { ZZJSON_VERY_LOOSE, NULL,
							   (int(*)(void*)) fgetc,
							   (int(*)(int,void*)) ungetc,
							   malloc, calloc, free, realloc,
							   stderr, jsonerror, stdout,
							   (int(*)(void*,const char*,...)) fprintf,
							   (int(*)(int,void*)) fputc };
	ZZJSON *json = NULL;
	ZZJSON *j = NULL;
	int quit = 0;
	input_t input;
	Mix_Music *music;
	SDL_Surface *temp;
	Uint32 now = 0;
	Uint32 next = 0;
	Uint32 modestart = 0;
	Uint32 voldraw = 0;
	DIR *dir = NULL;
	struct dirent *ent;
	struct stat st;
	char *buf;
	char *c;
	int i;
	SDL_Rect rect1;
	SDL_Rect rect2;
	SDL_Joystick *joy;
	char *credits;
	FILE *creditsfile;
	int volume = 24;

	FILE *foo;

	/* init */
	memset(&input, 0, sizeof(input_t));
	memset(&state, 0, sizeof(state));
	state.mode = MODE_TITLE;
	buf = malloc(1024);

	/* figure out screen zooming, if any */
	if(argc > 1) {
		if(!strncmp("-z", argv[1], strlen("-z"))) {
			zoom = -1;
			if(strlen(argv[1]) > strlen("-z=")) {
				zoom = strtod(argv[1] + strlen("-z="), NULL);
			}
		}
	}

	/* init SDL */
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK |
			 SDL_INIT_EVENTTHREAD | SDL_INIT_TIMER);

	if(zoom == -1) {
		screen = SDL_SetVideoMode(0, 0, 0, SDL_ANYFORMAT | SDL_FULLSCREEN);
	} else {
		screen = SDL_SetVideoMode(MEGABOT_SCREEN_W * zoom,
								  MEGABOT_SCREEN_H * zoom, 0, SDL_ANYFORMAT);
	}

	if(!screen) {
		printf("SDL ERROR: %s\n", SDL_GetError());
		return -1;
	} else {
		if(zoom == -1) {
			zoom = screen->w / MEGABOT_SCREEN_W;
			if(screen->h / MEGABOT_SCREEN_H < zoom) {
				zoom = screen->h / MEGABOT_SCREEN_H;
			}
		}
		printf("zoom: %d\n", zoom);
	}
	atexit(SDL_Quit);
	if(SDL_NumJoysticks()) {
		printf("grabbing %s...\n", SDL_JoystickName(0));
		SDL_JoystickEventState(SDL_ENABLE);
		joy = SDL_JoystickOpen(0);
	}
	SDL_ShowCursor(0);
	SDL_WM_SetCaption("MEGABOT", NULL);
	SDL_Flip(screen);

	/* load enough to draw "loading..." ;^) */
	temp = IMG_Load("base/text.png");
	GPX.text = scaleFactor(temp, zoom);
	SDL_FreeSurface(temp);

	draw_text(screen, zoom, "Loading...", 32, 32);
	SDL_Flip(screen);

	foo = fopen("out.txt", "w");
	fprintf(foo, "load credits\n");
	fclose(foo);

	/* load credits */
	creditsfile = fopen("CREDITS", "r");
	if(!creditsfile) {
		creditsfile = fopen("credits.txt", "r"); //fat or other goony fs
	}
	i = 0;
	if(creditsfile) {
		credits = malloc(2048 * sizeof(char));
		memset(credits, 0, sizeof(credits));
		i = (int)fread(credits, 2047, sizeof(char), creditsfile);
		printf("credits length: %d\n", i);
		fclose(creditsfile);
	} else {
		credits = strdup("ERROR: 'CREDITS' file not found");
	}

	foo = fopen("out.txt", "a");
	fprintf(foo, "got credits\n");
	fclose(foo);

	printf("---\n%s\n---\n", credits);

	/* load game config(s) */
	printf("Reading base player data...");

	foo = fopen("out.txt", "a");
	fprintf(foo, "player.json\n");
	fclose(foo);

	zzconfig.ihandle = fopen("base/player.json", "r");
	if(zzconfig.ihandle) {
		printf("ok.\n");
		json = zzjson_parse(&zzconfig);

		if(json) {
			j = zzjson_object_find_labels(json, "name", NULL);
			printf("name: %s\n", jsonstring(j));

	foo = fopen("out.txt", "a");
	fprintf(foo, "name: %s\n", jsonstring(j));
	fclose(foo);

			j = zzjson_object_find_labels(json, "speed", "walk", NULL);
			printf("walk speed: %d\n", jsonint(j));

	foo = fopen("out.txt", "a");
	fprintf(foo, "walkspeed: %d\n", jsonint(j));
	fclose(foo);

			fclose(zzconfig.ihandle);
			zzjson_free(&zzconfig, json);
		} else {
			printf("failed to load 'base/player.json'.\n");
			return -1;
		}
	} else {
		printf("failed. Pack games better know what they're doing...\n");
	}
	dir = opendir("pack");

	foo = fopen("out.txt", "a");
	fprintf(foo, "dir %p\n", dir);
	fclose(foo);

	if(dir) {
		while((ent = readdir(dir))) {
			sprintf(buf, "pack/%s", ent->d_name);

	foo = fopen("out.txt", "a");
	fprintf(foo, "dirent %s\n", ent->d_name);
	fclose(foo);

			if(ent->d_name[0] != '.' && !stat(buf, &st)) {
				//state.data.options.label .active
				printf("Reading game '%s'...\n", ent->d_name);
				sprintf(buf, "pack/%s/%s.json", ent->d_name, ent->d_name);
				zzconfig.ihandle = fopen(buf, "r");
				json = zzjson_parse(&zzconfig);

	foo = fopen("out.txt", "a");
	fprintf(foo, "pack %s\n", ent->d_name);
	fclose(foo);

				if(json) {
					j = zzjson_object_find_labels(json, "title", NULL);
					if(j) {
						i = ++state.data.options.active;
						if((state.data.options.label =
							realloc(state.data.options.label,
									sizeof(char *) * i + 1))) {
							state.data.options.label[i - 1] =
								strdup(jsonstring(j));
							state.data.options.label[i] = NULL;
						}
					}
					j = zzjson_object_find_labels(json, "foo", NULL);
					printf("foo: %s\n", jsonstring(j));
					fclose(zzconfig.ihandle);
					zzjson_free(&zzconfig, json);
				} else {
					printf("failed to load '%s/%s.json'.\n",
						   ent->d_name, ent->d_name);
					return -1;
				}
			}

	foo = fopen("out.txt", "a");
	fprintf(foo, "stat: %d\n", stat(ent->d_name, &st));
	fprintf(foo, "errno %d\n", errno);
	fclose(foo);

		}
		closedir(dir);
	} else {
		printf("no games found, bailing...\n");
		quit = 1;
	}
	state.data.options.active = 0;

	/* load assets */
	temp = IMG_Load("base/arrows.png");
	GPX.arrows = scaleFactor(temp, zoom);
	SDL_FreeSurface(temp);
	temp = IMG_Load("base/title.png");
	GPX.title = scaleFactor(temp, zoom);
	SDL_FreeSurface(temp);
	temp = IMG_Load("base/black.png");
	GPX.black = scaleFactor(temp, zoom);
	SDL_FreeSurface(temp);

	foo = fopen("out.txt", "a");
	fprintf(foo, "music\n");
	fclose(foo);

#ifdef GP2X
	Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 512);
	Mix_VolumeMusic(volume);
	Mix_Volume(-1, volume);
#else
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 512);
#endif
	music = Mix_LoadMUS("base/vibe-mil.ogg"); /* Milla */
	//	music = Mix_LoadMUS("base/bossintro.ogg");

	foo = fopen("out.txt", "a");
	fprintf(foo, "loaded\n");
	fclose(foo);

	/* start up to title/game select */
	Mix_PlayMusic(music, -1);

	foo = fopen("out.txt", "a");
	fprintf(foo, "playing\n");
	fclose(foo);

	modestart = SDL_GetTicks();
	while(!quit) {
		quit = input_read(&input);

		switch(state.mode) {
		case MODE_TITLE:
			/* update state */
			i = 0;
			while((c = state.data.options.label[i])) {
				i++;
			}
			if(input.up && state.data.options.active > 0) {
				state.data.options.active--;
			} else if(input.down && state.data.options.active < i + 1) {
				state.data.options.active++;
			}
			if(input.ok) {
				if(state.data.options.active == i + 1) {
					quit = 1;
				} else if(state.data.options.active == i) {
					state.mode = MODE_CREDITS;
					modestart = now;
					//credits
				} else {
					//fire up game: state.data.options.label[active]
				}
			}

			/* render frame */
			SDL_BlitSurface(GPX.title, NULL, screen, NULL);
			/* list games */
			i = 0;
			while((c = state.data.options.label[i])) {
				draw_text(screen, zoom, c, 128,
						  144 - (state.data.options.active *
								 MEGABOT_TEXT_SIZE));
				i++;
			}
			/* extra menu items: "Credits", "Quit" */
			i = state.data.options.active;
			draw_text(screen, zoom, "Credits", 128, 144 - ((i - 1) *
														   MEGABOT_TEXT_SIZE));
			draw_text(screen, zoom, "Quit", 128, 144 - ((i - 2) *
														MEGABOT_TEXT_SIZE));
			/* arrow */
			rect1.x = zoom * (8 * 3);
			rect1.y = now / 200 % 2 ? zoom * 8 : 0;
			rect2.x = zoom * 120;
			rect2.y = zoom * 144;
			rect1.w = rect1.h = rect2.w = rect2.h = zoom * 8;
			SDL_BlitSurface(GPX.arrows, &rect1, screen, &rect2);;
			break;
		case MODE_CREDITS:
			/* draw credits */
			SDL_BlitSurface(GPX.black, NULL, screen, NULL);
			i = (MEGABOT_SCREEN_H) - ((now - modestart) / 40);
			if(i < 0 - (MEGABOT_TEXT_SIZE * 
						(draw_text(screen, zoom, credits, 0, i) + 1))) {
				/* done */
				input.ok = 1;
			}
			/* handle input */
			if(input.any || input.ok || input.cancel || input.esc) {
				state.mode = MODE_TITLE;
				modestart = now;
			}
			break;
		default:
			break;
		}

		// handle+draw any volume change
		if(input.volup) {
			volume += 1;
			if(volume > MIX_MAX_VOLUME) {
				volume = MIX_MAX_VOLUME;
			}
		}
		if(input.voldown) {
			volume -= 1;
			if(volume < 0) {
				volume = 0;
			}
		}
		if(input.volup || input.voldown) {
			Mix_Volume(-1, volume);
			Mix_VolumeMusic(volume);
			voldraw = now;
		}
		if(voldraw && now - voldraw < 1500) {
			sprintf(buf, "volume: [-----------]");
			i = volume * 10 / MIX_MAX_VOLUME;
			buf[ 9 + i ] = '|';
			draw_text(screen, zoom, buf, 72, 224);
		}

		/* draw, cycle cleanup */
		SDL_Flip(screen);
		input_clear(&input);
		now = SDL_GetTicks();
		if(next - now > 0 && next - now < MEGABOT_FRAMETIME) {
			SDL_Delay(next - now);
		}
		next = now + MEGABOT_FRAMETIME;
	}
	/* TODO: nice fadeout */
	Mix_FadeOutMusic(500);

	/* free state/buffers */
	free(buf);
	i = 0;
	while((c = state.data.options.label[i])) {
		free(c);
		i++;
	}
	free(state.data.options.label);
	free(credits);

	/* free assets */
	Mix_FreeMusic(music);
	SDL_FreeSurface(GPX.title);
	SDL_FreeSurface(GPX.arrows);
	SDL_FreeSurface(GPX.text);

	Mix_CloseAudio();
	SDL_Quit();

#ifdef GP2X
	chdir("/usr/gp2x");
	execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
#endif

	return 0;
}
