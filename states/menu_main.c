#include "../menu_main.h"

#include "../engine/state_manager.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/controller.h"
#include "../engine/audio.h"
#include "../engine/graphics.h"
#include "../engine/lang.h"

#define MENU_ITEM_COUNT 5

static int stateInitialised = 0;
static int selectedItem;


// Menu items
static const TextID menuItems[MENU_ITEM_COUNT] =
{
    TXT_ONE_PLAYER,
	TXT_TWO_PLAYER,
    TXT_LINK_GAME,
    TXT_OPTIONS,
    TXT_QUIT_GAME
};


static const short menuY[MENU_ITEM_COUNT] = 
{
	100, 115, 130, 145, 160
};


static const short menuX[MENU_ITEM_COUNT] = 
{
	120, 120, 125, 120, 120
};


/*****************************************************
Called once when state is entered
*****************************************************/
static void StateInit(void)
{
	// Reset the selected menu index
	selectedItem = 0;
	
	SetBackgroundColor(COL_LIGHTBLUE);
	
	// Mark state as initialised
	stateInitialised = 1;
}


// Check if the menu item is selectable (is the controller inserted)
static int IsControllerMenuItemSelectable(int index)
{
    switch (index)
    {
        case 0:
            return Ctrl_IsGamepad(1);

        case 1:
            return Ctrl_IsGamepad(2);

        case 2:
            return 1; // "Link Game" is always selectable
		
		case 3:
            return 1; // "Options" is always selectable
		
		case 4:
            return 1; // "Quit Game" is always selectable
    }

    return 0;
}


static void UpdateMenuMain(void) 
{
	// First state entry
	if (!stateInitialised)
		StateInit();
	
	// If the up button is pressed traverse the menu upwards
	if (BTN_PRESSED(PADup))
	{
		int attempts = 0;
		
		// Prevent the item from being selected if the controller is not connected
		do
		{
			selectedItem = (selectedItem - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;
			attempts++;
		} while (!IsControllerMenuItemSelectable(selectedItem) && attempts < MENU_ITEM_COUNT);
	}
	
	
	// If the down button is pressed traverse the menu downwards
	if (BTN_PRESSED(PADdown))
	{
		int attempts = 0;
		
		// Prevent the item from being selected if the controller is not connected
		do
		{
			selectedItem = (selectedItem + 1) % MENU_ITEM_COUNT;

			if (selectedItem >= MENU_ITEM_COUNT)
				selectedItem = 0;
			
			attempts++;

		} while (!IsControllerMenuItemSelectable(selectedItem) && attempts < MENU_ITEM_COUNT);
	}
	

	// If the X button is pressed
    if (BTN_PRESSED(PADcross))
	{
		switch (selectedItem)
		{
			case 0:  StartNewGame();  		  			break;
			case 1:  StartNewGame();		  			break;
			case 2:  gameState = STATE_MENU_LOBBY;		break;
			case 3:  gameState = STATE_MENU_OPTIONS;	break;
			case 4:  gameState = STATE_EXIT;  			break;
			default:             		 	  			break;
		}
	}
}


static void RenderMenuMain(void) 
{
    int i;

    // Display the game title
    FontFX_FontBegin();
    FontFX_SetStyle(FONT_STYLE_2);
    FontFX_SetSize(2);
    FontFX_SetColour(COL_DARKBLUE);
    FontFX_SetOutline(COL_WHITE);
    FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
    FontFX_SetWave(1, 2, 2);
    FontFX_Print(12, 20, TXT(TXT_GAME_TITLE), &WorldOrderingTable[activeBuffer], OT_UI);
    FontFX_SetSize(1);
    FontFX_FontEnd();

    // Display the menu items
    for (i = 0; i < MENU_ITEM_COUNT; i++) 
    {
        // Set alpha based on whether the item is selectable
        int alpha = IsControllerMenuItemSelectable(i) ? 1 : 0;

        if (selectedItem == i) 
        {
            FontFX_FontBegin();
            FontFX_SetColour(COL_DARKGREEN);
            FontFX_SetOutline(COL_LIGHTGREY);
            FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
            FontFX_SetPulse(0, 255, 20);
            FontFX_SetAlpha(alpha);
            FontFX_Print(menuX[i], menuY[i], TXT(menuItems[i]), &WorldOrderingTable[activeBuffer], OT_UI);
            FontFX_SetAlpha(1);
            FontFX_FontEnd();
        } 
        else 
        {
            FontFX_FontBegin();
            FontFX_SetColour(COL_GREEN);
            FontFX_SetOutline(COL_DARKGREY);
            FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
            FontFX_SetAlpha(alpha);
            FontFX_Print(menuX[i], menuY[i], TXT(menuItems[i]), &WorldOrderingTable[activeBuffer], OT_UI);
            FontFX_SetAlpha(1);
            FontFX_FontEnd();
        }
    }
}


void StateMenuMain(void)
{
    UpdateMenuMain();
    RenderMenuMain();
}