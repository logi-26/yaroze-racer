#include <stdio.h>
#include <stdlib.h>
#include <libps.h>
#include <string.h>
#include "main.h"
#include "game/world.h"
#include "game/light.h"
#include "game/graphics.h"
#include "game/player.h"
#include "game/model.h"
#include "game/game.h"

//#include "controller.h"
//#include "padio.h"

PlayerStruct player1;
PlayerStruct player2;
unsigned long PLAYING = 1;

int main() {
	// Setup the font system
	InitialiseFontSystem();
	
	// Setup the controller pad
	//PadInit();
	//PadioInit();
	
	// Initialise the graphics, lights and view 
	InitialiseGraphics();
	
	InitialisePlayer(&player1, 1, 3605, -200, 9273, (long*)CAR_3_MEM_ADDR);
	{
		SVECTOR modelRot = {0, 0, 0, 0};
		RotModel(&player1.gsModelCoord, &modelRot, 3072, 2048, 0);
	}
	InitialisePlayer(&player2, 2, 2448, -200, 9273, (long*)CAR_2_MEM_ADDR);
	
	// Initialise the views (use horizontal split-screen in 2 player mode)
	if (NumberOfPlayers == 2) {
		InitSplitScreen(0, 0, 320, 120, 0, 120, 320, 120); // Horizontal (Only select 1 of these)
		//InitSplitScreen(0, 0, 160, 240, 160, 0, 160, 240); // Vertical   (Only select 1 of these)
		InitialiseTrackerViewPlayer1(&Camera[0], 250, 0, 0, -500, -2200, 0, -200, 0);
		InitialiseTrackerViewPlayer2(&Camera[1], 250, 0, 0, -500, -2200, 0, -200, 0);
	}
	else {
		InitSingleScreen();
		InitialiseTrackerViewPlayer1(&Camera[0], 250, 0, 0, -500, -1500, 0, -200, 0);
		//InitialiseFrontViewPlayer1(&Camera[0], 250, 0, 0, -500, -1000, 0, -800, 0);
		//InitialiseStaticView(&Camera[0], 250, 0, 0, -500, -1000, 0, -800, 0);
		//InitialiseTopDownView(&Camera[0], 250, 0, 0, 0);
	}
	
	// Initialise the lights 
	InitialiseAllLights();

	// Initialise the world
	InitialiseWorld();
	
	// Run the game loop
	while(PLAYING) {
		//ProcessUserInput();
		CheckWorldCollisions(&player1, &player1_lateralSpeed);
		if (NumberOfPlayers == 2) {
			CheckWorldCollisions(&player2, &player2_lateralSpeed);
		}
		RenderWorld();
	}
	
	// Clean up
	ResetGraph(0);
	return 0;
}