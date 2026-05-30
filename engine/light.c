#include <libps.h>
#include "light.h"

GsF_LIGHT flLights[2];

void InitialiseLight(GsF_LIGHT *flLight, int nLight, int nX, int nY, int nZ, int nRed, int nGreen, int nBlue) {
    flLight->vx = nX;
    flLight->vy = nY;
    flLight->vz = nZ;
    flLight->r  = nRed;
    flLight->g  = nGreen;
    flLight->b  = nBlue;
    GsSetFlatLight(nLight, flLight);
}