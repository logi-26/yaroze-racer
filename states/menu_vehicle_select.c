#include "menu_vehicle_select.h"

#include "../engine/state_manager.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/controller.h"
#include "../engine/graphics.h"
#include "../engine/light.h"
#include "../game/player.h"
#include "../game/world.h"
#include "../engine/model.h"
#include "../engine/ui.h"

#define NUM_MODELS 2
#define NUM_VARIANTS 3

// Models for the vehicles
static const long carTmdAddr[NUM_MODELS][NUM_VARIANTS] = {
    { CAR_3_MEM_ADDR,   CAR_3R_MEM_ADDR,  CAR_3Y_MEM_ADDR  },
    { CAR_2BL_MEM_ADDR, CAR_2B_MEM_ADDR,  CAR_2R_MEM_ADDR  },
};

// Textures for the vehicles
static const long carTexAddr[NUM_MODELS][NUM_VARIANTS] = {
    { CAR_3_TEX_MEM_ADDR,   CAR_3R_TEX_MEM_ADDR,  CAR_3Y_TEX_MEM_ADDR  },
    { CAR_2BL_TEX_MEM_ADDR, CAR_2B_TEX_MEM_ADDR,  CAR_2R_TEX_MEM_ADDR  },
};

// Two vehicle models to select
static const char *modelName[NUM_MODELS] = { "Hatchback", "Sports" };

// Colour variants for the two vehicles
static const char *variantName[NUM_MODELS][NUM_VARIANTS] = {
    { "GREEN", "RED",  "YELLOW" },
    { "BLACK", "BLUE", "RED"   },
};

static int stateInitialised     = 0;
static int selectedModelIndex   = 0;
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

	// Set the default selected vehicle/colour
    selectedModelIndex = selectedVehicleIndex / NUM_VARIANTS;
    selectedVariantIndex = selectedVehicleIndex % NUM_VARIANTS;
	
	// Set the background colour for the menu
    SetBackgroundColor(60, 60, 90);
    
	// Initialse the light
	InitialiseLight(&flLights[0], 0, -100, -100, -100, 0xff, 0xff, 0xff);
    InitialiseLight(&flLights[1], 1, 1000, 1000, 1000, 0xcc, 0xcc, 0xcc);
    GsSetAmbient(0x200, 0x200, 0x200);
    GsSetLightMode(0);

	// Initialise the vehicle
    InitialisePlayer(&showroomCar, 1, 0, 0, 0, (unsigned long*)carTmdAddr[selectedModelIndex][selectedVariantIndex]);
	
	// Load the texture for the vehicle
    LoadTexture(carTexAddr[selectedModelIndex][selectedVariantIndex]);
    
	// Set the vehicles rotation (facing towards the screen)
	ApplyShowroomOrientation();
}


// Set the slected vehicle for the player
static void ApplySelection(void)
{
    LoadTexture(carTexAddr[selectedModelIndex][selectedVariantIndex]);
    SwapTMD((unsigned long*)carTmdAddr[selectedModelIndex][selectedVariantIndex]);
    ApplyShowroomOrientation();
    selectedVehicleIndex = selectedModelIndex * NUM_VARIANTS + selectedVariantIndex;
}


static void UpdateMenuVehicleSelect(void)
{
    if (!stateInitialised)
        StateInit();

	// Left/right button change vehicle, up/down buttons change colour
    if (BTN_PRESSED(PADLleft))
    {
        selectedModelIndex = (selectedModelIndex + NUM_MODELS - 1) % NUM_MODELS;
        ApplySelection();
    }
    else if (BTN_PRESSED(PADLright))
    {
        selectedModelIndex = (selectedModelIndex + 1) % NUM_MODELS;
        ApplySelection();
    }
    else if (BTN_PRESSED(PADLup))
    {
        selectedVariantIndex = (selectedVariantIndex + NUM_VARIANTS - 1) % NUM_VARIANTS;
        ApplySelection();
    }
    else if (BTN_PRESSED(PADLdown))
    {
        selectedVariantIndex = (selectedVariantIndex + 1) % NUM_VARIANTS;
        ApplySelection();
    }

	// Spin the vehicle
    RotModel(&showroomCar.gsObjectCoord, &showroomCar.rotation, 0, 20, 0);

	// Return to the main menu
    if (BTN_PRESSED(PADcircle))
        gameState = STATE_MENU_MAIN;
    
	// Start the game
	if (BTN_PRESSED(PADstart))
        gameState = STATE_GAMEPLAY;
}


static void RenderMenuVehicleSelect(void)
{
	// Get the attributes for the currently selected vehicle
    VehicleAttributes *attribs = (selectedModelIndex == 0) ? &car3Attribs : &car2Attribs;

	// Setup the camera/projection
    GsSetProjection(250);
    Camera[0].vpx   = 0;
    Camera[0].vpy   = -500;
    Camera[0].vpz   = -1500;
    Camera[0].vrx   = 0;
    Camera[0].vry   = 0;
    Camera[0].vrz   = 0;
    Camera[0].rz    = 0;
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
    FontFX_Print(20, 20, "SELECT VEHICLE", &WorldOrderingTable[activeBuffer], OT_UI);
    FontFX_SetSize(1);
    FontFX_FontEnd();

    FontFX_FontBegin();
    FontFX_SetColour(COL_WHITE);
	
	// Display the vehicle attributes
    FontFX_Print(20, 180, "Model:",  &WorldOrderingTable[activeBuffer], OT_UI);
    FontFX_Print(80, 180, (char*)modelName[selectedModelIndex], &WorldOrderingTable[activeBuffer], OT_UI);

    FontFX_Print(20, 195, "Speed:", &WorldOrderingTable[activeBuffer], OT_UI);
    FontFX_Print(20, 205, "Accel:", &WorldOrderingTable[activeBuffer], OT_UI);
    FontFX_Print(20, 215, "Brake:", &WorldOrderingTable[activeBuffer], OT_UI);
    FontFX_Print(20, 225, "Grip: ", &WorldOrderingTable[activeBuffer], OT_UI);
    FontFX_FontEnd();
	
	// Draw segemnted bars to represent the vehicle attributes
    DrawBarSegmented(80, 196, 120, 6, attribs->maxSpeed,          400, 10, 2, 0, 180, 0, 40, 40, 40, &WorldOrderingTable[activeBuffer], OT_UI);
    DrawBarSegmented(80, 206, 120, 6, attribs->acceleration,      10, 10, 2, 0, 180, 0, 40, 40, 40, &WorldOrderingTable[activeBuffer], OT_UI);
    DrawBarSegmented(80, 216, 120, 6, attribs->brakeDeceleration,  15, 10, 2, 0, 180, 0, 40, 40, 40, &WorldOrderingTable[activeBuffer], OT_UI);
    DrawBarSegmented(80, 226, 120, 6, attribs->maxGrip,           250, 10, 2, 0, 180, 0, 40, 40, 40, &WorldOrderingTable[activeBuffer], OT_UI);
}


void StateMenuVehicleSelect(void)
{
    UpdateMenuVehicleSelect();
    RenderMenuVehicleSelect();
}