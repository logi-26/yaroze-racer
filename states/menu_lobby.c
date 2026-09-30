#include "menu_lobby.h"

#include "../engine/state_manager.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/controller.h"
#include "../engine/audio.h"
#include "../engine/graphics.h"
#include "../engine/memcard.h"
#include "../engine/link.h"

// How long "CONNECTED" is shown before going to vehicle select
#define CONNECTED_FRAMES (FRAME_RATE * 3 / 2)

static int stateInitialised = 0;
static int connectedFrames;


/*****************************************************
Called once when state is entered
*****************************************************/
static void StateInit(void)
{
	// Open the link and start looking for the other console
	Link_Open();
	connectedFrames = 0;

	// Mark state as initialised
	stateInitialised = 1;
}


/*****************************************************
Called once when state is exited
*****************************************************/
static void StateDeinitialise(void)
{
	// Reset the state initialised flag
	stateInitialised = 0;
}


// Update menu lobby
static void UpdateMenuLobby(void) {

	// First state entry
	if (!stateInitialised)
		StateInit();

	// The handshake runs in Link_Update(), called every frame by the state
	// manager. Once connected, show the player number, then both consoles
	// go to vehicle select
	if (Link_GetStatus() == LINK_CONNECTED) {
		if (++connectedFrames >= CONNECTED_FRAMES) {
			StateDeinitialise();
			StartNewGame();
			return;
		}
	} else {
		connectedFrames = 0;
	}

	// If circle is pressed, close the link and return to main menu
	if (BTN_PRESSED(PADcircle)) {
		Link_Close();
		StateDeinitialise();
        gameState = STATE_MENU_MAIN;
	}
}


// Render menu lobby
static void RenderMenuLobby(void) {

	static char playerText[] = "YOU ARE PLAYER 0";

	// Display the lobby title
	FontFX_FontBegin();
	FontFX_SetStyle(FONT_STYLE_2);
	FontFX_SetSize(2);
	FontFX_SetColour(COL_DARKGREEN);
	FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
	FontFX_Print(20, 20, "LOBBY", &WorldOrderingTable[activeBuffer], OT_UI);
	FontFX_SetSize(1);
	FontFX_FontEnd();

	FontFX_FontBegin();
	FontFX_SetColour(COL_DARKGREEN);
	FontFX_SetOutline(COL_LIGHTGREY);

	switch (Link_GetStatus())
	{
		case LINK_CONNECTED:
			playerText[sizeof(playerText) - 2] = '0' + Link_GetPlayerNumber();
			FontFX_Print(20, 60, "CONNECTED", &WorldOrderingTable[activeBuffer], OT_UI);
			FontFX_Print(20, 80, playerText, &WorldOrderingTable[activeBuffer], OT_UI);
			break;

		case LINK_HANDSHAKE:
			FontFX_SetPulse(0, 255, 20);
			FontFX_Print(20, 60, "CONNECTING...", &WorldOrderingTable[activeBuffer], OT_UI);
			break;

		default:
			FontFX_SetPulse(0, 255, 20);
			FontFX_Print(20, 60, "WAITING FOR OTHER PLAYER...", &WorldOrderingTable[activeBuffer], OT_UI);
			break;
	}

	FontFX_FontEnd();
}


void StateMenuLobby(void)
{
	UpdateMenuLobby();
	RenderMenuLobby();
}
