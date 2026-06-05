#include "game.h"
#include "game/player.h"
#include "ground.h"
#include "../engine/graphics.h"
#include "../engine/calculations.h"
#include "../engine/model.h"

long player1_lateralSpeed = 0;
long player2_lateralSpeed = 0;
int  player1_isBraking    = 0;
int  player2_isBraking    = 0;
long player1_pitch        = 0;
long player1_roll         = 0;


// Scale all vertices in a TMD by dividing by 'divisor' (must be called after InitialisePlayer)
void ScaleTmdVertices(unsigned long *tmdAddr, int divisor) {
	unsigned long vert_p_abs;
	unsigned long n_vert;
	short *v;
	int i;
	
	// Object table starts at word index 3 (after id, flags, nobject)
	vert_p_abs = tmdAddr[5];
	n_vert = tmdAddr[6];
	v = (short *)vert_p_abs;
	for (i = 0; i < (int)n_vert; i++) {
		v[0] = (short)(v[0] / divisor); // vx
		v[1] = (short)(v[1] / divisor); // vy
		v[2] = (short)(v[2] / divisor); // vz
		// v[3] is padding
		
		// 4 shorts = 8 bytes per vertex
		v += 4;
	}
}


void InitialiseTrackerViewPlayer1(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ) {
	// This is the distance between the eye and the imaginary projection screen
	GsSetProjection(200);

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


void InitialiseFrontViewPlayer1(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ) {
	// This is the distance between the eye and the imaginary projection screen
	GsSetProjection(nProjDist);

	// Set the eye position or center of projection
	view->vpx = 0;  // dont touch
	view->vpy = -400;
	view->vpz = -600;

	// Set the look at position
	view->vrx = 0; // dont touch
	view->vry = -500;
	view->vrz = 100;

	// Set which way is up
	view->rz =- nRZ;

	// Set the origin of the coord system in this case the car
	view->super = &player1.gsObjectCoord;

	// Activate view
	GsSetRefView2(view);
}


void InitialiseTopDownView(GsRVIEW2 *view, int nProjDist, int nHeight, int nVPX, int nVPZ) {
    // This is the distance between the eye and the imaginary projection screen
    GsSetProjection(nProjDist);

    // Set the eye position (camera position)
    // Place the camera directly above the car at a specified height
    view->vpx = 0;
	view->vpy = -3000; // height
	view->vpz = -1000;

    // Set the look-at position (car's position)
    iew->vrx = 12000;
	view->vry = 800;
	view->vrz = 0; // angle

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
	view->vpx = 0;
	view->vpy = -32000; // height
	view->vpz = -1000;

	// Set the look at position
	view->vrx = 12000;
	view->vry = 800;
	view->vrz = 18000; // angle

	// Set which way is up
	view->rz =- nRZ;

	// Set the origin of the coord system in this case the WORLD
	view->super = WORLD;

	// Activate view
	GsSetRefView2(view);
}


void RotateModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, int nRX, int nRY, int nRZ, long *speed, long *lateralSpeed) {
    MATRIX matTmp;
    long absSpeed;
    long absLateral;
    long gripLimit;
    long maxLateral;
    int steeringResponse;
    int maxRotation;
    int originalNRY;

    maxRotation = ONE / 8;
    absSpeed = abs(*speed);
    absLateral = abs(*lateralSpeed);
    originalNRY = nRY;

    if (absSpeed > 0) {
        steeringResponse = activeVehicle->steeringResponse - (absSpeed * (activeVehicle->steeringResponse - activeVehicle->minSteeringResponse) / activeVehicle->maxSpeed);
        if (steeringResponse < activeVehicle->minSteeringResponse) steeringResponse = activeVehicle->minSteeringResponse;

        gripLimit = activeVehicle->maxGrip - (absSpeed * (activeVehicle->maxGrip - activeVehicle->minGrip) / activeVehicle->maxSpeed);
        if (gripLimit < activeVehicle->minGrip) gripLimit = activeVehicle->minGrip;

        // Body roll: follows lateral velocity
        {
            long maxLat = (long)(activeVehicle->maxSpeed / 3) + 1L;
            long targetRoll = (*lateralSpeed * (long)activeSuspension->rollFactor) / maxLat;
            if (targetRoll >  (long)activeSuspension->rollFactor) targetRoll =  (long)activeSuspension->rollFactor;
            if (targetRoll < -(long)activeSuspension->rollFactor) targetRoll = -(long)activeSuspension->rollFactor;
            if (player1_roll < targetRoll) {
                player1_roll += activeSuspension->suspensionRate;
                if (player1_roll > targetRoll) player1_roll = targetRoll;
            } else if (player1_roll > targetRoll) {
                player1_roll -= activeSuspension->suspensionRate;
                if (player1_roll < targetRoll) player1_roll = targetRoll;
            }
        }

        // Axle-split grip via weight transfer (slip-angle model)
        {
            long rollAbs    = player1_roll < 0L ? -player1_roll : player1_roll;
            long pitchShift = player1_pitch / 4L + (long)activeVehicle->understeerBias;
            long frontGrip  = gripLimit - pitchShift - rollAbs / 10L;
            long rearGrip   = gripLimit + pitchShift - rollAbs / 10L;
            if (frontGrip < activeVehicle->minGrip) frontGrip = activeVehicle->minGrip;
            if (rearGrip  < activeVehicle->minGrip) rearGrip = activeVehicle->minGrip;

            if (*speed > 0) {
                maxLateral = activeVehicle->maxSpeed / 3;

                // Understeer
                if (absLateral > frontGrip / 2) {
                    long excess  = absLateral - frontGrip / 2;
                    long washout = 100L - (excess * 160L / (frontGrip > 0L ? frontGrip : 1L));
                    
					if (washout < 20L) {
						washout = 20L;
                    }
					
					nRY = (nRY * washout) / 100L;
                }

                // Oversteer: if rear grip exceeded, rear swings outward
                if (absLateral > rearGrip) {
                    long breakaway = absLateral - rearGrip;
                    if (*lateralSpeed > 0L) {
						*lateralSpeed += breakaway;
                    }
					
					else {
						*lateralSpeed -= breakaway;
					}
				}

                // Lateral centripetal correction
                if (absLateral > rearGrip) {
                    *lateralSpeed -= (*speed * originalNRY * 2) / activeVehicle->turnRadiusFactor;
                } else {
                    *lateralSpeed -= (*speed * originalNRY) / activeVehicle->turnRadiusFactor;
                }

                if (*lateralSpeed > maxLateral) *lateralSpeed = maxLateral;
                if (*lateralSpeed < -maxLateral) *lateralSpeed = -maxLateral;
            }
        }

        nRX = (nRX * steeringResponse) / 100;
        nRY = (nRY * steeringResponse) / 100;
        nRZ = (nRZ * steeringResponse) / 100;
        nRX = (nRX > maxRotation) ? maxRotation : (nRX < -maxRotation) ? -maxRotation : nRX;
        nRY = (nRY > maxRotation) ? maxRotation : (nRY < -maxRotation) ? -maxRotation : nRY;
        nRZ = (nRZ > maxRotation) ? maxRotation : (nRZ < -maxRotation) ? -maxRotation : nRZ;

        rotateVector->vx = (rotateVector->vx + nRX) % ONE;
        rotateVector->vy = (rotateVector->vy + nRY) % ONE;
        rotateVector->vz = (rotateVector->vz + nRZ) % ONE;

        ResetMatrix(gsObjectCoord->coord.m);
        RotMatrix(rotateVector, &matTmp);
        MulMatrix0(&gsObjectCoord->coord, &matTmp, &gsObjectCoord->coord);
        gsObjectCoord->flg = 0;
    }
}


// Handle acceleration/deceleration
void AdvanceModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, long *speed, long *lateralSpeed, int movementDirection, int isBraking) {
    MATRIX matTmp;
    SVECTOR forwardBasis = {0, 0, ONE};
    SVECTOR rightBasis = {ONE, 0, 0};
    SVECTOR forwardDir;
    SVECTOR lateralDir;
    long absLateral;
    long absSpeed;
    long gripLimit;
    long massedAccel;
    long massedBrake;
    long massedDecel;
    TerrainType terrain;

    // Keep suspension config in sync with the selected vehicle
    activeSuspension = (selectedVehicleIndex < 3) ? &car3Suspension : (selectedVehicleIndex < 6) ? &car2Suspension : &car5Suspension;

    // Mass-adjusted forces: heavier vehicles accelerate and brake more slowly
    massedAccel = ((long)activeVehicle->acceleration * 100L) / activeSuspension->mass;
    massedBrake = ((long)activeVehicle->brakeDeceleration * 100L) / activeSuspension->mass;
    massedDecel = ((long)activeVehicle->deceleration * 100L) / activeSuspension->mass;
    
	if (massedAccel < 1L) massedAccel = 1L;
    if (massedBrake < 1L) massedBrake = 1L;
    if (massedDecel < 1L) massedDecel = 1L;

    // Terrain detection: reduce acceleration on grass/sand
    terrain = GetTerrainType(gsObjectCoord->coord.t[0], gsObjectCoord->coord.t[2]);
    if (terrain != TERRAIN_TRACK) {
        massedAccel = massedAccel / 3;
        if (massedAccel < 1L) massedAccel = 1L;
    }

    // Gear acceleration: apply torque curve and power band for current gear
    if (movementDirection > 0) {
        long fwdSpeed = (*speed > 0L) ? *speed : 0L;
        ApplyGearAccel(&massedAccel, fwdSpeed, (long)activeVehicle->maxSpeed);
    }

    // Accelerating forward
    if (movementDirection > 0) {
        if (*speed < 0) {
            *speed += massedBrake * 2;
        } else {
            *speed += massedAccel;
            
			if (*speed > activeVehicle->maxSpeed) {
				*speed = activeVehicle->maxSpeed;
			}
        }
    }
	
    // Braking or reversing
    else if (movementDirection < 0) {
        
		if (*speed > 0) {
            *speed -= massedBrake * 2;
            
			if (*speed < 0) {
				*speed = 0;
			}

		} else {
            *speed -= massedAccel / 2;
            
			if (*speed < activeVehicle->maxReverseSpeed) {
				*speed = activeVehicle->maxReverseSpeed;
			}
		}
    }
    // No throttle
    else {
        if (*speed > 0) {
            *speed -= isBraking ? massedBrake : massedDecel;
            
			if (*speed < 0) {
				*speed = 0;
			}
		}
        else if (*speed < 0) {
            *speed += isBraking ? massedBrake : massedDecel;
            
			if (*speed > 0) {
				*speed = 0;
			}
		}
    }

    // Off-road speed cap
    if (terrain != TERRAIN_TRACK && *speed > 0) {
        long offRoadCap = (terrain == TERRAIN_SAND) ? (long)activeVehicle->maxSpeed / 4 : (long)activeVehicle->maxSpeed / 3;
        
		if (*speed > offRoadCap) {
            *speed -= massedDecel * 3;

			if (*speed < offRoadCap) {
				*speed = offRoadCap;
			}
		}
    }

    // Gear speed ceiling: engine cannot rev past the current gears redline
    if (movementDirection > 0 && *speed > 0L) {
        long gearCeiling = GetGearTopSpeed((long)activeVehicle->maxSpeed);
        
		if (*speed > gearCeiling) {
			*speed = gearCeiling;
		}
	}

    // Weight transfer: update nose pitch based on throttle/brake state
    if (movementDirection > 0 && *speed > 0) {

		// Acceleration squat fades as speed builds: peak is 75% of pitchFactor, falling to 0 at maxSpeed
        long targetPitch = ((long)activeSuspension->pitchFactor * 3L * (activeVehicle->maxSpeed - *speed)) / (4L * (long)activeVehicle->maxSpeed);
        
		if (targetPitch < 0L) {
			targetPitch = 0L;
		}
        
		if (player1_pitch < targetPitch) {
            player1_pitch += activeSuspension->suspensionRate;
            
			if (player1_pitch > targetPitch) {
				player1_pitch = targetPitch;
			}
		} else if (player1_pitch > targetPitch) {
            player1_pitch -= activeSuspension->suspensionRate;
			
            if (player1_pitch < targetPitch) {
				player1_pitch = targetPitch;
			}
        }
    } else if ((movementDirection < 0 || isBraking) && *speed > 0) {
        
		// Brake dive fades as speed drops: peak is 75% of pitchFactor at maxSpeed, falling to 0 at rest
        long targetPitch = -((long)activeSuspension->pitchFactor * 3L * *speed) / (4L * (long)activeVehicle->maxSpeed);
        if (targetPitch > 0L) {
			targetPitch = 0L;
		}

		if (player1_pitch > targetPitch) {
            player1_pitch -= activeSuspension->suspensionRate;
            
			if (player1_pitch < targetPitch) {
				player1_pitch = targetPitch;
			}
		} else if (player1_pitch < targetPitch) {
            player1_pitch += activeSuspension->suspensionRate;
			
            if (player1_pitch > targetPitch) {
				player1_pitch = targetPitch;
			}
        }
    } else if (player1_pitch > 0L) {
        
		// Recovering toward level
        player1_pitch -= activeSuspension->suspensionRate;
        if (player1_pitch < 0L) {
			player1_pitch = 0L;
		}
		
	} else if (player1_pitch < 0L) {
        player1_pitch += activeSuspension->suspensionRate;
        
		if (player1_pitch > 0L) {
			player1_pitch = 0L;
		}
	}

    // Off-road suspension bump: oscillate pitch to simulate rough terrain
    if (terrain != TERRAIN_TRACK && *speed > 0) {
        static int bumpPhase = 0;
        long bumpAmp;
        
		if (++bumpPhase >= 6) {
			bumpPhase = 0;
		}

		bumpAmp = (long)activeSuspension->suspensionRate * 2L * (*speed) / (long)activeVehicle->maxSpeed;
        player1_pitch += (bumpPhase < 3) ? bumpAmp : -bumpAmp;
    }

    // Friction: grippy tires snap back quickly, sliding tires recover slowly
    absLateral = abs(*lateralSpeed);
    absSpeed = abs(*speed);
    gripLimit = activeVehicle->maxGrip - (absSpeed * (activeVehicle->maxGrip - activeVehicle->minGrip) / activeVehicle->maxSpeed);
    
	if (gripLimit < activeVehicle->minGrip) {
		gripLimit = activeVehicle->minGrip;
	}

    if (absLateral > gripLimit) {
        *lateralSpeed = (*lateralSpeed * activeVehicle->slidingFriction) / 1000;
    } else {
        *lateralSpeed = (*lateralSpeed * activeVehicle->lateralFriction) / 1000;
    }
	
    if (*lateralSpeed > -2 && *lateralSpeed < 2) {
		*lateralSpeed = 0;
	}

    // Move along both forward and lateral axes
    if (*speed != 0 || *lateralSpeed != 0) {
        RotMatrix(rotateVector, &matTmp);
        ApplyMatrixSV(&matTmp, &forwardBasis, &forwardDir);
        ApplyMatrixSV(&matTmp, &rightBasis, &lateralDir);

        gsObjectCoord->coord.t[0] += (forwardDir.vx * (*speed) + lateralDir.vx * (*lateralSpeed)) / ONE;
        gsObjectCoord->coord.t[1] += (forwardDir.vy * (*speed) + lateralDir.vy * (*lateralSpeed)) / ONE;
        gsObjectCoord->coord.t[2] += (forwardDir.vz * (*speed) + lateralDir.vz * (*lateralSpeed)) / ONE;

        gsObjectCoord->flg = 0;
    }

    // Apply suspension body tilt to the visual model (pitch = nose dive/squat, roll = cornering lean)
    {
        SVECTOR modelTilt;
        modelTilt.vx = (short)(3072 + (int)player1_pitch);
        modelTilt.vy = 2048;
        modelTilt.vz = (short)player1_roll;
        RotModel(&player1.gsModelCoord, &modelTilt, 0, 0, 0);
    }
}


// Initialise single-screen mode
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


// Initialise the split-screen co-ordinates
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