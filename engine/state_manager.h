#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#define REGION_PAL   1
#define REGION_NTSCU 2
#define REGION_NTSCJ 3

// Define the game name used in the save file
// This must not be greater than 6 chars (need to confirm this)
#define GAME_NAME "MYGAME"

// Define the game region (based on above definitions)
#define GAME_REGION REGION_PAL


/****************************************
Game States
****************************************/

// Game states
typedef enum
{
    STATE_INIT,
    STATE_MENU_MAIN,
	STATE_MENU_MEMCARD,
	STATE_MENU_MEMCARD_LOAD,
	STATE_MENU_MEMCARD_SAVE,
	STATE_MENU_OPTIONS,
	STATE_MENU_LOBBY,
	STATE_MENU_VEHICLE_SELECT,
	STATE_MENU_PAUSE,
    STATE_GAMEPLAY,
    STATE_GAMEOVER,
    STATE_EXIT
} GameState;


/****************************************
Shared Globals
****************************************/

extern int activeBuffer;
extern int frameNumber;

extern GameState gameState;
extern GameState prevGameState;

extern int score;
extern int hiScore;
extern int lives;


/************* FUNCTION PROTOTYPES *******************/
void GameInit(void);
void GameUpdate(void);
void GameExit(void);

void StartNewGame(void);

void InitialiseGameObjects(void);
void InitialiseAllElements(void);
/*****************************************************/

#endif