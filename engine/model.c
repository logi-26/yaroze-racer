#include <libps.h>
#include "model.h"
#include "calculations.h"

void LinkModelToTMD(ModelStruct *theModel, int nX, int nY, int nZ, unsigned long *lModelAddress) {
    lModelAddress++;
    GsMapModelingData((unsigned long *)lModelAddress);
    GsInitCoordinate2(WORLD, &theModel->gsObjectCoord);
    lModelAddress++;
    lModelAddress++;
    GsLinkObject4((unsigned long *)lModelAddress, &theModel->gsObjectHandler, 0);
    theModel->gsObjectHandler.coord2 = &theModel->gsObjectCoord;
    theModel->gsObjectCoord.coord.t[0] = nX;
    theModel->gsObjectCoord.coord.t[1] = nY;
    theModel->gsObjectCoord.coord.t[2] = nZ;
    theModel->gsObjectCoord.flg = 0;
}

void InitialiseModel(ModelStruct *theModel, int nX, int nY, int nZ, int rotX, int rotY, int rotZ, unsigned long *lModelAddress) {
    theModel->rotation.vx = 0;
    theModel->rotation.vy = 0;
    theModel->rotation.vz = 0;
    theModel->speed = 0;
    theModel->collisionRadius = 300;
    LinkModelToTMD(theModel, nX, nY, nZ, lModelAddress);
}

void RotModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, int nRX, int nRY, int nRZ) {
    MATRIX matTmp;
    rotateVector->vx = (rotateVector->vx + nRX) % ONE;
    rotateVector->vy = (rotateVector->vy + nRY) % ONE;
    rotateVector->vz = (rotateVector->vz + nRZ) % ONE;
    ResetMatrix(gsObjectCoord->coord.m);
    RotMatrix(rotateVector, &matTmp);
    MulMatrix0(&gsObjectCoord->coord, &matTmp, &gsObjectCoord->coord);
    gsObjectCoord->flg = 0;
}

void RotateModel90(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector) {
    RotModel(gsObjectCoord, rotateVector, 0, 5125, 0);
}

void RotateModel180(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector) {
    RotModel(gsObjectCoord, rotateVector, 0, 10240, 0);
}

void RotateModel270(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector) {
    RotModel(gsObjectCoord, rotateVector, 0, 15360, 0);
}

void DrawModel(ModelStruct *theModel, GsOT *othWorld) {
    MATRIX tmpls, tmplw;
    GsGetLws(theModel->gsObjectHandler.coord2, &tmplw, &tmpls);
    GsSetLightMatrix(&tmplw);
    GsSetLsMatrix(&tmpls);
    GsSortObject4(&theModel->gsObjectHandler, othWorld, 3, (u_long *)getScratchAddr(0));
}