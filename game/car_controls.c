#include "car_controls.h"
#include "../engine/controller.h"
#include "player.h"
#include "game.h"


void UpdateControlPlayer1(void)
{
	// Toggle gearbox mode: SELECT switches between auto and manual
    if (BTN_PRESSED(PADselect))
        gearboxMode = (gearboxMode == GEAR_AUTO) ? GEAR_MANUAL : GEAR_AUTO;

	// Manual gear shifts: L2 down, R2 up (reverse only engageable when stationary)
    if (gearboxMode == GEAR_MANUAL) {
        if (BTN_PRESSED(PADL2)) {
            if (currentGear > 1)
                currentGear--;
            else if (currentGear == 1 && player1.speed == 0)
                currentGear = 0;
        }
        if (BTN_PRESSED(PADR2)) {
            if (currentGear == 0 && player1.speed == 0)
                currentGear = 1;
            else if (currentGear >= 1 && currentGear < 5)
                currentGear++;
        }
    }

	// Auto gearbox
    {
        int movDir = BTN_HELD(PADcross) ? 1 : (BTN_HELD(PADsquare) ? -1 : 0);
        UpdateGearbox(player1.speed, (long)activeVehicle->maxSpeed, movDir);
    }

	// Direction
    if (BTN_HELD(PADleft)) {
        RotateModel(&player1.gsObjectCoord, &player1.rotation, 0, -32, 0, &player1.speed, &player1_lateralSpeed);
    }
    if (BTN_HELD(PADright)) {
        RotateModel(&player1.gsObjectCoord, &player1.rotation, 0, 32, 0, &player1.speed, &player1_lateralSpeed);
    }

	// Movement
    {
        int movDir, isBrake;
        if (BTN_HELD(PADcross)) {
            isBrake = 0;
			
            // Manual: accelerator does nothing in reverse gear
            movDir = (gearboxMode == GEAR_MANUAL && currentGear == 0) ? 0 : 1;
        } else if (BTN_HELD(PADsquare)) {
            isBrake = 1;
			
            // Manual in forward gear: brake only (square never reverses)
            movDir = (gearboxMode == GEAR_MANUAL && currentGear >= 1) ? 0 : -1;
        } else {
            isBrake = 0;
            movDir = 0;
        }
		
        player1_isBraking = isBrake;
        AdvanceModel(&player1.gsObjectCoord, &player1.rotation, &player1.speed, &player1_lateralSpeed, movDir, isBrake);
    }
}