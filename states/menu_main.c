#include "menu_main.h"

#include "../engine/state_manager.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/controller.h"
#include "../engine/audio.h"
#include "../engine/graphics.h"
#include "../engine/lang.h"
#include "../game/game.h"
#include "../game/tournament.h"

/*****************************************************
The main menu has three pages of options (the main options, the one player game modes,
and the tournaments), side by side. Going to the next page, the page slides off to the
left while the next one slides in from the right (and the other way going back).
*****************************************************/

#define MENU_ITEM_COUNT 4
#define MODE_ITEM_COUNT 2
#define TOURNAMENT_ITEM_COUNT TOURNAMENT_COUNT

// How long a page takes to slide across (in frames)
#define SLIDE_FRAMES 10

enum { PAGE_MAIN, PAGE_MODES, PAGE_TOURNAMENTS };

static int stateInitialised = 0;
static int selectedItem = 0;       // On the main page
static int selectedMode;           // On the game modes page
static int selectedTournament = 0; // On the tournaments page

static int page;                   // The page the options are on (or are sliding to)
static int slide;                  // How far the pages have slid (SLIDE_FRAMES for each page)

// The page the race was chosen on (going back from the vehicle select returns to it)
static int chosenPage = PAGE_MAIN;
static int returnPage = PAGE_MAIN;
static int tournamentFinished = 0;     // A tournament has just finished (Tournament is selected on the game modes)


// Menu items
static const TextID menuItems[MENU_ITEM_COUNT] =
{
    TXT_ONE_PLAYER,
	TXT_TWO_PLAYER,
    TXT_LINK_GAME,
    TXT_OPTIONS
};


static const short menuY[MENU_ITEM_COUNT] =
{
	100, 115, 130, 145
};


// One player game modes
static const TextID modeItems[MODE_ITEM_COUNT] =
{
	TXT_SINGLE_RACE,
	TXT_TOURNAMENT
};


static const short modeY[MODE_ITEM_COUNT] =
{
	115, 130
};


// The tournaments (all day races, or all night races)
static const TextID tournamentItems[TOURNAMENT_ITEM_COUNT] =
{
	TXT_TOURNAMENT_1,
	TXT_TOURNAMENT_2
};


static const short tournamentY[TOURNAMENT_ITEM_COUNT] =
{
	115, 130
};


// Going back from the vehicle select (to the page the race was chosen on)
void MenuMainReturnFromVehicleSelect(void)
{
	returnPage = chosenPage;
}


// Returning to the main menu after a tournament has finished
void MenuMainReturnFromTournament(void)
{
	returnPage = PAGE_MODES;
	tournamentFinished = 1;
}


/*****************************************************
Called once when state is entered
*****************************************************/
static void StateInit(void)
{
	// Any tournament has ended (so it can't be carried on by mistake)
	Tournament_Clear();

	// The selected main option is kept from last time (and the selected tournament when going back to it)
	selectedMode = (returnPage == PAGE_TOURNAMENTS || tournamentFinished) ? 1 : 0;
	tournamentFinished = 0;

	// On the main page (or back on the page the race was chosen on)
	page = returnPage;
	slide = returnPage * SLIDE_FRAMES;
	returnPage = PAGE_MAIN;

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
    }

    return 0;
}


// Check if the game mode is selectable (they all are)
static int IsModeSelectable(int index)
{
	return index >= 0 && index < MODE_ITEM_COUNT;
}


// Check if the tournament is selectable (they all are at the moment)
static int IsTournamentSelectable(int index)
{
	return index >= 0 && index < TOURNAMENT_ITEM_COUNT;
}


// Move the selection up or down a list, past the items that cant be selected
static int MoveSelection(int selected, int direction, int count, int (*selectable)(int))
{
	int attempts = 0;

	do
	{
		selected = (selected + direction + count) % count;
		attempts++;
	} while (!selectable(selected) && attempts < count);

	return selected;
}


static void UpdateMainPage(void)
{
	// Up/down to traverse the menu (skipping the items whose controller isn't connected)
	if (BTN_PRESSED(PADup))
		selectedItem = MoveSelection(selectedItem, -1, MENU_ITEM_COUNT, IsControllerMenuItemSelectable);

	if (BTN_PRESSED(PADdown))
		selectedItem = MoveSelection(selectedItem, 1, MENU_ITEM_COUNT, IsControllerMenuItemSelectable);

	// If the X button is pressed
    if (BTN_PRESSED(PADcross))
	{
		// If the selected item is not the first one, slide to the game modes page
		if (selectedItem != 0)
			stateInitialised = 0;

		switch (selectedItem)
		{
			case 0:  page = PAGE_MODES; selectedMode = 0; 							break;	// Slide to the game modes
			case 1:  chosenPage = PAGE_MAIN; NumberOfPlayers = 2; StartNewGame();	break;	// Two player race
			case 2:  NumberOfPlayers = 1; gameState = STATE_MENU_LOBBY;				break;
			case 3:  gameState = STATE_MENU_OPTIONS;								break;
			default:             		 	  										break;
		}
	}
}


static void UpdateModesPage(void)
{
	if (BTN_PRESSED(PADup))
		selectedMode = MoveSelection(selectedMode, -1, MODE_ITEM_COUNT, IsModeSelectable);

	if (BTN_PRESSED(PADdown))
		selectedMode = MoveSelection(selectedMode, 1, MODE_ITEM_COUNT, IsModeSelectable);

	// Circle (slide back to the main options)
	if (BTN_PRESSED(PADcircle))
	{
		page = PAGE_MAIN;
		return;
	}

	if (BTN_PRESSED(PADcross))
	{
		switch (selectedMode)
		{
			case 0:  chosenPage = PAGE_MODES; stateInitialised = 0; NumberOfPlayers = 1; StartNewGame();	break;	// Single Race (the vehicle select)
			case 1:  page = PAGE_TOURNAMENTS;											break;	// Slide to the tournaments
			default:																	break;
		}
	}
}


static void UpdateTournamentsPage(void)
{
	if (BTN_PRESSED(PADup))
	{
		selectedTournament = MoveSelection(selectedTournament, -1, TOURNAMENT_ITEM_COUNT, IsTournamentSelectable);
	}

	if (BTN_PRESSED(PADdown))
	{
		selectedTournament = MoveSelection(selectedTournament, 1, TOURNAMENT_ITEM_COUNT, IsTournamentSelectable);
	}

	// Circle (slide back to the game modes)
	if (BTN_PRESSED(PADcircle))
	{
		page = PAGE_MODES;
		return;
	}

	// Start a new tournament (the vehicle select first)
	if (BTN_PRESSED(PADcross))
	{
		Tournament_Begin(selectedTournament);
		chosenPage = PAGE_TOURNAMENTS;
		stateInitialised = 0;
		NumberOfPlayers = 1;
		StartNewGame();
	}
}


static void UpdateMenuMain(void)
{
	int target;

	// First state entry
	if (!stateInitialised)
		StateInit();

	// While the pages slide, the buttons wait
	target = page * SLIDE_FRAMES;
	if (slide < target)
	{
		slide++;
		return;
	}
	if (slide > target)
	{
		slide--;
		return;
	}

	switch (page)
	{
		case PAGE_MAIN:   UpdateMainPage();          break;
		case PAGE_MODES:  UpdateModesPage();         break;
		default:          UpdateTournamentsPage();   break;
	}
}


// Draw a page of options, moved across the screen by offsetX
static void DrawItems(const TextID *items, const short *itemY, int count, int selected, int (*selectable)(int), int offsetX)
{
	int i;

	// Not on the screen
	if (offsetX <= -gScreenWidth || offsetX >= gScreenWidth)
	{
		return;
	}

	for (i = 0; i < count; i++)
	{
		// Set alpha based on whether the item is selectable
		int alpha = selectable(i) ? 1 : 0;

		FontFX_FontBegin();
		if (selected == i)
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
		FontFX_SetCenter(SCREEN_X_OFFSET + offsetX, gScreenWidth + offsetX);
		FontFX_SetAlpha(alpha);
		FontFX_Print(0, itemY[i], TXT(items[i]), &WorldOrderingTable[activeBuffer], OT_UI);
		FontFX_SetAlpha(1);
		FontFX_FontEnd();
	}
}


static void RenderMenuMain(void)
{
	int t, eased, shift;

    // Display the game title
	/*
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
	*/

	// How far the pages have slid
	// (eased in and out: t * t * (3 - 2t), with t from 0 to 256)
	t = (slide % SLIDE_FRAMES) * 256 / SLIDE_FRAMES;
	eased = t * t * (768 - 2 * t) >> 16;
	shift = (slide / SLIDE_FRAMES) * gScreenWidth + (gScreenWidth * eased >> 8);

	// The pages side by side: each one slides off to the left as the next comes in from the right
	DrawItems(menuItems, menuY, MENU_ITEM_COUNT, selectedItem, IsControllerMenuItemSelectable, -shift);
	DrawItems(modeItems, modeY, MODE_ITEM_COUNT, selectedMode, IsModeSelectable, gScreenWidth - shift);
	DrawItems(tournamentItems, tournamentY, TOURNAMENT_ITEM_COUNT, selectedTournament, IsTournamentSelectable, 2 * gScreenWidth - shift);
}


void StateMenuMain(void)
{
    UpdateMenuMain();
    RenderMenuMain();
}
