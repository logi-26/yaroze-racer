#include <libps.h>
#include "ai_racer.h"
#include "player.h"
#include "game.h"
#include "gear.h"
#include "world.h"
#include "../engine/model.h"

#define NUM_WAYPOINTS 73
#define LOOKAHEAD_DIST 1800
#define ADVANCE_DIST 600
#define BRAKE_SPEED 150
#define AVOID_DIST 2500  // Distance ahead to start steering around a car


AIRacer aiRacers[NUM_AI_RACERS];
int aiRacerBraking = 0;

typedef struct { long x; long z; } Waypoint;

static const Waypoint waypoints[NUM_WAYPOINTS] = {
    /*  0 */ {  3198, 10169 },
    /*  1 */ {  3150, 13148 },
    /*  2 */ {  3040, 21069 },
    /*  3 */ {  3040, 25595 },
    /*  4 */ {  3040, 28591 },
    /*  5 */ {  3386, 30500 },
    /*  6 */ {  4258, 31353 },
    /*  7 */ {  6768, 31713 },
    /*  8 */ { 11223, 31807 },
    /*  9 */ { 16343, 31807 },
    /* 10 */ { 20942, 31807 },
    /* 11 */ { 23734, 31807 },
    /* 12 */ { 26504, 31807 },
    /* 13 */ { 27921, 31807 },
    /* 14 */ { 29050, 31807 },
    /* 15 */ { 30191, 31807 },
    /* 16 */ { 31487, 30771 },
    /* 17 */ { 31916, 28520 },
    /* 18 */ { 31618, 26689 },
    /* 19 */ { 29832, 25657 },
    /* 20 */ { 29772, 25657 },
    /* 21 */ { 27119, 25677 },
    /* 22 */ { 25814, 25683 },
    /* 23 */ { 24679, 25688 },
    /* 24 */ { 23517, 25690 },
    /* 25 */ { 22255, 24923 },
    /* 26 */ { 22118, 23384 },
    /* 27 */ { 22118, 22356 },
    /* 28 */ { 22689, 20074 },
    /* 29 */ { 24421, 19558 },
    /* 30 */ { 26081, 19026 },
    /* 31 */ { 27043, 17743 },
    /* 32 */ { 27073, 16191 },
    /* 33 */ { 27038, 12243 },
    /* 34 */ { 27037,  8077 },
    /* 35 */ { 27037,  5762 },
    /* 36 */ { 26829,  3718 },
    /* 37 */ { 26245,  3101 },
    /* 38 */ { 24860,  2799 },
    /* 39 */ { 22044,  2947 },
    /* 40 */ { 20788,  3015 },
    /* 41 */ { 19358,  3097 },
    /* 42 */ { 18502,  3308 },
    /* 43 */ { 17809,  3679 },
    /* 44 */ { 17344,  4939 },
    /* 45 */ { 17332,  5798 },
    /* 46 */ { 17332,  8114 },
    /* 47 */ { 17332, 12055 },
    /* 48 */ { 17332, 16803 },
    /* 49 */ { 17332, 20434 },
    /* 50 */ { 17332, 22901 },
    /* 51 */ { 17332, 24411 },
    /* 52 */ { 17237, 25823 },
    /* 53 */ { 15263, 27042 },
    /* 54 */ { 13145, 26906 },
    /* 55 */ { 12083, 26823 },
    /* 56 */ { 10223, 25841 },
    /* 57 */ { 10045, 24066 },
    /* 58 */ { 10063, 21403 },
    /* 59 */ { 10103, 17429 },
    /* 60 */ { 10133, 13977 },
    /* 61 */ { 10170, 10296 },
    /* 62 */ { 10201,  7857 },
    /* 63 */ { 10203,  5408 },
    /* 64 */ {  9936,  3900 },
    /* 65 */ {  9114,  3008 },
    /* 66 */ {  7524,  2886 },
    /* 67 */ {  5691,  2944 },
    /* 68 */ {  4791,  3046 },
    /* 69 */ {  3277,  3934 },
    /* 70 */ {  3005,  5442 },
    /* 71 */ {  2994,  7622 },
    /* 72 */ {  2976, 10252 },
};


// Grid start positions — staggered by 3000 in Z (> AVOID_DIST)
// so avoidance does not fire between cars at race start
static const int spawnX[NUM_AI_RACERS] = { 3000, 3600, 4200 };
static const int spawnZ[NUM_AI_RACERS] = { 8800, 5800, 2800 };

// Per-racer vehicle data: racer 0 = car3, racer 1 = car2, racer 2 = car5
static const long aiTmdAddr[NUM_AI_RACERS] = {
    CAR_3Y_MEM_ADDR,
    CAR_2B_MEM_ADDR,
    CAR_5G_MEM_ADDR,
};

static VehicleAttributes * const aiVehicleAttribs[NUM_AI_RACERS] = {
    &car3Attribs,
    &car2Attribs,
    &car5Attribs,
};

static SuspensionConfig * const aiVehicleSuspensions[NUM_AI_RACERS] = {
    &car3Suspension,
    &car2Suspension,
    &car5Suspension,
};


// Initialise all AI-controlled racers at their starting positions and reset their runtime state
void InitialiseAIRacers(void) {
    int i;
	
	// Create and configure each AI racer
    for (i = 0; i < NUM_AI_RACERS; i++) 
	{
		
		// Initialise the AI racer using the predefined spawn position and model for this AI vehicle
        InitialisePlayer(&aiRacers[i].player, 0, spawnX[i], -200, spawnZ[i], (unsigned long *)aiTmdAddr[i]);

		// Set the models initial orientation so all AI racers start facing the correct direction on the track
        {
            SVECTOR modelRot = {0, 0, 0, 0};
            RotModel(&aiRacers[i].player.gsModelCoord, &modelRot, 3072, 2048, 0);
        }

		// Reset dynamic movement state
        aiRacers[i].lateralSpeed = 0;
        aiRacers[i].roll = 0;
        aiRacers[i].pitch = 0;
		
		// Start in first gear
        aiRacers[i].gear = 1;
		
		// Begin navigation at the first target waypoint
        aiRacers[i].waypointIndex = 1;
		
		// Clear steering interpolation/smoothing value
        aiRacers[i].steerSmooth = 0;
		
		// Assign vehicle-specific handling and suspension settings
        aiRacers[i].attribs = aiVehicleAttribs[i];
        aiRacers[i].suspension = aiVehicleSuspensions[i];
    }
}


// Avoidance — steer around any car (AI or player) directly ahead within AVOID_DIST
// Uses racer index to break ties (even racers prefer right, odd prefer left)
static int ComputeAvoidance(AIRacer *ai, int racerIdx) {
    
	// Forward direction vector of this AI racer
	long fwdX = ai->player.gsObjectCoord.coord.m[0][2];
    long fwdZ = ai->player.gsObjectCoord.coord.m[2][2];
    
	long dx, dz, dot, cross, absDX, absDZ;
    int j;

    // Check for other AI racers that may be blocking our path
    for (j = 0; j < NUM_AI_RACERS; j++) 
	{
		// Skip self
        if (j == racerIdx) 
		{
			continue;
		}
		
		// Vector from this racer to the other AI racer
        dx = aiRacers[j].player.gsObjectCoord.coord.t[0] - ai->player.gsObjectCoord.coord.t[0];
        dz = aiRacers[j].player.gsObjectCoord.coord.t[2] - ai->player.gsObjectCoord.coord.t[2];
        
		// Dot product determines whether the target is in front of us
		dot = (fwdX * (dx >> 5)) + (fwdZ * (dz >> 5));
		
		// Ignore racers that are behind us
        if (dot <= 0) 
		{
			continue; 
		}
        
		// Quick distance check using axis-aligned bounds
		absDX = dx < 0 ? -dx : dx;
        absDZ = dz < 0 ? -dz : dz;
		
		// Ignore racers outside the avoidance zone
        if (absDX > AVOID_DIST || absDZ > AVOID_DIST) 
		{
			continue;
		}
		
		// Cross product indicates which side the obstacle is on
        cross = (fwdZ * (dx >> 5)) - (fwdX * (dz >> 5));
		
		// Obstacle to right, steer left
        if (cross > 0) 
		{
			return -38; 
		}
		
		// Obstacle to left, steer right
        if (cross < 0) 
		{
			return  38; 
		}
		
		// Obstacle is directly ahead, pick a side
        return (racerIdx % 2 == 0) ? 38 : -38;
    }

    // Check the player vehicle using the same logic
    dx = player1.gsObjectCoord.coord.t[0] - ai->player.gsObjectCoord.coord.t[0];
    dz = player1.gsObjectCoord.coord.t[2] - ai->player.gsObjectCoord.coord.t[2];
	
	// Is the player in front of us
    dot = (fwdX * (dx >> 5)) + (fwdZ * (dz >> 5));
	
    if (dot > 0) 
	{
        absDX = dx < 0 ? -dx : dx;
        absDZ = dz < 0 ? -dz : dz;
		
		// Player is within avoidance range
        if (absDX <= AVOID_DIST && absDZ <= AVOID_DIST) 
		{
			// Determine which side of our forward vector the player is on
            cross = (fwdZ * (dx >> 5)) - (fwdX * (dz >> 5));
			
			// Player is on the right, steer left
            if (cross > 0) 
			{
				return -38;
			}
			
			// Player is on the left, steer right
            if (cross < 0) 
			{
				return  38;
			}

			// Player is directly ahead, pick a side
            return (racerIdx % 2 == 0) ? 38 : -38;
        }
    }

	// No avoidance action required
    return 0;
}


// Steering — proportional to turn angle via cross/dot ratio (≈ tan), distance-independent
static int ComputeSteering(GsCOORDINATE2 *coord, long targetX, long targetZ) {
    
	// Vector from the vehicle to the target waypoint
	long dX = targetX - coord->coord.t[0];
    long dZ = targetZ - coord->coord.t[2];
	
	// Vehicle forward direction vector
    long fwdX = coord->coord.m[0][2];
    long fwdZ = coord->coord.m[2][2];
	
	// Scale down values to reduce overflow risk in subsequent products
    long dXs = dX >> 5;
    long dZs = dZ >> 5;
	
	// Sign indicates which side of the vehicle the target is on
    long cross = (fwdZ * dXs) - (fwdX * dZs);
    
	// Measures how closely aligned the vehicle is with the target
	long dot = (fwdX * dXs) + (fwdZ * dZs);
    
	long absCross, absDot;
    int dir, mag;

	// Perfectly aligned with the target
    if (cross == 0) 
	{
		return 0;
	}
	
	// Determine steering direction
    dir = (cross > 0) ? 1 : -1;
	
    absCross = cross < 0 ? -cross : cross;
    absDot = dot   < 0 ? -dot : dot;

	// Small steering error
    if (absCross * 4 < absDot) 
	{
		mag = 14;
	}
	// Medium steering error
    else if (absCross * 4 < absDot * 3)  
	{
		mag = 26;   
	}
	// Large steering error
    else 
	{
		mag = 38; 
	}

    return dir * mag;
}


// Advances the AI racer to the next waypoint when the current one has been reached or has been passed
static void AdvanceWaypoint(AIRacer *ai, long *dX, long *dZ) {
    
	// Current target waypoint
	const Waypoint *wp = &waypoints[ai->waypointIndex];
    
	long fwdX, fwdZ, dot, absDX, absDZ;

	// Calculate vector from racer to the current waypoint
    *dX = wp->x - ai->player.gsObjectCoord.coord.t[0];
    *dZ = wp->z - ai->player.gsObjectCoord.coord.t[2];
	
	// Absolute distance components used for the proximity check
    absDX = *dX < 0 ? -(*dX) : *dX;
    absDZ = *dZ < 0 ? -(*dZ) : *dZ;

	// Racers forward direction vector
    fwdX = ai->player.gsObjectCoord.coord.m[0][2];
    fwdZ = ai->player.gsObjectCoord.coord.m[2][2];
	
	// Dot product between the forward vector and waypoint direction
    dot = (fwdX * (*dX >> 5)) + (fwdZ * (*dZ >> 5));

	// Advance to the next waypoint if we are close enough or we have already passed it
    if ((absDX < ADVANCE_DIST && absDZ < ADVANCE_DIST) || dot < 0) 
	{
		// Move to the next waypoint, wrapping back to the start
        ai->waypointIndex = (ai->waypointIndex + 1) % NUM_WAYPOINTS;
		
		// Recalculate the target vector using the new waypoint
        wp = &waypoints[ai->waypointIndex];
        *dX = wp->x - ai->player.gsObjectCoord.coord.t[0];
        *dZ = wp->z - ai->player.gsObjectCoord.coord.t[2];
    }
}


// AI vs AI collision — checks every pair and pushes both cars apart symmetrically
static void CheckAIAICollisions(void) {
    int i, j, dx, dz, combinedRadius, absDx, absDz, approxDist;
    long distSq, halfR, midX, midZ;
    AIRacer *a, *b;

	// Iterate over all unique pairs of AI racers
    for (i = 0; i < NUM_AI_RACERS - 1; i++) 
	{
        for (j = i + 1; j < NUM_AI_RACERS; j++) 
		{
            a = &aiRacers[i];
            b = &aiRacers[j];

			// Compute separation vector between the two racers
            dx = (int)(a->player.gsObjectCoord.coord.t[0] - b->player.gsObjectCoord.coord.t[0]);
            dz = (int)(a->player.gsObjectCoord.coord.t[2] - b->player.gsObjectCoord.coord.t[2]);
			
			// Combined collision radius defines overlap threshold
            combinedRadius = a->player.collisionRadius + b->player.collisionRadius;

			// Early rejection: outside bounding box in X axis
            if (dx > combinedRadius || dx < -combinedRadius) 
			{
				continue;
			}

			// Early rejection: outside bounding box in Z axis
            if (dz > combinedRadius || dz < -combinedRadius) 
			{
				continue;
			}

			// Precise collision test using squared distance
            distSq = (long)dx * dx + (long)dz * dz;
			
            if (distSq >= (long)combinedRadius * combinedRadius) 
			{
				continue;
			}

			// Absolute components used for approximate distance scaling
            absDx = dx < 0 ? -dx : dx;
            absDz = dz < 0 ? -dz : dz;
			
			// Cheap distance approximation (weighted Manhattan-like metric)
            approxDist = ((absDx > absDz ? absDx : absDz) * 123 + (absDx > absDz ? absDz : absDx) * 51) >> 7;

			// Half separation distance used to push each car apart equally
            halfR = combinedRadius / 2;
			
			// Handle near-zero distance case to avoid division instability
            if (approxDist < 1) 
			{
				// Nudge cars apart along X axis
                a->player.gsObjectCoord.coord.t[0] += halfR;
                b->player.gsObjectCoord.coord.t[0] -= halfR;
            } 
			else 
			{
				// Compute midpoint between the two cars
                midX = (a->player.gsObjectCoord.coord.t[0] + b->player.gsObjectCoord.coord.t[0]) / 2;
                midZ = (a->player.gsObjectCoord.coord.t[2] + b->player.gsObjectCoord.coord.t[2]) / 2;
				
				// Push each car away from midpoint along collision axis
                a->player.gsObjectCoord.coord.t[0] = midX + ((long)dx * halfR) / approxDist;
                a->player.gsObjectCoord.coord.t[2] = midZ + ((long)dz * halfR) / approxDist;
                b->player.gsObjectCoord.coord.t[0] = midX - ((long)dx * halfR) / approxDist;
                b->player.gsObjectCoord.coord.t[2] = midZ - ((long)dz * halfR) / approxDist;
            }

			// Force transform refresh after positional correction
            a->player.gsObjectCoord.flg = 0;
            b->player.gsObjectCoord.flg = 0;
			
			// Swap forward speed to simulate momentum exchange
            {
                long tmp = a->player.speed;
                a->player.speed = b->player.speed;
                b->player.speed = tmp;
            }
			
			// Remove sideways drift caused by collision resolution
            a->lateralSpeed = 0;
            b->lateralSpeed = 0;
        }
    }
}


// Handles collision between a single AI racer and the player vehicle
static void CheckAIPlayerCollision(AIRacer *ai) {
    int dx, dz, combinedRadius, absDx, absDz, approxDist;
    long distSq, halfR, midX, midZ, tmp;

	// Vector from player to AI racer
    dx = (int)(ai->player.gsObjectCoord.coord.t[0] - player1.gsObjectCoord.coord.t[0]);
    dz = (int)(ai->player.gsObjectCoord.coord.t[2] - player1.gsObjectCoord.coord.t[2]);
	
	// Combined radius defines collision threshold
    combinedRadius = ai->player.collisionRadius + player1.collisionRadius;

	// Early out if outside X bounds of collision range
    if (dx > combinedRadius || dx < -combinedRadius) 
	{
		return;
	}
	
	// Early out if outside Z bounds of collision range
    if (dz > combinedRadius || dz < -combinedRadius) 
	{
		return;
	}
	
	// Precise collision test using squared distance
    distSq = (long)dx * dx + (long)dz * dz;
	
    if (distSq >= (long)combinedRadius * combinedRadius) 
	{
		return;
	}
	
	// Absolute values for approximate distance scaling
    absDx = dx < 0 ? -dx : dx;
    absDz = dz < 0 ? -dz : dz;
	
	// Cheap approximation of distance magnitude (weighted metric)
    approxDist = ((absDx > absDz ? absDx : absDz) * 123 + (absDx > absDz ? absDz : absDx) * 51) >> 7;

	// Half separation distance applied to each object
    halfR = combinedRadius / 2;
	
	// Handle degenerate case where distance is extremely small
    if (approxDist < 1) 
	{
		// Push objects apart along X axis
        ai->player.gsObjectCoord.coord.t[0] += halfR;
        player1.gsObjectCoord.coord.t[0] -= halfR;
    } 
	else 
	{
		// Compute midpoint between AI and player
        midX = (ai->player.gsObjectCoord.coord.t[0] + player1.gsObjectCoord.coord.t[0]) / 2;
        midZ = (ai->player.gsObjectCoord.coord.t[2] + player1.gsObjectCoord.coord.t[2]) / 2;
		
		// Separate AI from midpoint
        ai->player.gsObjectCoord.coord.t[0] = midX + ((long)dx * halfR) / approxDist;
        ai->player.gsObjectCoord.coord.t[2] = midZ + ((long)dz * halfR) / approxDist;
		
		// Separate player from midpoint in opposite direction
        player1.gsObjectCoord.coord.t[0] = midX - ((long)dx * halfR) / approxDist;
        player1.gsObjectCoord.coord.t[2] = midZ - ((long)dz * halfR) / approxDist;
    }

	// Force transform updates after positional correction
    ai->player.gsObjectCoord.flg = 0;
    player1.gsObjectCoord.flg = 0;

	// Swap forward speeds to simulate impact momentum transfer
    tmp = ai->player.speed;
    ai->player.speed = player1.speed;
    player1.speed = tmp;
	
	// Clear lateral drift from collision response
    ai->lateralSpeed = 0;
    player1_lateralSpeed = 0;
}


// Returns 1 if the corner at the upcoming waypoint exceeds 45 degrees (used for predictive braking)
static int IsSharpCorner(AIRacer *ai) {
    
	// Index of the next waypoint
	int wpNext = (ai->waypointIndex + 1) % NUM_WAYPOINTS;
    
	const Waypoint *wp = &waypoints[ai->waypointIndex];
    const Waypoint *wpN = &waypoints[wpNext];
    
	// Vector from AI to current waypoint
	long d1x = (wp->x - ai->player.gsObjectCoord.coord.t[0]) >> 5;
    long d1z = (wp->z - ai->player.gsObjectCoord.coord.t[2]) >> 5;
    
	// Vector from current waypoint to next waypoint
	long d2x = (wpN->x - wp->x) >> 5;
    long d2z = (wpN->z - wp->z) >> 5;
	
	// Cross product indicates how sharply the path turns
    long cross = (d1z * d2x) - (d1x * d2z);
	
	// Dot product measures how aligned the two segments are
    long dot = (d1x * d2x) + (d1z * d2z);
	
    long absCross = cross < 0 ? -cross : cross;
    long absDot = dot < 0 ? -dot : dot;
	
	// If perpendicular component dominates alignment, corner is sharp
    return absCross > absDot;
}


// Main AI update loop (this function drives all AI racers each frame)
void UpdateAIRacers(void) {
    VehicleAttributes *savedVehicle;
    SuspensionConfig *savedSuspension;
    long savedRoll, savedPitch;
    int savedGear;
    GearboxMode savedGearMode;
	
    int i, steerWpIdx, steer, isBraking, movDir;
    long dX, dZ, absDX, absDZ;
    AIRacer *ai;

	// Save current player state so AI simulation can reuse it safely
    savedVehicle = activeVehicle;
    savedSuspension = activeSuspension;
    savedRoll = player1_roll;
    savedPitch = player1_pitch;
    savedGear = currentGear;
    savedGearMode = gearboxMode;
    
	// Force automatic gearbox for AI control
	gearboxMode = GEAR_AUTO;
	
	// Global flag: at least one AI is braking this frame
    aiRacerBraking = 0;

	// Update each AI racer individually
    for (i = 0; i < NUM_AI_RACERS; i++)
	{
        ai = &aiRacers[i];

		// Inject AI-specific vehicle setup into shared physics system
        activeVehicle = ai->attribs;
        activeSuspension = ai->suspension;
        player1_roll = ai->roll;
        player1_pitch = ai->pitch;
        currentGear = ai->gear;

		// Move toward current waypoint (and advance if needed)
        AdvanceWaypoint(ai, &dX, &dZ);

        absDX = dX < 0 ? -dX : dX;
        absDZ = dZ < 0 ? -dZ : dZ;
		
		// Lookahead waypoint adjustment to improve corner entry
        steerWpIdx = ai->waypointIndex;
		
        if (absDX < LOOKAHEAD_DIST && absDZ < LOOKAHEAD_DIST) 
		{
            steerWpIdx = (steerWpIdx + 1) % NUM_WAYPOINTS;
        }

		// Compute ideal steering toward chosen waypoint
        steer = ComputeSteering(&ai->player.gsObjectCoord, waypoints[steerWpIdx].x, waypoints[steerWpIdx].z);

        // Smooth steering (snap immediately when more steering is needed)
        {
            int absTarget = steer < 0 ? -steer : steer;
            int absCurrent = ai->steerSmooth < 0 ? -ai->steerSmooth : ai->steerSmooth;
            
			if (absTarget >= absCurrent) 
			{
                ai->steerSmooth = steer;
            } 
			else 
			{
                if (ai->steerSmooth < steer) 
				{
                    ai->steerSmooth += 5;
                    if (ai->steerSmooth > steer) 
					{
						ai->steerSmooth = steer;
					}
                } else if (ai->steerSmooth > steer) 
				{
                    ai->steerSmooth -= 5;
					
                    if (ai->steerSmooth < steer) 
					{
						ai->steerSmooth = steer;
					}
                }
            }
        }
		
        steer = ai->steerSmooth;

        // Braking (react to current steer and look ahead to brake before sharp corners)
        {
            int predictBrake = (absDX < LOOKAHEAD_DIST * 3 && absDZ < LOOKAHEAD_DIST * 3) && IsSharpCorner(ai);
            isBraking = ((steer != 0 || predictBrake) && ai->player.speed > BRAKE_SPEED) ? 1 : 0;
        }
		
        if (isBraking) 
		{
			aiRacerBraking = 1;
		}

        // Only attempt to overtake at racing speed (at race start cars are bunched)
        if (ai->player.speed > BRAKE_SPEED) 
		{
            int avoid = ComputeAvoidance(ai, i);
            if (avoid != 0) 
			{
                steer = avoid;
				
				// Prevent steering system fighting avoidance logic
                ai->steerSmooth = avoid;  
            }
        }

		// Movement direction
        movDir = isBraking ? 0 : 1;

		// Update gearbox based on current speed and max vehicle capability
        UpdateGearbox(ai->player.speed, (long)ai->attribs->maxSpeed, movDir);
        
		ai->gear = currentGear;

		// Apply steering rotation and physics integration
        RotateModel(&ai->player.gsObjectCoord, &ai->player.rotation, 0, steer, 0, &ai->player.speed, &ai->lateralSpeed);

		// Move the AI racer
        AdvanceModel(&ai->player.gsObjectCoord, &ai->player.rotation, &ai->player.speed, &ai->lateralSpeed, movDir, isBraking);

		// Collision handling (world + player interaction)
        CheckWorldCollisions(&ai->player, &ai->lateralSpeed);
        CheckAIPlayerCollision(ai);

		// Store resulting physics state back into AI structure
        ai->roll = player1_roll;
        ai->pitch = player1_pitch;
    }

	// Resolve AI to AI collisions after all individual updates
    CheckAIAICollisions();

	// Restore original player physics state
    activeVehicle = savedVehicle;
    activeSuspension = savedSuspension;
    player1_roll = savedRoll;
    player1_pitch = savedPitch;
    currentGear = savedGear;
    gearboxMode = savedGearMode;
}


// Renders all AI racers that are within the camera/player draw distance
void DrawAIRacers(PlayerStruct *cameraPlayer, GsOT *ot) {
    int i;
	
	// Iterate through all AI racers
    for (i = 0; i < NUM_AI_RACERS; i++) 
	{
		
		// Perform proximity check against the camera/player
        // If the racer is close enough, it is considered visible
        if (IsObjectNearPlayer(cameraPlayer, &aiRacers[i].player.gsObjectCoord)) 
		{
			// Submit AI racer for rendering into the ordering table
            DrawPlayer(&aiRacers[i].player, ot);
        }
    }
}