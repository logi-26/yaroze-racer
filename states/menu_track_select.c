#include "menu_track_select.h"

#include "../engine/state_manager.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/controller.h"
#include "../engine/graphics.h"
#include "../game/ground.h"

#define NUM_TRACKS 2

static int stateInitialised = 0;
static int selectedItem     = 0;

static const char *trackNames[NUM_TRACKS] = {
    "TRACK 1",
    "TRACK 2"
};

static const short trackY[NUM_TRACKS] = { 100, 120 };


static void StateInit(void)
{
    selectedItem     = 0;
    stateInitialised = 1;
    SetBackgroundColor(20, 20, 40);
}


static void UpdateMenuTrackSelect(void)
{
    if (!stateInitialised)
        StateInit();

    if (BTN_PRESSED(PADLup))
        selectedItem = (selectedItem + NUM_TRACKS - 1) % NUM_TRACKS;
    else if (BTN_PRESSED(PADLdown))
        selectedItem = (selectedItem + 1) % NUM_TRACKS;

    if (BTN_PRESSED(PADcircle))
    {
        stateInitialised = 0;
        gameState = STATE_MENU_VEHICLE_SELECT;
    }

    if (BTN_PRESSED(PADcross) || BTN_PRESSED(PADstart))
    {
        selectedTrackIndex = selectedItem;
        stateInitialised   = 0;
        gameState          = STATE_GAMEPLAY;
    }
}


static void RenderMenuTrackSelect(void)
{
    int i;
    GsOT *ot = &WorldOrderingTable[activeBuffer];

    FontFX_FontBegin();
    FontFX_SetStyle(FONT_STYLE_2);
    FontFX_SetSize(2);
    FontFX_SetColour(COL_DARKGREEN);
    FontFX_SetOutline(COL_WHITE);
    FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
    FontFX_Print(20, 20, "SELECT TRACK", ot, OT_UI);
    FontFX_SetSize(1);
    FontFX_FontEnd();

    for (i = 0; i < NUM_TRACKS; i++)
    {
        FontFX_FontBegin();
        FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
        if (i == selectedItem)
        {
            FontFX_SetColour(COL_DARKGREEN);
            FontFX_SetOutline(COL_LIGHTGREY);
            FontFX_SetPulse(0, 255, 20);
        }
        else
        {
            FontFX_SetColour(COL_GREEN);
            FontFX_SetOutline(COL_DARKGREY);
        }
        FontFX_Print(120, trackY[i], (char *)trackNames[i], ot, OT_UI);
        FontFX_FontEnd();
    }
}


void StateMenuTrackSelect(void)
{
    UpdateMenuTrackSelect();
    RenderMenuTrackSelect();
}
