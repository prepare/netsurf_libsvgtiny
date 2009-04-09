
#include <SDL/SDL.h>

#include "libnsfb.h"
#include "nsfb.h"
#include "frontend.h"

static void
set_palette(nsfb_t *nsfb)
{
    SDL_Surface *sdl_screen = nsfb->frontend_priv;
    SDL_Color palette[256];
    int rloop, gloop, bloop;
    int loop = 0;

    /* build a linear R:3 G:3 B:2 colour cube palette. */
    for (rloop = 0; rloop < 8; rloop++) {
        for (gloop = 0; gloop < 8; gloop++) {
            for (bloop = 0; bloop < 4; bloop++) {
                palette[loop].r = (rloop << 5) | (rloop << 2) | (rloop >> 1);
                palette[loop].g = (gloop << 5) | (gloop << 2) | (gloop >> 1);
                palette[loop].b = (bloop << 6) | (bloop << 4) | (bloop << 2) | (bloop);
                nsfb->palette[loop] = palette[loop].r | 
                                      palette[loop].g << 8 | 
                                      palette[loop].b << 16;
                loop++;
            }
        }
    }

    /* Set palette */
    SDL_SetColors(sdl_screen, palette, 0, 256);

}

static int sdl_initialise(nsfb_t *nsfb)
{
    SDL_Surface *sdl_screen;

    if (nsfb->frontend_priv != NULL)
        return -1;

    /* sanity checked depth. */
    if ((nsfb->bpp != 32) && (nsfb->bpp != 16) && (nsfb->bpp != 8))
        nsfb->bpp = 16; 

    /* initialise SDL library */
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        return -1;
    }
    atexit(SDL_Quit);

    sdl_screen = SDL_SetVideoMode(nsfb->width, 
                                  nsfb->height, 
                                  nsfb->bpp, 
                                  SDL_SWSURFACE);

    if (sdl_screen == NULL ) {
        fprintf(stderr, "Unable to set video: %s\n", SDL_GetError());
        return -1;
    }

    nsfb->frontend_priv = sdl_screen;

    if (nsfb->bpp == 8)
        set_palette(nsfb);

    nsfb->ptr = sdl_screen->pixels;
    nsfb->linelen = sdl_screen->pitch;
    
    SDL_ShowCursor(SDL_DISABLE);

    return 0;
}

static int sdl_finalise(nsfb_t *nsfb)
{
    nsfb=nsfb;
    return 0;
}

static int sdl_input(nsfb_t *nsfb)
{
    int got_event;
    SDL_Event event;

    got_event = SDL_WaitEvent(&event);
    if (event.type == SDL_QUIT)
        exit(0);
    nsfb=nsfb;
    return 1;
}

static int sdl_release(nsfb_t *nsfb, nsfb_bbox_t *box)
{
    SDL_Surface *sdl_screen = nsfb->frontend_priv;

    SDL_UpdateRect(sdl_screen, 
                   box->x0, 
                   box->y0, 
                   box->x1 - box->x0, 
                   box->y1 - box->y0);

    return 0;
}

const nsfb_frontend_rtns_t sdl_rtns = {
    .initialise = sdl_initialise,
    .finalise = sdl_finalise,
    .input = sdl_input,
    .release = sdl_release,
};

NSFB_FRONTEND_DEF(sdl, NSFB_FRONTEND_SDL, &sdl_rtns)
