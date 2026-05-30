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

#define NUM_VEHICLES 3

// Model addresses for the 3 Variants of the hatchback (green, red, yellow)
static const long carTmdAddr[NUM_VEHICLES] = {
	CAR_3_MEM_ADDR,
    CAR_3R_MEM_ADDR,
    CAR_3Y_MEM_ADDR,
};

// Texture addresses for the 3 Variants of the hatchback (green, red, yellow)
static const long carTexAddr[NUM_VEHICLES] = {
    CAR_3_TEX_MEM_ADDR,
    CAR_3R_TEX_MEM_ADDR,
    CAR_3Y_TEX_MEM_ADDR,
};

static const char *carName[NUM_VEHICLES] = {"GREEN", "RED", "YELLOW"};

static int stateInitialised = 0;
static int selectedVehicle  = 0;

static PlayerStruct showroomCar;


// Remap TMD and relink object handler without calling GsInitCoordinate2
static void SwapTMD(unsigned long *addr)
{
    addr++;
    GsMapModelingData(addr);
    addr++;
    addr++;
    GsLinkObject4(addr, &showroomCar.gsObjectHandler, 0);
}


static void ApplyShowroomOrientation(void)
{
    // Fix X tilt on gsModelCoord so the car sits horizontal
    SVECTOR modelRot = {1024, 0, 0};
    RotModel(&showroomCar.gsModelCoord, &modelRot, 0, 0, 0);
}


/*****************************************************
Called once when state is entered
*****************************************************/
static void StateInit(void)
{
    stateInitialised = 1;
	
	// Set the background colour for this menu
    SetBackgroundColor(60, 60, 90);

	// Initialise the light for the showrrom vehicle
    InitialiseLight(&flLights[0], 0, -100, -100, -100, 0xff, 0xff, 0xff);
    InitialiseLight(&flLights[1], 1, 1000, 1000, 1000, 0xcc, 0xcc, 0xcc);
    GsSetAmbient(0x200, 0x200, 0x200);
    GsSetLightMode(0);

    // Set up coord hierarchy
    InitialisePlayer(&showroomCar, 1, 0, 0, 0, (unsigned long*)carTmdAddr[selectedVehicle]);
    LoadTexture(carTexAddr[selectedVehicle]);
    ApplyShowroomOrientation();
}


static void UpdateMenuVehicleSelect(void)
{
    if (!stateInitialised)
        StateInit();

	// Up and sown button switch the vehicle colour
    if (BTN_PRESSED(PADLup))
    {
        selectedVehicle = (selectedVehicle + NUM_VEHICLES - 1) % NUM_VEHICLES;
        LoadTexture(carTexAddr[selectedVehicle]);
        SwapTMD((unsigned long*)carTmdAddr[selectedVehicle]);
        ApplyShowroomOrientation();
    }
    else if (BTN_PRESSED(PADLdown))
    {
        selectedVehicle = (selectedVehicle + 1) % NUM_VEHICLES;
        LoadTexture(carTexAddr[selectedVehicle]);
        SwapTMD((unsigned long*)carTmdAddr[selectedVehicle]);
        ApplyShowroomOrientation();
    }

    // Spin the vehicle
    RotModel(&showroomCar.gsObjectCoord, &showroomCar.rotation, 0, 20, 0);

	// Return to the main menu
    if (BTN_PRESSED(PADcircle))
        gameState = STATE_MENU_MAIN;
}


static void RenderMenuVehicleSelect(void)
{
	// Setup the camera and projection
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

	// Display the text info
    FontFX_FontBegin();
    FontFX_SetStyle(FONT_STYLE_2);
    FontFX_SetSize(2);
    FontFX_SetColour(COL_DARKGREEN);
    FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
    FontFX_Print(20, 20, "SELECT VEHICLE", &WorldOrderingTable[activeBuffer], OT_UI);
    FontFX_SetColour(COL_WHITE);
    FontFX_Print(20, 200, (char*)carName[selectedVehicle], &WorldOrderingTable[activeBuffer], OT_UI);
    FontFX_SetSize(1);
    FontFX_FontEnd();
}


void StateMenuVehicleSelect(void)
{
    UpdateMenuVehicleSelect();
    RenderMenuVehicleSelect();
}