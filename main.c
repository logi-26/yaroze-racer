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

// Local variables
unsigned long PLAYING = 1;


int main() {
	// Setup the font system
	InitialiseFontSystem();
	
	// Setup the controller pad
	PadInit();
	
	// Initialise the graphics, lights and view 
	InitialiseGraphics();
	InitialiseAllLights();
	InitialiseStaticView(&viewTop, 250, 0, 0, -500, -1000, 0, 0, 0);
	
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