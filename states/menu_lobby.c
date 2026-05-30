#include "../menu_lobby.h"

#include "../engine/state_manager.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/controller.h"
#include "../engine/audio.h"
#include "../engine/graphics.h"
#include "../engine/memcard.h"

static int stateInitialised = 0;


/*****************************************************
Called once when state is entered
*****************************************************/
static void StateInit(void)
{
	// Mark state as initialised
	stateInitialised = 1;
}


// Update menu lobby
static void UpdateMenuLobby(void) {
	
	// First state entry
	if (!stateInitialised)
		StateInit();
	
	// If circle is pressed, return to main menu
	if (BTN_PRESSED(PADcircle))
        gameState = STATE_MENU_MAIN;
	


}


// Render menu lobby
static void RenderMenuLobby(void) {
	
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
	FontFX_SetPulse(0, 255, 20);
	FontFX_Print(20, 60, "WAITING FOR OTHER PLAYER...", &WorldOrderingTable[activeBuffer], OT_UI);
	FontFX_FontEnd();
	
}


void StateMenuLobby(void)
{
	UpdateMenuLobby();
	RenderMenuLobby();
}