#ifndef MODEL_H
#define MODEL_H

typedef struct {
	long speed;
	SVECTOR rotation;
	GsDOBJ2 gsObjectHandler;
	GsCOORDINATE2 gsObjectCoord;
} ModelStruct;

extern ModelStruct theBuilding;

/************* FUNCTION PROTOTYPES *******************/
void InitialiseModel(ModelStruct *theModel, int nX, int nY, int nZ, int rotX, int rotY, int rotZ, unsigned long *lModelAddress);
void LinkModelToTMD(ModelStruct *theModel, int nX, int nY, int nZ, unsigned long *lModelAddress);
void DrawModel(ModelStruct *theModel, GsOT *othWorld);
/*****************************************************/

#endif // MODEL_H