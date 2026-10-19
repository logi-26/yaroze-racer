#include "gameplay.h"

#include "../engine/state_manager.h"
#include "../engine/lang.h"
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
#include "../game/minimap.h"
#include "../game/world.h"
#include "../game/hud.h"
#include "../game/vehicle_colour.h"
#include "../game/ai_racer.h"
#include "../game/link_race.h"
#include "../game/ground.h"
#include "../game/tournament.h"

// Debug: hold L1 and press Triangle in a single player race to finish it at once
// #define DEBUG_TOURNAMENT

PlayerStruct player1;
PlayerStruct player2;

static int stateInitialised = 0;

// Single player race: 0 while racing, then the finishing position (1-6)
static int finishPlace = 0;

// Two player race: player 2's finishing position (0 while racing)
static int finishPlace2 = 0;

// The countdown at the start of the race (3, 2, 1, then GO as the race starts)
// Nobody can move, the buttons do nothing and the lap timers dont run until it reaches GO
#define COUNTDOWN_NUMBER_MS 3000    					// How long each number (3, 2, 1) is on the screen
#define COUNTDOWN_MS (COUNTDOWN_NUMBER_MS * 3)  	    // From the race loading to GO
#define COUNTDOWN_GO_MS  1500       					// How long GO stays on the screen
#define COUNTDOWN_STEP_MS 100      						// The most a frame adds to it (the first frames time includes the loading)
static int countdownMs = 0;         					// How far into the countdown it is
static int countdownLastMs = 0;     					// gameMs last frame
static int raceStarted = 0;         					// The countdown has reached GO
static int raceWaiting = 0;         					// Link race (waiting for the other console to load the race)


// Move the countdown on by the time since the last frame (at most COUNTDOWN_STEP_MS, so the time
// spent loading the race or paused doesn't count)
static void UpdateCountdown(void)
{
	int step = gameMs - countdownLastMs;

	countdownLastMs = gameMs;
	if (step > COUNTDOWN_STEP_MS)
	{
		step = COUNTDOWN_STEP_MS;
	}

	if (step > 0 && countdownMs < COUNTDOWN_MS + COUNTDOWN_GO_MS)
	{
		countdownMs += step;
	}
}

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

	// Its own colour and brake lights (even on the same model as player 1's)
	PaintCar(&player2, 1, v);
}


static void StateInit(void)
{
    long startX, startZ;
    int startHeading;

    stateInitialised = 1;

	// A tournament race (its track, day or night, and the players vehicle are the tournaments)
	if (Tournament_IsActive())
	{
		selectedTrackIndex = Tournament_Track();
		nightMode = Tournament_IsNight();
		selectedVehicleIndex = tournament.playerVehicle;
	}

	// The selected tracks data (its start positions are needed before the world is initialised)
	SelectTrack();

	// Two player race (each player has half the screen)
	if (NumberOfPlayers == 2)
	{
		SetScreenMode(SCREEN_MODE_SPLITSCREEN);
	}

	// A link race if the link is connected (two players and no AI racers)
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

	// The tracks mini-map (after the track and the screen mode are set)
	InitialiseMiniMap();

	// Load the three vehicle textures (each car has its own CLUT for its colour and brake lights)
	LoadTexture(CAR_3Y_TEX_MEM_ADDR);
	LoadTexture(CAR_2B_TEX_MEM_ADDR);
	LoadTexture(CAR_5G_TEX_MEM_ADDR);

	// Player 1's colour and brake lights
	PaintCar(&player1, 0, selectedVehicleIndex);

	if (LinkRace_IsActive())
	{
		// The other players car (in its own colour even on the same model as ours)
		LinkRace_InitOpponent((unsigned long *)playerTmdAddr[remoteVehicleIndex]);
		PaintCar(&player2, 1, remoteVehicleIndex);
	}
	else if (NumberOfPlayers == 2)
	{
		// Two player race (no AI racers)
		InitialiseLocalPlayer2();
	}
	else
	{
		// Initialise the AI racers (in a tournament with the vehicles they have for all its races)
		InitialiseAIRacers(Tournament_IsActive() ? tournament.aiVehicle : NULL);
	}

	// Initialise player race-progress tracking to match their grid position
    InitialisePlayerRaceProgress(startX, startZ);

	// Start the race afresh (these carry over from a previous race)
	finishPlace = 0;
	finishPlace2 = 0;
	ResetLapTimer();
	countdownMs = 0;
	countdownLastMs = gameMs;
	raceStarted = 0;
	raceWaiting = 1;
	currentGear = 1;
	player1_lateralSpeed = 0;
	player1_pitch = 0;
	player1_roll = 0;
	player1_isBraking = 0;
}


static void StateDeinitialise(void)
{
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


// A tournament race is over (the player has pressed Start after finishing)
// The AI racers still racing are placed where they are now and everyone gets their points assigned
// Then the race results are shown
static void FinishTournamentRace(void)
{
	int order[1 + NUM_AI_RACERS];

	GetRaceFinishOrder(order);
	Tournament_FinaliseRace(order);
	Tournament_PrintResults();

	StateDeinitialise();
	gameState = STATE_MENU_TOURNAMENT_RESULTS;
}


// Two player race (the positions, and each player finishes after the last lap, first or second)
static void UpdateTwoPlayerRace(void)
{
	UpdateOpponentRaceProgress(player2.gsObjectCoord.coord.t[0], player2.gsObjectCoord.coord.t[2]);
	UpdateWrongWay(1, &player2);
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
// race in the next finishing place (UpdateRacePositions records it), which then stays
static void UpdateRaceFinish(void)
{
	if (!finishPlace && playerFinishPlace)
	{
		finishPlace = playerFinishPlace;
	}
}


static void UpdateGameplay(void)
{

	// Initialise the state on first entry
    if (!stateInitialised) 
	{
        StateInit();
	}

	// During the race countdown nothing moves and the buttons do nothing
	// In a link race the countdown waits until the other console has loaded the race before it starts
	if (raceWaiting)
	{
		LinkRace_Update();
		countdownLastMs = gameMs;
		if (!LinkRace_OpponentReady())
		{
			return;
		}

		raceWaiting = 0;
	}

	UpdateCountdown();
	if (!raceStarted)
	{
		if (countdownMs < COUNTDOWN_MS)
		{
			LinkRace_Update();
			return;
		}
		raceStarted = 1;
		ResetLapTimer();
	}

	// L1: record current position as a waypoint (for creating the AI route)
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

#ifdef DEBUG_TOURNAMENT
	// Debug: finish the race now (the player on their last lap and each AI racer on its last
	// lap or the one before, so some finish now and the others are still racing)
	if (AIRacersActive() && BTN_HELD(PADL1) && BTN_PRESSED(PADtriangle))
	{
		int i;
		playerRaceLapCount = NUM_RACE_LAPS;
		for (i = 0; i < NUM_AI_RACERS; i++)
		{
			if (!aiRacers[i].finishPlace)
			{
				aiRacers[i].raceLapCount = NUM_RACE_LAPS - ((frameNumber + i) & 1);
			}
		}
	}
#endif

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

	// The cars brake lights (each car has its own and they are lit when its drawn)
	player1.brakesOn = player1_isBraking;
	if (NumberOfPlayers == 2)
	{
		player2.brakesOn = player2_isBraking;
	}
	if (AIRacersActive())
	{
		int i;
		for (i = 0; i < NUM_AI_RACERS; i++)
		{
			aiRacers[i].player.brakesOn = aiRacers[i].braking;
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
	// In a tournament Start ends the race (the AI racers dont have to finish), or wait and they carry on
	if (NumberOfPlayers == 2 ? (finishPlace && finishPlace2) : finishPlace)
	{
		if (BTN_PRESSED(PADstart))
		{
			if (Tournament_IsActive())
			{
				FinishTournamentRace();
			}
			else
			{
				LeaveRace();
			}
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


// The countdown in the middle of a player's view: 3, 2, 1, then GO for a moment as the race starts
static void DrawCountdown(GsOT *ot)
{
	int elapsed = countdownMs;
	int split = (NumberOfPlayers == 2);
	char number[2];

	if (elapsed >= COUNTDOWN_MS + COUNTDOWN_GO_MS)
	{
		return;
	}

	// Link race (waiting for the other console)
	if (raceWaiting)
	{
		FontFX_FontBegin();
		FontFX_SetStyle(FONT_STYLE_2);
		FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
		FontFX_SetColour(COL_WHITE);
		FontFX_SetOutline(COL_DARKGREY);
		FontFX_SetPulse(0, 255, 20);
		FontFX_Print(20, split ? 50 : 100, (char *)TXT(TXT_WAITING_FOR_PLAYER), ot, OT_UI);
		FontFX_FontEnd();
		return;
	}

	FontFX_FontBegin();
	FontFX_SetStyle(FONT_STYLE_2);
	FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
	FontFX_SetSize(4);
	FontFX_SetOutline(COL_WHITE);
	if (elapsed < COUNTDOWN_MS)
	{
		number[0] = (char)('3' - elapsed / COUNTDOWN_NUMBER_MS);
		number[1] = 0;
		FontFX_SetColour(COL_GOLD);
		FontFX_Print(20, split ? 40 : 80, number, ot, OT_UI);
	}
	else
	{
		FontFX_SetColour(COL_GREEN);
		FontFX_Print(20, split ? 40 : 80, (char *)TXT(TXT_GO), ot, OT_UI);
	}
	FontFX_SetSize(1);
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
		DrawMiniMap(&player1, &WorldOrderingTable[activeBuffer]);
		DrawMiniMap(&player2, &WorldOrderingTable[activeBuffer + 2]);
		DrawRaceResult(finishPlace, raceOver, &WorldOrderingTable[activeBuffer]);
		DrawRaceResult(finishPlace2, raceOver, &WorldOrderingTable[activeBuffer + 2]);
		DrawCountdown(&WorldOrderingTable[activeBuffer]);
		DrawCountdown(&WorldOrderingTable[activeBuffer + 2]);
		return;
	}

	DrawGameplayHUD(1, playerRacePosition, &WorldOrderingTable[activeBuffer]);
	DrawMiniMap(&player1, &WorldOrderingTable[activeBuffer]);
	LinkRace_DrawHUD(&WorldOrderingTable[activeBuffer]);
	DrawRaceResult(finishPlace, 1, &WorldOrderingTable[activeBuffer]);
	DrawCountdown(&WorldOrderingTable[activeBuffer]);
}


void StateGameplay(void)
{
    UpdateGameplay();
    RenderGameplay();
}