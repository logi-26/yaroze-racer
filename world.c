#include <math.h>
#include <libps.h>
#include "world.h"
#include "player.h"
#include "graphics.h"

// Global variables
GsOT OTable_Header[4];
PACKET Packet_Memory[4][MAX_PACKETS];
GsRVIEW2 viewTop;

GsRVIEW2 Camera[2];


u_long vsyncInterval = 0;

//PlayerStruct player1;
//PlayerStruct player2;
WorldStruct theWorld;


int NumberOfPlayers = 2;



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

char worldGroundData[WORLD_GROUND_MAX_Z][WORLD_GROUND_MAX_X] = {
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3'},
	{'3','3','d','i','1','1','1','h','h','i','4','i','1','1','1','1','1','1','1','1','1','1','1','1','1','1','i','8','3','3'},
	{'3','3','l','f','2','2','2','g','g','k','5','k','2','2','2','2','2','2','2','2','2','2','2','2','2','2','9','j','3','3'},
	{'3','3','7','6','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3'},
	{'3','3','7','6','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3'},
	{'3','3','7','6','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3'},
	{'3','3','7','6','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3'},
	{'3','3','l','e','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','i','8','3','3','7','6','3','3'},
	{'3','3','c','k','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','9','j','3','3','7','6','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3','7','6','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3','7','6','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3','7','6','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3','7','6','3','3'},
	{'3','3','d','i','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','b','j','3','3','7','6','3','3'},
	{'3','3','l','f','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','k','a','3','3','7','6','3','3'},
	{'3','3','7','6','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3'},
	{'3','3','7','6','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3'},
	{'3','3','7','6','3','3','3','3','3','3','3','3','3','3','3','3','3','3','d','i','i','i','8','3','3','3','7','6','3','3'},
	{'3','3','7','6','3','3','3','3','3','3','3','3','3','3','3','3','3','3','l','f','k','9','j','3','3','3','7','6','3','3'},
	{'3','3','7','6','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','7','6','3','3','3','7','6','3','3'},
	{'3','3','7','6','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','7','6','3','3','3','7','6','3','3'},
	{'3','3','l','e','1','1','1','1','1','1','1','1','1','1','1','1','1','1','b','j','3','7','6','3','3','3','7','6','3','3'},
	{'3','3','c','k','2','2','2','2','2','2','2','2','2','2','2','2','2','2','k','a','3','7','6','3','3','3','7','6','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3','3','7','6','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3','3','7','6','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','l','e','1','1','1','b','j','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','c','k','2','2','2','k','a','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3'},
};

// Load up from conventional memeory into video memory
int LoadTexture(long addr) {
	RECT rect;
	GsIMAGE tim1;

	// Get tim info/header, again a little bit of majic is needd the pointer
	// is incremented past the first 4 positions to get to this!
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
	int tmpx, tmpz;
	char c;
	// Initialise total number of models to zero
	theWorld.nTotalModels = 0;
	
	// Copy textures into vram
	LoadTexture(LINE_L_TEX_MEM_ADDR);					
	LoadTexture(LINE_R_TEX_MEM_ADDR);					
	LoadTexture(STRAIGHT_L_1_TEX_MEM_ADDR);			
	LoadTexture(STRAIGHT_L_2_TEX_MEM_ADDR);			
	LoadTexture(STRAIGHT_R_1_TEX_MEM_ADDR);			
	LoadTexture(STRAIGHT_R_2_TEX_MEM_ADDR);			
	LoadTexture(TURN_L_1_TEX_MEM_ADDR);				
	LoadTexture(TURN_L_2_TEX_MEM_ADDR);				
	LoadTexture(TURN_L_3_TEX_MEM_ADDR);				
	LoadTexture(TURN_L_4_TEX_MEM_ADDR);				
	LoadTexture(TURN_R_1_TEX_MEM_ADDR);				
	LoadTexture(TURN_R_2_TEX_MEM_ADDR);				
	LoadTexture(TURN_R_3_TEX_MEM_ADDR);				
	LoadTexture(TURN_R_4_TEX_MEM_ADDR);	
	LoadTexture(GRID_L_TEX_MEM_ADDR);				
	LoadTexture(GRID_R_TEX_MEM_ADDR);
	LoadTexture(STRAIGHT_L_01_TEX_MEM_ADDR);			
	LoadTexture(STRAIGHT_L_02_TEX_MEM_ADDR);			
	LoadTexture(STRAIGHT_R_01_TEX_MEM_ADDR);			
	LoadTexture(STRAIGHT_R_02_TEX_MEM_ADDR);	
	LoadTexture(GRASS_TEX_MEM_ADDR); 


	// Then for each element of the worldGroundData array if we find a 1
	// then place an instance of square1.tmd at the appropriate position in the world.
	for (tmpz = 0; tmpz < WORLD_GROUND_MAX_Z; tmpz++) {
		for (tmpx = 0; tmpx < WORLD_GROUND_MAX_X; tmpx++) {
			c = worldGroundData[tmpz][tmpx];

			if (c == '1') {
				AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_L_1_MEM_ADDR);
			}
			if (c == '2') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_R_1_MEM_ADDR);
			}
			if (c == '3') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)GRASS_MEM_ADDR);
			}
			if (c == '4') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)LINE_L_MEM_ADDR);
			}
			if (c == '5') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)LINE_R_MEM_ADDR);
			}
			if (c == '6') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_L_2_MEM_ADDR);
			}
			if (c == '7') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_R_2_MEM_ADDR);
			}
			if (c == '8') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_L_1_MEM_ADDR);
			}
			if (c == 'a') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_L_2_MEM_ADDR);
			}
			if (c == 'c') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_L_3_MEM_ADDR);
			}
			if (c == 'd') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_L_4_MEM_ADDR);
			}
			if (c == '9') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_R_1_MEM_ADDR);
			}
			if (c == 'b') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_R_2_MEM_ADDR);
			}
			if (c == 'e') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_R_3_MEM_ADDR);
			}
			if (c == 'f') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_R_4_MEM_ADDR);
			}
			if (c == 'g') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)GRID_L_MEM_ADDR);
			}
			if (c == 'h') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)GRID_R_MEM_ADDR);
			}
			if (c == 'i') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_L_01_MEM_ADDR);
			}
			if (c == 'j') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_L_02_MEM_ADDR);
			}
			if (c == 'k') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_R_01_MEM_ADDR);
			}
			if (c == 'l') {
                AddModelToWorld(&theWorld, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_R_02_MEM_ADDR);
			}

		} // End for tmpx
	} // End for tmpz
}


void InitialiseWorldTextures() {
	
	// Load textures into VRAM
	//LoadTexture(CAR_TEX_MEM_ADDR);
	//LoadTexture(BUILDING_TEX_MEM_ADDR);
	//LoadTexture(BUILDING_2_TEX_MEM_ADDR);
	LoadTexture(BARRIER_2_TEX_MEM_ADDR);
	
	LoadTexture(BUILDING_1_TEX_MEM_ADDR);
	LoadTexture(BUILDING_2_TEX_MEM_ADDR);
	
	LoadTexture(STAND_CROWD_TEX_MEM_ADDR);
}


void InitialiseWorldModels() {
	
	
	// Initialise the player
	//InitialisePlayer(&player1, 3605, -200, 9273, (long*)CAR_MEM_ADDR);
	
	//InitialisePlayer(&player2, 2448, -200, 9273, (long*)CAR_MEM_ADDR);
	
	
	
	// Initialise the buildings
	InitialiseModel(&building1, -7500, -450, 8000, 0, 0, 0, (long*)BUILDING_1_MEM_ADDR);
	InitialiseModel(&building2, -6000, -300, 24000, 0, 0, 0, (long*)BUILDING_2_MEM_ADDR);
	InitialiseModel(&building3, -7500, -450, 30000, 0, 0, 0, (long*)BUILDING_1_MEM_ADDR);
	
	//InitialiseModel(&stand1, 0, -200, 1200, 0, 0, 0, (long*)STAND_CROWD_TEX_MEM_ADDR);
	
	
	//InitialiseModel(&building2, 0, -50, 6000, 0, 0, 0, (long*)BUILDING_2_MEM_ADDR);
	//InitialiseModel(&building3, 0, -200, 8000, 0, 0, 0, (long*)BUILDING_MEM_ADDR);
	//InitialiseModel(&building4, 0, -200, 10000, 0, 0, 0, (long*)BUILDING_MEM_ADDR);
	//InitialiseModel(&building5, 0, -200, 12000, 0, 0, 0, (long*)BUILDING_MEM_ADDR);
	//InitialiseModel(&building6, 0, -50, 14000, 0, 0, 0, (long*)BUILDING_2_MEM_ADDR);
	//InitialiseModel(&building7, 0, -200, 16000, 0, 0, 0, (long*)BUILDING_MEM_ADDR);
	
	//4790

	//InitialiseModel(&theBuilding, 1200, -200, 3700, 0, 0, 0, (long*)BARRIER_1_MEM_ADDR);
	InitialiseModel(&barrier1, 1200, -200, 6210, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier2, 1200, -200, 10850, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier3, 1200, -200, 15490, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier4, 1200, -200, 20130, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier5, 1200, -200, 24770, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier6, 1200, -200, 29470, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
}

void AddModelToWorld(WorldStruct *theWorld, int nX, int nY, int nZ, unsigned long *lModelAddress) {
    theWorld->lObjectPointer[theWorld->nTotalModels] = (unsigned long*)lModelAddress;
	
	// Increment the pointer to move past the model id. (weird huh?)
	theWorld->lObjectPointer[theWorld->nTotalModels]++;
	
	// Map tmd data to its actual address
	GsMapModelingData(theWorld->lObjectPointer[theWorld->nTotalModels]);
	
	// Initialise the objects coordinate system - set to be that of the WORLD
	GsInitCoordinate2(WORLD, &theWorld->gsObjectCoord[theWorld->nTotalModels]);
	
	// Increment pointer twice more - to point to top of model data (beats me!)
    theWorld->lObjectPointer[theWorld->nTotalModels]++;
	theWorld->lObjectPointer[theWorld->nTotalModels]++;
	
	// Link the model (tmd) with the players object handler
	GsLinkObject4((unsigned long *)theWorld->lObjectPointer[theWorld->nTotalModels], &theWorld->gsObjectHandler[theWorld->nTotalModels], 0);
	
	// Set the amount of polygon subdivision that will be done at runtime (none!)
	theWorld->gsObjectHandler[theWorld->nTotalModels].attribute = GsDIV1;
	
	// Assign the coordinates of the object model to the Object Handler
	theWorld->gsObjectHandler[theWorld->nTotalModels].coord2 = &theWorld->gsObjectCoord[theWorld->nTotalModels];
    
	// Set The Position of the Object
    theWorld->gsObjectCoord[theWorld->nTotalModels].coord.t[0] = nX;   // X
    theWorld->gsObjectCoord[theWorld->nTotalModels].coord.t[1] = nY;   // Y
	theWorld->gsObjectCoord[theWorld->nTotalModels].coord.t[2] = nZ;   // Z
	
	// Increment the object counter
	theWorld->nTotalModels++;
    
	// Flag the object as needing to be drawn
	theWorld->gsObjectCoord[theWorld->nTotalModels].flg = 0;
}

void DrawWorld(WorldStruct *theWorld, PlayerStruct* currentPlayer, GsOT *OTable_Header) {
    MATRIX  tmpls, tmplw;
    int nCurrentModel;
    
    FntPrint(fontID_1, "Ground models: %d\n\n", (theWorld->nTotalModels));

    for (nCurrentModel = 0; nCurrentModel < theWorld->nTotalModels; nCurrentModel++) {
        // Only render if object is near the current player
        if (IsObjectNearPlayer(currentPlayer, &theWorld->gsObjectCoord[nCurrentModel])) {
            // Get the local world and screen coordinates
            GsGetLws(theWorld->gsObjectHandler[nCurrentModel].coord2, &tmplw, &tmpls);
            
            // Set the resulting matrices
            GsSetLightMatrix(&tmplw);
            GsSetLsMatrix(&tmpls);
            
            // Send Object To Ordering Table
            GsSortObject4(&theWorld->gsObjectHandler[nCurrentModel], OTable_Header, 2, (u_long *)getScratchAddr(0));
        }
    }
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
	FntPrint(fontID_1, "\nVSync Interval: %d.\n", vsyncInterval);
	
	// Swap The Buffers
	GsSwapDispBuff();
	
	// Register clear-command: clear to black
	GsSortClear(0, 0, 0, &OTable_Header[currentBuffer]);
	
	// Set up top half for drawing
	PutDrawEnv(&SplitScreenInfo[currentBuffer]);        
	
	// Draw the commands in queue
	GsDrawOt(&OTable_Header[currentBuffer]);
	
	FntFlush(fontID_1);
	FntFlush(fontID_2);
	
	// If 2 player mode
	if (NumberOfPlayers == 2) {
		// Set up bottom half for drawing
		PutDrawEnv(&SplitScreenInfo[currentBuffer+2]);      
	  
		// Draw the commands in queue
		GsDrawOt(&OTable_Header[currentBuffer+2]);   
	}
}

// This function renders the world for player 1
void RenderWorldPlayer1(int currentBuffer) {
	// Draw the world
	DrawWorld(&theWorld, &player1, &OTable_Header[currentBuffer]);
	
	// Draw player 1
	DrawPlayer(&player1, &OTable_Header[currentBuffer]);
	
	// If it is 2 player mode, draw the second player vehicle
	if (NumberOfPlayers == 2) {
		DrawPlayer(&player2, &OTable_Header[currentBuffer]);
	}
	
	// Draw the buildings
	RenderBuildings(&player1, currentBuffer);
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
	DrawWorld(&theWorld, &player2, &OTable_Header[currentBuffer+2]);
	
	// Draw player 1
	DrawPlayer(&player1, &OTable_Header[currentBuffer+2]);
	
	// Draw player 2
	DrawPlayer(&player2, &OTable_Header[currentBuffer+2]);
	
	// Draw the buildings
	RenderBuildings(&player2, currentBuffer+2);
}

// This functions renders all of the models in the world
void RenderBuildings(PlayerStruct* currentPlayer, int currentBuffer) {
	// Draw the buildings
	if (IsObjectNearPlayer(currentPlayer, &building1.gsObjectCoord)) {
		DrawModel(&building1, &OTable_Header[currentBuffer]);
	}
	if (IsObjectNearPlayer(currentPlayer, &building2.gsObjectCoord)) {
		DrawModel(&building2, &OTable_Header[currentBuffer]);
	}
	if (IsObjectNearPlayer(currentPlayer, &building2.gsObjectCoord)) {
		DrawModel(&building3, &OTable_Header[currentBuffer]);
	}

	// Draw the barriers
	if (IsObjectNearPlayer(currentPlayer, &barrier1.gsObjectCoord)) {
		DrawModel(&barrier1, &OTable_Header[currentBuffer]);
	}
	if (IsObjectNearPlayer(currentPlayer, &barrier2.gsObjectCoord)) {
		DrawModel(&barrier2, &OTable_Header[currentBuffer]);
	}
	if (IsObjectNearPlayer(currentPlayer, &barrier3.gsObjectCoord)) {
		DrawModel(&barrier3, &OTable_Header[currentBuffer]);
	}
	if (IsObjectNearPlayer(currentPlayer, &barrier4.gsObjectCoord)) {
		DrawModel(&barrier4, &OTable_Header[currentBuffer]);
	}
	if (IsObjectNearPlayer(currentPlayer, &barrier5.gsObjectCoord)) {
		DrawModel(&barrier5, &OTable_Header[currentBuffer]);
	}
	if (IsObjectNearPlayer(currentPlayer, &barrier6.gsObjectCoord)) {
		DrawModel(&barrier6, &OTable_Header[currentBuffer]);
	}

	//DrawModel(&stand1, &OTable_Header[currentBuffer]);
}

int CalculateDistanceSquared(int x1, int y1, int z1, int x2, int y2, int z2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int dz = z2 - z1;
    return (dx*dx + dy*dy + dz*dz);
}