#include <libps.h>
#include "world.h"
#include "sky.h"
#include "ground.h"
#include "game/player.h"
#include "ai_racer.h"
#include "link_race.h"
#include "../engine/graphics.h"
#include "../engine/calculations.h"
#include "../engine/light.h"
#include "headlights.h"

// Global variables
GsRVIEW2 Camera[2];
u_long vsyncInterval = 0;
int NumberOfPlayers = 1;
int rearViewActive = 0;
int birdsEyeActive = 0;


/*****************************************************
Model declarations
*****************************************************/

// The track's buildings and barriers, from its map file
static ModelStruct buildingModels[MAX_TRACK_BUILDINGS];
static int numBuildings = 0;
static ModelStruct barrierModels[MAX_TRACK_BARRIERS];
static int numBarriers = 0;

// The arena walls are 200 inside the near edges and 700 inside the far edges of the
// arena (where they meet, the corners have collision posts)
#define WALL_NEAR      200
#define WALL_FAR_GAP   700

// Tunnel (just testing)
ModelStruct tunnel;

// Variables for the camera sway
static long camSwayX = 0;
static long prevCarYaw = 0;


// Load from RAM into VRAM
int LoadTexture(long addr) {
	RECT rect;
	GsIMAGE tim1;

	// Get tim info/header (the pointer is incremented past the first 4 positions to get to this)
	GsGetTimInfo((u_long *)(addr+4), &tim1);

	// Set the rect struct to contain the images x and y offset, width and height
	rect.x = tim1.px;
	rect.y = tim1.py;
	rect.w = tim1.pw;
	rect.h = tim1.ph;

	// Load image from main memory to video memory
	LoadImage(&rect, tim1.pixel);

	// If image has clut we need to load it too, pmode =8 for 4 bit and 9 for 8 bit colour
	if ((tim1.pmode>>3)&0x01) 
	{
		// Set the rect struct to contain the clut's x and y offset, width and height
		rect.x = tim1.cx;
		rect.y = tim1.cy;
		rect.w = tim1.cw;
		rect.h = tim1.ch;

		// Load the clut into video memeory
		LoadImage(&rect, tim1.clut);
	}
	DrawSync(0);
	return(0);
}


/*****************************************************
Lighting

The models are lit by the GTE, which has a key light and a fill light, and an ambient level.
The ambient level and parallel lights, with fog help fade everything into the colour of the skys horizon.
It also helps to reduce the model pop-in from the culling.
There is a night and a day mode, each with its own sky, fog and lighting setup.
*****************************************************/
typedef struct {
	short vx, vy, vz;               		// The direction the light travels
	u_char r, g, b;
} LightDef;

typedef struct {
	LightDef key;                   		// The main light (the moon/sun), from high up
	LightDef fill;                  		// A weaker light from the other side, so nothing is left flat
	short ambientR, ambientG, ambientB;    	// ONE (4096) = full brightness
	u_char fogR, fogG, fogB;        		// What things fade to in the distance (the sky at the horizon)
	long fogNear, fogFar;           		// No fog up to fogNear from the camera, all fog from fogFar
	u_long sky;                     		// The sky image (SKY_TEX_MEM_ADDR/SKY_ALT_TEX_MEM_ADDR)
	int headlights;                 		// The cars headlights are on (the beams, their glows and their light)
	int tailGlow;                   		// The tail lights glow when not braking (they always do when braking)
	long viewAhead;                 		// How far ahead things are drawn (VIEW_AHEAD), and the buildings
	long buildingViewAhead;
} SceneLighting;

// Night (sky.tim)
// Dim blue light with a warmer light for the track, and things fade into the dark in the distance. 
// The fog is black because the fog only tints textured polygons (the GPU multiplies the texture by the fogged colour)
// The sky fades to black at the horizon to match.
static const SceneLighting nightLighting = {
	{ -40, 100,  60, 0x58, 0x60, 0x80 },
	{  60,  30, -80, 0x40, 0x34, 0x28 },
	0x3c0, 0x400, 0x500,
	0, 0, 0,
	4500, 11000,
	SKY_ALT_TEX_MEM_ADDR,
	1, 1,
	11000, 12000
};

// Day (sky2.tim)
// Natural sunlight from high up with a pale blue fog-haze to match the daytime sky texture.
// More pop-in than the nightime because the fog is lighter and the sky is brighter, but the track is more visible.
static const SceneLighting dayLighting = {
	{ -40, 100,  60, 0xa0, 0xa0, 0x98 },
	{  60,  30, -80, 0x40, 0x48, 0x58 },
	0x600, 0x600, 0x660,
	150, 168, 188,
	3000, 10000,
	SKY_TEX_MEM_ADDR,
	0, 0,
	11000, 12000
};

// The lighting in use (for FogAmount)
static const SceneLighting *sceneLighting = &nightLighting;

// How far ahead the buildings are drawn (they are seen from further away)
static long buildingViewAhead = 12000;


// Whether the cars headlights are on, and whether the tail lights glow when not braking
int SceneHeadlights(void) {
	return sceneLighting->headlights;
}

int SceneTailGlow(void) {
	return sceneLighting->tailGlow;
}


// How much of something at this depth (from the camera) is hidden by the fog: 0 (none) to 256 (all)
int FogAmount(long depth) {
	long amount;
	const SceneLighting *s = sceneLighting;

	if (depth <= s->fogNear)
		return 0;
	if (depth >= s->fogFar)
		return 256;
	amount = (256 * s->fogFar / (s->fogFar - s->fogNear)) * (depth - s->fogNear) / depth;
	return (int)(amount > 256 ? 256 : amount);
}


static void InitialiseLighting(const SceneLighting *s) {
	GsF_LIGHT off = { 0, 0, 0, 0, 0, 0 };
	GsFOGPARAM fog;

	sceneLighting = s;
	viewAhead = s->viewAhead;
	buildingViewAhead = s->buildingViewAhead;

	InitialiseLight(&flLights[0], 0, s->key.vx, s->key.vy, s->key.vz, s->key.r, s->key.g, s->key.b);
	InitialiseLight(&flLights[1], 1, s->fill.vx, s->fill.vy, s->fill.vz, s->fill.r, s->fill.g, s->fill.b);
	GsSetFlatLight(2, &off);
	GsSetAmbient(s->ambientR, s->ambientG, s->ambientB);

	// The GTE's depth cue
	// P = DQB + DQA * H / depth (P from 0 = none to 1 = all fog colour)
	// Fitted so that P is 0 at fogNear and 1 at fogFar
	fog.dqa = (short)((256L * s->fogNear / RACE_PROJECTION) * s->fogFar / (s->fogNear - s->fogFar));
	fog.dqb = ((1L << 24) / (s->fogFar - s->fogNear)) * s->fogFar;
	fog.rfc = s->fogR;
	fog.gfc = s->fogG;
	fog.bfc = s->fogB;
	GsSetFogParam(&fog);
	GsSetLightMode(1);

	// Beyond the view distance, the screen is cleared to the fog colour
	// and with black fog the sky fades to black at the horizon to match
	SetBackgroundColor(s->fogR, s->fogG, s->fogB);
	SetSkyFade(s->fogR == 0 && s->fogG == 0 && s->fogB == 0);
}


void InitialiseWorld() {

	// Load the ground textures into VRAM
	InitialiseGroundTextures();

	// Initialise the world ground data
	InitialiseGround();

	// Load the world textures into VRAM
	InitialiseWorldTextures();

	// Initialise the models in the world
	InitialiseWorldModels();

	// The sky and the lighting for the night or the day
	{
		const SceneLighting *s = nightMode ? &nightLighting : &dayLighting;
		InitialiseSky(s->sky);
		InitialiseLighting(s);
	}

	// The cars headlights
	InitialiseHeadlights();
}


void InitialiseWorldTextures() {

	// Load world model textures into VRAM
	LoadTexture(BARRIER_1_TEX_MEM_ADDR);
	LoadTexture(BARRIER_2_TEX_MEM_ADDR);
	LoadTexture(BUILDING_1_TEX_MEM_ADDR);
	LoadTexture(BUILDING_2_TEX_MEM_ADDR);

	LoadTexture(CROWD_TEX_MEM_ADDR);
	LoadTexture(STONE_TEX_MEM_ADDR);

	LoadTexture(SIGN_1_TEX_MEM_ADDR);
	LoadTexture(CAR_3Y_TEX_MEM_ADDR);
}


void InitialiseWorldModels() {
	int i;

	// The track's buildings
	numBuildings = (numTrackBuildings < MAX_TRACK_BUILDINGS) ? numTrackBuildings : MAX_TRACK_BUILDINGS;
	for (i = 0; i < numBuildings; i++)
	{
		const BuildingDef *b = &trackBuildings[i];
		long addr = (b->model == 2) ? (long)STAND_MEM_ADDR : (b->model == 1) ? (long)BUILDING_2_MEM_ADDR : (long)BUILDING_1_MEM_ADDR;
		InitialiseModel(&buildingModels[i], b->x, b->y, b->z, 0, 0, 0, (long*)addr);

		if (b->rot == 1)
		{
			RotateModel180(&buildingModels[i].gsObjectCoord, &buildingModels[i].rotation);
		}

		buildingModels[i].collisionRadius = b->collisionRadius;
	}

	// The track's barriers
	numBarriers = (numTrackBarriers < MAX_TRACK_BARRIERS) ? numTrackBarriers : MAX_TRACK_BARRIERS;
	for (i = 0; i < numBarriers; i++)
	{
		const BarrierDef *b = &trackBarriers[i];
		long addr = (b->model == 0) ? (long)BARRIER_1_MEM_ADDR : (long)BARRIER_2_MEM_ADDR;
		InitialiseModel(&barrierModels[i], b->x, -200, b->z, 0, 0, 0, (long*)addr);

		if (b->rot == 1)
		{
			RotateModel180(&barrierModels[i].gsObjectCoord, &barrierModels[i].rotation);
		}
		else if (b->rot == 2)
		{
			RotateModel270(&barrierModels[i].gsObjectCoord, &barrierModels[i].rotation);
		}
		else if (b->rot == 3)
		{
			RotateModel90 (&barrierModels[i].gsObjectCoord, &barrierModels[i].rotation);
		}
	}
}


void AddModelToWorld() {

}


// Approximate 2D distance without sqrt using octagonal approximation
static int OctagonalDistance(int dx, int dz) {
	int absDx = (dx < 0) ? -dx : dx;
	int absDz = (dz < 0) ? -dz : dz;
	int maxD = (absDx > absDz) ? absDx : absDz;
	int minD = (absDx > absDz) ? absDz : absDx;
	return (maxD * 123 + minD * 51) >> 7;
}


// Extra clearance added to every position push so the vehicle starts the next
// frame with a gap from the surface rather than sitting exactly on it.
// This prevents the oscillation where the following frame's movement immediately
// re-enters the barrier and triggers another response.
#define COLLISION_PUSH_MARGIN 120

// Shared response: push player clear of the surface, bounce speed, clear drift
static void ApplyCollisionResponse(PlayerStruct *player, long *lateralSpeed,
	long closestX, long closestZ, int combinedRadius) {
	int dx = (int)(player->gsObjectCoord.coord.t[0] - closestX);
	int dz = (int)(player->gsObjectCoord.coord.t[2] - closestZ);
	int approxDist = OctagonalDistance(dx, dz);
	int pushDist   = combinedRadius + COLLISION_PUSH_MARGIN;

	if (approxDist < 1) 
	{
		player->gsObjectCoord.coord.t[0] = closestX + pushDist;
	} 
	else 
	{
		player->gsObjectCoord.coord.t[0] = closestX + ((long)dx * pushDist) / approxDist;
		player->gsObjectCoord.coord.t[2] = closestZ + ((long)dz * pushDist) / approxDist;
	}

	player->gsObjectCoord.flg = 0;
	player->speed  = -(player->speed) / 3;
	*lateralSpeed  = 0;
}


// Sphere-sphere collision for rounded objects (buildings, stands)
static void TestSphereCollision(PlayerStruct *player, long *lateralSpeed, ModelStruct *model) {
	int dx, dz, combinedRadius;
	long distSq;

	dx = (int)(player->gsObjectCoord.coord.t[0] - model->gsObjectCoord.coord.t[0]);
	dz = (int)(player->gsObjectCoord.coord.t[2] - model->gsObjectCoord.coord.t[2]);
	combinedRadius = player->collisionRadius + model->collisionRadius;

	if (dx > combinedRadius || dx < -combinedRadius) 
	{
		return;
	}
	
	if (dz > combinedRadius || dz < -combinedRadius) 
	{
		return;
	}
	
	distSq = (long)dx * dx + (long)dz * dz;
	
	if (distSq >= (long)combinedRadius * combinedRadius) 
	{
		return;
	}

	ApplyCollisionResponse(player, lateralSpeed,
		model->gsObjectCoord.coord.t[0],
		model->gsObjectCoord.coord.t[2],
		combinedRadius);
}


// Capsule collision for elongated barriers
// Models the barrier as a line segment (its length) with a small cross-section radius
// axisIsZ=1: segment runs along Z; axisIsZ=0: segment runs along X
// halfLen: half the barrier length. capsuleRadius: half the barrier's thickness
static void TestCapsuleCollision(PlayerStruct *player, long *lateralSpeed, int cx, int cz, int axisIsZ, int halfLen, int capsuleRadius) {
	int playerX, playerZ, combinedRadius, relAxis;
	int closestX, closestZ, dx, dz;
	long distSq;

	playerX = (int)player->gsObjectCoord.coord.t[0];
	playerZ = (int)player->gsObjectCoord.coord.t[2];
	combinedRadius = player->collisionRadius + capsuleRadius;

	// Closest point on the capsule centre-line to the player
	if (axisIsZ) 
	{
		closestX = cx;
		relAxis  = playerZ - cz;
		if (relAxis >  halfLen) 
		{
			relAxis =  halfLen;
		}
		
		if (relAxis < -halfLen) 
		{
			relAxis = -halfLen;
		}

		closestZ = cz + relAxis;
	} 
	else 
	{
		closestZ = cz;
		relAxis  = playerX - cx;
		if (relAxis >  halfLen) 
		{
			relAxis =  halfLen;
		}
		
		if (relAxis < -halfLen) 
		{
			relAxis = -halfLen;
		}

		closestX = cx + relAxis;
	}

	dx = playerX - closestX;
	dz = playerZ - closestZ;

	if (dx > combinedRadius || dx < -combinedRadius) 
	{
		return;
	}

	if (dz > combinedRadius || dz < -combinedRadius) 
	{
		return;
	}
	
	distSq = (long)dx * dx + (long)dz * dz;
	
	if (distSq >= (long)combinedRadius * combinedRadius) 
	{
		return;
	}

	ApplyCollisionResponse(player, lateralSpeed, closestX, closestZ, combinedRadius);
}


// Sphere at a wall corner — pushes the vehicle diagonally away before both
// perpendicular capsules can fire simultaneously and trap it.
static void TestCornerCollision(PlayerStruct *player, long *lateralSpeed, int cx, int cz, int cornerRadius) {
	int dx, dz, combinedRadius;
	long distSq;

	dx = (int)(player->gsObjectCoord.coord.t[0] - cx);
	dz = (int)(player->gsObjectCoord.coord.t[2] - cz);
	combinedRadius = player->collisionRadius + cornerRadius;

	if (dx > combinedRadius || dx < -combinedRadius) 
	{
		return;
	}
	
	if (dz > combinedRadius || dz < -combinedRadius) 
	{
		return;
	}
	
	distSq = (long)dx * dx + (long)dz * dz;
	
	if (distSq >= (long)combinedRadius * combinedRadius) 
	{
		return;
	}

	ApplyCollisionResponse(player, lateralSpeed, cx, cz, combinedRadius);
}


// Objects further than this from the car on either axis can't be touching it
// (half a barrier's length plus the barrier's and the car's radius)
#define COLLISION_REACH 2000

static int IsNearCar(long px, long pz, long x, long z) {
	long dx = x - px;
	long dz = z - pz;
	return dx < COLLISION_REACH && dx > -COLLISION_REACH && dz < COLLISION_REACH && dz > -COLLISION_REACH;
}


void CheckWorldCollisions(PlayerStruct *player, long *lateralSpeed) {
	long px = player->gsObjectCoord.coord.t[0];
	long pz = player->gsObjectCoord.coord.t[2];

	// Sphere collision for buildings and stands
	{
		int i;
		for (i = 0; i < numBuildings; i++)
		{
			if (IsNearCar(px, pz, buildingModels[i].gsObjectCoord.coord.t[0], buildingModels[i].gsObjectCoord.coord.t[2]))
			{
				TestSphereCollision(player, lateralSpeed, &buildingModels[i]);
			}
		}
	}
	
	// Corner spheres (resolve the junction between perpendicular outer walls)
	// before both capsules can fire simultaneously and trap the vehicle
	{
		int nearX = (int)trackArenaX + WALL_NEAR;
		int nearZ = (int)trackArenaZ + WALL_NEAR;
		int farX  = (int)(trackArenaX + trackArenaSize) - WALL_FAR_GAP;
		int farZ  = (int)(trackArenaZ + trackArenaSize) - WALL_FAR_GAP;
		TestCornerCollision(player, lateralSpeed, nearX, nearZ, 400);
		TestCornerCollision(player, lateralSpeed, farX,  nearZ, 400);
		TestCornerCollision(player, lateralSpeed, nearX, farZ,  400);
		TestCornerCollision(player, lateralSpeed, farX,  farZ,  400);
	}

	// Capsule collision for barriers
	{
		int i;
		for (i = 0; i < numBarriers; i++)
		{
			const BarrierDef *b = &trackBarriers[i];
			if (IsNearCar(px, pz, b->x, b->z))
			{
				TestCapsuleCollision(player, lateralSpeed, b->x, b->z, (b->rot == 2 || b->rot == 3) ? 0 : 1, 975, 100);
			}
		}
	}
}


// The buildings are seen from further away than the rest of the world, so they have a larger view distance for culling
#define BUILDING_VIEW_AHEAD  buildingViewAhead
#define BUILDING_VIEW_BEHIND 4000
#define BUILDING_VIEW_SIDE   3000

// Polygon subdivision by distance from the camera (the big buildings and stands, and the barriers)
static const DivisionStep buildingDivision[] = {
	{ 10000, GsDIV2 }, { 15000, GsDIV1 }, { 0, 0 }        // (the track is 7700+ from them; 8x8 would fill the packet area)
};
static const DivisionStep barrierDivision[] = {
	{ 2500, GsDIV2 }, { 5000, GsDIV1 }, { 0, 0 }
};

static void DrawDivided(PlayerStruct *currentPlayer, ModelStruct *model, const DivisionStep *division, int currentBuffer) {
	SetDivisionByDistance(&model->gsObjectHandler,
		model->gsObjectCoord.coord.t[0] - currentPlayer->gsObjectCoord.coord.t[0],
		model->gsObjectCoord.coord.t[2] - currentPlayer->gsObjectCoord.coord.t[2], division);
	HeadlightsOn(model->gsObjectCoord.coord.t[0], model->gsObjectCoord.coord.t[2], 0);
	DrawModel(model, &WorldOrderingTable[currentBuffer]);
	HeadlightsOff();
}

static void DrawModelCulled(PlayerStruct *currentPlayer, ModelStruct *model, const DivisionStep *division, int currentBuffer) {
	if (model && IsObjectInView(currentPlayer, &model->gsObjectCoord, VIEW_AHEAD, VIEW_BEHIND, VIEW_SIDE)) 
	{
		DrawDivided(currentPlayer, model, division, currentBuffer);
	}
}

static void DrawBuildingCulled(PlayerStruct *currentPlayer, ModelStruct *model, int currentBuffer) {
	if (model && IsObjectInView(currentPlayer, &model->gsObjectCoord, BUILDING_VIEW_AHEAD, BUILDING_VIEW_BEHIND, BUILDING_VIEW_SIDE)) 
	{
		DrawDivided(currentPlayer, model, buildingDivision, currentBuffer);
	}
}


void DrawWorldModels(PlayerStruct *currentPlayer, int currentBuffer) {

	// Draw the buildings (extended draw distance) and stands
	{
		int i;
		for (i = 0; i < numBuildings; i++)
		{
			if (trackBuildings[i].model == 2)
			{
				DrawModelCulled(currentPlayer, &buildingModels[i], buildingDivision, currentBuffer);
			}
			else
			{
				DrawBuildingCulled(currentPlayer, &buildingModels[i], currentBuffer);
			}
		}
	}

	// Draw the barriers
	{
		int i;
		for (i = 0; i < numBarriers; i++)
		{
			DrawModelCulled(currentPlayer, &barrierModels[i], barrierDivision, currentBuffer);
		}
	}

	//DrawModelCulled(currentPlayer, &tunnel, currentBuffer);
}


void DrawWorld(PlayerStruct *currentPlayer, int currentBuffer) {

	// Draw the ground
	DrawGround(currentPlayer, &WorldOrderingTable[currentBuffer]);

	// Draw the world models
	DrawWorldModels(currentPlayer, currentBuffer);

	// Draw the cars light pools and head/tail light glows
	DrawHeadlights(currentPlayer, currentBuffer);

	// Draw the sky (always draw last)
	DrawSky(&WorldOrderingTable[currentBuffer]);
}


// This function renders the world for player 1
void RenderWorldPlayer1(int currentBuffer) {

	// Draw the world
	DrawWorld(&player1, currentBuffer);

	// Draw player 1
	DrawPlayer(&player1, &WorldOrderingTable[currentBuffer]);

	// Draw AI racers (none in a link race)
	if (!LinkRace_IsActive())
	{
		DrawAIRacers(&player1, &WorldOrderingTable[currentBuffer]);
	}

	// If it is 2 player mode or a link race, draw the second player vehicle
	if (NumberOfPlayers == 2 || LinkRace_IsActive()) 
	{
		DrawPlayer(&player2, &WorldOrderingTable[currentBuffer]);
	}
}


// This function renders the world for player 2
void RenderWorldPlayer2(int currentBuffer) {

	// Activates the view for the bottom screen
	GsSetRefView2(&Camera[1]);

	// Set the address of the packet area for the bottom screen
	GsSetWorkBase((PACKET*)GpuPacketArea[currentBuffer+2]);

	// Clear the ordering table for the bottom screen
	GsClearOt(0, 0, &WorldOrderingTable[currentBuffer+2]);

	// Draw the world
	DrawWorld(&player2, currentBuffer+2);

	// Draw player 1
	DrawPlayer(&player1, &WorldOrderingTable[currentBuffer+2]);

	// Draw player 2
	DrawPlayer(&player2, &WorldOrderingTable[currentBuffer+2]);
}


// Update the camera sway when the vehicle turns
static void UpdateCameraLag() {
	long carYaw = player1.rotation.vy;
	long yawDelta = carYaw - prevCarYaw;
	prevCarYaw = carYaw;

	if (yawDelta >  2048) 
	{
		yawDelta -= 4096;
	}
	
	if (yawDelta < -2048) 
	{
		yawDelta += 4096;
	}

	camSwayX -= yawDelta * 2;
	camSwayX  = camSwayX * 15 / 16;
	
	if (camSwayX >  200) 
	{
		camSwayX =  200;
	}
	
	if (camSwayX < -200) 
	{
		camSwayX = -200;
	}

	Camera[0].vpx = (int)camSwayX;
}


// This function performs all of the rendering
void RenderWorld() {
	int currentBuffer = GsGetActiveBuff();
	
	// Update the camera sway (only meaningful for the normal chase camera)
	UpdateCameraLag();

	// Set camera mode — all fields written every frame so any mode switch is clean.
	// Bird's-eye uses world-space coordinates; the other two are car-local
	// (the car sits on the ground at Y CAR_GROUND_Y: the camera heights are
	// relative to that).
	if (birdsEyeActive) 
	{
		// Fixed overhead view of the whole track.
		// Camera sits above and slightly south of centre so the view direction
		// is near-vertical but not exactly down (avoids a gimbal singularity).
		// Track spans roughly X 3000-32000, Z 3000-32000 — centre ≈ (17500, 17000).
		GsSetProjection(150);
		GsSetLightMode(0);              // No fog in birdseye view (it hides the whole track)
		Camera[0].super = WORLD;
		Camera[0].vpx = 17500;
		Camera[0].vpy = -22000;
		Camera[0].vpz = 8000;
		Camera[0].vrx = 17500;
		Camera[0].vry = 0;
		Camera[0].vrz = 17000;
		Camera[0].rz  = 0;
	} 
	else 
	{
		GsSetProjection(RACE_PROJECTION);
		GsSetLightMode(1);
		Camera[0].super = &player1.gsObjectCoord;
		Camera[0].rz = 0;
		if (rearViewActive) 
		{
			// Camera at exterior rear bumper looking backward.
			// vry=6 (at ground level) tilts the view downward enough to keep sky out of frame.
			Camera[0].vpx = 0;
			Camera[0].vpy = -694;
			Camera[0].vpz = -600;
			Camera[0].vrx = 0;
			Camera[0].vry = 6;
			Camera[0].vrz = -3000;
		} 
		else 
		{
			// Normal chase camera — vpx is left to UpdateCameraLag for sway
			Camera[0].vpy = -694;
			Camera[0].vpz = -1500;
			Camera[0].vrx = 0;
			Camera[0].vry = -394;
			Camera[0].vrz = 0;
		}
	}

	// Set the camera view
	GsSetRefView2(&Camera[0]);
	
	// Render the world for player 1
	RenderWorldPlayer1(currentBuffer);
	
	// If two player mode, render the world for player 2
	if (NumberOfPlayers == 2) 
	{
		RenderWorldPlayer2(currentBuffer);
	}
}