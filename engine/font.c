/******************************************************************************
    NET YAROZE: FONT FX MODULE
    --------------------------
    Renders text using the 8DOTFONT.TIM spritesheet loaded from the AUTO file

    The font TIM is a 4bpp indexed image containing 8x8 pixel characters
    arranged in a 32-column grid starting at ASCII 33 (space+1)

    Characters are rendered pixel by pixel using GsSortBoxFill, which avoids
    the need for VRAM sprite uploads entirely

    AUTO file setup:
        local dload data\8dotfont.tim 800C8000
		
	Apply and stack multiple font effects:
	- scale
	- colour
	- style
	- outline
	- fade
	- flash
	- pulse
	- shake
	- wobble
	- scroll
	- typewritter
******************************************************************************/

#include "font.h"
#include <string.h>
#include "colours.h"

// Pointer to raw 4bpp pixel data within the TIM buffer in RAM
static unsigned char *fontPixels = 0;
static int            fontLoaded = 0;

// Current draw colour
static unsigned char fontR = 255;
static unsigned char fontG = 255;
static unsigned char fontB = 255;

// Font alpha value
static int fontAlpha = 255; // 0–255

// Offset for switching the font style
static int fontStyleOffset = 0;

// Scale value for scaling the font
static int fontScale = 1;

// Timers for the different font effects
static int flashTimer = 0;
static int rainbowTimer = 0;
static int typeTimer = 0;
static int scrollTimer = 0;
static int fadeTimer = 0;
static int waveTimer = 0;
static int shakeTimer = 0;

// Sine table for the wavy font effect
static int waveTable[8] = {0, 1, 2, 3, 2, 1, 0, -1};

// Basic randomiser for the shaking font effect
static int ShakeRand(int seed)
{
    seed ^= seed << 5;
    seed ^= seed >> 3;
    seed ^= seed << 2;
    return seed;
}

// Struct for the font effects
typedef struct FontFX
{
    int flags;

    int flashMin;
    int flashMax;
    int flashSpeed;

    int rainbowSpeed;

    int typeSpeed;

    int waveAmp;
    int waveSpeed;
    int waveSpacing;

    int shakeIntensity;
    int shakeSpeed;

    int pulseMin;
    int pulseMax;
    int pulseSpeed;
	
	int scrollSpeed;
	int scrollWidth;
	
	int fadeStart;
	int fadeEnd;
	int fadeSpeed;
	int fadeLoop;
	
	int outlineR;
	int outlineG;
	int outlineB;
	
	int centerStartX;
	int centerEndX;
	
	unsigned char gradTopR, gradTopG, gradTopB;
	unsigned char gradBotR, gradBotG, gradBotB;

} FontFX;

static FontFX gFontFX;


// Resets the font effects flags
void FontFX_FontBegin(void)
{
    gFontFX.flags = 0;
}


// Resets the font effects flags
void FontFX_FontEnd(void)
{
    gFontFX.flags = 0;
}


// Update the font FX timers
void FontFX_Update(void)
{
    flashTimer++;
    rainbowTimer++;
    typeTimer++;
    waveTimer++;
    shakeTimer++;
	scrollTimer++;
	fadeTimer++;
}


// Get the text width
static int FontFX_GetTextWidth(char *text)
{
    int len = 0;

    while (text[len] != '\0' && text[len] != '\n')
        len++;

    return len * FONT_CHAR_W * fontScale;
}


// Set the alpha value
void FontFX_SetAlpha(int alfa)
{
    fontAlpha = alfa;
}


// Set the font size (1=standard, 2=medium, 3=large)
void FontFX_SetSize(int size)
{
    if (size < 1) size = 1;
    fontScale = size;
}


// Set the font colour
void FontFX_SetColour(unsigned char r, unsigned char g, unsigned char b)
{
    fontR = r;
    fontG = g;
    fontB = b;
}


// Set the font offset for selecting a font style 
void FontFX_SetStyle(int offset)
{
    fontStyleOffset = offset;
}


// Set font centre aligned
void FontFX_SetCenter(int startX, int endX)
{
    gFontFX.flags |= FONT_FX_CENTER;
    gFontFX.centerStartX = startX;
    gFontFX.centerEndX   = endX;
}


// Set gradient effect
void FontFX_SetGradient(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char r2, unsigned char g2, unsigned char b2)
{
    gFontFX.flags   |= FONT_FX_GRADIENT;
    gFontFX.gradTopR = r1; gFontFX.gradTopG = g1; gFontFX.gradTopB = b1;
    gFontFX.gradBotR = r2; gFontFX.gradBotG = g2; gFontFX.gradBotB = b2;
}


// Set scroll effects
void FontFX_SetScroll(int width, int speed)
{
    gFontFX.flags |= FONT_FX_SCROLL;
    gFontFX.scrollWidth = width;
    gFontFX.scrollSpeed = speed;
}


// Set flash effects
void FontFX_SetFlash(int min, int max, int speed)
{
    gFontFX.flags |= FONT_FX_FLASH;
    gFontFX.flashMin = min;
    gFontFX.flashMax = max;
    gFontFX.flashSpeed = speed;
}


// Set wave effects
void FontFX_SetWave(int amplitude, int speed, int spacing)
{
    gFontFX.flags |= FONT_FX_WAVE;

    gFontFX.waveAmp = amplitude;
    gFontFX.waveSpeed = speed;
    gFontFX.waveSpacing = spacing;
}


// Set shake effects
void FontFX_SetShake(int intensity, int speed)
{
    gFontFX.flags |= FONT_FX_SHAKE;
    gFontFX.shakeIntensity = intensity;
    gFontFX.shakeSpeed = speed;
}


// Set rainbow effects
void FontFX_SetRainbow(int speed)
{
    gFontFX.flags |= FONT_FX_RAINBOW;
    gFontFX.rainbowSpeed = speed;
}


// Set typewriter effects
void FontFX_SetTypewriter(int speed)
{
    gFontFX.flags |= FONT_FX_TYPE;
    gFontFX.typeSpeed = speed;
}


// Set pulse effects
void FontFX_SetPulse(int min, int max, int speed)
{
    gFontFX.flags |= FONT_FX_PULSE;

    gFontFX.pulseMin = min;
    gFontFX.pulseMax = max;
    gFontFX.pulseSpeed = speed;
}


// Set fade effects
void FontFX_SetFade(int start, int end, int speed, int loop)
{
    gFontFX.flags    |= FONT_FX_FADE;
    gFontFX.fadeStart = start;
    gFontFX.fadeEnd   = end;
    gFontFX.fadeSpeed = speed;
    gFontFX.fadeLoop  = loop;
}


// Set outline effects
void FontFX_SetOutline(int r, int g, int b)
{
    gFontFX.flags    |= FONT_FX_OUTLINE;
    gFontFX.outlineR  = r;
    gFontFX.outlineG  = g;
    gFontFX.outlineB  = b;
}


// Apply scroll effects
static int ApplyScroll(int x, int i)
{
    int charW  = FONT_CHAR_W * fontScale;
    int startX = gFontFX.scrollWidth + (i * charW);
    return startX - (scrollTimer * gFontFX.scrollSpeed);
}


// Apply flash effects
static int ApplyFlash(void)
{
    int range = gFontFX.flashMax - gFontFX.flashMin;
    int cycle = (flashTimer * gFontFX.flashSpeed) % (range * 2);

    if (cycle > range)
        return gFontFX.flashMax - (cycle - range);
    else
        return gFontFX.flashMin + cycle;
}


// Apply shake effects
static void ApplyShake(int i, int *x, int *y)
{
    int seed = (shakeTimer * gFontFX.shakeSpeed) + i * 31;

    seed = ShakeRand(seed);
    *x += (seed % (gFontFX.shakeIntensity * 2 + 1)) - gFontFX.shakeIntensity;

    seed = ShakeRand(seed);
    *y += (seed % (gFontFX.shakeIntensity * 2 + 1)) - gFontFX.shakeIntensity;
}


// Apply wave effects
static int ApplyWave(int i)
{
    int t;

    t = (waveTimer / (8 / gFontFX.waveSpeed)); // proper divisor control

    t = (t + (i * gFontFX.waveSpacing)) & 7;

    return waveTable[t] * gFontFX.waveAmp;
}


// Apply pulse effects
static int ApplyPulse(void)
{
    int range = gFontFX.pulseMax - gFontFX.pulseMin;
    int cycle = (flashTimer * gFontFX.pulseSpeed) % (range * 2);

    if (range <= 0)
        return 255;

    if (cycle > range)
        return gFontFX.pulseMax - (cycle - range);
    else
        return gFontFX.pulseMin + cycle;
}


// Apply rainbow effects
static void ApplyRainbow(int i, unsigned char *r, unsigned char *g, unsigned char *b)
{
    int phase = (rainbowTimer * gFontFX.rainbowSpeed + i * 4) % 1536;

    if (phase < 256)
    {
        *r = 255;
        *g = phase;
        *b = 0;
    }
    else if (phase < 512)
    {
        *r = 511 - phase;
        *g = 255;
        *b = 0;
    }
    else if (phase < 768)
    {
        *r = 0;
        *g = 255;
        *b = phase - 512;
    }
    else if (phase < 1024)
    {
        *r = 0;
        *g = 1023 - phase;
        *b = 255;
    }
    else if (phase < 1280)
    {
        *r = phase - 1024;
        *g = 0;
        *b = 255;
    }
    else
    {
        *r = 255;
        *g = 0;
        *b = 1535 - phase;
    }
}


// Apply fade effects
static int ApplyFade(void)
{
    int range, v;

    range = gFontFX.fadeEnd - gFontFX.fadeStart;

    if (range == 0)
        return gFontFX.fadeStart;

    if (gFontFX.fadeLoop)
    {
        int absRange = range > 0 ? range : -range;
        int cycle    = (fadeTimer * gFontFX.fadeSpeed) % (absRange * 2);

        if (cycle > absRange)
            v = gFontFX.fadeEnd - (cycle - absRange);
        else
            v = gFontFX.fadeStart + cycle;
    }
    else
    {
        if (range > 0)
        {
            v = gFontFX.fadeStart + (fadeTimer * gFontFX.fadeSpeed);
            if (v > gFontFX.fadeEnd) v = gFontFX.fadeEnd;
        }
        else
        {
            v = gFontFX.fadeStart - (fadeTimer * gFontFX.fadeSpeed);
            if (v < gFontFX.fadeEnd) v = gFontFX.fadeEnd;
        }
    }

    return v;
}


// Load the sprite sheet TIM data
void FontFX_LoadFont(void)
{
	/*****************************************************
	FontFX_LoadFont
	Parses the TIM file in RAM and stores a pointer to the pixel data.
	No VRAM upload is needed - pixels are read directly from RAM each frame.

	TIM layout (4bpp with CLUT):
		+0x00   magic + flags (8 bytes)
		+0x08   CLUT block (blockSize bytes)
		+0x08+blockSize   image block header (12 bytes)
		+0x08+blockSize+12   pixel data
	*****************************************************/
	
    unsigned char *p = FONT_TIM_ADDR + 8;
    unsigned long  blockSize;

    // Read CLUT block size and skip past it
    blockSize = (unsigned long)p[0]
              | ((unsigned long)p[1] << 8)
              | ((unsigned long)p[2] << 16)
              | ((unsigned long)p[3] << 24);

    p += blockSize;

    // p now points to the image block header
    // Pixel data begins 12 bytes into the image block
    fontPixels = p + 12;
    fontLoaded = 1;
}


void FontFX_DrawChar(int screenX, int screenY, int c, GsOT *ot, int priority)
{
	/*****************************************************
	FontFX_DrawChar
	Draws a single character at the given screen position.
	The font sheet is 256 pixels wide (128 bytes in 4bpp) x 128 pixels tall.
	Characters are 8x8 pixels arranged in 32 columns.
	4bpp pixel layout per byte:
		bits 7-4 = left/even pixel
		bits 3-0 = right/odd pixel
		value 0  = draw pixel (character)
		non-zero = background (skip)
	*****************************************************/

    int charIndex, col, row;
    int px, py;
    int byteOffset;
    unsigned char byteVal;
    unsigned char pixel;
    GsBOXF box;

    if (!fontLoaded || c < FONT_FIRST_CHAR)
        return;

    charIndex = c - FONT_FIRST_CHAR + fontStyleOffset;
    col       = charIndex % FONT_COLS;
    row       = charIndex / FONT_COLS;

    //box.attribute = 0;
	//box.attribute = (1 << 30);   // Enable semi-transparency
    box.w         = (unsigned short)fontScale;
    box.h         = (unsigned short)fontScale;
	
	if (fontAlpha == 0)
		box.attribute = (1 << 30);   // semi-transparent
	else
		box.attribute = 0;      

	/*****************************************************
	OUTLINE PASS
	Draw character in outline colour at 8 surrounding offsets.
	Offset is always 1 pixel regardless of fontScale to keep
	the primitive count manageable and the outline looking clean.
	Drawn behind fill, so higher priority number.
	*****************************************************/
	if (gFontFX.flags & FONT_FX_OUTLINE)
	{
		unsigned char saveR = fontR;
		unsigned char saveG = fontG;
		unsigned char saveB = fontB;

		static int ox[4] = {  0,  0, -1, 1 };
		static int oy[4] = { -1,  1,  0, 0 };
		int d, spanStart, spanLen;

		box.r = (unsigned char)gFontFX.outlineR;
		box.g = (unsigned char)gFontFX.outlineG;
		box.b = (unsigned char)gFontFX.outlineB;
		box.h = (unsigned short)fontScale;

		for (d = 0; d < 4; d++)
		{
			for (py = 0; py < FONT_CHAR_H; py++)
			{
				spanStart = -1;
				spanLen   =  0;

				for (px = 0; px < FONT_CHAR_W; px++)
				{
					byteOffset = (row * FONT_CHAR_H + py) * 128
							   + (col * FONT_CHAR_W + px) / 2;

					byteVal = fontPixels[byteOffset];

					if (px & 1)
						pixel = (byteVal >> 4) & 0x0F;
					else
						pixel = byteVal & 0x0F;

					if (pixel == 0)
					{
						/* Start or extend a span */
						if (spanStart < 0)
							spanStart = px;
						spanLen++;
					}
					else
					{
						/* End of span — flush it */
						if (spanStart >= 0)
						{
							box.x = (short)(screenX + spanStart * fontScale + ox[d]);
							box.y = (short)(screenY + py        * fontScale + oy[d]);
							box.w = (unsigned short)(spanLen * fontScale);
							GsSortBoxFill(&box, ot, priority + 1);
							spanStart = -1;
							spanLen   =  0;
						}
					}
				}

				/* Flush any span still open at end of row */
				if (spanStart >= 0)
				{
					box.x = (short)(screenX + spanStart * fontScale + ox[d]);
					box.y = (short)(screenY + py        * fontScale + oy[d]);
					box.w = (unsigned short)(spanLen * fontScale);
					GsSortBoxFill(&box, ot, priority + 1);
				}
			}
		}

		fontR = saveR;
		fontG = saveG;
		fontB = saveB;
	}

	/*****************************************************
	FILL PASS
	Draw character in current font colour.
	Drawn in front, so lower priority number.
	*****************************************************/
	box.r = fontR;
	box.g = fontG;
	box.b = fontB;
	box.h = (unsigned short)fontScale;

	for (py = 0; py < FONT_CHAR_H; py++)
	{
		int spanStart = -1;
		int spanLen   =  0;

		// Calculate row colour — gradient or flat
		if (gFontFX.flags & FONT_FX_GRADIENT)
		{
			// Interpolate between top and bottom colour across FONT_CHAR_H rows
			box.r = (unsigned char)(gFontFX.gradTopR + ((gFontFX.gradBotR - gFontFX.gradTopR) * py) / (FONT_CHAR_H - 1));
			box.g = (unsigned char)(gFontFX.gradTopG + ((gFontFX.gradBotG - gFontFX.gradTopG) * py) / (FONT_CHAR_H - 1));
			box.b = (unsigned char)(gFontFX.gradTopB + ((gFontFX.gradBotB - gFontFX.gradTopB) * py) / (FONT_CHAR_H - 1));
		}
		else
		{
			box.r = fontR;
			box.g = fontG;
			box.b = fontB;
		}

		for (px = 0; px < FONT_CHAR_W; px++)
		{
			byteOffset = (row * FONT_CHAR_H + py) * 128 + (col * FONT_CHAR_W + px) / 2;
			byteVal = fontPixels[byteOffset];

			if (px & 1)
				pixel = (byteVal >> 4) & 0x0F;
			else
				pixel = byteVal & 0x0F;

			if (pixel == 0)
			{
				if (spanStart < 0)
					spanStart = px;
				spanLen++;
			}
			else
			{
				if (spanStart >= 0)
				{
					box.x = (short)(screenX + spanStart * fontScale);
					box.y = (short)(screenY + py        * fontScale);
					box.w = (unsigned short)(spanLen * fontScale);
					GsSortBoxFill(&box, ot, priority);
					spanStart = -1;
					spanLen   =  0;
				}
			}
		}

		// flush open span
		if (spanStart >= 0)
		{
			box.x = (short)(screenX + spanStart * fontScale);
			box.y = (short)(screenY + py        * fontScale);
			box.w = (unsigned short)(spanLen * fontScale);
			GsSortBoxFill(&box, ot, priority);
		}
	}
}


// Print font with visual effects applied
void FontFX_Print(int x, int y, char *text, GsOT *ot, int pri)
{
    int i, len;
    int cx, cy;
    int xx, yy;
    char c;

    unsigned char r0, g0, b0;
    int visible;

    if (!fontLoaded)
        return;

    len = strlen(text);

	cy = y;
	
	// Centre the font horizontally if the flag is set
	if (gFontFX.flags & FONT_FX_CENTER)
	{
		int width;
		int areaWidth;
		int areaCenter;

		width = FontFX_GetTextWidth(text);

		areaWidth  = gFontFX.centerEndX - gFontFX.centerStartX;
		areaCenter = gFontFX.centerStartX + (areaWidth / 2);

		cx = areaCenter - (width / 2);
	}
	else
	{
		cx = x;
	}
	
    r0 = fontR;
    g0 = fontG;
    b0 = fontB;

    for (i = 0; i < len; i++)
    {
        xx = cx;
        yy = cy;
		
		// Font scrolling
		if (gFontFX.flags & FONT_FX_SCROLL)
		{
			int charW = FONT_CHAR_W * fontScale;
			xx = ApplyScroll(x, i);

			if (xx < -charW || xx > gFontFX.scrollWidth)
			{
				cx += charW;
				continue;
			}
		}
				
        c  = text[i];
		
        // Handle newline
        if (c == '\n')
        {
            cx = x;
            cy += FONT_CHAR_H * fontScale;
            continue;
        }

        // Typewriter (visibility gate)
        if (gFontFX.flags & FONT_FX_TYPE)
        {
            visible = typeTimer / gFontFX.typeSpeed;

            if (i >= visible)
                break;
        }

        // Position FX (shake / wave)
        if (gFontFX.flags & FONT_FX_SHAKE)
            ApplyShake(i, &xx, &yy);

        if (gFontFX.flags & FONT_FX_WAVE)
            yy += ApplyWave(i);

        // Reset base colour
        fontR = r0;
        fontG = g0;
        fontB = b0;

        // Colour FX (rainbow first)
        if (gFontFX.flags & FONT_FX_RAINBOW)
        {
            ApplyRainbow(i, &fontR, &fontG, &fontB);
        }
		
		// Fade (brightness multiplier)
		if (gFontFX.flags & FONT_FX_FADE)
		{
			int v = ApplyFade();
			fontR = (fontR * v) / 255;
			fontG = (fontG * v) / 255;
			fontB = (fontB * v) / 255;
		}
		
        // Pulse / Flash (brightness modulation)
        if (gFontFX.flags & (FONT_FX_FLASH | FONT_FX_PULSE))
		{
			int v;

			if (gFontFX.flags & FONT_FX_PULSE)
				v = ApplyPulse();
			else
				v = ApplyFlash();

			fontR = (fontR * v) / 255;
			fontG = (fontG * v) / 255;
			fontB = (fontB * v) / 255;
		}
	
        // Draw char
        FontFX_DrawChar(xx, yy, c, ot, pri);

        // Advance cursor for the next char
        cx += FONT_CHAR_W * fontScale;
    }
}


// Print font without any visual effects applied
void FontFX_PrintWithoutEffects(int screenX, int screenY, char *text, int style, int r, int g, int b, GsOT *ot, int priority)
{
	/*****************************************************
	FontFX_PrintWithoutEffects
	Draws a null-terminated string at the given screen position
	Supports '\n' for newlines.
	Each character is drawn using FontFX_DrawChar()
	*****************************************************/
	
    int i, len, curX, curY;
    int c;
	
	// If font graphics were not loaded, do nothing
    if (!fontLoaded)
        return;

    // Set the font style
    fontStyleOffset = style;

    // Set the font colour
    fontR = (unsigned char)r;
    fontG = (unsigned char)g;
    fontB = (unsigned char)b;

	// Get string length
    len  = strlen(text);
	
	// Start drawing at requested screen position
    curX = screenX;
    curY = screenY;

	// Process each character in the string
    for (i = 0; i < len; i++)
    {
        c = (unsigned char)text[i];

		// Handle newline character
        if (c == '\n')
        {
			// Return to starting X position
            curX = screenX;
			
			// Move down one text row
            curY += FONT_CHAR_H * fontScale;
			
            continue;
        }

		// Draw current character
        FontFX_DrawChar(curX, curY, c, ot, priority);
		
		// Move cursor right for next character
        curX += FONT_CHAR_W * fontScale;
    }
}


// Print between start/end point
void FontFX_PrintCentered(int startX, int endX, int y, char *text, int style, int r, int g, int b, GsOT *ot, int priority)
{
    int width = FontFX_GetTextWidth(text);
    int center = startX + ((endX - startX) / 2);
    int x = center - (width / 2);

    FontFX_PrintWithoutEffects(x, y, text, style, r, g, b, ot, priority);
}


// Wrapper function for the pulse font effect
void FontFX_PrintPulse(int x, int y, char *text, int style, int r, int g, int b, int min, int max, int speed, GsOT *ot, int pri)
{
    FontFX_FontBegin();
    FontFX_SetColour(r, g, b);
    FontFX_SetStyle(style);
    FontFX_SetPulse(min, max, speed);
    FontFX_Print(x, y, text, ot, pri);
    FontFX_FontEnd();
}


// Wrapper function for the flash font effect
void FontFX_PrintFlash(int x, int y, char *text, int style, int r, int g, int b, int min, int max, int speed, GsOT *ot, int pri)
{
    FontFX_FontBegin();
    FontFX_SetColour(r, g, b);
    FontFX_SetStyle(style);
    FontFX_SetFlash(min, max, speed);
    FontFX_Print(x, y, text, ot, pri);
    FontFX_FontEnd();
}


// Wrapper function for the wave font effect
void FontFX_PrintWave(int x, int y, char *text, int style, int r, int g, int b, int amplitude, int speed, int spacing, GsOT *ot, int pri)
{
    FontFX_FontBegin();
    FontFX_SetColour(r, g, b);
    FontFX_SetStyle(style);
    FontFX_SetWave(amplitude, speed, spacing);
    FontFX_Print(x, y, text, ot, pri);
    FontFX_FontEnd();
}


// Wrapper function for the shake font effect
void FontFX_PrintShake(int x, int y, char *text, int style, int r, int g, int b, int intensity, int speed, GsOT *ot, int pri)
{
    FontFX_FontBegin();
    FontFX_SetColour(r, g, b);
    FontFX_SetStyle(style);
    FontFX_SetShake(intensity, speed);
    FontFX_Print(x, y, text, ot, pri);
    FontFX_FontEnd();
}


// Wrapper function for the rainbow font effect
void FontFX_PrintRainbow(int x, int y, char *text, int style, int speed, GsOT *ot, int pri)
{
    FontFX_FontBegin();
    FontFX_SetStyle(style);
    FontFX_SetRainbow(speed);
    FontFX_Print(x, y, text, ot, pri);
    FontFX_FontEnd();
}


// Wrapper function for the typewriter font effect
void FontFX_PrintTypewriter(int x, int y, char *text, int style, int r, int g, int b, int speed, GsOT *ot, int pri)
{
    FontFX_FontBegin();
    FontFX_SetColour(r, g, b);
    FontFX_SetStyle(style);
    FontFX_SetTypewriter(speed);
    FontFX_Print(x, y, text, ot, pri);
    FontFX_FontEnd();
}


// Wrapper function for the scrolling font effect
void FontFX_PrintScroll(int x, int y, char *text, int style, int r, int g, int b, int width, int speed, GsOT *ot, int pri)
{
    FontFX_FontBegin();
    FontFX_SetColour(r, g, b);
    FontFX_SetStyle(style);
    FontFX_SetScroll(width, speed);
    FontFX_Print(x, y, text, ot, pri);
    FontFX_FontEnd();
}


// Wrapper function for the fading font effect
void FontFX_PrintFade(int x, int y, char *text, int style, int r, int g, int b, int start, int end, int speed, int loop, GsOT *ot, int pri)
{
    FontFX_FontBegin();
    FontFX_SetColour(r, g, b);
    FontFX_SetStyle(style);
    FontFX_SetFade(start, end, speed, loop);
    FontFX_Print(x, y, text, ot, pri);
    FontFX_FontEnd();
}


// Wrapper function for the outline font effect
void FontFX_PrintOutline(int x, int y, char *text, int style, int r, int g, int b, int ro, int go, int bo, GsOT *ot, int pri)
{
    FontFX_FontBegin();
    FontFX_SetColour(r, g, b);
    FontFX_SetStyle(style);
    FontFX_SetOutline(ro, go, bo);
    FontFX_Print(x, y, text, ot, pri);
    FontFX_FontEnd();
}


// Wrapper function for the gradient font effect
void FontFX_PrintGradient(int x, int y, char *text, int style, int r1, int g1, int b1, int r2, int g2, int b2, GsOT *ot, int pri)
{
    FontFX_FontBegin();
    FontFX_SetStyle(style);
    FontFX_SetGradient(r1, g1, b1, r2, g2, b2);
    FontFX_Print(x, y, text, ot, pri);
    FontFX_FontEnd();
}