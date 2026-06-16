#include <libps.h>
#include "ai_racer.h"
#include "player.h"
#include "game.h"
#include "gear.h"
#include "world.h"
#include "ground.h"
#include "../engine/model.h"

#define NUM_WAYPOINTS 146
#define LOOKAHEAD_DIST 1800
#define ADVANCE_DIST 600
#define BRAKE_SPEED 150
#define AVOID_DIST 2500

// Trigger-detection probe distances
// Two probe points are projected ahead-left and ahead-right of the car
// When a probe lands off-track, the car steers away from that side
#define WALL_PROBE_FWD 800   			// Units ahead of car centre
#define WALL_PROBE_SIDE 450   			// Lateral offset of each probe

// Maximum waypoints caught up in a single frame after going off-track
// Prevents a skipped run of waypoints from stalling the index for several seconds.
#define MAX_WP_SKIP 10  				// max waypoints caught up per frame after going off-track
#define RACE_PROGRESS_LOOKAHEAD 15  	// segments scanned ahead when computing race-distance score

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

typedef struct { long x; long z; } Waypoint;

static const Waypoint waypoints[NUM_WAYPOINTS] = {
    /*   0 */ {  3198, 10169 },
    /*   1 */ {  3174, 11658 },
    /*   2 */ {  3150, 13148 },
    /*   3 */ {  3095, 17108 },
    /*   4 */ {  3040, 21069 },
    /*   5 */ {  3040, 23332 },
    /*   6 */ {  3040, 25595 },
    /*   7 */ {  3040, 27093 },
    /*   8 */ {  3040, 28591 },
    /*   9 */ {  3213, 29545 },
    /*  10 */ {  3386, 30500 },
    /*  11 */ {  3822, 30926 },
    /*  12 */ {  4258, 31353 },
    /*  13 */ {  5513, 31533 },
    /*  14 */ {  6768, 31713 },
    /*  15 */ {  8995, 31760 },
    /*  16 */ { 11223, 31807 },
    /*  17 */ { 13783, 31807 },
    /*  18 */ { 16343, 31807 },
    /*  19 */ { 18642, 31807 },
    /*  20 */ { 20942, 31807 },
    /*  21 */ { 22338, 31807 },
    /*  22 */ { 23734, 31807 },
    /*  23 */ { 25119, 31807 },
    /*  24 */ { 26504, 31807 },
    /*  25 */ { 27212, 31807 },
    /*  26 */ { 27921, 31807 },
    /*  27 */ { 28485, 31807 },
    /*  28 */ { 29050, 31807 },
    /*  29 */ { 29620, 31807 },
    /*  30 */ { 30191, 31807 },
    /*  31 */ { 30839, 31289 },
    /*  32 */ { 31487, 30771 },
    /*  33 */ { 31701, 29645 },
    /*  34 */ { 31916, 28520 },
    /*  35 */ { 31767, 27604 },
    /*  36 */ { 31618, 26689 },
    /*  37 */ { 30725, 26173 },
    /*  38 */ { 29832, 25657 },
    /*  39 */ { 29802, 25657 },
    /*  40 */ { 29772, 25657 },
    /*  41 */ { 28445, 25667 },
    /*  42 */ { 27119, 25677 },
    /*  43 */ { 26466, 25680 },
    /*  44 */ { 25814, 25683 },
    /*  45 */ { 25246, 25685 },
    /*  46 */ { 24679, 25688 },
    /*  47 */ { 24098, 25689 },
    /*  48 */ { 23517, 25690 },
    /*  49 */ { 22886, 25306 },
    /*  50 */ { 22255, 24923 },
    /*  51 */ { 22186, 24153 },
    /*  52 */ { 22118, 23384 },
    /*  53 */ { 22118, 22870 },
    /*  54 */ { 22118, 22356 },
    /*  55 */ { 22403, 21215 },
    /*  56 */ { 22689, 20074 },
    /*  57 */ { 23555, 19816 },
    /*  58 */ { 24421, 19558 },
    /*  59 */ { 25251, 19292 },
    /*  60 */ { 26081, 19026 },
    /*  61 */ { 26562, 18384 },
    /*  62 */ { 27043, 17743 },
    /*  63 */ { 27058, 16967 },
    /*  64 */ { 27073, 16191 },
    /*  65 */ { 27055, 14217 },
    /*  66 */ { 27038, 12243 },
    /*  67 */ { 27037, 10160 },
    /*  68 */ { 27037,  8077 },
    /*  69 */ { 27037,  6919 },
    /*  70 */ { 27037,  5762 },
    /*  71 */ { 26933,  4740 },
    /*  72 */ { 26829,  3718 },
    /*  73 */ { 26537,  3409 },
    /*  74 */ { 26245,  3101 },
    /*  75 */ { 25552,  2950 },
    /*  76 */ { 24860,  2799 },
    /*  77 */ { 23452,  2873 },
    /*  78 */ { 22044,  2947 },
    /*  79 */ { 21416,  2981 },
    /*  80 */ { 20788,  3015 },
    /*  81 */ { 20073,  3056 },
    /*  82 */ { 19358,  3097 },
    /*  83 */ { 18930,  3202 },
    /*  84 */ { 18502,  3308 },
    /*  85 */ { 18155,  3493 },
    /*  86 */ { 17809,  3679 },
    /*  87 */ { 17576,  4309 },
    /*  88 */ { 17344,  4939 },
    /*  89 */ { 17338,  5368 },
    /*  90 */ { 17332,  5798 },
    /*  91 */ { 17332,  6956 },
    /*  92 */ { 17332,  8114 },
    /*  93 */ { 17332, 10084 },
    /*  94 */ { 17332, 12055 },
    /*  95 */ { 17332, 14429 },
    /*  96 */ { 17332, 16803 },
    /*  97 */ { 17332, 18618 },
    /*  98 */ { 17332, 20434 },
    /*  99 */ { 17332, 21667 },
    /* 100 */ { 17332, 22901 },
    /* 101 */ { 17332, 23656 },
    /* 102 */ { 17332, 24411 },
    /* 103 */ { 17284, 25117 },
    /* 104 */ { 17237, 25823 },
    /* 105 */ { 16250, 26432 },
    /* 106 */ { 15263, 27042 },
    /* 107 */ { 14204, 26974 },
    /* 108 */ { 13145, 26906 },
    /* 109 */ { 12614, 26864 },
    /* 110 */ { 12083, 26823 },
    /* 111 */ { 11153, 26332 },
    /* 112 */ { 10223, 25841 },
    /* 113 */ { 10134, 24953 },
    /* 114 */ { 10045, 24066 },
    /* 115 */ { 10054, 22734 },
    /* 116 */ { 10063, 21403 },
    /* 117 */ { 10083, 19416 },
    /* 118 */ { 10103, 17429 },
    /* 119 */ { 10118, 15703 },
    /* 120 */ { 10133, 13977 },
    /* 121 */ { 10151, 12136 },
    /* 122 */ { 10170, 10296 },
    /* 123 */ { 10185,  9076 },
    /* 124 */ { 10201,  7857 },
    /* 125 */ { 10202,  6632 },
    /* 126 */ { 10203,  5408 },
    /* 127 */ { 10069,  4654 },
    /* 128 */ {  9936,  3900 },
    /* 129 */ {  9525,  3454 },
    /* 130 */ {  9114,  3008 },
    /* 131 */ {  8319,  2947 },
    /* 132 */ {  7524,  2886 },
    /* 133 */ {  6607,  2915 },
    /* 134 */ {  5691,  2944 },
    /* 135 */ {  5241,  2995 },
    /* 136 */ {  4791,  3046 },
    /* 137 */ {  4034,  3490 },
    /* 138 */ {  3277,  3934 },
    /* 139 */ {  3141,  4688 },
    /* 140 */ {  3005,  5442 },
    /* 141 */ {  2999,  6532 },
    /* 142 */ {  2994,  7622 },
    /* 143 */ {  2985,  8937 },
    /* 144 */ {  2976, 10252 },
    /* 145 */ {  3087, 10210 },
};


// Returns the index of the waypoint nearest to (px, pz) using Manhattan distance,
// then adds 1 so the result is the NEXT waypoint the vehicle should target
// Used once at spawn to align waypointIndex with physical grid position
static int FindNearestWaypoint(long px, long pz)
{
    int i, best = 0;
    long bestDist = 0x7FFFFFFF;
    for (i = 0; i < NUM_WAYPOINTS; i++)
    {
        long dx = waypoints[i].x - px;
        long dz = waypoints[i].z - pz;
        long dist = (dx < 0 ? -dx : dx) + (dz < 0 ? -dz : dz);
        if (dist < bestDist) 
		{ 
			bestDist = dist; best = i; 
		}
    }
    return (best + 1) % NUM_WAYPOINTS;
}


// Returns a continuous race-distance score for ordering all vehicles
//
// score = lapCount * NUM_WAYPOINTS * 1024 + waypointIndex * 1024 + segmentT
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
    int prevWp = (waypointIndex - 1 + NUM_WAYPOINTS) % NUM_WAYPOINTS;
    long segDX = (waypoints[waypointIndex].x - waypoints[prevWp].x) >> 3;
    long segDZ = (waypoints[waypointIndex].z - waypoints[prevWp].z) >> 3;
    long toDX = (px - waypoints[prevWp].x) >> 3;
    long toDZ = (pz - waypoints[prevWp].z) >> 3;
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

    return (long)lapCount * ((long)NUM_WAYPOINTS * 1024L)
        + (long)waypointIndex * 1024L
        + t;
}


// Finds the segment the vehicle is actually on by scanning RACE_PROGRESS_LOOKAHEAD
// segments ahead of the tracked index.  For each candidate segment it computes
// the projection T without clamping; T in [0, 1023] means the car is on that
// segment.  Returns the highest-scoring match so that skipped waypoints (due to
// going off-track) don't drop the vehicle's race-distance score
// Falls back to the tracked index with T=0 if no segment matches
static long ComputeBestRaceProgress(long px, long pz, int trackedWpIdx, int lapCount)
{
    long best = (long)lapCount * ((long)NUM_WAYPOINTS * 1024L)
             + (long)trackedWpIdx * 1024L;
    int i;

    for (i = 0; i < RACE_PROGRESS_LOOKAHEAD; i++)
    {
        int wpRaw = trackedWpIdx + i;
        int wp = wpRaw % NUM_WAYPOINTS;
        int prev = (wp - 1 + NUM_WAYPOINTS) % NUM_WAYPOINTS;
        int lc = (wpRaw >= NUM_WAYPOINTS) ? lapCount + 1 : lapCount;
        long segDX = (waypoints[wp].x - waypoints[prev].x) >> 3;
        long segDZ = (waypoints[wp].z - waypoints[prev].z) >> 3;
        long toDX = (px - waypoints[prev].x) >> 3;
        long toDZ = (pz - waypoints[prev].z) >> 3;
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

        prog = (long)lc * ((long)NUM_WAYPOINTS * 1024L)
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
        int wp = (currentSeg + i) % NUM_WAYPOINTS;
        int prev = (wp - 1 + NUM_WAYPOINTS) % NUM_WAYPOINTS;
        long segDX = (waypoints[wp].x - waypoints[prev].x) >> 3;
        long segDZ = (waypoints[wp].z - waypoints[prev].z) >> 3;
        long toDX = (px - waypoints[prev].x) >> 3;
        long toDZ = (pz - waypoints[prev].z) >> 3;
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
            long origDX = waypoints[wp].x - waypoints[prev].x;
            long origDZ = waypoints[wp].z - waypoints[prev].z;
            long projX = waypoints[prev].x + t * origDX / 1024L;
            long projZ = waypoints[prev].z + t * origDZ / 1024L;
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
        int seg = (*segIdx + s) % NUM_WAYPOINTS;
        (*distSinceLap)++;
        if (seg == 0)
        {
            if (*distSinceLap >= NUM_WAYPOINTS * 3 / 4)
			{
                (*lapCount)++;
			}
			
            *distSinceLap = 0;
		}
    }

    if (offset > 0)
	{
        *segIdx = (*segIdx + offset) % NUM_WAYPOINTS;
	}
}


// Grid start positions — two-column staggered formation, spaced > AVOID_DIST
// so avoidance does not fire between adjacent cars at race start
static const int spawnX[NUM_AI_RACERS] = { 3000, 4200, 3600, 4800, 4200 };
static const int spawnZ[NUM_AI_RACERS] = { 8800, 7800, 5800, 4800, 2800 };

// Per-racer vehicle data: cycle through the three available models
static const long aiTmdAddr[NUM_AI_RACERS] = {
    CAR_3Y_MEM_ADDR,
    CAR_2B_MEM_ADDR,
    CAR_5G_MEM_ADDR,
    CAR_3Y_MEM_ADDR,
    CAR_2B_MEM_ADDR,
};

// AI-specific attributes: grip (maxGrip, minGrip, slidingFriction,
// lateralFriction), braking (brakeDeceleration) and steering
// (steeringResponse, minSteeringResponse) are all +20% vs player equivalents
// All other fields (maxSpeed, acceleration, etc.) are unchanged
// slidingFriction and lateralFriction are per-frame retention multipliers
// (/1000 applied to lateral speed each frame). Lower = faster decay = more grip
// So "20% better grip" means these go DOWN by 20%, not up
// brakeDeceleration, steeringResponse, minSteeringResponse, maxGrip, minGrip
// are all direct-magnitude values where higher = better, so they go UP 20%
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
		
		// Begin navigation at the waypoint nearest to this cars grid position
        aiRacers[i].waypointIndex = FindNearestWaypoint(spawnX[i], spawnZ[i]);
        aiRacers[i].lapCount          = 0;
        aiRacers[i].waypointsSinceLap = 0;
        aiRacers[i].racePosition      = 1;

        // Race-segment tracker starts at the same position as navigation
        aiRacers[i].raceSegIdx          = aiRacers[i].waypointIndex;
        aiRacers[i].raceLapCount        = 0;
        aiRacers[i].raceSegDistSinceLap = 0;

		// Clear steering interpolation/smoothing value
        aiRacers[i].steerSmooth  = 0;
        aiRacers[i].recoverTimer = 0;

        // Stagger stuck checks so all three racers don't fire the same frame
        aiRacers[i].stuckTimer = 60 + i * 20;
        aiRacers[i].lastPosX   = spawnX[i];
        aiRacers[i].lastPosZ   = spawnZ[i];
		
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
                if (absDX <= AVOID_DIST && absDZ <= AVOID_DIST) {
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


/* Returns 1 when position (px,pz) has passed waypoint wpIdx.
   Two conditions, either is sufficient:
   (a) within ADVANCE_DIST units in both X and Z — catches normal approach
   (b) segment-dot < 0 — catches wide-arc passes that miss the proximity box
   sX/sZ are >>5 to prevent int32 overflow; dX/dZ are unscaled so short
   segments (< 32 units) are detected correctly. */
static int PassedWaypoint(long px, long pz, int wpIdx)
{
    int prev = (wpIdx - 1 + NUM_WAYPOINTS) % NUM_WAYPOINTS;
    long dX = waypoints[wpIdx].x - px;
    long dZ = waypoints[wpIdx].z - pz;
    long absDX = dX < 0 ? -dX : dX;
    long absDZ = dZ < 0 ? -dZ : dZ;
    long sX = (waypoints[wpIdx].x - waypoints[prev].x) >> 5;
    long sZ = (waypoints[wpIdx].z - waypoints[prev].z) >> 5;
    return (absDX < ADVANCE_DIST && absDZ < ADVANCE_DIST) || (sX * dX + sZ * dZ < 0);
}

static void AdvanceWaypoint(AIRacer *ai, long *dX, long *dZ)
{
    long px = ai->player.gsObjectCoord.coord.t[0];
    long pz = ai->player.gsObjectCoord.coord.t[2];

    *dX = waypoints[ai->waypointIndex].x - px;
    *dZ = waypoints[ai->waypointIndex].z - pz;

    {
        int k;
        for (k = 0; k < MAX_WP_SKIP; k++)
        {
            if (!PassedWaypoint(px, pz, ai->waypointIndex))
			{
                break;
			}

            {
                int nextWp = (ai->waypointIndex + 1) % NUM_WAYPOINTS;
                ai->waypointsSinceLap++;
                if (nextWp == 0)
                {
                    if (ai->waypointsSinceLap >= (NUM_WAYPOINTS * 3 / 4))
					{
                        ai->lapCount++;
					}
					
                    ai->waypointsSinceLap = 0;
                }
                ai->waypointIndex = nextWp;
            }
        }
    }

    *dX = waypoints[ai->waypointIndex].x - px;
    *dZ = waypoints[ai->waypointIndex].z - pz;
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


// Artificial environment perception (paper section 3.3)
// Projects two probe points ahead-left and ahead-right of the car using the
// car's own transform axes (PS1 matrix values are 4096 = 1.0 fixed-point)
// GetTerrainType checks the ground tile at each probe position — anything
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


// Call once from StateInit, passing the player's spawn world position.
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
}


// Call once per frame (before UpdateRacePositions).
// Advances the player's waypoint index exactly as the AI system does, but
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
                int nextWp = (playerWaypointIndex + 1) % NUM_WAYPOINTS;
                playerWaypointsSinceLap++;
                if (nextWp == 0)
                {
                    if (playerWaypointsSinceLap >= (NUM_WAYPOINTS * 3 / 4))
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
}


// Call once per frame after UpdatePlayerRaceProgress and UpdateAIRacers
// Computes a continuous race-distance score for every vehicle then ranks
// them by counting how many others are further ahead.  O(N²) over 6
// vehicles = 36 comparisons — trivial on PS1
void UpdateRacePositions(void)
{
    long progress[1 + NUM_AI_RACERS];
    int i, j;

    progress[0] = ComputeRaceProgress(player1.gsObjectCoord.coord.t[0], player1.gsObjectCoord.coord.t[2], playerRaceSegIdx, playerRaceLapCount);

    for (i = 0; i < NUM_AI_RACERS; i++)
	{
        progress[1 + i] = ComputeRaceProgress(
            aiRacers[i].player.gsObjectCoord.coord.t[0],
            aiRacers[i].player.gsObjectCoord.coord.t[2],
            aiRacers[i].raceSegIdx, aiRacers[i].raceLapCount);
	}

    // Player position: count AI racers with higher progress
    playerRacePosition = 1;
    for (j = 1; j <= NUM_AI_RACERS; j++)
	{
        if (progress[j] > progress[0])
		{
            playerRacePosition++;
		}
	}

    // Each AI racer's position: count everyone else with higher progress
    for (i = 0; i < NUM_AI_RACERS; i++)
    {
        aiRacers[i].racePosition = 1;
        for (j = 0; j <= NUM_AI_RACERS; j++)
		{
            if (j != (1 + i) && progress[j] > progress[1 + i])
			{
                aiRacers[i].racePosition++;
			}
		}
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

        ai = &aiRacers[i];

        activeVehicle = ai->attribs;
        activeSuspension = ai->suspension;
        player1_roll = ai->roll;
        player1_pitch = ai->pitch;
        currentGear = ai->gear;

        // Recovery or Navigation
        if (ai->recoverTimer > 0)
        {
            // Reverse away from the wall. SenseWalls detects which side the
            // boundary is on; inverting the result rotates the rear of the car
            // away from it as the car backs up. When both probes fire (head-on
            // impact) steer=0 so the car reverses straight back
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
        // The extra lookahead means small heading deviations on straights dont
        // produce a steering correction (the target direction barely changes),
        // and corners are set up earlier so the car doesn't overshoot
        steerWpIdx = (ai->waypointIndex + 2) % NUM_WAYPOINTS;

        steer = ComputeSteering(&ai->player.gsObjectCoord, waypoints[steerWpIdx].x, waypoints[steerWpIdx].z);

        // Smooth steering — always interpolate toward the target at a fixed rate
        // Bidirectional lerp prevents the discrete jumps in ComputeSteering output
        // (0 / ±14 / ±26 / ±38) from producing visible frame-to-frame twitching
        {
            if (ai->steerSmooth < steer) 
			{ 
				ai->steerSmooth += 4; 
				if (ai->steerSmooth > steer) 
				{
					ai->steerSmooth = steer; 
				}
			}
			else if (ai->steerSmooth > steer) 
			{ 
				ai->steerSmooth -= 4;
				if (ai->steerSmooth < steer) 
				{
					ai->steerSmooth = steer; 
				}
			}
		}

        // Brake for medium and sharp corners; also predict-brake before
        // the next waypoint when it requires a sharp turn.
        {
            int absRaw = steer < 0 ? -steer : steer;
            int predictBrake = (absDX < LOOKAHEAD_DIST * 3 && absDZ < LOOKAHEAD_DIST * 3) && IsSharpCorner(ai);
            isBraking = ((absRaw >= 26 || predictBrake) && ai->player.speed > BRAKE_SPEED) ? 1 : 0;
        }

        steer = ai->steerSmooth;
        if (isBraking) 
		{
			aiRacerBraking = 1;
		}

        // Avoidance: steer around any car ahead (AI or stopped player).
        if (ai->player.speed > BRAKE_SPEED)
        {
            int avoid = ComputeAvoidance(ai, i);
            if (avoid != 0)
			{
                steer = avoid;
			}
        }

        // Wall sensing: nudges the steering target when a track boundary is
        // detected ahead. Does not bypass steerSmooth so the correction
        // blends in via the lerp rather than snapping instantly.
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
            // Wall hit: begin reverse recovery so the car backs away rather
            // than accelerating straight back into the same wall next frame
            ai->player.speed = -30;
            ai->lateralSpeed = 0;
            ai->recoverTimer = 35;
        }

        // Stuck recovery
        // Every 20 frames, measure net displacement. Anything below 400
        // units means the car is oscillating against a wall — it should be
        // doing ~1600 units of movement in 20 frames at racing speed
        // When stuck: skip 2 waypoints so the new target is past the wall,
        // and kick the speed to 80 so steering has momentum to work with
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
        if (IsObjectNearPlayer(cameraPlayer, &aiRacers[i].player.gsObjectCoord)) 
		{
			// Submit AI racer for rendering into the ordering table
            DrawPlayer(&aiRacers[i].player, ot);
        }
    }
}