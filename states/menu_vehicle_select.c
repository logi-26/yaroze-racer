#include "menu_vehicle_select.h"
#include "menu_main.h"

#include "../engine/state_manager.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/controller.h"
#include "../engine/graphics.h"
#include "../engine/light.h"
#include "../engine/model.h"
#include "../engine/ui.h"
#include "../engine/lang.h"
#include "../engine/link.h"
#include "../game/vehicle_colour.h"
#include "../game/player.h"
#include "../game/world.h"
#include "../game/game.h"

#define NUM_MODELS   3
#define MAX_VARIANTS 5

static const int numVariants[NUM_MODELS]  = { 5, 5, 5 };
static const int modelOffset[NUM_MODELS]  = { 0, 5, 10 };

// Models for the vehicles
static const long carTmdAddr[NUM_MODELS][MAX_VARIANTS] = {
    { CAR_3Y_MEM_ADDR, CAR_3Y_MEM_ADDR, CAR_3Y_MEM_ADDR, CAR_3Y_MEM_ADDR, CAR_3Y_MEM_ADDR },
    { CAR_2B_MEM_ADDR, CAR_2B_MEM_ADDR, CAR_2B_MEM_ADDR, CAR_2B_MEM_ADDR, CAR_2B_MEM_ADDR },
    { CAR_5G_MEM_ADDR, CAR_5G_MEM_ADDR, CAR_5G_MEM_ADDR, CAR_5G_MEM_ADDR, CAR_5G_MEM_ADDR },
};

// Textures (car3 = yellow base, car2 = blue base)
static const long carTexAddr[NUM_MODELS][MAX_VARIANTS] = {
    { CAR_3Y_TEX_MEM_ADDR, CAR_3Y_TEX_MEM_ADDR, CAR_3Y_TEX_MEM_ADDR, CAR_3Y_TEX_MEM_ADDR, CAR_3Y_TEX_MEM_ADDR },
    { CAR_2B_TEX_MEM_ADDR, CAR_2B_TEX_MEM_ADDR, CAR_2B_TEX_MEM_ADDR, CAR_2B_TEX_MEM_ADDR, CAR_2B_TEX_MEM_ADDR },
    { CAR_5G_TEX_MEM_ADDR, CAR_5G_TEX_MEM_ADDR, CAR_5G_TEX_MEM_ADDR, CAR_5G_TEX_MEM_ADDR, CAR_5G_TEX_MEM_ADDR },
};

static const TextID modelNameID[NUM_MODELS] = {
    TXT_VEHICLE_HATCHBACK,
    TXT_VEHICLE_SALOON,
    TXT_VEHICLE_AMERICAN,
};

static const char *variantName[NUM_MODELS][MAX_VARIANTS] = {
    { "GREEN",  "RED",    "YELLOW", "BLUE",   "ORANGE" },
    { "BLACK",  "BLUE",   "RED",    "ORANGE", "WHITE"  },
    { "BLUE",   "GREEN",  "RED",    "ORANGE", "PINK"   },
};

static int stateInitialised = 0;
static int selectedModelIndex = 0;
static int selectedVariantIndex = 0;
static int localReady = 0;          	// Link game (this player has pressed Start)

static PlayerStruct showroomCar;


// Two player race: the same model as player 1's has player 1's colour (the cars share the same texture)
static int p2ModelIndex = 0;
static int p2VariantIndex = 0;
static int p2Ready = 0;
static PlayerStruct showroomCar2;

static int IsTwoPlayer(void);


// Swap a showroom vehicle's model
static void SwapTMD(PlayerStruct *car, unsigned long *addr)
{
    addr++;
    GsMapModelingData(addr);
    addr++;
    addr++;
    GsLinkObject4((unsigned long)addr, &car->gsObjectHandler, 0);
}


// Set a showroom vehicle's orientation
static void ApplyShowroomOrientation(PlayerStruct *car)
{
    SVECTOR modelRot = {1024, 0, 0};
    RotModel(&car->gsModelCoord, &modelRot, 0, 0, 0);
}


// Load a models texture and give a showroom car its own colour (its own CLUT so the two
// players cars can be the same model in different colours)
static void ColourModel(PlayerStruct *car, int slot, int model, int variant)
{
    LoadTexture(carTexAddr[model][variant]);
    PaintCar(car, slot, modelOffset[model] + variant);
}


// Decode a vehicle index back to its model and variant (using the per-model offsets)
static void DecodeVehicle(int index, int *model, int *variant)
{
	*model = (index < modelOffset[1]) ? 0 : (index < modelOffset[2]) ? 1 : 2;
	*variant = index - modelOffset[*model];
}


// Set player 2's selected vehicle (its own colour even on the same model as player 1's)
static void ApplySelection2(void)
{
	ColourModel(&showroomCar2, 1, p2ModelIndex, p2VariantIndex);
	SwapTMD(&showroomCar2, (unsigned long*)carTmdAddr[p2ModelIndex][p2VariantIndex]);
	ApplyShowroomOrientation(&showroomCar2);
	player2VehicleIndex = modelOffset[p2ModelIndex] + p2VariantIndex;
}


static void StateInit(void)
{
    stateInitialised = 1;
    localReady = 0;
	p2Ready = 0;

	DecodeVehicle(selectedVehicleIndex, &selectedModelIndex, &selectedVariantIndex);

	// Set the background colour for the menu
    SetBackgroundColor(60, 60, 90);

	// Initialse the light
	InitialiseLight(&flLights[0], 0, -100, -100, -100, 0xff, 0xff, 0xff);
    InitialiseLight(&flLights[1], 1, 1000, 1000, 1000, 0xcc, 0xcc, 0xcc);
    GsSetAmbient(0x200, 0x200, 0x200);
    GsSetLightMode(0);

	// Initialise the vehicle, load its texture and apply its colour, facing towards the screen
    InitialisePlayer(&showroomCar, 1, 0, 0, 0, (unsigned long*)carTmdAddr[selectedModelIndex][selectedVariantIndex]);
	ColourModel(&showroomCar, 0, selectedModelIndex, selectedVariantIndex);
	ApplyShowroomOrientation(&showroomCar);

	// If it is a two player race, initialise player 2's vehicle
	if (IsTwoPlayer())
	{
		SetScreenMode(SCREEN_MODE_SPLITSCREEN);
		DecodeVehicle(player2VehicleIndex, &p2ModelIndex, &p2VariantIndex);
		InitialisePlayer(&showroomCar2, 2, 0, 0, 0, (unsigned long*)carTmdAddr[p2ModelIndex][p2VariantIndex]);
		ApplySelection2();
	}
}


// Set the selected vehicle for the player
static void ApplySelection(void)
{
	ColourModel(&showroomCar, 0, selectedModelIndex, selectedVariantIndex);
    SwapTMD(&showroomCar, (unsigned long*)carTmdAddr[selectedModelIndex][selectedVariantIndex]);
    ApplyShowroomOrientation(&showroomCar);
    selectedVehicleIndex = modelOffset[selectedModelIndex] + selectedVariantIndex;

}


// Left/right buttons change the vehicle, up/down buttons change the colour
static void UpdateSelection(void)
{
	// Left/right button change vehicle, up/down buttons change the colour
    if (BTN_PRESSED(PADLleft))
    {
        selectedModelIndex = (selectedModelIndex + NUM_MODELS - 1) % NUM_MODELS;
        
		if (selectedVariantIndex >= numVariants[selectedModelIndex]) 
		{
            selectedVariantIndex = 0;
		}

		ApplySelection();
	}
    else if (BTN_PRESSED(PADLright))
    {
        selectedModelIndex = (selectedModelIndex + 1) % NUM_MODELS;
        
		if (selectedVariantIndex >= numVariants[selectedModelIndex]) 
		{
            selectedVariantIndex = 0;
		}

		ApplySelection();
    }
    else if (BTN_PRESSED(PADLup))
    {
        selectedVariantIndex = (selectedVariantIndex + numVariants[selectedModelIndex] - 1) % numVariants[selectedModelIndex];
        ApplySelection();
    }
    else if (BTN_PRESSED(PADLdown))
    {
        selectedVariantIndex = (selectedVariantIndex + 1) % numVariants[selectedModelIndex];
        ApplySelection();
    }
}


/*****************************************************
Called once when state is exited
*****************************************************/
static void StateDeinitialise(void)
{
	// Reset the state initialised flag (so the showroom is set up again next time)
	stateInitialised = 0;

	// The two player half screens
	if (gScreenMode == SCREEN_MODE_SPLITSCREEN)
	{
		SetScreenMode(SCREEN_MODE_FULLSCREEN);
	}
}


static int IsLinkGame(void)
{
	return Link_GetStatus() != LINK_OFF;
}


// A two player race (not a link game) both players choose at once
static int IsTwoPlayer(void)
{
	return NumberOfPlayers == 2 && !IsLinkGame();
}


// Two player race: (player 2 chooses with the second controller) 
static void UpdateTwoPlayer(void)
{
	u_long savedPad = pad, savedOldPad = oldPad;

	// Spin player 2's vehicle
    RotModel(&showroomCar2.gsObjectCoord, &showroomCar2.rotation, 0, 20, 0);

	// Player 1
	if (BTN_PRESSED(PADcircle))
	{
		if (!localReady)
		{
			StateDeinitialise();
			MenuMainReturnFromVehicleSelect();
			gameState = STATE_MENU_MAIN;
			return;
		}
		localReady = 0;
	}
	else if (BTN_PRESSED(PADstart))
	{
		localReady = 1;
	}

	// Player 2
	pad = (pad >> 16) & 0xFFFF;
	oldPad = (oldPad >> 16) & 0xFFFF;
	if (!p2Ready)
	{
		if (BTN_PRESSED(PADLleft))
		{
			p2ModelIndex = (p2ModelIndex + NUM_MODELS - 1) % NUM_MODELS;
			ApplySelection2();
		}
		else if (BTN_PRESSED(PADLright))
		{
			p2ModelIndex = (p2ModelIndex + 1) % NUM_MODELS;
			ApplySelection2();
		}
		else if (BTN_PRESSED(PADLup))
		{
			p2VariantIndex = (p2VariantIndex + numVariants[p2ModelIndex] - 1) % numVariants[p2ModelIndex];
			ApplySelection2();
		}
		else if (BTN_PRESSED(PADLdown))
		{
			p2VariantIndex = (p2VariantIndex + 1) % numVariants[p2ModelIndex];
			ApplySelection2();
		}
	}
	if (BTN_PRESSED(PADstart))
	{
		p2Ready = 1;
	}
	else if (BTN_PRESSED(PADcircle))
	{
		p2Ready = 0;
	}
	pad = savedPad;
	oldPad = savedOldPad;

	// Both ready: player 1 chooses the track
	if (localReady && p2Ready)
	{
		StateDeinitialise();
		gameState = STATE_MENU_TRACK_SELECT;
	}
}


// Is the other player ready (its status: on vehicle select, ready)
static int RemoteReady(int remote)
{
	return remote >= 0 && LINK_STATUS_STAGE(remote) == LINK_STAGE_VEHICLE && LINK_STATUS_FLAG(remote);
}


// Link game: Start = ready (locks the selection), Circle = not ready (while the
// other player isn't ready yet) or leave the link game. When both are ready,
// both consoles go to track select.
static void UpdateLinkGame(void)
{
	int remote;

	// Lost connection to the other console, return to the lobby
	if (Link_GetStatus() != LINK_CONNECTED)
	{
		StateDeinitialise();
		gameState = STATE_MENU_LOBBY;
		return;
	}

	remote = Link_GetRemoteData();

	if (BTN_PRESSED(PADcircle))
	{
		if (!localReady)
		{
			Link_Close();
			StateDeinitialise();
			gameState = STATE_MENU_MAIN;
			return;
		}
		if (!RemoteReady(remote))
			localReady = 0;
	}
	else if (BTN_PRESSED(PADstart))
	{
		localReady = 1;
	}

	// Tell the other console our vehicle and whether we're ready (note theirs)
	Link_SetLocalData(LINK_STATUS(LINK_STAGE_VEHICLE, selectedVehicleIndex, localReady));
	if (remote >= 0 && LINK_STATUS_STAGE(remote) == LINK_STAGE_VEHICLE)
		remoteVehicleIndex = LINK_STATUS_VALUE(remote);

	// Both ready, or the other console has already gone on to track select or
	// the race (it saw us ready)
	if ((localReady && RemoteReady(remote)) || (remote >= 0 && LINK_STATUS_STAGE(remote) != LINK_STAGE_VEHICLE))
	{
		StateDeinitialise();
		gameState = STATE_MENU_TRACK_SELECT;
	}
}


static void UpdateMenuVehicleSelect(void)
{
    if (!stateInitialised) 
	{
        StateInit();
	}

	// Change the vehicle/colour (locked once ready in a link game)
	if (!localReady)
		UpdateSelection();

	// Spin the vehicle
    RotModel(&showroomCar.gsObjectCoord, &showroomCar.rotation, 0, 20, 0);

	// Link game (exchange the selections, and go on when both players are ready)
	if (IsLinkGame())
	{
		UpdateLinkGame();
		return;
	}

	// Two player race (both players choose at once)
	if (IsTwoPlayer())
	{
		UpdateTwoPlayer();
		return;
	}

	// Return to the main menu
    if (BTN_PRESSED(PADcircle))
	{
		StateDeinitialise();
		MenuMainReturnFromVehicleSelect();
        gameState = STATE_MENU_MAIN;
	}

	// Advance to track selection
	else if (BTN_PRESSED(PADstart))
	{
		StateDeinitialise();
        gameState = STATE_MENU_TRACK_SELECT;
	}
}


// Two player race (a players choice in their half of the screen)
static void RenderHalf(int p, GsOT *ot)
{
	static char title[] = "PLAYER 0";
	PlayerStruct *car = p ? &showroomCar2 : &showroomCar;
	int model = p ? p2ModelIndex : selectedModelIndex;
	int ready = p ? p2Ready : localReady;
	VehicleAttributes *attribs = (model == 0) ? &car3Attribs : (model == 1) ? &car2Attribs : &car5Attribs;

	DrawPlayer(car, ot);

	FontFX_FontBegin();
	FontFX_SetStyle(FONT_STYLE_2);
	FontFX_SetColour(COL_DARKGREEN);
	FontFX_SetOutline(COL_WHITE);
	title[sizeof(title) - 2] = '1' + p;
	FontFX_Print(12, 6, title, ot, OT_UI);
	FontFX_FontEnd();

	FontFX_FontBegin();
	FontFX_SetColour(COL_WHITE);
	FontFX_Print(12, 22, (char*)TXT(TXT_VEHICLE_MODEL), ot, OT_UI);
	FontFX_Print(62, 22, (char*)TXT(modelNameID[model]), ot, OT_UI);
	FontFX_Print(12, 46, (char*)TXT(TXT_VEHICLE_SPEED), ot, OT_UI);
	FontFX_Print(12, 56, (char*)TXT(TXT_VEHICLE_ACCELERATION), ot, OT_UI);
	FontFX_Print(12, 66, (char*)TXT(TXT_VEHICLE_BRAKES), ot, OT_UI);
	FontFX_Print(12, 76, (char*)TXT(TXT_VEHICLE_GRIP), ot, OT_UI);
	FontFX_FontEnd();

	DrawBarSegmented(62, 47, 90, 6, attribs->maxSpeed, 400, 10, 2, 0, 180, 0, 40, 40, 40, ot, OT_UI);
	DrawBarSegmented(62, 57, 90, 6, attribs->acceleration, 10, 10, 2, 0, 180, 0, 40, 40, 40, ot, OT_UI);
	DrawBarSegmented(62, 67, 90, 6, attribs->brakeDeceleration, 15, 10, 2, 0, 180, 0, 40, 40, 40, ot, OT_UI);
	DrawBarSegmented(62, 77, 90, 6, attribs->maxGrip, 250, 10, 2, 0, 180, 0, 40, 40, 40, ot, OT_UI);

	FontFX_FontBegin();
	FontFX_SetColour(COL_WHITE);
	if (ready)
	{
		FontFX_SetPulse(0, 255, 20);
		FontFX_Print(12, 96, "READY", ot, OT_UI);
	}
	else
	{
		FontFX_Print(12, 96, "PRESS START WHEN READY", ot, OT_UI);
	}
	FontFX_FontEnd();
}


static void RenderTwoPlayer(void)
{
	// The camera (to the left of the vehicles, so they show on the right of each half)
    GsSetProjection(187);
    Camera[0].vpx = -720;
    Camera[0].vpy = -580;
    Camera[0].vpz = -1500;
    Camera[0].vrx = -720;
    Camera[0].vry = -80;
    Camera[0].vrz = 0;
    Camera[0].rz  = 0;
    Camera[0].super = WORLD;
    GsSetRefView2(&Camera[0]);

	RenderHalf(0, &WorldOrderingTable[activeBuffer]);

	// A thin black line between the halves (the bottom row of the top half, the top row of the bottom)
	DrawRect(0, gScreenHeight / 2 - 1, gScreenWidth, 1, COL_BLACK, SOLID, &WorldOrderingTable[activeBuffer], OT_UI);
	DrawRect(0, 0, gScreenWidth, 1, COL_BLACK, SOLID, &WorldOrderingTable[activeBuffer + 2], OT_UI);

	// The bottom half (with its own packet area, as in the race)
	GsSetWorkBase((PACKET *)GpuPacketArea[activeBuffer + 2]);
	RenderHalf(1, &WorldOrderingTable[activeBuffer + 2]);
}


static void RenderMenuVehicleSelect(void)
{
	static char playerText[] = "PLAYER 0";

	// Get the attributes for the currently selected vehicle
    VehicleAttributes *attribs;

	// Two player race: (each player in their half of the screen)
	if (IsTwoPlayer())
	{
		RenderTwoPlayer();
		return;
	}

    attribs = (selectedModelIndex == 0) ? &car3Attribs
                               : (selectedModelIndex == 1) ? &car2Attribs
                               :                            &car5Attribs;

	// Setup the camera/projection
    GsSetProjection(250);
    Camera[0].vpx = 0;
    Camera[0].vpy = -500;
    Camera[0].vpz = -1500;
    Camera[0].vrx = 0;
    Camera[0].vry = 0;
    Camera[0].vrz = 0;
    Camera[0].rz  = 0;
    Camera[0].super = WORLD;
    GsSetRefView2(&Camera[0]);

	// Draw the vehicle
    DrawPlayer(&showroomCar, &WorldOrderingTable[activeBuffer]);
	
	// Display the screen title
    FontFX_FontBegin();
    FontFX_SetStyle(FONT_STYLE_2);
    FontFX_SetSize(2);
    FontFX_SetColour(COL_DARKGREEN);
    FontFX_SetOutline(COL_WHITE);
    FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
    FontFX_Print(20, 20, (char*)TXT(TXT_VEHICLE_SELECT_TITLE), &WorldOrderingTable[activeBuffer], OT_UI);
    FontFX_SetSize(1);

	// Link game (show which player this console is)
	if (Link_GetPlayerNumber() != 0)
	{
		playerText[sizeof(playerText) - 2] = '0' + Link_GetPlayerNumber();
		FontFX_Print(20, 45, playerText, &WorldOrderingTable[activeBuffer], OT_UI);
		FontFX_FontEnd();

		FontFX_FontBegin();
		FontFX_SetColour(COL_WHITE);
		FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
		if (localReady)
		{
			FontFX_SetPulse(0, 255, 20);
			FontFX_Print(20, 60, "READY - WAITING FOR OTHER PLAYER", &WorldOrderingTable[activeBuffer], OT_UI);
		}
		else if (RemoteReady(Link_GetRemoteData()))
		{
			FontFX_Print(20, 60, "OTHER PLAYER IS READY - PRESS START", &WorldOrderingTable[activeBuffer], OT_UI);
		}
		else
		{
			FontFX_Print(20, 60, "PRESS START WHEN READY", &WorldOrderingTable[activeBuffer], OT_UI);
		}
	}
    FontFX_FontEnd();

    FontFX_FontBegin();
    FontFX_SetColour(COL_WHITE);
	
	// Display the vehicle attributes
    FontFX_Print(20, 180, (char*)TXT(TXT_VEHICLE_MODEL), &WorldOrderingTable[activeBuffer], OT_UI);
    FontFX_Print(90, 180, (char*)TXT(modelNameID[selectedModelIndex]), &WorldOrderingTable[activeBuffer], OT_UI);

    FontFX_Print(20, 195, (char*)TXT(TXT_VEHICLE_SPEED), &WorldOrderingTable[activeBuffer], OT_UI);
    FontFX_Print(20, 205, (char*)TXT(TXT_VEHICLE_ACCELERATION), &WorldOrderingTable[activeBuffer], OT_UI);
    FontFX_Print(20, 215, (char*)TXT(TXT_VEHICLE_BRAKES), &WorldOrderingTable[activeBuffer], OT_UI);
    FontFX_Print(20, 225, (char*)TXT(TXT_VEHICLE_GRIP), &WorldOrderingTable[activeBuffer], OT_UI);
    FontFX_FontEnd();
	
	// Draw segemnted bars to represent the vehicle attributes
    DrawBarSegmented(90, 196, 120, 6, attribs->maxSpeed, 400, 10, 2, 0, 180, 0, 40, 40, 40, &WorldOrderingTable[activeBuffer], OT_UI);
    DrawBarSegmented(90, 206, 120, 6, attribs->acceleration, 10, 10, 2, 0, 180, 0, 40, 40, 40, &WorldOrderingTable[activeBuffer], OT_UI);
    DrawBarSegmented(90, 216, 120, 6, attribs->brakeDeceleration, 15, 10, 2, 0, 180, 0, 40, 40, 40, &WorldOrderingTable[activeBuffer], OT_UI);
    DrawBarSegmented(90, 226, 120, 6, attribs->maxGrip, 250, 10, 2, 0, 180, 0, 40, 40, 40, &WorldOrderingTable[activeBuffer], OT_UI);
}


void StateMenuVehicleSelect(void)
{
    UpdateMenuVehicleSelect();
    RenderMenuVehicleSelect();
}