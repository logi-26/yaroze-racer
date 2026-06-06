#include <libps.h>
#include "sky.h"
#include "world.h"
#include "player.h"
#include "../engine/graphics.h"

// Sprite for the skybox image
static GsSPRITE skySpr;

// Two texture pages because the image is 512px wide
static unsigned short tpageL, tpageR;


// Initialise the skybox image (currently a 16-bit image)
void InitialiseSky(void) {
    GsIMAGE tim;

	// Load the image texture
    LoadTexture(SKY_TEX_MEM_ADDR);
    GsGetTimInfo((u_long *)(SKY_TEX_MEM_ADDR + 4), &tim);

    // Left tpage = image pixels 0-255 (VRAM x=512, tpage col 8)
    // Right tpage = image pixels 256-511 (VRAM x=768, tpage col 12)
    tpageL = (unsigned short)((2 << 7) | ((tim.py & 0x100) >> 4) | (tim.px >> 6));
    tpageR = (unsigned short)((2 << 7) | ((tim.py & 0x100) >> 4) | ((tim.px + 256) >> 6));

	// Set the sprite attributes
    skySpr.attribute = (2 << 24);
    skySpr.y  = 0;
    skySpr.h  = 100;
    skySpr.v  = 0;
    skySpr.cx = 0;
    skySpr.cy = 0;
    skySpr.r  = skySpr.g = skySpr.b = 128;
    skySpr.mx = 0;
    skySpr.my = 0;
    skySpr.scalex = ONE;
    skySpr.scaley = ONE;
    skySpr.rotate = 0;
}


// Draw a horizontally scrolling sky panorama
// The sky texture is 512px wide and split across two 256-pixel texture pages
// We need to draw portions from both pages
void DrawSky(GsOT *ot) {
	
    int scroll;       // Horizontal sky offset in texture space (0-511)
    int seg_img;      // Current position inside the 512-pixel sky image
    int seg_screen;   // Current X position on screen
    int remaining;    // Number of screen pixels still needing to be drawn
    int u;            // U coordinate within the current texture page (0-255)
    int draw_w;       // Width of the current sprite segment

    // Convert player yaw rotation into a horizontal texture offset
    scroll = (int)((player1.rotation.vy * 512L) / (long)ONE);
	
	// Ensure the result stays in the range [0, 511]
    scroll = ((scroll % 512) + 512) % 512;

	// Start drawing from the computed texture position
    seg_img = scroll;
	
	// Start drawing at the left edge of the screen
    seg_screen = 0;
	
	// Entire screen width that must be covered
    remaining = gScreenWidth;

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