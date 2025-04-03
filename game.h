#ifndef GAME_H
#define GAME_H

#include <libps.h>
#include "world.h"
#include "player.h"

extern int NumberOfPlayers;

/************* FUNCTION PROTOTYPES *******************/
void InitialiseTopDownView(GsRVIEW2 *view, int nProjDist, int nHeight, int nVPX, int nVPZ);
void InitialiseTrackerView(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ);

void InitialiseTrackerViewPlayer1(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ);
void InitialiseTrackerViewPlayer2(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ);




void InitialiseStaticView(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ);
void RotateModel(GsCOORDINATE2 *gsObjectCoord,SVECTOR *rotateVector , int nRX, int nRY, int nRZ, long *speed);
void AdvanceModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, long *speed, int movementDirection, int isBraking);
void ResetMatrix(short m[3][3]);
void InitSplitScreen(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
/*****************************************************/

#endif // GAME_H