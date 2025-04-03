#include <libps.h>
#include "ground.h"

char groundDataTrack1[GROUND_MAX_Z][GROUND_MAX_X] = {
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

void InitialiseGroundTextures() {
	
	// Load ground textures into vram
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
}

void InitialiseGround() {
	int tmpx, tmpz;
	char groundData;
	// Initialise total number of models to zero
	theGround.nTotalModels = 0;

	// Read the worldGroundData array and place an instance of the model at the appropriate position in the world
	for (tmpz = 0; tmpz < GROUND_MAX_Z; tmpz++) {
		for (tmpx = 0; tmpx < GROUND_MAX_X; tmpx++) {
			groundData = groundDataTrack1[tmpz][tmpx];

			if (groundData == '1') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_L_1_MEM_ADDR);
			}
			if (groundData == '2') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_R_1_MEM_ADDR);
			}
			if (groundData == '3') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)GRASS_MEM_ADDR);
			}
			if (groundData == '4') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)LINE_L_MEM_ADDR);
			}
			if (groundData == '5') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)LINE_R_MEM_ADDR);
			}
			if (groundData == '6') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_L_2_MEM_ADDR);
			}
			if (groundData == '7') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_R_2_MEM_ADDR);
			}
			if (groundData == '8') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_L_1_MEM_ADDR);
			}
			if (groundData == 'a') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_L_2_MEM_ADDR);
			}
			if (groundData == 'c') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_L_3_MEM_ADDR);
			}
			if (groundData == 'd') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_L_4_MEM_ADDR);
			}
			if (groundData == '9') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_R_1_MEM_ADDR);
			}
			if (groundData == 'b') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_R_2_MEM_ADDR);
			}
			if (groundData == 'e') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_R_3_MEM_ADDR);
			}
			if (groundData == 'f') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_R_4_MEM_ADDR);
			}
			if (groundData == 'g') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)GRID_L_MEM_ADDR);
			}
			if (groundData == 'h') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)GRID_R_MEM_ADDR);
			}
			if (groundData == 'i') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_L_01_MEM_ADDR);
			}
			if (groundData == 'j') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_L_02_MEM_ADDR);
			}
			if (groundData == 'k') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_R_01_MEM_ADDR);
			}
			if (groundData == 'l') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_R_02_MEM_ADDR);
			}
		}
	}
}

void AddModelToGround(GroundStruct *theGround, int nX, int nY, int nZ, unsigned long *lModelAddress) {
    theGround->lObjectPointer[theGround->nTotalModels] = (unsigned long*)lModelAddress;
	
	// Increment the pointer to move past the model id
	theGround->lObjectPointer[theGround->nTotalModels]++;
	
	// Map tmd data to its actual address
	GsMapModelingData(theGround->lObjectPointer[theGround->nTotalModels]);
	
	// Initialise the objects coordinate system - set to be that of the WORLD
	GsInitCoordinate2(WORLD, &theGround->gsObjectCoord[theGround->nTotalModels]);
	
	// Increment pointer twice more - to point to top of model data
    theGround->lObjectPointer[theGround->nTotalModels]++;
	theGround->lObjectPointer[theGround->nTotalModels]++;
	
	// Link the model (tmd) with the object handler
	GsLinkObject4((unsigned long *)theGround->lObjectPointer[theGround->nTotalModels], &theGround->gsObjectHandler[theGround->nTotalModels], 0);
	
	// Set the amount of polygon subdivision that will be done at runtime (none)
	theGround->gsObjectHandler[theGround->nTotalModels].attribute = GsDIV1;
	
	// Assign the coordinates of the object model to the Object Handler
	theGround->gsObjectHandler[theGround->nTotalModels].coord2 = &theGround->gsObjectCoord[theGround->nTotalModels];
    
	// Set The Position of the Object
    theGround->gsObjectCoord[theGround->nTotalModels].coord.t[0] = nX;
    theGround->gsObjectCoord[theGround->nTotalModels].coord.t[1] = nY;
	theGround->gsObjectCoord[theGround->nTotalModels].coord.t[2] = nZ;
	
	// Increment the object counter
	theGround->nTotalModels++;
    
	// Flag the object as needing to be drawn
	theGround->gsObjectCoord[theGround->nTotalModels].flg = 0;
}

void DrawGround(GroundStruct *theGround, PlayerStruct *currentPlayer, GsOT *OTable_Header) {
    MATRIX  tmpls, tmplw;
    int nCurrentModel;
    
    //FntPrint(fontID_1, "Ground models: %d\n\n", (theGround->nTotalModels));

    for (nCurrentModel = 0; nCurrentModel < theGround->nTotalModels; nCurrentModel++) {
        
		// Only render if object is near the current player
        if (IsObjectNearPlayer(currentPlayer, &theGround->gsObjectCoord[nCurrentModel])) {
            
			// Get the local world and screen coordinates
            GsGetLws(theGround->gsObjectHandler[nCurrentModel].coord2, &tmplw, &tmpls);
            
            // Set the resulting matrices
            GsSetLightMatrix(&tmplw);
            GsSetLsMatrix(&tmpls);
            
            // Send Object To Ordering Table
            GsSortObject4(&theGround->gsObjectHandler[nCurrentModel], OTable_Header, 2, (u_long *)getScratchAddr(0));
        }
    }
}