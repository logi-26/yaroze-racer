#ifndef PLAYER_H
#define PLAYER_H

#define MAX_SPEED 500
#define ACCELERATION 8
#define DECELERATION 6				// How quickly the vehicle slows down (higher value slows vehicle faster)
#define BRAKE_DECELERATION 8		// How strong are the brakes (higher value brakes harder)
#define MAX_REVERSE_SPEED -150
#define STEERING_RESPONSE 300		// Steering sensitivity (higher value steers vehicle faster)
#define DRIFT_FACTOR 400			// Oversteer (higher value is more drift)
#define TRACTION 400				// Steering damping (higher value for tighter handling in corners)

#define MIN_STEERING_RESPONSE 50

typedef struct {
	long speed;
	int playerNumber;
	SVECTOR rotation;
	GsDOBJ2 gsObjectHandler;
	GsCOORDINATE2 gsObjectCoord;
	int collisionRadius;
} PlayerStruct;

// Define players
extern PlayerStruct player1;
extern PlayerStruct player2;

/************* FUNCTION PROTOTYPES *******************/
void InitialisePlayer(PlayerStruct *thePlayer, int playerNumber, int nX, int nY, int nZ, unsigned long *lModelAddress);
void AddModelToPlayer(PlayerStruct *thePlayer, int nX, int nY, int nZ, unsigned long *lModelAddress);
void DrawPlayer(PlayerStruct *thePlayer, GsOT *othWorld);
int IsObjectNearPlayer(PlayerStruct* player, GsCOORDINATE2* objectCoord);
/*****************************************************/

/*****************************************************/
// Player model and texture memory addresses
/*****************************************************/
//#define CAR_MEM_ADDR 			(0x80090000)
//#define CAR_TEX_MEM_ADDR 		(0x800A69C0)

#define CAR_MEM_ADDR            (0x800F5000)
#define CAR_2_MEM_ADDR          (0x80104000)
/*****************************************************/

#endif // PLAYER_H