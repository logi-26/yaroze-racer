#include "menu_vehicle_select.h"

#include "../engine/state_manager.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/controller.h"
#include "../engine/graphics.h"
#include "../engine/light.h"
#include "../engine/model.h"
#include "../engine/ui.h"
#include "../engine/lang.h"
#include "../game/vehicle_colour.h"
#include "../game/player.h"
#include "../game/world.h"

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

static PlayerStruct showroomCar;


// Swap the vehicle model
static void SwapTMD(unsigned long *addr)
{
    addr++;
    GsMapModelingData(addr);
    addr++;
    addr++;
    GsLinkObject4(addr, &showroomCar.gsObjectHandler, 0);
}


// Set the vehicle orientation
static void ApplyShowroomOrientation(void)
{
    SVECTOR modelRot = {1024, 0, 0};
    RotModel(&showroomCar.gsModelCoord, &modelRot, 0, 0, 0);
}


static void StateInit(void)
{
    stateInitialised = 1;

	// Decode selectedVehicleIndex back to model + variant using per-model offsets
    if (selectedVehicleIndex < modelOffset[1]) 
	{ 
		selectedModelIndex = 0; 
	}
    else if (selectedVehicleIndex < modelOffset[2]) 
	{ 
		selectedModelIndex = 1; 
	}
    else { 
		selectedModelIndex = 2; 
	}
	
    selectedVariantIndex = selectedVehicleIndex - modelOffset[selectedModelIndex];
	
	// Set the background colour for the menu
    SetBackgroundColor(60, 60, 90);
    
	// Initialse the light
	InitialiseLight(&flLights[0], 0, -100, -100, -100, 0xff, 0xff, 0xff);
    InitialiseLight(&flLights[1], 1, 1000, 1000, 1000, 0xcc, 0xcc, 0xcc);
    GsSetAmbient(0x200, 0x200, 0x200);
    GsSetLightMode(0);

	// Initialise the vehicle
    InitialisePlayer(&showroomCar, 1, 0, 0, 0, (unsigned long*)carTmdAddr[selectedModelIndex][selectedVariantIndex]);
	
	// Load the texture for the vehicle and apply colour variant
    LoadTexture(carTexAddr[selectedModelIndex][selectedVariantIndex]);
    if (selectedModelIndex == 0) 
	{
        ApplyVehicleColour(carTexAddr[0][selectedVariantIndex], CAR3_BODY_CLUT_START, CAR3_BODY_CLUT_END, car3Colours[selectedVariantIndex]);
    } else if (selectedModelIndex == 1) 
	{
        ApplyVehicleColour(carTexAddr[1][selectedVariantIndex], CAR2_BODY_CLUT_START_1, CAR2_BODY_CLUT_END_1, car2Colours[selectedVariantIndex]);
        ApplyVehicleColour(carTexAddr[1][selectedVariantIndex], CAR2_BODY_CLUT_START_2, CAR2_BODY_CLUT_END_2, car2Colours[selectedVariantIndex]);
    } else if (selectedModelIndex == 2) 
	{
        ApplyVehicleColour(carTexAddr[2][selectedVariantIndex], CAR5_BODY_CLUT_START_1, CAR5_BODY_CLUT_END_1, car5Colours[selectedVariantIndex]);
        ApplyVehicleColour(carTexAddr[2][selectedVariantIndex], CAR5_BODY_CLUT_START_2, CAR5_BODY_CLUT_END_2, car5Colours[selectedVariantIndex]);
        ApplyVehicleColour(carTexAddr[2][selectedVariantIndex], CAR5_BODY_CLUT_START_3, CAR5_BODY_CLUT_END_3, car5Colours[selectedVariantIndex]);
    }

	// Set the vehicles rotation (facing towards the screen)
	ApplyShowroomOrientation();
}


// Set the slected vehicle for the player
static void ApplySelection(void)
{
    LoadTexture(carTexAddr[selectedModelIndex][selectedVariantIndex]);
    if (selectedModelIndex == 0) 
	{
        ApplyVehicleColour(carTexAddr[0][selectedVariantIndex], CAR3_BODY_CLUT_START, CAR3_BODY_CLUT_END, car3Colours[selectedVariantIndex]);
    } else if (selectedModelIndex == 1) 
	{
        ApplyVehicleColour(carTexAddr[1][selectedVariantIndex], CAR2_BODY_CLUT_START_1, CAR2_BODY_CLUT_END_1, car2Colours[selectedVariantIndex]);
        ApplyVehicleColour(carTexAddr[1][selectedVariantIndex], CAR2_BODY_CLUT_START_2, CAR2_BODY_CLUT_END_2, car2Colours[selectedVariantIndex]);
    } else if (selectedModelIndex == 2) 
	{
        ApplyVehicleColour(carTexAddr[2][selectedVariantIndex], CAR5_BODY_CLUT_START_1, CAR5_BODY_CLUT_END_1, car5Colours[selectedVariantIndex]);
        ApplyVehicleColour(carTexAddr[2][selectedVariantIndex], CAR5_BODY_CLUT_START_2, CAR5_BODY_CLUT_END_2, car5Colours[selectedVariantIndex]);
        ApplyVehicleColour(carTexAddr[2][selectedVariantIndex], CAR5_BODY_CLUT_START_3, CAR5_BODY_CLUT_END_3, car5Colours[selectedVariantIndex]);
    }
	
    SwapTMD((unsigned long*)carTmdAddr[selectedModelIndex][selectedVariantIndex]);
    ApplyShowroomOrientation();
    selectedVehicleIndex = modelOffset[selectedModelIndex] + selectedVariantIndex;
}


static void UpdateMenuVehicleSelect(void)
{
    if (!stateInitialised) 
	{
        StateInit();
	}

	// Left/right button change vehicle, up/down buttons change colour
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

	// Spin the vehicle
    RotModel(&showroomCar.gsObjectCoord, &showroomCar.rotation, 0, 20, 0);

	// Return to the main menu
    if (BTN_PRESSED(PADcircle)) 
	{
        gameState = STATE_MENU_MAIN;
	}
    
	// Start the game
	if (BTN_PRESSED(PADstart)) 
	{
        gameState = STATE_GAMEPLAY;
	}
}


static void RenderMenuVehicleSelect(void)
{
	// Get the attributes for the currently selected vehicle
    VehicleAttributes *attribs = (selectedModelIndex == 0) ? &car3Attribs
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