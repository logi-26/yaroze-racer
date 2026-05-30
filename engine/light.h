#ifndef ENGINE_LIGHT_H
#define ENGINE_LIGHT_H

#include <libps.h>

extern GsF_LIGHT flLights[2];

void InitialiseLight(GsF_LIGHT *flLight, int nLight, int nX, int nY, int nZ, int nRed, int nGreen, int nBlue);

#endif