#include "gameplay.h"

#include "../engine/state_manager.h"
#include "../engine/controller.h"
#include "../engine/graphics.h"
#include "../engine/model.h"
#include "../engine/light.h"
#include "../engine/audio.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../game/car_controls.h"
#include "../game/player.h"
#include "../game/game.h"
#include "../game/world.h"
#include "../game/hud.h"
#include "../game/brakelights.h"
#include "../game/vehicle_colour.h"
#include "../game/ai_racer.h"
#include "../game/link_race.h"
#include "../game/ground.h"

PlayerStruct player1;
PlayerStruct player2;

static int stateInitialised = 0;

// Single player race: 0 while racing, then the finishing position (1-6)
static int finishPlace = 0;

// Two player race: player 2's finishing position (0 while racing)
static int finishPlace2 = 0;

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


// Re-colour a vehicle's body (its base texture) to its variant
static void ColourVehicle(int vehicleIndex)
{
	// Re-colour car3 body (yellow base) to selected variant
    if (vehicleIndex < 5) 
	{
        ApplyVehicleColour(playerTexAddr[vehicleIndex], CAR3_BODY_CLUT_START, CAR3_BODY_CLUT_END, car3Colours[vehicleIndex]);
	}

	// Re-colour car2 body (blue base) to selected variant
    if (vehicleIndex >= 5 && vehicleIndex < 10) 
	{
        int v = vehicleIndex - 5;
        ApplyVehicleColour(playerTexAddr[vehicleIndex], CAR2_BODY_CLUT_START_1, CAR2_BODY_CLUT_END_1, car2Colours[v]);
        ApplyVehicleColour(playerTexAddr[vehicleIndex], CAR2_BODY_CLUT_START_2, CAR2_BODY_CLUT_END_2, car2Colours[v]);
    }

	// Re-colour car5 body (green base) to selected variant
    if (vehicleIndex >= 10) 
	{
        int v = vehicleIndex - 10;
        ApplyVehicleColour(playerTexAddr[vehicleIndex], CAR5_BODY_CLUT_START_1, CAR5_BODY_CLUT_END_1, car5Colours[v]);
        ApplyVehicleColour(playerTexAddr[vehicleIndex], CAR5_BODY_CLUT_START_2, CAR5_BODY_CLUT_END_2, car5Colours[v]);
        ApplyVehicleColour(playerTexAddr[vehicleIndex], CAR5_BODY_CLUT_START_3, CAR5_BODY_CLUT_END_3, car5Colours[v]);
    }
}


// Two player race: player 2's car, on its start position, driven with the second controller
static void InitialiseLocalPlayer2(void)
{
	const StartDef *s = &trackStarts[START_PLAYER2];
	int v = player2VehicleIndex;
	SVECTOR modelRot = {0, 0, 0, 0};

	InitialisePlayer(&player2, 2, s->x, CAR_GROUND_Y, s->z, (unsigned long *)playerTmdAddr[v]);
	SetPlayerHeading(&player2, s->heading);
	RotModel(&player2.gsModelCoord, &modelRot, 3072, 2048, 0);
	InitialisePlayer2Driving((v < 5) ? &car3Attribs : (v < 10) ? &car2Attribs : &car5Attribs,
	                         (v < 5) ? &car3Suspension : (v < 10) ? &car2Suspension : &car5Suspension);
	InitialiseOpponentRaceProgress(s->x, s->z);
	InitialiseTrackerViewPlayer2(&Camera[1], RACE_PROJECTION, 0, 0, -694, -1500, 0, -394, 0);
	player2_lateralSpeed = 0;
	player2_isBraking = 0;
	rearViewActive2 = 0;

	// A different model has its own colour and brake lights
	// The same vehicle model as player 1 has player 1's colour and brake lights (due to the shared base texture)
	if (v / 5 != selectedVehicleIndex / 5)
	{
		ColourVehicle(v);
		InitAIBrakeLightEffect(playerTexAddr[v], brakeLightIdx[v]);
	}
}


static void StateInit(void)
{
    long startX, startZ;
    int startHeading;

    stateInitialised = 1;

	// The selected track's data (its start positions are needed before the world is initialised)
	SelectTrack();

	// Two player race (each player has half the screen)
	if (NumberOfPlayers == 2)
	{
		SetScreenMode(SCREEN_MODE_SPLITSCREEN);
	}

	// A link race if the link is connected (two players, no AI racers)
	LinkRace_Begin();
	LinkRace_GetStart(&startX, &startZ, &startHeading);
	
	activeVehicle    = (selectedVehicleIndex <  5) ? &car3Attribs    : (selectedVehicleIndex < 10) ? &car2Attribs    : &car5Attribs;
	activeSuspension = (selectedVehicleIndex <  5) ? &car3Suspension : (selectedVehicleIndex < 10) ? &car2Suspension : &car5Suspension;

	// Initialise player 1 with the vehicle chosen on the select screen
    InitialisePlayer(&player1, 1, startX, CAR_GROUND_Y, startZ, (long*)playerTmdAddr[selectedVehicleIndex]);
    SetPlayerHeading(&player1, startHeading);
    {
        SVECTOR modelRot = {0, 0, 0, 0};
        RotModel(&player1.gsModelCoord, &modelRot, 3072, 2048, 0);
    }

	// Initialise the view for player 1
    InitialiseTrackerViewPlayer1(&Camera[0], 250, 0, 0, -694, -1500, 0, -394, 0);

	// Initialise the world (this initialises the vehicle and textures)
	InitialiseWorld();

	// Load the vehicle texture (car3 variants all share the yellow base)
    LoadTexture(playerTexAddr[selectedVehicleIndex]);

	// Re-colour the body to the selected variant
	ColourVehicle(selectedVehicleIndex);

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

	if (LinkRace_IsActive())
	{
		// The other player's car (re-coloured unless it's the same model as ours)
		LinkRace_InitOpponent((unsigned long *)playerTmdAddr[remoteVehicleIndex]);
		if (remoteVehicleIndex / 5 != selectedVehicleIndex / 5)
		{
			ColourVehicle(remoteVehicleIndex);
		}
	}
	else if (NumberOfPlayers == 2)
	{
		// Two player race (no AI racers)
		InitialiseLocalPlayer2();
	}
	else
	{
		// Initialise the AI racers
		InitialiseAIRacers();
	}

	// Initialise player race-progress tracking to match their grid position
    InitialisePlayerRaceProgress(startX, startZ);

	// Start the race afresh (these carry over from a previous race)
	finishPlace = 0;
	finishPlace2 = 0;
	ResetLapTimer();
	currentGear = 1;
	player1_lateralSpeed = 0;
	player1_pitch = 0;
	player1_roll = 0;
	player1_isBraking = 0;
}


static void StateDeinitialise(void)
{
    SetBrakeLightTexture(0);
    SetAIBrakeLightTexture(0);
    stateInitialised = 0;
}


// Leave the race for the main menu (closes the link in a link race)
static void LeaveRace(void)
{
	if (LinkRace_IsActive())
	{
		LinkRace_End();
	}
	StateDeinitialise();
	SetScreenMode(SCREEN_MODE_FULLSCREEN);
	gameState = STATE_MENU_MAIN;
	RestartMusic();
	PauseMusic();
}


// Two player race (the positions, and each player finishes after the last lap, first or second)
static void UpdateTwoPlayerRace(void)
{
	UpdateOpponentRaceProgress(player2.gsObjectCoord.coord.t[0], player2.gsObjectCoord.coord.t[2]);
	UpdateLinkRacePositions(player2.gsObjectCoord.coord.t[0], player2.gsObjectCoord.coord.t[2]);

	if (!finishPlace && playerRaceLapCount >= NUM_RACE_LAPS)
	{
		finishPlace = finishPlace2 ? 2 : 1;
	}
	if (!finishPlace2 && OpponentRaceLapCount() >= NUM_RACE_LAPS)
	{
		finishPlace2 = finishPlace ? 2 : 1;
	}
}


// Each players race position (once finished, where they finished)
static int Player1Position(void)
{
	return (NumberOfPlayers == 2 && finishPlace) ? finishPlace : playerRacePosition;
}

static int Player2Position(void)
{
	return finishPlace2 ? finishPlace2 : (playerRacePosition == 1 ? 2 : 1);
}


// Single player race: crossing the line after the last lap finishes the
// race in the current position, which then stays
static void UpdateRaceFinish(void)
{
	if (!finishPlace && playerRaceLapCount >= NUM_RACE_LAPS)
	{
		finishPlace = playerRacePosition;
	}
	if (finishPlace)
	{
		playerRacePosition = finishPlace;
	}
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
    if (BTN_PRESSED(PADL1) && !LinkRace_IsActive()) 
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

	// Set the screen back to split-screen mode after a game pause (if it was a two player race) — the pause screen is full-screen
	if (gameState == STATE_GAMEPLAY && NumberOfPlayers == 2 && gScreenMode != SCREEN_MODE_SPLITSCREEN)
	{
		SetScreenMode(SCREEN_MODE_SPLITSCREEN);
	}
	if (NumberOfPlayers == 2 && gScreenMode != SCREEN_MODE_SPLITSCREEN)
	{
		SetScreenMode(SCREEN_MODE_SPLITSCREEN);
	}

	// Update the player 1 controls (and player 2's in a two player race)
    UpdateControlPlayer1();
	if (NumberOfPlayers == 2)
	{
		UpdateControlPlayer2();
	}

	// Update AI racers (not in a link race or a two player race)
	if (AIRacersActive())
	{
		UpdateAIRacers();
	}

	// Update race progress and positions
    UpdatePlayerRaceProgress();
	if (AIRacersActive())
	{
		UpdateRacePositions();
		UpdateRaceFinish();
	}
	else if (NumberOfPlayers == 2)
	{
		UpdateTwoPlayerRace();
	}

	// Check player 1 collisions (and player 2's, and the two cars, in a two player race)
    CheckWorldCollisions(&player1, &player1_lateralSpeed);
	if (NumberOfPlayers == 2)
	{
		CheckWorldCollisions(&player2, &player2_lateralSpeed);
		CollideCars(&player1, &player1_lateralSpeed, &player2);
		CollideCars(&player2, &player2_lateralSpeed, &player1);
	}

	// Link race (exchange car positions with the other console, positions, finish)
	LinkRace_Update();

	// Player brake lights (in a two player race the same model shares them)
	{
		int braking = player1_isBraking ||
			(NumberOfPlayers == 2 && player2VehicleIndex / 5 == selectedVehicleIndex / 5 && player2_isBraking);
		if (braking != prevBraking)
		{
			SetBrakeLightTexture(braking);
			prevBraking = braking;
		}
	}

	// In a two player race, the other players brake lights are only active if they are a different model (as the same model shares the brake light effect)
	if (NumberOfPlayers == 2 && player2VehicleIndex / 5 != selectedVehicleIndex / 5)
	{
		if (player2_isBraking != prevAIBraking)
		{
			SetAIBrakeLightTexture(player2_isBraking);
			prevAIBraking = player2_isBraking;
		}
	}

	// AI brake lights — only active when player uses car2/car5 (different CLUT from AI car3 texture)
    if (AIRacersActive() && selectedVehicleIndex >= 5)
	{
        if (aiRacerBraking != prevAIBraking) 
		{
            SetAIBrakeLightTexture(aiRacerBraking);
            prevAIBraking = aiRacerBraking;
        }
    }

	// No pause during a link race
	// Once finished (or the other player has gone) Start returns to the main menu
	if (LinkRace_IsActive())
	{
		if (BTN_PRESSED(PADstart) && LinkRace_CanLeave())
		{
			LeaveRace();
		}
		return;
	}

	// Race finished (in a two player race both players start button returns to the main menu)
	if (NumberOfPlayers == 2 ? (finishPlace && finishPlace2) : finishPlace)
	{
		if (BTN_PRESSED(PADstart))
		{
			LeaveRace();
		}
		return;
	}

	// Pause the game (the pause screen has the whole screen)
    if (BTN_PRESSED(PADstart))
	{
		if (NumberOfPlayers == 2)
		{
			SetScreenMode(SCREEN_MODE_FULLSCREEN);
		}
        gameState = STATE_MENU_PAUSE;
	}
}


// A players result once finished (in their half of the screen in a two player race),
// and PRESS START once the race is over
static void DrawRaceResult(int place, int raceOver, GsOT *ot)
{
	static const char *places[] = { "1ST", "2ND", "3RD", "4TH", "5TH", "6TH" };
	char text[16];
	int split = (NumberOfPlayers == 2);

	if (!place)
	{
		return;
	}

	sprintf(text, "FINISHED %s", places[(place - 1) % 6]);

	FontFX_FontBegin();
	FontFX_SetStyle(FONT_STYLE_2);
	FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
	FontFX_SetSize(2);
	FontFX_SetColour(place == 1 ? COL_GOLD : COL_WHITE);
	FontFX_SetOutline(COL_DARKGREY);
	FontFX_Print(20, split ? 40 : 80, text, ot, OT_UI);
	FontFX_SetSize(1);
	FontFX_FontEnd();

	if (!raceOver)
	{
		return;
	}

	FontFX_FontBegin();
	FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
	FontFX_SetColour(COL_WHITE);
	FontFX_SetOutline(COL_DARKGREY);
	FontFX_SetPulse(0, 255, 20);
	FontFX_Print(20, split ? 66 : 110, "PRESS START", ot, OT_UI);
	FontFX_FontEnd();
}


static void RenderGameplay(void)
{
	RenderWorld();

	// Two player race (each players HUD and result in their half of the screen)
	if (NumberOfPlayers == 2)
	{
		int raceOver = finishPlace && finishPlace2;
		DrawGameplayHUD(1, Player1Position(), &WorldOrderingTable[activeBuffer]);
		DrawGameplayHUD(2, Player2Position(), &WorldOrderingTable[activeBuffer + 2]);
		DrawRaceResult(finishPlace, raceOver, &WorldOrderingTable[activeBuffer]);
		DrawRaceResult(finishPlace2, raceOver, &WorldOrderingTable[activeBuffer + 2]);
		return;
	}

	DrawGameplayHUD(1, playerRacePosition, &WorldOrderingTable[activeBuffer]);
	LinkRace_DrawHUD(&WorldOrderingTable[activeBuffer]);
	DrawRaceResult(finishPlace, 1, &WorldOrderingTable[activeBuffer]);
}


void StateGameplay(void)
{
    UpdateGameplay();
    RenderGameplay();
}