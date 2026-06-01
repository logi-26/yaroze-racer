#ifndef GROUND_H
#define GROUND_H

#include <libps.h>
#include "game/player.h"


// Size of the ground (900 ground tiles: 30x30 tile ground)
#define MAX_GROUND_OBJECTS (900)
#define GROUND_MAX_X (30)
#define GROUND_MAX_Z (30)


// Seperation between ground tiles
#define SEPERATION (1200)


// Array for the ground tiles
extern char groundDataTrack1[GROUND_MAX_Z][GROUND_MAX_X];


// Struct to store the ground data 
typedef struct {
	int nTotalModels;										
	GsDOBJ2 gsObjectHandler[MAX_GROUND_OBJECTS];			
	GsCOORDINATE2 gsObjectCoord[MAX_GROUND_OBJECTS];
	unsigned long *lObjectPointer[MAX_GROUND_OBJECTS];

} GroundStruct;


// Create an instance of the ground
extern GroundStruct theGround;


typedef enum {
    TERRAIN_TRACK = 0,
    TERRAIN_GRASS = 1,
    TERRAIN_SAND  = 2,
} TerrainType;

/************* FUNCTION PROTOTYPES *******************/
void InitialiseGroundTextures();
void InitialiseGround();
void AddModelToGround(GroundStruct *theGround, int nX, int nY, int nZ, unsigned long *lModelAddress);
void RotateGround90(GroundStruct *theGround);
void RotateGround180(GroundStruct *theGround);
void RotateGround270(GroundStruct *theGround);
void RotateGround(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, int nRX, int nRY, int nRZ);
void DrawGround(GroundStruct *theGround, PlayerStruct* currentPlayer, GsOT *ot);
int IsOnStartLine(long worldX, long worldZ);
TerrainType GetTerrainType(long worldX, long worldZ);
/*****************************************************/


/*****************************************************/
// Ground model and texture memory addresses
/*****************************************************/
#define LINE_L_MEM_ADDR                     (0x80090000)
#define LINE_L_TEX_MEM_ADDR                 (0x80091000)
#define LINE_R_MEM_ADDR                     LINE_L_MEM_ADDR
#define LINE_R_TEX_MEM_ADDR                 LINE_L_TEX_MEM_ADDR

#define STRAIGHT_L_1_MEM_ADDR               (0x80092000)
#define STRAIGHT_L_1_TEX_MEM_ADDR           (0x80093000)

#define TURN_L_1_MEM_ADDR                   (0x80094000)
#define TURN_L_1_TEX_MEM_ADDR               (0x80095000)

#define TURN_00_MEM_ADDR                    (0x80096000)
#define TURN_00_TEX_MEM_ADDR                (0x80097000)

#define TURN_01_MEM_ADDR                    (0x80098000)
#define TURN_01_TEX_MEM_ADDR                (0x80099000)

#define TURN_02_MEM_ADDR                    (0x8009A000)
#define TURN_02_TEX_MEM_ADDR                (0x8009B000)

#define TURN_R_1_MEM_ADDR                   (0x8009C000)
#define TURN_R_1_TEX_MEM_ADDR               (0x8009D000)

#define GRID_MEM_ADDR                       (0x8009E000)
#define GRID_TEX_MEM_ADDR                   (0x8009F000)

#define STRAIGHT_L_01_MEM_ADDR              (0x800A0000)
#define STRAIGHT_L_01_TEX_MEM_ADDR          (0x800A1000)

#define GRASS_MEM_ADDR                      (0x800A2000)
#define GRASS_TEX_MEM_ADDR                  (0x800A3000)

#define SAND_MEM_ADDR                      (0x800A4000)
#define SAND_TEX_MEM_ADDR                  (0x800A5000)

/*****************************************************/	

#endif // GROUND_H