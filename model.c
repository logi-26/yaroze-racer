#include <libps.h>
#include "graphics.h"
#include "model.h"
#include "player.h"

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
	
	// Initialise the collision radius
	theModel->collisionRadius = 300;
	
	// Initialise other model variables and link in tmd
	LinkModelToTMD(theModel, nX, nY, nZ, lModelAddress);
}

// Rotate a model 90 degrees around Y axis
void RotateModel90(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector) {
	RotModel(gsObjectCoord, rotateVector, 0, 5125, 0);
}

// Rotate a model 180 degrees around Y axis
void RotateModel180(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector) {
	RotModel(gsObjectCoord, rotateVector, 0, 10250, 0);
}

// Rotate a model 270 degrees around Y axis
void RotateModel270(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector) {
	RotModel(gsObjectCoord, rotateVector, 0, 15375, 0);
}

// Rotate a model using X/Y/Z values
void RotModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, int nRX, int nRY, int nRZ) {
    MATRIX matTmp;

	// Update rotation vector
	rotateVector->vx = (rotateVector->vx + nRX) % ONE;
	rotateVector->vy = (rotateVector->vy + nRY) % ONE;
	rotateVector->vz = (rotateVector->vz + nRZ) % ONE;
	
	// Reset the coordinate system
	ResetMatrix(gsObjectCoord->coord.m);
	
	// Set up the rotation matrix
	RotMatrix(rotateVector, &matTmp);
	
	// Apply the rotation
	MulMatrix0(&gsObjectCoord->coord, &matTmp, &gsObjectCoord->coord);
	
	// Mark for redraw
	gsObjectCoord->flg = 0;
}

// Only draw the model if it is near to the player
void DrawModelCulled(PlayerStruct *currentPlayer, ModelStruct *model, int currentBuffer) {
    if (model && IsObjectNearPlayer(currentPlayer, &model->gsObjectCoord)) {
        DrawModel(model, &OTable_Header[currentBuffer]);
    }
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