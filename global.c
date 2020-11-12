#include "debugmalloc.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"

//SZiNVÁLASZTÉK:
SDL_Color snek_green = { 0xc4, 0xff, 0x0e, 0xff };
SDL_Color black = { 0x00, 0x00, 0x00, 0xff };
SDL_Color snek_bg = { 0xff, 0xe9, 0x9b, 0xff };
SDL_Color red = { 0xff, 0x00, 0x00, 0xff };
SDL_Color white = { 0xff, 0xff, 0xff, 0xff };

//GOMBHALMAZ MEGRAJZOLÁSÁRA SZOLGÁLÓ FÜGGVÉNY:
void showButtons(WindowSpecs* wspecs, ButtonSet* btnSet) {
    Button* btns = btnSet->btns;
    int sizeBtn = btnSet->sizeBtn;

    //GOMBOK:
    TTF_Font *font = TTF_OpenFont("./data/LUZRO.ttf", 18);
    if (!font) {
        SDL_Log("Nem sikerult megnyitni a fontot: %s\n", TTF_GetError());
        exit(1);
    }
    SDL_Surface *label = NULL;
    SDL_Texture *label_t = NULL;

    for (int i = 0; i < sizeBtn; i++) {
        boxRGBA(wspecs->renderer, btns[i].x, btns[i].y, btns[i].x + btns[i].w, btns[i].y + btns[i].h, snek_green.r, snek_green.g, snek_green.b, snek_green.a);
        rectangleRGBA(wspecs->renderer, btns[i].x, btns[i].y, btns[i].x + btns[i].w + 1, btns[i].y + btns[i].h + 1, black.r, black.g, black.b, black.a);

        label = TTF_RenderUTF8_Blended(font, btns[i].name, black);
        label_t = SDL_CreateTextureFromSurface(wspecs->renderer, label);

        SDL_Rect target = {
            btns[i].x + (btns[i].w - label->w) / 2,
            btns[i].y + (btns[i].h - label->h) / 2,
            label->w,
            label->h
        };
        SDL_RenderCopy(wspecs->renderer, label_t, NULL, &target);
    }

    SDL_FreeSurface(label);
    SDL_DestroyTexture(label_t);
    TTF_CloseFont(font);

    //MEGJELENÍTÉS:
    SDL_RenderPresent(wspecs->renderer);
}

//MEGADJA, MELYIK GOMB LETT KATTINTVA A MEGADOTT GOMBHALMAZBÓL:
int getButtonClicked(SDL_Event* ev, ButtonSet* btnSet) {
    Button* btns = btnSet->btns;
    SDL_MouseButtonEvent button = ev->button;

    int i = 0;
    while (i < btnSet->sizeBtn && !((button.y >= btns[i].y) && (button.y <= (btns[i].y + btns[i].h)) && (button.x >= btns[i].x) && (button.x <= (btns[i].x + btns[i].w))))
        i++;
    if (i < btnSet->sizeBtn) {
        return i;
    }
    return btnSet->sizeBtn;
}

//TTF-ES SZÖVEG KIÍRÁSA PARAMÉTEREK ÁLTAL TESTRESZABVA:
void showLabel(WindowSpecs* wspecs, int fontSize, char* str, SDL_Rect* targetCoord) {
    TTF_Font *font = TTF_OpenFont("./data/LUZRO.ttf", fontSize);
    if (!font) {
        SDL_Log("Nem sikerult megnyitni a fontot: %s\n", TTF_GetError());
        exit(1);
    }

    SDL_Surface *label = TTF_RenderUTF8_Blended(font, str, black);
    SDL_Texture *label_t = SDL_CreateTextureFromSurface(wspecs->renderer, label);

    targetCoord->w = label->w;
    targetCoord->h = label->h;

    SDL_RenderCopy(wspecs->renderer, label_t, NULL, targetCoord);
    TTF_CloseFont(font);
}

//MODULOK ÁLTAL GYAKRAN HASZNÁLT SDL ELEMEKET INICIALIZÁLÓ FÜGGVÉNY:
void sdlInitGeneral(char const *label, WindowSpecs* wspecs) {
    //ABLAK:
    SDL_Window *window = SDL_CreateWindow(label, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, wspecs->w, wspecs->h, 0);
    if (window == NULL) {
        SDL_Log("Nem hozhato letre az ablak: %s", SDL_GetError());
        exit(1);
    }
    //RENDERER:
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
        SDL_Log("Nem hozhato letre a megjelenito: %s", SDL_GetError());
        exit(1);
    }
    SDL_RenderClear(renderer);
    //IKON:
    SDL_Surface* icon = IMG_Load("./data/png/icon.png");
    if (icon == NULL) {
        SDL_Log("Kep betoltesi hiba: %s", IMG_GetError());
        exit(1);
    }
    SDL_SetWindowIcon(window, icon);
    SDL_FreeSurface(icon);

    //ÁTADÁS:
    wspecs->window = window;
    wspecs->renderer = renderer;
    wspecs->bitMap = NULL;
    wspecs->bgm = NULL;
}

//MODULOK ÁLTAL GYAKRAN HASZNÁLT SDL ELEMEKET TÖRLŐ FÜGGVÉNY:
void sdlCloseGeneral(WindowSpecs* wspecs) {
    //RENDERER TÖRLÉSE:
    SDL_RenderClear(wspecs->renderer);
    SDL_DestroyRenderer(wspecs->renderer);
    wspecs->renderer = NULL;

    //ABLAK TÖRLÉSE:
    SDL_DestroyWindow(wspecs->window);
    wspecs->window = NULL;

    //BITMAP TÖRLÉSE:
    SDL_DestroyTexture(wspecs->bitMap);
    wspecs->bitMap = NULL;

    //ZENE TÖRLÉSE:
    Mix_FreeMusic(wspecs->bgm);
    wspecs->bgm = NULL;
}
