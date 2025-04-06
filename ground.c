#include <libps.h>
#include "ground.h"

char groundDataTrack1[GROUND_MAX_Z][GROUND_MAX_X] = {
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3'},
	{'3','3','d','i','1','1','1','h','h','i','4','i','1','1','1','1','1','1','1','1','1','1','1','1','1','1','i','8','3','3'},
	{'3','3','l','9','u','2','2','g','g','k','5','k','2','2','2','2','2','2','2','2','2','2','2','2','2','b','9','j','3','3'},
	{'3','3','7','m','r','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','n','o','6','3','3'},
	{'3','3','7','6','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3'},
	{'3','3','7','6','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3'},
	{'3','3','7','t','q','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3'},
	{'3','3','l','9','f','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','i','8','3','3','7','6','3','3'},
	{'3','3','c','k','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','b','9','j','3','3','7','6','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','n','o','6','3','3','7','6','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3','7','6','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3','7','6','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','p','e','6','3','3','7','6','3','3'},
	{'3','3','d','i','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','s','9','j','3','3','7','6','3','3'},
	{'3','3','l','9','u','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','k','a','3','3','7','6','3','3'},
	{'3','3','7','m','r','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3'},
	{'3','3','7','6','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3'},
	{'3','3','7','6','3','3','3','3','3','3','3','3','3','3','3','3','d','1','1','1','1','1','8','3','3','3','7','6','3','3'},
	{'3','3','7','6','3','3','3','3','3','3','3','3','3','3','3','3','l','9','u','2','b','9','j','3','3','3','7','6','3','3'},
	{'3','3','7','6','3','3','3','3','3','3','3','3','3','3','3','3','7','m','r','3','n','o','6','3','3','3','7','6','3','3'},
	{'3','3','7','t','q','3','3','3','3','3','3','3','3','3','3','p','e','6','3','3','3','7','6','3','3','3','7','6','3','3'},
	{'3','3','l','9','f','1','1','1','1','1','1','1','1','1','1','s','9','j','3','3','3','7','6','3','3','3','7','6','3','3'},
	{'3','3','c','k','2','2','2','2','2','2','2','2','2','2','2','2','k','a','3','3','3','7','6','3','3','3','7','6','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','6','3','3','3','7','6','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','7','t','q','3','p','e','6','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','l','9','f','1','s','9','j','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','c','k','2','2','2','k','a','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3'},
	{'3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3'},
};

void InitialiseGroundTextures() {
	
	// Load ground textures into vram
	LoadTexture(LINE_L_TEX_MEM_ADDR);					
	LoadTexture(LINE_R_TEX_MEM_ADDR);					
	LoadTexture(STRAIGHT_L_1_TEX_MEM_ADDR);								
	LoadTexture(TURN_L_1_TEX_MEM_ADDR);						
	LoadTexture(TURN_R_1_TEX_MEM_ADDR);	
	LoadTexture(GRID_TEX_MEM_ADDR);				
	LoadTexture(STRAIGHT_L_01_TEX_MEM_ADDR);							
	LoadTexture(GRASS_TEX_MEM_ADDR); 
	LoadTexture(TURN_00_TEX_MEM_ADDR); 
	LoadTexture(TURN_01_TEX_MEM_ADDR); 
	LoadTexture(TURN_02_TEX_MEM_ADDR); 
}

/*
1 = STRAIGHT_L_1_MEM_ADDR 					data\track\st_l_1\st_l_1
2 = STRAIGHT_L_1_MEM_ADDR + rotate 180 		*data\track\st_l_1\st_l_1
3 = GRASS_MEM_ADDR          				data\track\grass_1\grass_1
4 = LINE_L_MEM_ADDR           				data\track\line_l\line_l
5 = LINE_L_MEM_ADDR + rotate 180           	*data\track\line_l\line_l
6 = STRAIGHT_L_1_MEM_ADDR + rotate 90		*data\track\st_l_1\st_l_1
7 = STRAIGHT_L_1_MEM_ADDR + rotate 270		*data\track\st_l_1\st_l_1
8 = TURN_L_1_MEM_ADDR 						data\track\t_l_1\t_l_1
9 = TURN_R_1_MEM_ADDR 						data\track\t_r_1\t_r_1
a = TURN_L_1_MEM_ADDR + rotate 90			*data\track\t_l_1\t_l_1
b = TURN_00_MEM_ADDR 						data\track\t_00\t_00
c = TURN_L_1_MEM_ADDR + rotate 180			*data\track\t_l_1\t_l_1
d = TURN_L_1_MEM_ADDR + rotate 270			*data\track\t_l_1\t_l_1
e = TURN_00_MEM_ADDR + rotate 90			data\track\t_00\t_00
f = TURN_00_MEM_ADDR + rotate 180			*data\track\t_00\t_00
g = GRID_MEM_ADDR         				    data\track\grid\grid
h = GRID_MEM_ADDR + rotate 180        	    *data\track\grid_l\grid_l
i = STRAIGHT_L_01_MEM_ADDR					data\track\st_l_01\st_l_01
j = STRAIGHT_L_01_MEM_ADDR + rotate 90		*data\track\st_l_01\st_l_01
k = STRAIGHT_L_01_MEM_ADDR + rotate 180		*data\track\st_l_01\st_l_01
l = STRAIGHT_L_01_MEM_ADDR + rotate 270		*data\track\st_l_01\st_l_01
m = TURN_00_MEM_ADDR + rotate 270			*data\track\t_00\t_00
n = TURN_01_MEM_ADDR						data\track\t_01\t_01
o = TURN_02_MEM_ADDR						data\track\t_02\t_02
p = TURN_01_MEM_ADDR + rotate 90			*data\track\t_01\t_01
q = TURN_01_MEM_ADDR + rotate 180			*data\track\t_01\t_01
r = TURN_01_MEM_ADDR + rotate 270			*data\track\t_01\t_01
s = TURN_02_MEM_ADDR + rotate 90			*data\track\t_02\t_02
t = TURN_02_MEM_ADDR + rotate 180			*data\track\t_02\t_02
u = TURN_02_MEM_ADDR + rotate 270			*data\track\t_02\t_02
v = 
x = 
y = 
z = 
*/

void InitialiseGround() {
	int tmpx, tmpz;
	char groundData;
	// Initialise total number of models to zero
	theGround.nTotalModels = 0;

	// Read the worldGroundData array and place an instance of the model at the appropriate position in the world
	for (tmpz = 0; tmpz < GROUND_MAX_Z; tmpz++) {
		for (tmpx = 0; tmpx < GROUND_MAX_X; tmpx++) {
			
			groundData = groundDataTrack1[tmpz][tmpx];

			// Straight road (left and right lanes)
			if (groundData == '1') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_L_1_MEM_ADDR);
			}
			if (groundData == '2') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_L_1_MEM_ADDR);
				RotateGround180(&theGround);
			}

			// Grass
			if (groundData == '3') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)GRASS_MEM_ADDR);
			}
			
			// Race line (left and right lanes)
			if (groundData == '4') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)LINE_L_MEM_ADDR);
			}
			if (groundData == '5') {

				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)LINE_L_MEM_ADDR);
				RotateGround180(&theGround);
			}
			
			// Straight road (left and right lanes rotated)
			if (groundData == '6') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_L_1_MEM_ADDR);
				RotateGround90(&theGround);
			}
			if (groundData == '7') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_L_1_MEM_ADDR);
				RotateGround270(&theGround);
			}
			
			// Turn (outter lane and rotations)
			if (groundData == '8') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_L_1_MEM_ADDR);
				
			}
			if (groundData == 'a') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_L_1_MEM_ADDR);
				RotateGround90(&theGround);
			}
			if (groundData == 'c') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_L_1_MEM_ADDR);
				RotateGround180(&theGround);
			}
			if (groundData == 'd') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_L_1_MEM_ADDR);
				RotateGround270(&theGround);
			}
			
			// Blank tarmac tile
			if (groundData == '9') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_R_1_MEM_ADDR);
			}
			
			// Racing grid (left and right lanes)
			if (groundData == 'g') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)GRID_MEM_ADDR);
				RotateGround180(&theGround);
			}
			if (groundData == 'h') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)GRID_MEM_ADDR);
			}
			
			// Straight road no centre line (left and rotated)
			if (groundData == 'i') {
                AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_L_01_MEM_ADDR);
			}
			if (groundData == 'j') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_L_01_MEM_ADDR);
				RotateGround90(&theGround);
			}
			
			// Straight road no centre line (right and rotated)
			if (groundData == 'k') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_L_01_MEM_ADDR);
				RotateGround180(&theGround);
			}
			if (groundData == 'l') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)STRAIGHT_L_01_MEM_ADDR);
				RotateGround270(&theGround);
			}			
			
			// Inner turn (entering corner and rotations)
			if (groundData == 'b') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_00_MEM_ADDR);
			}
			if (groundData == 'e') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_00_MEM_ADDR);
				RotateGround90(&theGround);
			}
			if (groundData == 'f') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_00_MEM_ADDR);
				RotateGround180(&theGround);
			}
			if (groundData == 'm') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_00_MEM_ADDR);
				RotateGround270(&theGround);
			}

			// Inner turn (corner and rotations)
			if (groundData == 'n') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_01_MEM_ADDR);
			}
			if (groundData == 'p') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_01_MEM_ADDR);
				RotateGround90(&theGround);
			}
			if (groundData == 'q') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_01_MEM_ADDR);
				RotateGround180(&theGround);
			}
			if (groundData == 'r') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_01_MEM_ADDR);
				RotateGround270(&theGround);
			}
			
			// Inner turn (exiting corner and rotations)
			if (groundData == 'o') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_02_MEM_ADDR);
			}
			if (groundData == 's') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_02_MEM_ADDR);
				RotateGround90(&theGround);
			}
			if (groundData == 't') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_02_MEM_ADDR);
				RotateGround180(&theGround);
			}
			if (groundData == 'u') {
				AddModelToGround(&theGround, (tmpz * SEPERATION), (0), (tmpx * SEPERATION), (long *)TURN_02_MEM_ADDR);
				RotateGround270(&theGround);
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

// Rotate a ground section 90 degrees around Y axis
void RotateGround90(GroundStruct *theGround) {
    SVECTOR rotateVector;
    RotateGround(&theGround->gsObjectCoord[theGround->nTotalModels -1], &rotateVector, 0, 5125, 0);
}

// Rotate a ground section 180 degrees around Y axis
void RotateGround180(GroundStruct *theGround) {
    SVECTOR rotateVector;
    RotateGround(&theGround->gsObjectCoord[theGround->nTotalModels -1], &rotateVector, 0, 10250, 0);
}

// Rotate a ground section 270 degrees around Y axis
void RotateGround270(GroundStruct *theGround) {
    SVECTOR rotateVector;
    RotateGround(&theGround->gsObjectCoord[theGround->nTotalModels -1], &rotateVector, 0, 15375, 0);
}

// Rotate a ground section using X/Y/Z values
void RotateGround(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, int nRX, int nRY, int nRZ) {
    MATRIX matTmp;
	
    // Update rotation vector
    rotateVector->vx = nRX;
    rotateVector->vy = (rotateVector->vy + nRY) % ONE;
    rotateVector->vz = nRZ;
    
    // Reset the coordinate system
    ResetMatrix(gsObjectCoord->coord.m);
    
    // Set up the rotation matrix
    RotMatrix(rotateVector, &matTmp);
    
    // Apply the rotation
    MulMatrix0(&gsObjectCoord->coord, &matTmp, &gsObjectCoord->coord);
    
    // Mark for redraw
    gsObjectCoord->flg = 0;
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