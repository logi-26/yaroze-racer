#ifndef GAME_H
#define GAME_H

#include <libps.h>
#include "world.h"

/************* FUNCTION PROTOTYPES *******************/
void InitialiseTopDownView(GsRVIEW2 *view, int nProjDist, int nHeight, int nVPX, int nVPZ);
void InitialiseTrackerView(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ);
void InitialiseStaticView(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ);
void RotateModel(GsCOORDINATE2 *gsObjectCoord,SVECTOR *rotateVector , int nRX, int nRY, int nRZ);
void AdvanceModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, long *speed, int nD);
void ResetMatrix(short m[3][3]);
/*****************************************************/

#endif // GAME_H