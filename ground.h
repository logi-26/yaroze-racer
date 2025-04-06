#ifndef GROUND_H
#define GROUND_H

#include <libps.h>
#include "player.h"

// Size of the ground
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

/************* FUNCTION PROTOTYPES *******************/
void InitialiseGroundTextures();
void InitialiseGround();
void AddModelToGround(GroundStruct *theGround, int nX, int nY, int nZ, unsigned long *lModelAddress);
void RotateGround90(GroundStruct *theGround);
void RotateGround180(GroundStruct *theGround);
void RotateGround270(GroundStruct *theGround);
void RotateGround(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, int nRX, int nRY, int nRZ);
void DrawGround(GroundStruct *theGround, PlayerStruct* currentPlayer, GsOT *othWorld);
/*****************************************************/

/*****************************************************/
// Ground model and texture memory addresses
/*****************************************************/
#define LINE_L_MEM_ADDR                     (0x80093000)
#define LINE_L_TEX_MEM_ADDR                 (0x80093070)  // +112B (0x70)

#define STRAIGHT_L_1_MEM_ADDR               (0x800942CC)  // +4,700B (0x125C)
#define STRAIGHT_L_1_TEX_MEM_ADDR           (0x8009433C)  // +112B (0x70)

#define TURN_L_1_MEM_ADDR                   (0x80095598)  // +4,700B (0x125C)
#define TURN_L_1_TEX_MEM_ADDR               (0x80095608)  // +112B (0x70)

#define TURN_00_MEM_ADDR                    (0x80096864)  // +4,700B (0x125C)
#define TURN_00_TEX_MEM_ADDR                (0x800968D4)  // +112B (0x70)

#define TURN_01_MEM_ADDR                    (0x80098DFC)  // +4,700B (0x125C)
#define TURN_01_TEX_MEM_ADDR                (0x80098E6C)  // +112B (0x70)

#define TURN_02_MEM_ADDR                    (0x8009B394)  // +4,700B (0x125C)
#define TURN_02_TEX_MEM_ADDR                (0x8009B404)  // +112B (0x70)

#define TURN_R_1_MEM_ADDR                   (0x8009FEC4)  // +4,700B (0x125C)
#define TURN_R_1_TEX_MEM_ADDR               (0x8009FF34)  // +112B (0x70)

#define GRID_MEM_ADDR                       (0x800A5CC0)  // +4,700B (0x125C)
#define GRID_TEX_MEM_ADDR                   (0x800A5D30)  // +112B (0x70)

#define STRAIGHT_L_01_MEM_ADDR              (0x800A6F8C)  // +4,700B (0x125C)
#define STRAIGHT_L_01_TEX_MEM_ADDR          (0x800A6FFC)  // +112B (0x70)

#define GRASS_MEM_ADDR                      (0x800A8258)  // +4,700B (0x125C)
#define GRASS_TEX_MEM_ADDR                  (0x800A82C8)  // +112B (0x70)     
/*****************************************************/	

#endif // GROUND_H