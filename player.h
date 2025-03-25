#ifndef PLAYER_H
#define PLAYER_H

// Memory addresses
#define CAR_MEM_ADDR 			(0x80090000)
#define CAR_TEX_MEM_ADDR 		(0x800A69C0)

#define MAX_SPEED 500  
#define ACCELERATION 5  
#define DECELERATION 3 
#define MAX_REVERSE_SPEED -100


typedef struct {
	long speed;
	SVECTOR rotation;
	GsDOBJ2 gsObjectHandler;
	GsCOORDINATE2 gsObjectCoord;
} PlayerStruct;

extern PlayerStruct theCar;

extern PlayerStruct theBuilding;

/************* FUNCTION PROTOTYPES *******************/
void InitialisePlayer(PlayerStruct *thePlayer, int nX, int nY, int nZ, unsigned long *lModelAddress);
void AddModelToPlayer(PlayerStruct *thePlayer, int nX, int nY, int nZ, unsigned long *lModelAddress);
void DrawPlayer(PlayerStruct *thePlayer, GsOT *othWorld);
/*****************************************************/

#endif // PLAYER_H