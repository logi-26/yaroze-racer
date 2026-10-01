#ifndef PLAYER_H
#define PLAYER_H

#include "vehicle_attribs.h"


// Struct for a player
typedef struct {
	long speed;
	int playerNumber;
	SVECTOR rotation;
	GsDOBJ2 gsObjectHandler;
	GsCOORDINATE2 gsObjectCoord;
	GsCOORDINATE2 gsModelCoord;
	int collisionRadius;
} PlayerStruct;


// Define the players
extern PlayerStruct player1;
extern PlayerStruct player2;

// Height (Y) the cars are placed at: the car models are built up from their
// origin, with the bottom of the tyres at model Z -4 to -6, so this puts the
// tyres on the ground (the ground tiles are at Y = 0)
#define CAR_GROUND_Y (-6)

// Index of the vehicle chosen on the vehicle select screen
// 0-2 = car3 (green/red/yellow), 3-5 = car2 (black/blue/red)
extern int selectedVehicleIndex;
extern int remoteVehicleIndex;


/************* FUNCTION PROTOTYPES *******************/
void InitialisePlayer(PlayerStruct *thePlayer, int playerNumber, int nX, int nY, int nZ, unsigned long *lModelAddress);
void AddModelToPlayer(PlayerStruct *thePlayer, int nX, int nY, int nZ, unsigned long *lModelAddress);
void DrawPlayer(PlayerStruct *thePlayer, GsOT *othWorld);
int IsObjectNearPlayer(PlayerStruct* player, GsCOORDINATE2* objectCoord);
int IsObjectWithinDist(PlayerStruct* player, GsCOORDINATE2* objectCoord, long thresholdSq);
int IsObjectInView(PlayerStruct *player, GsCOORDINATE2 *objectCoord, long ahead, long behind, long side);
int IsPointInView(PlayerStruct *player, long x, long z, long ahead, long behind, long side);
/*****************************************************/


/*****************************************************/
// Player model and texture memory addresses
/*****************************************************/
#define CAR_3Y_MEM_ADDR         (0x800CC4A0)  // car3 yellow TMD
#define CAR_3Y_TEX_MEM_ADDR     (0x800C8280)  // car3 base TIM

#define CAR_2B_MEM_ADDR         (0x800D0670)  // car2 blue TMD
#define CAR_2B_TEX_MEM_ADDR     (0x800D3150)  // car2 blue TIM

#define CAR_5G_MEM_ADDR         (0x80098BB0)  // car5 green TMD
#define CAR_5G_TEX_MEM_ADDR     (0x8009CA60)  // car5 green TIM
/*****************************************************/

#endif // PLAYER_H