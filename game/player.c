#include <libps.h>
#include "../engine/graphics.h"
#include "player.h"
#include "game.h"

int selectedVehicleIndex = 0;

// Hatchback car (car3) FWD
VehicleAttributes car3Attribs = {
    240,   // maxSpeed
    3,     // acceleration
    2,     // deceleration
    6,     // brakeDeceleration
    -100,  // maxReverseSpeed
    120,   // steeringResponse
    80,    // minSteeringResponse
    200,   // maxGrip
    60,    // minGrip
    840,   // slidingFriction
    920,   // lateralFriction
    2048,  // turnRadiusFactor
    15     // understeerBias (FWD: front loses grip first)
};

// Saloon car (car2) neutral balance
VehicleAttributes car2Attribs = {
    340,   // maxSpeed
    5,     // acceleration
    2,     // deceleration
    10,    // brakeDeceleration
    -60,   // maxReverseSpeed
    150,   // steeringResponse
    100,   // minSteeringResponse
    140,   // maxGrip
    30,    // minGrip
    750,   // slidingFriction
    880,   // lateralFriction
    1600,  // turnRadiusFactor
    0      // understeerBias (neutral)
};

// American car (car5) RWD
VehicleAttributes car5Attribs = {
    400,   // maxSpeed
    6,     // acceleration
    2,     // deceleration
    12,    // brakeDeceleration
    -50,   // maxReverseSpeed
    160,   // steeringResponse
    110,   // minSteeringResponse
    100,   // maxGrip
    20,    // minGrip
    700,   // slidingFriction
    850,   // lateralFriction
    1400,  // turnRadiusFactor
    -25    // understeerBias (RWD: rear loses grip first)
};

VehicleAttributes *activeVehicle = &car3Attribs;


// This function associates a model with our player datastructure
// later we will want to add more than one model to the player to implement animation
void AddModelToPlayer(PlayerStruct *thePlayer, int nX, int nY, int nZ, unsigned long *lModelAddress) {
	// Increment the pointer to move past the model id. (weird huh?)
	lModelAddress++;
	
	// Map tmd data to its actual address
	GsMapModelingData((unsigned long *)lModelAddress);
	
	// Initialise the players coordinate system - set to be that of the world
	GsInitCoordinate2(WORLD, &thePlayer->gsObjectCoord);

	// Initialise a child coordinate for the visual model (separate from the camera parent)
	GsInitCoordinate2(&thePlayer->gsObjectCoord, &thePlayer->gsModelCoord);
	thePlayer->gsModelCoord.flg = 0;

	// Increment pointer twice more - to point to top of model data (beats me!)
	lModelAddress++;
	lModelAddress++;

	// Link the model (tmd) with the players object handler
	GsLinkObject4((unsigned long *)lModelAddress, &thePlayer->gsObjectHandler, 0);

	// Assign the model coordinate (child) to the Object Handler so RotModel on it
	// does not affect the camera, which is parented to gsObjectCoord
	thePlayer->gsObjectHandler.coord2 = &thePlayer->gsModelCoord;
	
	// Setting the players gsObjectCoord.flg to 0 indicates it is to be drawn
	// Set the initial position of the object
	thePlayer->gsObjectCoord.coord.t[0] = nX;   // X
	thePlayer->gsObjectCoord.coord.t[1] = nY;   // Y
	thePlayer->gsObjectCoord.coord.t[2] = nZ;   // Z

	// Setting the players gsObjectCoord.flg to 0 indicates it is to be drawn
	thePlayer->gsObjectCoord.flg = 0;
}


void InitialisePlayer(PlayerStruct *thePlayer, int playerNumber, int nX, int nY, int nZ, unsigned long *lModelAddress) {

	thePlayer->playerNumber = playerNumber;

	// Initialise the players rotation vector to 0
	thePlayer->rotation.vx = 0;
	thePlayer->rotation.vy = 0;
	thePlayer->rotation.vz = 0;
	
	// Initialise speed to 0
	thePlayer->speed = 0;
	
	// Initialise the collision radius
	thePlayer->collisionRadius = 300;
	
	// Initialise other player variables and link in tmd
	AddModelToPlayer(thePlayer, nX, nY, nZ, lModelAddress);
}


void DrawPlayer(PlayerStruct *thePlayer, GsOT *othWorld) {
	MATRIX tmpls, tmplw;

	GsGetLws(thePlayer->gsObjectHandler.coord2, &tmplw, &tmpls);
	GsSetLightMatrix(&tmplw);
	GsSetLsMatrix(&tmpls);

	GsSortObject4(&thePlayer->gsObjectHandler, othWorld, 3, (u_long *)getScratchAddr(0));
}


int IsObjectNearPlayer(PlayerStruct* player, GsCOORDINATE2* objectCoord) {
    int distanceSq = CalculateDistanceSquared(
        player->gsObjectCoord.coord.t[0],
        player->gsObjectCoord.coord.t[1],
        player->gsObjectCoord.coord.t[2],
        objectCoord->coord.t[0],
        objectCoord->coord.t[1],
        objectCoord->coord.t[2]
    );
    return (distanceSq <= DISTANCE_THRESHOLD_SQUARED);
}