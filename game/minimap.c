#include "minimap.h"
#include "game.h"
#include "ground.h"
#include "ai_racer.h"
#include "link_race.h"
#include "../engine/graphics.h"
#include "../engine/ui.h"

/*****************************************************
The mini-map

The selected tracks tiles are made into a small 4-bit image once, at the start of the race.
Each frame its drawn as one sprite over a semi-transparent box, with a square to represent each racers position.
*****************************************************/

// Where the image is in VRAM (a 4-bit texture page at x 512, y 256), and its CLUT
#define MINIMAP_VRAM_X   512
#define MINIMAP_VRAM_Y   256
#define MINIMAP_CLUT_X   512
#define MINIMAP_CLUT_Y   384

// The size of the map
#define MINIMAP_SIZE         48
#define MINIMAP_SIZE_SPLIT   36

// The space around the mini-map (the semi-transparent box)
#define MINIMAP_BORDER   3
#define MINIMAP_MARGIN   10
#define MINIMAP_MARGIN_SPLIT 6

// The images colours (0 = transparent for the grass)
enum { MAP_GRASS, MAP_ROAD, MAP_LINE, MAP_GRID, MAP_SAND };

// The CLUT (15-bit colours, with the top bit set so none are transparent, apart from the grass)
#define RGB15(r, g, b) (0x8000 | ((b) >> 3 << 10) | ((g) >> 3 << 5) | ((r) >> 3))
static u_short miniMapClut[16];

static u_short miniMapPixels[(MINIMAP_SIZE / 4 + 1) * MINIMAP_SIZE];
static GsSPRITE miniMapSprite;

// The tiles shown (the part of the map with track on it), and the images size
static int mapRow0, mapCol0, mapTiles, mapW, mapH;


// A map letters colour on the mini-map
static int MiniMapColour(char letter)
{
	switch (letter)
	{
		case '3': return MAP_GRASS;
		case '4': case '5': return MAP_LINE;
		case 'g': case 'h': return MAP_GRID;
		case 'v': return MAP_SAND;
		case '1': case '2': case '6': case '7': case '8': case '9':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
		case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
			return MAP_ROAD;
	}
	return MAP_GRASS;
}


// Make the selected tracks mini-map (when the race starts, after SelectTrack and the screen mode)
void InitialiseMiniMap(void)
{
	char (*tiles)[GROUND_MAX_X] = TrackTiles((selectedTrackIndex >= 0 && selectedTrackIndex < NUM_TRACKS) ? selectedTrackIndex : 0);
	int size = (NumberOfPlayers == 2) ? MINIMAP_SIZE_SPLIT : MINIMAP_SIZE;
	int r, c, r0 = GROUND_MAX_X, r1 = -1, c0 = GROUND_MAX_Z, c1 = -1, rows, cols, x, y, words;
	RECT rect;

	// The tiles with track on them
	for (r = 0; r < GROUND_MAX_X; r++)
	{
		for (c = 0; c < GROUND_MAX_Z; c++)
		{
			if (MiniMapColour(tiles[r][c]) != MAP_GRASS)
			{
				if (r < r0) r0 = r;
				if (r > r1) r1 = r;
				if (c < c0) c0 = c;
				if (c > c1) c1 = c;
			}
		}
	}

	if (r1 < 0)
	{
		r0 = c0 = 0;
		r1 = GROUND_MAX_X - 1;
		c1 = GROUND_MAX_Z - 1;
	}

	rows = r1 - r0 + 1;
	cols = c1 - c0 + 1;

	// Scaled so that the longer side is the mini-maps size
	mapRow0 = r0;
	mapCol0 = c0;
	mapTiles = (rows > cols) ? rows : cols;
	mapW = cols * size / mapTiles;
	mapH = rows * size / mapTiles;
	words = (mapW + 3) / 4;

	// The image (4 pixels to each 16 bits, the first in the lowest bits), each pixel the tile under it
	for (y = 0; y < mapH; y++)
	{
		for (x = 0; x < words * 4; x++)
		{
			int colour = MAP_GRASS;

			if (x < mapW) {
				colour = MiniMapColour(tiles[r0 + y * mapTiles / size][c0 + x * mapTiles / size]);
			}
			if ((x & 3) == 0) {
				miniMapPixels[y * words + (x >> 2)] = 0;
			}

			miniMapPixels[y * words + (x >> 2)] |= (u_short)(colour << ((x & 3) * 4));
		}
	}

	rect.x = MINIMAP_VRAM_X;
	rect.y = MINIMAP_VRAM_Y;
	rect.w = words;
	rect.h = mapH;
	LoadImage(&rect, (u_long *)miniMapPixels);

	// The colours
	for (x = 0; x < 16; x++) {
		miniMapClut[x] = 0;
	}

	miniMapClut[MAP_ROAD] = RGB15(170, 170, 180);
	miniMapClut[MAP_LINE] = RGB15(255, 255, 255);
	miniMapClut[MAP_GRID] = RGB15(210, 210, 215);
	miniMapClut[MAP_SAND] = RGB15(190, 170, 110);
	rect.x = MINIMAP_CLUT_X;
	rect.y = MINIMAP_CLUT_Y;
	rect.w = 16;
	rect.h = 1;
	LoadImage(&rect, (u_long *)miniMapClut);
	DrawSync(0);

	// The sprite (4-bit, not scaled)
	miniMapSprite.attribute = 0;
	miniMapSprite.w = mapW;
	miniMapSprite.h = mapH;
	miniMapSprite.tpage = (unsigned short)(((MINIMAP_VRAM_Y & 0x100) >> 4) | (MINIMAP_VRAM_X >> 6));
	miniMapSprite.u = 0;
	miniMapSprite.v = (u_char)(MINIMAP_VRAM_Y & 0xFF);
	miniMapSprite.cx = MINIMAP_CLUT_X;
	miniMapSprite.cy = MINIMAP_CLUT_Y;
	miniMapSprite.r = miniMapSprite.g = miniMapSprite.b = 128;
	miniMapSprite.mx = 0;
	miniMapSprite.my = 0;
	miniMapSprite.scalex = ONE;
	miniMapSprite.scaley = ONE;
	miniMapSprite.rotate = 0;
}


// A car on the mini-map (a square of the size and colour, centred where the car is)
static void DrawCarMarker(PlayerStruct *car, int mapX, int mapY, int size, int r, int g, int b, GsOT *ot, int priority)
{
	// Get the mini-map size (the whole screen, or half in a two player race)
	int fullSize = (NumberOfPlayers == 2) ? MINIMAP_SIZE_SPLIT : MINIMAP_SIZE;
	long tileSpan = (long)mapTiles * SEPERATION;
	int x = mapX + (int)((car->gsObjectCoord.coord.t[2] - (long)mapCol0 * SEPERATION) * fullSize / tileSpan);
	int y = mapY + (int)((car->gsObjectCoord.coord.t[0] - (long)mapRow0 * SEPERATION) * fullSize / tileSpan);

	// Draw the square (the car) on the mini-map
	DrawRect(x - size / 2, y - size / 2, size, size, r, g, b, SOLID, ot, priority);
}


// The mini-map for a players view (the whole screen, or their half in a two player race)
// The players car is yellow, the other players car is red and the AI racers are white
void DrawMiniMap(PlayerStruct *viewer, GsOT *ot)
{
	int split = (NumberOfPlayers == 2);
	int margin = split ? MINIMAP_MARGIN_SPLIT : MINIMAP_MARGIN;
	int height = split ? gScreenHeight / 2 : gScreenHeight;
	int x = margin + MINIMAP_BORDER;
	int y = height - margin - MINIMAP_BORDER - mapH;
	PlayerStruct *other = (viewer == &player1) ? &player2 : &player1;
	int i;

	// Draw the semi-transparent box
	DrawRect(x - MINIMAP_BORDER, y - MINIMAP_BORDER, mapW + MINIMAP_BORDER * 2, mapH + MINIMAP_BORDER * 2, 0, 0, 0, TRANS_HALF, ot, OT_UI + 3);
	
	// Draw the mini-map sprite (the track) over the semi-transparent box
	miniMapSprite.x = x;
	miniMapSprite.y = y;
	GsSortSprite(&miniMapSprite, ot, OT_UI + 2);

	// Draw the AI racers on the mini-map (if any)
	if (AIRacersActive())
	{
		for (i = 0; i < NUM_AI_RACERS; i++) {
			DrawCarMarker(&aiRacers[i].player, x, y, 3, 255, 255, 255, ot, OT_UI + 1);
		}
	}

	// Draw the other players car on the mini-map (if any)
	if (NumberOfPlayers == 2 || LinkRace_IsActive()) {
		DrawCarMarker(other, x, y, 3, 255, 60, 60, ot, OT_UI + 1);
	}

	// Draw the players car on the mini-map
	DrawCarMarker(viewer, x, y, 4, 255, 220, 0, ot, OT_UI);
}
