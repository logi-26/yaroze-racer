#ifndef GROUND_H
#define GROUND_H

#include <libps.h>
#include "game/player.h"


// Size of the ground map in tiles (64x64)
#define GROUND_MAX_X (64)
#define GROUND_MAX_Z (64)


// Separation between ground tiles
#define SEPERATION (1200)


// The tracks (map_1.h to map_6.h), and which one to use (0 = map_1)
#define NUM_TRACKS (6)
extern int selectedTrackIndex;

// Whether the race is at night (1) or in the day (0)
extern int nightMode;

// The walled-in area of the world (from the map file)
extern long trackArenaX;
extern long trackArenaZ;
extern long trackArenaSize;


// An AI waypoint (world X, Z)
typedef struct { long x; long z; } Waypoint;

// A barrier (world X, Z), one-sided
// rot:   0 / 1 = along Z facing +X / -X,  2 / 3 = along X facing +Z / -Z
// model: 0 = BARRIER_1,  1 = BARRIER_2
typedef struct { int x; int z; int rot; int model; } BarrierDef;

// A building (world X, Y, Z)
// rot:   0 = no rotation,  1 = 180-deg
// model: 0 = BUILDING_1,   1 = BUILDING_2,  2 = STAND
typedef struct { int x; int y; int z; int rot; int model; int collisionRadius; } BuildingDef;

// A start position (world X, Z) and heading (0-4095: 0 = facing +Z, 1024 = facing +X)
typedef struct { int x; int z; int heading; } StartDef;

// The start positions of a track, player 1, player 2 and the the AI racers
#define NUM_STARTS (7)
#define START_PLAYER1 (0)
#define START_PLAYER2 (1)
#define START_AI (2)

// The most barriers and buildings a track can have
#define MAX_TRACK_BARRIERS (600)
#define MAX_TRACK_BUILDINGS (120)

// The selected track's AI waypoints, in race order (made in the map editor, set by InitialiseGround)
// (for the race the start line is added to them as the last waypoint, so the laps change at the line)
extern const Waypoint *trackWaypoints;
extern int numTrackWaypoints;

// The selected track's barriers and buildings (the arena walls, the buildings around it
// and everything inside it)
extern const BarrierDef *trackBarriers;
extern int numTrackBarriers;
extern const BuildingDef *trackBuildings;
extern int numTrackBuildings;

// The selected track's start positions
extern const StartDef *trackStarts;


typedef enum {
    TERRAIN_TRACK = 0,
    TERRAIN_GRASS = 1,
    TERRAIN_SAND  = 2,
} TerrainType;

/************* FUNCTION PROTOTYPES *******************/
void InitialiseGroundTextures();
void SelectTrack();
char (*TrackTiles(int track))[GROUND_MAX_X];
const StartDef *TrackStarts(int track);
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