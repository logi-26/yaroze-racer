#include <libps.h>

#include "state_manager.h"
#include "graphics.h"
#include "controller.h"
#include "audio.h"
#include "memcard.h"
#include "colours.h"
#include "font.h"
#include "timer.h"
#include "ui.h"
#include "message.h"
#include "profiler.h"
#include "asset_manager.h"

// Game states
#include "../states/menu_main.h"
#include "../states/menu_memcard.h"
#include "../states/menu_memcard_load.h"
#include "../states/menu_options.h"
#include "../states/menu_lobby.h"
#include "../states/menu_vehicle_select.h"
#include "../states/menu_pause.h"
#include "../states/gameplay.h"

#define ROUND_FRAMES (FRAME_RATE * 60)

// Variables for game engine
int activeBuffer = 0;
int frameNumber  = 0;
int gameMs       = 0;

// Variables for the game sample
int score = 0;
int hiScore = 0;
int lives = 3;

// Current/previous game state
GameState gameState = STATE_INIT;
GameState prevGameState = STATE_INIT;


// Add game objects here for initialisation
void InitialiseGameObjects(void)
{
}


// Initialise all game elements
void InitialiseAllElements(void)
{
	// Initialise the graphics system
    InitialiseGraphicsSystem();
	
	// Initialise the controller system
	Ctrl_Init();
	
	// Initialise all of the game objects
    InitialiseGameObjects();
}


// Reset the game variables for a new game
void StartNewGame(void) {
	score = 0;
	lives = 3;
	CountdownStart(ROUND_FRAMES);
	
	SetVolume(32, 32);
	ResumeMusic();
	PlayMusic();
	//gameState = STATE_GAMEPLAY;
	gameState = STATE_MENU_VEHICLE_SELECT;
}


void CheckMemoryLayout(void)
{
    // Verify no memory regions overlap
    printf("VH:    %08X - %08X\n", MEM_VH_ADDR,    MEM_VH_ADDR    + MEM_VH_SIZE);
    printf("VB:    %08X - %08X\n", MEM_VB_ADDR,    MEM_VB_ADDR    + MEM_VB_SIZE);
    printf("SEQ:   %08X - %08X\n", MEM_SEQ_ADDR,   MEM_SEQ_ADDR   + MEM_SEQ_SIZE);
    printf("FONT:  %08X - %08X\n", MEM_FONT_ADDR,  MEM_FONT_ADDR  + MEM_FONT_SIZE);
    printf("ICON1: %08X - %08X\n", MEM_ICON1_ADDR, MEM_ICON1_ADDR + MEM_ICON1_SIZE);
    printf("POOL:  %08X - %08X\n", MEM_ASSET_POOL_START, MEM_ASSET_POOL_END);
}


/*****************************************************
Public Interface
*****************************************************/

// Initialise the game
void GameInit(void)
{
    InitialiseAllElements();
	
	// Start in fullscreen mode
	SetScreenMode(SCREEN_MODE_FULLSCREEN);
	
	// Load font pixel data from RAM
	FontFX_LoadFont();
	
	// Ensure that there are no memory overlaps
	CheckMemoryLayout();
	
	// Initialise the profiler
	Profiler_Init();
	
    hiScore = 0;

	// Display the game menu
    gameState = STATE_MENU_MAIN;
}


// Update the game
void GameUpdate(void)
{
	frameNumber++;
	
	// Get the buffer when the game is updated 
	activeBuffer = GetActiveBuffer();
	

	/*
	// Set the screen view mode (fullscreen or split-screen)
	if (gScreenMode == SCREEN_MODE_FULLSCREEN) {
		GsSetRefView2(&Camera[0]);
		GsSetWorkBase((PACKET*)GpuPacketArea[buf]);
		// draw everything
	} else {
		// Top half - player 1
		GsSetRefView2(&Camera[0]);
		GsSetWorkBase((PACKET*)GpuPacketArea[buf]);
		// draw player 1 view

		// Bottom half - player 2
		GsSetRefView2(&Camera[1]);
		GsSetWorkBase((PACKET*)GpuPacketArea[buf + 2]);
		// draw player 2 view
	}
	*/
	
	
	
	
	// Begin the profiler
	Profiler_BeginFrame();
	
	// Display the FPS
	//DisplayFPS(260, 5);
	
	// Update the font system
	FontFX_Update();

	// Update the controller module
	Ctrl_Update();
	
	
	
	// Toggle profiler on/off
	if (BTN_HELD(PADselect) && BTN_PRESSED(PADL1))
		Profiler_Toggle();

	// Cycle pages while profiler is visible
	if (BTN_HELD(PADselect) && BTN_PRESSED(PADL2))
	{
		Profiler_SetPage(prof.page + 1);
	}



	// Profile the state update
    PROFILE_BEGIN(PROF_UPDATE);
	
	// Free all game assets when changing state
	if (gameState != prevGameState)
		Asset_FreeAll();

	// Game State Manager
    switch (gameState)
    {
        case STATE_MENU_MAIN:     	   		StateMenuMain();     		break;
		case STATE_MENU_MEMCARD:       		StateMenuMemcard();     	break;
		case STATE_MENU_MEMCARD_LOAD:  		StateMenuMemcardLoad();  	break;
		case STATE_MENU_MEMCARD_SAVE:  		StateMenuMemcardSave();  	break;
		case STATE_MENU_OPTIONS:       		StateMenuOptions();     	break;
		case STATE_MENU_LOBBY:    	   		StateMenuLobby();			break;
		case STATE_MENU_VEHICLE_SELECT: 	StateMenuVehicleSelect();	break;
		case STATE_MENU_PAUSE:    	   		StateMenuPause();    		break;
		case STATE_GAMEOVER: 		   		StateGameOver(); 			break;
		case STATE_GAMEPLAY: 				StateGameplay(); 			break;
        default:             						 					break;
    }
	
	// End the state profiler
	PROFILE_END(PROF_UPDATE);
	
	// Store the previous game state
	prevGameState = gameState;
	
	// Render the profiler overlay
    Profiler_Render(&WorldOrderingTable[activeBuffer], OT_POPUP);
	
	// Profile the graphics flush
    PROFILE_BEGIN(PROF_RENDER);
	
	gameMs += UpdateGraphicsSystem(activeBuffer) * (1000 / FRAME_RATE);
	FntFlush(-1);

	PROFILE_END(PROF_RENDER);
	
	// End the profiler
	Profiler_TickVSync();
    Profiler_EndFrame();   
}


// Exit the game
void GameExit(void)
{
    StopMusic();

    // Wait for GPU to finish
    DrawSync(0);
    VSync(0);

    // Reset graphics system
    ResetGraph(3);
}