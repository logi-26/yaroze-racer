#include <libps.h>
#include "engine/state_manager.h"


int main(void)
{
	// Initialise the game
    GameInit();
	
	// Run the game
    while (gameState != STATE_EXIT)
        GameUpdate();

    // Exit the game
    GameExit();

    return 0;
}