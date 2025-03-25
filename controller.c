#include "controller.h"
#include "pad.h"
#include "world.h"

u_long PADstatus = 0;

void ProcessUserInput() {
	
	PADstatus=PadRead();
	
	if (PADstatus & PADselect)PLAYING = 0;
	if (PADstatus & PADLleft) {
		RotateModel(&theCar.gsObjectCoord, &theCar.rotation, 0, -32, -0, &theCar.speed);
	}
	if (PADstatus & PADLright) {
		RotateModel(&theCar.gsObjectCoord, &theCar.rotation, 0, 32, 0, &theCar.speed);
	}
	if (PADstatus & PADstart) {
		theCar.gsObjectCoord.coord=GsIDMATRIX;
        theCar.gsObjectCoord.coord.t[1] =- 200;
		theCar.speed = 0;
		theCar.rotation.vx = 0;  
		theCar.rotation.vy = 0;  
		theCar.rotation.vy = 0;
        theCar.gsObjectCoord.flg = 0;
	}
	
	// Cross moves the vehicle forwards and Square moves it backwards
	if (PADstatus & PADcross) {
		AdvanceModel(&theCar.gsObjectCoord, &theCar.rotation, &theCar.speed, 1);
	}
	else if (PADstatus & PADsquare) {
		AdvanceModel(&theCar.gsObjectCoord, &theCar.rotation, &theCar.speed, -1);
	}
	else {
		AdvanceModel(&theCar.gsObjectCoord, &theCar.rotation, &theCar.speed, 0);
	}
	
	
	if (PADstatus & PADL1) {
		InitialiseStaticView(&viewTop, 250, 0, 0, -500, -1000, 0, 0, 0);		// W
	}
	if (PADstatus & PADL2) {
		InitialiseTrackerView(&viewTop, 250, 0, 0, -300, -1500, 0, 200, 0);		// E
	}
	if (PADstatus & PADR1) {
		InitialiseTopDownView(&viewTop, 250, -3000, 0, 0);						// R
	}
	if (PADstatus & PADR2) {
		InitialiseTopDownView(&viewTop, 250, -6000, 0, 0);						// T
	}
	
}