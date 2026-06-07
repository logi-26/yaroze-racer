#ifndef VEHICLE_COLOUR_H
#define VEHICLE_COLOUR_H

#include <libps.h>

typedef enum {
    VEHICLE_COLOUR_ORIGINAL = 0,
    VEHICLE_COLOUR_ORANGE,
    VEHICLE_COLOUR_RED,
    VEHICLE_COLOUR_BLUE_FROM_YELLOW,   // blue from yellow-based texture
    VEHICLE_COLOUR_GREEN_FROM_YELLOW,  // green from yellow-based texture
    VEHICLE_COLOUR_ORANGE_FROM_BLUE,   // orange from blue-based texture
    VEHICLE_COLOUR_DARK,               // darken any texture (~20% brightness)
    VEHICLE_COLOUR_RED_FROM_BLUE,      // red from blue-based texture
    VEHICLE_COLOUR_WHITE_FROM_BLUE,    // white/silver from blue-based texture
    VEHICLE_COLOUR_GREEN_FROM_BLUE,    // green from blue-based texture
    VEHICLE_COLOUR_YELLOW_FROM_BLUE,   // yellow from blue-based texture
    VEHICLE_COLOUR_RED_FROM_GREEN,     // red from green-based texture
    VEHICLE_COLOUR_BLUE_FROM_GREEN,    // blue from green-based texture
    VEHICLE_COLOUR_ORANGE_FROM_GREEN,  // orange from green-based texture
    VEHICLE_COLOUR_PINK_FROM_GREEN,    // pink from green-based texture
    VEHICLE_COLOUR_COUNT
} VehicleColour;


// car3 body colour CLUT index range
#define CAR3_BODY_CLUT_START 41
#define CAR3_BODY_CLUT_END   69

// car2 body colour CLUT index ranges (two non-contiguous bands)
#define CAR2_BODY_CLUT_START_1  10
#define CAR2_BODY_CLUT_END_1    47
#define CAR2_BODY_CLUT_START_2  49
#define CAR2_BODY_CLUT_END_2    50

// car5 body colour CLUT index ranges (three non-contiguous bands)
#define CAR5_BODY_CLUT_START_1  19
#define CAR5_BODY_CLUT_END_1    24
#define CAR5_BODY_CLUT_START_2  26
#define CAR5_BODY_CLUT_END_2    28
#define CAR5_BODY_CLUT_START_3  40
#define CAR5_BODY_CLUT_END_3    48

// car3 colour per variant index (0=green, 1=red, 2=yellow, 3=blue, 4=orange)
extern const VehicleColour car3Colours[5];

// car2 colour per variant index (0=green, 1=blue, 2=yellow, 3=orange, 4=white)
extern const VehicleColour car2Colours[5];

// car5 colour per variant index (0=blue, 1=green, 2=red, 3=orange, 4=pink)
extern const VehicleColour car5Colours[5];


/************* FUNCTION PROTOTYPES *******************/
void ApplyVehicleColour(long texAddr, int bodyStart, int bodyEnd, VehicleColour colour);
/*****************************************************/

#endif