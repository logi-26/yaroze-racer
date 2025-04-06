#include "controller.h"
#include "pad.h"
#include "world.h"
#include "game.h"
#include "graphics.h"

u_long padStatus = 0;

void ProcessUserInput() {
	
	int controllerPort1;
	int controllerPort2;
	
	// Read
	padStatus = PadRead();
	
	//FntPrint(fontID_1, "\n\n\n\n\n\nLeft:  H = %3d V = %3d\n\nRight: H = %3d V = %3d\n", PAD1lh, PAD1lv, PAD1rh, PAD1rv);
	
	// Check the controller type in each port
	controllerPort1 = CheckPort1();
	controllerPort2 = CheckPort2();
	
	// If port 1 has a standard controller or analogue controller connected
	if (controllerPort1 == 3 || controllerPort1 == 4) {
		ProcessControllerPort1();
	}
	// Negcon controller
	else if (controllerPort1 == 2) {
		ProcessNegconPort1();
	}
	
	// If port 2 has a standard controller or analogue controller connected
	if (controllerPort2 == 3 || controllerPort2 == 4) {
		ProcessControllerPort2();
	}
	
	// Reset player positions (for testing)
    if (padStatus & PAD1start) {
        player1.gsObjectCoord.coord = GsIDMATRIX;
        player1.gsObjectCoord.coord.t[1] = -200;
        player1.speed = 0;
        player1.rotation.vx = 0;  
        player1.rotation.vy = 0;  
        player1.rotation.vz = 0;
        player1.gsObjectCoord.flg = 0;
		
		player2.gsObjectCoord.coord = GsIDMATRIX;
        player2.gsObjectCoord.coord.t[1] = -200;
        player2.speed = 0;
        player2.rotation.vx = 0;  
        player2.rotation.vy = 0;  
        player2.rotation.vz = 0;
        player2.gsObjectCoord.flg = 0;
    }

	// End the game
    if (padStatus & PAD1select) {
        PLAYING = 0;
    }
}

void ProcessControllerPort1() {
	
	// Direction (buttons and sticks)
    if (padStatus & PAD1left || PAD1lh < 80) {
        RotateModel(&player1.gsObjectCoord, &player1.rotation, 0, -32, 0, &player1.speed);
    }
    if (padStatus & PAD1right || PAD1lh > 140) {
        RotateModel(&player1.gsObjectCoord, &player1.rotation, 0, 32, 0, &player1.speed);
    }
	
	// Movement (buttons and sticks)
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

void ProcessControllerPort2() {
	
	// Direction (buttons)
	if (padStatus & PAD2left) {
        RotateModel(&player2.gsObjectCoord, &player2.rotation, 0, -32, 0, &player2.speed);
    }
	if (padStatus & PAD2right) {
        RotateModel(&player2.gsObjectCoord, &player2.rotation, 0, 32, 0, &player2.speed);
    }
	
	// Movement (buttons)
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

void ProcessNegconPort1() {
	
	// Direction (buttons)
	if (padStatus & NEGCON1_LEFT) {
        RotateModel(&player1.gsObjectCoord, &player1.rotation, 0, -32, 0, &player1.speed);
    }
	if (padStatus & NEGCON1_RIGHT) {
        RotateModel(&player1.gsObjectCoord, &player1.rotation, 0, 32, 0, &player1.speed);
    }
	
	// Movement (buttons)
	if (padStatus & NEGCON1_A) {
        AdvanceModel(&player1.gsObjectCoord, &player1.rotation, &player1.speed, 1, 1);
    }
	else if (padStatus & NEGCON1_B) {
        AdvanceModel(&player1.gsObjectCoord, &player1.rotation, &player1.speed, -1, 1);
    }
	else {
        AdvanceModel(&player1.gsObjectCoord, &player1.rotation, &player1.speed, 0, 1);
    }
}