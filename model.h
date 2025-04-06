#ifndef MODEL_H
#define MODEL_H

typedef struct {
	long speed;
	SVECTOR rotation;
	GsDOBJ2 gsObjectHandler;
	GsCOORDINATE2 gsObjectCoord;
	int collisionRadius;
} ModelStruct;

/************* FUNCTION PROTOTYPES *******************/
void InitialiseModel(ModelStruct *theModel, int nX, int nY, int nZ, int rotX, int rotY, int rotZ, unsigned long *lModelAddress);
void LinkModelToTMD(ModelStruct *theModel, int nX, int nY, int nZ, unsigned long *lModelAddress);
void RotateModel90(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector);
void RotateModel180(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector);
void RotateModel270(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector);
void DrawModelCulled(PlayerStruct *currentPlayer, ModelStruct *model, int currentBuffer);
void DrawModel(ModelStruct *theModel, GsOT *othWorld);
/*****************************************************/

#endif // MODEL_H