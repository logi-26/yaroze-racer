#ifndef ENGINE_MODEL_H
#define ENGINE_MODEL_H

#include <libps.h>

typedef struct {
    long speed;
    SVECTOR rotation;
    GsDOBJ2 gsObjectHandler;
    GsCOORDINATE2 gsObjectCoord;
    int collisionRadius;
} ModelStruct;

// Runtime polygon subdivision
typedef struct {
    long distance;
    u_long division;
} DivisionStep;

void LinkModelToTMD(ModelStruct *theModel, int nX, int nY, int nZ, unsigned long *lModelAddress);
void InitialiseModel(ModelStruct *theModel, int nX, int nY, int nZ, int rotX, int rotY, int rotZ, unsigned long *lModelAddress);
void RotModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, int nRX, int nRY, int nRZ);
void RotateModel90(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector);
void RotateModel180(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector);
void RotateModel270(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector);
void DrawModel(ModelStruct *theModel, GsOT *othWorld);
void SetDivisionByDistance(GsDOBJ2 *obj, long dx, long dz, const DivisionStep *steps);

#endif