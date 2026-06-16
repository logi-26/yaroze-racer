#include "ui.h"

#include "graphics.h"
#include "colours.h"
#include "font.h"

static int frameCounter = 0;
static int fps = 0;
static int fpsTimer = 0;


// Display the FPS counter on the screen
void DisplayFPS(int x, int y)
{
	// FPS counter
	char fpsStr[16];
    frameCounter++;
    fpsTimer++;
	
	// Calculate the FPS (region independent)
    if (fpsTimer >= FRAME_RATE)
    {
        fps = frameCounter;
        frameCounter = 0;
        fpsTimer = 0;
    }
	
	// Convert the FPS value to a string for printing
	sprintf(fpsStr, "%d", fps);
	
	// Draw the FPS display
	FontFX_FontBegin();
	FontFX_SetStyle(FONT_STYLE_0);
	FontFX_SetColour(COL_DARKRED);
	FontFX_SetAlpha(0);
	FontFX_Print(x, y, "FPS:", &WorldOrderingTable[activeBuffer], 0);
	FontFX_SetColour(COL_WHITE);
	FontFX_Print(x + (4 * FONT_CHAR_W), y, fpsStr, &WorldOrderingTable[activeBuffer], 0);
	FontFX_SetAlpha(1);
	FontFX_FontEnd();
}


// Draw the slider bar
void DrawBar(int x, int y, int style, int val, int max, GsOT *ot, int priority)
{
    int i;
    int filled = (val * 16) / max;
    int curX   = x;
    char seg[2] = { 0, 0 };

    // Opening bracket
    FontFX_PrintWithoutEffects(curX, y, "[", style, COL_WHITE, ot, priority);
    curX += FONT_CHAR_W;

    // Bar segments
    for (i = 0; i < 16; i++)
    {
        seg[0] = (i < filled) ? '=' : '-';
        FontFX_PrintWithoutEffects(curX, y, seg, style, i < filled ? COL_GREEN : COL_DARKGREY, ot, priority);
        curX += FONT_CHAR_W;
    }

    // Closing bracket
    FontFX_PrintWithoutEffects(curX, y, "]", style, COL_WHITE, ot, priority);
}


void DrawRect(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b, int drawMode, GsOT *ot, int priority)
{
    GsBOXF box;

    // Solid by default
    box.attribute = 0;

    // Enable semi-transparency if not solid
    if (drawMode != SOLID) 
	{
        box.attribute = (1 << 30) | ((drawMode & 0x3) << 5);
	}

    box.x = (short)x;
    box.y = (short)y;
    box.w = (unsigned short)w;
    box.h = (unsigned short)h;

    box.r = r;
    box.g = g;
    box.b = b;

    GsSortBoxFill(&box, ot, priority);
}


/*===========================================================================
    DrawBarSolid
    A clean filled rectangle bar with a background track
===========================================================================*/
void DrawBarSolid(int x, int y, int barW, int barH, int val, int max, unsigned char fillR, unsigned char fillG, unsigned char fillB, unsigned char bgR, unsigned char bgG, unsigned char bgB, GsOT *ot, int priority)
{
    int filledW;
    GsBOXF box;

    if (max <= 0) 
	{
		max = 1;
	}
	
    filledW = (val * barW) / max;
	
    if (filledW > barW) 
	{
		filledW = barW;
	}

    box.attribute = 0;
    box.y = (short)y;
    box.h = (unsigned short)barH;

    // Background track
    box.x = (short)x;
    box.w = (unsigned short)barW;
    box.r = bgR; box.g = bgG; box.b = bgB;
    GsSortBoxFill(&box, ot, priority + 1);

    // Filled portion
    if (filledW > 0)
    {
        box.x = (short)x;
        box.w = (unsigned short)filledW;
        box.r = fillR; box.g = fillG; box.b = fillB;
        GsSortBoxFill(&box, ot, priority);
    }
}


/*===========================================================================
    DrawBarSegmented
    Bar divided into equal segments, each drawn filled or empty
===========================================================================*/
void DrawBarSegmented(int x, int y, int barW, int barH, int val, int max, int segments, int gap, unsigned char fillR, unsigned char fillG, unsigned char fillB, unsigned char bgR, unsigned char bgG, unsigned char bgB, GsOT *ot, int priority)
{
    int i;
    int filledSegs;
    int segW;
    int sx;
    GsBOXF box;

    if (max <= 0) 
	{
		max = 1;
	}
	
    if (segments <= 0) 
	{
		segments = 1;
	}

    filledSegs = (val * segments) / max;
    segW = (barW - (gap * (segments - 1))) / segments;

    box.attribute = 0;
    box.y = (short)y;
    box.h = (unsigned short)barH;

    for (i = 0; i < segments; i++)
    {
        sx = x + i * (segW + gap);

        box.x = (short)sx;
        box.w = (unsigned short)segW;

        if (i < filledSegs) 
		{
            box.r = fillR; box.g = fillG; box.b = fillB;
        } 
		else 
		{
            box.r = bgR; box.g = bgG; box.b = bgB;
        }

        GsSortBoxFill(&box, ot, priority);
    }
}


/*===========================================================================
    DrawBarGradient
    Solid bar where the filled portion fades from one colour to another
===========================================================================*/
void DrawBarGradient(int x, int y, int barW, int barH, int val, int max, unsigned char r1, unsigned char g1, unsigned char b1, unsigned char r2, unsigned char g2, unsigned char b2, unsigned char bgR, unsigned char bgG, unsigned char bgB, GsOT *ot, int priority)
{
    int filledW;
    int px;
    GsBOXF box;

    if (max <= 0) 
	{
		max = 1;
	}
	
    filledW = (val * barW) / max;
	
    if (filledW > barW) 
	{
		filledW = barW;
	}

    box.attribute = 0;
    box.y = (short)y;
    box.h = (unsigned short)barH;

    // Background track
    box.x = (short)x;
    box.w = (unsigned short)barW;
    box.r = bgR; box.g = bgG; box.b = bgB;
    GsSortBoxFill(&box, ot, priority + 1);

    // Draw filled portion one pixel column at a time with interpolated colour
    for (px = 0; px < filledW; px++)
    {
        box.x = (short)(x + px);
        box.w = 1;
        box.r = (unsigned char)(r1 + ((r2 - r1) * px) / barW);
        box.g = (unsigned char)(g1 + ((g2 - g1) * px) / barW);
        box.b = (unsigned char)(b1 + ((b2 - b1) * px) / barW);
        GsSortBoxFill(&box, ot, priority);
    }
}


/*===========================================================================
Country flags
===========================================================================*/

// Flag England
void DrawFlagEngland(int x, int y, int scale, GsOT *ot, int priority)
{
	DrawRect(x,              y,               24 * scale, 16 * scale, COL_WHITE, SOLID, ot, priority + 1);
	DrawRect(x + (10*scale), y,                4 * scale, 16 * scale, COL_RED,   SOLID, ot, priority);
	DrawRect(x,              y + (6 * scale), 24 * scale,  4 * scale, COL_RED,   SOLID, ot, priority);
}


// Flag Frane
void DrawFlagFrance(int x, int y, int scale, GsOT *ot, int priority)
{
	DrawRect(x,              y,  8 * scale, 16 * scale, COL_BLUE,  SOLID, ot, priority + 1);
	DrawRect(x + (8*scale),  y,  8 * scale, 16 * scale, COL_WHITE, SOLID, ot, priority + 1);
	DrawRect(x + (16*scale), y,  8 * scale, 16 * scale, COL_RED,   SOLID, ot, priority + 1);
}


// Flag Netherlands
void DrawFlagNetherlands(int x, int y, int scale, GsOT *ot, int priority)
{
	DrawRect(x, y,               24 * scale, 5 * scale, COL_RED,   SOLID, ot, priority + 1);
	DrawRect(x, y + (5 * scale), 24 * scale, 6 * scale, COL_WHITE, SOLID, ot, priority + 1);
	DrawRect(x, y + (11*scale),  24 * scale, 5 * scale, COL_BLUE,  SOLID, ot, priority + 1);
}


// Flag Germany
void DrawFlagGermany(int x, int y, int scale, GsOT *ot, int priority)
{
	DrawRect(x, y,               24 * scale, 5 * scale, COL_BLACK,  SOLID, ot, priority + 1);
	DrawRect(x, y + (5 * scale), 24 * scale, 6 * scale, COL_RED,    SOLID, ot, priority + 1);
	DrawRect(x, y + (11*scale),  24 * scale, 5 * scale, COL_YELLOW, SOLID, ot, priority + 1);
}
