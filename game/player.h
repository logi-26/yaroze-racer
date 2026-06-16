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

// Index of the vehicle chosen on the vehicle select screen
// 0-2 = car3 (green/red/yellow), 3-5 = car2 (black/blue/red)
extern int selectedVehicleIndex;


/************* FUNCTION PROTOTYPES *******************/
void InitialisePlayer(PlayerStruct *thePlayer, int playerNumber, int nX, int nY, int nZ, unsigned long *lModelAddress);
void AddModelToPlayer(PlayerStruct *thePlayer, int nX, int nY, int nZ, unsigned long *lModelAddress);
void DrawPlayer(PlayerStruct *thePlayer, GsOT *othWorld);
int IsObjectNearPlayer(PlayerStruct* player, GsCOORDINATE2* objectCoord);
int IsObjectWithinDist(PlayerStruct* player, GsCOORDINATE2* objectCoord, long thresholdSq);
/*****************************************************/


/*****************************************************/
// Player model and texture memory addresses
/*****************************************************/
#define CAR_3Y_MEM_ADDR         (0x80118000)  // car3 yellow TMD
#define CAR_3Y_TEX_MEM_ADDR     (0x80113000)  // car3 base TIM

#define CAR_2B_MEM_ADDR         (0x80125000)  // car2 blue TMD
#define CAR_2B_TEX_MEM_ADDR     (0x80128000)  // car2 blue TIM

#define CAR_5G_MEM_ADDR         (0x800B9000)  // car5 green TMD
#define CAR_5G_TEX_MEM_ADDR     (0x800BD000)  // car5 green TIM
/*****************************************************/

#endif // PLAYER_H