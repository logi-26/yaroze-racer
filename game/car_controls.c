#include "car_controls.h"
#include "../engine/controller.h"
#include "player.h"
#include "game.h"

void UpdateControlPlayer1(void)
{
	// Direction
    if (BTN_HELD(PADleft)) {
        RotateModel(&player1.gsObjectCoord, &player1.rotation, 0, -32, 0, &player1.speed, &player1_lateralSpeed);
    }
    if (BTN_HELD(PADright)) {
        RotateModel(&player1.gsObjectCoord, &player1.rotation, 0, 32, 0, &player1.speed, &player1_lateralSpeed);
    }

	// Movement
    if (BTN_HELD(PADcross)) {
        player1_isBraking = 0;
        AdvanceModel(&player1.gsObjectCoord, &player1.rotation, &player1.speed, &player1_lateralSpeed, 1, 0);
    }
    else if (BTN_HELD(PADsquare)) {
        player1_isBraking = 1;
        AdvanceModel(&player1.gsObjectCoord, &player1.rotation, &player1.speed, &player1_lateralSpeed, -1, 1);
    }
    else {
        player1_isBraking = 0;
        AdvanceModel(&player1.gsObjectCoord, &player1.rotation, &player1.speed, &player1_lateralSpeed, 0, 0);
    }
}