#include <math.h>
#include <libps.h>
#include "world.h"
#include "game/player.h"
#include "../engine/graphics.h"
#include "../engine/calculations.h"
#include "ground.h"

// Global variables
GsRVIEW2 Camera[2];
u_long vsyncInterval = 0;
GroundStruct theGround;
int NumberOfPlayers = 1;


/*****************************************************
Model declarations
*****************************************************/

// Buildings
ModelStruct building1;
ModelStruct building2;
ModelStruct building3;
ModelStruct building4;
ModelStruct building5;
ModelStruct building6;

// Crowd stand
ModelStruct stand1;
ModelStruct stand2;

// Track signs
ModelStruct sign0;
ModelStruct sign1;
ModelStruct sign2;
ModelStruct sign3;
ModelStruct sign4;
ModelStruct sign5;
ModelStruct sign6;
ModelStruct sign7;
ModelStruct sign8;

// Track barriers
ModelStruct barrier00;
ModelStruct barrier0;
ModelStruct barrier1;
ModelStruct barrier2;
ModelStruct barrier3;
ModelStruct barrier4;
ModelStruct barrier5;
ModelStruct barrier6;

ModelStruct barrier7;
ModelStruct barrier8;
ModelStruct barrier9;
ModelStruct barrier10;
ModelStruct barrier11;

ModelStruct barrier12;
ModelStruct barrier13;
ModelStruct barrier14;
ModelStruct barrier15;
ModelStruct barrier16;

ModelStruct barrier17;
ModelStruct barrier18;
ModelStruct barrier19;
ModelStruct barrier20;
ModelStruct barrier21;

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
	if ((tim1.pmode>>3)&0x01) {
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
	LoadTexture(CAR_3_TEX_MEM_ADDR);
}


void InitialiseWorldModels() {

	// Initialise the buildings
	InitialiseModel(&building1, -5500, -450, 6000, 0, 0, 0, (long*)BUILDING_1_MEM_ADDR);
	InitialiseModel(&building2, -4000, -300, 25000, 0, 0, 0, (long*)BUILDING_2_MEM_ADDR);
	InitialiseModel(&building3, -5500, -450, 30000, 0, 0, 0, (long*)BUILDING_1_MEM_ADDR);

	// Stand
	InitialiseModel(&stand1, -1000, -450, 15000, 0, 5000, 5000, (long*)STAND_MEM_ADDR);

	InitialiseModel(&stand2, 7000, -450, 15000, 0, 5000, 5000, (long*)STAND_MEM_ADDR);
	RotateModel180(&stand2.gsObjectCoord, &stand2.rotation);

	// Signs
	InitialiseModel(&sign0, 4140, -200,  1380, 0, 0, 0, (long*)SIGN_1_MEM_ADDR);
	RotateModel270(&sign0.gsObjectCoord, &sign0.rotation);

	InitialiseModel(&sign1, 3560, -200,  1380, 0, 0, 0, (long*)SIGN_1_MEM_ADDR);
	RotateModel270(&sign1.gsObjectCoord, &sign1.rotation);

	InitialiseModel(&sign2, 3000, -200,  1430, 0, 0, 0, (long*)SIGN_1_MEM_ADDR);
	RotModel(&sign2.gsObjectCoord, &sign2.rotation, 0, -900, 0);

	InitialiseModel(&sign3, 2490, -200, 1610, 0, 0, 0, (long*)SIGN_1_MEM_ADDR);
	RotModel(&sign3.gsObjectCoord, &sign3.rotation, 0, -750, 0);

	InitialiseModel(&sign4, 2020, -200, 1910, 0, 0, 0, (long*)SIGN_1_MEM_ADDR);
	RotModel(&sign4.gsObjectCoord, &sign4.rotation, 0, -600, 0);

	InitialiseModel(&sign5, 1650, -200, 2300, 0, 0, 0, (long*)SIGN_1_MEM_ADDR);
	RotModel(&sign5.gsObjectCoord, &sign5.rotation, 0, -450, 0);

	InitialiseModel(&sign6, 1390, -200, 2740, 0, 0, 0, (long*)SIGN_1_MEM_ADDR);
	RotModel(&sign6.gsObjectCoord, &sign6.rotation, 0, -300, 0);

	InitialiseModel(&sign7, 1250, -200, 3230, 0, 0, 0, (long*)SIGN_1_MEM_ADDR);
	RotModel(&sign7.gsObjectCoord, &sign7.rotation, 0, -150, 0);

	InitialiseModel(&sign8, 1200, -200, 3800, 0, 0, 0, (long*)SIGN_1_MEM_ADDR);

	// Initialise the barriers (gap between barriers = 1950)
	InitialiseModel(&barrier00, 7360, -200, 1380, 0, 0, 0, (long*)BARRIER_1_MEM_ADDR);
	RotateModel270(&barrier00.gsObjectCoord, &barrier00.rotation);

	InitialiseModel(&barrier0, 5410, -200, 1380, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	RotateModel270(&barrier0.gsObjectCoord, &barrier0.rotation);

	InitialiseModel(&barrier1, 1200, -200, 5000, 0, 0, 0, (long*)BARRIER_1_MEM_ADDR);
	InitialiseModel(&barrier2, 1200, -200, 6950, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier3, 1200, -200, 8900, 0, 0, 0, (long*)BARRIER_1_MEM_ADDR);
	InitialiseModel(&barrier4, 1200, -200, 10850, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier5, 1200, -200, 12800, 0, 0, 0, (long*)BARRIER_1_MEM_ADDR);
	InitialiseModel(&barrier6, 1200, -200, 14750, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier7, 1200, -200, 16700, 0, 0, 0, (long*)BARRIER_1_MEM_ADDR);
	InitialiseModel(&barrier8, 1200, -200, 18650, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier9, 1200, -200, 20600, 0, 0, 0, (long*)BARRIER_1_MEM_ADDR);
	InitialiseModel(&barrier10, 1200, -200, 22550, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier11, 1200, -200, 24500, 0, 0, 0, (long*)BARRIER_1_MEM_ADDR);
	InitialiseModel(&barrier12, 1200, -200, 26450, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier13, 1200, -200, 28400, 0, 0, 0, (long*)BARRIER_1_MEM_ADDR);

	InitialiseModel(&barrier17, 4800, -200, 7000, 0, 0, 0, (long*)BARRIER_1_MEM_ADDR);
	RotateModel180(&barrier17.gsObjectCoord, &barrier17.rotation);

	InitialiseModel(&barrier15, 4800, -200, 8950, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	RotateModel180(&barrier15.gsObjectCoord, &barrier15.rotation);

	InitialiseModel(&barrier16, 4800, -200, 10900, 0, 0, 0, (long*)BARRIER_1_MEM_ADDR);
	RotateModel180(&barrier16.gsObjectCoord, &barrier16.rotation);

	InitialiseModel(&barrier17, 4800, -200, 12850, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	RotateModel180(&barrier17.gsObjectCoord, &barrier17.rotation);

	InitialiseModel(&barrier18, 4800, -200, 14800, 0, 0, 0, (long*)BARRIER_1_MEM_ADDR);
	RotateModel180(&barrier18.gsObjectCoord, &barrier18.rotation);

	InitialiseModel(&barrier19, 4800, -200, 16750, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	RotateModel180(&barrier19.gsObjectCoord, &barrier19.rotation);

	InitialiseModel(&barrier20, 4800, -200, 18700, 0, 0, 0, (long*)BARRIER_1_MEM_ADDR);
	RotateModel180(&barrier20.gsObjectCoord, &barrier20.rotation);

	InitialiseModel(&barrier21, 4800, -200, 20650, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	RotateModel180(&barrier21.gsObjectCoord, &barrier21.rotation);

	//InitialiseModel(&tunnel, 18500, -200, 29000, 0, 0, 0, (long*)TUNNEL_MEM_ADDR);

	// Sphere collision radii for rounded structures.
	// Barriers use capsule collision and do not need a radius override.
	building1.collisionRadius = 400;
	building2.collisionRadius = 400;
	building3.collisionRadius = 400;
	stand1.collisionRadius    = 350;
	stand2.collisionRadius    = 350;
}


void AddModelToWorld() {

}


// Approximate 2D distance without sqrt using octagonal approximation
static int OctagonalDistance(int dx, int dz) {
	int absDx = (dx < 0) ? -dx : dx;
	int absDz = (dz < 0) ? -dz : dz;
	int maxD  = (absDx > absDz) ? absDx : absDz;
	int minD  = (absDx > absDz) ? absDz : absDx;
	return (maxD * 123 + minD * 51) >> 7;
}


// Shared response: push player to the combined-sphere surface, bounce speed, clear drift
static void ApplyCollisionResponse(PlayerStruct *player, long *lateralSpeed,
	long closestX, long closestZ, int combinedRadius) {
	int dx = (int)(player->gsObjectCoord.coord.t[0] - closestX);
	int dz = (int)(player->gsObjectCoord.coord.t[2] - closestZ);
	int approxDist = OctagonalDistance(dx, dz);
	
	if (approxDist < 1) {
		player->gsObjectCoord.coord.t[0] = closestX + combinedRadius;
	} else {
		player->gsObjectCoord.coord.t[0] = closestX + ((long)dx * combinedRadius) / approxDist;
		player->gsObjectCoord.coord.t[2] = closestZ + ((long)dz * combinedRadius) / approxDist;
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

	if (dx > combinedRadius || dx < -combinedRadius) return;
	if (dz > combinedRadius || dz < -combinedRadius) return;
	distSq = (long)dx * dx + (long)dz * dz;
	if (distSq >= (long)combinedRadius * combinedRadius) return;

	ApplyCollisionResponse(player, lateralSpeed,
		model->gsObjectCoord.coord.t[0],
		model->gsObjectCoord.coord.t[2],
		combinedRadius);
}


// Capsule collision for elongated barriers
// Models the barrier as a line segment (its length) with a small cross-section radius
// axisIsZ=1: segment runs along Z; axisIsZ=0: segment runs along X
// halfLen: half the barrier length. capsuleRadius: half the barrier's thickness
static void TestCapsuleCollision(PlayerStruct *player, long *lateralSpeed,
	int cx, int cz, int axisIsZ, int halfLen, int capsuleRadius) {
	int playerX, playerZ, combinedRadius, relAxis;
	int closestX, closestZ, dx, dz;
	long distSq;

	playerX = (int)player->gsObjectCoord.coord.t[0];
	playerZ = (int)player->gsObjectCoord.coord.t[2];
	combinedRadius = player->collisionRadius + capsuleRadius;

	// Closest point on the capsule centre-line to the player
	if (axisIsZ) {
		closestX = cx;
		relAxis  = playerZ - cz;
		if (relAxis >  halfLen) relAxis =  halfLen;
		if (relAxis < -halfLen) relAxis = -halfLen;
		closestZ = cz + relAxis;
	} else {
		closestZ = cz;
		relAxis  = playerX - cx;
		if (relAxis >  halfLen) relAxis =  halfLen;
		if (relAxis < -halfLen) relAxis = -halfLen;
		closestX = cx + relAxis;
	}

	dx = playerX - closestX;
	dz = playerZ - closestZ;

	if (dx > combinedRadius || dx < -combinedRadius) return;
	if (dz > combinedRadius || dz < -combinedRadius) return;
	distSq = (long)dx * dx + (long)dz * dz;
	if (distSq >= (long)combinedRadius * combinedRadius) return;

	ApplyCollisionResponse(player, lateralSpeed, closestX, closestZ, combinedRadius);
}


void CheckWorldCollisions(PlayerStruct *player, long *lateralSpeed) {
	// Sphere collision for buildings and crowd stands
	TestSphereCollision(player, lateralSpeed, &building1);
	TestSphereCollision(player, lateralSpeed, &building2);
	TestSphereCollision(player, lateralSpeed, &building3);
	TestSphereCollision(player, lateralSpeed, &stand1);
	TestSphereCollision(player, lateralSpeed, &stand2);

	// Capsule collision for barriers
	// halfLen=975: each barrier is 1950 units long. capsuleRadius=100: thin panel
	// player(300) + capsule(100) = 400 combined -- triggers 400 units from barrier surface
	// Adjacent capsule end-caps meet exactly (spacing 1950 = 2 x halfLen)

	// X-aligned barriers at Z=1380 (start straight)
	TestCapsuleCollision(player, lateralSpeed, 7360, 1380, 0, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 5410, 1380, 0, 975, 100);

	// Z-aligned left wall at X=1200
	TestCapsuleCollision(player, lateralSpeed, 1200,  5000, 1, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 1200,  6950, 1, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 1200,  8900, 1, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 1200, 10850, 1, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 1200, 12800, 1, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 1200, 14750, 1, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 1200, 16700, 1, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 1200, 18650, 1, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 1200, 20600, 1, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 1200, 22550, 1, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 1200, 24500, 1, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 1200, 26450, 1, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 1200, 28400, 1, 975, 100);

	// Z-aligned right wall at X=4800
	TestCapsuleCollision(player, lateralSpeed, 4800,  8950, 1, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 4800, 10900, 1, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 4800, 12850, 1, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 4800, 14800, 1, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 4800, 16750, 1, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 4800, 18700, 1, 975, 100);
	TestCapsuleCollision(player, lateralSpeed, 4800, 20650, 1, 975, 100);
}


static void DrawModelCulled(PlayerStruct *currentPlayer, ModelStruct *model, int currentBuffer) {
	if (model && IsObjectNearPlayer(currentPlayer, &model->gsObjectCoord)) {
		DrawModel(model, &WorldOrderingTable[currentBuffer]);
	}
}


void DrawWorldModels(PlayerStruct *currentPlayer, int currentBuffer) {

	// Draw the buildings
	DrawModelCulled(currentPlayer, &building1, currentBuffer);
	DrawModelCulled(currentPlayer, &building2, currentBuffer);
	DrawModelCulled(currentPlayer, &building3, currentBuffer);

	// Draw the stands
	DrawModelCulled(currentPlayer, &stand1, currentBuffer);
	DrawModelCulled(currentPlayer, &stand2, currentBuffer);

	// Draw the signs
	DrawModelCulled(currentPlayer, &sign0, currentBuffer);
	DrawModelCulled(currentPlayer, &sign1, currentBuffer);
	DrawModelCulled(currentPlayer, &sign2, currentBuffer);
	DrawModelCulled(currentPlayer, &sign3, currentBuffer);
	DrawModelCulled(currentPlayer, &sign4, currentBuffer);
	DrawModelCulled(currentPlayer, &sign5, currentBuffer);
	DrawModelCulled(currentPlayer, &sign6, currentBuffer);
	DrawModelCulled(currentPlayer, &sign7, currentBuffer);
	DrawModelCulled(currentPlayer, &sign8, currentBuffer);

	// Draw the barriers
	DrawModelCulled(currentPlayer, &barrier00, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier0, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier1, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier2, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier3, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier4, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier5, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier6, currentBuffer);

	DrawModelCulled(currentPlayer, &barrier7, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier8, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier9, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier10, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier11, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier12, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier13, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier14, currentBuffer);

	DrawModelCulled(currentPlayer, &barrier15, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier16, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier17, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier18, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier19, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier20, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier21, currentBuffer);

	//DrawModelCulled(currentPlayer, &tunnel, currentBuffer);
}


void DrawWorld(PlayerStruct *currentPlayer, int currentBuffer) {

	// Draw the ground
	DrawGround(&theGround, currentPlayer, &WorldOrderingTable[currentBuffer]);

	// Draw the world models
	DrawWorldModels(currentPlayer, currentBuffer);
}


// This function renders the world for player 1
void RenderWorldPlayer1(int currentBuffer) {

	// Draw the world
	DrawWorld(&player1, currentBuffer);

	// Draw player 1
	DrawPlayer(&player1, &WorldOrderingTable[currentBuffer]);

	// If it is 2 player mode, draw the second player vehicle
	if (NumberOfPlayers == 2) {
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

	if (yawDelta >  2048) yawDelta -= 4096;
	if (yawDelta < -2048) yawDelta += 4096;

	camSwayX -= yawDelta * 2;
	camSwayX  = camSwayX * 15 / 16;
	if (camSwayX >  200) camSwayX =  200;
	if (camSwayX < -200) camSwayX = -200;

	Camera[0].vpx = (int)camSwayX;
}


// This function performs all of the rendering
void RenderWorld() {
	int currentBuffer = GsGetActiveBuff();
	
	// Update the camera sway
	UpdateCameraLag();
	
	// Set the camera view
	GsSetRefView2(&Camera[0]);
	
	// Render the world for player 1
	RenderWorldPlayer1(currentBuffer);
	
	// If two player mode, render the world for player 2
	if (NumberOfPlayers == 2) {
		RenderWorldPlayer2(currentBuffer);
	}
}