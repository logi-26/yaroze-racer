#include <libps.h>
#include "brakelights.h"

// PlayStation CLUT entries are 15-bit colour: bits 0-4 = R, 5-9 = G, 10-14 = B (5 bits each)
// (255,0,0) maps to R=31, G=0, B=0 - 0x001F.
#define BRAKE_RED ((u_short)0x001F)

// CLUT indexes for the brake light on each vehicle
static const int car3Entries[] = { 187, 188, 189 };
static const int car2Entries[] = { 4, 15, 200 };
static const int car5Entries[] = { 78, 79, 167, 168, 233, 234, 235, 236 };

// Used to store the original vehicle CLUT
static u_short savedClut[256];

// Used to store the modified vehicle CLUT
static u_short brakeClut[256];

static RECT clutRect;
static int ready = 0;


// Initialise the brake light effect
void InitBrakeLightEffect(long texAddr, int vehicleIndex) {
    GsIMAGE tim;
    const int *entries;
    int numEntries;
    int i, clutWords;

    GsGetTimInfo((u_long *)(texAddr + 4), &tim);

    // Requires a CLUT-based texture (4-bit or 8-bit)
    if (!((tim.pmode >> 3) & 0x01)) {
        ready = 0;
        return;
    }

	// Get the CLUT rect
    clutRect.x = tim.cx;
    clutRect.y = tim.cy;
    clutRect.w = tim.cw;
    clutRect.h = tim.ch;
	
	// Store the original CLUT
    StoreImage(&clutRect, (u_long *)savedClut);
    DrawSync(0);

    clutWords = tim.cw * tim.ch;
    for (i = 0; i < clutWords; i++)
        brakeClut[i] = savedClut[i];

	// Get the CLUT brake light indexes for the specific vehicle
    if (vehicleIndex < 3) {
        entries = car3Entries;
        numEntries = sizeof(car3Entries) / sizeof(int);
    } else if (vehicleIndex < 6) {
        entries = car2Entries;
        numEntries = sizeof(car2Entries) / sizeof(int);
    } else {
        entries = car5Entries;
        numEntries = sizeof(car5Entries) / sizeof(int);
    }

	// Set the CLUT indexes red to simulate the brake lights
    for (i = 0; i < numEntries; i++)
        brakeClut[entries[i]] = BRAKE_RED;

    ready = 1;
}


// Load the appropriate brake light CLUT
void SetBrakeLightTexture(int braking) {
    if (!ready) return;
    LoadImage(&clutRect, (u_long *)(braking ? brakeClut : savedClut));
    DrawSync(0);
}