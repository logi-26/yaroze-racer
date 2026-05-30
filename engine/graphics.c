#include <libps.h>
#include "graphics.h"
#include "../main.h"

// Runtime screen dimensions - set during InitialiseGraphicsSystem
int gScreenWidth = PAL_WIDTH;
int gScreenHeight = PAL_HEIGHT;

// Global graphics state
ScreenMode gScreenMode = SCREEN_MODE_FULLSCREEN;

GsOT WorldOrderingTable[OT_NUMBER];
GsOT_TAG OrderingTableTags[OT_NUMBER][1 << OT_LENGTH];
PACKET GpuPacketArea[OT_NUMBER][PACKETMAX];
DRAWENV SplitScreenInfo[OT_NUMBER];

// Internal color tracking states
static int gBackgroundR = 0;
static int gBackgroundG = 0;
static int gBackgroundB = 0;

int fontID_1;
int fontID_2;


// Initialise the font System
void InitialiseFontSystem(void) {
    FntLoad(960, 256);
    fontID_1 = FntOpen(-150, -100, gScreenWidth, gScreenHeight, 0, 512);
    fontID_2 = FntOpen(-150, 0, gScreenWidth, gScreenHeight, 0, 512);
}


// Set background colour
void SetBackgroundColor(int r, int g, int b) {
    gBackgroundR = r;
    gBackgroundG = g;
    gBackgroundB = b;
}


// Get background colour
void GetBackgroundColor(int *r, int *g, int *b) {
    *r = gBackgroundR;
    *g = gBackgroundG;
    *b = gBackgroundB;
}


/*****************************************************
InitialiseGraphicsSystem
Detects region, sets runtime dimensions, initialises graphics system
Always starts in fullscreen mode
*****************************************************/
void InitialiseGraphicsSystem(void) {
    int i;
 
    if (REGION_PAL) {
        SetVideoMode(MODE_PAL);
        GsInitGraph(PAL_WIDTH, PAL_HEIGHT, 4, 0, 0);
        GsDefDispBuff(0, 0, 0, PAL_HEIGHT);
        gScreenWidth = PAL_WIDTH;
        gScreenHeight = PAL_HEIGHT;
    } else {
        SetVideoMode(MODE_NTSC);
        GsInitGraph(NTSC_WIDTH, NTSC_HEIGHT, 4, 0, 0);
        GsDefDispBuff(0, 0, 0, NTSC_HEIGHT);
        gScreenWidth = NTSC_WIDTH;
        gScreenHeight = NTSC_HEIGHT;
    }
 
    GsDISPENV.screen.x = SCREEN_X_OFFSET;
    GsDISPENV.screen.y = SCREEN_Y_OFFSET;
 
    GsInit3D();
 
    for (i = 0; i < OT_NUMBER; i++) {
        WorldOrderingTable[i].length = OT_LENGTH;
        WorldOrderingTable[i].org = OrderingTableTags[i];
        GsClearOt(0, 0, &WorldOrderingTable[i]);
    }
 
    SetScreenMode(SCREEN_MODE_FULLSCREEN);
}


/*****************************************************
SetScreenMode
  Switch between fullscreen and splitscreen at any time
  Uses gScreenWidth/gScreenHeight so it works for both PAL and NTSC

  Splitscreen divides the screen horizontally into two equal halves:
    PAL:  each half is 320x120
    NTSC: each half is 320x128
*****************************************************/ 
void SetScreenMode(ScreenMode mode) {
    int halfH;
 
#if !ENABLE_SPLITSCREEN
    // Splitscreen disabled at compile time - ignore the request
    if (mode == SCREEN_MODE_SPLITSCREEN) return;
#endif
 
    gScreenMode = mode;
    halfH = gScreenHeight / 2;
 
    if (mode == SCREEN_MODE_FULLSCREEN) {
        // Fullscreen - Buffer 0 
        SplitScreenInfo[0].clip.x = 0;
        SplitScreenInfo[0].clip.y = 0;
        SplitScreenInfo[0].clip.w = gScreenWidth;
        SplitScreenInfo[0].clip.h = gScreenHeight;
        SplitScreenInfo[0].ofs[0] = 0;
        SplitScreenInfo[0].ofs[1] = 0;
        SplitScreenInfo[0].isbg   = 0;
        SplitScreenInfo[0].r      = 0; SplitScreenInfo[0].g = 0; SplitScreenInfo[0].b = 0;

        // Fullscreen - Buffer 1
        SplitScreenInfo[1].clip.x = 0;
        SplitScreenInfo[1].clip.y = gScreenHeight;
        SplitScreenInfo[1].clip.w = gScreenWidth;
        SplitScreenInfo[1].clip.h = gScreenHeight;
        SplitScreenInfo[1].ofs[0] = 0;
        SplitScreenInfo[1].ofs[1] = gScreenHeight;
        SplitScreenInfo[1].isbg   = 0;
        SplitScreenInfo[1].r      = 0; SplitScreenInfo[1].g = 0; SplitScreenInfo[1].b = 0;

        GsSetProjection(250);
 
#if ENABLE_SPLITSCREEN
    } else {
		/*****************************************************
		Split screen draw environments
        Top half occupies display y=0 to y=halfH-1
        Bottom half occupies display y=halfH to y=gScreenHeight-1
		*****************************************************/ 
		
        // Splitscreen Top half - buffer 0
        SplitScreenInfo[0].clip.x = 0;
        SplitScreenInfo[0].clip.y = gScreenHeight;
        SplitScreenInfo[0].clip.w = gScreenWidth;
        SplitScreenInfo[0].clip.h = halfH;
        SplitScreenInfo[0].ofs[0] = 0;
        SplitScreenInfo[0].ofs[1] = gScreenHeight;
        SplitScreenInfo[0].isbg   = 0; // DISABLE AUTO-CLEAR
        SplitScreenInfo[0].r      = 0; SplitScreenInfo[0].g = 0; SplitScreenInfo[0].b = 0;
 
        // Splitscreen Top half - buffer 1
        SplitScreenInfo[1].clip.x = 0;
        SplitScreenInfo[1].clip.y = 0;
        SplitScreenInfo[1].clip.w = gScreenWidth;
        SplitScreenInfo[1].clip.h = halfH;
        SplitScreenInfo[1].ofs[0] = 0;
        SplitScreenInfo[1].ofs[1] = 0;
        SplitScreenInfo[1].isbg   = 0; // DISABLE AUTO-CLEAR
        SplitScreenInfo[1].r      = 0; SplitScreenInfo[1].g = 0; SplitScreenInfo[1].b = 0;
 
        // Splitscreen Bottom half - buffer 2
        SplitScreenInfo[2].clip.x = 0;
        SplitScreenInfo[2].clip.y = gScreenHeight + halfH;
        SplitScreenInfo[2].clip.w = gScreenWidth;
        SplitScreenInfo[2].clip.h = halfH;
        SplitScreenInfo[2].ofs[0] = 0;
        SplitScreenInfo[2].ofs[1] = gScreenHeight + halfH;
        SplitScreenInfo[2].isbg   = 0;
        SplitScreenInfo[2].r      = 0; SplitScreenInfo[2].g = 0; SplitScreenInfo[2].b = 0;
 
        // Splitscreen Bottom half - buffer 3
        SplitScreenInfo[3].clip.x = 0;
        SplitScreenInfo[3].clip.y = halfH;
        SplitScreenInfo[3].clip.w = gScreenWidth;
        SplitScreenInfo[3].clip.h = halfH;
        SplitScreenInfo[3].ofs[0] = 0;
        SplitScreenInfo[3].ofs[1] = halfH;
        SplitScreenInfo[3].isbg   = 0;
        SplitScreenInfo[3].r      = 0; SplitScreenInfo[3].g = 0; SplitScreenInfo[3].b = 0;
 
        GsSetProjection(200);
#endif
    }
}


/*****************************************************
GetActiveBuffer
In splitscreen, use activeBuffer+2 for the bottom half OT
*****************************************************/ 
int GetActiveBuffer(void) {
    int activeBuffer = GsGetActiveBuff();
 
    GsSetWorkBase((PACKET *)GpuPacketArea[activeBuffer]);
    GsClearOt(0, 0, &WorldOrderingTable[activeBuffer]);
 
#if ENABLE_SPLITSCREEN
    if (gScreenMode == SCREEN_MODE_SPLITSCREEN) {
        GsSetWorkBase((PACKET *)GpuPacketArea[activeBuffer + 2]);
        GsClearOt(0, 0, &WorldOrderingTable[activeBuffer + 2]);
        
        GsSetWorkBase((PACKET *)GpuPacketArea[activeBuffer]);
    }
#endif
 
    return activeBuffer;
}


/*****************************************************
UpdateGraphicsSystem
Fullscreen:  flips buffers and draws the single OT.
Splitscreen: uses GsDrawOt directly because Yaroze does not have PutDrawEnv
*****************************************************/ 
void UpdateGraphicsSystem(int activeBuffer) {
	
	int r, g, b;
	
    DrawSync(0);
    VSync(0);
 
    GsSwapDispBuff();
 
    r = gBackgroundR;
    g = gBackgroundG;
    b = gBackgroundB;

#if ENABLE_SPLITSCREEN
    if (gScreenMode == SCREEN_MODE_SPLITSCREEN) {
        // Top half
        PutDrawEnv(&SplitScreenInfo[activeBuffer]);
        GsSortClear(r, g, b, &WorldOrderingTable[activeBuffer]);
        GsDrawOt(&WorldOrderingTable[activeBuffer]);
 
        // Bottom half
        PutDrawEnv(&SplitScreenInfo[activeBuffer + 2]);
        GsSortClear(r, g, b, &WorldOrderingTable[activeBuffer + 2]);
        GsDrawOt(&WorldOrderingTable[activeBuffer + 2]);
    } else {
#endif
        // Fullscreen Mode
        PutDrawEnv(&SplitScreenInfo[activeBuffer]);
        GsSortClear(r, g, b, &WorldOrderingTable[activeBuffer]);
        GsDrawOt(&WorldOrderingTable[activeBuffer]);
#if ENABLE_SPLITSCREEN
    }
#endif
}




