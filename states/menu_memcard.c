#include "../menu_options.h"

#include "../engine/memcard_context.h"
#include "../engine/memcard.h"
#include "../engine/state_manager.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/controller.h"
#include "../engine/audio.h"
#include "../engine/graphics.h"
#include "../engine/lang.h"

#define MENU_MEMCARD_ITEM_COUNT 3


static int stateInitialised = 0;


// Memory card menu state
typedef struct
{
    int selectedItem;
    long slotAStatus;
    long slotBStatus;
} MenuMemCardState;

static MenuMemCardState menuMemcardState;


// Memory card menu items
static const TextID menuMemcardItems[MENU_MEMCARD_ITEM_COUNT] =
{
    TXT_MEMCARD_SLOT_A,
    TXT_MEMCARD_SLOT_B,
    TXT_MEMCARD_DONT_LOAD
};


static const short menuMemcardItemsY[MENU_MEMCARD_ITEM_COUNT] =
{
    100, 115, 130
};


static const short menuMemcardItemsX[MENU_MEMCARD_ITEM_COUNT] =
{
    130, 130, 130
};


/*****************************************************
Called once when state is entered
*****************************************************/
static void StateInit(void)
{
	// Reset the menu index
    menuMemcardState.selectedItem = 0;
		
	// Set the state initialised flag
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


// Check if the menu item is selectable (is the card inserted)
static int IsMenuItemSelectable(int index)
{
    switch (index)
    {
        case 0:
            return (menuMemcardState.slotAStatus == 1);

        case 1:
            return (menuMemcardState.slotBStatus == 1);

        case 2:
            return 1; // "Do not load" is always selectable
    }

    return 0;
}


static void UpdateMenuMemcard(void)
{
	// First state entry
    if (!stateInitialised)
		StateInit();

	// Get the memory card status
	menuMemcardState.slotAStatus = TestCard(0);
	menuMemcardState.slotBStatus = TestCard(1);
	
	// If the up button is pressed traverse the menu upwards
	if (BTN_PRESSED(PADup))
	{
		// Prevent the item from being selected if the memory card is not inserted
		do
		{
			menuMemcardState.selectedItem--;

			if (menuMemcardState.selectedItem < 0)
				menuMemcardState.selectedItem = MENU_MEMCARD_ITEM_COUNT - 1;

		} while (!IsMenuItemSelectable(menuMemcardState.selectedItem));

		PlaySFX(1, 60, 127, 127);
	}

	// If the down button is pressed traverse the menu downwards
	if (BTN_PRESSED(PADdown))
	{
		// Prevent the item from being selected if the memory card is not inserted
		do
		{
			menuMemcardState.selectedItem++;

			if (menuMemcardState.selectedItem >= MENU_MEMCARD_ITEM_COUNT)
				menuMemcardState.selectedItem = 0;

		} while (!IsMenuItemSelectable(menuMemcardState.selectedItem));

		PlaySFX(1, 60, 127, 127);
	}
	
	// If cross is pressed
	if (BTN_PRESSED(PADcross))
	{
		switch (menuMemcardState.selectedItem)
		{
			case 0:
				// Load Slot A
				StateDeinitialise();
				memcardContext.selectedSlot = 0;
				gameState = STATE_MENU_MEMCARD_LOAD;
				break;

			case 1:
				// Load Slot B
				StateDeinitialise();
				memcardContext.selectedSlot = 1;
				gameState = STATE_MENU_MEMCARD_LOAD;
				//gameState = STATE_MENU_MEMCARD_SAVE;
				break;

			case 2:
				// Do not load - return to the main menu
				StateDeinitialise();
				gameState = STATE_MENU_MAIN;
				break;
		}

		PlaySFX(1, 60, 127, 127);
	}
}


static void RenderMenuMemcard(void)
{
	int i;

	FontFX_FontBegin();
	FontFX_SetColour(COL_DARKGREEN);
	FontFX_SetStyle(FONT_STYLE_2);
	FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
	FontFX_Print(0, 30, TXT(TXT_MEMCARD_LOAD_WHICH), &WorldOrderingTable[activeBuffer], OT_UI);
	FontFX_FontEnd();
	
	// Display the menu items
    for (i = 0; i < MENU_MEMCARD_ITEM_COUNT; i++) 
	{
		int alpha = 1;
		
		// If slot A has no valid memory card inserted, set the alpha value
		if (i == 0 && menuMemcardState.slotAStatus != 1)
			alpha = 0;

		// If slot B has no valid memory card inserted, set the alpha value
		else if (i == 1 && menuMemcardState.slotBStatus != 1)
			alpha = 0;
		
        if (menuMemcardState.selectedItem == i) {
            // Highlight the selected item
            FontFX_FontBegin();
            FontFX_SetColour(COL_DARKGREEN);
            FontFX_SetOutline(COL_LIGHTGREY);
            FontFX_SetPulse(0, 255, 20);
			FontFX_SetAlpha(alpha);
            FontFX_Print(menuMemcardItemsX[i], menuMemcardItemsY[i], TXT(menuMemcardItems[i]), &WorldOrderingTable[activeBuffer], OT_UI);
			FontFX_SetAlpha(1);
            FontFX_FontEnd();
        
		} else {
			// Normal item
			FontFX_FontBegin();
            FontFX_SetColour(COL_DARKGREEN);
			FontFX_SetAlpha(alpha);
            FontFX_Print(menuMemcardItemsX[i], menuMemcardItemsY[i], TXT(menuMemcardItems[i]), &WorldOrderingTable[activeBuffer], GOT_UI);
			FontFX_SetAlpha(1);
            FontFX_FontEnd();	
        }
    }
}


void StateMenuMemcard(void)
{
	UpdateMenuMemcard();
    RenderMenuMemcard();
}