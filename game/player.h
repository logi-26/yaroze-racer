#ifndef PLAYER_H
#define PLAYER_H

#define MAX_SPEED 150
#define ACCELERATION 3
#define DECELERATION 2              // Natural coast deceleration
#define BRAKE_DECELERATION 6       // Hard braking (only applied when brake button held)
#define MAX_REVERSE_SPEED -150
#define STEERING_RESPONSE 120       // Steering sensitivity (higher value steers vehicle faster)
#define DRIFT_FACTOR 400            // Oversteer (higher value is more drift)
#define TRACTION 400                // Steering damping (higher value for tighter handling in corners)

#define MIN_STEERING_RESPONSE 80
#define MAX_GRIP 200                // Lateral speed limit before slide-out at low speed
#define MIN_GRIP 60                 // Lateral speed limit before slide-out at max speed
#define SLIDING_FRICTION 840        // Kinetic friction while sliding (lower = longer slide)
#define TURN_RADIUS_FACTOR 2048     // Cornering momentum scale (lower = more slip per turn)


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
#define CAR_2_MEM_ADDR          (0x80113000)
#define CAR_3_MEM_ADDR          (0x800F5000)
#define CAR_3_TEX_MEM_ADDR      (0x80122000)
/*****************************************************/

#endif // PLAYER_H