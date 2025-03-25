#include "game.h"
#include "player.h"

void InitialiseTrackerView(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ) {
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
	view->super = &theCar.gsObjectCoord;
	
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
    view->super = &theCar.gsObjectCoord;
    
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

void RotateModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, int nRX, int nRY, int nRZ, long *speed) {
    // Only allow rotation if the car is moving
    if (*speed != 0) {
        MATRIX matTmp;

        // Reset the player's coordinate system to the identity matrix
        ResetMatrix(gsObjectCoord->coord.m);

        // Calculate speedFactor based on the car's speed
        // Directly scale rotation by the car's speed
        int speedFactor = abs(*speed); // Use absolute value of speed

        // Clamp speedFactor to avoid overscaling
        if (speedFactor > MAX_SPEED) speedFactor = MAX_SPEED;

        // Scale the rotation values based on speedFactor
        nRX = (nRX * speedFactor) / MAX_SPEED;
        nRY = (nRY * speedFactor) / MAX_SPEED;
        nRZ = (nRZ * speedFactor) / MAX_SPEED;

        // Clamp rotation values to prevent excessive rotation at low speeds
        int maxRotation = (ONE / 16); // Maximum rotation at low speeds (adjust as needed)
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
