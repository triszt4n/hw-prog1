#include "debugmalloc.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "global.h"
#include "entername.h"

//SDL COMPONENSEKET INICIALIZÁLÓ FÜGGVÉNY:
static void sdlInit(char const *label, WindowSpecs* wspecs);

//SDL COMPONENSEKET TÖRLÕ FÜGGVÉNY:
static void sdlClose(WindowSpecs* wspecs);

//MÁSOLVA INFOC-PORTÁLRÓL:
static bool inputText(WindowSpecs* wspecs, char *dest, size_t hossz, SDL_Rect teglalap, SDL_Color hatter, SDL_Color szoveg);

//MEGJELENÍTI AZ ELSŐ PLAYERHEZ AZ INPUT BOXOKAT:
static void enterName1(WindowSpecs* wspecs);

//MEGJELENÍTI A MÁSODIK JÁTÉKOS NEVÉHEZ AZ INPUT BOXOKAT:
static void enterName2(WindowSpecs* wspecs);

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
GameSettings openEnterName(GameSettings gSet) {
    WindowSpecs wspecs;
    wspecs.gSet = &gSet;
    wspecs.h = 300;
    wspecs.w = 500;

    sdlInit("FOLLOW THE INSTRUCTIONS!", &wspecs);

    enterName1(&wspecs);

    if (gSet.isMulti)
        enterName2(&wspecs);

    sdlClose(&wspecs);
    return gSet;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------

//MÁSOLVA INFOC-PORTÁLRÓL:
static bool inputText(WindowSpecs* wspecs, char *dest, size_t hossz, SDL_Rect teglalap, SDL_Color hatter, SDL_Color szoveg) {
    TTF_Font *font = TTF_OpenFont("./data/LUZRO.ttf", 24);
    if (!font) {
        SDL_Log("Nem sikerult megnyitni a fontot: %s\n", TTF_GetError());
        exit(1);
    }

    /* Ez tartalmazza az aktualis szerkesztest */
    char composition[SDL_TEXTEDITINGEVENT_TEXT_SIZE];
    composition[0] = '\0';
    /* Ezt a kirajzolas kozben hasznaljuk */
    char textandcomposition[hossz + SDL_TEXTEDITINGEVENT_TEXT_SIZE + 1];
    /* Max hasznalhato szelesseg */
    int maxw = teglalap.w - 2;
    int maxh = teglalap.h - 2;

    dest[0] = '\0';

    bool enter = false;
    bool kilep = false;

    SDL_StartTextInput();
    while (!kilep && !enter) {
        /* doboz kirajzolasa */
        boxRGBA(wspecs->renderer, teglalap.x, teglalap.y, teglalap.x + teglalap.w - 1, teglalap.y + teglalap.h - 1, hatter.r, hatter.g, hatter.b, 255);
        rectangleRGBA(wspecs->renderer, teglalap.x, teglalap.y, teglalap.x + teglalap.w - 1, teglalap.y + teglalap.h - 1, szoveg.r, szoveg.g, szoveg.b, 255);
        /* szoveg kirajzolasa */
        int w;
        strcpy(textandcomposition, dest);
        strcat(textandcomposition, composition);
        if (textandcomposition[0] != '\0') {
            SDL_Surface *label = TTF_RenderUTF8_Blended(font, textandcomposition, szoveg);
            SDL_Texture *label_t = SDL_CreateTextureFromSurface(wspecs->renderer, label);
            SDL_Rect cel = { teglalap.x, teglalap.y, label->w < maxw ? label->w : maxw, label->h < maxh ? label->h : maxh };
            SDL_RenderCopy(wspecs->renderer, label_t, NULL, &cel);
            SDL_FreeSurface(label);
            SDL_DestroyTexture(label_t);
            w = cel.w;
        } else {
            w = 0;
        }
        /* kurzor kirajzolasa */
        if (w < maxw) {
            vlineRGBA(wspecs->renderer, teglalap.x + w + 2, teglalap.y + 2, teglalap.y + teglalap.h - 3, szoveg.r, szoveg.g, szoveg.b, 192);
        }
        /* megjeleniti a képernyon az eddig rajzoltakat */
        SDL_RenderPresent(wspecs->renderer);

        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type) {
            /* Kulonleges karakter */
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_BACKSPACE) {
                    int textlen = strlen(dest);
                    do {
                        if (textlen == 0) {
                            break;
                        }
                        if ((dest[textlen-1] & 0x80) == 0x00)   {
                            /* Egy bajt */
                            dest[textlen-1] = 0x00;
                            break;
                        }
                        if ((dest[textlen-1] & 0xC0) == 0x80) {
                            /* Bajt, egy tobb-bajtos szekvenciabol */
                            dest[textlen-1] = 0x00;
                            textlen--;
                        }
                        if ((dest[textlen-1] & 0xC0) == 0xC0) {
                            /* Egy tobb-bajtos szekvencia elso bajtja */
                            dest[textlen-1] = 0x00;
                            break;
                        }
                    } while(true);
                }
                if (event.key.keysym.sym == SDLK_RETURN) {
                    enter = true;
                }
                break;

            /* A feldolgozott szoveg bemenete */
            case SDL_TEXTINPUT:
                if (strlen(dest) + strlen(event.text.text) < hossz) {
                    strcat(dest, event.text.text);
                }

                /* Az eddigi szerkesztes torolheto */
                composition[0] = '\0';
                break;

            /* Szoveg szerkesztese */
            case SDL_TEXTEDITING:
                strcpy(composition, event.edit.text);
                break;
        }
    }

    TTF_CloseFont(font);
    /* igaz jelzi a helyes beolvasast; = ha enter miatt allt meg a ciklus */
    SDL_StopTextInput();
    return enter;
}

//MEGJELENÍTI AZ ELSŐ PLAYERHEZ AZ INPUT BOXOKAT:
static void enterName1(WindowSpecs* wspecs) {
    SDL_Rect target;

    //LABELS:
    target = (SDL_Rect){ 20, 45, 0, 0 };
    showLabel(wspecs, 14, "1. Type name of Player 1:", &target);
    target = (SDL_Rect){ 20, 200, 0, 0 };
    showLabel(wspecs, 14, "2. Press enter to save it.", &target);
    target = (SDL_Rect){ 20, 230, 0, 0 };
    showLabel(wspecs, 14, "(Leave it blank if you do not want to change, and press enter.)", &target);

    //TEXTBOX:
    target = (SDL_Rect){ 20, 80, 460, 30 };
    char str[21] = "";
    inputText(wspecs, str, 21, target, black, white);

    if (str[0] != '\0') {
        strcpy(wspecs->gSet->playerName1, str);
        target = (SDL_Rect){ 20, 150, 0, 0 };
        boxRGBA(wspecs->renderer, target.x, target.y, target.x+400, target.y+20, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);
        showLabel(wspecs, 14, "NOTICE: Successfully entered name of Player 1.", &target);

    }
    else {
        target = (SDL_Rect){ 20, 150, 0, 0 };
        boxRGBA(wspecs->renderer, target.x, target.y, target.x+400, target.y+20, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);
        showLabel(wspecs, 14, "NOTICE: Name of Player 1 left unchanged.", &target);
    }
}

//MEGJELENÍTI A MÁSODIK JÁTÉKOS NEVÉHEZ AZ INPUT BOXOKAT:
static void enterName2(WindowSpecs* wspecs) {
    SDL_Rect target;

    //LABELS:
    target = (SDL_Rect){ 20, 45, 0, 0 };
    boxRGBA(wspecs->renderer, target.x, target.y, target.x+400, target.y+20, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);
    showLabel(wspecs, 14, "1. Now type name of Player 2:", &target);

    //TEXTBOX:
    target = (SDL_Rect){ 20, 80, 460, 30 };
    char str[21] = "";
    inputText(wspecs, str, 21, target, black, white);

    if (str[0] != '\0') {
        strcpy(wspecs->gSet->playerName2, str);
        target = (SDL_Rect){ 20, 150, 0, 0 };
        boxRGBA(wspecs->renderer, target.x, target.y, target.x+400, target.y+20, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);
        showLabel(wspecs, 14, "NOTICE: Successfully entered name of Player 2.", &target);
    }
    else {
        target = (SDL_Rect){ 20, 150, 0, 0 };
        boxRGBA(wspecs->renderer, target.x, target.y, target.x+400, target.y+20, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);
        showLabel(wspecs, 14, "NOTICE: Name of Player 2 left unchanged.", &target);
    }
}

//INIT ON START
static void sdlInit(char const *label, WindowSpecs* wspecs) {
    //ABLAK, RENDERER, IKON:
    sdlInitGeneral(label, wspecs);

    //HÁTTÉR KITÖLTÉSE:
    boxRGBA(wspecs->renderer, 0, 0, wspecs->w, wspecs->h, snek_bg.r, snek_bg.g, snek_bg. b, snek_bg.a);
}

//CLOSE ON EXIT
static void sdlClose(WindowSpecs* wspecs) {
    sdlCloseGeneral(wspecs);
}
