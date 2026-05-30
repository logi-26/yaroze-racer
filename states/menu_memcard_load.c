#include "../menu_memcard_load.h"

#include "../engine/memcard_context.h"
#include "../engine/state_manager.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/controller.h"
#include "../engine/audio.h"
#include "../engine/graphics.h"
#include "../engine/memcard.h"
#include "../engine/message.h"
#include "../engine/ui.h"
#include "../engine/lang.h"

#define MENU_ITEM_COUNT 6
#define MAX_MENU_ITEMS SAVE_SLOT_COUNT

static int stateInitialised = 0;
static int selectedItem;


// Menu items
static char menuItems[MAX_MENU_ITEMS][32];
static int menuSlotUsed[MAX_MENU_ITEMS];
static int menuSlotMap[MAX_MENU_ITEMS];

static int menuItemCount = 0;


// Menu item X values
static const short menuX[MENU_ITEM_COUNT] = 
{
	55, 55, 55, 55, 55, 55
};


// Menu item Y values
static const short menuY[MENU_ITEM_COUNT] = 
{
	66, 86, 106, 126, 146, 166
};


/*****************************************************
Called once when state is entered
*****************************************************/
static void StateInit(void)
{
	// Reset the menu index
	selectedItem = 0;
	
	// Load the save slots from the memory card
	LoadSaveSlots();
	
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


/*****************************************************
Load save slots from memory card
*****************************************************/
static void LoadSaveSlots(void)
{
    int i;

    menuItemCount = 0;

    printf("loading\n");

	// Load the selected memory card port
    if (!CardLoad(memcardContext.selectedSlot))
    {
        printf("CardLoad failed\n");
		
		// Show popup message
        Message_Show(TXT(TXT_FAILED_READ_MEMCARD));
		
		 // Add placeholder entry
        strcpy(menuItems[0], TXT(TXT_NO_SAVE_DATA));

        menuSlotUsed[0] = 0;
        menuSlotMap[0] = -1;

        menuItemCount = 1;
		
        return;
    }

	// Loop through the save slots in the block
    for (i = 0; i < SAVE_SLOT_COUNT; i++)
    {
		// Get the slot at this index
        SaveSlot *slot = GetSlot(i);

		// If the slot contains data
        if (slot && slot->used)
        {
            printf("Slot used: %d\n", slot->used);
            printf("Slot number: %d\n", slot->slotNumber);
            printf("Slot title: %s\n", slot->title);
            printf("Slot value1: %d\n", slot->value1);
			printf("Slot value2: %d\n", slot->value2);
			printf("Slot value3: %d\n", slot->value3);

			// Copy save title into menu
            strcpy(menuItems[menuItemCount], slot->title);

            menuSlotUsed[menuItemCount] = 1;
            menuSlotMap[menuItemCount] = i;

            menuItemCount++;
        }
    }

	// No save data found
    if (menuItemCount == 0)
    {
        strcpy(menuItems[0], TXT(TXT_NO_SAVE_DATA));
        menuSlotUsed[0] = 0;
        menuSlotMap[0] = -1;
        menuItemCount = 1;
    }
}


static void UpdateMenuMemcardLoad(void)
{
    // First state entry
	if (!stateInitialised)
		StateInit();
	
	// If circle is pressed, return to the main menu
	if (BTN_PRESSED(PADcircle))
	{
		// Deinitialise the state
		StateDeinitialise();
		
		// Return to the main menu
		gameState = STATE_MENU_MAIN;
	}
	
	// Load save
	if (BTN_PRESSED(PADcross))
	{
		// The slot index is based on the menu item index
		int slot = menuSlotMap[selectedItem];

		if (slot >= 0)
		{
			// Get the save slot
			SaveSlot *s = GetSlot(slot);

			// LOAD THE GAME STATE HERE!!!
			if (s && s->used)
			{
				printf("Loading save value=%d\n", s->value1);
			}
		}
	}
	
	// If the up button is pressed traverse the menu upwatds
	if (BTN_PRESSED(PADup))
	{
		selectedItem = (selectedItem - 1 + menuItemCount) % menuItemCount;
		PlaySFX(1, 60, 127, 127);
	}
	
	// If the down button is pressed traverse the menu downwards
	if (BTN_PRESSED(PADdown))
	{
		selectedItem = (selectedItem + 1) % menuItemCount;
		PlaySFX(1, 60, 127, 127);
	}
}


static void RenderMenuMemcardLoad(void)
{
	int i;

	// Display the load title
	FontFX_FontBegin();
	FontFX_SetStyle(FONT_STYLE_2);
	FontFX_SetSize(2);
	FontFX_SetColour(COL_DARKGREEN);
	FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
	FontFX_Print(80, 20, TXT(TXT_LOAD_TITLE), &WorldOrderingTable[activeBuffer], OT_UI);
	FontFX_SetSize(1);
	FontFX_FontEnd();

	// Draw the semi-transparent black box
	DrawRect(50, 60, 220, 120, COL_BLACK, TRANS_HALF, &WorldOrderingTable[activeBuffer], OT_UI +1);
	
	// Display the menu items
	for (i = 0; i < menuItemCount; i++)
	{
        if (selectedItem == i) 
		{
			// Highlight the selected item using a semi-transparent white box
			DrawRect(menuX[i] -5, menuY[i] -6, 220, 20, COL_WHITE, TRANS_HALF, &WorldOrderingTable[activeBuffer], OT_UI +1);
			
            FontFX_FontBegin();
            FontFX_SetColour(COL_DARKGREEN);
            FontFX_SetPulse(0, 255, 20);
            FontFX_Print(menuX[i], menuY[i], menuItems[i], &WorldOrderingTable[activeBuffer], OT_UI);
            FontFX_FontEnd();
        } 
		else 
		{
            // Normal item
            FontFX_PrintOutline(menuX[i], menuY[i], menuItems[i], FONT_STYLE_2, COL_GREEN, COL_DARKGREY, &WorldOrderingTable[activeBuffer], OT_UI);
        }
    }
}


void StateMenuMemcardLoad(void)
{
	UpdateMenuMemcardLoad();
	RenderMenuMemcardLoad();
}