#include "megabot.h"
#include "util.h"


extern gpx_t GPX;


void jsonerror(void *ehandle, const char *format, ...) {
    va_list ap;
    fprintf(ehandle, "error: ");
    va_start(ap, format);
    vfprintf(ehandle, format, ap);
    va_end(ap);
    fputc('\n', ehandle);
}

char *jsonstring(ZZJSON *obj) {
	if(obj && obj->type == ZZJSON_STRING) {
		return obj->value.string.string;
	}
	return NULL;
}

int jsonint(ZZJSON *obj) {
	if(obj && obj->type == ZZJSON_NUMBER_NEGINT) {
		return (int)(0 - obj->value.number.val.ival);
	} else if(obj && obj->type == ZZJSON_NUMBER_POSINT) {
		return (int)(obj->value.number.val.ival);
	} else if(obj && obj->type == ZZJSON_NUMBER_DOUBLE) {
		return (int)(obj->value.number.val.dval);
	}
	return 0;
}

void draw_fade(Uint8 alpha, SDL_Surface *surf) {
	SDL_Rect dest = {0, 0, 0, 0};
	SDL_SetAlpha(GPX.black, SDL_SRCALPHA | SDL_RLEACCEL, alpha);
	SDL_BlitSurface(GPX.black, NULL, surf, NULL);
	return;
}

int draw_text(SDL_Surface *dest, int zoom, char *text, int x, int y)
{
	char *letter = text;
	SDL_Rect srcrect;
	SDL_Rect destrect;
	int newlines = 0;
	int dx;
	int dy;

	srcrect.w = MEGABOT_TEXT_SIZE * zoom;
	srcrect.h = MEGABOT_TEXT_SIZE * zoom;
	dx = x * zoom;
	dy = y * zoom;
	while(*letter) {
		/* move if formatting chars */
		if(*letter == '\n') {
			dx = x * zoom;
			dy += MEGABOT_TEXT_SIZE * zoom;
			newlines++;
			letter++;
			continue;
		} else if(*letter == '\t') {
			dx += 4 * MEGABOT_TEXT_SIZE * zoom;
			letter++;
			continue;
		}
		/* skip if nonprinting or lowercase */
		if(*letter < 32 || *letter > 128) {
			letter++;
			continue;
		}

		/* draw letter */
		srcrect.x = (*letter % 16) * MEGABOT_TEXT_SIZE * zoom;
		srcrect.y = ((*letter - 32) / 16) * MEGABOT_TEXT_SIZE * zoom;
		destrect.x = dx;
		destrect.y = dy;
		if(SDL_BlitSurface(GPX.text, &srcrect, dest, &destrect)) {
			printf("%s\n", SDL_GetError());
		}

		dx += (MEGABOT_TEXT_SIZE * zoom);
		letter++;
	}
	return newlines;
}
