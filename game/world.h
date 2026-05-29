#ifndef WORLD_H
#define WORLD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libps.h>
#include "game/player.h"
#include "game/model.h"

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
void CheckWorldCollisions(PlayerStruct *player, long *lateralSpeed);
void RenderWorld();
void RenderWorldPlayer1(int currentBuffer);
void RenderWorldPlayer2(int currentBuffer);
/*****************************************************/

/*****************************************************/
// World model and texture memory addresses
/*****************************************************/
#define BARRIER_1_MEM_ADDR                  (0x800A6000)
#define BARRIER_1_TEX_MEM_ADDR              (0x800A7000)

#define BARRIER_2_MEM_ADDR                  (0x800A8000)
#define BARRIER_2_TEX_MEM_ADDR              (0x800A9000)
 
 
#define STAND_MEM_ADDR                  	(0x800AA000)
#define CROWD_TEX_MEM_ADDR                  (0x800AB000)
#define STONE_TEX_MEM_ADDR                  (0x800AC000)

#define TUNNEL_MEM_ADDR                  	(0x800AD000)

#define SIGN_1_MEM_ADDR                  	(0x800AE000)
#define SIGN_1_TEX_MEM_ADDR                 (0x800AF000)
 
#define BUILDING_1_MEM_ADDR                 (0x800E004C)
#define BUILDING_1_TEX_MEM_ADDR             (0x800E01DC)

#define BUILDING_2_MEM_ADDR                 (0x800E312C)
#define BUILDING_2_TEX_MEM_ADDR             (0x800E32BC)

/*****************************************************/

#endif // WORLD_H