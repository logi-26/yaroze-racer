#include <math.h>
#include <libps.h>
#include "world.h"
#include "player.h"
#include "graphics.h"
#include "ground.h"

// Global variables
GsOT OTable_Header[4];
PACKET Packet_Memory[4][MAX_PACKETS];
GsRVIEW2 Camera[2];
u_long vsyncInterval = 0;
GroundStruct theGround;
int NumberOfPlayers = 1;

// Model declarations
ModelStruct building1;
ModelStruct building2;
ModelStruct building3;

ModelStruct stand1;

ModelStruct barrier1;
ModelStruct barrier2;
ModelStruct barrier3;
ModelStruct barrier4;
ModelStruct barrier5;
ModelStruct barrier6;

// Load up from conventional memeory into video memory
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
	//LoadTexture(CAR_TEX_MEM_ADDR);
	LoadTexture(BARRIER_2_TEX_MEM_ADDR);
	LoadTexture(BUILDING_1_TEX_MEM_ADDR);
	LoadTexture(BUILDING_2_TEX_MEM_ADDR);
	LoadTexture(STAND_CROWD_TEX_MEM_ADDR);
}

void InitialiseWorldModels() {
	
	// Initialise the buildings
	InitialiseModel(&building1, -7500, -450, 8000, 0, 0, 0, (long*)BUILDING_1_MEM_ADDR);
	InitialiseModel(&building2, -6000, -300, 24000, 0, 0, 0, (long*)BUILDING_2_MEM_ADDR);
	InitialiseModel(&building3, -7500, -450, 30000, 0, 0, 0, (long*)BUILDING_1_MEM_ADDR);
	
	//InitialiseModel(&stand1, 0, -200, 1200, 0, 0, 0, (long*)STAND_CROWD_TEX_MEM_ADDR);

	//InitialiseModel(&theBuilding, 1200, -200, 3700, 0, 0, 0, (long*)BARRIER_1_MEM_ADDR);
	InitialiseModel(&barrier1, 1200, -200, 6210, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier2, 1200, -200, 10850, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier3, 1200, -200, 15490, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier4, 1200, -200, 20130, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier5, 1200, -200, 24770, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier6, 1200, -200, 29470, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
}

void AddModelToWorld() {
	
}

void DrawWorldModels(PlayerStruct *currentPlayer, int currentBuffer) {
	
	// Draw the buildings
	DrawModelCulled(currentPlayer, &building1, currentBuffer);
	DrawModelCulled(currentPlayer, &building2, currentBuffer);
	DrawModelCulled(currentPlayer, &building3, currentBuffer);
	
	// Draw the barriers
	DrawModelCulled(currentPlayer, &barrier1, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier2, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier3, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier4, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier5, currentBuffer);
	DrawModelCulled(currentPlayer, &barrier6, currentBuffer);
	
	//DrawModel(&stand1, &OTable_Header[currentBuffer]);
}

void DrawWorld(PlayerStruct *currentPlayer, int currentBuffer) {
	
	// Draw the ground
	DrawGround(&theGround, currentPlayer, &OTable_Header[currentBuffer]);
	
	// Draw the world models
	DrawWorldModels(currentPlayer, currentBuffer);
}

// This function renders the world for player 1
void RenderWorldPlayer1(int currentBuffer) {
	
	// Draw the world
	DrawWorld(&player1, currentBuffer);
	
	// Draw player 1
	DrawPlayer(&player1, &OTable_Header[currentBuffer]);
	
	// If it is 2 player mode, draw the second player vehicle
	if (NumberOfPlayers == 2) {
		DrawPlayer(&player2, &OTable_Header[currentBuffer]);
	}
}

// This function renders the world for player 2
void RenderWorldPlayer2(int currentBuffer) {
	
	// Activates the view for the bottom screen
	GsSetRefView2(&Camera[1]);    

	// Set the address of the packet area for the bottom screen
	GsSetWorkBase((PACKET*)Packet_Memory[currentBuffer+2]);
	
	// Clear the ordering table for the bottom screen
	GsClearOt(0, 0, &OTable_Header[currentBuffer+2]); 
	
	// Draw the world
	DrawWorld(&player2, currentBuffer+2);
	
	// Draw player 1
	DrawPlayer(&player1, &OTable_Header[currentBuffer+2]);
	
	// Draw player 2
	DrawPlayer(&player2, &OTable_Header[currentBuffer+2]);
}

// This function performs all of the rendering
void RenderWorld() {
	
	// This variable keeps track of the current buffer for double buffering
	int currentBuffer;

	// Get the current buffer
	currentBuffer = GsGetActiveBuff();
	
	// Activates the view for the top screen
	GsSetRefView2(&Camera[0]);                          
	
	// Set the address of the packet area for the top screen
	GsSetWorkBase((PACKET*)Packet_Memory[currentBuffer]);
	
	// Clear the ordering table for the top screen
	GsClearOt(0, 0, &OTable_Header[currentBuffer]);
	
	// Draw the world for player 1
	RenderWorldPlayer1(currentBuffer);
	
	// Draw the world for player 2
	if (NumberOfPlayers == 2) {
		RenderWorldPlayer2(currentBuffer);
	}
	
	// Wait for end of drawing
	DrawSync(0);
	
	// Wait for V_BLANK interrupt
	vsyncInterval = VSync(0);
	
	// Print the vSync interval
	//FntPrint(fontID_1, "\nVSync Interval: %d.\n", vsyncInterval);
	
	// Swap The Buffers
	GsSwapDispBuff();
	
	// Register clear-command: clear to black
	GsSortClear(0, 0, 0, &OTable_Header[currentBuffer]);
	
	// Set up top half for drawing
	PutDrawEnv(&SplitScreenInfo[currentBuffer]);        
	
	// Draw the commands in queue
	GsDrawOt(&OTable_Header[currentBuffer]);
	
	FntFlush(fontID_1);
	//FntFlush(fontID_2);
	
	// If 2 player mode
	if (NumberOfPlayers == 2) {
		// Set up bottom half for drawing
		PutDrawEnv(&SplitScreenInfo[currentBuffer+2]);      
	  
		// Draw the commands in queue
		GsDrawOt(&OTable_Header[currentBuffer+2]);   
	}
}

int CalculateDistanceSquared(int x1, int y1, int z1, int x2, int y2, int z2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int dz = z2 - z1;
    return (dx*dx + dy*dy + dz*dz);
}