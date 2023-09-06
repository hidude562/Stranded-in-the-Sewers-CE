////////////////////////////////////////
// { Stranded in the Sewers, NY } { VERSION }
// Author: hidude562
// License: None
// Description:
////////////////////////////////////////

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>
#include <time.h>
#include "gfx/gfx.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphx.h>
#include <keypadc.h>
#include <usbdrvce.h>
/* Put function prototypes here or in a header (.h) file */

/* Note: uint8_t is an unsigned integer that can range from 0-255. */
/* It performs faster than just an int, so try to use it (or int8_t) when possible */
void printText(const char *text, uint8_t x, uint8_t y);
bool pixelCoordsToisBlock(uint16_t x, uint16_t y);
void stairs4by3flat(uint8_t x, uint8_t y);
void rectangleFill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void renderInitialBackground();
void clearCurrentMap();

uint8_t backc = 0xD6;

uint16_t screenX = 320;
uint16_t screenY = 240;

// TODO: I have a crazy idea but what if you used the serial port and 2 calculators to play...


struct Velocity {
    int16_t x;
    int16_t y;
};

struct SquareCollision {
    bool leftCollide;
    bool rightCollide;
    bool upCollide;
    bool downCollide;
};

struct PulleyButton {
    // USE THESE TWO
    int16_t pulleyStartRelX;
    int16_t pulleyStartRelY;

    int16_t pulleyEndRelY;

    // DONT USE THESE TWO
    int16_t pulleyYoffset;
    int16_t pulleyXoffset;

    int16_t buttonX;
    int16_t buttonY;
    bool    isButtonDown;

    int8_t pulleySpeedX;
    int8_t pulleySpeedY;

    struct Velocity pulleyVelocity;
};

struct Map {
    uint8_t mapX;
    uint8_t mapY;

    bool isBlock[15][20];
    uint8_t numPulleys;
    struct PulleyButton pulleyButtons[2];

};


const struct Map defaultMap;
struct Map currentMap;
struct Map maps[10];


uint8_t currentMapIndex = 0;

struct Player {
    int16_t x;
    int16_t y;
    struct   Velocity velocity;
    struct   SquareCollision collision;
};

// playerCat == players[0]
// playerDog == players[1]

struct Player players[2];

uint8_t numPlayers = 2;



//struct Player playerCat;
//struct Player playerDog;


/* This is the entry point of your program. */
/* argc and argv can be there if you need to use arguments, see the toolchain example. */

struct PulleyButton createPulleyButtonSimple(int16_t buttonXTile,
                                             int16_t buttonYTile,
                                             int16_t pulleyXTileOffset,
                                             int16_t pulleyYTileOffset,
                                             int16_t pulleyYMaxDistTiles) {
    struct PulleyButton pulleyButtonNew;

    pulleyButtonNew.buttonX = buttonXTile * 160;
    pulleyButtonNew.buttonY = buttonYTile  * 160;
    pulleyButtonNew.pulleyStartRelX = pulleyXTileOffset * 160;
    pulleyButtonNew.pulleyStartRelY = pulleyYTileOffset * 160 + 70;
    pulleyButtonNew.pulleySpeedY = 10;
    pulleyButtonNew.pulleyEndRelY = pulleyYMaxDistTiles * 160 + 70;

    return pulleyButtonNew;
}

void initMaps () {
    srand(4);

    currentMap.mapX = 20;
    currentMap.mapY = 15;

    /*
    for(uint8_t y = 0; y < currentMap.mapY; y++) {
        for(uint8_t x = 0; x < currentMap.mapX; x++) {
            currentMap.isBlock[y][x] = !(rand() & 10);
        }
    }
    */

    rectangleFill(5,8,15,15);
    currentMap.numPulleys = 1;
    currentMap.pulleyButtons[0] = createPulleyButtonSimple(5, 7, -1, 0, 4);
    maps[0] = currentMap;

    clearCurrentMap();

    rectangleFill(5,11,6,15);
    rectangleFill(5,5,10,6);
    rectangleFill(7,6,10,7);

    rectangleFill(13,5,15,15);

    currentMap.numPulleys = 1;
    currentMap.pulleyButtons[0] = createPulleyButtonSimple(9, 4, 1, 0, 8);
    maps[2] = currentMap;

    clearCurrentMap();

    rectangleFill(8,12,20,15);
    rectangleFill(15,9,20,15);
    stairs4by3flat(4,12);
    currentMap.numPulleys = 0;
    //currentMap.pulleyButtons[0] = createPulleyButtonSimple(9, 4, 1, 0, 4);
    maps[1] = currentMap;

    clearCurrentMap();

    rectangleFill(5,8,15,15);
    currentMap.numPulleys = 1;
    currentMap.pulleyButtons[0] = createPulleyButtonSimple(5, 7, -1, 0, 4);
    maps[2] = currentMap;

    clearCurrentMap();
}

void clearCurrentMap() {
    currentMap = defaultMap;
    currentMap.mapX = 20;
    currentMap.mapY = 15;
}

/*
Looks like:

0011
0111
1111

Modifies currentMap

All of these functions go by topleft corner
*/

void stairs4by3flat(uint8_t x, uint8_t y) {
    bool stairs[3][4] =
    {
        {false, false, true, true},
        {false, true , true, true},
        {true , true , true, true}
    };

    for(uint8_t y1 = 0; y1 < 3; y1++) {
        for(uint8_t x1 = 0; x1 < 4; x1++) {
            currentMap.isBlock[y+y1][x+x1] = stairs[y1][x1];
        }
    }
}

void rectangleFill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    uint8_t x_dim = x2 - x1;
    uint8_t y_dim = y2 - y1;

    for(uint8_t yIndex = 0; yIndex < y_dim; yIndex++) {
        for(uint8_t xIndex = 0; xIndex < x_dim; xIndex++) {
            currentMap.isBlock[y1+yIndex][x1+xIndex] = true;
        }
    }
}

void resetPlayers() {
    players[0].x = 0;
    players[0].y = (screenY - 16) * 10;

    players[1].x = 200;
    players[1].y = (screenY - 16) * 10;

    players[0].velocity.x = 0;
    players[0].velocity.y = 0;

    players[1].velocity.x = 0;
    players[1].velocity.y = 0;
}


// Input stuff

bool kb_Was_Up;
bool kb_Was_7;

void playerInputs() {
    kb_Scan();

    if(kb_Data[7] & kb_Right) {
        players[1].velocity.x += 12;
    }

    if(kb_Data[7] & kb_Up  && players[1].collision.downCollide) {
        players[1].velocity.y = 50;
    }

    if(kb_Data[7] & kb_Left) {
        players[1].velocity.x -= 12;
    }



    if(kb_Data[4] & kb_5) {
        players[0].velocity.x += 12;
    }

    if(kb_Data[3] & kb_7 && players[0].collision.downCollide) {
        players[0].velocity.y = 50;
    }

    if(kb_Data[3] & kb_4) {
        players[0].velocity.x -= 12;
    }

    players[1].velocity.x = players[1].velocity.x * 10 / 13;
    players[0].velocity.x = players[0].velocity.x * 10 / 13;

    kb_Was_Up = kb_Data[7] & kb_Up;
    kb_Was_7 = kb_Data[3] & kb_7;
}

void stopPlayerClipping(uint8_t playerIndex) {
    if(players[playerIndex].collision.rightCollide
       ||
       players[playerIndex].collision.leftCollide) {

       players[playerIndex].x = (players[playerIndex].x + 80) / 160 * 160;
       return;
    }
    if(players[playerIndex].collision.downCollide
       ||
       players[playerIndex].collision.upCollide) {

       players[playerIndex].y = (players[playerIndex].y + 80) / 160 * 160;
       return;
    }
}

void updateCollision(uint8_t playerIndex) {
    players[playerIndex].collision.downCollide  = false;
    players[playerIndex].collision.rightCollide = false;
    players[playerIndex].collision.leftCollide  = false;
    players[playerIndex].collision.upCollide    = false;

    if(players[playerIndex].y / 10 + 16 >= 240) {
        players[playerIndex].collision.downCollide = true;
        players[playerIndex].y = (screenY - 16) * 10;
    }


    // Test bottom left and bottom right if inside block
    if(pixelCoordsToisBlock(players[playerIndex].x / 10 + 2, players[playerIndex].y /10 + 16) ||
        pixelCoordsToisBlock(players[playerIndex].x/10 + 14, players[playerIndex].y/10 + 16)) {
        players[playerIndex].collision.downCollide = true;
        stopPlayerClipping(playerIndex);
    }

    if(pixelCoordsToisBlock(players[playerIndex].x / 10 + 2, players[playerIndex].y /10) ||
        pixelCoordsToisBlock(players[playerIndex].x/10 + 14, players[playerIndex].y/10)) {
        players[playerIndex].collision.upCollide = true;
        stopPlayerClipping(playerIndex);
    }

    if(pixelCoordsToisBlock(players[playerIndex].x/10, players[playerIndex].y /10) ||
        pixelCoordsToisBlock(players[playerIndex].x/10, players[playerIndex].y/10 + 15)) {
        players[playerIndex].collision.leftCollide = true;
        stopPlayerClipping(playerIndex);
    }

    if(pixelCoordsToisBlock(players[playerIndex].x/10 + 16, players[playerIndex].y /10) ||
        pixelCoordsToisBlock(players[playerIndex].x/10 + 16, players[playerIndex].y/10 + 15)) {
        players[playerIndex].collision.rightCollide = true;
        stopPlayerClipping(playerIndex);
    }

    // Absoute spehgetti

    // Detect collision with other player
    // Also fix clipping at same time since collision is not gridded
    for(uint8_t i=0; i < numPlayers; i++) {
        if(i != playerIndex) {
            bool sameYaxis = (players[playerIndex].y + 50 > players[i].y &&
               players[playerIndex].y - 50 < players[i].y);
            bool sameXaxis = (players[playerIndex].x + 120 > players[i].x &&
               players[playerIndex].x - 120 < players[i].x);

            if(players[playerIndex].x + 160 > players[i].x &&
               players[playerIndex].x - 0 < players[i].x) {
                if(sameYaxis) {
                    players[playerIndex].collision.rightCollide = true;
                    players[playerIndex].x = players[i].x - 160;
                }
            }

            if(players[playerIndex].x - 0 > players[i].x &&
               players[playerIndex].x - 160 < players[i].x) {
                if(sameYaxis) {
                    players[playerIndex].collision.leftCollide = true;
                    players[playerIndex].x = players[i].x + 160;
                }
            }

            if(players[playerIndex].y + 160 > players[i].y &&
               players[playerIndex].y - 0 < players[i].y) {
                if(sameXaxis) {
                    players[playerIndex].collision.downCollide = true;
                    players[playerIndex].y = players[i].y - 150;
                }
            }

            if(players[playerIndex].y - 160 < players[i].y &&
               players[playerIndex].y - 0 > players[i].y) {
                if(sameXaxis) {
                    players[playerIndex].collision.upCollide = true;
                    players[playerIndex].y = players[i].y + 150;
                }
            }
        }
    }

    // The collision of the pulley button
    for(uint8_t i=0; i < currentMap.numPulleys; i++) {
        bool sameYaxis = (players[playerIndex].y + 50 > currentMap.pulleyButtons[i].buttonY &&
           players[playerIndex].y - 50 < currentMap.pulleyButtons[i].buttonY);
        bool sameXaxis = (players[playerIndex].x + 160 > currentMap.pulleyButtons[i].buttonX &&
           players[playerIndex].x - 160 < currentMap.pulleyButtons[i].buttonX);

        if(players[playerIndex].x + 160 > currentMap.pulleyButtons[i].buttonX &&
           players[playerIndex].x - 0 < currentMap.pulleyButtons[i].buttonX) {
            if(sameYaxis) {
                players[playerIndex].collision.rightCollide = true;
                players[playerIndex].x = currentMap.pulleyButtons[i].buttonX - 160;
            }
        }

        if(players[playerIndex].x - 0 > currentMap.pulleyButtons[i].buttonX &&
           players[playerIndex].x - 160 < currentMap.pulleyButtons[i].buttonX) {
            if(sameYaxis) {
                players[playerIndex].collision.leftCollide = true;
                players[playerIndex].x = currentMap.pulleyButtons[i].buttonX + 160;
            }
        }

        if(players[playerIndex].y + 70 > currentMap.pulleyButtons[i].buttonY &&
           players[playerIndex].y - 0 < currentMap.pulleyButtons[i].buttonY) {
            if(sameXaxis) {
                players[playerIndex].collision.downCollide = true;
                currentMap.pulleyButtons[i].isButtonDown = true;
                players[playerIndex].y = currentMap.pulleyButtons[i].buttonY - 60;
            }
        }
    }


    // The collision of the pulley up down thing
    // TODO: Sticky player

    for(uint8_t i=0; i < currentMap.numPulleys; i++) {
        int16_t pulleyX = currentMap.pulleyButtons[i].buttonX + currentMap.pulleyButtons[i].pulleyStartRelX;
        int16_t pulleyY = (currentMap.pulleyButtons[i].buttonY + currentMap.pulleyButtons[i].pulleyYoffset + currentMap.pulleyButtons[i].pulleyStartRelY);

        bool sameYaxis = (players[playerIndex].y + 50 > pulleyY &&
           players[playerIndex].y - 50 < pulleyY);
        bool sameXaxis = (players[playerIndex].x + 160 > pulleyX &&
           players[playerIndex].x - 160 < pulleyX);


        /*
        if(players[playerIndex].x + 160 > pulleyX &&
           players[playerIndex].x - 0 < pulleyX) {
            if(sameYaxis) {
                players[playerIndex].collision.rightCollide = true;
                players[playerIndex].x = pulleyX - 160;
            }
        }

        if(players[playerIndex].x - 0 > pulleyX &&
           players[playerIndex].x - 160 < pulleyX) {
            if(sameYaxis) {
                players[playerIndex].collision.leftCollide = true;
                players[playerIndex].x = pulleyX + 160;
            }
        }
        */

        if(players[playerIndex].y + 70 > pulleyY &&
           players[playerIndex].y - 0 < pulleyY) {
            if(sameXaxis) {
                players[playerIndex].collision.downCollide = true;
                players[playerIndex].y = pulleyY - 60;
            }
        }

        if(players[playerIndex].y - 160 < pulleyY &&
           players[playerIndex].y - 0 > pulleyY) {
            if(sameXaxis) {
                players[playerIndex].collision.upCollide = true;
                players[playerIndex].y = pulleyY + 150;
            }
        }
    }
}

void guiResetDraw() {
    gfx_SetColor(0);
    gfx_FillRectangle(0, 0, 240, 8);
}

void physics() {
    for(uint8_t i = 0; i < currentMap.numPulleys; i++) {
        currentMap.pulleyButtons[i].isButtonDown = false;
    }

    for(uint8_t i = 0; i < numPlayers; i++) {

        playerInputs();

        if(!players[i].collision.downCollide) {
            players[i].velocity.y -= 2;
        }

        if((players[i].velocity.y < 0 && !players[i].collision.downCollide) || (players[i].velocity.y > 0 && !players[i].collision.upCollide)) {
            players[i].y -= players[i].velocity.y;
        } else {
            players[i].velocity.y = 0;
        }

        if(players[i].velocity.x > 0) {
            if(players[i].collision.rightCollide) {
                players[i].velocity.x = 0;
            }
        } else {
            if(players[i].collision.leftCollide) {
                players[i].velocity.x = 0;
            }
        }

        players[i].x += players[i].velocity.x;

        updateCollision(i);
    }

    // Pulley logic
    for(uint8_t i = 0; i < currentMap.numPulleys; i++) {
        currentMap.pulleyButtons[i].pulleyVelocity.x = 0;
        currentMap.pulleyButtons[i].pulleyVelocity.y = 0;

        if(currentMap.pulleyButtons[i].isButtonDown) {
            if(currentMap.pulleyButtons[i].pulleySpeedY > 0) {
                if(currentMap.pulleyButtons[i].pulleyYoffset < currentMap.pulleyButtons[i].pulleyEndRelY)
                    currentMap.pulleyButtons[i].pulleyVelocity.y = currentMap.pulleyButtons[i].pulleySpeedY;
            } else {
                if(currentMap.pulleyButtons[i].pulleyYoffset > currentMap.pulleyButtons[i].pulleyEndRelY)
                    currentMap.pulleyButtons[i].pulleyVelocity.y = 0 - currentMap.pulleyButtons[i].pulleySpeedY;
            }
        } else {
            if(currentMap.pulleyButtons[i].pulleyYoffset > currentMap.pulleyButtons[i].pulleySpeedY) {
                currentMap.pulleyButtons[i].pulleyVelocity.y = 0 - currentMap.pulleyButtons[i].pulleySpeedY;
            }
            else {
                currentMap.pulleyButtons[i].pulleyYoffset = 0;
            }
        }

        currentMap.pulleyButtons[i].pulleyYoffset += currentMap.pulleyButtons[i].pulleyVelocity.y;
        currentMap.pulleyButtons[i].pulleyXoffset += currentMap.pulleyButtons[i].pulleyVelocity.x;

    }
}

// Renders the player and calculates physics
void playerPhysicsRender() {
    gfx_Sprite(cat, players[0].x / 10, players[0].y / 10);
    gfx_Sprite(dog, players[1].x / 10, players[1].y / 10);

    physics();
}

void pulleyRender() {
    for(uint8_t i = 0; i < currentMap.numPulleys; i++) {
        // It is inversed sorry
        if(currentMap.pulleyButtons[i].isButtonDown) {
            gfx_TransparentSprite(buttonUp, currentMap.pulleyButtons[i].buttonX / 10, currentMap.pulleyButtons[i].buttonY / 10);
        } else {
            gfx_TransparentSprite(buttonDown, currentMap.pulleyButtons[i].buttonX / 10, currentMap.pulleyButtons[i].buttonY / 10);
        }
        gfx_TransparentSprite(platform, (currentMap.pulleyButtons[i].buttonX + currentMap.pulleyButtons[i].pulleyStartRelX) / 10, (currentMap.pulleyButtons[i].buttonY + currentMap.pulleyButtons[i].pulleyStartRelY + currentMap.pulleyButtons[i].pulleyYoffset) / 10);    }
}

void renderMap() {
    gfx_BlitScreen();
    for(uint8_t y = 0; y < currentMap.mapY; y++) {
        for(uint8_t x = 0; x < currentMap.mapX; x++) {
            if(currentMap.isBlock[y][x]) {
                gfx_Sprite(foreground, x * 16, y * 16);
            }
        }
    }
    gfx_SwapDraw();
}

void loadMap() {
    currentMap = maps[currentMapIndex];
}

void nextLevel() {
    for(uint16_t i = 0; i < 19; i++) {
        gfx_BlitScreen();
        gfx_ShiftLeft(16);
        gfx_SwapDraw();
    }

    resetPlayers();
    currentMapIndex++;
    loadMap();

    renderInitialBackground();
    renderMap();
}

void renderInitialBackground() {
    gfx_BlitScreen();
    for(uint16_t y = 0; y < screenY; y+=16) {
        for(uint16_t x = 0; x < screenX; x+=16) {
            gfx_Sprite(wall, x, y);
        }
    }
    gfx_SwapDraw();
}

bool pixelCoordsToisBlock(uint16_t x, uint16_t y) {
    return currentMap.isBlock[(y) / 16][(x) / 16];
}


// Since the background is a sprite you must replace the sprites closest to the dog/cat
void erase() {
    for(uint8_t i = 0; i < numPlayers; i++) {
        // Updating all 9 tiles the player could be around because im lazy
        // TODO: there should only be 4 tiles touching but im lazy

        uint16_t tileX = (players[i].x + 80) / 160 * 16;
        uint16_t tileY = (players[i].y + 80) / 160 * 16;

        for(int8_t y = -1; y < 2; y++) {
            for(int8_t x = -1; x < 2; x++) {
                if(!pixelCoordsToisBlock(tileX + x*16, tileY + y*16))
                gfx_Sprite(wall, tileX + x*16, tileY + y*16);
                //else
                //gfx_Sprite(foreground, tileX + x*16, tileY + y*16);
            }
        }
    }


    for(uint8_t i = 0; i < currentMap.numPulleys; i++) {
        // Updating all 9 tiles the player could be around because im lazy
        // TODO: there should only be 4 tiles touching but im lazy

        uint16_t tileX = (currentMap.pulleyButtons[i].buttonX + currentMap.pulleyButtons[i].pulleyStartRelX + 80) / 160 * 16;
        uint16_t tileY = (currentMap.pulleyButtons[i].buttonY + currentMap.pulleyButtons[i].pulleyStartRelY + currentMap.pulleyButtons[i].pulleyYoffset + 80) / 160 * 16;

        for(int8_t y = -1; y < 2; y++) {
            for(int8_t x = 0; x < 1; x++) {
                if(!pixelCoordsToisBlock(tileX + x*16, tileY + y*16))
                gfx_Sprite(wall, tileX + x*16, tileY + y*16);
                //else
                //gfx_Sprite(foreground, tileX + x*16, tileY + y*16);
            }
        }
    }
}

bool testFinishLevel() {
    bool qualifyForFinish = true;
    for(uint8_t i = 0; i < numPlayers; i++) {
        if(players[i].x < ((screenX - 32) * 10)) {
            qualifyForFinish = false;
        }
    }

    if(qualifyForFinish) {
        nextLevel();
        return true;
    }
    return false;
}

int main() {
    os_ClrHomeFull();
    gfx_Begin(gfx_8bpp);
    gfx_SetDrawBuffer();

    gfx_SetPalette(global_palette, 256, 0);
    gfx_SetTextFGColor(1);
    initMaps();
    loadMap();

    renderInitialBackground();
    renderMap();

    resetPlayers();

    while (true){ // Refer to https://ce-programming.github.io/toolchain/libraries/keypadc.html for key register info
        gfx_BlitScreen();

        guiResetDraw();
        erase();
        playerPhysicsRender();
        pulleyRender();
        gfx_SwapDraw();
        testFinishLevel();

        if(kb_On){
            break; //Stops program
        }
    }

        //A for loop that copies temp_life to lifez
    gfx_End();
    return 0;
}
