#ifndef WORLD_H
#define WORLD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libps.h>
#include "player.h"
#include "model.h"


// ******************************************************************	

#define LINE_L_MEM_ADDR                     (0x80093000)
#define LINE_L_TEX_MEM_ADDR                 (0x80093070)  // +112B (0x70)

#define LINE_R_MEM_ADDR                     (0x800942CC)  // +4,700B (0x125C)
#define LINE_R_TEX_MEM_ADDR                 (0x8009433C)  // +112B (0x70)

#define STRAIGHT_L_1_MEM_ADDR               (0x80095598)  // +4,700B (0x125C)
#define STRAIGHT_L_1_TEX_MEM_ADDR           (0x80095608)  // +112B (0x70)

#define STRAIGHT_L_2_MEM_ADDR               (0x80096864)  // +4,700B (0x125C)
#define STRAIGHT_L_2_TEX_MEM_ADDR           (0x800968D4)  // +112B (0x70)

#define STRAIGHT_R_1_MEM_ADDR               (0x80097B30)  // +4,700B (0x125C)
#define STRAIGHT_R_1_TEX_MEM_ADDR           (0x80097BA0)  // +112B (0x70)

#define STRAIGHT_R_2_MEM_ADDR               (0x80098DFC)  // Starts after STRAIGHT_R_1_TEX
#define STRAIGHT_R_2_TEX_MEM_ADDR           (0x80098E6C)  // +112B (0x70)

#define TURN_L_1_MEM_ADDR                   (0x8009A0C8)  // +4,700B (0x125C)
#define TURN_L_1_TEX_MEM_ADDR               (0x8009A138)  // +112B (0x70)

#define TURN_L_2_MEM_ADDR                   (0x8009B394)  // +4,700B (0x125C)
#define TURN_L_2_TEX_MEM_ADDR               (0x8009B404)  // +112B (0x70)

#define TURN_L_3_MEM_ADDR                   (0x8009C660)  // +4,700B (0x125C)
#define TURN_L_3_TEX_MEM_ADDR               (0x8009C6D0)  // +112B (0x70)

#define TURN_L_4_MEM_ADDR                   (0x8009D92C)  // +4,700B (0x125C)
#define TURN_L_4_TEX_MEM_ADDR               (0x8009D99C)  // +112B (0x70)

#define TURN_R_1_MEM_ADDR                   (0x8009EBF8)  // +4,700B (0x125C)
#define TURN_R_1_TEX_MEM_ADDR               (0x8009EC68)  // +112B (0x70)

#define TURN_R_2_MEM_ADDR                   (0x8009FEC4)  // +4,700B (0x125C)
#define TURN_R_2_TEX_MEM_ADDR               (0x8009FF34)  // +112B (0x70)

#define TURN_R_3_MEM_ADDR                   (0x800A1190)  // +4,700B (0x125C)
#define TURN_R_3_TEX_MEM_ADDR               (0x800A1200)  // +112B (0x70)

#define TURN_R_4_MEM_ADDR                   (0x800A245C)  // +4,700B (0x125C)
#define TURN_R_4_TEX_MEM_ADDR               (0x800A24CC)  // +112B (0x70)

#define GRID_R_MEM_ADDR                     (0x800A49F4)  // +4,700B (0x125C)
#define GRID_R_TEX_MEM_ADDR                 (0x800A4A64)  // +112B (0x70)

#define GRID_L_MEM_ADDR                     (0x800A5CC0)  // +4,700B (0x125C)
#define GRID_L_TEX_MEM_ADDR                 (0x800A5D30)  // +112B (0x70)

#define STRAIGHT_L_01_MEM_ADDR              (0x800A6F8C)  // +4,700B (0x125C)
#define STRAIGHT_L_01_TEX_MEM_ADDR          (0x800A6FFC)  // +112B (0x70)

#define STRAIGHT_L_02_MEM_ADDR              (0x800A8258)  // +4,700B (0x125C)
#define STRAIGHT_L_02_TEX_MEM_ADDR          (0x800A82C8)  // +112B (0x70)

#define STRAIGHT_R_01_MEM_ADDR              (0x800A9524)  // +4,700B (0x125C)
#define STRAIGHT_R_01_TEX_MEM_ADDR          (0x800A9594)  // +112B (0x70)

#define STRAIGHT_R_02_MEM_ADDR              (0x800AA7F0)  // +4,700B (0x125C)
#define STRAIGHT_R_02_TEX_MEM_ADDR          (0x800AA860)  // +112B (0x70)

#define GRASS_MEM_ADDR                      (0x800ABABC)  // +4,700B (0x125C)
#define GRASS_TEX_MEM_ADDR                  (0x800ABB2C)  // +112B (0x70)     


#define BARRIER_2_MEM_ADDR                 	(0x800ADB2C)



//#define BUILDING_MEM_ADDR                   (GRASS_TEX_MEM_ADDR + 4096) 		// 0x800A2690
//#define BUILDING_TEX_MEM_ADDR               (BUILDING_MEM_ADDR + 400) 			// 0x800A2820


//#define BUILDING_2_MEM_ADDR                 (BUILDING_TEX_MEM_ADDR + 8192) 		// 0x800A4820
//#define BUILDING_2_TEX_MEM_ADDR             (BUILDING_2_MEM_ADDR + 400) 		// 0x800A49B0

//#define BARRIER_1_MEM_ADDR                 (BUILDING_2_TEX_MEM_ADDR + 8192) 	// 0x800A69B0


// ******************************************************************


#define MAX_NO_PACKETS (248000)

#define WORLD_GROUND_MAX_X (30)
#define WORLD_GROUND_MAX_Z (30)


#define MAX_WORLD_OBJECTS (2000)
#define SEPERATION (1200)

extern GsRVIEW2 viewTop, viewBottom;		// Two separate views for top and bottom viewports
extern u_long vsyncInterval;

typedef struct {
	int nTotalModels;									// A variable to track the total number of models in the world
	GsDOBJ2 gsObjectHandler[MAX_WORLD_OBJECTS];			// We need one of each of the following two data structures for each object in the world
	GsCOORDINATE2 gsObjectCoord[MAX_WORLD_OBJECTS];
	unsigned long *lObjectPointer[MAX_WORLD_OBJECTS];	// We also need an array of pointers to tmds in memory

} WorldStruct;

extern WorldStruct theWorld;

/************* FUNCTION PROTOTYPES *******************/
int LoadTexture(long addr);
void InitialiseWorld ();
void AddModelToWorld(WorldStruct *theWorld, int nX, int nY, int nZ, unsigned long *lModelAddress);
void DrawWorld(WorldStruct *theWorld, GsOT *othWorld);
void RenderWorld();

void InitialiseWorldTextures();
void InitialiseWorldModels();
/*****************************************************/

#endif // WORLD_H