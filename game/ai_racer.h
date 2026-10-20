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
    long roll;           		// Body roll state
    long pitch;          		// Body pitch state
    int gear;           		// Current gear
    int waypointIndex;       	// Next waypoint for navigation (PassedWaypoint gate system)
    int lapCount;            	// Completed laps (navigation system)
    int waypointsSinceLap;   	// Waypoints passed since last lap (must hit 3/4 circuit before crossing counts)
    int racePosition;        	// Current race position (1 = leading)
    int finishPlace;         	// Where it finished (0 while still racing)
    int raceSegIdx;          	// Current circuit segment for race-progress scoring (scan-based, independent of waypointIndex)
    int raceLapCount;        	// Laps completed per race-progress tracker
    int raceSegDistSinceLap; 	// Segments advanced by race tracker since last lap start
    int steerSmooth;    		// Smoothed steering value
    int recoverTimer;   		// > 0 while reversing away from a wall
    int braking;        		// 1 while braking (for its brake lights)
    int vehicleIndex;   		// Its model and colour (0-14, as the players)
    int stuckTimer;     		// Counts down to the next position-based stuck check
    long lastPosX;      		// World X at the start of the stuck-check window
    long lastPosZ;      		// World Z at the start of the stuck-check window
    int mistakeWaypoint;     	// The waypoint the mistake below was decided for
    int mistake;             	// 1 = it brakes late for this waypoint's corner (its drivers mistake)
    VehicleAttributes tuned; 	// Its handling (its cars, changed by its drivers personality)
    VehicleAttributes *attribs;
    SuspensionConfig *suspension;
} AIRacer;


// External array of AI racers
extern AIRacer aiRacers[NUM_AI_RACERS];

// 1 if any AI racer is currently braking
extern int aiRacerBraking;

// Whether there are AI racers (not in a link race or a two player race)
int AIRacersActive(void);
int OpponentRaceLapCount(void);

// Player race-progress tracking
extern int playerWaypointIndex;
extern int playerLapCount;
extern int playerRacePosition;
extern int playerRaceSegIdx;
extern int playerRaceLapCount;
extern int playerFinishPlace;

/************* FUNCTION PROTOTYPES *******************/
void InitialiseAIRacers(const int *lockedVehicles);
void ResolveAIVehicles(int vehicles[NUM_AI_RACERS]);
void InitialisePlayerRaceProgress(long startX, long startZ);
void UpdateAIRacers(void);
void UpdatePlayerRaceProgress(void);
void UpdateRacePositions(void);
void GetRaceFinishOrder(int order[1 + NUM_AI_RACERS]);
void DrawAIRacers(PlayerStruct *cameraPlayer, GsOT *ot);
void InitialiseOpponentRaceProgress(long startX, long startZ);
void UpdateOpponentRaceProgress(long x, long z);
void UpdateLinkRacePositions(long opponentX, long opponentZ);
void InitialiseWrongWay(int car, long x, long z);
void UpdateWrongWay(int car, PlayerStruct *player);
int IsWrongWay(int car);
/*****************************************************/


#endif // AI_RACER_H