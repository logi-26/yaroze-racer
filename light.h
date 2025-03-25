#ifndef LIGHT_H
#define LIGHT_H

#include <libps.h>

GsF_LIGHT flLights[2];	// This is an array of structures for the lights

/************* FUNCTION PROTOTYPES *******************/
void InitialiseLight(GsF_LIGHT *flLight, int nLight, int nX, int nY, int nZ,int nRed, int nGreen, int nBlue);
void InitialiseAllLights();
/*****************************************************/

#endif // LIGHT_H
