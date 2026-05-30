#include "../menu_pause.h"

#include "../engine/state_manager.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/controller.h"
#include "../engine/audio.h"
#include "../engine/graphics.h"
#include "../engine/memcard.h"


// Update pause screen
static void UpdatePauseScreen(void) {
	// If the start button is pressed, resume the game
    if (BTN_PRESSED(PADstart))
    {
        ResumeMusic();
        gameState = STATE_GAMEPLAY;
    }

	// If the select button is pressed, exit the game
    if (BTN_PRESSED(PADselect))
    {
        ResumeMusic();
        gameState = STATE_EXIT;
    }
}


// Render pause screen
static void RenderPauseScreen(void) {
	char scoreStr[6];
	char livesStr[6];

	// Display the lives and score
	FontFX_PrintWithoutEffects(10, 40, "SCORE:", FONT_STYLE_0, COL_DARKGREEN, &WorldOrderingTable[activeBuffer], OT_UI);
	FontFX_PrintWithoutEffects(10, 50, "LIVES:", FONT_STYLE_0, COL_DARKGREEN, &WorldOrderingTable[activeBuffer], OT_UI);
	
	sprintf(scoreStr, "%d", score);
	FontFX_PrintWithoutEffects(10 + (6 * FONT_CHAR_W), 40, scoreStr, FONT_STYLE_0, COL_WHITE, &WorldOrderingTable[activeBuffer], OT_UI);
	
	sprintf(livesStr, "%d", lives);
	FontFX_PrintWithoutEffects(10 + (6 * FONT_CHAR_W), 50, livesStr, FONT_STYLE_0, COL_WHITE, &WorldOrderingTable[activeBuffer], OT_UI);
	
	// Display the pause text
	FontFX_FontBegin();
	FontFX_SetStyle(FONT_STYLE_0);
	FontFX_SetColour(COL_RED);
	FontFX_SetSize(2);
	FontFX_SetOutline(COL_WHITE);
	FontFX_Print(105, 90, "PAUSED", &WorldOrderingTable[activeBuffer], OT_UI);
	FontFX_SetSize(1);
	FontFX_FontEnd();
	
	// Display the controls
	FontFX_PrintFlash(80, 190, "PRESS START TO RESUME", FONT_STYLE_0, COL_PURPLE, 0, 255, 20, &WorldOrderingTable[activeBuffer], OT_UI);
	FontFX_PrintFlash(100, 205, "SELECT TO EXIT", FONT_STYLE_0, COL_PURPLE, 0, 255, 20, &WorldOrderingTable[activeBuffer], OT_UI);
}


void StateMenuPause(void)
{
	UpdatePauseScreen();
	RenderPauseScreen();
}