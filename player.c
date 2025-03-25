#include <libps.h>
#include "graphics.h"
#include "player.h"

// This function associates a model with our player datastructure
// later we will want to add more than one model to the player to implement animation
void AddModelToPlayer(PlayerStruct *thePlayer, int nX, int nY, int nZ, unsigned long *lModelAddress) {
	// Increment the pointer to move past the model id. (weird huh?)
	lModelAddress++;
	
	// Map tmd data to its actual address
	GsMapModelingData((unsigned long *)lModelAddress);
	
	// Initialise the players coordinate system - set to be that of the world
	GsInitCoordinate2(WORLD, &thePlayer->gsObjectCoord);
	
	// Increment pointer twice more - to point to top of model data (beats me!)
	lModelAddress++;  
	lModelAddress++;
	
	// Link the model (tmd) with the players object handler
	GsLinkObject4((unsigned long *)lModelAddress, &thePlayer->gsObjectHandler, 0);
	
	// Assign the coordinates of the object model to the Object Handler
	thePlayer->gsObjectHandler.coord2 = &thePlayer->gsObjectCoord;
	
	// Setting the players gsObjectCoord.flg to 0 indicates it is to be drawn
	// Set the initial position of the object
	thePlayer->gsObjectCoord.coord.t[0] = nX;   // X
	thePlayer->gsObjectCoord.coord.t[1] = nY;   // Y
	thePlayer->gsObjectCoord.coord.t[2] = nZ;   // Z

	// Setting the players gsObjectCoord.flg to 0 indicates it is to be drawn
	thePlayer->gsObjectCoord.flg = 0;
}

void InitialisePlayer(PlayerStruct *thePlayer, int nX, int nY, int nZ, unsigned long *lModelAddress) {

	// Initialise the players rotation vector to 0
	thePlayer->rotation.vx = 0;
	thePlayer->rotation.vy = 0;
	thePlayer->rotation.vz = 0;
	
	// Initialise speed to 0
	thePlayer->speed = 0;
	
	// Initialise other player variables and link in tmd
	AddModelToPlayer(thePlayer, nX, nY, nZ, lModelAddress);
}

// This function deals with setting up matrices needed for rendering
// and sends the object to the ordering table so it will be drawn
void DrawPlayer(PlayerStruct *thePlayer, GsOT *othWorld) {
	MATRIX  tmpls, tmplw;
	
	// Get the local world and screen coordinates, needed for light calculations
	GsGetLws(thePlayer->gsObjectHandler.coord2, &tmplw, &tmpls);
	
	// Set the resulting light source matrix
	GsSetLightMatrix(&tmplw);
	
	// Set the local screen matrix for the GTE (so it works out perspective etc)
	GsSetLsMatrix(&tmpls);
	
	FntPrint(fontID_1, "POS x: %d, y: %d, z: %d\n", thePlayer->gsObjectCoord.coord.t[0], thePlayer->gsObjectCoord.coord.t[1], thePlayer->gsObjectCoord.coord.t[2]);
	FntPrint(fontID_1, "ROT x: %d, y: %d, z: %d\n", (int)thePlayer->rotation.vx, (int)thePlayer->rotation.vy, (int)thePlayer->rotation.vz);
	FntPrint(fontID_1, "speed: %d\n", thePlayer->speed);
	//FntFlush();
	
	// Send Object To Ordering Table
	GsSortObject4(&thePlayer->gsObjectHandler, othWorld, 3, (u_long *)getScratchAddr(0));
}