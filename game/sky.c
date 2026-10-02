#include <libps.h>
#include "sky.h"
#include "world.h"
#include "player.h"
#include "../engine/graphics.h"

// Sprite for the skybox image
static GsSPRITE skySpr;

// Two texture pages because the image is 512px wide
static unsigned short tpageL, tpageR;

// The skys top row in its texture page
static u_char skyV;

// Whether the bottom of the sky fades to black (for the nights black fog)
static int skyFade = 0;

// Used for creating the sky fading effect (the bottom of the sky fades to black, for the nights black fog)
#define SKY_HEIGHT       100
#define SKY_FADE_HEIGHT  40
#define SKY_FADE_STEP    4

// Fixed-point drift accumulator (4 fractional bits, range 0..8191 = 512 pixels * 16)
// Increments by 2 per frame => 6 pixels/second at 50fps, full cycle 82 seconds
static long skyDrift = 0;


// Initialise the skybox image (currently a 16-bit image)
void InitialiseSky(u_long texMemAddr) {
    GsIMAGE tim;

    LoadTexture(texMemAddr);
    GsGetTimInfo((u_long *)(texMemAddr + 4), &tim);

    // Left tpage = image pixels 0-255 (VRAM x=512, tpage col 8)
    // Right tpage = image pixels 256-511 (VRAM x=768, tpage col 12)
    tpageL = (unsigned short)((2 << 7) | ((tim.py & 0x100) >> 4) | (tim.px >> 6));
    tpageR = (unsigned short)((2 << 7) | ((tim.py & 0x100) >> 4) | ((tim.px + 256) >> 6));

	// Set the sprite attributes
    skySpr.attribute = (2 << 24);
    skySpr.y  = 0;
    skySpr.h  = 100;
    skyV      = (u_char)(tim.py & 0xFF);
    skySpr.v  = skyV;
    skySpr.cx = 0;
    skySpr.cy = 0;
    skySpr.r  = skySpr.g = skySpr.b = 128;
    skySpr.mx = 0;
    skySpr.my = 0;
    skySpr.scalex = ONE;
    skySpr.scaley = ONE;
    skySpr.rotate = 0;
}


// Fade the bottom of the sky to black (on for black fog)
void SetSkyFade(int on) {
    skyFade = on;
}


// Draw one horizontal band of the sky (rows y to y + h) at a brightness (128 = normal)
static void DrawSkyBand(GsOT *ot, int scroll, int y, int h, int bright) {
    int seg_img = scroll;          // Current position inside the 512-pixel sky image
    int seg_screen = 0;            // Current X position on screen
    int remaining = gScreenWidth;  // Number of screen pixels still needing to be drawn
    int u;                         // U coordinate within the current texture page (0-255)
    int draw_w;                    // Width of the current sprite segment

    skySpr.y = (short)y;
    skySpr.h = (u_short)h;
    skySpr.v = (u_char)(skyV + y);
    skySpr.r = skySpr.g = skySpr.b = (u_char)bright;

    // At most 3 iterations: one per tpage segment crossing
    while (remaining > 0) {

		// Offset within current 256-px tpage
        u = seg_img & 255;

		// Maximum width we can draw before reaching the end of the current texture page
		draw_w = 256 - u;

		// Dont draw beyond the remaining screen area
        if (draw_w > remaining) {
			draw_w = remaining;
		}

		// Configure the sprite segment
        skySpr.x = (short)seg_screen;
        skySpr.w = (u_short)draw_w;
        skySpr.u = (u_char)u;

		// Select the correct texture page
        skySpr.tpage = (seg_img < 256) ? tpageL : tpageR;

		// Send sprite to the ordering table
        GsSortSprite(&skySpr, ot, (1 << OT_LENGTH) - 2);

		// Advance screen position by the amount just drawn
        seg_screen += draw_w;

		// Wrap at 512 (allowing the sky texture to loop seamlessly)
        seg_img = (seg_img + draw_w) & 511;

		// Reduce the amount of screen still needing coverage
        remaining -= draw_w;
    }
}


// Draw a horizontally scrolling sky panorama
// The sky texture is 512px wide and split across two 256-pixel texture pages
// We need to draw portions from both pages
void DrawSky(GsOT *ot) {

    int scroll;       // Horizontal sky offset in texture space (0-511)
    int y;

    // Advance cloud drift (6 pixels/second at 50fps)
    skyDrift = (skyDrift + 2) & 8191;

    // Combine player yaw with cloud drift
    scroll = (int)((player1.rotation.vy * 512L) / (long)ONE);
    scroll = ((scroll + (int)(skyDrift >> 4)) % 512 + 512) % 512;

    if (!skyFade) {
        DrawSkyBand(ot, scroll, 0, SKY_HEIGHT, 128);
        return;
    }

    // The sky, then the bands fading to black at the horizon
    DrawSkyBand(ot, scroll, 0, SKY_HEIGHT - SKY_FADE_HEIGHT, 128);
    for (y = SKY_HEIGHT - SKY_FADE_HEIGHT; y < SKY_HEIGHT; y += SKY_FADE_STEP) {
        int bright = 128 * (SKY_HEIGHT - SKY_FADE_STEP - y) / (SKY_FADE_HEIGHT - SKY_FADE_STEP);
        DrawSkyBand(ot, scroll, y, SKY_FADE_STEP, bright);
    }
}
