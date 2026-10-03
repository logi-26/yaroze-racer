#ifndef CAR_CONTROLS_H
#define CAR_CONTROLS_H

#include <libps.h>
#include "player.h"
#include "gear.h"
#include "suspension.h"


/************* FUNCTION PROTOTYPES *******************/
void UpdateControlPlayer1(void);
void InitialisePlayer2Driving(VehicleAttributes *vehicle, SuspensionConfig *suspension);
void UpdateControlPlayer2(void);
int Player2Gear(void);
GearboxMode Player2Gearbox(void);
VehicleAttributes *Player2Vehicle(void);
/*****************************************************/

#endif // CAR_CONTROLS_H
