#ifndef HEADLIGHTS_H
#define HEADLIGHTS_H

#include <libps.h>
#include "player.h"

// RAM addresses of the headlight files
#define HEADLIGHT_GLOW_TEX_MEM_ADDR  (0x80137000)   // data/game/headlight/glow.tim
#define HEADLIGHT_BEAM_TEX_MEM_ADDR  (0x80138240)   // data/game/headlight/beam.tim
#define HEADLIGHT_BEAM_MEM_ADDR      (0x80138300)   // data/game/headlight/BEAM.TMD


/************* FUNCTION PROTOTYPES *******************/
void InitialiseHeadlights(void);
void DrawHeadlights(PlayerStruct *viewer, int currentBuffer);
void HeadlightsOn(long x, long z, PlayerStruct *self);
void HeadlightsOff(void);
/*****************************************************/

#endif
