#include "game.h"
#include "player.h"
#include "graphics.h"




/*
void InitView(GsRVIEW2 *tView, int tProjDist, int tRZ, int tFromX, int tFromY, int tFromZ, int tToX, int tToY, int tToZ, GsCOORDINATE2 *tReference) {
	GsSetProjection(tProjDist);                        
	tView->vpx = tFromX;  tView->vpy = tFromY;  tView->vpz = tFromZ;                             
	tView->vrx = tToX;    tView->vry = tToY;    tView->vrz = tToZ;
                                                     
	tView->rz =- tRZ;                                     
	if (tReference != false) {
		tView->super = tReference; // set origin of model
	}
	else {
		tView->super = WORLD;
	}
};
*/




void InitialiseTrackerViewPlayer1(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ) {
	// This is the distance between the eye and the imaginary projection screen
	GsSetProjection(nProjDist);
	
	// Set the eye position or center of projection
	view->vpx = nVPX; 
	view->vpy = nVPY;  
	view->vpz = nVPZ;
	
	// Set the look at position
	view->vrx = nVRX; 
	view->vry = nVRY; 
	view->vrz = nVRZ;
	
	// Set which way is up
	view->rz =- nRZ;
	
	// Set the origin of the coord system in this case the car
	view->super = &player1.gsObjectCoord;
	
	// Activate view
	GsSetRefView2(view);
}






void InitialiseTrackerViewPlayer2(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ) {
	// This is the distance between the eye and the imaginary projection screen
	GsSetProjection(nProjDist);
	
	// Set the eye position or center of projection
	view->vpx = nVPX; 
	view->vpy = nVPY;  
	view->vpz = nVPZ;
	
	// Set the look at position
	view->vrx = nVRX; 
	view->vry = nVRY; 
	view->vrz = nVRZ;
	
	// Set which way is up
	view->rz =- nRZ;
	
	// Set the origin of the coord system in this case the car
	view->super = &player2.gsObjectCoord;
	
	// Activate view
	GsSetRefView2(view);
}








void InitialiseTopDownView(GsRVIEW2 *view, int nProjDist, int nHeight, int nVPX, int nVPZ) {
    // This is the distance between the eye and the imaginary projection screen
    GsSetProjection(nProjDist);
    
    // Set the eye position (camera position)
    // Place the camera directly above the car at a specified height
    view->vpx = nVPX;        // X position (same as car's X)
    view->vpy = nHeight;     // Y position (height above the car)
    view->vpz = nVPZ;        // Z position (same as car's Z)
    
    // Set the look-at position (car's position)
    view->vrx = nVPX;        // Look at the car's X position
    view->vry = 1500;        // Look at the car's Y position (initial Y position of the car)
    view->vrz = nVPZ;        // Look at the car's Z position
    
    // Set which way is up
    view->rz = -1000;
    
    // Set the origin of the coordinate system (the car)
    view->super = &player1.gsObjectCoord;
    
    // Activate view
    GsSetRefView2(view);
}

void InitialiseStaticView(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ) {
	// This is the distance between the eye and the imaginary projection screen
	GsSetProjection(nProjDist);
	
	// Set the eye position or center of projection
	view->vpx = nVPX; 
	view->vpy = nVPY;  
	view->vpz = nVPZ;
	
	// Set the look at position
	view->vrx = nVRX; 
	view->vry = nVRY; 
	view->vrz = nVRZ;
	
	// Set which way is up
	view->rz =- nRZ;
	
	// Set the origin of the coord system in this case the WORLD
	view->super = WORLD;
	
	// Activate view
	GsSetRefView2(view);
}

void ResetMatrix(short m[3][3]) {
	m[0][0]=m[1][1]=m[2][2]=ONE;
	m[0][1]=m[0][2]=m[1][0]=m[1][2]=m[2][0]=m[2][1]=0;
}


/*
void RotateModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, int nRX, int nRY, int nRZ, long *speed) {
    int speedFactor;
	int maxRotation;
	
	// Only allow rotation if the car is moving
    if (*speed != 0) {
        MATRIX matTmp;

        // Reset the player's coordinate system to the identity matrix
        ResetMatrix(gsObjectCoord->coord.m);

        // Calculate speedFactor based on the car's speed
        // Directly scale rotation by the car's speed
        speedFactor = abs(*speed); // Use absolute value of speed

        // Clamp speedFactor to avoid overscaling
        if (speedFactor > MAX_SPEED) speedFactor = MAX_SPEED;

        // Scale the rotation values based on speedFactor
        nRX = (nRX * speedFactor) / MAX_SPEED;
        nRY = (nRY * speedFactor) / MAX_SPEED;
        nRZ = (nRZ * speedFactor) / MAX_SPEED;

        // Clamp rotation values to prevent excessive rotation at low speeds
        maxRotation = (ONE / 16); // Maximum rotation at low speeds (adjust as needed)
        if (abs(nRX) > maxRotation) nRX = (nRX > 0) ? maxRotation : -maxRotation;
        if (abs(nRY) > maxRotation) nRY = (nRY > 0) ? maxRotation : -maxRotation;
        if (abs(nRZ) > maxRotation) nRZ = (nRZ > 0) ? maxRotation : -maxRotation;

        // Apply rotation
        rotateVector->vx = (rotateVector->vx + nRX) % ONE;
        rotateVector->vy = (rotateVector->vy + nRY) % ONE;
        rotateVector->vz = (rotateVector->vz + nRZ) % ONE;

        // Set up the rotation matrix
        RotMatrix(rotateVector, &matTmp);

        // Multiply the existing object's matrix with the rotation matrix
        MulMatrix0(&gsObjectCoord->coord, &matTmp, &gsObjectCoord->coord);

        // Mark for redraw
        gsObjectCoord->flg = 0;
    }
}

// Move the model with proper acceleration, deceleration, and smooth direction changes
void AdvanceModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, long *speed, int movementDirection) {
    MATRIX matTmp;
    SVECTOR startVector;
    SVECTOR currentDirection;

    // Apply acceleration or deceleration based on movementDirection
    if (movementDirection > 0) {  // Accelerating forward
        if (*speed < 0) {
            // If reversing, slow down first before going forward
            *speed += DECELERATION;
        } else {
            // Normal acceleration
            *speed += ACCELERATION;
            if (*speed > MAX_SPEED) {
                *speed = MAX_SPEED;
            }
        }
    } else if (movementDirection < 0) {  // Reversing
        if (*speed > 0) {
            // If moving forward, slow down first before reversing
            *speed -= DECELERATION;
        } else {
            // Normal reverse acceleration
            *speed -= ACCELERATION;
            if (*speed < MAX_REVERSE_SPEED) {
                *speed = MAX_REVERSE_SPEED;
            }
        }
    } else {  
        // No input, apply smooth deceleration to gradually stop
        if (*speed > 0) {
            *speed -= DECELERATION;
            if (*speed < 0) *speed = 0;
        } else if (*speed < 0) {
            *speed += DECELERATION;
            if (*speed > 0) *speed = 0;
        }
    }
	
    // Only move if speed is not zero
    if (*speed != 0) {
        // Set up original vector, pointing down the positive Z-axis
        startVector.vx = 0; startVector.vy = 0; startVector.vz = ONE;

        // Compute rotation matrix
        RotMatrix(rotateVector, &matTmp);

        // Multiply startVector by matTmp to get the direction vector
        ApplyMatrixSV(&matTmp, &startVector, &currentDirection);

        // Apply movement based on speed
        gsObjectCoord->coord.t[0] += (currentDirection.vx * (*speed)) / 4096;
        gsObjectCoord->coord.t[1] += (currentDirection.vy * (*speed)) / 4096;
        gsObjectCoord->coord.t[2] += (currentDirection.vz * (*speed)) / 4096;
		
        // Mark the object for redrawing
        gsObjectCoord->flg = 0;
    }
}
*/









void RotateModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, int nRX, int nRY, int nRZ) {
    MATRIX matTmp;
	long absSpeed;
    int steeringResponse;
	long multiplier;
	int maxRotation
	
	maxRotation = ONE / 8;
	absSpeed = abs(player1.speed);

    if (absSpeed > 0) {
        steeringResponse = STEERING_RESPONSE;
		
		// Prevent exterme rotation when at slower speeds
		if (absSpeed <= 40) {
			steeringResponse = 50;
		}
		else {
			multiplier = 30 + (absSpeed * 190 / MAX_SPEED); 			// Linear ramp
			if (multiplier > 220) {
				multiplier = 220; 										// Cap at max
			}
			steeringResponse = (STEERING_RESPONSE * multiplier) / absSpeed;
		}

		// Clamp to minimum response (50)
		if (steeringResponse < 50) {
			steeringResponse = 50;
		}

        // Scale rotation based on steering response
        nRX = (nRX * steeringResponse) / 100;
        nRY = (nRY * steeringResponse) / 100;
        nRZ = (nRZ * steeringResponse) / 100;
        
		// Apply rotation with limits
		nRX = (nRX > maxRotation) ? maxRotation : (nRX < -maxRotation) ? -maxRotation : nRX;
		nRY = (nRY > maxRotation) ? maxRotation : (nRY < -maxRotation) ? -maxRotation : nRY;
		nRZ = (nRZ > maxRotation) ? maxRotation : (nRZ < -maxRotation) ? -maxRotation : nRZ;
		

		// Add drifting effect at higher speeds
        //if (absSpeed > 200) {
            //nRY += (nRX * absSpeed) / (DRIFT_FACTOR * 10000);
        //}

		// Update rotation vector
		rotateVector->vx = (rotateVector->vx + nRX) % ONE;
		rotateVector->vy = (rotateVector->vy + nRY) % ONE;
		rotateVector->vz = (rotateVector->vz + nRZ) % ONE;
		
		// Reset the coordinate system
		ResetMatrix(gsObjectCoord->coord.m);
		
		// Set up the rotation matrix
		RotMatrix(rotateVector, &matTmp);
		
		// Apply the rotation
		MulMatrix0(&gsObjectCoord->coord, &matTmp, &gsObjectCoord->coord);
		
		// Mark for redraw
		gsObjectCoord->flg = 0;
    }
}










void AdvanceModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, long *speed, int movementDirection, int isBraking) {
    MATRIX matTmp;
    SVECTOR startVector = {0, 0, ONE};
    SVECTOR currentDirection;
    
    // Handle acceleration/deceleration
    if (movementDirection > 0) {  // Accelerating forward
        if (*speed < 0) {
            // Braking while reversing
            *speed += isBraking ? BRAKE_DECELERATION * 2 : DECELERATION;
        } else {
            // Accelerate forward
            *speed += ACCELERATION;
            if (*speed > MAX_SPEED) *speed = MAX_SPEED;
        }
    } 
    else if (movementDirection < 0) {  // Reversing
        if (*speed > 0) {
            // Braking while moving forward
            *speed -= isBraking ? BRAKE_DECELERATION * 2 : DECELERATION;
        } else {
            // Accelerate in reverse
            *speed -= ACCELERATION / 2; // Reverse is slower
            if (*speed < MAX_REVERSE_SPEED) *speed = MAX_REVERSE_SPEED;
        }
    } 
    else {  // No input - natural deceleration
        if (*speed > 0) {
            *speed -= isBraking ? BRAKE_DECELERATION : DECELERATION / 2;
            if (*speed < 0) *speed = 0;
        } 
        else if (*speed < 0) {
            *speed += isBraking ? BRAKE_DECELERATION : DECELERATION / 2;
            if (*speed > 0) *speed = 0;
        }
    }
    
    // Only move if speed is not zero
    if (*speed != 0) {
        // Compute rotation matrix
        RotMatrix(rotateVector, &matTmp);
        
        // Get current direction vector
        ApplyMatrixSV(&matTmp, &startVector, &currentDirection);
        
        // Apply movement 
        gsObjectCoord->coord.t[0] += (currentDirection.vx * (*speed)) / ONE;
        gsObjectCoord->coord.t[1] += (currentDirection.vy * (*speed)) / ONE;
        gsObjectCoord->coord.t[2] += (currentDirection.vz * (*speed)) / ONE;
        
        // Mark the object for redrawing
        gsObjectCoord->flg = 0;
    }
}




void InitSingleScreen() {

  SplitScreenInfo[0].clip.x = 0;                     
  SplitScreenInfo[0].clip.y = 0;
  SplitScreenInfo[0].clip.w = 320;
  SplitScreenInfo[0].clip.h = 240;
  SplitScreenInfo[0].ofs[0] = 0 + (320 / 2);          
  SplitScreenInfo[0].ofs[1] = 0 + (240 / 2);

  SplitScreenInfo[1].clip.x = 0;                     
  SplitScreenInfo[1].clip.y = 240;
  SplitScreenInfo[1].clip.w = 320;
  SplitScreenInfo[1].clip.h = 240;
  SplitScreenInfo[1].ofs[0] = 0 + (320 / 2);          
  SplitScreenInfo[1].ofs[1] = 0 + (240 / 2) + 240;
};






void InitSplitScreen(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {

  SplitScreenInfo[0].clip.x = x1;                     
  SplitScreenInfo[0].clip.y = y1;
  SplitScreenInfo[0].clip.w = w1;
  SplitScreenInfo[0].clip.h = h1;
  SplitScreenInfo[0].ofs[0] = x1 + (w1 / 2);          
  SplitScreenInfo[0].ofs[1] = y1 + (h1 / 2);

  SplitScreenInfo[1].clip.x = x1;                     
  SplitScreenInfo[1].clip.y = y1 + 240;
  SplitScreenInfo[1].clip.w = w1;
  SplitScreenInfo[1].clip.h = h1;
  SplitScreenInfo[1].ofs[0] = x1 + (w1 / 2);          
  SplitScreenInfo[1].ofs[1] = y1 + (h1 / 2) + 240;

  SplitScreenInfo[2].clip.x = x2;                     
  SplitScreenInfo[2].clip.y = y2;
  SplitScreenInfo[2].clip.w = w2;
  SplitScreenInfo[2].clip.h = h2;
  SplitScreenInfo[2].ofs[0] = x2 + (w2 / 2);          
  SplitScreenInfo[2].ofs[1] = y2 + (h2 / 2);

  SplitScreenInfo[3].clip.x = x2;                     
  SplitScreenInfo[3].clip.y = y2 + 240;
  SplitScreenInfo[3].clip.w = w2;
  SplitScreenInfo[3].clip.h = h2;
  SplitScreenInfo[3].ofs[0] = x2 + (w2 / 2);          
  SplitScreenInfo[3].ofs[1] = y2 + (h2 / 2) + 240;
};