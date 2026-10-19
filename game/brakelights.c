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

// Light the brake lights in a CLUT
void LightBrakeLights(u_short *clut, int config) {
    const int *entries;
    int numEntries, i;

    if (config < 0 || config > 10)
	{
		return;
	}

    entries = allEntries[config].entries;
    numEntries = allEntries[config].count;

	for (i = 0; i < numEntries; i++)
	{
        clut[entries[i]] = BRAKE_RED;
	}
}
