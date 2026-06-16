#include <libps.h>
#include "../engine/graphics.h"
#include "player.h"
#include "game.h"

int selectedVehicleIndex = 0;

// Hatchback car (car3)
VehicleAttributes car3Attribs = {
    300,   // maxSpeed
    5,     // acceleration
    2,     // deceleration
    3,     // brakeDeceleration
    -100,  // maxReverseSpeed
    160,   // steeringResponse
    110,   // minSteeringResponse
    150,   // maxGrip
    50,    // minGrip
    780,   // slidingFriction
    900,   // lateralFriction
    2048,  // turnRadiusFactor
    20     // understeerBias
};

// Saloon car (car2)
VehicleAttributes car2Attribs = {
    310,   // maxSpeed
    6,     // acceleration
    2,     // deceleration
    3,     // brakeDeceleration
    -90,   // maxReverseSpeed
    150,   // steeringResponse
    100,   // minSteeringResponse
    130,   // maxGrip
    30,    // minGrip
    750,   // slidingFriction
    880,   // lateralFriction
    1800,  // turnRadiusFactor
    -20    // understeerBias
};

// American car (car5)
VehicleAttributes car5Attribs = {
    340,   // maxSpeed
    4,     // acceleration
    2,     // deceleration
    2,     // brakeDeceleration
    -80,   // maxReverseSpeed
    110,   // steeringResponse
    90,    // minSteeringResponse
    100,   // maxGrip
    20,    // minGrip
    700,   // slidingFriction
    850,   // lateralFriction
    1600,  // turnRadiusFactor
    -25    // understeerBias
};

VehicleAttributes *activeVehicle = &car3Attribs;


// This function associates a model with our player datastructure
void AddModelToPlayer(PlayerStruct *thePlayer, int nX, int nY, int nZ, unsigned long *lModelAddress) {
	// Increment the pointer to move past the model id
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

int IsObjectWithinDist(PlayerStruct* player, GsCOORDINATE2* objectCoord, long thresholdSq) {
    long distanceSq = CalculateDistanceSquared(
        player->gsObjectCoord.coord.t[0],
        player->gsObjectCoord.coord.t[1],
        player->gsObjectCoord.coord.t[2],
        objectCoord->coord.t[0],
        objectCoord->coord.t[1],
        objectCoord->coord.t[2]
    );
	
    return (distanceSq <= thresholdSq);
}