#ifndef AI_DRIVERS_H
#define AI_DRIVERS_H

#include "ai_racer.h"


// An AI racers driver
// Driver i drives AI racer i (so they always have the same model)
typedef struct {
    const char *name;           // Their name (upper case, for the font)
    const char *country;        // Their country code

    // Personality (changes to the AI cars handling and driving)
    int speedPct;               // Top speed (% of the AI cars)
    int accelAdj;               // Added to the acceleration
    int brakeDecelAdj;          // Added to the braking
    int gripPct;                // Grip (% of the AI cars)
    int steerPct;               // Steering response (% of the AI cars)
    int brakeSpeed;             // Only brakes for corners above this speed
    int cornerBrakeSteer;       // Brakes when steering at least this hard
    int steerRate;              // How fast the steering moves to where it is going (per frame)
    int avoidDist;              // How far ahead it steers around other cars
    int mistakeChance;          // The chance of a mistake at a sharp corner (0-255, 0 = never)
    int formSpread;             // Top speed changes by up to this % each race (0 = the same every race)
} DriverProfile;


// The five AI drivers (one for each AI racer)
extern const DriverProfile aiDrivers[NUM_AI_RACERS];

#endif // AI_DRIVERS_H
