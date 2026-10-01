#include <libps.h>
#include "world.h"
#include "sky.h"
#include "ground.h"
#include "game/player.h"
#include "ai_racer.h"
#include "link_race.h"
#include "../engine/graphics.h"
#include "../engine/calculations.h"

// Global variables
GsRVIEW2 Camera[2];
u_long vsyncInterval = 0;
int NumberOfPlayers = 1;
int rearViewActive = 0;
int birdsEyeActive = 0;


/*****************************************************
Model declarations
*****************************************************/

// Building data table
// rot:   0 = no rotation,  1 = 180-deg
// model: 0 = BUILDING_1,   1 = BUILDING_2
typedef struct { int x; int y; int z; int rot; int model; int collisionRadius; } BuildingDef;

static const BuildingDef buildingDefs[] = {
	
	// Left row
	{ -4700, -450,     0, 0, 0, 400 },
    { -4700, -450,  5000, 0, 0, 400 },
	{ -4700, -450, 10000, 0, 0, 400 },
    { -4700, -300, 25000, 0, 0, 400 },
    { -4700, -450, 30000, 0, 0, 400 },
	{ -4700, -450, 35000, 0, 0, 400 },
	
	// Bottom row
	{ -1000, -450, 40000, 0, 0, 400 },
	{  4000, -450, 40000, 0, 0, 400 },
	{  9000, -450, 40000, 0, 0, 400 },
	{ 14000, -450, 40000, 0, 0, 400 },
	{ 19000, -450, 40000, 0, 0, 400 },
	{ 24000, -450, 40000, 0, 0, 400 },
	{ 29000, -450, 40000, 0, 0, 400 },
	{ 34000, -450, 40000, 0, 0, 400 },
	{ 39000, -450, 40000, 0, 0, 400 },
	
	// Right row
	{ 39000, -450, 35000, 0, 0, 400 },
	{ 39000, -450, 30000, 0, 0, 400 },
	{ 39000, -450, 25000, 0, 0, 400 },
	{ 39000, -450, 20000, 0, 0, 400 },
	{ 39000, -450, 15000, 0, 0, 400 },
	{ 39000, -450, 10000, 0, 0, 400 },
	{ 39000, -450,  5000, 0, 0, 400 },
	{ 39000, -450,     0, 0, 0, 400 },
	
	// Top row
	{ 35000, -450, -5000, 0, 0, 400 },
	{ 30000, -450, -5000, 0, 0, 400 },
	{ 25000, -450, -5000, 0, 0, 400 },
	{ 20000, -450, -5000, 0, 0, 400 },
	{ 15000, -450, -5000, 0, 0, 400 },
	{ 10000, -450, -5000, 0, 0, 400 },
	{  5000, -450, -5000, 0, 0, 400 },
	{     0, -450, -5000, 0, 0, 400 },

};
#define NUM_BUILDINGS ((int)(sizeof(buildingDefs) / sizeof(buildingDefs[0])))
#define MAX_BUILDINGS 40

static ModelStruct buildingModels[MAX_BUILDINGS];

// Crowd stand
ModelStruct stand1;
ModelStruct stand2;


// Barrier data table
// rot:   0 = Z-aligned (default),  1 = Z-aligned 180-deg,  2 = X-aligned 270-deg
// model: 0 = BARRIER_1,            1 = BARRIER_2
typedef struct { int x; int z; int rot; int model; } BarrierDef;

// Outer (perimeter) barriers — drawn on all maps
// rot: 0=Z-aligned  1=Z-aligned-180  2=X-aligned-270  3=X-aligned-90
static const BarrierDef outerBarrierDefs[] = {

	// Left wall
    {   200,  1200, 0, 0 }, {  200,  3150, 0, 1 },
    {   200,  5100, 0, 0 }, {  200,  7050, 0, 1 },
    {   200,  9000, 0, 0 }, {  200, 10950, 0, 1 },
    {   200, 12900, 0, 0 }, {  200, 14850, 0, 1 },
    {   200, 16800, 0, 0 }, {  200, 18750, 0, 1 },
    {   200, 20700, 0, 0 }, {  200, 22650, 0, 1 },
    {   200, 24600, 0, 0 }, {  200, 26550, 0, 1 },
    {   200, 28500, 0, 0 }, {  200, 30450, 0, 1 },
    {   200, 32400, 0, 0 }, {  200, 34350, 0, 1 },

	// Right wall
    { 35300,  1200, 1, 0 }, { 35300,  3150, 1, 1 },
    { 35300,  5100, 1, 0 }, { 35300,  7050, 1, 1 },
    { 35300,  9000, 1, 0 }, { 35300, 10950, 1, 1 },
    { 35300, 12900, 1, 0 }, { 35300, 14850, 1, 1 },
    { 35300, 16800, 1, 0 }, { 35300, 18750, 1, 1 },
    { 35300, 20700, 1, 0 }, { 35300, 22650, 1, 1 },
    { 35300, 24600, 1, 0 }, { 35300, 26550, 1, 1 },
    { 35300, 28500, 1, 0 }, { 35300, 30450, 1, 1 },
    { 35300, 32400, 1, 0 }, { 35300, 34350, 1, 1 },

	// Bottom wall
    {  1200,   200, 2, 0 }, {  3150,   200, 2, 1 },
    {  5100,   200, 2, 0 }, {  7050,   200, 2, 1 },
    {  9000,   200, 2, 0 }, { 10950,   200, 2, 1 },
    { 12900,   200, 2, 0 }, { 14850,   200, 2, 1 },
    { 16800,   200, 2, 0 }, { 18750,   200, 2, 1 },
    { 20700,   200, 2, 0 }, { 22650,   200, 2, 1 },
    { 24600,   200, 2, 0 }, { 26550,   200, 2, 1 },
    { 28500,   200, 2, 0 }, { 30450,   200, 2, 1 },
    { 32400,   200, 2, 0 }, { 34350,   200, 2, 1 },

	// Top wall
    {  1200, 35300, 3, 0 }, {  3150, 35300, 3, 1 },
    {  5100, 35300, 3, 0 }, {  7050, 35300, 3, 1 },
    {  9000, 35300, 3, 0 }, { 10950, 35300, 3, 1 },
    { 12900, 35300, 3, 0 }, { 14850, 35300, 3, 1 },
    { 16800, 35300, 3, 0 }, { 18750, 35300, 3, 1 },
    { 20700, 35300, 3, 0 }, { 22650, 35300, 3, 1 },
    { 24600, 35300, 3, 0 }, { 26550, 35300, 3, 1 },
    { 28500, 35300, 3, 0 }, { 30450, 35300, 3, 1 },
    { 32400, 35300, 3, 0 }, { 34350, 35300, 3, 1 },

};
#define NUM_OUTER_BARRIERS ((int)(sizeof(outerBarrierDefs) / sizeof(outerBarrierDefs[0])))
#define MAX_OUTER_BARRIERS 80

static ModelStruct outerBarrierModels[MAX_OUTER_BARRIERS];

// Inner barriers for map_1
static const BarrierDef innerBarrierDefsMap1[] = {

	// Inner track barriers
    {  5300,  7000, 1, 0 }, {  5300,  8950, 1, 1 },
    {  5300, 10900, 1, 0 }, {  5300, 12850, 1, 1 },
    {  5300, 14800, 1, 0 }, {  5300, 16750, 1, 1 },
    {  5300, 18700, 1, 0 }, {  5300, 20650, 1, 1 },
    {  5300, 22600, 1, 0 }, {  5300, 24550, 1, 1 },
    {  5300, 26500, 1, 0 }, {  5300, 28450, 1, 1 },

    // Inner track bottom
    {  6290, 29400, 2, 0 }, {  8240, 29400, 2, 1 },
    { 10190, 29400, 2, 0 }, { 12140, 29400, 2, 1 },
	{ 14090, 29400, 2, 0 }, { 16040, 29400, 2, 1 },
	{ 17990, 29400, 2, 0 }, { 19940, 29400, 2, 1 },
	{ 21890, 29400, 2, 0 }, { 23840, 29400, 2, 1 },
	{ 25790, 29400, 2, 0 }, { 27740, 29400, 2, 1 },

	// Runs parallel with the previous barriers but facing the opposite direction
	{  8240, 29395, 3, 1 },
    { 10190, 29395, 3, 0 }, { 12140, 29395, 3, 1 },
	{ 14090, 29395, 3, 0 }, { 16040, 29395, 3, 1 },
	{ 17990, 29395, 3, 0 }, { 19940, 29395, 3, 1 },
	{ 21890, 29395, 3, 0 }, { 23840, 29395, 3, 1 },
	{ 25790, 29395, 3, 0 }, { 27740, 29395, 3, 1 },

	// Inner wall
	{ 7265, 28420, 0, 0 }, { 7265, 26470, 0, 1 },
	{ 7265, 24520, 0, 0 }, { 7265, 22570, 0, 1 },
	{ 7265, 20620, 0, 0 }, { 7265, 18670, 0, 1 },
	{ 7265, 16720, 0, 0 }, { 7265, 14770, 0, 1 },
	{ 7265, 12820, 0, 0 }, { 7265, 10870, 0, 1 },
	{ 7265,  8920, 0, 0 }, { 7265,  6970, 0, 1 },

	// Connector bridging the X=7265 wall and the X=5300 inner wall
	{ 6300, 6000, 3, 0 },

	// Inner
	{ 19700,  6962, 0, 0 }, { 19700,  8912, 0, 1 },
	{ 19700, 10862, 0, 0 }, { 19700, 12812, 0, 1 },
	{ 19700, 14762, 0, 0 }, { 19700, 16712, 0, 1 },
	{ 19700, 18662, 0, 0 }, { 19700, 20612, 0, 1 },
	{ 19700, 22562, 0, 0 }, { 19700, 24512, 0, 1 },
	{ 19700, 26462, 0, 0 }, { 19700, 28412, 0, 1 },

	// Parallel row
	{ 19695,  6962, 1, 0 }, { 19695,  8912, 1, 1 },
	{ 19695, 10862, 1, 0 }, { 19695, 12812, 1, 1 },
	{ 19695, 14762, 1, 0 }, { 19695, 16712, 1, 1 },
	{ 19695, 18662, 1, 0 }, { 19695, 20612, 1, 1 },
	{ 19695, 22562, 1, 0 }, { 19695, 24512, 1, 1 },
	{ 19695, 26462, 1, 0 }, { 19695, 28412, 1, 1 },

	// Inner
	{ 13750,  1218, 0, 0 }, { 13750,  3168, 0, 1 },
	{ 13750,  5118, 0, 0 }, { 13750,  7068, 0, 1 },
	{ 13750,  9018, 0, 0 }, { 13750, 10968, 0, 1 },
	{ 13750, 12918, 0, 0 }, { 13750, 14868, 0, 1 },
	{ 13750, 16818, 0, 0 }, { 13750, 18768, 0, 1 },
	{ 13750, 20718, 0, 0 }, { 13750, 22668, 0, 1 },

	// Parallel row
	{ 13745,  1218, 1, 0 }, { 13745,  3168, 1, 1 },
	{ 13745,  5118, 1, 0 }, { 13745,  7068, 1, 1 },
	{ 13745,  9018, 1, 0 }, { 13745, 10968, 1, 1 },
	{ 13745, 12918, 1, 0 }, { 13745, 14868, 1, 1 },
	{ 13745, 16818, 1, 0 }, { 13745, 18768, 1, 1 },
	{ 13745, 20718, 1, 0 }, { 13745, 22668, 1, 1 }

};
#define NUM_INNER_BARRIERS_MAP1 ((int)(sizeof(innerBarrierDefsMap1) / sizeof(innerBarrierDefsMap1[0])))

// Inner barriers for map_2 — none currently defined
#define NUM_INNER_BARRIERS_MAP2 0

#define MAX_INNER_BARRIERS 100

static ModelStruct innerBarrierModels[MAX_INNER_BARRIERS];
static const BarrierDef *activeInnerDefs = 0;
static int numActiveInnerBarriers = 0;

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


void InitialiseWorld() {

	// Load the ground textures into VRAM
	InitialiseGroundTextures();

	// Initialise the world ground data
	InitialiseGround();

	// Load the world textures into VRAM
	InitialiseWorldTextures();

	// Initialise the models in the world
	InitialiseWorldModels();

	// Initialise the sky — pass SKY_TEX_MEM_ADDR or SKY_ALT_TEX_MEM_ADDR
	InitialiseSky(SKY_ALT_TEX_MEM_ADDR);
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

	// Initialise all buildings from the data table
	{
		int i;
		for (i = 0; i < NUM_BUILDINGS; i++) 
		{
			const BuildingDef *b = &buildingDefs[i];
			long addr = (b->model == 0) ? (long)BUILDING_1_MEM_ADDR : (long)BUILDING_2_MEM_ADDR;
			InitialiseModel(&buildingModels[i], b->x, b->y, b->z, 0, 0, 0, (long*)addr);
			
			if (b->rot == 1) 
			{
				RotateModel180(&buildingModels[i].gsObjectCoord, &buildingModels[i].rotation);
			}

			buildingModels[i].collisionRadius = b->collisionRadius;
		}
	}

	// Stand
	InitialiseModel(&stand1, -1700, -450, 15000, 0, 5000, 5000, (long*)STAND_MEM_ADDR);

	InitialiseModel(&stand2, 7500, -450, 15000, 0, 5000, 5000, (long*)STAND_MEM_ADDR);
	RotateModel180(&stand2.gsObjectCoord, &stand2.rotation);

	// Initialise outer (perimeter) barriers — always loaded for all maps
	{
		int i;
		for (i = 0; i < NUM_OUTER_BARRIERS; i++)
		{
			const BarrierDef *b = &outerBarrierDefs[i];
			long addr = (b->model == 0) ? (long)BARRIER_1_MEM_ADDR : (long)BARRIER_2_MEM_ADDR;
			InitialiseModel(&outerBarrierModels[i], b->x, -200, b->z, 0, 0, 0, (long*)addr);

			if (b->rot == 1)
			{
				RotateModel180(&outerBarrierModels[i].gsObjectCoord, &outerBarrierModels[i].rotation);
			}
			else if (b->rot == 2)
			{
				RotateModel270(&outerBarrierModels[i].gsObjectCoord, &outerBarrierModels[i].rotation);
			}
			else if (b->rot == 3)
			{
				RotateModel90 (&outerBarrierModels[i].gsObjectCoord, &outerBarrierModels[i].rotation);
			}
		}
	}

	// Select and initialise inner barriers for the active map
	if (selectedTrackIndex == 1)
	{
		activeInnerDefs = 0;
		numActiveInnerBarriers = NUM_INNER_BARRIERS_MAP2;
	}
	else
	{
		activeInnerDefs = innerBarrierDefsMap1;
		numActiveInnerBarriers = NUM_INNER_BARRIERS_MAP1;
	}

	{
		int i;
		for (i = 0; i < numActiveInnerBarriers; i++)
		{
			const BarrierDef *b = &activeInnerDefs[i];
			long addr = (b->model == 0) ? (long)BARRIER_1_MEM_ADDR : (long)BARRIER_2_MEM_ADDR;
			InitialiseModel(&innerBarrierModels[i], b->x, -200, b->z, 0, 0, 0, (long*)addr);

			if (b->rot == 1)
			{
				RotateModel180(&innerBarrierModels[i].gsObjectCoord, &innerBarrierModels[i].rotation);
			}
			else if (b->rot == 2)
			{
				RotateModel270(&innerBarrierModels[i].gsObjectCoord, &innerBarrierModels[i].rotation);
			}
			else if (b->rot == 3)
			{
				RotateModel90 (&innerBarrierModels[i].gsObjectCoord, &innerBarrierModels[i].rotation);
			}
		}
	}

	// Collision radius for stands (buildings set their radius in the init loop above)
	stand1.collisionRadius = 350;
	stand2.collisionRadius = 350;
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


void CheckWorldCollisions(PlayerStruct *player, long *lateralSpeed) {
	// Sphere collision for buildings and crowd stands
	{
		int i;
		for (i = 0; i < NUM_BUILDINGS; i++)
		{
			TestSphereCollision(player, lateralSpeed, &buildingModels[i]);
		}
	}
	
	TestSphereCollision(player, lateralSpeed, &stand1);
	TestSphereCollision(player, lateralSpeed, &stand2);

	// Corner spheres (resolve the junction between perpendicular outer walls)
	// before both capsules can fire simultaneously and trap the vehicle
	TestCornerCollision(player, lateralSpeed,   200,   200, 400);
	TestCornerCollision(player, lateralSpeed, 35300,   200, 400);
	TestCornerCollision(player, lateralSpeed,   200, 35300, 400);
	TestCornerCollision(player, lateralSpeed, 35300, 35300, 400);

	// Capsule collision for outer (perimeter) barriers
	{
		int i;
		for (i = 0; i < NUM_OUTER_BARRIERS; i++)
		{
			const BarrierDef *b = &outerBarrierDefs[i];
			TestCapsuleCollision(player, lateralSpeed, b->x, b->z, (b->rot == 2 || b->rot == 3) ? 0 : 1, 975, 100);
		}
	}

	// Capsule collision for inner barriers of the active map
	{
		int i;
		for (i = 0; i < numActiveInnerBarriers; i++)
		{
			const BarrierDef *b = &activeInnerDefs[i];
			TestCapsuleCollision(player, lateralSpeed, b->x, b->z, (b->rot == 2 || b->rot == 3) ? 0 : 1, 975, 100);
		}
	}
}


// The buildings are seen from further away than the rest of the world, so they have a larger view distance for culling
#define BUILDING_VIEW_AHEAD  18000
#define BUILDING_VIEW_BEHIND 4000
#define BUILDING_VIEW_SIDE   12800

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
	DrawModel(model, &WorldOrderingTable[currentBuffer]);
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

	// Draw all buildings from the data table (extended draw distance)
	{
		int i;
		for (i = 0; i < NUM_BUILDINGS; i++)
		{
			DrawBuildingCulled(currentPlayer, &buildingModels[i], currentBuffer);
		}
	}

	// Draw the stands
	DrawModelCulled(currentPlayer, &stand1, buildingDivision, currentBuffer);
	DrawModelCulled(currentPlayer, &stand2, buildingDivision, currentBuffer);

	// Draw outer (perimeter) barriers
	{
		int i;
		for (i = 0; i < NUM_OUTER_BARRIERS; i++)
		{
			DrawModelCulled(currentPlayer, &outerBarrierModels[i], barrierDivision, currentBuffer);
		}
	}

	// Draw inner barriers for the active map
	{
		int i;
		for (i = 0; i < numActiveInnerBarriers; i++)
		{
			DrawModelCulled(currentPlayer, &innerBarrierModels[i], barrierDivision, currentBuffer);
		}
	}

	//DrawModelCulled(currentPlayer, &tunnel, currentBuffer);
}


void DrawWorld(PlayerStruct *currentPlayer, int currentBuffer) {

	// Draw the ground
	DrawGround(currentPlayer, &WorldOrderingTable[currentBuffer]);

	// Draw the world models
	DrawWorldModels(currentPlayer, currentBuffer);

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
		GsSetProjection(200);
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