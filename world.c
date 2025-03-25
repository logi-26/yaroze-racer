#include <libps.h>
#include "world.h"
#include "player.h"

// Global variables
GsOT othWorld[2];r
GsRVIEW2 viewTop;
u_long vsyncInterval = 0;


PlayerStruct theCar;
WorldStruct theWorld;

//ModelStruct building1;
//ModelStruct building2;
//ModelStruct building3;
//ModelStruct building4;
//ModelStruct building5;
//ModelStruct building6;
//ModelStruct building7;

ModelStruct barrier2;
ModelStruct barrier3;
ModelStruct barrier4;
ModelStruct barrier5;
ModelStruct barrier6;

// Local variables
PACKET out_packet[2][MAX_NO_PACKETS];

//1 - 9
//2 - b
//3 - e
//4 - f

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
}




void InitialiseWorldModels() {
	
	// Initialise the player
	InitialisePlayer(&theCar, 0, -200, 0, (long*)CAR_MEM_ADDR);
	
	// Initialise the buildings
	//InitialiseModel(&building1, 0, -200, 4000, 0, 0, 0, (long*)BUILDING_MEM_ADDR);
	//InitialiseModel(&building2, 0, -50, 6000, 0, 0, 0, (long*)BUILDING_2_MEM_ADDR);
	//InitialiseModel(&building3, 0, -200, 8000, 0, 0, 0, (long*)BUILDING_MEM_ADDR);
	//InitialiseModel(&building4, 0, -200, 10000, 0, 0, 0, (long*)BUILDING_MEM_ADDR);
	//InitialiseModel(&building5, 0, -200, 12000, 0, 0, 0, (long*)BUILDING_MEM_ADDR);
	//InitialiseModel(&building6, 0, -50, 14000, 0, 0, 0, (long*)BUILDING_2_MEM_ADDR);
	//InitialiseModel(&building7, 0, -200, 16000, 0, 0, 0, (long*)BUILDING_MEM_ADDR);

	
	//InitialiseModel(&theBuilding, 1200, -200, 3700, 0, 0, 0, (long*)BARRIER_1_MEM_ADDR);
	InitialiseModel(&barrier2, 1200, -200, 3750, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier3, 1200, -200, 8540, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier4, 1200, -200, 13330, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier5, 1200, -200, 18120, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
	InitialiseModel(&barrier6, 1200, -200, 22970, 0, 0, 0, (long*)BARRIER_2_MEM_ADDR);
}



//4790





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


void DrawWorld(WorldStruct *theWorld, GsOT *othWorld) {
	MATRIX  tmpls, tmplw;
	int nCurrentModel;
	char c;
	
	FntPrint(fontID_1, "total objects: %d\n\n", (theWorld->nTotalModels +5));

	for (nCurrentModel = 0; nCurrentModel < theWorld->nTotalModels; nCurrentModel++) {
		// Get the local world and screen coordinates, needed for light calculations
		GsGetLws(theWorld->gsObjectHandler[nCurrentModel].coord2, &tmplw, &tmpls);
		
		// Set the resulting light source matrix
		GsSetLightMatrix(&tmplw);
		
		// Set the local screen matrix for the GTE (so it works out perspective etc)
		GsSetLsMatrix(&tmpls);
		
		// Send Object To Ordering Table
		GsSortObject4(&theWorld->gsObjectHandler[nCurrentModel], othWorld, 2, (u_long *)getScratchAddr(0));
	}
}


// This function deals with double buffering and drawing of 3D objects
void RenderWorld() {
	// This variable keeps track of the current buffer for double buffering
	int currentBuffer;
	
	// Get the current buffer
	currentBuffer = GsGetActiveBuff();
	
	// Set the address of the packet area that will contain drawing commands
	GsSetWorkBase((PACKET*)out_packet[currentBuffer]);
	
	// Clear the ordering table
	GsClearOt(0, 0, &othWorld[currentBuffer]);
	
	// Set the viewport
	GsSetRefView2(&viewTop);
	
	// Draw the world
	DrawWorld(&theWorld, &othWorld[currentBuffer]);
	
	// Draw the player
	DrawPlayer(&theCar, &othWorld[currentBuffer]);
	
	
	
	
	//DrawModel(&building1, &othWorld[currentBuffer]);
	//DrawModel(&building2, &othWorld[currentBuffer]);
	//DrawModel(&building3, &othWorld[currentBuffer]);
	//DrawModel(&building4, &othWorld[currentBuffer]);
	//DrawModel(&building5, &othWorld[currentBuffer]);
	//DrawModel(&building6, &othWorld[currentBuffer]);
	//DrawModel(&building7, &othWorld[currentBuffer]);
	
	DrawModel(&barrier2, &othWorld[currentBuffer]);
	DrawModel(&barrier3, &othWorld[currentBuffer]);
	DrawModel(&barrier4, &othWorld[currentBuffer]);
	DrawModel(&barrier5, &othWorld[currentBuffer]);
	DrawModel(&barrier6, &othWorld[currentBuffer]);
	
	
	
	
	// Wait for end of drawing
	DrawSync(0);
	
	// Wait for V_BLANK interrupt
	vsyncInterval = VSync(0);
	
	// Print the vSync interval
	//FntPrint("VSync Interval: %d.\n", vsyncInterval);
	
	// Force text output
	FntFlush(-1);
	
	// Swap The Buffers
	GsSwapDispBuff();
	
	// Register clear-command: clear to black
	GsSortClear(0, 0, 0, &othWorld[currentBuffer]);
	
	// Register request to draw ordering table
	GsDrawOt(&othWorld[currentBuffer]);
}