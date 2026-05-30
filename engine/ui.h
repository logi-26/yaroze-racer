#ifndef UI_H
#define UI_H

#include <libps.h>


// Draw modes
#define SOLID             0
#define TRANS_HALF        1
#define TRANS_ADD         2
#define TRANS_SUBTRACT    3
#define TRANS_ADD_QUARTER 4


/************* FUNCTION PROTOTYPES *******************/
void DisplayFPS(int x, int y);
void DrawBar(int x, int y, int style, int val, int max, GsOT *ot, int priority);
void DrawBarSolid(int x, int y, int barW, int barH, int val, int max, unsigned char fillR, unsigned char fillG, unsigned char fillB, unsigned char bgR, unsigned char bgG, unsigned char bgB, GsOT *ot, int priority);
void DrawBarSegmented(int x, int y, int barW, int barH, int val, int max, int segments, int gap, unsigned char fillR, unsigned char fillG, unsigned char fillB, unsigned char bgR, unsigned char bgG, unsigned char bgB, GsOT *ot, int priority);
void DrawBarGradient(int x, int y, int barW, int barH, int val, int max, unsigned char r1, unsigned char g1, unsigned char b1, unsigned char r2, unsigned char g2, unsigned char b2, unsigned char bgR, unsigned char bgG, unsigned char bgB, GsOT *ot, int priority);
void DrawRect(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b, int drawMode, GsOT *ot, int priority);

// Country flags
void DrawFlagEngland(int x, int y, int scale, GsOT *ot, int priority);
void DrawFlagFrance(int x, int y, int scale, GsOT *ot, int priority);
void DrawFlagNetherlands(int x, int y, int scale, GsOT *ot, int priority);
void DrawFlagGermany(int x, int y, int scale, GsOT *ot, int priority);
/*****************************************************/

#endif