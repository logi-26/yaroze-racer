#ifndef WORLD_H
#define WORLD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libps.h>
#include "player.h"
#include "model.h"

#define MAX_WORLD_OBJECTS (50)

// Array of models in the world
//ModelStruct worldModels[MAX_WORLD_OBJECTS];

// Track how many models are actually used
//extern int numWorldModels = 0;

// 2 cameras, one for each player
extern GsRVIEW2 Camera[2]; 

extern u_long vsyncInterval;

/************* FUNCTION PROTOTYPES *******************/
int LoadTexture(long addr);
void InitialiseWorld ();
void InitialiseWorldTextures();
void InitialiseWorldModels();
void DrawWorldModels(PlayerStruct* currentPlayer, int currentBuffer);
void RenderWorld();
void RenderWorldPlayer1(int currentBuffer);
void RenderWorldPlayer2(int currentBuffer);
/*****************************************************/

/*****************************************************/
// World model and texture memory addresses
/*****************************************************/
#define BARRIER_2_MEM_ADDR                  (0x800ADB2C)
#define BARRIER_2_TEX_MEM_ADDR              (0x800AEB2C)
 
#define BUILDING_1_MEM_ADDR                 (0x800AFF20)
#define BUILDING_1_TEX_MEM_ADDR             (0x800B00B0)

#define BUILDING_2_MEM_ADDR                 (0x800B3000)
#define BUILDING_2_TEX_MEM_ADDR             (0x800B3190)

#define STAND_MEM_ADDR                      (0x800B5000)  
#define STAND_CROWD_TEX_MEM_ADDR            (0x800B5070) 
/*****************************************************/

#endif // WORLD_H