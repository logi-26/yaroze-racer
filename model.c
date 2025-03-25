#include <libps.h>
#include "graphics.h"
#include "model.h"


/*
// This function associates a model with our player datastructure
void LinkModelToTMD(ModelStruct *theModel, int nX, int nY, int nZ, unsigned long *lModelAddress) {
	// Increment the pointer to move past the model id. (weird huh?)
	lModelAddress++;
	
	// Map tmd data to its actual address
	GsMapModelingData((unsigned long *)lModelAddress);
	
	// Initialise the players coordinate system - set to be that of the world
	GsInitCoordinate2(WORLD, &theModel->gsObjectCoord);
	
	// Increment pointer twice more - to point to top of model data (beats me!)
	lModelAddress++;  
	lModelAddress++;
	
	// Link the model (tmd) with the players object handler
	GsLinkObject4((unsigned long *)lModelAddress, &theModel->gsObjectHandler, 0);
	
	// Assign the coordinates of the object model to the Object Handler
	theModel->gsObjectHandler.coord2 = &theModel->gsObjectCoord;
	
	// Set the initial position of the object
	theModel->gsObjectCoord.coord.t[0] = nX;   // X
	theModel->gsObjectCoord.coord.t[1] = nY;   // Y
	theModel->gsObjectCoord.coord.t[2] = nZ;   // Z

	// Setting the players gsObjectCoord.flg to 0 indicates it is to be drawn
	theModel->gsObjectCoord.flg = 0;
}
*/

void LinkModelToTMD(ModelStruct *theModel, int nX, int nY, int nZ, unsigned long *lModelAddress) {
	// Increment the pointer to move past the model ID
	lModelAddress++;

	// Map TMD data to its actual address
	GsMapModelingData((unsigned long *)lModelAddress);

	// Initialize the object's coordinate system (relative to WORLD)
	GsInitCoordinate2(WORLD, &theModel->gsObjectCoord);

	// Increment pointer twice more - to point to top of model data
	lModelAddress++;  
	lModelAddress++;

	// Link the model (TMD) with the object's handler
	GsLinkObject4((unsigned long *)lModelAddress, &theModel->gsObjectHandler, 0);

	// Assign the coordinate structure
	theModel->gsObjectHandler.coord2 = &theModel->gsObjectCoord;

	// Set the position
	theModel->gsObjectCoord.coord.t[0] = nX;   // X
	theModel->gsObjectCoord.coord.t[1] = nY;   // Y
	theModel->gsObjectCoord.coord.t[2] = nZ;   // Z

	// Mark the object to be drawn
	theModel->gsObjectCoord.flg = 0;
}



void InitialiseModel(ModelStruct *theModel, int nX, int nY, int nZ, int rotX, int rotY, int rotZ, unsigned long *lModelAddress) {

	// Initialise the models rotation 
	theModel->rotation.vx = 0;
	theModel->rotation.vy = 0;
	theModel->rotation.vz = 0;
	
	// Initialise speed to 0
	theModel->speed = 0;
	
	// Initialise other model variables and link in tmd
	LinkModelToTMD(theModel, nX, nY, nZ, lModelAddress);
}









// Setup matrices needed for rendering and send the object to the ordering table so it will be drawn
void DrawModel(ModelStruct *theModel, GsOT *othWorld) {
	MATRIX  tmpls, tmplw;
	
	// Get the local world and screen coordinates, needed for light calculations
	GsGetLws(theModel->gsObjectHandler.coord2, &tmplw, &tmpls);
	
	// Set the resulting light source matrix
	GsSetLightMatrix(&tmplw);
	
	// Set the local screen matrix for the GTE (so it works out perspective etc)
	GsSetLsMatrix(&tmpls);
	
	// Send Object To Ordering Table
	GsSortObject4(&theModel->gsObjectHandler, othWorld, 3, (u_long *)getScratchAddr(0));
}