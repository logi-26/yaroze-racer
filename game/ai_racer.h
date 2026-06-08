#ifndef AI_RACER_H
#define AI_RACER_H

#include <libps.h>
#include "player.h"
#include "vehicle_attribs.h"
#include "suspension.h"

// Define the number of AI racers
#define NUM_AI_RACERS 3


// Struct for the AI racers
typedef struct {
    PlayerStruct       player;
    long               lateralSpeed;
    long               roll;           // body roll state (mirrors player1_roll)
    long               pitch;          // body pitch state (mirrors player1_pitch)
    int                gear;           // current gear (mirrors currentGear)
    int                waypointIndex;  // index into the waypoint circuit array
    int                steerSmooth;    // smoothed steering value (prevents twitchy snap between ±38)
    VehicleAttributes *attribs;
    SuspensionConfig  *suspension;
} AIRacer;


// External array of AI racers
extern AIRacer aiRacers[NUM_AI_RACERS];

// 1 if any AI racer is currently braking
extern int aiRacerBraking;


/************* FUNCTION PROTOTYPES *******************/
void InitialiseAIRacers(void);
void UpdateAIRacers(void);
void DrawAIRacers(PlayerStruct *cameraPlayer, GsOT *ot);
/*****************************************************/


#endif // AI_RACER_H