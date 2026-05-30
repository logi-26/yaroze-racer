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

static int stateInitialised = 0;

// Local player struct so we don't touch the gameplay player1
static PlayerStruct showroomCar;


/*****************************************************
Called once when state is entered
*****************************************************/
static void StateInit(void)
{
    stateInitialised = 1;

    SetBackgroundColor(60, 60, 90);

    // Load car texture into VRAM
    LoadTexture(CAR_3_TEX_MEM_ADDR);

    // Initialise the car model
    InitialisePlayer(&showroomCar, 1, 0, 0, 0, (long*)CAR_3_MEM_ADDR);

    // Apply a fixed X tilt to gsModelCoord so the car sits horizontal
    {
        SVECTOR modelRot = {1024, 0, 0};
        RotModel(&showroomCar.gsModelCoord, &modelRot, 0, 0, 0);
    }

    // Lighting
    InitialiseLight(&flLights[0], 0, -100, -100, -100, 0xff, 0xff, 0xff);
    InitialiseLight(&flLights[1], 1, 1000, 1000, 1000, 0xcc, 0xcc, 0xcc);
    GsSetAmbient(0x200, 0x200, 0x200);
    GsSetLightMode(0);
}


static void UpdateMenuVehicleSelect(void)
{
    if (!stateInitialised)
        StateInit();

    // Spin the vehicle
    RotModel(&showroomCar.gsObjectCoord, &showroomCar.rotation, 0, 20, 0);

	// Return to the main menu
    if (BTN_PRESSED(PADcircle))
        gameState = STATE_MENU_MAIN;
}


static void RenderMenuVehicleSelect(void)
{
    // Setup the camera relative to the car coord
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

    // Draw the car using the existing DrawPlayer function
    DrawPlayer(&showroomCar, &WorldOrderingTable[activeBuffer]);

    // Title text
    FontFX_FontBegin();
    FontFX_SetStyle(FONT_STYLE_2);
    FontFX_SetSize(2);
    FontFX_SetColour(COL_DARKGREEN);
    FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
    FontFX_Print(20, 20, "SELECT VEHICLE", &WorldOrderingTable[activeBuffer], OT_UI);
    FontFX_SetSize(1);
    FontFX_FontEnd();
}


void StateMenuVehicleSelect(void)
{
    UpdateMenuVehicleSelect();
    RenderMenuVehicleSelect();
}