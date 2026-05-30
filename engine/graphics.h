#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <libps.h>
#include "../main.h"
#include "state_manager.h"


/*****************************************************
Build configuration
Set ENABLE_SPLITSCREEN to 0 for a single-player only build
This halves the OT and packet buffer memory usage
*****************************************************/
#define ENABLE_SPLITSCREEN 1


/*****************************************************
Region configuration
Set IS_PAL_USED to TRUE for PAL, FALSE for NTSC
*****************************************************/

#define FRAME_RATE (GAME_REGION == REGION_PAL ? 50 : 60)

#define PAL_WIDTH   320
#define PAL_HEIGHT  240
#define NTSC_WIDTH  320
#define NTSC_HEIGHT 256


/*****************************************************
Runtime screen dimensions - set by InitialiseGraphicsSystem()
*****************************************************/
extern int gScreenWidth;
extern int gScreenHeight;

// Screen offset
#define SCREEN_X_OFFSET 0
#define SCREEN_Y_OFFSET 16


/*****************************************************
Ordering table and buffer counts
*****************************************************/ 
#if ENABLE_SPLITSCREEN
    #define OT_NUMBER 4
#else
    #define OT_NUMBER 2
#endif

// Ordering table depth
#define OT_LENGTH 12

// Depth layers
#define OT_BACKGROUND 11
#define OT_WORLD       8
#define OT_UI          4
#define OT_POPUP      0

// Packet buffer size per slot
#define PACKETMAX 65000

// Distance threshold for culling
#define DISTANCE_THRESHOLD_SQUARED (8000*8000)


/*****************************************************
Screen mode
*****************************************************/  
typedef enum {
    SCREEN_MODE_FULLSCREEN = 0,
    SCREEN_MODE_SPLITSCREEN = 1
} ScreenMode;

extern ScreenMode gScreenMode;


/*****************************************************
Graphics buffers - sized at compile time based on ENABLE_SPLITSCREEN
*****************************************************/
extern GsOT WorldOrderingTable[OT_NUMBER];
extern GsOT_TAG OrderingTableTags[OT_NUMBER][1 << OT_LENGTH];
extern PACKET GpuPacketArea[OT_NUMBER][PACKETMAX];

/*****************************************************
SplitScreenInfo[0..1] = fullscreen or top-half environments
SplitScreenInfo[2..3] = bottom-half environments (splitscreen only)
*****************************************************/
extern DRAWENV SplitScreenInfo[OT_NUMBER];

extern int fontID_1;
extern int fontID_2;


/************* FUNCTION PROTOTYPES *******************/
void InitialiseGraphicsSystem(void);
void InitialiseFontSystem(void);
void SetScreenMode(ScreenMode mode);
void UpdateGraphicsSystem(int activeBuffer);
int  GetActiveBuffer(void);

void SetBackgroundColor(int r, int g, int b);
void GetBackgroundColor(int *r, int *g, int *b);
/*****************************************************/

#endif