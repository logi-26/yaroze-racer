#include "../menu_memcard_load.h"

#include "../engine/memcard_context.h"
#include "../engine/state_manager.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/controller.h"
#include "../engine/audio.h"
#include "../engine/graphics.h"
#include "../engine/memcard.h"
#include "../engine/ui.h"
#include "../engine/message.h"
#include "../engine/keyboard.h"
#include "../engine/lang.h"

#define EMPTY_SLOT_TEXT "EMPTY SLOT"
#define MENU_ITEM_COUNT 6
#define MAX_MENU_ITEMS SAVE_SLOT_COUNT

static int stateInitialised = 0;
static int selectedItem;

// Deferred save state
static int animationTimer = 0;
static int saveStage = 0;
static int pendingOverwriteSlot = -1;

// For the message dialogue box
static int lastMsgActive = 0;

// Keyboard state
static int showKeyboard = 0;
static char saveFileName[KB_MAX_INPUT + 1] = {0};

// Menu items
static char menuItems[MAX_MENU_ITEMS][32];
static int menuSlotUsed[MAX_MENU_ITEMS];
static int menuSlotMap[MAX_MENU_ITEMS];

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
	// Reset the selected menu index
	selectedItem = 0;

	// Load the save slot data into the menu
	LoadSaveSlots();

	// Initialise the keyboard
	KB_Init();

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


/*****************************************************
Load save slot data into the menu
*****************************************************/
static void LoadSaveSlots(void)
{
    int i;

	// Failed to load memory card
    if (!CardLoad(memcardContext.selectedSlot))
    {
		// Display a message if there was an issue reading the memory card
		Message_Show(TXT(TXT_FAILED_READ_MEMCARD));
		
        for (i = 0; i < SAVE_SLOT_COUNT; i++)
        {
            strcpy(menuItems[i], TXT(TXT_EMPTY_SLOT));

            menuSlotUsed[i] = 0;
            menuSlotMap[i] = i;
        }

        return;
    }

	// Loop through the save slots
    for (i = 0; i < SAVE_SLOT_COUNT; i++)
    {
		// Get the data from the save slot
        SaveSlot *slot = GetSlot(i);

		 // Valid occupied slot
        if (slot && slot->used)
        {
            printf("Slot used: %d\n", slot->used);
            printf("Slot number: %d\n", slot->slotNumber);
            printf("Slot title: %s\n", slot->title);

            strcpy(menuItems[i], slot->title);

            menuSlotUsed[i] = 1;
        }
        else
        {
            strcpy(menuItems[i], TXT(TXT_EMPTY_SLOT));

            menuSlotUsed[i] = 0;
        }

        menuSlotMap[i] = i;
    }
}


/*****************************************************
Update
*****************************************************/
static void UpdateMenuMemcardSave(void)
{
	// Get the countdown timer value
	int remaining = CountdownGetRemainingFrames();
	
	// Update the countdown timer
	CountdownUpdate();
	
	/*****************************************************
    Deferred save execution
    *****************************************************/
	
	// If a save is pending and the countdown timer has ended
	if (saveStage == 1 && remaining <= 0)
    {
		// Write the data into the selected save slot
        //CardWriteSlot(memcardContext.selectedSlot, pendingOverwriteSlot, 666, saveFileName);
		
		SaveSlot slotData;
		memset(&slotData, 0, sizeof(SaveSlot));

		slotData.used = 1;
		slotData.value1 = 10;
		slotData.value2 = 20;
		slotData.value3 = 30;
		
		strncpy(slotData.title, saveFileName, 31);
		slotData.title[31] = '\0';
		
		CardWriteSlot(memcardContext.selectedSlot, pendingOverwriteSlot, &slotData);
		
		// Reset save state
        saveFileName[0] = '\0';
        pendingOverwriteSlot = -1;
		saveStage = 0;
		
		// Re-load the slot data into the menu (to see the new save)
		LoadSaveSlots();
		
        // Close the popup message box
        Message_Dismiss();
        return;
    }
	
    // First state entry
	if (!stateInitialised)
		StateInit();
		
	/*****************************************************
	Handle menu input
	*****************************************************/
	if (!showKeyboard && !Message_IsActive())
	{
		// Return to main menu
		if (BTN_PRESSED(PADcircle))
		{
			// Deinitialise the state
			StateDeinitialise();
			
			// Return to the main menu
			gameState = STATE_MENU_MAIN;
		}

		// Select slot
		if (BTN_PRESSED(PADcross))
		{
			int slot = menuSlotMap[selectedItem];

			if (slot >= 0)
			{
				// Get the save slot
				SaveSlot *s = GetSlot(slot);

				// Occupied slot
				if (s && s->used)
				{
					pendingOverwriteSlot = slot;

					// Set the message box theme
					MSG_SetPredefinedTheme(MSG_THEME_DEFAULT);

					// Display a yes/no message box
					Message_Prompt(TXT(TXT_OVERWRITE_SAVE_SLOT), MSG_YES_NO);
				}
				else
				{
					// Empty slot
					pendingOverwriteSlot = slot;

					// Display the on-screen-keyboard
					if (!showKeyboard)
					{
						KB_SetPosition(60, 100);
						KB_Open();
						showKeyboard = 1;
					}
				}
			}
		}

		// Up
		if (BTN_PRESSED(PADup))
		{
			selectedItem = (selectedItem - 1 + SAVE_SLOT_COUNT) % SAVE_SLOT_COUNT;
			PlaySFX(1, 60, 127, 127);
		}

		// Down
		if (BTN_PRESSED(PADdown))
		{
			selectedItem = (selectedItem + 1) % SAVE_SLOT_COUNT;
			PlaySFX(1, 60, 127, 127);
		}
	}


	/*****************************************************
	Message box response
	*****************************************************/
	if (!Message_IsActive() && lastMsgActive)
	{
		// Get the result from the message box
		int result = Message_GetResult();

		// If result is Yes
		if (result == MSG_RESULT_YES)
		{
			printf("Overwrite slot: %d\n", pendingOverwriteSlot);

			// Display the on-screen-keyboard
			if (!showKeyboard)
			{
				KB_SetPosition(60, 100);
				KB_Open();
				showKeyboard = 1;
			}
		}
		// If result is No
		else if (result == MSG_RESULT_NO)
			pendingOverwriteSlot = -1;
	}

	lastMsgActive = Message_IsActive();


	/*****************************************************
	Keyboard input
	*****************************************************/
	if (showKeyboard)
	{
		int result = KB_Update();

		// Confirm
		if (result == KB_RESULT_CONFIRM)
		{
			// Get the text from the keyboard
			strncpy(saveFileName, KB_GetInput(), KB_MAX_INPUT);

			// Add the null terminator
			saveFileName[KB_MAX_INPUT] = '\0';

			printf("Queued save: %s\n", saveFileName);

			// Close keyboard
			showKeyboard = 0;

			// Begin the save process
			saveStage = 1;
			
			// Display a message showing that the save is being created
			Message_ShowTimed(TXT(TXT_CREATING_SAVE_FILE), 1);
			
			// Countdown timer to start the save process once the message is rendered
			CountdownStart(MSG_ANIM_FRAMES);
		}
		// Cancel
		else if (result == KB_RESULT_CANCEL)
		{
			// Hide the keyboard
			showKeyboard = 0;
		}
	}

	// Update popup system
	Message_Update();
}


/*****************************************************
Render
*****************************************************/
static void RenderMenuMemcardSave(void)
{
	int i;

	// Title
	FontFX_FontBegin();
	FontFX_SetStyle(FONT_STYLE_2);
	FontFX_SetSize(2);
	FontFX_SetColour(COL_DARKGREEN);
	FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
	FontFX_Print(80, 20, TXT(TXT_SAVE_TITLE), &WorldOrderingTable[activeBuffer], OT_UI);
	FontFX_SetSize(1);
	FontFX_FontEnd();

	// Background box (black semi-transparant)
	DrawRect(50, 60, 220, 120, COL_BLACK, TRANS_HALF, &WorldOrderingTable[activeBuffer], OT_UI + 1);

	// Menu items
	for (i = 0; i < SAVE_SLOT_COUNT; i++)
	{
		if (selectedItem == i)
		{
			// Menu highlight box (white semi-transparant)
			DrawRect(menuX[i] - 5, menuY[i] - 6, 220, 20, COL_WHITE, TRANS_HALF, &WorldOrderingTable[activeBuffer], OT_UI + 1);

			// If the slot is empty use green font
			if (strcmp(menuItems[i], EMPTY_SLOT_TEXT) == 0)
			{
				FontFX_FontBegin();
				FontFX_SetColour(COL_DARKGREEN);
				FontFX_SetPulse(0, 255, 20);
				FontFX_Print(menuX[i], menuY[i], menuItems[i], &WorldOrderingTable[activeBuffer], OT_UI);
				FontFX_FontEnd();
			}
			// If the slot is occupied use red font
			else
			{
				FontFX_FontBegin();
				FontFX_SetColour(COL_RED);
				FontFX_SetPulse(0, 255, 20);
				FontFX_Print(menuX[i], menuY[i], menuItems[i], &WorldOrderingTable[activeBuffer], OT_UI);
				FontFX_FontEnd();
			}
		}
		else
		{
			// None selected item
			if (strcmp(menuItems[i], EMPTY_SLOT_TEXT) == 0)
				FontFX_PrintOutline(menuX[i], menuY[i], menuItems[i], FONT_STYLE_2, COL_GREEN, COL_DARKGREY, &WorldOrderingTable[activeBuffer], OT_UI);
			else
				FontFX_PrintOutline(menuX[i], menuY[i], menuItems[i], FONT_STYLE_2, COL_RED, COL_WHITE, &WorldOrderingTable[activeBuffer], OT_UI);
		}
	}

	// Render the keyboard if it is active
	if (showKeyboard && saveStage == 0)
		KB_RenderAtStoredPos(&WorldOrderingTable[activeBuffer], OT_POPUP);

	// Render the message box popup if it is active
	Message_Render(&WorldOrderingTable[activeBuffer], OT_POPUP);
}


/*****************************************************
Memory Card Save Menu State Manager
*****************************************************/
void StateMenuMemcardSave(void)
{
	UpdateMenuMemcardSave();
	RenderMenuMemcardSave();
}