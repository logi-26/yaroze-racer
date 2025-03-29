#include "controller.h"
#include "pad.h"
#include "world.h"
#include "game.h"

u_long PADstatus = 0;





void InitialiseController() {
	
}





void ProcessUserInput() {
	
	PADstatus=PadRead();
	
	if (PADstatus & PAD1select)PLAYING = 0;
	
	
	
	// Player 1 direction
	if (PADstatus & PAD1left) {
		RotateModel(&player1.gsObjectCoord, &player1.rotation, 0, -32, -0);
	}
	if (PADstatus & PAD1right) {
		RotateModel(&player1.gsObjectCoord, &player1.rotation, 0, 32, 0);
	}
	
	
	
	
	// Player 2 direction
	if (PADstatus & PAD2left) {
		RotateModel(&player2.gsObjectCoord, &player2.rotation, 0, -32, -0);
	}
	if (PADstatus & PAD2right) {
		RotateModel(&player2.gsObjectCoord, &player2.rotation, 0, 32, 0);
	}
	
	
	
	
	
	// Player 1 forwards and backwards movement
	if (PADstatus & PAD1cross) {
		AdvanceModel(&player1.gsObjectCoord, &player1.rotation, &player1.speed, 1, 1);
	}
	else if (PADstatus & PAD1square) {
		AdvanceModel(&player1.gsObjectCoord, &player1.rotation, &player1.speed, -1, 1);
	}
	else {
		AdvanceModel(&player1.gsObjectCoord, &player1.rotation, &player1.speed, 0, 1);
	}
	
	
	
	// Player 2 forwards and backwards movement
	if (PADstatus & PAD2cross) {
		AdvanceModel(&player2.gsObjectCoord, &player2.rotation, &player2.speed, 1, 1);
	}
	else if (PADstatus & PAD2square) {
		AdvanceModel(&player2.gsObjectCoord, &player2.rotation, &player2.speed, -1, 1);
	}
	else {
		AdvanceModel(&player2.gsObjectCoord, &player2.rotation, &player2.speed, 0, 1);
	}






	if (PADstatus & PAD1start) {
		player1.gsObjectCoord.coord=GsIDMATRIX;
        player1.gsObjectCoord.coord.t[1] =- 200;
		player1.speed = 0;
		player1.rotation.vx = 0;  
		player1.rotation.vy = 0;  
		player1.rotation.vy = 0;
        player1.gsObjectCoord.flg = 0;
	}



	// Camera
	if (PADstatus & PAD1L1) {
		InitSplitScreen(0, 0, 160, 240, 160, 0, 160, 240);
	}
	if (PADstatus & PAD1L2) {
		InitSplitScreen(0, 0, 320, 120, 0, 120, 320, 120);
	}
}