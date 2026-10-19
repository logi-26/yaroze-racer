#include "ai_drivers.h"

/*****************************************************
The AI drivers
Driver i drives AI racer i, so each driver always has the same model:
(0 hatchback, 1 saloon, 2 american, 3 hatchback, 4 saloon)
The AI racers personalities are all the same at the moment
*****************************************************/

const DriverProfile aiDrivers[NUM_AI_RACERS] = {
    // Name Country  Speed Accel Brake Grip Steer BrkSpd Corner Rate Avoid Mistake Form

    // The consistent rival
    { "ALEX MERCER", "GB", 100, 0, 0, 100, 100, 150, 26, 4, 2500, 0, 0 },

    // The precision driver
    { "KLAUS RICHTER", "DE", 100, 0, 0, 100, 100, 150, 26, 4, 2500, 0, 0 },

    // The technical specialist
    { "BRAM VISSER", "NL", 100, 0, 0, 100, 100, 150, 26, 4, 2500, 0, 0 },

    // The unpredictable contender
    { "MARC DELACROIX", "FR", 100, 0, 0, 100, 100, 150, 26, 4, 2500, 0, 0 },

    // The aggressive challenger
    { "LUCA MORETTI", "IT", 100, 0, 0, 100, 100, 150, 26, 4, 2500, 0, 0 },
};
