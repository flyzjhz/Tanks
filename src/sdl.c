#include "sdl.h"

int show_pic(char *pic)
{
    SDL_Surface *sdl_pic;
    sdl_pic = IMG_LOAD(pic);
    SDL_SetColorKey(sdl_pic, SDL_SRCCOLORKEY, SDL_MapRGB(sdl_pic->format,255,0,255));
    sdl_pic = SDL_DisplayFormat(sdl_pic);
}
