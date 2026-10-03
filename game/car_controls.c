#include "car_controls.h"
#include "../engine/controller.h"
#include "player.h"
#include "game.h"
#include "world.h"


// Player 2's cars driving state (two player races). The driving code works on the globals that
// hold player 1's (the gear, the gearbox mode, the suspensions pitch and roll, the vehicles
// attributes)
// For player 2 these are swapped in, as for the AI racers
typedef struct {
	VehicleAttributes *vehicle;
	SuspensionConfig *suspension;
	long pitch, roll;
	int gear;
	GearboxMode gearbox;
} DriveState;

static DriveState player2Drive;


// Drive a car with the buttons (pad/oldPad) and the gear and suspension globals
static void DriveCar(PlayerStruct *car, long *lateralSpeed, int *isBraking)
{
	// Toggle gearbox mode: SELECT switches between auto and manual
    if (BTN_PRESSED(PADselect))
        gearboxMode = (gearboxMode == GEAR_AUTO) ? GEAR_MANUAL : GEAR_AUTO;

	// Manual gear shifts: L2 down, R2 up (reverse only engageable when stationary)
    if (gearboxMode == GEAR_MANUAL)
	{
        if (BTN_PRESSED(PADL2))
		{
            if (currentGear > 1)
			{
                currentGear--;
			}
            else if (currentGear == 1 && car->speed == 0)
			{
                currentGear = 0;
			}
        }
        if (BTN_PRESSED(PADR2)) {

            if (currentGear == 0 && car->speed == 0)
			{
                currentGear = 1;
			}
            else if (currentGear >= 1 && currentGear < 5)
			{
                currentGear++;
			}
        }
    }

	// Auto gearbox
    {
        int movDir = BTN_HELD(PADcross) ? 1 : (BTN_HELD(PADsquare) ? -1 : 0);
        UpdateGearbox(car->speed, (long)activeVehicle->maxSpeed, movDir);
    }

	// Direction
    if (BTN_HELD(PADleft))
	{
        RotateModel(&car->gsObjectCoord, &car->rotation, 0, -32, 0, &car->speed, lateralSpeed, activeVehicle, activeSuspension);
    }
    if (BTN_HELD(PADright))
	{
        RotateModel(&car->gsObjectCoord, &car->rotation, 0, 32, 0, &car->speed, lateralSpeed, activeVehicle, activeSuspension);
    }

	// Movement
    {
        int movDir, isBrake;
        if (BTN_HELD(PADcross))
		{
            isBrake = 0;

            // Manual: accelerator does nothing in reverse gear
            movDir = (gearboxMode == GEAR_MANUAL && currentGear == 0) ? 0 : 1;
        } else if (BTN_HELD(PADsquare))
		{
            isBrake = 1;

            // Manual in forward gear: brake only (square never reverses)
            movDir = (gearboxMode == GEAR_MANUAL && currentGear >= 1) ? 0 : -1;
        } else
		{
            isBrake = 0;
            movDir = 0;
        }

        *isBraking = isBrake;
        AdvanceModel(&car->gsObjectCoord, &car->rotation, &car->speed, lateralSpeed, movDir, isBrake, activeVehicle, activeSuspension);
    }

    // Apply suspension body tilt to the players model
    {
        SVECTOR modelTilt;
        modelTilt.vx = (short)(3072 + (int)player1_pitch);
        modelTilt.vy = 2048;
        modelTilt.vz = (short)player1_roll;
        RotModel(&car->gsModelCoord, &modelTilt, 0, 0, 0);
    }
}


void UpdateControlPlayer1(void)
{
	// R1: hold for rear-view camera
	rearViewActive  = BTN_HELD(PADR1) ? 1 : 0;

	// Up + Select: hold for birds-eye overview camera (one player only)
	birdsEyeActive  = (NumberOfPlayers == 1 && BTN_HELD(PADup) && BTN_HELD(PADselect)) ? 1 : 0;

	DriveCar(&player1, &player1_lateralSpeed, &player1_isBraking);
}


// Player 2's car starts in first gear, automatic, level
void InitialisePlayer2Driving(VehicleAttributes *vehicle, SuspensionConfig *suspension)
{
	player2Drive.vehicle = vehicle;
	player2Drive.suspension = suspension;
	player2Drive.pitch = 0;
	player2Drive.roll = 0;
	player2Drive.gear = 1;
	player2Drive.gearbox = GEAR_AUTO;
}


// Player 2 drives with the second controller (its buttons are the top half of pad/oldPad)
void UpdateControlPlayer2(void)
{
	VehicleAttributes *savedVehicle = activeVehicle;
	SuspensionConfig *savedSuspension = activeSuspension;
	long savedPitch = player1_pitch, savedRoll = player1_roll;
	int savedGear = currentGear;
	GearboxMode savedGearbox = gearboxMode;
	u_long savedPad = pad, savedOldPad = oldPad;

	activeVehicle = player2Drive.vehicle;
	activeSuspension = player2Drive.suspension;
	player1_pitch = player2Drive.pitch;
	player1_roll = player2Drive.roll;
	currentGear = player2Drive.gear;
	gearboxMode = player2Drive.gearbox;
	pad = (pad >> 16) & 0xFFFF;
	oldPad = (oldPad >> 16) & 0xFFFF;

	// R1: hold for player 2's rear-view camera
	rearViewActive2 = BTN_HELD(PADR1) ? 1 : 0;

	DriveCar(&player2, &player2_lateralSpeed, &player2_isBraking);

	player2Drive.pitch = player1_pitch;
	player2Drive.roll = player1_roll;
	player2Drive.gear = currentGear;
	player2Drive.gearbox = gearboxMode;

	activeVehicle = savedVehicle;
	activeSuspension = savedSuspension;
	player1_pitch = savedPitch;
	player1_roll = savedRoll;
	currentGear = savedGear;
	gearboxMode = savedGearbox;
	pad = savedPad;
	oldPad = savedOldPad;
}


// Player 2's gear, gearbox mode and vehicle (for its HUD)
int Player2Gear(void)
{
	return player2Drive.gear;
}

GearboxMode Player2Gearbox(void)
{
	return player2Drive.gearbox;
}

VehicleAttributes *Player2Vehicle(void)
{
	return player2Drive.vehicle;
}
