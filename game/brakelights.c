#include <libps.h>
#include "brakelights.h"

// PlayStation CLUT entries are 15-bit colour: bits 0-4 = R, 5-9 = G, 10-14 = B (5 bits each)
// (255,0,0) maps to R=31, G=0, B=0 - 0x001F.
#define BRAKE_RED ((u_short)0x001F)


// CLUT indices per texture variant
static const int tex0[] = { 4, 86, 87 };             			// car3 green
static const int tex1[] = { 4, 90, 91, 92 };         			// car3 red
static const int tex2[] = { 4, 84, 85, 86 };         			// car3 yellow
static const int tex3[] = { 4, 64, 65, 66, 67 };     			// car2 black
static const int tex4[] = { 4, 79, 80, 81 };         			// car2 blue
static const int tex5[] = { 4, 79, 80, 81 };         			// car2 red
static const int tex6[] = { 4, 53, 54, 55, 56, 57, 58, 59 }; 	// car5 black
static const int tex7[] = { 4, 77, 78, 79, 80, 81, 82 };     	// car5 green
static const int tex8[] = { 4, 88, 89 };             			// car5 red
static const int tex9[]  = { 4, 77, 78, 79, 80, 81, 82 };    	// car5 yellow
static const int tex10[] = { 4, 77, 78, 79, 80, 81, 82 };    	// car5 white

typedef struct { const int *entries; int count; } TexEntries;

static const TexEntries allEntries[11] = {
    { tex0,  sizeof(tex0)  / sizeof(int) },
    { tex1,  sizeof(tex1)  / sizeof(int) },
    { tex2,  sizeof(tex2)  / sizeof(int) },
    { tex3,  sizeof(tex3)  / sizeof(int) },
    { tex4,  sizeof(tex4)  / sizeof(int) },
    { tex5,  sizeof(tex5)  / sizeof(int) },
    { tex6,  sizeof(tex6)  / sizeof(int) },
    { tex7,  sizeof(tex7)  / sizeof(int) },
    { tex8,  sizeof(tex8)  / sizeof(int) },
    { tex9,  sizeof(tex9)  / sizeof(int) },
    { tex10, sizeof(tex10) / sizeof(int) },
};

// Player brake light CLUT buffers
static u_short savedClut[256];
static u_short brakeClut[256];
static RECT clutRect;
static int ready = 0;

// AI brake light CLUT buffers (independent from player)
static u_short aiSavedClut[256];
static u_short aiBrakeClut[256];
static RECT aiClutRect;
static int aiReady = 0;


// Initialise the brake light effect
void InitBrakeLightEffect(long texAddr, int vehicleIndex) {
    GsIMAGE tim;
    const int *entries;
    int numEntries;
    int i, clutWords;

    if (vehicleIndex < 0 || vehicleIndex > 10) 
	{
		ready = 0; 
		return; 
	}

    GsGetTimInfo((u_long *)(texAddr + 4), &tim);

    // Requires a CLUT-based texture (4-bit or 8-bit)
    if (!((tim.pmode >> 3) & 0x01)) 
	{ 
		ready = 0; 
		return; 
	}

    // Get the CLUT rect
    clutRect.x = tim.cx;
    clutRect.y = tim.cy;
    clutRect.w = tim.cw;
    clutRect.h = tim.ch;

    // Store the original CLUT from VRAM
    StoreImage(&clutRect, (u_long *)savedClut);
    DrawSync(0);

    // Copy to brake version
    clutWords = tim.cw * tim.ch;
    for (i = 0; i < clutWords; i++) 
	{
        brakeClut[i] = savedClut[i];
	}

    // Apply red to the tail-light CLUT indices for this specific texture variant
    entries = allEntries[vehicleIndex].entries;
    numEntries = allEntries[vehicleIndex].count;
    
	for (i = 0; i < numEntries; i++) 
	{
        brakeClut[entries[i]] = BRAKE_RED;
	}

    ready = 1;
}


void InitAIBrakeLightEffect(long texAddr, int vehicleIndex) {
    GsIMAGE tim;
    const int *entries;
    int numEntries, i, clutWords;

    if (vehicleIndex < 0 || vehicleIndex > 10) 
	{ 
		aiReady = 0; 
		return; 
	}

    GsGetTimInfo((u_long *)(texAddr + 4), &tim);
    if (!((tim.pmode >> 3) & 0x01)) 
	{ 
		aiReady = 0; 
		return; 
	}

    aiClutRect.x = tim.cx;
    aiClutRect.y = tim.cy;
    aiClutRect.w = tim.cw;
    aiClutRect.h = tim.ch;

    StoreImage(&aiClutRect, (u_long *)aiSavedClut);
    DrawSync(0);

    clutWords = tim.cw * tim.ch;
    for (i = 0; i < clutWords; i++) 
	{
		aiBrakeClut[i] = aiSavedClut[i];
	}

    entries = allEntries[vehicleIndex].entries;
    numEntries = allEntries[vehicleIndex].count;
    
	for (i = 0; i < numEntries; i++) 
	{
		aiBrakeClut[entries[i]] = BRAKE_RED;
	}

    aiReady = 1;
}


void SetAIBrakeLightTexture(int braking) {
    if (!aiReady) 
	{
		return;
	}
	
    LoadImage(&aiClutRect, (u_long *)(braking ? aiBrakeClut : aiSavedClut));
    DrawSync(0);
}


// Load the appropriate brake light CLUT
void SetBrakeLightTexture(int braking) {
    if (!ready) 
	{
		return;
	}
	
    LoadImage(&clutRect, (u_long *)(braking ? brakeClut : savedClut));
    DrawSync(0);
}