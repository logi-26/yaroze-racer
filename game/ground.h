#ifndef GROUND_H
#define GROUND_H

#include <libps.h>
#include "game/player.h"


// Size of the ground map in tiles (30x30)
#define GROUND_MAX_X (30)
#define GROUND_MAX_Z (30)


// Seperation between ground tiles
#define SEPERATION (1200)


// Which track map to use (0 = map_1, 1 = map_2)
extern int selectedTrackIndex;


typedef enum {
    TERRAIN_TRACK = 0,
    TERRAIN_GRASS = 1,
    TERRAIN_SAND  = 2,
} TerrainType;

/************* FUNCTION PROTOTYPES *******************/
void InitialiseGroundTextures();
void InitialiseGround();
void DrawGround(PlayerStruct *currentPlayer, GsOT *ot);
int IsOnStartLine(long worldX, long worldZ);
TerrainType GetTerrainType(long worldX, long worldZ);
/*****************************************************/


/*****************************************************/
// Ground model and texture memory addresses
/*****************************************************/
#define LINE_L_MEM_ADDR                     (0x80090000)
#define LINE_L_TEX_MEM_ADDR                 (0x80090070)
#define LINE_R_MEM_ADDR                     LINE_L_MEM_ADDR
#define LINE_R_TEX_MEM_ADDR                 LINE_L_TEX_MEM_ADDR

#define STRAIGHT_L_1_MEM_ADDR               (0x800908B0)
#define STRAIGHT_L_1_TEX_MEM_ADDR           (0x80090920)

#define TURN_L_1_MEM_ADDR                   (0x80091160)
#define TURN_L_1_TEX_MEM_ADDR               (0x800911D0)

#define TURN_00_MEM_ADDR                    (0x80091A10)
#define TURN_00_TEX_MEM_ADDR                (0x80091A80)

#define TURN_01_MEM_ADDR                    (0x800922C0)
#define TURN_01_TEX_MEM_ADDR                (0x80092330)

#define TURN_02_MEM_ADDR                    (0x80092B70)
#define TURN_02_TEX_MEM_ADDR                (0x80092BE0)

#define TURN_R_1_MEM_ADDR                   (0x80093420)
#define TURN_R_1_TEX_MEM_ADDR               (0x80093490)

#define GRID_MEM_ADDR                       (0x80093CD0)
#define GRID_TEX_MEM_ADDR                   (0x80093D40)

#define STRAIGHT_L_01_MEM_ADDR              (0x80094580)
#define STRAIGHT_L_01_TEX_MEM_ADDR          (0x800945F0)

#define GRASS_MEM_ADDR                      (0x80094E30)
#define GRASS_TEX_MEM_ADDR                  (0x80094EA0)

#define SAND_MEM_ADDR                      (0x800956E0)
#define SAND_TEX_MEM_ADDR                  (0x80095750)

/*****************************************************/	

#endif // GROUND_H