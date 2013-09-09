#include "level.h"
#include <SDL/SDL.h>
int main(void)
{
    SDL_Surface *screen;
    init_windows(screen);
    getchar();
}

int init_windows(SDL_Surface *screen)
{
    int ret;
    ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    ERRP(-1 == ret, goto ERR0, 1, "SDL_Init failed!\n");

    screen = SDL_SetVideoMode(480, 272, 32, SDL_SWSURFACE);
    ERRP(NULL == screen, goto ERR0, 1, "SDL_SetVideoMode failed!\n");
    
    return 0;
ERR0:
    return -1;
}
