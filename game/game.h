#ifndef GAME_H
#define GAME_H

#include <libps.h>
#include "game/world.h"
#include "game/player.h"

extern int NumberOfPlayers;

// Lateral velocity for each player (sideways momentum / slip)
extern long player1_lateralSpeed;
extern long player2_lateralSpeed;

// Brake light state for each player
extern int player1_isBraking;
extern int player2_isBraking;

// Tire grip per frame, out of 1000 (lower = more slide, 920 = grippy, 860 = drifty)
#define LATERAL_FRICTION 920
// Controls how quickly lateral speed builds up when turning (higher = less slip)
#define SLIP_FACTOR 1000

/************* FUNCTION PROTOTYPES *******************/
void InitialiseTopDownView(GsRVIEW2 *view, int nProjDist, int nHeight, int nVPX, int nVPZ);
void InitialiseTrackerView(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ);
void InitialiseTrackerViewPlayer1(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ);
void InitialiseTrackerViewPlayer2(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ);
void InitialiseStaticView(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ);
void RotateModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, int nRX, int nRY, int nRZ, long *speed, long *lateralSpeed);
void AdvanceModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, long *speed, long *lateralSpeed, int movementDirection, int isBraking);
void InitSingleScreen(void);
void InitSplitScreen(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
void DrawBrakeLights(PlayerStruct *player, GsOT *ot, int otIdx);
void ScaleTmdVertices(unsigned long *tmdAddr, int divisor);
/*****************************************************/

#endif // GAME_H