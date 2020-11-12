#ifndef GLOBAL_H
#define GLOBAL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>

typedef struct Coord {
    int x, y;
} Coord;

//MODULOK KÖZT VÁNDORLÓ JÁTSZMABEÁLLÍTÁSOK:
typedef struct GameSettings {
    int GRID_X, GRID_Y;
    bool isMulti, isChallenge;
    char playerName1[20+1];
    char playerName2[20+1];
    double pts1, pts2;
    bool canPlayMusic;
} GameSettings;

//ICSÖSÉGTÁBLA KIÍRÁSÁNÁL HASZNÁLT BEÁLLÍTÁSOK:
typedef struct LeaderSettings {
    bool isMulti, isChallenge;
} LeaderSettings;

//HELYI SDL PROPERTIES:
typedef struct WindowSpecs {
    GameSettings* gSet;
    int w; //window's width
    int h; //window's height
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *bitMap;
    Mix_Music* bgm;
} WindowSpecs;

//GOMBOKHOZ:
typedef struct Button {
    char* name;
    int x, y, w, h; //x, y, width, height
} Button;

typedef struct ButtonSet {
    Button* btns;
    int sizeBtn; //number of pieces
    int screenNum; //state to show
} ButtonSet;

//FÜGGVÉNYEK:
void showButtons(WindowSpecs* wspecs, ButtonSet* btnSet);
int getButtonClicked(SDL_Event* ev, ButtonSet* btnSet);
void showLabel(WindowSpecs* wspecs, int fontSize, char* str, SDL_Rect* targetCoord);
void sdlInitGeneral(char const *label, WindowSpecs* wspecs);
void sdlCloseGeneral(WindowSpecs* wspecs);

//SZÍNVÁLASZTÉK:
extern SDL_Color snek_green;
extern SDL_Color black;
extern SDL_Color white;
extern SDL_Color snek_bg;
extern SDL_Color red;

#endif
