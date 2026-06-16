#ifndef AI_RACER_H
#define AI_RACER_H

#include <libps.h>
#include "player.h"
#include "vehicle_attribs.h"
#include "suspension.h"

// Define the number of AI racers
#define NUM_AI_RACERS 5

// Total laps in a race
#define NUM_RACE_LAPS 3


// Struct for the AI racers
typedef struct {
    PlayerStruct player;
    long lateralSpeed;
    long roll;           		// body roll state
    long pitch;          		// body pitch state
    int gear;           		// current gear
    int waypointIndex;       	// next waypoint for navigation (PassedWaypoint gate system)
    int lapCount;            	// completed laps (navigation system)
    int waypointsSinceLap;   	// waypoints passed since last lap (must hit 3/4 circuit before crossing counts)
    int racePosition;        	// current race position (1 = leading)
    int raceSegIdx;          	// current circuit segment for race-progress scoring (scan-based, independent of waypointIndex)
    int raceLapCount;        	// laps completed per race-progress tracker
    int raceSegDistSinceLap; 	// segments advanced by race tracker since last lap start
    int steerSmooth;    		// smoothed steering value
    int recoverTimer;   		// > 0 while reversing away from a wall
    int stuckTimer;     		// counts down to the next position-based stuck check
    long lastPosX;      		// world X at the start of the stuck-check window
    long lastPosZ;      		// world Z at the start of the stuck-check window
    VehicleAttributes *attribs;
    SuspensionConfig *suspension;
} AIRacer;


// External array of AI racers
extern AIRacer aiRacers[NUM_AI_RACERS];

// 1 if any AI racer is currently braking
extern int aiRacerBraking;

// Player race-progress tracking (mirrors the per-field equivalents on AIRacer)
extern int playerWaypointIndex;
extern int playerLapCount;
extern int playerRacePosition;
extern int playerRaceSegIdx;
extern int playerRaceLapCount;

/************* FUNCTION PROTOTYPES *******************/
void InitialiseAIRacers(void);
void InitialisePlayerRaceProgress(long startX, long startZ);
void UpdateAIRacers(void);
void UpdatePlayerRaceProgress(void);
void UpdateRacePositions(void);
void DrawAIRacers(PlayerStruct *cameraPlayer, GsOT *ot);
/*****************************************************/


#endif // AI_RACER_H