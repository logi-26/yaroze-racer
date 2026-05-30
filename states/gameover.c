#include "../gameover.h"

#include "../engine/state_manager.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/controller.h"
#include "../engine/audio.h"
#include "../engine/graphics.h"
#include "../engine/memcard.h"


// Update gameover screen
static void UpdateGameoverScreen(void) {
	
	// If the X button is pressed, return to start menu
    if (BTN_PRESSED(PADcross))
	{
        gameState = STATE_MENU_MAIN;
		RestartMusic();
		PauseMusic();
	}

	// If the select button is pressed, exit the game
    if (BTN_PRESSED(PADselect))
        gameState = STATE_EXIT;
}


// Render gameover screen
static void RenderGameoverScreen(void) {
	char scoreStr[6];
	char highScoreStr[6];

	// Display the score and high-score labels
	FontFX_PrintWithoutEffects(10, 40, "SCORE:", FONT_STYLE_0, COL_DARKGREEN, &WorldOrderingTable[activeBuffer], OT_UI);
	FontFX_PrintWithoutEffects(10, 50, "HIGH SCORE:", FONT_STYLE_0, COL_DARKGREEN, &WorldOrderingTable[activeBuffer], OT_UI);
	
	// Display the score and high-score values
	sprintf(scoreStr, "%d", score);
	FontFX_PrintWithoutEffects(10 + (6 * FONT_CHAR_W), 40, scoreStr, FONT_STYLE_0, COL_WHITE, &WorldOrderingTable[activeBuffer], OT_UI);
	
	sprintf(highScoreStr, "%d", hiScore);
	FontFX_PrintWithoutEffects(10 + (11 * FONT_CHAR_W), 50, highScoreStr, FONT_STYLE_0, COL_WHITE, &WorldOrderingTable[activeBuffer], OT_UI);

	// Display the gameover text
	FontFX_FontBegin();
	FontFX_SetStyle(FONT_STYLE_0);
	FontFX_SetColour(COL_RED);
	FontFX_SetSize(2);
	FontFX_SetOutline(COL_WHITE);
	FontFX_Print(90, 100, "GAME OVER", &WorldOrderingTable[activeBuffer], OT_UI);
	FontFX_SetSize(1);
	FontFX_FontEnd();
	
	// If the player has achieved a high score
    if (score > hiScore && score > 0)
	{
		FontFX_PrintWithoutEffects(10, 20, "NEW HI-SCORE", FONT_STYLE_0, COL_GOLD, &WorldOrderingTable[activeBuffer], OT_UI);
		FontFX_PrintWithoutEffects(55, 175, "PRESS O TO SAVE HIGH SCORE", FONT_STYLE_0, COL_ORANGE, &WorldOrderingTable[activeBuffer], OT_UI);
	}
	
	// Display the controls
	FontFX_PrintFlash(80, 190, "PRESS X TO RESTART", FONT_STYLE_0, COL_PURPLE, 0, 255, 20, &WorldOrderingTable[activeBuffer], OT_UI);
	FontFX_PrintFlash(100, 205, "SELECT TO EXIT", FONT_STYLE_0, COL_PURPLE, 0, 255, 20, &WorldOrderingTable[activeBuffer], OT_UI);
}


void StateGameOver(void)
{
	UpdateGameoverScreen();
	RenderGameoverScreen();
}