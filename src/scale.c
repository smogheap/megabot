#include "SDL.h"
#include "SDL_image.h"
#include "scale.h"


#define ZOOM_DEPTH 32
#define ZOOM_R 0x00ff0000
#define ZOOM_G 0x0000ff00
#define ZOOM_B 0x000000ff
#define ZOOM_A 0xff000000

typedef struct tColorRGBA {
	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;
} tColorRGBA;


int scale(int src, double ratio)
{
	return (int)((double)src * ratio);
}

static int zoomSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int smooth)
{
	int flipx = 0;
	int flipy = 0;
	int x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy, ex, ey, t1, t2, sstep;
	tColorRGBA *c00, *c01, *c10, *c11;
	tColorRGBA *sp, *csp, *dp;
	int dgap;

	/*
	 * Variable setup 
	 */
	if(smooth) {
		/*
		 * For interpolation: assume source dimension is one pixel 
		 * smaller to avoid overflow on right and bottom edge.     
		 */
		sx = (int) (65536.0 * (float) (src->w - 1) / (float) dst->w);
		sy = (int) (65536.0 * (float) (src->h - 1) / (float) dst->h);
	} else {
		sx = (int) (65536.0 * (float) src->w / (float) dst->w);
		sy = (int) (65536.0 * (float) src->h / (float) dst->h);
	}

	/*
	 * Allocate memory for row increments 
	 */
	if((sax = (int *) malloc((dst->w + 1) * sizeof(Uint32))) == NULL) {
		return (-1);
	}
	if((say = (int *) malloc((dst->h + 1) * sizeof(Uint32))) == NULL) {
		free(sax);
		return (-1);
	}

	/*
	 * Precalculate row increments 
	 */
	sp = csp = (tColorRGBA *) src->pixels;
	dp = (tColorRGBA *) dst->pixels;

	if(flipx) {
		csp += (src->w-1);
	}
    if(flipy) {
		csp = (tColorRGBA*)( (Uint8*)csp + src->pitch*(src->h-1) );
	}

	csx = 0;
	csax = sax;
	for(x = 0; x <= dst->w; x++) {
		*csax = csx;
		csax++;
		csx &= 0xffff;
		csx += sx;
	}
	csy = 0;
	csay = say;
	for(y = 0; y <= dst->h; y++) {
		*csay = csy;
		csay++;
		csy &= 0xffff;
		csy += sy;
	}

	dgap = dst->pitch - dst->w * 4;

	/*
	 * Switch between interpolating and non-interpolating code 
	 */
	if(smooth) {
		/*
		 * Interpolating Zoom 
		 */

		/*
		 * Scan destination 
		 */
		csay = say;
		for(y = 0; y < dst->h; y++) {
			/*
			 * Setup color source pointers 
			 */
			c00 = csp;
			c01 = csp;
			c01++;
			c10 = (tColorRGBA *) ((Uint8 *) csp + src->pitch);
			c11 = c10;
			c11++;
			csax = sax;
			for(x = 0; x < dst->w; x++) {
				/*
				 * Interpolate colors 
				 */
				ex = (*csax & 0xffff);
				ey = (*csay & 0xffff);
				t1 = ((((c01->r - c00->r) * ex) >> 16) + c00->r) & 0xff;
				t2 = ((((c11->r - c10->r) * ex) >> 16) + c10->r) & 0xff;
				dp->r = (((t2 - t1) * ey) >> 16) + t1;
				t1 = ((((c01->g - c00->g) * ex) >> 16) + c00->g) & 0xff;
				t2 = ((((c11->g - c10->g) * ex) >> 16) + c10->g) & 0xff;
				dp->g = (((t2 - t1) * ey) >> 16) + t1;
				t1 = ((((c01->b - c00->b) * ex) >> 16) + c00->b) & 0xff;
				t2 = ((((c11->b - c10->b) * ex) >> 16) + c10->b) & 0xff;
				dp->b = (((t2 - t1) * ey) >> 16) + t1;
				t1 = ((((c01->a - c00->a) * ex) >> 16) + c00->a) & 0xff;
				t2 = ((((c11->a - c10->a) * ex) >> 16) + c10->a) & 0xff;
				dp->a = (((t2 - t1) * ey) >> 16) + t1;

				/*
				 * Advance source pointers 
				 */
				csax++;
				sstep = (*csax >> 16);
				c00 += sstep;
				c01 += sstep;
				c10 += sstep;
				c11 += sstep;
				/*
				 * Advance destination pointer 
				 */
				dp++;
			}
			/*
			 * Advance source pointer 
			 */
			csay++;
			csp = (tColorRGBA *) ((Uint8 *) csp + (*csay >> 16) * src->pitch);
			/*
			 * Advance destination pointers 
			 */
			dp = (tColorRGBA *) ((Uint8 *) dp + dgap);
		}
	} else {
		/*
		 * Non-Interpolating Zoom 
		 */
		csay = say;
		for(y = 0; y < dst->h; y++) {
			sp = csp;
			csax = sax;
			for(x = 0; x < dst->w; x++) {
				/*
				 * Draw 
				 */
				*dp = *sp;
				/*
				 * Advance source pointers 
				 */
				csax++;
				sstep = (*csax >> 16);
				if(flipx) {
					sstep = -sstep;
				}
				sp += sstep;
				/*
				 * Advance destination pointer 
				 */
				dp++;
			}
			/*
			 * Advance source pointer 
			 */
			csay++;
			sstep = (*csay >> 16) * src->pitch;
			if(flipy) {
				sstep = -sstep;
			}
			csp = (tColorRGBA *) ((Uint8 *) csp + sstep);

			/*
			 * Advance destination pointers 
			 */
			dp = (tColorRGBA *) ((Uint8 *) dp + dgap);
		}
	}
	/*
	 * Remove temp arrays 
	 */
	free(sax);
	free(say);

	return (0);
}

static int shrinkSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int factorx, int factory)
{
    int x, y, dx, dy, sgap, dgap, ra, ga, ba, aa;
    int n_average;
    tColorRGBA *sp, *osp, *oosp;
    tColorRGBA *dp;

    /*
     * Averaging integer shrink
     */

    /* Precalculate division factor */
    n_average = factorx*factory;
   
    /*
     * Scan destination
     */
    sp = (tColorRGBA *) src->pixels;
    sgap = src->pitch - src->w * 4;

    dp = (tColorRGBA *) dst->pixels;
    dgap = dst->pitch - dst->w * 4;

    for (y = 0; y < dst->h; y++) {

      osp=sp;
      for (x = 0; x < dst->w; x++) {

        /* Trace out source box and accumulate */
        oosp=sp;
        ra=ga=ba=aa=0;
        for (dy=0; dy < factory; dy++) {
         for (dx=0; dx < factorx; dx++) {
          ra += sp->r;
          ga += sp->g;
          ba += sp->b;
          aa += sp->a;
          
          sp++;
         } // src dx loop
         sp = (tColorRGBA *)((Uint8*)sp + (src->pitch - 4*factorx)); // next y
        } // src dy loop

        // next box-x
        sp = (tColorRGBA *)((Uint8*)oosp + 4*factorx);
                
        /* Store result in destination */
        dp->r = ra/n_average;
        dp->g = ga/n_average;
        dp->b = ba/n_average;
        dp->a = aa/n_average;
                 
        /*
         * Advance destination pointer 
         */
         dp++;
        } // dst x loop

        // next box-y
        sp = (tColorRGBA *)((Uint8*)osp + src->pitch*factory);

        /*
         * Advance destination pointers 
         */
        dp = (tColorRGBA *) ((Uint8 *) dp + dgap);
      } // dst y loop

    return (0);
}

static int shrinkSurface(SDL_Surface *src, SDL_Surface *dst)
{
	SDL_Surface *temp;
	tColorRGBA *sp;
	tColorRGBA *dp;
	int x, y;
	int i;
	int match;
	int den;
	int r, g, b, a;
	int foo = 0;
	double ratio = (double)dst->w / (double)src->w;


	temp = SDL_CreateRGBSurface(SDL_SRCALPHA, scale(src->w, ratio), src->h,
								32, 0x000000ff, 0x0000ff00, 0x00ff0000,
								0xff000000);
	sp = (tColorRGBA *)src->pixels;
	dp = (tColorRGBA *)temp->pixels;

	/* first squish it */
	for(y = 0; y < src->h; y++) {
		for(x = 0; x < src->w; x++) {
			den = 0;
			for(i = 0; i < temp->w; i++) {
				r = g = b = a = 0;
				if(scale(x, ratio) != i) {
					if(den) {
						dp->r = r / den;
						dp->g = g / den;
						dp->b = b / den;
						dp->a = a / den;
						dp++;
					}
					den = 0;
					continue;
				}
				r += sp->r;
				g += sp->g;
				b += sp->b;
				a += sp->a;
				den++;
			}
			sp++;
		}
	}

	sp = (tColorRGBA *)temp->pixels;
	dp = (tColorRGBA *)dst->pixels;

	/* then squash it */
	for(x = 0; x < temp->w; x++) {
		for(y = 0; y < temp->h; y++) {
			den = 0;
			for(i = 0; i < dst->h; i++) {
				r = g = b = a = 0;
				if(scale(y, ratio) != i) {
					if(den) {
						dp->r = r / den;
						dp->g = g / den;
						dp->b = b / den;
						dp->a = a / den;
						dp++;
					}
					den = 0;
					continue;
				}
				r += sp->r;
				g += sp->g;
				b += sp->b;
				a += sp->a;
				den++;
			}
			sp++;
		}
	}

	SDL_FreeSurface(temp);
	
	return (0);
}

SDL_Surface *scaleSurf(SDL_Surface *src, double ratio)
{
	return scaleSurfEx(src, ratio, 1);
}

SDL_Surface *scaleSurfEx(SDL_Surface *rawsrc, double ratio, int smooth)
{
	SDL_Surface *src = NULL;
	SDL_Surface *surf = NULL;
	SDL_Surface *temp = NULL;
	SDL_Surface *final = NULL;

	src = SDL_DisplayFormatAlpha(rawsrc);

	/* FIXME: zoomSurface shrinks fugly-ly; this is a very slow workaround */
	if(ratio < 1.0) {
		surf = SDL_CreateRGBSurface(SDL_SRCALPHA,
									scale(src->w, ratio) * 4,
									scale(src->h, ratio) * 4,
									ZOOM_DEPTH, ZOOM_R, ZOOM_G, ZOOM_B, ZOOM_A);
		if(zoomSurfaceRGBA(src, surf, 0)) {
			SDL_FreeSurface(surf);
			return NULL;
		}
		temp = SDL_CreateRGBSurface(SDL_SRCALPHA,
									scale(src->w, ratio), scale(src->h, ratio),
									ZOOM_DEPTH, ZOOM_R, ZOOM_G, ZOOM_B, ZOOM_A);
		if(!shrinkSurfaceRGBA(surf, temp, 4, 4)) {
			final = SDL_DisplayFormatAlpha(temp);
		}
		SDL_FreeSurface(temp);
	} else {
		surf = SDL_CreateRGBSurface(SDL_SRCALPHA,
									scale(src->w, ratio), scale(src->h, ratio),
									ZOOM_DEPTH, ZOOM_R, ZOOM_G, ZOOM_B, ZOOM_A);
		if(!zoomSurfaceRGBA(src, surf, smooth)) {
			final = SDL_DisplayFormatAlpha(surf);
		}
	}
	SDL_FreeSurface(src);
	SDL_FreeSurface(surf);

	return final;
}

SDL_Surface *scaleFactor(SDL_Surface *rawsrc, int factor)
{
	SDL_Surface *src = NULL;
	SDL_Surface *tmp = NULL;
	SDL_Surface *final = NULL;
	SDL_Rect rect;
	Uint32 *pixel;
	int x;
	int y;

	src = SDL_DisplayFormatAlpha(rawsrc);
	tmp = SDL_CreateRGBSurface(SDL_SRCALPHA | SDL_SWSURFACE,
							   src->w * factor, src->h * factor,
							   ZOOM_DEPTH, ZOOM_R, ZOOM_G, ZOOM_B, ZOOM_A);
	pixel = src->pixels;
	rect.w = factor;
	rect.h = factor;
	for(y = 0; y < src->h; y++) {
		for(x = 0; x < src->w; x++) {
			rect.x = x * factor;
			rect.y = y * factor;
			if(SDL_FillRect(tmp, &rect, *pixel)) {
				printf("%s\n", SDL_GetError());
			}
			pixel++;
		}
	}
	final = SDL_DisplayFormatAlpha(tmp);
	SDL_FreeSurface(src);
	SDL_FreeSurface(tmp);
	return final;
}
