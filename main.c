#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include "menu.h"

static void sdlInitMainComponents(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        SDL_Log("Nem indithato az SDL: %s", SDL_GetError());
        exit(1);
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("Audio nem letrehozhato: %s\n", Mix_GetError());
        exit(1);
    }
    TTF_Init();
}

int main(int argc, char *argv[]) {
    sdlInitMainComponents();

    openMenu();

    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
