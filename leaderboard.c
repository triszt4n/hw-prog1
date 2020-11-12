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
#include "leaderboard.h"

//MODUL GLOBÁLIS GOMBHALMAZAI:
static Button leaderBtns[] = {
    { "MAIN MENU", 50, 520, 200, 40 },
    {"<<", 400, 520, 60, 40 },
    {">>", 490, 520, 60, 40 }
};

static ButtonSet pagingBtnSet = {
    leaderBtns+1, 2, 0
};

static ButtonSet leaderBtnSet = {
    leaderBtns, 3, 0
};

//DOBOGÓS SZÍNEK:
static SDL_Color colors[] = {
    { 0xfc, 0xc2, 0x01, 0xff },
    { 0xbf, 0xba, 0xba, 0xff },
    { 0xcd, 0x7f, 0x32, 0xff }
};

//LÁNCOLT LISTA AZ EREDMÉNYEK TÁROLÁSÁRA:
typedef struct Result {
    char playerName[20+1];
    double pts;
    struct Result* next;
} Result;

//SDL COMPONENSEKET INICIALIZÁLÓ FÜGGVÉNY:
static void sdlInit(char const *label, WindowSpecs* wspecs);

//SDL COMPONENSEKET TÖRLÕ FÜGGVÉNY:
static void sdlClose(WindowSpecs* wspecs);

//MEGJELENÉST INICIALIZÁLÓ FÜGGVÉNY:
static void initStartingLook(WindowSpecs* wspecs, LeaderSettings* lSet, bool canPlayMusic, Result** pRes, int* maxPage);

//KIMUTATJA AZ EREDMÉNYEKET:
static void showResults(WindowSpecs* wspecs, Result* res, int pageNum);

//KIRAJZOLJA AZ ÉPPEN SZÜKSÉGES LAPOZÓ GOMBOKAT:
static void showPagingButtons(WindowSpecs* wspecs, int pageNum, int maxPage);

//EREDMÉNYEKET BEOLVASÓ FÜGGVÉNY:
static void loadFile(LeaderSettings* lSet, Result** pRes, int* resN);

//EREDMÉNYT RENDEZVE ILLESZTI AZ EREDMÉNYEK LÁNCOLT LISTÁJÁBA:
static void insertResult(Result** pRes, char* playerName, double pts);

//FELSZABADÍTJA A VÉGÉN A TÁROLT EREDMÉNYEK LÁNCOLT LISTÁJÁT:
static void freeResult(Result* res);

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
void openLeaderBoard(LeaderSettings lSet, bool canPlayMusic) {
    WindowSpecs wspecs;

    //KEZDETI MEGJELENÉS:
    int currentPage = 0;
    int maxPage = 0;

    Result* res = NULL;
    initStartingLook(&wspecs, &lSet, canPlayMusic, &res, &maxPage);

    SDL_Event ev;
    bool quit = false;

    while (!quit) {
        SDL_WaitEvent(&ev);

        switch (ev.type) {
            //MEGNÉZI, HOVA KATTINTUNK:
            case SDL_MOUSEBUTTONDOWN:
                switch (getButtonClicked(&ev, &leaderBtnSet)) {
                    case 0: //MAIN MENU
                        quit = true;
                        break;
                    case 1: //<<
                        if (currentPage != 0)
                            currentPage--;

                        showPagingButtons(&wspecs, currentPage, maxPage);
                        showResults(&wspecs, res, currentPage);
                        break;
                    case 2: //>>
                        if (currentPage < (maxPage - 1))
                            currentPage++;

                        showPagingButtons(&wspecs, currentPage, maxPage);
                        showResults(&wspecs, res, currentPage);
                        break;
                }
                break;

            //FENTI X:
            case SDL_QUIT:
                quit = true;
                break;
        }
    }

    freeResult(res);
    sdlClose(&wspecs);
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------

//EREDMÉNYEKET BEOLVASÓ FÜGGVÉNY:
static void loadFile(LeaderSettings* lSet, Result** pRes, int* resN) {
    FILE* fp;
    fp = fopen("./data/gamesaves.txt", "rt");
    if (fp == NULL) {
        SDL_Log("GAMEOVER: Fajl megnyitasa sikertelen.");
        *pRes = NULL;
        *resN = 0;
        return;
    }

    int int_isMulti = 0, int_isChallenge = 0;
    char playerName1[20+1];
    char playerName2[20+1];
    double pts1 = 0, pts2 = 0;

    //BUFFERBE TESZI A SORT:
    char buffer[100+1];
    //SOROK BEOLVASÁSA (HASZNOS SORRAL MUNKA):
    while (fgets(buffer, 100, fp) != NULL) {
        int_isMulti = atoi(buffer);
        int_isChallenge = atoi(buffer+2);

        if (((int_isMulti == 1 && lSet->isMulti) || (int_isMulti == 0 && !lSet->isMulti)) &&
            ((int_isChallenge == 1 && lSet->isChallenge) || (int_isChallenge == 0 && !lSet->isChallenge))) {
            //BEÁLLÍTÁSOK ALAPJÁN ADATOK KIOLVASÁSA:
            if (int_isMulti == 1) {
                //ELSŐ 4 KARAKTER UTÁN:
                sscanf(buffer, "%*c%*c%*c%*c%[^\t]\t%lf\t%[^\t]\t%lf\n", playerName1, &pts1, playerName2, &pts2);

                //HOZZÁFŰZÉS:
                insertResult(pRes, playerName1, pts1);
                insertResult(pRes, playerName2, pts2);
                *resN += 2;
            }
            else {
                //ELSŐ 4 KARAKTER UTÁN:
                sscanf(buffer, "%*c%*c%*c%*c%[^\t]\t%lf\n", playerName1, &pts1);

                //HOZZÁFŰZÉS:
                insertResult(pRes, playerName1, pts1);
                *resN += 1;
            }
        }
    }

    fclose(fp);
}

//EREDMÉNYT RENDEZVE ILLESZTI AZ EREDMÉNYEK LÁNCOLT LISTÁJÁBA:
static void insertResult(Result** pRes, char* playerName, double pts) {
    //ÚJELEM:
    Result* newRes = (Result*) malloc(sizeof(Result));
    strcpy(newRes->playerName, playerName);
    newRes->pts = pts;

    Result* res = *pRes;
    //BEFŰZÉS:
    if (res == NULL || pts >= res->pts) { //ÜRES LISTA VAGY ELEJÉRE BESZÚRÁS:
        newRes->next = res;
        res = newRes;
    }
    else { //KÖZEPÉBE SZÚRÁS:
        Result *iter;
        for (iter = res; iter->next != NULL && iter->next->pts > pts; iter = iter->next);
        newRes->next = iter->next;
        iter->next = newRes;
    }

    *pRes = res;
}

//FELSZABADÍTJA A VÉGÉN A TÁROLT EREDMÉNYEK LÁNCOLT LISTÁJÁT:
static void freeResult(Result* res) {
    Result *iter = res;
    Result *inchworm = NULL;
    while (iter != NULL) {
        inchworm = iter;
        iter = iter->next;
        free(inchworm);
    }
}

//KIRAJZOLJA AZ ÉPPEN SZÜKSÉGES LAPOZÓ GOMBOKAT:
static void showPagingButtons(WindowSpecs* wspecs, int pageNum, int maxPage) {
    //EGYOLDALAS ESET KEZELÉSE:
    if (maxPage <= 1) return;

    //ELŐTÖRLÉS:
    boxRGBA(wspecs->renderer, leaderBtns[1].x, leaderBtns[1].y, leaderBtns[1].x + 150, leaderBtns[1].y + 40, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);

    //TOVÁBBI ESETEK:
    if (pageNum == 0) { //1. OLDALON JÁRVA:
        ButtonSet customBtnSet = {
            &leaderBtns[2], 1, 0
        };
        showButtons(wspecs, &customBtnSet);
    }
    else if (pageNum == maxPage - 1) { //UTOLSÓ OLDALON JÁRVA:
        ButtonSet customBtnSet = {
            &leaderBtns[1], 1, 0
        };
        showButtons(wspecs, &customBtnSet);
    }
    else { //EGYÉB ESET:
        showButtons(wspecs, &pagingBtnSet);
    }

    //MEGJELENÍTÉS:
    SDL_RenderPresent(wspecs->renderer);
}

//KIMUTATJA AZ EREDMÉNYEKET:
static void showResults(WindowSpecs* wspecs, Result* res, int pageNum) {
    SDL_Rect target;

    //ÜRES LISTA:
    if (res == NULL) {
        target = (SDL_Rect){ 50, 100, 0, 0 };
        showLabel(wspecs, 14, "No result to show.", &target);
        return;
    }

    //ELŐTÖRLÉS:
    boxRGBA(wspecs->renderer, 0, 80, 600, 400, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);

    //KEZDÉSI HELY MEGKERESÉSE:
    Result* iter = res;
    int j;
    for (iter = res, j = 0; j < pageNum*10; j++, iter = iter->next);

    //KIÍRJA A 10-ET:
    for (int i = 0; iter != NULL && i < 10; iter = iter->next, i++) {
        //RANK:
        int rank = pageNum*10 + i + 1;
        if (rank <= 3)
            boxRGBA(wspecs->renderer, 0, 95 + 30 * i, 600, 120 + 30 * i, colors[rank-1].r, colors[rank-1].g, colors[rank-1].b, colors[rank-1].a);

        target = (SDL_Rect){ 50, 100 + 30 * i, 0, 0 };
        char* str1 = (char*) malloc(sizeof(char) * ((int)(log10(rank)) + 3)); //+3: LSDigit, #, \0
        sprintf(str1, "#%d", rank);
        showLabel(wspecs, 14, str1, &target);
        free(str1);

        //NAME:
        target = (SDL_Rect){ 130, 100 + 30 * i, 0, 0 };
        showLabel(wspecs, 14, iter->playerName, &target);

        //PTS:
        target = (SDL_Rect){ 450, 100 + 30 * i, 0, 0 };
        char* str2 = (char*) malloc(sizeof(char) * ((iter->pts > 0) ? ((int)(log10(iter->pts)) + 9) : 9) + 1); //+9: 0.000_pts, +1: \0
        sprintf(str2, "%.3f pts", iter->pts);
        showLabel(wspecs, 14, str2, &target);
        free(str2);
    }

    //MEGJELENÍTÉS:
    SDL_RenderPresent(wspecs->renderer);
}

//MEGJELENÉST INICIALIZÁLÓ FÜGGVÉNY:
static void initStartingLook(WindowSpecs* wspecs, LeaderSettings* lSet, bool canPlayMusic, Result** pRes, int* maxPage) {
    wspecs->w = 600;
    wspecs->h = 600;
    sdlInit("LIL'SNEK GAME 2019 - LEADERBOARD", wspecs);
    boxRGBA(wspecs->renderer, 0, 0, wspecs->w, wspecs->h, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);

    //BG MUSIC:
    if (canPlayMusic)
        Mix_PlayMusic(wspecs->bgm, -1);

    //CÍM:
    TTF_Font* font = TTF_OpenFont("./data/LUZRO.ttf", 24);
    if (!font) {
        SDL_Log("Nem sikerult megnyitni a fontot: %s\n", TTF_GetError());
        exit(1);
    }

    SDL_Surface *label = TTF_RenderUTF8_Blended(font, "HALL OF FAME", black);
    SDL_Texture *label_t = SDL_CreateTextureFromSurface(wspecs->renderer, label);

    SDL_Rect target = { (wspecs->w - label->w) / 2, 20, label->w, label->h };
    SDL_RenderCopy(wspecs->renderer, label_t, NULL, &target);

    SDL_FreeSurface(label);
    SDL_DestroyTexture(label_t);

    TTF_CloseFont(font);

    //ALCÍM:
    font = TTF_OpenFont("./data/LUZRO.ttf", 14);
    if (!font) {
        SDL_Log("Nem sikerult megnyitni a fontot: %s\n", TTF_GetError());
        exit(1);
    }

    char* str = (char*) malloc(sizeof(char) * (((lSet->isMulti) ? 11 : 13 ) + ((lSet->isChallenge) ? 14 : 9 ) + 3)); //SINGLE_PLAYER,_CHALLENGE_MODE\0
    strcpy(str, (lSet->isMulti) ? "MULTIPLAYER, " : "SINGLE PLAYER, ");
    strcat(str, (lSet->isChallenge) ? "CHALLENGE MODE" : "EASY MODE");

    label = TTF_RenderUTF8_Blended(font, str, black);
    label_t = SDL_CreateTextureFromSurface(wspecs->renderer, label);

    target = (SDL_Rect){ (wspecs->w - label->w) / 2, 50, label->w, label->h };
    SDL_RenderCopy(wspecs->renderer, label_t, NULL, &target);

    SDL_FreeSurface(label);
    SDL_DestroyTexture(label_t);
    free(str);

    TTF_CloseFont(font);

    //KÉP BEILLESZTÉSE:
    SDL_Texture *wallpaper = IMG_LoadTexture(wspecs->renderer, "./data/png/leaderboard.png");
    if (wallpaper == NULL) {
        SDL_Log("Nem nyithato meg a kepfajl: %s", IMG_GetError());
        exit(1);
    }

    int pic_width = 500;
    int pic_height = 90;
    SDL_Rect dest = { 50, wspecs->h - pic_height - 100, pic_width, pic_height };
    SDL_RenderCopy(wspecs->renderer, wallpaper, NULL, &dest);

    //KÉPTEXTURE FELSZABADÍTÁSA:
    SDL_DestroyTexture(wallpaper);

    //BETÖLTÉS:
    int resN = 0;
    loadFile(lSet, pRes, &resN);

    //KEZDETI OLDAL MUTATÁSA:
    *maxPage = (int) ceil(resN / 10.0);
    showResults(wspecs, *pRes, 0);

    ButtonSet menuBtnSet = { leaderBtns, 1, 0 };
    showButtons(wspecs, &menuBtnSet);
    showPagingButtons(wspecs, 0, *maxPage);

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
