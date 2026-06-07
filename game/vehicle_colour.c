#include <libps.h>
#include "vehicle_colour.h"


/*****************************************************
Channel transform applied to each body-colour CLUT entry
rMul/gMul/bMul: multiply own channel in 1/16 units (16 = no change)
rAdd/gAdd/bAdd: fixed value added after multiply

PS1 CLUT: 5-5-5 BGR — bits 0-4=R, 5-9=G, 10-14=B, 15=STP
*****************************************************/
typedef struct {
    int rMul, gMul, bMul;
    int rAdd, gAdd, bAdd;
    int rFromB, gFromB;
} ColourTransform;


// Colour transforms (to modify the CLUT colours rather than replacing the colours)
// This enables colours to be replcaed whilst preserving the colour shades from the texture
static const ColourTransform transforms[VEHICLE_COLOUR_COUNT] = {
    { 16, 16, 16,    0,   0,   0,     0,    0 }, 	// ORIGINAL
    { 16,  7, 16,    0,   0,   0,     0,    0 }, 	// ORANGE (from yellow)
    { 12,  2,  2,    0,   0,   0,     0,    0 }, 	// RED (from yellow)
    {  2,  2, 16,    0,   0,   2,     0,    0 }, 	// BLUE (from yellow)
    {  2,  5, 16,    0,   0,   0,     0,    0 }, 	// GREEN (from yellow)
    {  2,  2,  0,    4,   1,   0,    12,    4 }, 	// ORANGE (from blue)
    {  3,  3,  4,    0,   0,   1,     0,    0 }, 	// DARK (any texture)
    {  2,  0,  0,    6,   0,   0,    14,    0 }, 	// RED (from blue)
    {  0,  0, 16,    5,   5,   0,    10,   10 }, 	// WHITE (from blue)
    {  0,  0,  0,    0,   2,   0,     0,   10 }, 	// GREEN (from blue)
    {  0,  0,  0,    4,   4,   0,    12,   12 }, 	// YELLOW (from blue)
    {  0,  2,  2,   11,   0,   0,     0,    0 }, 	// RED (from green)
    {  3,  3, 14,    0,   0,   8,     0,    0 }, 	// BLUE (from green)
    {  0, 14,  0,   12,   0,   0,     0,    0 }, 	// ORANGE (from green)
    {  0,  8,  0,   12,   0,  12,     0,    0 }, 	// PINK (from green)
};


// car3 colour variants
const VehicleColour car3Colours[5] = {
    VEHICLE_COLOUR_GREEN_FROM_YELLOW, 	// variant 0: green
    VEHICLE_COLOUR_RED,               	// variant 1: red
    VEHICLE_COLOUR_ORIGINAL,          	// variant 2: yellow
    VEHICLE_COLOUR_BLUE_FROM_YELLOW,  	// variant 3: blue
    VEHICLE_COLOUR_ORANGE,            	// variant 4: orange
};


// car2 colour variants
const VehicleColour car2Colours[5] = {
    VEHICLE_COLOUR_GREEN_FROM_BLUE,  	// variant 0: green
    VEHICLE_COLOUR_ORIGINAL,         	// variant 1: blue
    VEHICLE_COLOUR_YELLOW_FROM_BLUE, 	// variant 2: yellow
    VEHICLE_COLOUR_ORANGE_FROM_BLUE, 	// variant 3: orange
    VEHICLE_COLOUR_WHITE_FROM_BLUE,  	// variant 4: white
};


// car5 colour variants
const VehicleColour car5Colours[5] = {
    VEHICLE_COLOUR_BLUE_FROM_GREEN,   	// variant 0: blue
    VEHICLE_COLOUR_ORIGINAL,          	// variant 1: green
    VEHICLE_COLOUR_RED_FROM_GREEN,    	// variant 2: red
    VEHICLE_COLOUR_ORANGE_FROM_GREEN, 	// variant 3: orange
    VEHICLE_COLOUR_PINK_FROM_GREEN,   	// variant 4: pink
};


// Vraiables to store the modified CLUT data
static u_short workClut[256];
static RECT clutRect;


// Recolours a vehicle by modifying entries in its TIM texture CLUT
void ApplyVehicleColour(long texAddr, int bodyStart, int bodyEnd, VehicleColour colour) {
    GsIMAGE tim;
    const ColourTransform *t;
    int i, clutWords, r, g, b, orig_b;
    u_short pixel;

	// Ignore requests for the original colour or invalid colour IDs
    if (colour == VEHICLE_COLOUR_ORIGINAL || (int)colour >= VEHICLE_COLOUR_COUNT) {
        return;
	}

	// Read TIM texture metadata from memory (skip past the 4-byte TIM header)
    GsGetTimInfo((u_long *)(texAddr + 4), &tim);

	// Ensure this texture actually contains a CLUT (must be 4-bit or 8-bit image)
    if (!((tim.pmode >> 3) & 0x01)) {
        return;
	}

	// Build a rectangle for the CLUT location in VRAM
    clutRect.x = tim.cx;
    clutRect.y = tim.cy;
    clutRect.w = tim.cw;
    clutRect.h = tim.ch;

	// Copy the CLUT from VRAM into a working buffer so it can be edited
    StoreImage(&clutRect, (u_long *)workClut);
    
	// Wait for GPU transfer to complete
	DrawSync(0);

	// Select the colour transf params for the requested paint colour
    t = &transforms[colour];
	
	// Total number of palette entries available
    clutWords = tim.cw * tim.ch;

	// Only re-colour the palette range used by the vehicle body
    for (i = bodyStart; i <= bodyEnd && i < clutWords; i++) {
        
		// PS1 colours stored as (bit 15 = STP, bits 10-14 = Blue, bits 5- 9 = Green, bits 0- 4 = Red)
		pixel = workClut[i];
        
		// Extract 5-bit RGB components
		r = (pixel >>  0) & 0x1F;
        g = (pixel >>  5) & 0x1F;
        b = (pixel >> 10) & 0x1F;
        
		// Preserve the original blue channel (Some colour transforms use it as an intensity/shading source)
		orig_b = b;

		// Apply channel scaling (multipliers use fixed-point values where 16 == 1.0)
        r = (r * t->rMul) >> 4;
        g = (g * t->gMul) >> 4;
        b = (b * t->bMul) >> 4;

		// Apply colour offsets and optional blue-channel contribution
		// Using blue as a source helps preserve texture shading and highlights
        r += t->rAdd + ((orig_b * t->rFromB) >> 4);
        g += t->gAdd + ((orig_b * t->gFromB) >> 4);
        b += t->bAdd;

		// Clamp back to the PS1's 5-bit colour range (0-31)
        if (r > 31) r = 31;
        if (g > 31) g = 31;
        if (b > 31) b = 31;

        // Rebuild the palette entry while preserving the STP flag
        workClut[i] = (pixel & 0x8000) | (u_short)((b << 10) | (g << 5) | r);
    }

	// Upload the modified CLUT back into VRAM
    LoadImage(&clutRect, (u_long *)workClut);
	
	// Wait for the GPU transfer to complete before the texture can be used
    DrawSync(0);
}