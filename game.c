#include "debugmalloc.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "game.h"
#include "global.h"
#include "gameover.h"

//BITMAPON TALÁLHATÓ ELEMEK ELÉRÉSÉNEK DEFINIÁLÁSA (GLOBÁLISAN ELÉRHETŐ):
typedef enum GridType {
    body = 0, headup = 1, headdown = 2, headleft = 3, headright = 4,
              tailup = 5, taildown = 6, tailleft = 7, tailright = 8,
    blank = 9, wall = 10, wallfail = 11, food = 12, bodyfail = 13
} GridType;

static Coord const gridMapping1[] = {
    { 0*28, 0*28 }, { 1*28, 0*28 }, { 2*28, 0*28 }, { 3*28, 0*28 }, { 4*28, 0*28 },
                    { 1*28, 1*28 }, { 2*28, 1*28 }, { 3*28, 1*28 }, { 4*28, 1*28 },
    { 0*28, 4*28 }, { 1*28, 4*28 }, { 2*28, 4*28 }, { 3*28, 4*28 }, { 4*28, 4*28 }
};

static Coord const gridMapping2[] = {
    { 0*28, 2*28 }, { 1*28, 2*28 }, { 2*28, 2*28 }, { 3*28, 2*28 }, { 4*28, 2*28 },
                    { 1*28, 3*28 }, { 2*28, 3*28 }, { 3*28, 3*28 }, { 4*28, 3*28 },
    { 0*28, 4*28 }, { 1*28, 4*28 }, { 2*28, 4*28 }, { 3*28, 4*28 }, { 4*28, 4*28 }
};

//JÁTÉKOST JELLEMZŐ TÍPUSOK:
typedef enum Direction {
    Up, Down, Right, Left
} Direction;

typedef struct SnakeItem { //LÁNCOLT LISTA A KÍGYÓ ELEMEIHEZ
    Direction dir;
    GridType gtype;
    int grid_x, grid_y;
    struct SnakeItem *next;
} SnakeItem;

typedef struct Player {
    int player_num;
    SnakeItem* head;
    Direction state;

    bool canScan;
    bool hasCrashed;

    double pts;
    int eaten;
} Player;

//ZSÁKMÁNYT JELLEMZŐ TÍPUS:
typedef struct Bunny {
    int grid_x, grid_y;
} Bunny;

//MEGAJDA A GRIDTÍPUS BITMAP-BAN TALÁLHATÓ KOORDINÁTÁIT:
static Coord getCoordOfGridType(GridType gtype, int player_num);

//BETESZ EGY SAJÁT ESEMÉNYT BEÁLLÍTOTT IDÕTARTAMONKÉNT AZ ESEMÉNYTÉRBE:
static Uint32 timerCb(Uint32 ms, void* param);

//VALAMI GRIDRE RAK VALAMILYEN TÍPUST:
static void copyToGrid(WindowSpecs* wspecs, GridType gtype, int player_num, int grid_x, int grid_y);

//ALAPRA ÁLLíTJA A KÍGYÓT:
static void initPlayer(WindowSpecs* wspecs, Player* player, int player_num, int grid_x, int grid_y);

//DINAMIKUS TÁRHELY FELSZABADÍTÁSA:
static void destroyPlayer(Player* pplayer);

//ALAPRA ÁLLÍTJA A NYUSZIT:
static void initBunny(WindowSpecs* wspecs, Bunny* bunny, int grid_x, int grid_y);

//ALAPOKRA ÁLLÍTJA A KÍGYÓKAT:
static void initGame(WindowSpecs* wspecs, Player* pplayer1, Player* pplayer2, Bunny* pbunny1, Bunny* pbunny2);

//MEGRAJZOLJA AZ ALAPTERET:
static void initGridSystem(WindowSpecs* wspecs);

//KIMUTATJA A PONTSZÁMOKAT:
static void showPoint(WindowSpecs* wspecs, Player* pplayer);

//SZABAD-E AZ A HELY, AHOVA MENNE AZ ÚJ NYUSZI
static bool isFree(WindowSpecs* wspecs, int grid_x, int grid_y, Bunny* otherBunny, Player* pplayer1, Player* pplayer2);

//ÚJ NYUSZIT TESZ KI:
static void putNewBunny(WindowSpecs* wspecs, Bunny* bunny /*to change*/, Bunny* otherBunny, Player* player1, Player* player2);

//VISSZAADJA, A JÁTÉKOS EZT A NYUSZIT ÉRINTI-E
static bool checkBunny(WindowSpecs* wspecs, Player* pplayer, Player* pOtherPlayer, Bunny* pbunny, Uint32* nextMs);

//VISSZAADJA, TÖRTÉNT-E ÜTKÖZÉS
static bool checkCrash(WindowSpecs* wspecs, Player* pplayer, Player* pOtherplayer);

//MEGNÖVELI A KIGYÓT:
static void growPlayer(WindowSpecs* wspecs, Player* pplayer);

//SHIFTELÉST HAJT VÉGRE: VISSZAKAPJA A RÉGI IRÁNYÁT A FAROKNAK A NÖVEKEDÉSHEZ
static void movePlayer(WindowSpecs* wspecs, Player* pplayer);

//FRISSÍTI A PLAYER ÁLLAPOTÁT, VISSZAADJA, HOGY GAMEOVER VAN-E:
static bool updatePlayer(WindowSpecs* wspecs, Player* pplayer, Player* pOtherPlayer, Bunny* pbunny1, Bunny* pbunny2, Uint32* pnextMs);

//SDL COMPONENSEKET INICIALIZÁLÓ FÜGGVÉNY:
static void sdlInit(char const *label, WindowSpecs* wspecs);

//SDL COMPONENSEKET TÖRLŐ FÖGGVÉNY:
static void sdlClose(WindowSpecs* wspecs, SDL_TimerID* ptimer);

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------

void openGame(GameSettings gSet) {
    srand(time(0));
    WindowSpecs wspecs;
    Player player1;
    Player player2;
    Bunny bunny1;
    Bunny bunny2;

    //WSPECS BEÁLLÍTÁSA, INICIALIZÁLÁS:
    wspecs.gSet = &gSet;
    initGame(&wspecs, &player1, &player2, &bunny1, &bunny2);

    //TIMER LÉTREHOZÁSA: 500 ms --> 120 BPM
    Uint32 nextMs = 500; //ÁLLAPOTVÁLTOZÓ
    SDL_TimerID timer = SDL_AddTimer(nextMs, timerCb, &nextMs);

    SDL_Event event;
    bool quit = false;
    bool isGameOver = false;

    while (!quit && !isGameOver) {
        SDL_WaitEvent(&event);

        switch (event.type) {
            //IDŐZITETT SAJÁT ESEMÉNY ESETE
            case SDL_USEREVENT:
                isGameOver = updatePlayer(&wspecs, &player1, &player2, &bunny1, &bunny2, &nextMs);
                if (gSet.isMulti && !isGameOver)
                    isGameOver = updatePlayer(&wspecs, &player2, &player1, &bunny1, &bunny2, &nextMs);
                break;

            //GOMBNYOMÁS ESETE
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_LEFT:
                        if (player1.canScan && player1.state != Right) {
                            player1.state = Left;
                            player1.canScan = false;
                        }
                        break;
                    case SDL_SCANCODE_RIGHT:
                        if (player1.canScan && player1.state != Left) {
                            player1.state = Right;
                            player1.canScan = false;
                        }
                        break;
                    case SDL_SCANCODE_UP:
                        if (player1.canScan && player1.state != Down) {
                            player1.state = Up;
                            player1.canScan = false;
                        }
                        break;
                    case SDL_SCANCODE_DOWN:
                        if (player1.canScan && player1.state != Up) {
                            player1.state = Down;
                            player1.canScan = false;
                        }
                        break;
                    case SDL_SCANCODE_A:
                        if (player2.canScan && player2.state != Right) {
                            player2.state = Left;
                            player2.canScan = false;
                        }
                        break;
                    case SDL_SCANCODE_D:
                        if (player2.canScan && player2.state != Left) {
                            player2.state = Right;
                            player2.canScan = false;
                        }
                        break;
                    case SDL_SCANCODE_W:
                        if (player2.canScan && player2.state != Down) {
                            player2.state = Up;
                            player2.canScan = false;
                        }
                        break;
                    case SDL_SCANCODE_S:
                        if (player2.canScan && player2.state != Up) {
                            player2.state = Down;
                            player2.canScan = false;
                        }
                        break;
                    case SDL_SCANCODE_ESCAPE:
                        quit = true;
                        break;
                    default:;
                        break;
                }
                break;

            //BEZÁRÁS ESETE
            case SDL_QUIT:
                quit = true;
                break;
        }
    }

    //JÁTÉKOSOK KÍGYÓINAK LISTÁJÁT FELSZABADÍTÓ FÜGGVÉNYEK:
    destroyPlayer(&player1);
    if (gSet.isMulti)
        destroyPlayer(&player2);

    //BEZÁRÁS, JÁTÉK VÉGE FÁZIS INDÍTÁSA:
    sdlClose(&wspecs, &timer);
    gSet.pts1 = player1.pts;
    gSet.pts2 = player2.pts;
    openGameOver(gSet);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------

//MEGAJDA A GRIDTÍPUS BITMAP-BAN TALÁLHATÓ KOORDINÁTÁIT:
static Coord getCoordOfGridType(GridType gtype, int player_num) {
    if (player_num == 1) {
        return gridMapping1[gtype];
    }
    return gridMapping2[gtype];
}

//BETESZ EGY SAJÁT ESEMÉNYT BEÁLLÍTOTT IDÕTARTAMONKÉNT AZ ESEMÉNYTÉRBE:
static Uint32 timerCb(Uint32 ms, void* param) {
    SDL_Event ev;
    ev.type = SDL_USEREVENT;
    SDL_PushEvent(&ev);

    //GYORSÍTÁS:
    int* nextMs = (int*) param;
    if (*nextMs != ms) {
        return *nextMs;
    }
    return ms;
}

//VALAMI GRIDRE RAK VALAMILYEN TÍPUST:
static void copyToGrid(WindowSpecs* wspecs, GridType gtype, int player_num, int grid_x, int grid_y) {
    SDL_Rect dest = { (grid_x + 1) * 28, (grid_y + 1) * 28, 28, 28 };
    Coord co = getCoordOfGridType(gtype, player_num);
    SDL_Rect src = { co.x, co.y, 28, 28 };
    SDL_RenderCopy(wspecs->renderer, wspecs->bitMap, &src, &dest);
}

//ALAPRA ÁLLíTJA A KÍGYÓT:
static void initPlayer(WindowSpecs* wspecs, Player* player, int player_num, int grid_x, int grid_y) {
    GridType types[] = { tailright, body, headright };

    player->eaten = 0;
    player->player_num = player_num;
    player->pts = 0;
    player->state = Right;
    player->canScan = true;
    player->hasCrashed = false;

    //BESZÚRÁS ELÕRE:
    SnakeItem* head = NULL;
    for (int i = 0; i < 3; i++) {
        SnakeItem* newCell = (SnakeItem*) malloc(sizeof(SnakeItem));
        newCell->dir = Right;
        newCell->grid_x = i + grid_x - 2;
        newCell->grid_y = grid_y;
        newCell->gtype = types[i];
        newCell->next = head;
        head = newCell;
        copyToGrid(wspecs, newCell->gtype, player->player_num, newCell->grid_x, newCell->grid_y);
    }

    player->head = head;
}

//FREE:
static void destroyPlayer(Player* pplayer) {
    SnakeItem* moving = pplayer->head;
    SnakeItem* next;
    while (moving != NULL) {
        next = moving->next;
        free(moving);
        moving = next;
    }
}

//ALAPRA ÁLLÍTJA A NYUSZIT:
static void initBunny(WindowSpecs* wspecs, Bunny* bunny, int grid_x, int grid_y) {
    bunny->grid_x = grid_x;
    bunny->grid_y = grid_y;
    copyToGrid(wspecs, food, 0, grid_x, grid_y);
}

//ALAPOKRA ÁLLÍTJA A KÍGYÓKAT:
static void initGame(WindowSpecs* wspecs, Player* pplayer1, Player* pplayer2, Bunny* pbunny1, Bunny* pbunny2) {
    wspecs->w = 28 * (wspecs->gSet->GRID_X + 2) + 300; //WIDTH
    wspecs->h = 28 * (wspecs->gSet->GRID_Y + 2); //HEIGHT
    sdlInit("LIL'SNEK 2019 - PLAYING GAME", wspecs);

    //HÁTTÉR:
    int grid_x = wspecs->gSet->GRID_X;
    int grid_y = wspecs->gSet->GRID_Y;
    boxRGBA(wspecs->renderer, 0, 0, wspecs->w, wspecs->h, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);

    //GRIDSYSTEM KIRAJZOLÁSA:
    initGridSystem(wspecs);

    //INICIALIZÁLÁSOK:
    initPlayer(wspecs, pplayer1, 1, 3, 5);
    if (wspecs->gSet->isMulti) {
        initPlayer(wspecs, pplayer2, 2, 3, 7);
    }

    initBunny(wspecs, pbunny1, 6, 5);
    if (wspecs->gSet->isMulti) {
        initBunny(wspecs, pbunny2, 6, 7);
    }

    //PONTOZÁS:
    SDL_Rect target = { 28 * (grid_x + 2) + 30, 30, 0, 0 };
    showLabel(wspecs, 14, "Current scores:", &target);

    showPoint(wspecs, pplayer1);
    if (wspecs->gSet->isMulti) {
        showPoint(wspecs, pplayer2);
    }

    //QUIT JELZÉS:
    target = (SDL_Rect){ 28 * (grid_x + 2) + 30, 28 * grid_y, 0, 0 };
    showLabel(wspecs, 14, "Press ESC to quit the game.", &target);

    //MEGJELENITÉS:
    SDL_RenderPresent(wspecs->renderer);

    //HÁTTÉRZENE:
    if (wspecs->gSet->canPlayMusic)
        Mix_PlayMusic(wspecs->bgm, -1);
}

//KIMUTATJA A PONTSZÁMOKAT:
static void showPoint(WindowSpecs* wspecs, Player* pplayer) {
    int dest_x = 28 * (wspecs->gSet->GRID_X + 2) + 30;
    int dest_y = 20 + pplayer->player_num * 50;

    //ELŐTÖRLÉS:
    boxRGBA(wspecs->renderer, dest_x, dest_y, dest_x+300, dest_y+50, snek_bg.r, snek_bg.g, snek_bg.b, snek_bg.a);

    //NÉV:
    SDL_Rect targetName = { dest_x, dest_y, 0, 0 };
    if (pplayer->player_num == 1)
        showLabel(wspecs, 12, wspecs->gSet->playerName1, &targetName);
    else
        showLabel(wspecs, 12, wspecs->gSet->playerName2, &targetName);

    //PONTSZÁM:
    char* str = (char*) malloc(sizeof(char) * ((pplayer->pts > 0) ? ((int)(log10(pplayer->pts)) + 12) : 12) + 1); //+12: score:_0.000, +1: \0
    sprintf(str, "score: %.3f", pplayer->pts);

    SDL_Rect targetPts = { dest_x, dest_y + 20, 0, 0 };
    showLabel(wspecs, 16, str, &targetPts);

    free(str);
}

//MEGNÉZI, HOGY A MEGADOTT GRIDX/Y KOORDINÁTÁKON VAN-E KÍGYÓ, NYUSZI MÁR:
static bool isFree(WindowSpecs* wspecs, int grid_x, int grid_y, Bunny* otherBunny, Player* pplayer1, Player* pplayer2) {
    if ( wspecs->gSet->isMulti && grid_x == otherBunny->grid_x && grid_y == otherBunny->grid_y )
        return false;

    SnakeItem* moving;
    for (moving = pplayer1->head; moving != NULL; moving = moving->next) {
        if ( grid_x == moving->grid_x && grid_y == moving->grid_y )
            return false;
    }

    if (wspecs->gSet->isMulti) {
        for (moving = pplayer2->head; moving != NULL; moving = moving->next) {
            if ( grid_x == moving->grid_x && grid_y == moving->grid_y )
                return false;
        }
    }
    return true;
}

//ÚJ NYUSZIT TESZ KI:
static void putNewBunny(WindowSpecs* wspecs, Bunny* bunny /*to change*/, Bunny* otherBunny, Player* player1, Player* player2) {
    int grid_x, grid_y;
    do {
        grid_x = rand() % wspecs->gSet->GRID_X;
        grid_y = rand() % wspecs->gSet->GRID_Y;
    } while (!isFree(wspecs, grid_x, grid_y, otherBunny, player1, player2));
    bunny->grid_x = grid_x;
    bunny->grid_y = grid_y;
    copyToGrid(wspecs, food, 0, grid_x, grid_y);
}

static bool checkBunny(WindowSpecs* wspecs, Player* pplayer, Player* pOtherPlayer, Bunny* pbunny, Uint32* nextMs) {
    //KISZÁMÍTOM, MERRE FOG LÉPNI A KÍGYÓ:
    int grid_x = pplayer->head->grid_x;
    int grid_y = pplayer->head->grid_y;

    switch (pplayer->state) {
        case Right:
            grid_x++;
            break;
        case Left:
            grid_x--;
            break;
        case Up:
            grid_y--;
            break;
        case Down:
            grid_y++;
            break;
    }

    //TALÁLAT VIZSGÁLATA:
    if (grid_x == pbunny->grid_x && grid_y == pbunny->grid_y) {
        pplayer->eaten++;

        //PONTOZÁS, GYORSÍTÁS:
        if (wspecs->gSet->isChallenge) {
            //PONTOZÁS: 1.5-SZERES SZORZÓ / 5 ZSÁKMÁNY
            pplayer->pts += 10 * pow(1.5, floor(pplayer->eaten / 5));

            /** GYORSÍTÁS: 1.2-SZERES BPM (GLOBÁLIS)
            * SINGLE: 5 ZSÁKMÁNYONKÉNT
            * MULTI: ÖSSZESEN 10 ZSÁKMÁNYONKÉNT
            */
            if (wspecs->gSet->isMulti) {
                if ((pplayer->eaten + pOtherPlayer->eaten) % 10 == 0) {
                    *nextMs /= 1.2;
                }
            }
            else {
                if (pplayer->eaten % 5 == 0) {
                    *nextMs /= 1.2;
                }
            }
        }
        else {
            //PONTOZÁS: 10 PONT / ZSÁKMÁNY
            pplayer->pts += 10;
        }

        return true;
    }

    return false;
}

static bool checkCrash(WindowSpecs* wspecs, Player* pplayer, Player* pOtherplayer) {
    //FALBA:
    int grid_x = pplayer->head->grid_x;
    int grid_y = pplayer->head->grid_y;
    if (grid_x >= wspecs->gSet->GRID_X || grid_y >= wspecs->gSet->GRID_Y || grid_x < 0 || grid_y < 0) {
        copyToGrid(wspecs, wallfail, 0, grid_x, grid_y);
        return true;
    }

    //ÖNMAGÁBA:
    SnakeItem* moving;
    for (moving = pplayer->head->next; moving != NULL; moving = moving->next) {
        if (grid_x == moving->grid_x && grid_y == moving->grid_y) {
            copyToGrid(wspecs, bodyfail, 0, grid_x, grid_y);
            return true;
        }
    }

    //EGYMÁSBA ÜTKÖZÉS:
    if (wspecs->gSet->isMulti) {
        SnakeItem* moving;
        for (moving = pOtherplayer->head; moving != NULL; moving = moving->next) {
            if (grid_x == moving->grid_x && grid_y == moving->grid_y) {
                copyToGrid(wspecs, bodyfail, 0, grid_x, grid_y);
                return true;
            }
        }
    }

    return false;
}

//MEGNÖVELI A KIGYÓT:
static void growPlayer(WindowSpecs* wspecs, Player* pplayer) {
    SnakeItem* head = pplayer->head;

    //ÚJ FEJ:
    SnakeItem* newHead = (SnakeItem*) malloc(sizeof(SnakeItem));
    newHead->next = head;
    newHead->dir = pplayer->state;
    newHead->grid_x = head->grid_x;
    newHead->grid_y = head->grid_y;

    switch (pplayer->state) {
        case Right:
            newHead->grid_x++;
            newHead->gtype = headright;
            break;
        case Left:
            newHead->grid_x--;
            newHead->gtype = headleft;
            break;
        case Up:
            newHead->grid_y--;
            newHead->gtype = headup;
            break;
        case Down:
            newHead->grid_y++;
            newHead->gtype = headdown;
            break;
    }

    head->gtype = body;
    copyToGrid(wspecs, newHead->gtype, pplayer->player_num, newHead->grid_x, newHead->grid_y);
    copyToGrid(wspecs, head->gtype, pplayer->player_num, head->grid_x, head->grid_y);

    head = newHead;
    pplayer->head = head;
}

//SHIFTELÉST HAJT VÉGRE
static void movePlayer(WindowSpecs* wspecs, Player* pplayer) {
    Player player = *pplayer;

    //VÉGIGFUTTATNI AZ ÁLLAPOTOKAT, KÖZBEN RAJZOLNI:
    SnakeItem* head = player.head;
    int player_num = player.player_num;
    SnakeItem* moving;

    Direction dir = 0;
    Direction inFrontOfMe = 0;
    Direction next_state = player.state;

    for (moving = head; moving != NULL; moving = moving->next) {
        GridType gtype = 0;

        //CSERE:
        dir = moving->dir;
        moving->dir = next_state;
        next_state = dir;

        if (moving->next != NULL)
            inFrontOfMe = moving->dir;

        //TÖRLÖM JELENLEGI POZÍCIÓM, HA AZ ÉPPEN NEM A FEJ (LÁSD 4-ES HOSSZNÁL):
        if ( !(moving->grid_x == head->grid_x && moving->grid_y == head->grid_y) )
            copyToGrid(wspecs, blank, 0, moving->grid_x, moving->grid_y);

        //RAJZOLÁS JELENLEGI IRÁNYBA:
        switch (moving->dir) {
            case Right:
                if (head == moving)
                    gtype = headright;
                else if (moving->next == NULL)
                    gtype = tailright;
                else
                    gtype = moving->gtype;
                moving->grid_x++;
                break;
            case Left:
                if (head == moving)
                    gtype = headleft;
                else if (moving->next == NULL)
                    gtype = tailleft;
                else
                    gtype = moving->gtype;
                moving->grid_x--;
                break;
            case Up:
                if (head == moving)
                    gtype = headup;
                else if (moving->next == NULL)
                    gtype = tailup;
                else
                    gtype = moving->gtype;
                moving->grid_y--;
                break;
            case Down:
                if (head == moving)
                    gtype = headdown;
                else if (moving->next == NULL)
                    gtype = taildown;
                else
                    gtype = moving->gtype;
                moving->grid_y++;
                break;
        }
        copyToGrid(wspecs, gtype, player_num, moving->grid_x, moving->grid_y);

        //FAROKFORGATÁS ELÕTTE LÉVÕ IRÁNYA SZERINT:
        if (moving->next == NULL) {
            switch (inFrontOfMe) {
                case Right:
                    gtype = tailright;
                    break;
                case Left:
                    gtype = tailleft;
                    break;
                case Up:
                    gtype = tailup;
                    break;
                case Down:
                    gtype = taildown;
                    break;
            }
            copyToGrid(wspecs, gtype, player_num, moving->grid_x, moving->grid_y);
        }
    }

    player.head = head;
    *pplayer = player;
}

//FRISSÍTI A PLAYER ÁLLAPOTÁT, VISSZAADJA, HOGY GAMEOVER VAN-E:
static bool updatePlayer(WindowSpecs* wspecs, Player* pplayer, Player* pOtherPlayer, Bunny* pbunny1, Bunny* pbunny2, Uint32* pnextMs) {
    bool hitBunny1 = checkBunny(wspecs, pplayer, pOtherPlayer, pbunny1, pnextMs);
    bool hitBunny2 = checkBunny(wspecs, pplayer, pOtherPlayer, pbunny2, pnextMs);

    if (hitBunny1 || hitBunny2) {
        growPlayer(wspecs, pplayer);
        if (hitBunny1) {
            putNewBunny(wspecs, pbunny1, pbunny2, pplayer, pOtherPlayer);
        }
        else if (hitBunny2) {
            putNewBunny(wspecs, pbunny2, pbunny1, pplayer, pOtherPlayer);
        }
    }
    else {
        movePlayer(wspecs, pplayer);
    }

    pplayer->canScan = true;

    //CRASH ÉS PONTSZÁM:
    bool hasCrashed = checkCrash(wspecs, pplayer, pOtherPlayer);
    showPoint(wspecs, pplayer);

    SDL_RenderPresent(wspecs->renderer);

    //CRASH:
    if (hasCrashed) {
        pplayer->hasCrashed = hasCrashed;
        return true;
    }
    return false;
}

//MEGRAJZOLJA AZ ALAPTERET
static void initGridSystem(WindowSpecs* wspecs) {
    int grid_x = wspecs->gSet->GRID_X;
    int grid_y = wspecs->gSet->GRID_Y;
    //PÁLYA RAJZOLÁSA:
    for (int gx = 0; gx < grid_x + 2; gx++) {
        for (int gy = 0; gy < grid_y + 2; gy++) {
            if (gx == 0 || gy == 0 || gx == grid_x + 1 || gy == grid_y + 1)
                copyToGrid(wspecs, wall, 0, gx - 1, gy - 1);
            else
                copyToGrid(wspecs, blank, 0, gx - 1, gy - 1);
        }
    }
}

//SDL COMPONENSEKET INICIALIZÁLÓ FÜGGVÉNY
static void sdlInit(char const *label, WindowSpecs* wspecs) {
    //ABLAK, RENDERER, IKON:
    sdlInitGeneral(label, wspecs);

    //BITMAP:
    SDL_Texture *bitMap = IMG_LoadTexture(wspecs->renderer, "./data/png/map.png");
    if (bitMap == NULL) {
        SDL_Log("Nem nyithato meg a kepfajl: %s", IMG_GetError());
        exit(1);
    }

    //HÁTTÉRZENE:
    Mix_Music* bgm = Mix_LoadMUS("./data/music/ingame.mp3");
    if (bgm == NULL) {
        SDL_Log("Zene betoltesi hiba: %s\n", Mix_GetError());
    }

    //ÁTADÁS:
    wspecs->bitMap = bitMap;
    wspecs->bgm = bgm;
}

static void sdlClose(WindowSpecs* wspecs, SDL_TimerID* ptimer) {
    Mix_HaltMusic();
    sdlCloseGeneral(wspecs);

    SDL_RemoveTimer(*ptimer);
}
