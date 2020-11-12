#include "debugmalloc.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "gameover.h"
#include "leaderboard.h"

//MODUL GLOBÁLIS GOMBHALMAZA:
static Button GOverBtns[] = {
    { "MAIN MENU", 50, 300, 200, 40 },
    { "LEADERBOARD", 350, 300, 200, 40 }
};

static ButtonSet GOverBtnSet = {
    GOverBtns, 2, 0
};

//SDL COMPONENSEKET INICIALIZÁLÓ FÜGGVÉNY:
static void sdlInit(char const *label, WindowSpecs* wspecs);

//KIMUTATJA A PONTSZÁMOKAT:
static void showFinalPoint(WindowSpecs* wspecs, int playerNum);

//CÍMET ÍR A TETEJÉRE:
static void showTitle(WindowSpecs* wspecs);

//SDL COMPONENSEKET TÖRLÕ FÜGGVÉNY:
static void sdlClose(WindowSpecs* wspecs);

//KEZDETI MEGJELENÉS KIRAJZOLÁSA:
static void initStartingLook(WindowSpecs* wspecs, bool isSaved, bool canPlayMusic);

//ELMENTI A JÁTÉKMENETET A FÁJLBA:
static bool saveGame(GameSettings* gSet);

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
void openGameOver(GameSettings gSet) {
    WindowSpecs wspecs;
    wspecs.gSet = &gSet;

    //EREDMÉNYEK MENTÉSE FÁJLBA APPENDELÉSSEL: (WIP)
    bool isSaved = saveGame(&gSet);

    //KEZDETI MEGJELENÉS:
    initStartingLook(&wspecs, isSaved, gSet.canPlayMusic);


    LeaderSettings lSet;
    lSet.isChallenge = gSet.isChallenge;
    lSet.isMulti = gSet.isMulti;

    SDL_Event ev;
    bool quit = false;
    bool canOpen = false;

    while (!quit) {
        SDL_WaitEvent(&ev);

        switch (ev.type) {
            //MEGNÉZI, HOVA KATTINTUNK:
            case SDL_MOUSEBUTTONDOWN:
                switch (getButtonClicked(&ev, &GOverBtnSet)) {
                    case 0: //MAIN MENU
                        quit = true;
                        break;
                    case 1: //LEADERBOARD
                        quit = true;
                        canOpen = true;
                        break;
                }
                break;

            //FENTI X:
            case SDL_QUIT:
                quit = true;
                break;
        }
    }

    sdlClose(&wspecs);
    if (canOpen)
        openLeaderBoard(lSet, gSet.canPlayMusic);
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------

//ELMENTI A JÁTÉKMENETET A FÁJLBA, VISSZAADJA, SIKERES-E A MENTÉS:
static bool saveGame(GameSettings* gSet) {
    FILE* fp;
    fp = fopen("./data/gamesaves.txt", "at");
    if (fp == NULL) {
        SDL_Log("GAMEOVER: Fajl megnyitasa sikertelen.");
        return false;
    }

    fprintf(fp, "%d\t%d\t%s\t%.3f",
        (gSet->isMulti)? 1 : 0,
        (gSet->isChallenge)? 1 : 0,
        gSet->playerName1,
        gSet->pts1
    );

    if (gSet->isMulti) {
        fprintf(fp, "\t%s\t%.3f\n",
            gSet->playerName2,
            gSet->pts2
        );
    }
    else {
        fprintf(fp, "\n");
    }

    fclose(fp);

    return true;
}

//KEZDETI MEGJELENÉS KIRAJZOLÁSA:
static void initStartingLook(WindowSpecs* wspecs, bool isSaved, bool canPlayMusic) {
    //SDL COMPONENTS:
    wspecs->w = 600; //WIDTH
    wspecs->h = 400; //HEIGHT
    sdlInit("LIL'SNEK 2019 - GAME OVER", wspecs);

    //HÁTTÉR, GOMBOK:
    boxRGBA(wspecs->renderer, 0, 0, wspecs->w, wspecs->h, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);
    showButtons(wspecs, &GOverBtnSet);
    showTitle(wspecs);

    //KÉP BEILLESZTÉSE:
    SDL_Texture *wallpaper = IMG_LoadTexture(wspecs->renderer, "./data/png/gameover.png");
    if (wallpaper == NULL) {
        SDL_Log("Nem nyithato meg a kepfajl: %s", IMG_GetError());
        exit(1);
    }

    int pic_width = 140;
    int pic_height = 155;
    SDL_Rect dest = { wspecs->w - pic_width - 20, wspecs->h - pic_height - 150, pic_width, pic_height };
    SDL_RenderCopy(wspecs->renderer, wallpaper, NULL, &dest);

    //EREDMÉNY:
    showFinalPoint(wspecs, 1);
    if (wspecs->gSet->isMulti)
        showFinalPoint(wspecs, 2);

    //HIBAJELZÉS:
    if (!isSaved) {
        SDL_Rect target = { 50, 250, 0, 0 };
        showLabel(wspecs, 14, "WARNING: Game failed to save. Check save file.", &target);
    }
    else {
        SDL_Rect target = { 50, 250, 0, 0 };
        showLabel(wspecs, 14, "NOTICE: Game saved successfully.", &target);
    }

    //GAMEOVER HANGEFFEKT LEJÁTSZÁSA:
    if (canPlayMusic) {
        Mix_Chunk* fx = Mix_LoadWAV("./data/music/gameover.wav");
        if (fx == NULL) {
            SDL_Log("SFX betoltesi hiba: %s", Mix_GetError());
        }
        Mix_PlayChannel(-1, fx, 0);
    }

    //KÉPTEXTURE FELSZABADÍTÁSA:
    SDL_DestroyTexture(wallpaper);

    //MEGJELENÍTÉS:
    SDL_RenderPresent(wspecs->renderer);
}

//CÍMET ÍR A TETEJÉRE:
static void showTitle(WindowSpecs* wspecs) {
    TTF_Font *font;
    SDL_Surface *label;
    SDL_Texture *label_t;
    SDL_Rect hova;

    //GAMEOVER:
    font = TTF_OpenFont("./data/LUZRO.ttf", 24);
    if (!font) {
        SDL_Log("Nem sikerult megnyitni a fontot: %s\n", TTF_GetError());
        exit(1);
    }

    label = TTF_RenderUTF8_Blended(font, "GAME OVER", black);
    label_t = SDL_CreateTextureFromSurface(wspecs->renderer, label);

    hova = (SDL_Rect){ (wspecs->w - label->w) / 2, 20, label->w, label->h };
    SDL_RenderCopy(wspecs->renderer, label_t, NULL, &hova);

    SDL_FreeSurface(label);
    SDL_DestroyTexture(label_t);

    TTF_CloseFont(font);

    //FINAL SCORE(S):
    font = TTF_OpenFont("./data/LUZRO.ttf", 14);
    if (!font) {
        SDL_Log("Nem sikerult megnyitni a fontot: %s\n", TTF_GetError());
        exit(1);
    }

    label = TTF_RenderUTF8_Blended(font, "FINAL SCORE(S):", black);
    label_t = SDL_CreateTextureFromSurface(wspecs->renderer, label);

    hova = (SDL_Rect){ (wspecs->w - label->w) / 2, 50, label->w, label->h };
    SDL_RenderCopy(wspecs->renderer, label_t, NULL, &hova);

    SDL_FreeSurface(label);
    SDL_DestroyTexture(label_t);

    TTF_CloseFont(font);
}

//KIMUTATJA A PONTSZÁMOKAT:
static void showFinalPoint(WindowSpecs* wspecs, int playerNum) {
    char str[36+1] = "";
    if (playerNum == 1)
        sprintf(str, "%s: %g pts", wspecs->gSet->playerName1, wspecs->gSet->pts1);
    else
        sprintf(str, "%s: %g pts", wspecs->gSet->playerName2, wspecs->gSet->pts2);

    SDL_Rect target = { 50, 100+20*playerNum, 0, 0 };
    showLabel(wspecs, 14, str, &target);
}

//INIT ON START
static void sdlInit(char const *label, WindowSpecs* wspecs) {
    //ABLAK, RENDERER, IKON:
    sdlInitGeneral(label, wspecs);
}

//CLOSE ON EXIT
static void sdlClose(WindowSpecs* wspecs) {
    sdlCloseGeneral(wspecs);
}
