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

PlayerStruct player1;
PlayerStruct player2;

static int stateInitialised = 0;


static void StateInit(void)
{
    stateInitialised = 1;
	
	// Initialise single screen mode
    InitSingleScreen();

	// Initialise player 1
    InitialisePlayer(&player1, 1, 3605, -200, 9273, (long*)CAR_3_MEM_ADDR);
    {
        SVECTOR modelRot = {0, 0, 0, 0};
        RotModel(&player1.gsModelCoord, &modelRot, 3072, 2048, 0);
    }

	// Initialise player 2
    //InitialisePlayer(&player2, 2, 2448, -200, 9273, (long*)CAR_2_MEM_ADDR);

	// Initialise the view for player 1
    InitialiseTrackerViewPlayer1(&Camera[0], 250, 0, 0, -500, -1500, 0, -200, 0);
	
	// Initialise the lights
    InitialiseLight(&flLights[0], 0, -100, -100, -100, 0xff, 0xff, 0xff);
    InitialiseLight(&flLights[1], 1, 1000, 1000, 1000, 0xcc, 0xcc, 0xcc);
    GsSetAmbient(0, 0, 0);
    GsSetLightMode(0);
    
	// initialise the world
	InitialiseWorld();
}


static void StateDeinitialise(void)
{
    stateInitialised = 0;
}


static void UpdateGameplay(void)
{
	// Initialise the state on first entry
    if (!stateInitialised)
        StateInit();

	// Update the player 1 controls
    UpdateControlPlayer1();

	// Check player 1 collisions
    CheckWorldCollisions(&player1, &player1_lateralSpeed);

	// Pause the game
    if (BTN_PRESSED(PADstart))
        gameState = STATE_MENU_PAUSE;
}


static void RenderGameplay(void)
{
	// Render the world
    RenderWorld();
}


void StateGameplay(void)
{
    UpdateGameplay();
    RenderGameplay();
}