#include <libps.h>
#include "../engine/calculations.h"
#include "../engine/state_manager.h"
#include "../engine/graphics.h"
#include "../engine/model.h"
#include "game.h"
#include "ground.h"
#include "world.h"
#include "player.h"
#include "ai_racer.h"
#include "ai_drivers.h"
#include "link_race.h"
#include "gear.h"
#include "vehicle_colour.h"

#define LOOKAHEAD_DIST 1800
#define ADVANCE_DIST 600
#define BRAKE_SPEED 150      // The usual speed the AI brakes for corners above (each driver has their own)
#define AVOID_DIST 2500      // The usual distance the AI steers around cars (each driver has their own, never more than this)

// Trigger-detection probe distances
// Two probe points are projected ahead-left and ahead-right of the car
// When a probe lands off-track, the car steers away from that side
#define WALL_PROBE_FWD 800   			// Units ahead of car centre
#define WALL_PROBE_SIDE 450   			// Lateral offset of each probe

// Maximum waypoints caught up in a single frame after going off-track
// Prevents a skipped run of waypoints from stalling the index for several seconds.
#define MAX_WP_SKIP 10  				// Max waypoints caught up per frame after going off-track
#define RACE_PROGRESS_LOOKAHEAD 15  	// Segments scanned ahead when computing race-distance score

// Forward window for the scan-based race segment tracker (independent of navigation)
#define RACE_SEG_SEARCH_AHEAD 20
// Maximum perpendicular distance (world units) to accept a segment as valid
// Prevents false jumps when a distant parallel segment has T in [0, 1023]
#define RACE_SEG_MAX_PERP 3000


AIRacer aiRacers[NUM_AI_RACERS];
int aiRacerBraking = 0;

// Player race-progress state — updated by UpdatePlayerRaceProgress each frame
int playerWaypointIndex = 1;
int playerLapCount = 0;
int playerRacePosition = 1;
int playerWaypointsSinceLap = 0;

// Player race-segment tracker — scan-based, independent of the navigation system
int playerRaceSegIdx = 1;
int playerRaceLapCount = 0;
int playerRaceSegDistSinceLap = 0;

// Where the player finished (0 while still racing)
int playerFinishPlace = 0;

// Finishing the race (each car is numbered 0 = player 1, 1 + i = AI racer i)
#define NUM_RACE_CARS (1 + NUM_AI_RACERS)
static int finishedCount = 0;                   // The cars that have finished so far
static long raceProgress[NUM_RACE_CARS];        // Each cars race progress (from the last UpdateRacePositions)

// Returns the index of the waypoint nearest to (px, pz) using Manhattan distance,
// then adds 1 so the result is the NEXT waypoint the vehicle should target
// Used once at spawn to align waypointIndex with physical grid position
static int FindNearestWaypoint(long px, long pz)
{
    int i, best = 0;
    long bestDist = 0x7FFFFFFF;
    for (i = 0; i < numTrackWaypoints; i++)
    {
        long dx = trackWaypoints[i].x - px;
        long dz = trackWaypoints[i].z - pz;
        long dist = (dx < 0 ? -dx : dx) + (dz < 0 ? -dz : dz);
        if (dist < bestDist) 
		{ 
			bestDist = dist; best = i; 
		}
    }
    return (best + 1) % numTrackWaypoints;
}


// Returns a continuous race-distance score for ordering all vehicles
//
// score = lapCount * numTrackWaypoints * 1024 + waypointIndex * 1024 + segmentT
//
// segmentT is the dot-product projection of the vehicle onto the segment
// from the previous waypoint to waypointIndex, giving sub-waypoint accuracy
// so two vehicles on the same segment are ordered by exact track position.
//
// Arithmetic: vectors are >>3 before squaring to keep products inside 32-bit
// dot is clamped to [0, lenSq] before the *1024 multiply to prevent overflow
// when a vehicle is far off-course.
static long ComputeRaceProgress(long px, long pz, int waypointIndex, int lapCount)
{
    int prevWp = (waypointIndex - 1 + numTrackWaypoints) % numTrackWaypoints;
    long segDX = (trackWaypoints[waypointIndex].x - trackWaypoints[prevWp].x) >> 3;
    long segDZ = (trackWaypoints[waypointIndex].z - trackWaypoints[prevWp].z) >> 3;
    long toDX = (px - trackWaypoints[prevWp].x) >> 3;
    long toDZ = (pz - trackWaypoints[prevWp].z) >> 3;
    long dot = segDX * toDX + segDZ * toDZ;
    long lenSq = segDX * segDX + segDZ * segDZ;
    long t;

    if (dot < 0)     
	{
		dot = 0;
	}
	
    if (dot > lenSq) 
	{
		dot = lenSq;
	}
	
    t = (lenSq > 0) ? (dot * 1024L / lenSq) : 0;

    return (long)lapCount * ((long)numTrackWaypoints * 1024L) + (long)waypointIndex * 1024L + t;
}


// Finds the segment the vehicle is actually on by scanning RACE_PROGRESS_LOOKAHEAD
// segments ahead of the tracked index.  For each candidate segment it computes
// the projection T without clamping; T in [0, 1023] means the car is on that
// segment.  Returns the highest-scoring match so that skipped waypoints (due to
// going off-track) don't drop the vehicle's race-distance score
// Falls back to the tracked index with T=0 if no segment matches
static long ComputeBestRaceProgress(long px, long pz, int trackedWpIdx, int lapCount)
{
    long best = (long)lapCount * ((long)numTrackWaypoints * 1024L) + (long)trackedWpIdx * 1024L;
    int i;

    for (i = 0; i < RACE_PROGRESS_LOOKAHEAD; i++)
    {
        int wpRaw = trackedWpIdx + i;
        int wp = wpRaw % numTrackWaypoints;
        int prev = (wp - 1 + numTrackWaypoints) % numTrackWaypoints;
        int lc = (wpRaw >= numTrackWaypoints) ? lapCount + 1 : lapCount;
        long segDX = (trackWaypoints[wp].x - trackWaypoints[prev].x) >> 3;
        long segDZ = (trackWaypoints[wp].z - trackWaypoints[prev].z) >> 3;
        long toDX = (px - trackWaypoints[prev].x) >> 3;
        long toDZ = (pz - trackWaypoints[prev].z) >> 3;
        long dot = segDX * toDX + segDZ * toDZ;
        long lenSq = segDX * segDX + segDZ * segDZ;
        long t, prog;

        if (lenSq <= 0) 
		{
			continue;
		}

        t = dot * 1024L / lenSq;

        // Only claim this segment if the car is genuinely on it
        if (t < 0 || t > 1023) 
		{
			continue;
		}

        prog = (long)lc * ((long)numTrackWaypoints * 1024L)
            + (long)wp * 1024L
            + t;

        if (prog > best) 
		{
			best = prog;
		}
    }

    return best;
}


// Scans forward up to RACE_SEG_SEARCH_AHEAD segments from currentSeg
// Returns the offset of the furthest-forward segment the car is genuinely on
// (projection T in [0, 1023]).  Returns 0 when the car is still on the
// current segment (no advancement).  *outT receives the winning T value
// NOTE: only used by the race-progress tracker — navigation is unchanged
static int FindRaceSegOffset(long px, long pz, int currentSeg, long *outT)
{
    int bestOffset = 0;
    long bestT = 0;
    int i;

    for (i = 0; i < RACE_SEG_SEARCH_AHEAD; i++)
    {
        int wp = (currentSeg + i) % numTrackWaypoints;
        int prev = (wp - 1 + numTrackWaypoints) % numTrackWaypoints;
        long segDX = (trackWaypoints[wp].x - trackWaypoints[prev].x) >> 3;
        long segDZ = (trackWaypoints[wp].z - trackWaypoints[prev].z) >> 3;
        long toDX = (px - trackWaypoints[prev].x) >> 3;
        long toDZ = (pz - trackWaypoints[prev].z) >> 3;
        long dot = segDX * toDX + segDZ * toDZ;
        long lenSq = segDX * segDX + segDZ * segDZ;
        long t;

        if (lenSq <= 0) 
		{
			continue;
		}
		
        t = dot * 1024L / lenSq;
		
		
        if (t < 0 || t > 1023) 
		{
			continue;
		}

        // Reject segments the car is not physically near (catches distant parallel
        // segments that share the same T range due to track geometry)
        {
            long origDX = trackWaypoints[wp].x - trackWaypoints[prev].x;
            long origDZ = trackWaypoints[wp].z - trackWaypoints[prev].z;
            long projX = trackWaypoints[prev].x + t * origDX / 1024L;
            long projZ = trackWaypoints[prev].z + t * origDZ / 1024L;
            long perpX = px - projX;
            long perpZ = pz - projZ;
            long perpMag = (perpX < 0 ? -perpX : perpX) + (perpZ < 0 ? -perpZ : perpZ);
			
            if (perpMag > RACE_SEG_MAX_PERP) 
			{
				continue;
			}
        }

        bestOffset = i;
        bestT = t;
        // keep iterating — want the furthest-forward valid segment
    }

    *outT = bestT;
    return bestOffset;
}


// Advance the scan-based race-segment tracker.  Call once per frame per vehicle,
// AFTER the vehicle has moved.  Does not affect waypointIndex or navigation
static void UpdateRaceSegTracker(long px, long pz, int *segIdx, int *lapCount, int *distSinceLap)
{
    long t;
    int offset = FindRaceSegOffset(px, pz, *segIdx, &t);
    int s;

    for (s = 1; s <= offset; s++)
    {
        int seg = (*segIdx + s) % numTrackWaypoints;
        (*distSinceLap)++;
        if (seg == 0)
        {
            if (*distSinceLap >= numTrackWaypoints * 3 / 4)
			{
                (*lapCount)++;
			}
			
            *distSinceLap = 0;
		}
    }

    if (offset > 0)
	{
        *segIdx = (*segIdx + offset) % numTrackWaypoints;
	}
}


// Grid start positions — two-column staggered formation, spaced > AVOID_DIST
// so avoidance does not fire between adjacent cars at race start

// Per-racer vehicle data: cycle through the three available models
static const long aiTmdAddr[NUM_AI_RACERS] = {
    CAR_3Y_MEM_ADDR,
    CAR_2B_MEM_ADDR,
    CAR_5G_MEM_ADDR,
    CAR_3Y_MEM_ADDR,
    CAR_2B_MEM_ADDR,
};

// Some of the AI-specific attributes re +20% vs player equivalents but speed and 
// acceleration are unchanged.  The AI cars are more grippy and responsive, and brake better.
static VehicleAttributes aiCar3Attribs = {
    310,   // maxSpeed
    12,    // acceleration
    3,     // deceleration
    8,     // brakeDeceleration    3   * 1.2 = 3.6 → 4
    -100,  // maxReverseSpeed
    200,   // steeringResponse     160 * 1.2 = 192
    160,   // minSteeringResponse  110 * 1.2 = 132
    180,   // maxGrip              150 * 1.2 = 180
    140,   // minGrip              50  * 1.2 = 60
    630,   // slidingFriction      780 * 0.8 = 624
    710,   // lateralFriction      900 * 0.8 = 720
    2048,  // turnRadiusFactor
    20     // understeerBias
};

static VehicleAttributes aiCar2Attribs = {
    330,   // maxSpeed
    12,    // acceleration
    3,     // deceleration
    8,     // brakeDeceleration    3   * 1.2 = 3.6 → 4
    -90,   // maxReverseSpeed
    200,   // steeringResponse     150 * 1.2 = 180
    160,   // minSteeringResponse  100 * 1.2 = 120
    180,   // maxGrip              130 * 1.2 = 156
    120,   // minGrip              30  * 1.2 = 36
    630,   // slidingFriction      750 * 0.8 = 600
    710,   // lateralFriction      880 * 0.8 = 704
    2048,  // turnRadiusFactor
    -10    // understeerBias
};

static VehicleAttributes aiCar5Attribs = {
    360,   // maxSpeed
    12,    // acceleration
    3,     // deceleration
    8,     // brakeDeceleration    2   * 1.2 = 2.4 → 3
    -80,   // maxReverseSpeed
    200,   // steeringResponse     110 * 1.2 = 132
    160,   // minSteeringResponse  90  * 1.2 = 108
    180,   // maxGrip              100 * 1.2 = 120
    120,   // minGrip              20  * 1.2 = 24
    630,   // slidingFriction      700 * 0.8 = 560
    710,   // lateralFriction      850 * 0.8 = 680
    2048,  // turnRadiusFactor
    -20    // understeerBias
};

static VehicleAttributes * const aiVehicleAttribs[NUM_AI_RACERS] = {
    &aiCar3Attribs,
    &aiCar2Attribs,
    &aiCar5Attribs,
    &aiCar3Attribs,
    &aiCar2Attribs,
};

static SuspensionConfig * const aiVehicleSuspensions[NUM_AI_RACERS] = {
    &car3Suspension,
    &car2Suspension,
    &car5Suspension,
    &car3Suspension,
    &car2Suspension,
};


// The AI racers usual vehicle (model and colour)
static const int aiVehicleIndex[NUM_AI_RACERS] = {
    2,      // car3 yellow
    6,      // car2 blue
    11,     // car5 green
    1,      // car3 red
    9,      // car2 white
};

// The AI racers vehicle (model and colour)
static int AIVehicle(int racer, const int vehicles[NUM_AI_RACERS])
{
    int model = aiVehicleIndex[racer] / 5;
    int variant = aiVehicleIndex[racer] % 5;
    int tries, j, taken;

    for (tries = 0; tries < 5; tries++)
    {
        int v = model * 5 + (variant + tries) % 5;

        taken = (v == selectedVehicleIndex);
        for (j = 0; j < racer && !taken; j++)
        {
            if (vehicles[j] == v)
            {
                taken = 1;
            }
        }
        if (!taken)
        {
            return v;
        }
    }
    return aiVehicleIndex[racer];
}


// Every AI racers vehicle (model and colour) for the players selected vehicle
void ResolveAIVehicles(int vehicles[NUM_AI_RACERS])
{
    int i;

    for (i = 0; i < NUM_AI_RACERS; i++)
    {
        vehicles[i] = AIVehicle(i, vehicles);
    }
}


// A percentage of a value
static int Percent(int value, int pct)
{
    return value * pct / 100;
}


// An AI racers handling (its cars AI handling, changed by its drivers personality)
// A driver with a form spread is a little faster or slower each race
static void TuneVehicle(AIRacer *ai, const VehicleAttributes *base, const DriverProfile *driver)
{
    int speedPct = driver->speedPct;

    if (driver->formSpread > 0)
    {
        speedPct += RandomNumber() % (2 * driver->formSpread + 1) - driver->formSpread;
    }

    ai->tuned = *base;
    ai->tuned.maxSpeed            = Percent(base->maxSpeed, speedPct);
    ai->tuned.acceleration        = base->acceleration + driver->accelAdj;
    ai->tuned.brakeDeceleration   = base->brakeDeceleration + driver->brakeDecelAdj;
    ai->tuned.maxGrip             = Percent(base->maxGrip, driver->gripPct);
    ai->tuned.minGrip             = Percent(base->minGrip, driver->gripPct);
    ai->tuned.steeringResponse    = Percent(base->steeringResponse, driver->steerPct);
    ai->tuned.minSteeringResponse = Percent(base->minSteeringResponse, driver->steerPct);
    ai->attribs = &ai->tuned;
}


// Initialise all AI-controlled racers at their starting positions and reset their runtime state
// Each AI racer uses the same vehicle throughout the tournament
void InitialiseAIRacers(const int *lockedVehicles) {
    int i;
    int vehicles[NUM_AI_RACERS];

    // The drivers form and mistakes are different each race
    SeedRandom((unsigned long)frameNumber * 2654435761UL + (unsigned long)gameMs);

    if (lockedVehicles)
    {
        for (i = 0; i < NUM_AI_RACERS; i++)
        {
            vehicles[i] = lockedVehicles[i];
        }
    }
    else
    {
        ResolveAIVehicles(vehicles);
    }

	// Create and configure each AI racer
    for (i = 0; i < NUM_AI_RACERS; i++) 
	{
		// Initialise the AI racer at its start position on the track (and facing its heading)
        const StartDef *start = &trackStarts[START_AI + i];
        InitialisePlayer(&aiRacers[i].player, 0, start->x, CAR_GROUND_Y, start->z, (unsigned long *)aiTmdAddr[i]);
        SetPlayerHeading(&aiRacers[i].player, start->heading);

		// Its own colour and brake lights (paint slots 2 onwards: 0 and 1 are the players)
        aiRacers[i].vehicleIndex = vehicles[i];
        PaintCar(&aiRacers[i].player, 2 + i, aiRacers[i].vehicleIndex);

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
		
		// Begin navigation at the waypoint nearest to this cars grid position
        aiRacers[i].waypointIndex = FindNearestWaypoint(start->x, start->z);
        aiRacers[i].lapCount = 0;
        aiRacers[i].waypointsSinceLap = 0;
        aiRacers[i].racePosition = 1;
        aiRacers[i].finishPlace = 0;

        // Race-segment tracker starts at the same position as navigation
        aiRacers[i].raceSegIdx = aiRacers[i].waypointIndex;
        aiRacers[i].raceLapCount = 0;
        aiRacers[i].raceSegDistSinceLap = 0;

		// Clear steering interpolation/smoothing value
        aiRacers[i].steerSmooth = 0;
        aiRacers[i].recoverTimer = 0;
        aiRacers[i].braking = 0;

        // Stagger stuck checks so all three racers dont fire the same frame
        aiRacers[i].stuckTimer = 60 + i * 20;
        aiRacers[i].lastPosX = start->x;
        aiRacers[i].lastPosZ = start->z;
		
		// No mistake decided yet
        aiRacers[i].mistakeWaypoint = -1;
        aiRacers[i].mistake = 0;

		// Assign vehicle-specific handling (changed by its drivers personality) and suspension settings
        TuneVehicle(&aiRacers[i], aiVehicleAttribs[i], &aiDrivers[i]);
        aiRacers[i].suspension = aiVehicleSuspensions[i];
    }
}


// Avoidance — steer around any car (AI or player) directly ahead within the drivers avoid distance
// (a shorter distance passes closer). Uses racer index to break ties (even racers prefer right, odd prefer left)
static int ComputeAvoidance(AIRacer *ai, int racerIdx) {

	// Forward direction vector of this AI racer
	long fwdX = ai->player.gsObjectCoord.coord.m[0][2];
    long fwdZ = ai->player.gsObjectCoord.coord.m[2][2];
    long avoidDist = aiDrivers[racerIdx].avoidDist;

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
        if (absDX > avoidDist || absDZ > avoidDist)
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

    // Check the player — but only if they are not racing alongside us in the same direction.
    // A player overtaking from behind will have a forward vector aligned with ours (alignDot > 0)
    // and be moving at racing speed, so we leave them alone. A stopped or spun player is an
    // obstacle and should be avoided.
    {
        long playerFwdX = player1.gsObjectCoord.coord.m[0][2];
        long playerFwdZ = player1.gsObjectCoord.coord.m[2][2];
        long alignDot = (fwdX * (playerFwdX >> 6)) + (fwdZ * (playerFwdZ >> 6));
        int playerIsRacing = (alignDot > 0) && (player1.speed > BRAKE_SPEED / 2);

        if (!playerIsRacing) {
            dx = player1.gsObjectCoord.coord.t[0] - ai->player.gsObjectCoord.coord.t[0];
            dz = player1.gsObjectCoord.coord.t[2] - ai->player.gsObjectCoord.coord.t[2];
            dot = (fwdX * (dx >> 5)) + (fwdZ * (dz >> 5));
            if (dot > 0) {
                absDX = dx < 0 ? -dx : dx;
                absDZ = dz < 0 ? -dz : dz;
                if (absDX <= avoidDist && absDZ <= avoidDist) {
                    cross = (fwdZ * (dx >> 5)) - (fwdX * (dz >> 5));
                    
					if (cross > 0) 
					{
						return -38;
					}
	
                    if (cross < 0) 
					{
						return  38;
					}

                    return (racerIdx % 2 == 0) ? 38 : -38;
                }
            }
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
    absDot = dot < 0 ? -dot : dot;

	// Deadband: ignore errors under ~7 degrees so the car holds its heading
	// on straights instead of hunting left/right around small misalignments.
    if (absCross * 8 < absDot)
	{
		return 0;
	}
	
	// Small steering error (~7-14 degrees)
    if (absCross * 4 < absDot)
	{
		mag = 14;
	}
	
	// Medium steering error (~14-37 degrees)
    else if (absCross * 4 < absDot * 3)
	{
		mag = 26;
	}
	
	// Large steering error (>37 degrees)
    else
	{
		mag = 38;
	}

    return dir * mag;
}


// Returns 1 when position (px,pz) has passed waypoint wpIdx
static int PassedWaypoint(long px, long pz, int wpIdx)
{
    int prev = (wpIdx - 1 + numTrackWaypoints) % numTrackWaypoints;
    long dX = trackWaypoints[wpIdx].x - px;
    long dZ = trackWaypoints[wpIdx].z - pz;
    long absDX = dX < 0 ? -dX : dX;
    long absDZ = dZ < 0 ? -dZ : dZ;
    long sX = (trackWaypoints[wpIdx].x - trackWaypoints[prev].x) >> 5;
    long sZ = (trackWaypoints[wpIdx].z - trackWaypoints[prev].z) >> 5;
    return (absDX < ADVANCE_DIST && absDZ < ADVANCE_DIST) || (sX * dX + sZ * dZ < 0);
}

static void AdvanceWaypoint(AIRacer *ai, long *dX, long *dZ)
{
    long px = ai->player.gsObjectCoord.coord.t[0];
    long pz = ai->player.gsObjectCoord.coord.t[2];

    *dX = trackWaypoints[ai->waypointIndex].x - px;
    *dZ = trackWaypoints[ai->waypointIndex].z - pz;

    {
        int k;
        for (k = 0; k < MAX_WP_SKIP; k++)
        {
            if (!PassedWaypoint(px, pz, ai->waypointIndex))
			{
                break;
			}

            {
                int nextWp = (ai->waypointIndex + 1) % numTrackWaypoints;
                ai->waypointsSinceLap++;
                if (nextWp == 0)
                {
                    if (ai->waypointsSinceLap >= (numTrackWaypoints * 3 / 4))
					{
                        ai->lapCount++;
					}
					
                    ai->waypointsSinceLap = 0;
                }
                ai->waypointIndex = nextWp;
            }
        }
    }

    *dX = trackWaypoints[ai->waypointIndex].x - px;
    *dZ = trackWaypoints[ai->waypointIndex].z - pz;
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
	int wpNext = (ai->waypointIndex + 1) % numTrackWaypoints;
    
	const Waypoint *wp = &trackWaypoints[ai->waypointIndex];
    const Waypoint *wpN = &trackWaypoints[wpNext];
    
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


// Called when the position-based stuck detector fires.
// Kicks speed so the car has momentum to steer away from walls.
static void RecoverFromStuck(AIRacer *ai)
{
    ai->player.speed = -30;
    ai->lateralSpeed = 0;
    ai->steerSmooth  = 0;
    ai->recoverTimer = 35;
    ai->lastPosX     = ai->player.gsObjectCoord.coord.t[0];
    ai->lastPosZ     = ai->player.gsObjectCoord.coord.t[2];
    ai->stuckTimer   = 60;
}


// Projects two probe points ahead-left and ahead-right of the car using the
// cars own transform axes (4096 = 1.0 fixed-point)
// GetTerrainType checks the ground tile at each probe position and anything
// other than TERRAIN_TRACK means a boundary is about to be crossed
// Returns a steering correction: +38 (steer right), -38 (steer left), 0 (clear)
static int SenseWalls(AIRacer *ai)
{
    long px   = ai->player.gsObjectCoord.coord.t[0];
    long pz   = ai->player.gsObjectCoord.coord.t[2];
    long fwdX = (long)ai->player.gsObjectCoord.coord.m[0][2];
    long fwdZ = (long)ai->player.gsObjectCoord.coord.m[2][2];
    long rgtX = (long)ai->player.gsObjectCoord.coord.m[0][0];
    long rgtZ = (long)ai->player.gsObjectCoord.coord.m[2][0];

    long aX = px + ((fwdX * WALL_PROBE_FWD)  >> 12);
    long aZ = pz + ((fwdZ * WALL_PROBE_FWD)  >> 12);

    int leftOff  = (GetTerrainType(aX - ((rgtX * WALL_PROBE_SIDE) >> 12), aZ - ((rgtZ * WALL_PROBE_SIDE) >> 12)) != TERRAIN_TRACK);
    int rightOff = (GetTerrainType(aX + ((rgtX * WALL_PROBE_SIDE) >> 12), aZ + ((rgtZ * WALL_PROBE_SIDE) >> 12)) != TERRAIN_TRACK);

    if (leftOff && !rightOff) return 38;
    if (rightOff && !leftOff)  return -38;
    if (leftOff && rightOff) return 38;
    return 0;
}


// Sets waypointIndex to match the nearest circuit segment so the initial
// race positions reflect the physical grid layout.
void InitialisePlayerRaceProgress(long startX, long startZ)
{
    playerWaypointIndex = FindNearestWaypoint(startX, startZ);
    playerLapCount = 0;
    playerRacePosition = 1;
    playerWaypointsSinceLap = 0;

    playerRaceSegIdx = playerWaypointIndex;
    playerRaceLapCount = 0;
    playerRaceSegDistSinceLap = 0;
    InitialiseWrongWay(0, startX, startZ);

    // Nobody has finished yet
    playerFinishPlace = 0;
    finishedCount = 0;
}


// Advances the players waypoint index exactly as the AI system does, but
// also requires the player to be on the track surface — grass shortcuts do
// not earn waypoint credit.
void UpdatePlayerRaceProgress(void)
{
    long px = player1.gsObjectCoord.coord.t[0];
    long pz = player1.gsObjectCoord.coord.t[2];

    // Loop up to MAX_WP_SKIP times so the index catches up in a single frame
    // if the player went off-track and re-entered past several waypoints.
    {
        int k;
        for (k = 0; k < MAX_WP_SKIP; k++)
        {
            if (!PassedWaypoint(px, pz, playerWaypointIndex))
			{
                break;
			}

            {
                int nextWp = (playerWaypointIndex + 1) % numTrackWaypoints;
                playerWaypointsSinceLap++;
                if (nextWp == 0)
                {
                    if (playerWaypointsSinceLap >= (numTrackWaypoints * 3 / 4))
					{
                        playerLapCount++;
					}
					
                    playerWaypointsSinceLap = 0;
                }
                playerWaypointIndex = nextWp;
            }
        }
    }

    // Update the scan-based race-segment tracker (used for race-position scoring)
    UpdateRaceSegTracker(px, pz, &playerRaceSegIdx, &playerRaceLapCount, &playerRaceSegDistSinceLap);

    // Whether player 1 is going the wrong way
    UpdateWrongWay(0, &player1);
}


// A cars finishing place (0 while still racing) and its laps
static int *CarFinishPlace(int car)
{
    return (car == 0) ? &playerFinishPlace : &aiRacers[car - 1].finishPlace;
}

static int CarLaps(int car)
{
    return (car == 0) ? playerRaceLapCount : aiRacers[car - 1].raceLapCount;
}


// Is car a ahead of car b in the race
// The finished cars are ahead (in the order they finished), then the cars still racing
// by their race progress (the same progress: the lower car number, so no two cars share a position)
static int IsAheadInRace(int a, int b)
{
    int finishA = *CarFinishPlace(a);
    int finishB = *CarFinishPlace(b);

    if (finishA || finishB)
    {
        if (finishA && finishB)
        {
            return finishA < finishB;
        }
        return finishA != 0;
    }

    if (raceProgress[a] != raceProgress[b])
    {
        return raceProgress[a] > raceProgress[b];
    }
    return a < b;
}


// The cars that have just finished their last lap get the next finishing places
// Cars finishing on the same frame: the one furthest past the line first
static void UpdateRaceFinishes(void)
{
    int finishing[NUM_RACE_CARS];
    int count = 0, car, i, j;

    for (car = 0; car < NUM_RACE_CARS; car++)
    {
        if (!*CarFinishPlace(car) && CarLaps(car) >= NUM_RACE_LAPS)
        {
            // Insert it after the cars further past the line
            for (j = count; j > 0 && raceProgress[car] > raceProgress[finishing[j - 1]]; j--)
            {
                finishing[j] = finishing[j - 1];
            }
            finishing[j] = car;
            count++;
        }
    }

    for (i = 0; i < count; i++)
    {
        *CarFinishPlace(finishing[i]) = ++finishedCount;
    }
}


// Computes a continuous race-distance score for every vehicle, records the cars
// that have finished, then ranks them by counting how many others are ahead
void UpdateRacePositions(void)
{
    int i, j;

    raceProgress[0] = ComputeRaceProgress(player1.gsObjectCoord.coord.t[0], player1.gsObjectCoord.coord.t[2], playerRaceSegIdx, playerRaceLapCount);

    for (i = 0; i < NUM_AI_RACERS; i++)
	{
        raceProgress[1 + i] = ComputeRaceProgress(
            aiRacers[i].player.gsObjectCoord.coord.t[0],
            aiRacers[i].player.gsObjectCoord.coord.t[2],
            aiRacers[i].raceSegIdx, aiRacers[i].raceLapCount);
	}

    UpdateRaceFinishes();

    // Player position: count the cars ahead of it
    playerRacePosition = 1;
    for (j = 1; j < NUM_RACE_CARS; j++)
	{
        if (IsAheadInRace(j, 0))
		{
            playerRacePosition++;
		}
	}

    // Each AI racers position: count the cars ahead of it
    for (i = 0; i < NUM_AI_RACERS; i++)
    {
        aiRacers[i].racePosition = 1;
        for (j = 0; j < NUM_RACE_CARS; j++)
		{
            if (j != (1 + i) && IsAheadInRace(j, 1 + i))
			{
                aiRacers[i].racePosition++;
			}
		}
    }
}


// The finishing order (the car in each position, 1st first: 0 = player 1, 1 + i = AI racer i)
// The finished cars in the order they finished, then the cars still racing where they are now
// (their progress from the last UpdateRacePositions, so it doesn't change once the race has stopped)
void GetRaceFinishOrder(int order[NUM_RACE_CARS])
{
    int car, j;

    for (car = 0; car < NUM_RACE_CARS; car++)
    {
        for (j = car; j > 0 && IsAheadInRace(car, order[j - 1]); j--)
        {
            order[j] = order[j - 1];
        }
        order[j] = car;
    }
}


void UpdateAIRacers(void) {

    VehicleAttributes *savedVehicle = activeVehicle;
    SuspensionConfig *savedSuspension = activeSuspension;
    long savedRoll = player1_roll;
    long savedPitch = player1_pitch;
    int savedGear = currentGear;
    GearboxMode savedGearMode = gearboxMode;

    int i;
    AIRacer *ai;

    gearboxMode = GEAR_AUTO;
    aiRacerBraking = 0;

    for (i = 0; i < NUM_AI_RACERS; i++)
    {
        int steerWpIdx, steer, isBraking, movDir;
        long dX, dZ, absDX, absDZ;
        long speedBefore;
        const DriverProfile *driver = &aiDrivers[i];

        ai = &aiRacers[i];

        activeVehicle = ai->attribs;
        activeSuspension = ai->suspension;
        player1_roll = ai->roll;
        player1_pitch = ai->pitch;
        currentGear = ai->gear;

        // Recovery or Navigation
        if (ai->recoverTimer > 0)
        {
            // Reverse away from the wall. SenseWalls detects which side the boundary is on.
            // Inverting the result rotates the rear of the car away from it as the car backs up. 
            // When both probes fire (head-on impact) steer=0 so the car reverses straight backwards.
            int ws = SenseWalls(ai);
            steer = (ws != 0) ? -ws : 0;
            ai->steerSmooth = steer;
            isBraking = 0;
            movDir = -1;
            ai->recoverTimer--;
        }
        else
        {
        // Navigation
        AdvanceWaypoint(ai, &dX, &dZ);
        absDX = dX < 0 ? -dX : dX;
        absDZ = dZ < 0 ? -dZ : dZ;

        // Always steer toward 2 waypoints ahead of the current navigation target
        // The extra lookahead means small heading deviations on straights dont produce a steering correction,
        // and corners are set up earlier so the car doesn't overshoot
        steerWpIdx = (ai->waypointIndex + 2) % numTrackWaypoints;

        steer = ComputeSteering(&ai->player.gsObjectCoord, trackWaypoints[steerWpIdx].x, trackWaypoints[steerWpIdx].z);

        // Smooth steering — always interpolate toward the target at the drivers rate
        // Bidirectional lerp prevents the discrete jumps in ComputeSteering output
        // (0 / ±14 / ±26 / ±38) from producing visible frame-to-frame twitching
        {
            if (ai->steerSmooth < steer)
			{
				ai->steerSmooth += driver->steerRate;
				if (ai->steerSmooth > steer)
				{
					ai->steerSmooth = steer;
				}
			}
			else if (ai->steerSmooth > steer)
			{
				ai->steerSmooth -= driver->steerRate;
				if (ai->steerSmooth < steer)
				{
					ai->steerSmooth = steer;
				}
			}
		}

        // A mistake (the driver misses the braking point for the corner at this waypoint)
        // Decided once for each waypoint, so it is the whole approach to the corner
        if (ai->mistakeWaypoint != ai->waypointIndex)
        {
            ai->mistakeWaypoint = ai->waypointIndex;
            ai->mistake = (driver->mistakeChance > 0) && ((RandomNumber() & 255) < driver->mistakeChance);
        }

        // Brake for medium and sharp corners (how hard it steers, the drivers corner steer) and also
        // predict-brake before the next waypoint when it requires a sharp turn (not when it is making
        // A mistake (then it only brakes once it is in the corner). Only above the drivers brake speed)
        {
            int absRaw = steer < 0 ? -steer : steer;
            int predictBrake = (absDX < LOOKAHEAD_DIST * 3 && absDZ < LOOKAHEAD_DIST * 3) && !ai->mistake && IsSharpCorner(ai);
            isBraking = ((absRaw >= driver->cornerBrakeSteer || predictBrake) && ai->player.speed > driver->brakeSpeed) ? 1 : 0;
        }
        ai->braking = isBraking;

        steer = ai->steerSmooth;
        if (isBraking) 
		{
			aiRacerBraking = 1;
		}

        // Avoidance: steer around any car ahead (AI or stopped player)
        if (ai->player.speed > BRAKE_SPEED)
        {
            int avoid = ComputeAvoidance(ai, i);
            if (avoid != 0)
			{
                steer = avoid;
			}
        }

        // Wall sensing: nudges the steering target when a track boundary is detected ahead. 
        // Does not bypass steerSmooth so the correction blends in via the lerp rather than snapping instantly.
        {
            int wallSteer = SenseWalls(ai);
            if (wallSteer != 0)
			{
                steer = wallSteer;
			}
        }

        movDir = isBraking ? 0 : 1;
        }

        // Physics
        UpdateGearbox(ai->player.speed, (long)ai->attribs->maxSpeed, movDir);
        ai->gear = currentGear;

        RotateModel(&ai->player.gsObjectCoord, &ai->player.rotation, 0, steer, 0, &ai->player.speed, &ai->lateralSpeed, ai->attribs, ai->suspension);

        AdvanceModel(&ai->player.gsObjectCoord, &ai->player.rotation, &ai->player.speed, &ai->lateralSpeed, movDir, isBraking, ai->attribs, ai->suspension);

        {
            SVECTOR modelTilt;
            modelTilt.vx = (short)(3072 + (int)player1_pitch);
            modelTilt.vy = 2048;
            modelTilt.vz = (short)player1_roll;
            RotModel(&ai->player.gsModelCoord, &modelTilt, 0, 0, 0);
        }

        // Collision
        speedBefore = ai->player.speed;
        CheckWorldCollisions(&ai->player, &ai->lateralSpeed);
        CheckAIPlayerCollision(ai);

        if (ai->player.speed < 0 && speedBefore > 0)
        {
            // Wall hit (begin reverse recovery so the car backs away rather than accelerating straight back into the same wall next frame)
            ai->player.speed = -30;
            ai->lateralSpeed = 0;
            ai->recoverTimer = 35;
        }

        // Stuck recovery (Every 20 frames, measure net displacement)
        // Anything below 400 units means the car is oscillating against a wall. It should be doing ~1600 units of movement in 20 frames at racing speed.
        // When stuck, skip 2 waypoints so the new target is past the wall and kick the speed to 80 so steering has momentum to work with
        if (--ai->stuckTimer <= 0)
        {
            long dx = ai->player.gsObjectCoord.coord.t[0] - ai->lastPosX;
            long dz = ai->player.gsObjectCoord.coord.t[2] - ai->lastPosZ;
            long moved = (dx < 0 ? -dx : dx) + (dz < 0 ? -dz : dz);

            if (moved < 400)
			{
                RecoverFromStuck(ai);
			}
            else
            {
                ai->lastPosX = ai->player.gsObjectCoord.coord.t[0];
                ai->lastPosZ = ai->player.gsObjectCoord.coord.t[2];
                ai->stuckTimer = 20;
            }
        }

        ai->roll = player1_roll;
        ai->pitch = player1_pitch;

        // Update the scan-based race-segment tracker after the car has moved
        UpdateRaceSegTracker(
            ai->player.gsObjectCoord.coord.t[0],
            ai->player.gsObjectCoord.coord.t[2],
            &ai->raceSegIdx, &ai->raceLapCount, &ai->raceSegDistSinceLap);
    }

    CheckAIAICollisions();

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
        if (IsObjectInView(cameraPlayer, &aiRacers[i].player.gsObjectCoord, VIEW_AHEAD, VIEW_BEHIND, VIEW_SIDE)) 
		{
			// Submit AI racer for rendering into the ordering table
            DrawPlayer(&aiRacers[i].player, ot);
        }
    }
}


/*****************************************************
Link race: the other player's car (driven on the other console) is tracked like the player's
and the two players are ranked against each other (no AI racers)
*****************************************************/
static int opponentRaceSegIdx;
static int opponentRaceLapCount;
static int opponentRaceSegDistSinceLap;


void InitialiseOpponentRaceProgress(long startX, long startZ)
{
    opponentRaceSegIdx = FindNearestWaypoint(startX, startZ);
    opponentRaceLapCount = 0;
    opponentRaceSegDistSinceLap = 0;
    InitialiseWrongWay(1, startX, startZ);
}


// The other players laps (a link race or a two player race)
int OpponentRaceLapCount(void)
{
    return opponentRaceLapCount;
}


// Whether there are AI racers (only in a one player race, not a link race or a two player race)
int AIRacersActive(void)
{
    return !LinkRace_IsActive() && NumberOfPlayers == 1;
}


// Call once per frame with the other player's car position
void UpdateOpponentRaceProgress(long x, long z)
{
    UpdateRaceSegTracker(x, z, &opponentRaceSegIdx, &opponentRaceLapCount, &opponentRaceSegDistSinceLap);
}


/*****************************************************
Wrong way detection

Each players car has its own segment of the track (from a waypoint to the next one) 
These segments are used to measure how far the car has driven along the track, and whether it is facing the right way or not.
If the car is facing the wrong way and driving forwards, the distance driven along the segments is added to a counter. 
If the car is facing the right way and driving forwards, the distance driven along the segments is subtracted from the counter.
If the counter exceeds a threshold a "WRONG WAY" warning is displayed on the screen.
*****************************************************/
#define WRONG_WAY_DISTANCE    3600          // Distance driven back along the track before the warning (3 tiles)
#define WRONG_WAY_MIN_SPEED   20            // Driving forwards at least this fast (10 MPH)
#define WRONG_WAY_CLEAR       25            // Frames facing the right way that clear it (half a second)
#define WRONG_WAY_SEARCH      3             // The segments either side of the last one looked at
#define WRONG_WAY_LOST        (4000L >> 3)  // Further than this from them: look at them all (lost)

typedef struct {
    int seg;            // The segment the car is on (from waypoint seg - 1 to seg)
    long backDistance;  // Distance driven back along the track the wrong way (less any driven on)
    int rightFrames;    // Frames driving the right way since
    long lastX, lastZ;  // Where the car was the last frame
} WrongWayTracker;

static WrongWayTracker wrongWay[2];     // Player 1, player 2


// How far (squared, in 1/8ths) a point is from a segment (from waypoint seg - 1 to seg)
static long DistanceToSegmentSq(long px, long pz, int seg)
{
    int prev = (seg - 1 + numTrackWaypoints) % numTrackWaypoints;
    long ax = trackWaypoints[prev].x >> 3, az = trackWaypoints[prev].z >> 3;
    long segDX = (trackWaypoints[seg].x >> 3) - ax;
    long segDZ = (trackWaypoints[seg].z >> 3) - az;
    long toDX = (px >> 3) - ax;
    long toDZ = (pz >> 3) - az;
    long lenSq = segDX * segDX + segDZ * segDZ;
    long t = 0, dx, dz;

    if (lenSq > 0)
    {
        t = (segDX * toDX + segDZ * toDZ) / (lenSq >> 10 > 0 ? lenSq >> 10 : 1);
        if (t < 0) t = 0;
        if (t > 1024) t = 1024;
    }

    dx = toDX - segDX * t / 1024;
    dz = toDZ - segDZ * t / 1024;

    return dx * dx + dz * dz;
}


// The nearest segment to a point
static int NearestSegment(long px, long pz, int first, int count, long *distSq)
{
    int i, best = first;
    long bestDist = 0x7FFFFFFF;

    for (i = 0; i < count; i++)
    {
        int seg = (first + i + numTrackWaypoints) % numTrackWaypoints;
        long d = DistanceToSegmentSq(px, pz, seg);
        if (d < bestDist)
        {
            bestDist = d;
            best = seg;
        }
    }

    *distSq = bestDist;
    return best;
}


void InitialiseWrongWay(int car, long x, long z)
{
    long d;
    wrongWay[car].seg = (numTrackWaypoints >= 2) ? NearestSegment(x, z, 0, numTrackWaypoints, &d) : 0;
    wrongWay[car].backDistance = 0;
    wrongWay[car].rightFrames = 0;
    wrongWay[car].lastX = x;
    wrongWay[car].lastZ = z;
}


// The square root of a whole number (rounded down)
static long SquareRoot(long n)
{
    long x = n, y;

    if (n <= 0) 
    {
        return 0;
    }

    y = (x + 1) / 2;

    while (y < x)
    {
        x = y;
        y = (x + n / x) / 2;
    }

    return x;
}


// Call once per frame for each player's car (after it has moved)
void UpdateWrongWay(int car, PlayerStruct *player)
{
    WrongWayTracker *w = &wrongWay[car];
    long px = player->gsObjectCoord.coord.t[0];
    long pz = player->gsObjectCoord.coord.t[2];
    int forwards = player->speed > WRONG_WAY_MIN_SPEED;
    int facingWrong, seg;
    long distSq, along;

    if (numTrackWaypoints < 2) {
        return;
    }

    // The nearest segment of those around the last one (or of all of them, if the car is far from those)
    seg = NearestSegment(px, pz, w->seg - WRONG_WAY_SEARCH, WRONG_WAY_SEARCH * 2 + 1, &distSq);
    if (distSq > WRONG_WAY_LOST * WRONG_WAY_LOST)
    {
        // Lost: start again from where the car is, without counting it
        w->seg = NearestSegment(px, pz, 0, numTrackWaypoints, &distSq);
        w->lastX = px;
        w->lastZ = pz;
        return;
    }

    w->seg = seg;

    // The way the segment goes, whether the car faces against it (its forward direction is its
    // local Z), and how far the car has moved along it since the last frame (+ on, - back)
    {
        int prev = (seg - 1 + numTrackWaypoints) % numTrackWaypoints;
        long segDX = (trackWaypoints[seg].x - trackWaypoints[prev].x) >> 3;
        long segDZ = (trackWaypoints[seg].z - trackWaypoints[prev].z) >> 3;
        long len = SquareRoot(segDX * segDX + segDZ * segDZ);
        MATRIX *m = &player->gsObjectCoord.coord;

        facingWrong = (segDX * m->m[0][2] + segDZ * m->m[2][2]) < 0;
        along = (len > 0) ? ((px - w->lastX) * segDX + (pz - w->lastZ) * segDZ) / len : 0;
    }

    w->lastX = px;
    w->lastZ = pz;

    // Add up the distance driven back the wrong way (less any driven on)
    if (along < 0 && forwards && facingWrong)
    {
        w->backDistance -= along;
    }
        else if (along > 0)
    {
        w->backDistance -= along;
        if (w->backDistance < 0)
        {
            w->backDistance = 0;
        }
    }

    // Driving forwards the right way for a moment clears it
    if (forwards && !facingWrong)
    {
        if (++w->rightFrames >= WRONG_WAY_CLEAR)
        {
            w->backDistance = 0;
        }
    }
    else if (forwards)
    {
        w->rightFrames = 0;
    }
}


// Whether to show "WRONG WAY" for a players car (0 = player 1, 1 = player 2)
int IsWrongWay(int car)
{
    PlayerStruct *player = car ? &player2 : &player1;
    return wrongWay[car].backDistance >= WRONG_WAY_DISTANCE && player->speed > WRONG_WAY_MIN_SPEED;
}


// Link race positions: 1 or 2 (call after UpdatePlayerRaceProgress and UpdateOpponentRaceProgress)
void UpdateLinkRacePositions(long opponentX, long opponentZ)
{
    long playerProgress = ComputeRaceProgress(player1.gsObjectCoord.coord.t[0], player1.gsObjectCoord.coord.t[2], playerRaceSegIdx, playerRaceLapCount);
    long opponentProgress = ComputeRaceProgress(opponentX, opponentZ, opponentRaceSegIdx, opponentRaceLapCount);

    playerRacePosition = (opponentProgress > playerProgress) ? 2 : 1;
}
