#include "debugmalloc.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "menu.h"
#include "global.h"
#include "gamesettings.h"
#include "leaderboard.h"

//MODUL GLOBÁLIS GOMBHALMAZAI:
static Button mainBtns[] = {
    { "START", 100, 100, 200, 40 },
    { "LEADERBOARD", 100, 170, 200, 40 },
    { "MUSIC ON/OFF", 100, 240, 200, 40 },
    { "EXIT", 100, 450, 200, 40 }
};

static Button startBtns[] = {
    { "SINGLE PLAYER", 100, 100, 200, 40 },
    { "MULTIPLAYER", 100, 170, 200, 40 },
    { "BACK", 100, 450, 200, 40 }
};

static Button leaderBtns[] = {
    { "SINGLE - EASY", 100, 100, 200, 40 },
    { "SINGLE - CHALLENGE", 100, 170, 200, 40 },
    { "MULTI - EASY", 100, 240, 200, 40 },
    { "MULTI - CHALLENGE", 100, 310, 200, 40 },
    { "BACK", 100, 450, 200, 40 }
};

static ButtonSet mainBtnSet = {
    mainBtns, 4, 0
};

static ButtonSet startBtnSet = {
    startBtns, 3, 1
};

static ButtonSet leaderBtnSet = {
    leaderBtns, 5, 2
};

//SDL COMPONENSEKET INICIALIZÁLÓ FÜGGVÉNY:
static void sdlInit(char const *label, WindowSpecs* wspecs);

//SDL COMPONENSEKET TÖRLŐ FÜGGVÉNY:
static void sdlClose(WindowSpecs* wspecs);

//KEZELI A VISSZATÉRÉST LEADERBOARDBÓL VAGY GAMESETTINGSBŐL:
static void returnHere(WindowSpecs* wspecs, GameSettings* gSet, LeaderSettings* lSet, ButtonSet** currentBtnSet);

//A MENÜ JOBB PANELE MINDIG UGYANAZ:
static void drawRightPanel(WindowSpecs* wspecs);

//ALAPBEÁLLÍTÁSOKRA INICIALIZÁLJA A JÁTÉKBEÁLLÍTÁSOKAT:
static void initGameSettings(GameSettings* gSet);

//ALAPBEÁLLÍTÁSOKRA INICIALIZÁLJA A DICSÖSÉGTÁBLA KIÍRÁSÁNÁL HASZNÁLT BEÁLLÍTÁSOKAT:
static void initLeaderSettings(LeaderSettings* lSet);

//KEZDŐ MEGJELENÍTÉST SZOLGÁLÓ FÜGGVÉNY:
static void initStartingLook(WindowSpecs* wspecs, ButtonSet* currentBtnSet);

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
void openMenu(void) {
    srand(time(0));

    //INICIALIZÁCIÓK:
    WindowSpecs wspecs;
    ButtonSet* currentBtnSet = &mainBtnSet;
    initStartingLook(&wspecs, currentBtnSet);

    GameSettings gSet;
    initGameSettings(&gSet);
    gSet.canPlayMusic = true;
    LeaderSettings lSet;
    initLeaderSettings(&lSet);

    SDL_Event ev;
    bool quit = false;
    int clickNum = 0;
    int screenNum = 0;

    while (!quit) {
        SDL_WaitEvent(&ev);

        switch (ev.type) {
            //MEGNÉZI, HOVA KATTINTUNK:
            case SDL_MOUSEBUTTONDOWN:
                screenNum = currentBtnSet->screenNum;
                clickNum = getButtonClicked(&ev, currentBtnSet);

                switch (screenNum) {
                    case 0:
                        switch (clickNum) {
                            case 0: //START
                                currentBtnSet = &startBtnSet;
                                //ELÕTTE TISZTÍTJUK A PANELT:
                                boxRGBA(wspecs.renderer, 0, 0, wspecs.w / 2, wspecs.h, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);
                                showButtons(&wspecs, currentBtnSet);
                                break;
                            case 1: //LEADERBOARD
                                currentBtnSet = &leaderBtnSet;
                                //ELÕTTE TISZTÍTJUK A PANELT:
                                boxRGBA(wspecs.renderer, 0, 0, wspecs.w / 2, wspecs.h, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);
                                showButtons(&wspecs, currentBtnSet);
                                break;
                            case 2: //MUSIC
                                if (gSet.canPlayMusic) {
                                    gSet.canPlayMusic = false;
                                    Mix_PauseMusic();
                                }
                                else {
                                    gSet.canPlayMusic = true;
                                    Mix_ResumeMusic();
                                }
                                break;
                            case 3: //EXIT
                                quit = true;
                                break;
                        }
                        break;
                    case 1:
                        switch (clickNum) {
                            case 0: //SINGLE
                                sdlClose(&wspecs);
                                openGameSettings(gSet);
                                //VISSZATÉRÉSKOR:
                                returnHere(&wspecs, &gSet, &lSet, &currentBtnSet);
                                break;
                            case 1: //MULTI
                                gSet.isMulti = true;
                                sdlClose(&wspecs);
                                openGameSettings(gSet);
                                //VISSZATÉRÉSKOR:
                                returnHere(&wspecs, &gSet, &lSet, &currentBtnSet);
                                break;
                            case 2: //BACK
                                currentBtnSet = &mainBtnSet;
                                //ELÕTTE TISZTÍTJUK A PANELT:
                                boxRGBA(wspecs.renderer, 0, 0, wspecs.w / 2, wspecs.h, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);
                                showButtons(&wspecs, currentBtnSet);
                                break;
                        }
                        break;
                    case 2:
                        switch (clickNum) {
                            case 0: //SINGLE - EASY
                                sdlClose(&wspecs);
                                openLeaderBoard(lSet, gSet.canPlayMusic);
                                //VISSZATÉRÉSKOR:
                                returnHere(&wspecs, &gSet, &lSet, &currentBtnSet);
                                break;
                            case 1: //SINGLE - CHALLENGE
                                lSet.isChallenge = true;
                                sdlClose(&wspecs);
                                openLeaderBoard(lSet, gSet.canPlayMusic);
                                //VISSZATÉRÉSKOR:
                                returnHere(&wspecs, &gSet, &lSet, &currentBtnSet);
                                break;
                            case 2: //MULTI - EASY
                                lSet.isMulti = true;
                                sdlClose(&wspecs);
                                openLeaderBoard(lSet, gSet.canPlayMusic);
                                //VISSZATÉRÉSKOR:
                                returnHere(&wspecs, &gSet, &lSet, &currentBtnSet);
                                break;
                            case 3: //MULTI - CHALLENGE
                                lSet.isChallenge = true;
                                lSet.isMulti = true;
                                sdlClose(&wspecs);
                                openLeaderBoard(lSet, gSet.canPlayMusic);
                                //VISSZATÉRÉSKOR:
                                returnHere(&wspecs, &gSet, &lSet, &currentBtnSet);
                                break;
                            case 4: //BACK
                                currentBtnSet = &mainBtnSet;
                                //ELÕTTE TISZTÍTJUK A PANELT:
                                boxRGBA(wspecs.renderer, 0, 0, wspecs.w / 2, wspecs.h, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);
                                showButtons(&wspecs, currentBtnSet);
                                break;
                        }
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
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------

//KEZELI A VISSZATÉRÉST LEADERBOARDBÓL VAGY GAMESETTINGSBŐL:
static void returnHere(WindowSpecs* wspecs, GameSettings* gSet, LeaderSettings* lSet, ButtonSet** currentBtnSet) {
    *currentBtnSet = &mainBtnSet;
    initStartingLook(wspecs, *currentBtnSet);
    initGameSettings(gSet);
    initLeaderSettings(lSet);
}

//A MENÜ JOBB PANELE MINDIG UGYANAZ, EZT RAJZOLJA KI A FÜGGVÉNY:
static void drawRightPanel(WindowSpecs* wspecs) {
    //WALLPAPER BETÖLTÉS:
    SDL_Texture *wallpaper = IMG_LoadTexture(wspecs->renderer, "./data/png/main.png");
    if (wallpaper == NULL) {
        SDL_Log("Nem nyithato meg a kepfajl: %s", IMG_GetError());
        exit(1);
    }

    //HÁTTÉR:
    boxRGBA(wspecs->renderer, 0, 0, wspecs->w, wspecs->h, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);

    //KÉP BEILLESZTÉSE RENDERERBE
    int pic_width = 300;
    int pic_height = 350;
    SDL_Rect dest = { wspecs->w - pic_width - 50, wspecs->h - pic_height - 50, pic_width, pic_height };
    SDL_RenderCopy(wspecs->renderer, wallpaper, NULL, &dest);

    TTF_Font* font;
    SDL_Rect target;
    SDL_Surface *label;
    SDL_Texture *label_t;

    //CÍM:
    font = TTF_OpenFont("./data/LUZRO.ttf", 32);
    if (!font) {
        SDL_Log("Nem sikerult megnyitni a fontot: %s\n", TTF_GetError());
        exit(1);
    }

    label = TTF_RenderUTF8_Blended(font, "Lil'Snek Game", black);
    label_t = SDL_CreateTextureFromSurface(wspecs->renderer, label);

    target = (SDL_Rect){ wspecs->w - 350 + (350 - label->w) / 2 - 50, 50, label->w, label->h };
    SDL_RenderCopy(wspecs->renderer, label_t, NULL, &target);

    TTF_CloseFont(font);

    //ALCÍM:
    font = TTF_OpenFont("./data/LUZRO.ttf", 12);
    if (!font) {
        SDL_Log("Nem sikerult megnyitni a fontot: %s\n", TTF_GetError());
        exit(1);
    }

    label = TTF_RenderUTF8_Blended(font, "Pixelart, music and programming by Trisztan Piller", black);
    label_t = SDL_CreateTextureFromSurface(wspecs->renderer, label);

    target = (SDL_Rect){ wspecs->w - 350 + (350 - label->w) / 2 - 50, 90, label->w, label->h };
    SDL_RenderCopy(wspecs->renderer, label_t, NULL, &target);

    SDL_FreeSurface(label);
    SDL_DestroyTexture(label_t);

    TTF_CloseFont(font);

    //MEMÓRIA FELSZABADÍTÁSA
    SDL_DestroyTexture(wallpaper);

    //MEGJELENÍTÉS:
    SDL_RenderPresent(wspecs->renderer);
}

//ALAPBEÁLLÍTÁSOKRA INICIALIZÁLJA A JÁTÉKBEÁLLÍTÁSOKAT:
static void initGameSettings(GameSettings* gSet) {
    gSet->GRID_X = 17;
    gSet->GRID_Y = 15;
    gSet->isChallenge = false;
    gSet->isMulti = false;
    char str1[10+1] = "GUEST_xxxx";
    char str2[10+1] = "GUEST_xxxx";
    int r;
    for (int i = 6; i < 10; i++) {
        r = rand() % 26;
        str1[i] = (char)(r + 'A');
        r = rand() % 26;
        str2[i] = (char)(r + 'A');
    }
    strcpy(gSet->playerName1, str1);
    strcpy(gSet->playerName2, str2);
    gSet->pts1 = 0;
    gSet->pts2 = 0;
}

//ALAPBEÁLLÍTÁSOKRA INICIALIZÁLJA A DICSÖSÉGTÁBLA KIÍRÁSÁNÁL HASZNÁLT BEÁLLÍTÁSOKAT:
static void initLeaderSettings(LeaderSettings* lSet) {
    lSet->isChallenge = false;
    lSet->isMulti = false;
}

//KEZDŐ MEGJELENÍTÉST SZOLGÁLÓ FÜGGVÉNY:
static void initStartingLook(WindowSpecs* wspecs, ButtonSet* currentBtnSet) {
    //SDL COMPONENTS:
    wspecs->w = 800; //WIDTH
    wspecs->h = 600; //HEIGHT
    sdlInit("LIL'SNEK 2019 - MAIN MENU", wspecs);

    //MAIN MENU:
    drawRightPanel(wspecs);
    showButtons(wspecs, currentBtnSet);

    //HÁTTÉRZENE ELINDÍTÁSA:
    Mix_PlayMusic(wspecs->bgm, -1);
}

//INIT ON START:
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

//CLOSE ON EXIT:
static void sdlClose(WindowSpecs* wspecs) {
    Mix_HaltMusic();
    sdlCloseGeneral(wspecs);
}
