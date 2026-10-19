#include <libps.h>
#include "vehicle_colour.h"
#include "brakelights.h"


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
// This enables colours to be replaced whilst preserving the colour shades from the texture
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


// Recolour the body range of a CLUT (in RAM) to a colour, keeping the texture's shading
static void TransformClut(u_short *clut, int clutWords, int bodyStart, int bodyEnd, VehicleColour colour) {
    const ColourTransform *t;
    int i, r, g, b, orig_b;
    u_short pixel;

	// Ignore requests for the original colour or invalid colour IDs
    if (colour == VEHICLE_COLOUR_ORIGINAL || (int)colour >= VEHICLE_COLOUR_COUNT) {
        return;
	}

	// Select the colour transf params for the requested paint colour
    t = &transforms[colour];

	// Only re-colour the palette range used by the vehicle body
    for (i = bodyStart; i <= bodyEnd && i < clutWords; i++) {

		// PS1 colours stored as (bit 15 = STP, bits 10-14 = Blue, bits 5- 9 = Green, bits 0- 4 = Red)
		pixel = clut[i];

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
        clut[i] = (pixel & 0x8000) | (u_short)((b << 10) | (g << 5) | r);
    }
}


// Recolours a vehicle by modifying entries in its TIM texture CLUT (in VRAM)
void ApplyVehicleColour(long texAddr, int bodyStart, int bodyEnd, VehicleColour colour) {
    GsIMAGE tim;

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
	DrawSync(0);

	TransformClut(workClut, tim.cw * tim.ch, bodyStart, bodyEnd, colour);

	// Upload the modified CLUT back into VRAM
    LoadImage(&clutRect, (u_long *)workClut);
    DrawSync(0);
}


/*****************************************************
Each cars own paint

All the cars of the same model share the same texture and its polygons all use the textures CLUT.
So that cars of the same model can be different colours each car gets its own two CLUTs in VRAM 
(its colour and its colour with the brake lights lit)

*****************************************************/
#define PAINT_CLUT_X   320          // Where the cars CLUTs are in VRAM (2 rows for each slot)
#define PAINT_CLUT_Y   488

// The base texture of each model (car3, car2, car5)
static const long modelTexAddr[3] = { CAR_3Y_TEX_MEM_ADDR, CAR_2B_TEX_MEM_ADDR, CAR_5G_TEX_MEM_ADDR };

// The brake light entries to light for each vehicle
static const int brakeConfig[15] = { 2, 2, 2, 2, 2,  4, 4, 4, 4, 4,  6, 7, 8, 9, 10 };

// Store the modified CLUTs for each car on the screen (one slot for each car)
// Also store the original CLUT of each model (from its TIM texture) so it can be restored when a car is removed
#define MAX_CAR_POLYGONS 400
static const long modelTmdAddr[3] = { CAR_3Y_MEM_ADDR, CAR_2B_MEM_ADDR, CAR_5G_MEM_ADDR };
static u_char *modelPrims[3];
static u_short clutOffsets[3][MAX_CAR_POLYGONS];
static int numClutOffsets[3];
static u_short modelOriginalClut[3];
static u_short modelCurrentClut[3];


// A models polygons (an objects offsets are made into addresses by GsMapModelingData)
static u_char *ModelPolygons(u_long *object) {
	return (object[4] & 0x80000000) ? (u_char *)object[4] : (u_char *)object + object[4];
}


// Find where each car models polygons have their CLUT (once, when the game starts before the
// models are drawn, while their polygons headers are still as they are in the TMD files)
void InitialiseCarPaint(void) {
	int m;

	for (m = 0; m < 3; m++) {
		// The TMD, its ID, flags and number of objects
		u_long *object = (u_long *)modelTmdAddr[m] + 3;
		u_char *prim = ModelPolygons(object);
		u_long k, n = object[5];
		int count = 0;

		modelPrims[m] = prim;
		for (k = 0; k < n && count < MAX_CAR_POLYGONS; k++) {
			// For each polygon get olen, ilen, flag, mode, then for a textured one u0, v0 and its CLUT)
			if (prim[3] & 0x04) {
				clutOffsets[m][count++] = (u_short)(prim + 6 - modelPrims[m]);
			}
			prim += 4 + prim[1] * 4;
		}
		numClutOffsets[m] = count;
		modelOriginalClut[m] = modelCurrentClut[m] = count ? *(u_short *)(modelPrims[m] + clutOffsets[m][0]) : 0;
	}
}


// Recolour a CLUT (in RAM) to a vehicles colour (index 0-4 car3, 5-9 car2, 10-14 car5)
static void ColourClut(u_short *clut, int clutWords, int vehicleIndex) {
	int v = vehicleIndex % 5;

	if (vehicleIndex < 5) {
		TransformClut(clut, clutWords, CAR3_BODY_CLUT_START, CAR3_BODY_CLUT_END, car3Colours[v]);
	}
	else if (vehicleIndex < 10) {
		TransformClut(clut, clutWords, CAR2_BODY_CLUT_START_1, CAR2_BODY_CLUT_END_1, car2Colours[v]);
		TransformClut(clut, clutWords, CAR2_BODY_CLUT_START_2, CAR2_BODY_CLUT_END_2, car2Colours[v]);
	}
	else {
		TransformClut(clut, clutWords, CAR5_BODY_CLUT_START_1, CAR5_BODY_CLUT_END_1, car5Colours[v]);
		TransformClut(clut, clutWords, CAR5_BODY_CLUT_START_2, CAR5_BODY_CLUT_END_2, car5Colours[v]);
		TransformClut(clut, clutWords, CAR5_BODY_CLUT_START_3, CAR5_BODY_CLUT_END_3, car5Colours[v]);
	}
}


// Give each car its own paint (its two CLUTs in its assigned slot)
// Each slot is derived from the models original texture CLUT (the one loaded from the TIM into RAM)
// This enable two cars of the same model to be different colours, and for each car to have its own brake lights
void PaintCar(PlayerStruct *car, int slot, int vehicleIndex) {
	static u_short clut[256];
	GsIMAGE tim;
	RECT rect;
	int i, lit, clutWords;
	u_short *original;

	car->paintClut[0] = car->paintClut[1] = 0;
	car->brakesOn = 0;
	if (vehicleIndex < 0 || vehicleIndex >= 15 || slot < 0 || slot >= CAR_PAINT_SLOTS) {
		return;
	}

	GsGetTimInfo((u_long *)(modelTexAddr[vehicleIndex / 5] + 4), &tim);
	
	if (!((tim.pmode >> 3) & 0x01)) {
		return;
	}

	clutWords = tim.cw * tim.ch;

	if (clutWords > 256) {
		clutWords = 256;
	}

	original = (u_short *)tim.clut;

	for (lit = 0; lit < 2; lit++) {
		for (i = 0; i < clutWords; i++) {
			clut[i] = original[i];
		}

		ColourClut(clut, clutWords, vehicleIndex);

		if (lit) {
			LightBrakeLights(clut, brakeConfig[vehicleIndex]);
		}

		rect.x = PAINT_CLUT_X;
		rect.y = PAINT_CLUT_Y + slot * 2 + lit;
		rect.w = clutWords;
		rect.h = 1;
		LoadImage(&rect, (u_long *)clut);
		DrawSync(0);

		// The CLUTs ID as the polygons hold it (y in bits 6-14, x / 16 in bits 0-5)
		car->paintClut[lit] = (u_short)((rect.y << 6) | (rect.x >> 4));
	}
}


// Point a cars models polygons at the cars CLUT just before the car is drawn. 
// A car without its own paint gets its models own CLUT back.
void ApplyCarPaint(PlayerStruct *car) {
	u_long *object = car->gsObjectHandler.tmd;
	u_char *prims;
	u_short cba;
	int m, i;

	if (!object) {
		return;
	}

	// Its model
	prims = ModelPolygons(object);
	for (m = 0; m < 3 && modelPrims[m] != prims; m++)
		;
	if (m == 3) {
		return;
	}

	cba = car->paintClut[0] ? car->paintClut[car->brakesOn ? 1 : 0] : modelOriginalClut[m];
	if (cba == modelCurrentClut[m]) {
		return;
	}
	modelCurrentClut[m] = cba;

	for (i = 0; i < numClutOffsets[m]; i++) {
		*(u_short *)(prims + clutOffsets[m][i]) = cba;
	}
}
