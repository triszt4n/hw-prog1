#include "debugmalloc.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "global.h"
#include "gamesettings.h"
#include "entername.h"

//MODUL GLOBÁLIS GOMBHALMAZA:
static Button GSetBtns[] = {
    { " ", 225, 80, 25, 25 }, //UP WITH GRID_X
    { " ", 225, 105, 25, 25 }, //DOWN WITH GRID_X
    { " ", 225, 140, 25, 25 }, //UP WITH GRID_Y
    { " ", 225, 165, 25, 25 }, //DOWN WITH GRID_Y
    { "CHANGE MODE", 50, 250, 200, 40 },
    { "CHANGE NAME(S)", 300, 80, 200, 40 },
    { "BACK", 50, 320, 200, 40 },
    { "LET'S PLAY", 460, 320, 200, 40 }
};

static ButtonSet GSetBtnSet = {
    GSetBtns, 8, 0
};

//CÍMET ÍR A TETEJÉRE, EGYÉB STATIKUS labelOK:
static void showLabels(WindowSpecs* wspecs);

//KIÍRJA A MODE VÁLTOZÁSÁT:
static void showMode(WindowSpecs* wspecs, bool isChallenge);

//KIÍRJA A GRID VÁLTOZÁSÁT:
static void showGridXY(WindowSpecs* wspecs, int GRID_X, int GRID_Y);

//LERAJZOLJA A HÁROMSZÖGEKET A NYILAKRA:
static void showArrows(WindowSpecs* wspecs, ButtonSet* btnSet);

//KIÍRJA A MEGADOTT NEV(EK)ET:
static void showNames(WindowSpecs* wspecs, char* newName1, char* newName2);

//KEZDETI MEGJELENÉS KIRAJZOLÁSA:
static void initStartingLook(WindowSpecs* wspecs);

//SDL COMPONENSEKET INICIALIZÁLÓ FÜGGVÉNY:
static void sdlInit(char const *label, WindowSpecs* wspecs);

//SDL COMPONENSEKET TÖRLÕ FÜGGVÉNY:
static void sdlClose(WindowSpecs* wspecs);

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
void openGameSettings(GameSettings gSet) {
    WindowSpecs wspecs;
    wspecs.gSet = &gSet;

    //KEZDETI MEGJELENÉS:
    initStartingLook(&wspecs);

    SDL_Event ev;
    bool quit = false;
    bool startGame = false;
    int clickNum = 0;

    while (!quit && !startGame) {
        SDL_WaitEvent(&ev);

        switch (ev.type) {
            //MEGNÉZI, HOVA KATTINTUNK:
            case SDL_MOUSEBUTTONDOWN:
                clickNum = getButtonClicked(&ev, &GSetBtnSet);
                switch (clickNum) {
                    case 0:
                        if (gSet.GRID_X < 45)
                            gSet.GRID_X++;
                        showGridXY(&wspecs, gSet.GRID_X, 0);
                        break;
                    case 1:
                        if (gSet.GRID_X > 10)
                            gSet.GRID_X--;
                        showGridXY(&wspecs, gSet.GRID_X, 0);
                        break;
                    case 2:
                        if (gSet.GRID_Y < 30)
                            gSet.GRID_Y++;
                        showGridXY(&wspecs, 0, gSet.GRID_Y);
                        break;
                    case 3:
                        if (gSet.GRID_Y > 10 || (!gSet.isMulti && gSet.GRID_Y > 8))
                            gSet.GRID_Y--;
                        showGridXY(&wspecs, 0, gSet.GRID_Y);
                        break;
                    case 4:
                        gSet.isChallenge = !gSet.isChallenge;
                        showMode(&wspecs, gSet.isChallenge);
                        break;
                    case 5:
                        gSet = openEnterName(gSet);
                        showNames(&wspecs, gSet.playerName1, gSet.playerName2);
                        break;
                    case 6:
                        quit = true;
                        break;
                    case 7:
                        startGame = true;
                    break;
                    default:;
                }
            break;

            //FENTI X:
            case SDL_QUIT:
                quit = true;
                break;

            default:;
        }
    }

    sdlClose(&wspecs);
    if (startGame) {
        openGame(gSet);
    }
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------

//KEZDETI MEGJELENÉS KIRAJZOLÁSA:
static void initStartingLook(WindowSpecs* wspecs) {
    //SDL COMPONENTS:
    wspecs->w = 700; //WIDTH
    wspecs->h = 400; //HEIGHT
    sdlInit("LIL'SNEK 2019 - SETTING UP GAME", wspecs);

    //HÁTTÉR, GOMBOK:
    boxRGBA(wspecs->renderer, 0, 0, wspecs->w, wspecs->h, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);
    showButtons(wspecs, &GSetBtnSet);
    showArrows(wspecs, &GSetBtnSet);
    showLabels(wspecs);
    showNames(wspecs, wspecs->gSet->playerName1, wspecs->gSet->playerName2);

    //KÉP BEILLESZTÉSE:
    SDL_Texture *wallpaper = IMG_LoadTexture(wspecs->renderer, "./data/png/gamesettings.png");
    if (wallpaper == NULL) {
        SDL_Log("Nem nyithato meg a kepfajl: %s", IMG_GetError());
        exit(1);
    }

    int pic_width = 60;
    int pic_height = 90;
    SDL_Rect dest = { wspecs->w - pic_width - 50, wspecs->h - pic_height - 100, pic_width, pic_height };
    SDL_RenderCopy(wspecs->renderer, wallpaper, NULL, &dest);

    //MEGJELENÍTÉS:
    SDL_RenderPresent(wspecs->renderer);

    //BG MUSIC:
    Mix_PlayMusic(wspecs->bgm, -1);

    //KÉPTEXTURE FELSZABADÍTÁSA:
    SDL_DestroyTexture(wallpaper);
}

//KIÍRJA A GRID VÁLTOZÁSÁT:
static void showGridXY(WindowSpecs* wspecs, int GRID_X, int GRID_Y) {
    char str[2+1];
    SDL_Rect target;

    if (GRID_X != 0) {
        target = (SDL_Rect){ 200, 100, 0, 0 };
        sprintf(str, "%2d", GRID_X);
    }
    else {
        target = (SDL_Rect){ 200, 160, 0, 0 };
        sprintf(str, "%2d", GRID_Y);
    }

    //ELŐTÖRLÉS:
    boxRGBA(wspecs->renderer, target.x, target.y, target.x + 20, target.y + 20, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);
    showLabel(wspecs, 14, str, &target);

    //MEGJELENÍTÉS:
    SDL_RenderPresent(wspecs->renderer);
}

//KIÍRJA A MODE VÁLTOZÁSÁT:
static void showMode(WindowSpecs* wspecs, bool isChallenge) {
    SDL_Rect target = { 300, 265, 0, 0 };
    char str[22+1] = "Game Mode: ";
    if (isChallenge)
        strcat(str, "Challenge");
    else
        strcat(str, "Easy");

    //ELŐTÖRLÉS:
    boxRGBA(wspecs->renderer, target.x, target.y, target.x+180, target.y+20, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);
    showLabel(wspecs, 14, str, &target);

    //MEGJELENÍTÉS:
    SDL_RenderPresent(wspecs->renderer);
}

//LERAJZOLJA A HÁROMSZÖGEKET A NYILAKRA:
static void showArrows(WindowSpecs* wspecs, ButtonSet* btnSet) {
    for (int i = 0; i < 4; i++) {
        Sint16 vx[] = {
            btnSet->btns[i].x + 2,
            btnSet->btns[i].x + btnSet->btns[i].w - 2,
            btnSet->btns[i].x + btnSet->btns[i].w / 2
        };
        Sint16 vy[3];
        if (i % 2 == 0) { //FELSŐ GOMBOK
            vy[0] = btnSet->btns[i].y + btnSet->btns[i].h - 2;
            vy[1] = btnSet->btns[i].y + btnSet->btns[i].h - 2;
            vy[2] = btnSet->btns[i].y;
        }
        else { //ALSÓ GOMBOK
            vy[0] = btnSet->btns[i].y + 2;
            vy[1] = btnSet->btns[i].y + 2;
            vy[2] = btnSet->btns[i].y + btnSet->btns[i].h;
        }

        filledPolygonRGBA(wspecs->renderer, vx, vy, 3, red.r, red.g, red.b, red.a);
    }
}

//CÍMET ÍR A TETEJÉRE:
static void showLabels(WindowSpecs* wspecs) {
    SDL_Rect target;

    //CÍM: ENNÉL AZ EGYNÉL KELL A KÖZÉPRE IGAZÍTÁSHOZ A TELJES SHOWLABEL FÜGGVÉNY
    TTF_Init();
    TTF_Font *font = TTF_OpenFont("./data/LUZRO.ttf", 24);
    if (!font) {
        SDL_Log("Nem sikerult megnyitni a fontot: %s\n", TTF_GetError());
        exit(1);
    }

    SDL_Surface *label = TTF_RenderUTF8_Blended(font, "SET UP YOUR GAME", black);
    SDL_Texture *label_t = SDL_CreateTextureFromSurface(wspecs->renderer, label);

    target = (SDL_Rect){ (wspecs->w - label->w) / 2, 20, label->w, label->h };

    SDL_RenderCopy(wspecs->renderer, label_t, NULL, &target);
    TTF_CloseFont(font);

    //EGYÉB labelOK:
    showMode(wspecs, wspecs->gSet->isChallenge);
    showGridXY(wspecs, wspecs->gSet->GRID_X, 0);
    showGridXY(wspecs, 0, wspecs->gSet->GRID_Y);

    target = (SDL_Rect){ 50, 100, 0, 0 };
    showLabel(wspecs, 14, "Grids horizontally:", &target);

    target = (SDL_Rect){ 50, 160, 0, 0 };
    showLabel(wspecs, 14, "Grids vertically:", &target);
}

//KIÍRJA A MEGADOTT NEV(EK)ET:
static void showNames(WindowSpecs* wspecs, char* newName1, char* newName2) {
    SDL_Rect target;

    //PLAYER1:
    target = (SDL_Rect){ 300, 140, 0, 0 };

    //ELŐTÖRLÉS:
    boxRGBA(wspecs->renderer, target.x, target.y, target.x+350, target.y+20, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);

    char str1[30+1] = "Player1: ";
    strcat(str1, newName1);
    showLabel(wspecs, 14, str1, &target);

    //PLAYER2:
    if (wspecs->gSet->isMulti) {
        target = (SDL_Rect){ 300, 170, 0, 0 };

        //ELŐTÖRLÉS:
        boxRGBA(wspecs->renderer, target.x, target.y, target.x+350, target.y+20, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);

        char str2[30+1] = "Player2: ";
        strcat(str2, newName2);
        showLabel(wspecs, 14, str2, &target);
    }

    //MEGJELENÍTÉS:
    SDL_RenderPresent(wspecs->renderer);
}

//INIT ON START
static void sdlInit(char const *label, WindowSpecs* wspecs) {
    //ABLAK, RENDERER, IKON:
    sdlInitGeneral(label, wspecs);

    //HÁTTÉRZENE BETÖLTÉSE:
    Mix_Music* bgm = Mix_LoadMUS("./data/music/menu.mp3");
    if (bgm == NULL) {
        SDL_Log("Zene betoltesi hiba: %s", Mix_GetError());
    }

    //ÁTADÁS:
    wspecs->bgm = bgm;
}

//CLOSE ON EXIT
static void sdlClose(WindowSpecs* wspecs) {
    Mix_HaltMusic();
    sdlCloseGeneral(wspecs);
}
