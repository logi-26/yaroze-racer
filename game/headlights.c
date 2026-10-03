#include <libps.h>
#include "headlights.h"
#include "player.h"
#include "game.h"
#include "world.h"
#include "ai_racer.h"
#include "link_race.h"
#include "../engine/graphics.h"
#include "../engine/model.h"

/*****************************************************
Headlights

Each car lights up the road in front of it with a beam. It is a grid of polygons with a
colour at each corner (bright in the middle, fading to the sides and into the distance),
drawn with additive semi-transparency so it brightens whatever is under it. 

The barriers, buildings and cars in a cars headlights are lit by them.
Just before one is drawn, light 2 (normally turned off) is turned on, shining the same direction that 
the car is facing, as strong as the beam is where the object is (HeadlightsOn), and turned off again after it (HeadlightsOff).

The cars head and tail lights glow (using additive sprites at the lights location)
*****************************************************/

// Where the lights are on each car model (the car's own coordinates: X right, Y down, Z forward)
typedef struct { short side, y, z; } LampPos;
typedef struct { long tmd; LampPos head, tail; short halfWidth, height, front, back; } CarLamps;
static const CarLamps carLamps[] = {
	{ CAR_5G_MEM_ADDR, {  89,  -95,  554 }, { 102, -147, -532 }, 209, 303, 559, -546 },    // car5
	{ CAR_3Y_MEM_ADDR, {  87, -150,  423 }, { 100, -148, -416 }, 199, 316, 425, -417 },    // car3
	{ CAR_2B_MEM_ADDR, { 134, -143,  465 }, { 134, -171, -484 }, 217, 326, 485, -495 },    // car2
};
#define NUM_CAR_LAMPS ((int)(sizeof(carLamps) / sizeof(carLamps[0])))

// The size of a glow in the world, and its colours (128 = the texture as it is)
#define GLOW_SIZE       170
#define TAIL_GLOW_SIZE  110
static const u_char headGlow[3] = { 150, 140, 115 };

// The tail lights glow faintly, and brighter while the car is braking
static const u_char tailGlow[3] = { 70, 12, 8 };
static const u_char brakeGlow[3] = { 220, 36, 26 };

// The beams shape
#define BEAM_NEAR        450
#define BEAM_FAR         3450
#define BEAM_HALF_NEAR   350
#define BEAM_HALF_FAR    1300
#define LIT_SPILL        700

// The headlights light on the objects at full strength
static const u_char litColour[3] = { 0x50, 0x4c, 0x40 };

// The light on the objects comes in gradually from LIT_START in front of the car to full at LIT_FULL,
// then fades out to the end of the beam (so nothing flashes on and off as the cars go past it)
#define LIT_START        300
#define LIT_FULL         1500
static int headlightLightOn = 0;

// The beam (one drawable, placed on each car in turn)
static GsDOBJ2 beamObj;
static GsCOORDINATE2 beamCoord;

// The glow texture holds the glow at several sizes (scaled sprites go through the GTE and pick up
// whatever rotation and position it was last left with, so the glows aren't scaled)
static const u_char glowSizes[][3] = {
	{ 6, 56, 92 }, { 8, 48, 92 }, { 12, 48, 80 }, { 16, 48, 64 },
	{ 24, 32, 112 }, { 32, 0, 112 }, { 48, 0, 64 }, { 64, 0, 0 }
};
#define NUM_GLOW_SIZES ((int)(sizeof(glowSizes) / sizeof(glowSizes[0])))

static GsSPRITE glowSpr;
static u_char glowU, glowV;        // Where the glow texture is in its texture page

// The lights follow the car body
static MATRIX modelBase;

// The size each lights glow had last frame, so it only changes size when its clearly nearer or further 
// (not back and forth with the camera sway)
#define MAX_LIT_CARS (2 + NUM_AI_RACERS)
#define NO_SIZE 0xFF
static u_char glowSizeUsed[2][MAX_LIT_CARS][2];


void InitialiseHeadlights(void) {
	unsigned long *tmd;
	GsIMAGE tim;

	LoadTexture(HEADLIGHT_BEAM_TEX_MEM_ADDR);
	LoadTexture(HEADLIGHT_GLOW_TEX_MEM_ADDR);

	// The beam model (past the TMD's ID), linked to a coordinate placed on each car
	tmd = (unsigned long *)HEADLIGHT_BEAM_MEM_ADDR + 1;
	GsMapModelingData(tmd);
	GsInitCoordinate2(WORLD, &beamCoord);
	GsLinkObject4((unsigned long)(tmd + 2), &beamObj, 0);
	beamObj.coord2 = &beamCoord;
	beamObj.attribute = 0;

	// The glow sprite (4-bit, semi-transparent and additive)
	GsGetTimInfo((u_long *)(HEADLIGHT_GLOW_TEX_MEM_ADDR + 4), &tim);
	glowSpr.attribute = (1 << 30) | (1 << 28);
	glowSpr.tpage = GetTPage(0, 1, tim.px, tim.py);
	glowU = (u_char)((tim.px & 63) * 4);
	glowV = (u_char)(tim.py & 255);
	glowSpr.cx = tim.cx;
	glowSpr.cy = tim.cy;
	glowSpr.scalex = glowSpr.scaley = ONE;
	glowSpr.rotate = 0;

	{
		SVECTOR base;
		base.vx = 3072;
		base.vy = 2048;
		base.vz = 0;
		RotMatrix(&base, &modelBase);
	}

	{
		int a, b, c;
		for (a = 0; a < 2; a++)
			for (b = 0; b < MAX_LIT_CARS; b++)
				for (c = 0; c < 2; c++)
					glowSizeUsed[a][b][c] = NO_SIZE;
	}
}


// A point on a car in the cameras coordinates (x right, y down, z = depth): through the cars
// matrix to the world, then through the world-screen matrix of the camera (GsWSMATRIX)
static void CarPointToCamera(PlayerStruct *car, long lx, long ly, long lz, long *out) {
	MATRIX *m = &car->gsObjectCoord.coord;
	long w[3];
	int r;

	for (r = 0; r < 3; r++)
		w[r] = ((m->m[r][0] * lx + m->m[r][1] * ly + m->m[r][2] * lz) >> 12) + m->t[r];
	for (r = 0; r < 3; r++)
		out[r] = ((GsWSMATRIX.m[r][0] * w[0] + GsWSMATRIX.m[r][1] * w[1] + GsWSMATRIX.m[r][2] * w[2]) >> 12)
			+ GsWSMATRIX.t[r];
}


// Where a point in the cameras coordinates is on the screen
static void ToScreen(long *cam, long *sx, long *sy) {
	int centreY = (NumberOfPlayers == 2) ? gScreenHeight / 4 : gScreenHeight / 2;

	*sx = gScreenWidth / 2 + cam[0] * RACE_PROJECTION / cam[2];
	*sy = centreY + cam[1] * RACE_PROJECTION / cam[2];
}


// A cars lamps
static const CarLamps *LampsOf(PlayerStruct *car) {
	unsigned long model = (unsigned long)car->gsObjectHandler.tmd;
	const CarLamps *found = &carLamps[0];
	int i;

	for (i = 0; i < NUM_CAR_LAMPS; i++)
		if ((unsigned long)carLamps[i].tmd <= model && (unsigned long)carLamps[i].tmd > (unsigned long)found->tmd)
			found = &carLamps[i];
	return found;
}


// The rectangle each car covers on the screen and how near its nearest corner is
// (used to avoid drawing glows that are hidden by nearer cars)
typedef struct { PlayerStruct *car; long x0, y0, x1, y1, nearest; } CarBox;
static CarBox carBoxes[2 + NUM_AI_RACERS];
static int numCarBoxes;

static void AddCarBox(PlayerStruct *viewer, PlayerStruct *car) {
	const CarLamps *c = LampsOf(car);
	CarBox *b = &carBoxes[numCarBoxes];
	long cam[3], sx, sy;
	int i;

	if (!IsObjectInView(viewer, &car->gsObjectCoord, VIEW_AHEAD, VIEW_BEHIND, VIEW_SIDE))
		return;

	b->x0 = b->y0 = b->nearest = 0x7fffffff;
	b->x1 = b->y1 = -0x7fffffff;
	for (i = 0; i < 8; i++) {
		CarPointToCamera(car, (i & 1) ? c->halfWidth : -c->halfWidth, (i & 2) ? -c->height : 0,
			(i & 4) ? c->front : c->back, cam);
		if (cam[2] < 200)
			return;                 // Partly behind the camera
		ToScreen(cam, &sx, &sy);
		if (sx < b->x0) b->x0 = sx;
		if (sx > b->x1) b->x1 = sx;
		if (sy < b->y0) b->y0 = sy;
		if (sy > b->y1) b->y1 = sy;
		if (cam[2] < b->nearest) b->nearest = cam[2];
	}
	b->car = car;
	numCarBoxes++;
}


// A point on a cars body to camera co-ordinates
static void CarBodyPointToCamera(PlayerStruct *car, long lx, long ly, long lz, long *out) {
	MATRIX *b = &car->gsModelCoord.coord;
	long p[3], q[3];
	int r;

	for (r = 0; r < 3; r++)
		p[r] = (modelBase.m[0][r] * lx + modelBase.m[1][r] * ly + modelBase.m[2][r] * lz) >> 12;
	for (r = 0; r < 3; r++)
		q[r] = ((b->m[r][0] * p[0] + b->m[r][1] * p[1] + b->m[r][2] * p[2]) >> 12) + b->t[r];
	CarPointToCamera(car, q[0], q[1], q[2], out);
}


// A glow at a point on a car
static void DrawGlow(PlayerStruct *car, long lx, long ly, long lz, long worldSize, const u_char *colour,
                     u_char *sizeUsed, GsOT *ot) {
	long cam[3];
	long size, fade, sx, sy;
	int k;
	int centreY = (NumberOfPlayers == 2) ? gScreenHeight / 4 : gScreenHeight / 2;

	CarBodyPointToCamera(car, lx, ly, lz, cam);
	if (cam[2] < 400)
		return;

	// Faded by the fog, and too small to see far away
	fade = 256 - FogAmount(cam[2]);
	size = worldSize * RACE_PROJECTION / cam[2];
	if (fade <= 0 || size < glowSizes[0][0] - 2)
		return;

	sx = gScreenWidth / 2 + cam[0] * RACE_PROJECTION / cam[2];
	sy = centreY + cam[1] * RACE_PROJECTION / cam[2];
	if (sx < -size || sx > gScreenWidth + size || sy < -size || sy > gScreenHeight + size)
		return;

	// Hidden by another car thats nearer
	for (k = 0; k < numCarBoxes; k++) {
		CarBox *b = &carBoxes[k];
		if (b->car != car && b->nearest < cam[2] && sx >= b->x0 && sx <= b->x1 && sy >= b->y0 && sy <= b->y1)
			return;
	}

	// The size of glow to use (the glow texture has several sizes, and the glows are scaled by the GTE)
	k = *sizeUsed;
	if (k >= NUM_GLOW_SIZES) {
		for (k = NUM_GLOW_SIZES - 1; k > 0 && glowSizes[k][0] > size; k--)
			;
	}
	while (k < NUM_GLOW_SIZES - 1 && size * 10 >= glowSizes[k + 1][0] * 9)
		k++;
	while (k > 0 && size * 10 < glowSizes[k][0] * 8)
		k--;
	*sizeUsed = (u_char)k;

	// Placed by its middle (mx, my)
	glowSpr.w = glowSpr.h = glowSizes[k][0];
	glowSpr.u = (u_char)(glowU + glowSizes[k][1]);
	glowSpr.v = (u_char)(glowV + glowSizes[k][2]);
	glowSpr.mx = glowSpr.my = glowSizes[k][0] / 2;
	glowSpr.x = (short)sx;
	glowSpr.y = (short)sy;
	glowSpr.r = (u_char)(colour[0] * fade >> 8);
	glowSpr.g = (u_char)(colour[1] * fade >> 8);
	glowSpr.b = (u_char)(colour[2] * fade >> 8);

	// On top of everything but the HUD
	GsSortSprite(&glowSpr, ot, OT_UI + 1);
}


// One cars beam and glows
static void DrawCarLights(PlayerStruct *viewer, PlayerStruct *car, int slot, int braking, GsOT *ot) {
	u_char *sizes = glowSizeUsed[(viewer == &player2) ? 1 : 0][slot];
	MATRIX lw, ls;
	long centre[3], ahead[3];

	if (!IsObjectInView(viewer, &car->gsObjectCoord, VIEW_AHEAD, VIEW_BEHIND, VIEW_SIDE))
		return;

	// The beam (not when the car is right by the camera, or it looks like a big white square)
	CarPointToCamera(car, 0, 0, 0, centre);
	if (centre[2] > 800 && SceneHeadlights()) {
		beamCoord.super = &car->gsObjectCoord;
		beamCoord.flg = 0;
		GsGetLws(&beamCoord, &lw, &ls);
		GsSetLsMatrix(&ls);
		GsSortObject4(&beamObj, ot, 3, (u_long *)getScratchAddr(0));
	}

	// The glows
	if (birdsEyeActive)
		return;
	CarPointToCamera(car, 0, 0, 1000, ahead);
	if (ahead[2] < centre[2]) {
		const LampPos *h = &LampsOf(car)->head;
		if (!SceneHeadlights())
			return;
		DrawGlow(car, -h->side, h->y, h->z, GLOW_SIZE, headGlow, &sizes[0], ot);
		DrawGlow(car,  h->side, h->y, h->z, GLOW_SIZE, headGlow, &sizes[1], ot);
	} else {
		const u_char *glow = braking ? brakeGlow : tailGlow;
		const LampPos *l = &LampsOf(car)->tail;
		if (!braking && !SceneTailGlow())
			return;
		DrawGlow(car, -l->side, l->y, l->z, TAIL_GLOW_SIZE, glow, &sizes[0], ot);
		DrawGlow(car,  l->side, l->y, l->z, TAIL_GLOW_SIZE, glow, &sizes[1], ot);
	}
}


// How much a cars headlights light up a point in the world (0 to 256)
static int BeamAt(PlayerStruct *car, long x, long z) {
	MATRIX *m = &car->gsObjectCoord.coord;
	long dx = x - m->t[0];
	long dz = z - m->t[2];
	long forward, side, half, t, across, along;

	if (dx > BEAM_FAR || dx < -BEAM_FAR || dz > BEAM_FAR || dz < -BEAM_FAR)
		return 0;
	forward = (dx * m->m[0][2] + dz * m->m[2][2]) >> 12;
	side = (dx * m->m[0][0] + dz * m->m[2][0]) >> 12;
	if (side < 0)
		side = -side;
	if (forward <= LIT_START || forward >= BEAM_FAR)
		return 0;

	// Fading to the edges of the beam (all the way across it)
	t = (forward - BEAM_NEAR) * 256 / (BEAM_FAR - BEAM_NEAR);
	if (t < 0)
		t = 0;
	half = BEAM_HALF_NEAR + (BEAM_HALF_FAR - BEAM_HALF_NEAR) * t / 256 + LIT_SPILL;
	if (side >= half)
		return 0;
	across = (half - side) * 256 / half;

	// Coming in gradually in front of the car, then fading out to the end of the beam
	if (forward < LIT_FULL)
		along = (forward - LIT_START) * 256 / (LIT_FULL - LIT_START);
	else
		along = (BEAM_FAR - forward) * 256 / (BEAM_FAR - LIT_FULL);
	return (int)(across * along >> 8);
}


// The car whose headlights light a point the most
static int BrightestBeam(long x, long z, PlayerStruct *self, PlayerStruct **by) {
	int best = 0, amount, i;

	*by = 0;
#define TRY(car) if ((car) != self && (amount = BeamAt((car), x, z)) > best) { best = amount; *by = (car); }
	TRY(&player1);
	if (NumberOfPlayers == 2 || LinkRace_IsActive()) {
		TRY(&player2);
	}
	if (AIRacersActive()) {
		for (i = 0; i < NUM_AI_RACERS; i++) {
			TRY(&aiRacers[i].player);
		}
	}
#undef TRY
	return best;
}


// Light up an object at (x, z) with the headlights shining on it (before it's drawn)
void HeadlightsOn(long x, long z, PlayerStruct *self) {
	PlayerStruct *by;
	GsF_LIGHT light;
	int amount, i, raceCar = (self == 0 || self == &player1 || self == &player2);

	for (i = 0; i < NUM_AI_RACERS && !raceCar; i++)
		raceCar = (self == &aiRacers[i].player);
	amount = (raceCar && SceneHeadlights()) ? BrightestBeam(x, z, self, &by) : 0;
	if (amount <= 0) {
		HeadlightsOff();
		return;
	}

	// Shining the direction that the car is pointing (and a little bit down)
	light.vx = by->gsObjectCoord.coord.m[0][2];
	light.vy = 1200;
	light.vz = by->gsObjectCoord.coord.m[2][2];
	light.r = (u_char)(litColour[0] * amount >> 8);
	light.g = (u_char)(litColour[1] * amount >> 8);
	light.b = (u_char)(litColour[2] * amount >> 8);
	GsSetFlatLight(2, &light);
	headlightLightOn = 1;
}


// Turn the headlights light off again (after the object)
void HeadlightsOff(void) {
	GsF_LIGHT off = { 0, 0, 0, 0, 0, 0 };

	if (headlightLightOn) {
		GsSetFlatLight(2, &off);
		headlightLightOn = 0;
	}
}


// The lights of every car in the race, seen from the viewers camera
void DrawHeadlights(PlayerStruct *viewer, int currentBuffer) {
	GsOT *ot = &WorldOrderingTable[currentBuffer];
	int i;

	// Where the cars are on the screen (for hiding the glows behind them)
	numCarBoxes = 0;
	AddCarBox(viewer, &player1);
	if (NumberOfPlayers == 2 || LinkRace_IsActive())
		AddCarBox(viewer, &player2);
	if (AIRacersActive()) {
		for (i = 0; i < NUM_AI_RACERS; i++)
			AddCarBox(viewer, &aiRacers[i].player);
	}

	DrawCarLights(viewer, &player1, 0, player1_isBraking, ot);

	if (NumberOfPlayers == 2 || LinkRace_IsActive())
		DrawCarLights(viewer, &player2, 1, player2_isBraking, ot);

	if (AIRacersActive()) {
		for (i = 0; i < NUM_AI_RACERS; i++)
			DrawCarLights(viewer, &aiRacers[i].player, 2 + i, aiRacers[i].braking, ot);
	}

}
