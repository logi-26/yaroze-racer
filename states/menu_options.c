#include "../menu_options.h"

#include "../engine/state_manager.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/controller.h"
#include "../engine/audio.h"
#include "../engine/graphics.h"
#include "../engine/memcard.h"
#include "../engine/ui.h"
#include "../engine/lang.h"

#include <string.h>

#define MENU_OPTIONS_ITEM_COUNT 4
#define LANG_ITEM 0
#define MUSIC_ITEM 1
#define SFX_ITEM 2
#define LOAD_ITEM 3

static int stateInitialised = 0;
static int selectedItem;

static const TextID menuOptionItems[MENU_OPTIONS_ITEM_COUNT] =
{
	TXT_LANGUAGE,
    TXT_MUSIC_VOLUME,
    TXT_SFX_VOLUME,
	TXT_LOAD_GAME
};


static short menuOptionsY[MENU_OPTIONS_ITEM_COUNT] = 
{
	65, 80, 95, 110
};


static short menuOptionsX[MENU_OPTIONS_ITEM_COUNT] = 
{
	20, 20, 20, 20
};


// Struct to get the current/max volumes 
typedef struct
{
    int (*get)(void);
    int max;
} VolumeOption;

VolumeOption options[] =
{
    { GetVolume,    VOL_MAX     },
    { GetSFXVolume, SFX_VOL_MAX }
};


/*****************************************************
Called once when state is entered
*****************************************************/
static void StateInit(void)
{
	// Reset the selected menu index
	selectedItem = 0;

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


static void UpdateMenuOptions(void)
{
	// First state entry
	if (!stateInitialised)
		StateInit();
	
	// If circle is pressed, return to main menu
	if (BTN_PRESSED(PADcircle))
	{
		// Deinitialise the state
		StateDeinitialise();
		
		// Switch to the main menu
        gameState = STATE_MENU_MAIN;
	}
	
	// If the up button is pressed traverse the menu upwatds
	if (BTN_PRESSED(PADup))
		selectedItem = (selectedItem - 1 + MENU_OPTIONS_ITEM_COUNT) % MENU_OPTIONS_ITEM_COUNT;
	
	// If the down button is pressed traverse the menu downwards
	if (BTN_PRESSED(PADdown))
		selectedItem = (selectedItem + 1) % MENU_OPTIONS_ITEM_COUNT;
	
	// If the left/right buttons are pressed, adjust the volume
	if (BTN_PRESSED(PADright) || BTN_PRESSED(PADleft))
	{
		int direction = BTN_PRESSED(PADright) ? 1 : -1;

		switch (selectedItem)
		{
			case LANG_ITEM:
				currentLanguage = (currentLanguage + (BTN_PRESSED(PADright) ? 1 : -1) + LANG_COUNT) % LANG_COUNT;
				break;

			case MUSIC_ITEM:
				if (BTN_PRESSED(PADright)) VolumeUp();
				else VolumeDown();
				break;

			case SFX_ITEM:
				if (BTN_PRESSED(PADright)) SFXVolumeUp();
				else SFXVolumeDown();
				break;
		}
	}	
	
	if (BTN_PRESSED(PADcross))
	{
		if (selectedItem == LOAD_ITEM)
			gameState = STATE_MENU_MEMCARD;
	}
}


static void RenderMenuOptions(void)
{
	int i = 0;
	
	// Display the options title
	FontFX_FontBegin();
	FontFX_SetStyle(FONT_STYLE_2);
	FontFX_SetSize(2);
	FontFX_SetColour(COL_DARKGREEN);
	FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
	FontFX_Print(20, 20, TXT(TXT_OPTIONS_TITLE), &WorldOrderingTable[activeBuffer], OT_UI);
	FontFX_SetSize(1);
	FontFX_FontEnd();
	
	// Display the options menu items
    for (i = 0; i < MENU_OPTIONS_ITEM_COUNT; i++) {
		int optionIndex;
		int vol;
		int maxVol;

		// Language menu item
		if (i == LANG_ITEM)
		{
			if (selectedItem == i)
			{
				FontFX_FontBegin();
				FontFX_SetColour(COL_DARKGREEN);
				FontFX_SetOutline(COL_LIGHTGREY);
				FontFX_SetPulse(0, 255, 20);
				FontFX_Print(menuOptionsX[i], menuOptionsY[i],TXT(menuOptionItems[i]), &WorldOrderingTable[activeBuffer], OT_UI);
				FontFX_FontEnd();
			}
			else
			{
				FontFX_PrintOutline(menuOptionsX[i], menuOptionsY[i], TXT(menuOptionItems[i]), FONT_STYLE_2, COL_GREEN, COL_DARKGREY, &WorldOrderingTable[activeBuffer], OT_UI);
			}
			
			// Draw country flag
			switch (currentLanguage)
			{
				case LANG_ENGLISH:
					DrawFlagEngland(215, menuOptionsY[i] - 6, 1, &WorldOrderingTable[activeBuffer], OT_UI);
					break;

				case LANG_FRENCH:
					DrawFlagFrance(215, menuOptionsY[i] - 6, 1, &WorldOrderingTable[activeBuffer], OT_UI);
					break;

				case LANG_DUTCH:
					DrawFlagNetherlands(215, menuOptionsY[i] - 6, 1, &WorldOrderingTable[activeBuffer], OT_UI);
					break;

				case LANG_GERMAN:
					DrawFlagGermany(215, menuOptionsY[i] - 6, 1, &WorldOrderingTable[activeBuffer], OT_UI);
					break;
			}
			
			continue;
		}
		
		// Load menu item
		if (i == LOAD_ITEM)
		{
			if (selectedItem == i)
			{
				FontFX_FontBegin();
				FontFX_SetColour(COL_DARKGREEN);
				FontFX_SetOutline(COL_LIGHTGREY);
				FontFX_SetPulse(0, 255, 20);
				FontFX_Print(menuOptionsX[i], menuOptionsY[i],TXT(menuOptionItems[i]), &WorldOrderingTable[activeBuffer], OT_UI);
				FontFX_FontEnd();
			}
			else
			{
				FontFX_PrintOutline(menuOptionsX[i], menuOptionsY[i], TXT(menuOptionItems[i]), FONT_STYLE_2, COL_GREEN, COL_DARKGREY, &WorldOrderingTable[activeBuffer], OT_UI);
			}
			
			continue;
		}
		
		// Shift the index
		optionIndex = i - 1;
		
		// Get the current/max volumes
		vol = options[optionIndex].get();
		maxVol = options[optionIndex].max;
		
		// Highlight the selected item
        if (selectedItem == i) {
            FontFX_FontBegin();
            FontFX_SetColour(COL_DARKGREEN);
            FontFX_SetOutline(COL_LIGHTGREY);
            FontFX_SetPulse(0, 255, 20);
            FontFX_Print(menuOptionsX[i], menuOptionsY[i], TXT(menuOptionItems[i]), &WorldOrderingTable[activeBuffer], OT_UI);
            FontFX_FontEnd();
        // Normal item
		} else {
            FontFX_PrintOutline(menuOptionsX[i], menuOptionsY[i], TXT(menuOptionItems[i]), FONT_STYLE_2, COL_GREEN, COL_DARKGREY, &WorldOrderingTable[activeBuffer], OT_UI);
        }
		
		// Draw the volume slider bar
		DrawBarSolid(180, menuOptionsY[i], 100, 8, vol, maxVol, COL_GREEN, COL_DARKGREY, &WorldOrderingTable[activeBuffer], OT_UI);
    }
}


void StateMenuOptions(void)
{
	UpdateMenuOptions();
	RenderMenuOptions();
}