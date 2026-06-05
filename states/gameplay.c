#include "gameplay.h"

#include "../engine/state_manager.h"
#include "../engine/controller.h"

#include "../engine/graphics.h"
#include "../game/car_controls.h"
#include "../game/player.h"
#include "../engine/model.h"
#include "../engine/light.h"
#include "../game/game.h"
#include "../game/world.h"
#include "../game/hud.h"
#include "../game/brakelights.h"

PlayerStruct player1;
PlayerStruct player2;

static int stateInitialised = 0;


static const long playerTmdAddr[9] = {
    CAR_3_MEM_ADDR,   CAR_3R_MEM_ADDR,  CAR_3Y_MEM_ADDR,
    CAR_2BL_MEM_ADDR, CAR_2B_MEM_ADDR,  CAR_2R_MEM_ADDR,
    CAR_5B_MEM_ADDR,  CAR_5G_MEM_ADDR,  CAR_5_MEM_ADDR,
};
static const long playerTexAddr[9] = {
    CAR_3_TEX_MEM_ADDR,   CAR_3R_TEX_MEM_ADDR,  CAR_3Y_TEX_MEM_ADDR,
    CAR_2BL_TEX_MEM_ADDR, CAR_2B_TEX_MEM_ADDR,  CAR_2R_TEX_MEM_ADDR,
    CAR_5B_TEX_MEM_ADDR,  CAR_5G_TEX_MEM_ADDR,  CAR_5_TEX_MEM_ADDR,
};

static void StateInit(void)
{
    stateInitialised = 1;
	
	activeVehicle = (selectedVehicleIndex < 3) ? &car3Attribs
	              : (selectedVehicleIndex < 6) ? &car2Attribs
	              :                              &car5Attribs;

	// Initialise player 1 with the vehicle chosen on the select screen
    InitialisePlayer(&player1, 1, 3605, -200, 9273, (long*)playerTmdAddr[selectedVehicleIndex]);
    {
        SVECTOR modelRot = {0, 0, 0, 0};
        RotModel(&player1.gsModelCoord, &modelRot, 3072, 2048, 0);
    }

	// Initialise the view for player 1
    InitialiseTrackerViewPlayer1(&Camera[0], 250, 0, 0, -500, -1500, 0, -200, 0);

	// Initialise the lights
    InitialiseLight(&flLights[0], 0, -100, -100, -100, 0xff, 0xff, 0xff);
    InitialiseLight(&flLights[1], 1, 1000, 1000, 1000, 0xcc, 0xcc, 0xcc);
    GsSetAmbient(0, 0, 0);
    GsSetLightMode(0);

	// Initialise the world (this initialises the vehicle and textures)
	InitialiseWorld();

	// Override the car texture with the selected variant
    if (selectedVehicleIndex != 0)
        LoadTexture(playerTexAddr[selectedVehicleIndex]);

	// Prepare CLUT swap buffers for brake light effect
    InitBrakeLightEffect(playerTexAddr[selectedVehicleIndex], selectedVehicleIndex);
}


static void StateDeinitialise(void)
{
    SetBrakeLightTexture(0);
    stateInitialised = 0;
}


static void UpdateGameplay(void)
{
    static int prevBraking = 0;

	// Initialise the state on first entry
    if (!stateInitialised)
        StateInit();

	// Update the player 1 controls
    UpdateControlPlayer1();

	// Check player 1 collisions
    CheckWorldCollisions(&player1, &player1_lateralSpeed);

	// Swap brake-light CLUT only on state transition (not every frame)
    if (player1_isBraking != prevBraking) {
        SetBrakeLightTexture(player1_isBraking);
        prevBraking = player1_isBraking;
    }

	// Pause the game
    if (BTN_PRESSED(PADstart))
        gameState = STATE_MENU_PAUSE;
}


static void RenderGameplay(void)
{
	RenderWorld();
	DrawGameplayHUD(&WorldOrderingTable[activeBuffer]);
}


void StateGameplay(void)
{
    UpdateGameplay();
    RenderGameplay();
}