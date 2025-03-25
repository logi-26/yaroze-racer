#include <libps.h>
#include "graphics.h"

GsOT othWorld[2]; 
GsOT_TAG otWorld[2][1<<ORDERING_TABLE_LENGTH];

int fontID_1;

void InitialiseGraphics() {
	// Initialise The Graphics System to PAL as opposed to NTSC
	SetVideoMode(MODE_PAL);
	
	// The Actual Size of the Video memory
	GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsINTER|GsOFSGPU, 1, 0);
    
	// The Top Left Coordinates Of The Two Buffers
	GsDefDispBuff(0, 0, 0, SCREEN_HEIGHT);
    
	// Initialise the 3D Graphics
	GsInit3D();
    
	// Before we can use the ordering table headers we need to...
    // 1. Set them to the right length
	othWorld[0].length = ORDERING_TABLE_LENGTH;
	othWorld[1].length = ORDERING_TABLE_LENGTH;
    
	// 2. Associate them with an actual ordering table 
    othWorld[0].org = otWorld[0];
	othWorld[1].org = otWorld[1];
	
	// 3. initialise the World Ordering Table Headers and Arrays
	GsClearOt(0,0,&othWorld[0]);
	GsClearOt(0,0,&othWorld[1]);
}

void InitialiseFontSystem() {
	FntLoad(960, 256);
	fontID_1 = FntOpen(-150, -120, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 512);
}