#include "ai_drivers.h"

/*****************************************************
The AI drivers
Driver i drives AI racer i, so each driver always has the same model:
(0 hatchback, 1 saloon, 2 american, 3 hatchback, 4 saloon)

Their personalities change the AI's usual handling and driving:
  Speed, Grip, Steer:   % of the AI car's (100 = the same)
  Accel, Brake:         added to the AI car's acceleration and braking
  BrkSpd:               only brakes for corners above this speed (usually 150: lower brakes earlier)
  Corner:               brakes when steering at least this hard (usually 26: the steering is 0, 14, 26 or 38)
  Rate:                 how fast the steering moves to where it is going (usually 4)
  Avoid:                how far ahead it steers around cars (usually 2500: lower passes closer, never more than 2500)
  Mistake:              the chance of braking late for a corner (0-255, 0 = never)
  Form:                 the top speed changes by up to this % each race (0 = the same every race)
*****************************************************/

const DriverProfile aiDrivers[NUM_AI_RACERS] = {
    // Name Country  Speed Accel Brake Grip Steer BrkSpd Corner Rate Avoid Mistake Form

    // The AI's usual driving
    { "ALEX MERCER", "GB", 100, 0, 0, 100, 100, 150, 26, 4, 2500, 0, 0 },

    // Brakes early, a bit more grip and quicker steering corrections, a bit slower, never makes mistakes
    { "KLAUS RICHTER", "DE", 98, 0, 0, 105, 100, 140, 26, 5, 2500, 0, 0 },

    // More grip and steering, so it brakes less in corners, a bit slower on the straights
    { "BRAM VISSER", "NL", 97, 0, 0, 110, 105, 150, 30, 4, 2500, 0, 0 },

    // Quicker, but faster or slower each race, and sometimes brakes late
    { "MARC DELACROIX", "FR", 103, 0, 0, 100, 100, 150, 26, 4, 2500, 40, 4 },

    // Accelerates and brakes harder, brakes later, passes closer, sometimes brakes too late
    { "LUCA MORETTI", "IT", 100, 1, 1, 100, 100, 170, 26, 4, 1800, 15, 0 },
};
