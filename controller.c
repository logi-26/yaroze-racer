#include "controller.h"
#include "pad.h"
#include "world.h"
#include "game.h"
#include "graphics.h"

u_long padStatus = 0;

void ProcessUserInput() {

	// Read
	padStatus = PadRead();
	
	//FntPrint(fontID_1, "\n\n\n\n\n\nLeft:  H = %3d V = %3d\n\nRight: H = %3d V = %3d\n", PAD1lh, PAD1lv, PAD1rh, PAD1rv);

    // Process player 1 controls
	ProcessInputPort1();

	// Process player 2 controls
	ProcessInputPort2();

	// Reset player 1 position
    if (padStatus & PAD1start) {
        player1.gsObjectCoord.coord = GsIDMATRIX;
        player1.gsObjectCoord.coord.t[1] = -200;
        player1.speed = 0;
        player1.rotation.vx = 0;  
        player1.rotation.vy = 0;  
        player1.rotation.vz = 0;
        player1.gsObjectCoord.flg = 0;
    }

	// End the game
    if (padStatus & PAD1select) {
        PLAYING = 0;
    }
}

void ProcessInputPort1() {
	// Direction
    if (padStatus & PAD1left || PAD1lh < 80) {
        RotateModel(&player1.gsObjectCoord, &player1.rotation, 0, -32, 0, &player1.speed);
    }
    if (padStatus & PAD1right || PAD1lh > 140) {
        RotateModel(&player1.gsObjectCoord, &player1.rotation, 0, 32, 0, &player1.speed);
    }
	
	// Movement
    if (padStatus & PAD1cross || PAD1rv < 125) {
        AdvanceModel(&player1.gsObjectCoord, &player1.rotation, &player1.speed, 1, 1);
    }
    else if (padStatus & PAD1square || PAD1rv > 175) {
        AdvanceModel(&player1.gsObjectCoord, &player1.rotation, &player1.speed, -1, 1);
    }
    else {
        AdvanceModel(&player1.gsObjectCoord, &player1.rotation, &player1.speed, 0, 1);
    }
}

void ProcessInputPort2() {
	// Direction
	if (padStatus & PAD2left) {
        RotateModel(&player2.gsObjectCoord, &player2.rotation, 0, -32, 0, &player2.speed);
    }
	
	if (padStatus & PAD2right) {
        RotateModel(&player2.gsObjectCoord, &player2.rotation, 0, 32, 0, &player2.speed);
    }
	
	// Movement
	if (padStatus & PAD2cross) {
        AdvanceModel(&player2.gsObjectCoord, &player2.rotation, &player2.speed, 1, 1);
    }
    else if (padStatus & PAD2square) {
        AdvanceModel(&player2.gsObjectCoord, &player2.rotation, &player2.speed, -1, 1);
    }
    else {
        AdvanceModel(&player2.gsObjectCoord, &player2.rotation, &player2.speed, 0, 1);
    }
}