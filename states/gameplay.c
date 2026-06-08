#include "gameplay.h"

#include "../engine/state_manager.h"
#include "../engine/controller.h"
#include "../engine/graphics.h"
#include "../engine/model.h"
#include "../engine/light.h"
#include "../game/car_controls.h"
#include "../game/player.h"
#include "../game/game.h"
#include "../game/world.h"
#include "../game/hud.h"
#include "../game/brakelights.h"
#include "../game/vehicle_colour.h"

PlayerStruct player1;
PlayerStruct player2;

static int stateInitialised = 0;

// Waypoints for the AI routes
long waypointX = 0;
long waypointZ = 0;
int waypointDisplayTimer = 0;


// indices 0-4: car3 (5 variants), 5-9: car2 (5 variants), 10-14: car5 (5 variants)
static const long playerTmdAddr[15] = {
    CAR_3Y_MEM_ADDR, CAR_3Y_MEM_ADDR, CAR_3Y_MEM_ADDR, CAR_3Y_MEM_ADDR, CAR_3Y_MEM_ADDR,
    CAR_2B_MEM_ADDR, CAR_2B_MEM_ADDR, CAR_2B_MEM_ADDR, CAR_2B_MEM_ADDR, CAR_2B_MEM_ADDR,
    CAR_5G_MEM_ADDR, CAR_5G_MEM_ADDR, CAR_5G_MEM_ADDR, CAR_5G_MEM_ADDR, CAR_5G_MEM_ADDR,
};


static const long playerTexAddr[15] = {
    CAR_3Y_TEX_MEM_ADDR, CAR_3Y_TEX_MEM_ADDR, CAR_3Y_TEX_MEM_ADDR, CAR_3Y_TEX_MEM_ADDR, CAR_3Y_TEX_MEM_ADDR,
    CAR_2B_TEX_MEM_ADDR, CAR_2B_TEX_MEM_ADDR, CAR_2B_TEX_MEM_ADDR, CAR_2B_TEX_MEM_ADDR, CAR_2B_TEX_MEM_ADDR,
    CAR_5G_TEX_MEM_ADDR, CAR_5G_TEX_MEM_ADDR, CAR_5G_TEX_MEM_ADDR, CAR_5G_TEX_MEM_ADDR, CAR_5G_TEX_MEM_ADDR,
};


// car3 maps to yellow brake config (2); car2 maps to blue brake config (4)
static const int brakeLightIdx[15] = { 2, 2, 2, 2, 2,  4, 4, 4, 4, 4,  6, 7, 8, 9, 10 };


static void StateInit(void)
{
    stateInitialised = 1;
	
	activeVehicle = (selectedVehicleIndex <  5) ? &car3Attribs
	              : (selectedVehicleIndex < 10) ? &car2Attribs
	              :                               &car5Attribs;

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

	// Load the vehicle texture (car3 variants all share the yellow base)
    LoadTexture(playerTexAddr[selectedVehicleIndex]);

	// Re-colour car3 body (yellow base) to selected variant
    if (selectedVehicleIndex < 5) 
	{
        ApplyVehicleColour(playerTexAddr[selectedVehicleIndex], CAR3_BODY_CLUT_START, CAR3_BODY_CLUT_END, car3Colours[selectedVehicleIndex]);
	}

	// Re-colour car2 body (blue base) to selected variant
    if (selectedVehicleIndex >= 5 && selectedVehicleIndex < 10) 
	{
        int v = selectedVehicleIndex - 5;
        ApplyVehicleColour(playerTexAddr[selectedVehicleIndex], CAR2_BODY_CLUT_START_1, CAR2_BODY_CLUT_END_1, car2Colours[v]);
        ApplyVehicleColour(playerTexAddr[selectedVehicleIndex], CAR2_BODY_CLUT_START_2, CAR2_BODY_CLUT_END_2, car2Colours[v]);
    }

	// Re-colour car5 body (green base) to selected variant
    if (selectedVehicleIndex >= 10) 
	{
        int v = selectedVehicleIndex - 10;
        ApplyVehicleColour(playerTexAddr[selectedVehicleIndex], CAR5_BODY_CLUT_START_1, CAR5_BODY_CLUT_END_1, car5Colours[v]);
        ApplyVehicleColour(playerTexAddr[selectedVehicleIndex], CAR5_BODY_CLUT_START_2, CAR5_BODY_CLUT_END_2, car5Colours[v]);
        ApplyVehicleColour(playerTexAddr[selectedVehicleIndex], CAR5_BODY_CLUT_START_3, CAR5_BODY_CLUT_END_3, car5Colours[v]);
    }

	// Prepare CLUT swap buffers for player brake light effect
    InitBrakeLightEffect(playerTexAddr[selectedVehicleIndex], brakeLightIdx[selectedVehicleIndex]);

	// Load textures for all three AI vehicles (skip whichever the player already loaded)
    if (selectedVehicleIndex >= 5) 
	{
		LoadTexture(CAR_3Y_TEX_MEM_ADDR);
	}
    
	if (selectedVehicleIndex < 5  || selectedVehicleIndex >= 10)
	{		
		LoadTexture(CAR_2B_TEX_MEM_ADDR);
	}
    
	if (selectedVehicleIndex < 10)                                       
	{
		LoadTexture(CAR_5G_TEX_MEM_ADDR);
	}
	
	// Enable the brake light effect for the AI racers
    InitAIBrakeLightEffect(CAR_3Y_TEX_MEM_ADDR, 2);

	// Initialise the AI racers
    InitialiseAIRacers();
}


static void StateDeinitialise(void)
{
    SetBrakeLightTexture(0);
    SetAIBrakeLightTexture(0);
    stateInitialised = 0;
}


static void UpdateGameplay(void)
{
    static int prevBraking   = 0;
    static int prevAIBraking = 0;

	// Initialise the state on first entry
    if (!stateInitialised) 
	{
        StateInit();
	}

	// L1: record current position as a waypoint (for craeting AI route)
    if (BTN_PRESSED(PADL1)) 
	{
        waypointX = player1.gsObjectCoord.coord.t[0];
        waypointZ = player1.gsObjectCoord.coord.t[2];
        waypointDisplayTimer = 180;
        printf("{ %ld, %ld },\n", waypointX, waypointZ);
    }
	
	
    if (waypointDisplayTimer > 0)
	{
        waypointDisplayTimer--;
	}

	// Update the player 1 controls
    UpdateControlPlayer1();

	// Update AI racers
    UpdateAIRacers();

	// Check player 1 collisions
    CheckWorldCollisions(&player1, &player1_lateralSpeed);

	// Player brake lights
    if (player1_isBraking != prevBraking) 
	{
        SetBrakeLightTexture(player1_isBraking);
        prevBraking = player1_isBraking;
    }

	// AI brake lights — only active when player uses car2/car5 (different CLUT from AI car3 texture)
    if (selectedVehicleIndex >= 5) 
	{
        if (aiRacerBraking != prevAIBraking) 
		{
            SetAIBrakeLightTexture(aiRacerBraking);
            prevAIBraking = aiRacerBraking;
        }
    }

	// Pause the game
    if (BTN_PRESSED(PADstart)) 
	{
        gameState = STATE_MENU_PAUSE;
	}
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