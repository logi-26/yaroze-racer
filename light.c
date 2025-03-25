#include <libps.h>
#include "light.h"

void InitialiseLight(GsF_LIGHT *flLight, int nLight, int nX, int nY, int nZ, int nRed, int nGreen, int nBlue) {
	// Set the direction in which the light travels
	flLight->vx = nX;
	flLight->vy = nY;    
	flLight->vz = nZ;
	
	// Set the colour
	flLight->r = nRed; 
	flLight->g = nGreen; 
	flLight->b = nBlue;
	
	// Activate light
	GsSetFlatLight(nLight, flLight);
}

void InitialiseAllLights() {
	InitialiseLight(&flLights[0], 0, -100, -100, -100, 0xff, 0xff, 0xff);
	InitialiseLight(&flLights[1], 1, 1000, 1000, 1000, 0xcc, 0xcc, 0xcc);
	GsSetAmbient(0, 0, 0);
	GsSetLightMode(0);
}