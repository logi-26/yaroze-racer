#ifndef WORLD_H
#define WORLD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libps.h>
#include "sky.h"
#include "player.h"
#include "engine/model.h"

#define MAX_WORLD_OBJECTS (50)

// 2 cameras, one for each player
extern GsRVIEW2 Camera[2];

// Set to 1 while R1 is held to activate the rear-view camera
extern int rearViewActive;

// Set to 1 while Up+Select are held to activate the birds-eye overview camera
extern int birdsEyeActive;

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
#define BARRIER_1_MEM_ADDR              (0x80095F90)
#define BARRIER_1_TEX_MEM_ADDR          (0x80096020)

#define BARRIER_2_MEM_ADDR              (0x80096860)
#define BARRIER_2_TEX_MEM_ADDR          (0x800968F0)

#define STAND_MEM_ADDR                  (0x80097130)
#define CROWD_TEX_MEM_ADDR              (0x800972C0)
#define STONE_TEX_MEM_ADDR              (0x80097700)

#define TUNNEL_MEM_ADDR                 (0x80097F40)

#define SIGN_1_MEM_ADDR                 (0x800982E0)
#define SIGN_1_TEX_MEM_ADDR             (0x80098370)

#define BUILDING_1_MEM_ADDR             (0x800A1EA0)
#define BUILDING_1_TEX_MEM_ADDR         (0x800A2030)

#define BUILDING_2_MEM_ADDR             (0x800A4250)
#define BUILDING_2_TEX_MEM_ADDR         (0x800A0C80)
/*****************************************************/

#endif // WORLD_H