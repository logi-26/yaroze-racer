#include "menu_track_select.h"

#include "../engine/state_manager.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/controller.h"
#include "../engine/graphics.h"
#include "../engine/link.h"
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


static int IsLinkGame(void)
{
    return Link_GetStatus() != LINK_OFF;
}


// Player 1 chooses the track in a link game
static int ChoosesTrack(void)
{
    return !IsLinkGame() || Link_GetPlayerNumber() == 1;
}


static void StartRace(int track)
{
    selectedTrackIndex = track;

    // Link game (tell the other console the race has started and on which track)
    // The race then keeps sending our car's position with it
    if (IsLinkGame())
    {
        Link_SetLocalData(LINK_RACE_STATUS(0, 0));
        Link_SetLocalExtra(LINK_RACE_EXTRA(0, 0, track));
    }

    stateInitialised = 0;
    gameState        = STATE_GAMEPLAY;
}


static void UpdateMenuTrackSelect(void)
{
    int remote = Link_GetRemoteData();

    if (!stateInitialised)
        StateInit();

    // The connection with the other console has been lost, return to the lobby
    if (IsLinkGame() && Link_GetStatus() != LINK_CONNECTED)
    {
        stateInitialised = 0;
        gameState = STATE_MENU_LOBBY;
        return;
    }

    if (ChoosesTrack())
    {
        if (BTN_PRESSED(PADLup))
            selectedItem = (selectedItem + NUM_TRACKS - 1) % NUM_TRACKS;
        else if (BTN_PRESSED(PADLdown))
            selectedItem = (selectedItem + 1) % NUM_TRACKS;
    }
    else if (remote >= 0 && LINK_STATUS_STAGE(remote) == LINK_STAGE_TRACK &&
             LINK_STATUS_VALUE(remote) < NUM_TRACKS)
    {
        // Player 2 follows player 1's choice
        selectedItem = LINK_STATUS_VALUE(remote);
    }

    // Back to vehicle select (a link game is left: the other console goes back to the lobby)
    if (BTN_PRESSED(PADcircle))
    {
        stateInitialised = 0;
        if (IsLinkGame())
        {
            Link_Close();
            gameState = STATE_MENU_MAIN;
        }
        else
        {
            gameState = STATE_MENU_VEHICLE_SELECT;
        }
        return;
    }

    if (ChoosesTrack())
    {
        if (BTN_PRESSED(PADcross) || BTN_PRESSED(PADstart))
        {
            StartRace(selectedItem);
            return;
        }
    }
    else if (remote >= 0 && LINK_STATUS_STAGE(remote) == LINK_STAGE_RACE &&
             LINK_RACE_TRACK(Link_GetRemoteExtra()) < NUM_TRACKS)
    {
        // Player 1 has chosen (race on that track)
        StartRace(LINK_RACE_TRACK(Link_GetRemoteExtra()));
        return;
    }

    // Tell the other console we're here, and which track is highlighted
    if (IsLinkGame())
        Link_SetLocalData(LINK_STATUS(LINK_STAGE_TRACK, selectedItem, 0));
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

    // Link game (player 2 waits for player 1 to choose the track)
    if (!ChoosesTrack())
    {
        FontFX_FontBegin();
        FontFX_SetColour(COL_WHITE);
        FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
        FontFX_SetPulse(0, 255, 20);
        FontFX_Print(20, 60, "PLAYER 1 IS CHOOSING THE TRACK", ot, OT_UI);
        FontFX_FontEnd();
    }
}


void StateMenuTrackSelect(void)
{
    UpdateMenuTrackSelect();
    RenderMenuTrackSelect();
}
