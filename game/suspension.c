#include "suspension.h"


// Hatchback (car3) - medium weight, moderate suspension
SuspensionConfig car3Suspension = {
    100,  // mass 				(baseline weight)    
    8,    // suspensionRate 	(moderate travel rate)
    80,   // pitchFactor   		(moderate nose dive/squat)
    60    // rollFactor    		(noticeable body roll in corners)
};


// Saloon (car2) - lighter, stiffer sport suspension
SuspensionConfig car2Suspension = {
    85,   // mass          		(lighter than baseline)
    12,   // suspensionRate 	(snaps to position quickly)
    50,   // pitchFactor   		(minimal pitch, planted feel)
    30    // rollFactor    		(very little roll, low centre of gravity)
};


// American (car5) - heavy, very soft suspension
SuspensionConfig car5Suspension = {
    130,  // mass          		(significantly heavier)
    5,    // suspensionRate 	(very slow travel)
    120,  // pitchFactor   		(dramatic nose dive and squat)
    80    // rollFactor    		(significant body roll in corners)
};


SuspensionConfig *activeSuspension = &car3Suspension;