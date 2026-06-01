#ifndef GEAR_H
#define GEAR_H


typedef enum {
    GEAR_AUTO   = 0,
    GEAR_MANUAL = 1,
} GearboxMode;


extern int currentGear;
extern GearboxMode gearboxMode;


/************* FUNCTION PROTOTYPES *******************/
void UpdateGearbox(long speed, long maxSpeed, int movementDirection);
void ApplyGearAccel(long *massedAccel, long speed, long maxSpeed);
long GetGearTopSpeed(long maxSpeed);
int  GetGearRevPct(long speed, long maxSpeed);
/*****************************************************/

#endif // GEAR_H