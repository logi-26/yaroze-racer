#include <stdio.h>
#include <stdlib.h>
#include <libps.h>
#include <string.h>
#include "main.h"
#include "world.h"
#include "light.h"
#include "graphics.h"
#include "player.h"
#include "model.h"
#include "game.h"
#include "controller.h"

PlayerStruct player1;
PlayerStruct player2;
unsigned long PLAYING = 1;

int main() {
	// Setup the font system
	InitialiseFontSystem();
	
	// Setup the controller pad
	PadInit();
	
	// Initialise the graphics, lights and view 
	InitialiseGraphics();
	
	InitialisePlayer(&player1, 1, 3605, -200, 9273, (long*)CAR_MEM_ADDR);
	InitialisePlayer(&player2, 2, 2448, -200, 9273, (long*)CAR_MEM_ADDR);
	
	// Initialise the views (use horizontal split-screen in 2 player mode)
	if (NumberOfPlayers == 2) {
		InitSplitScreen(0, 0, 320, 120, 0, 120, 320, 120);
		InitialiseTrackerViewPlayer1(&Camera[0], 250, 0, 0, -500, -2200, 0, -200, 0);
		InitialiseTrackerViewPlayer2(&Camera[1], 250, 0, 0, -500, -2200, 0, -200, 0);
	}
	else {
		InitSingleScreen();
		InitialiseTrackerViewPlayer1(&Camera[0], 250, 0, 0, -500, -1500, 0, -200, 0);
	}
	
	// Initialise the lights 
	InitialiseAllLights();

	// Initialise the models in the world
	InitialiseWorldModels();
	
	// Load the world textures into VRAM
	InitialiseWorldTextures();
	
	// Initialise the world
	InitialiseWorld();
	
	// Run the game loop
	while(PLAYING) {
		// Handle user input and render the game world
		ProcessUserInput();
		RenderWorld();
	}
	
	// Clean up
	ResetGraph(0);
	return 0;
}