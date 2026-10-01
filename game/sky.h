#ifndef SKY_H
#define SKY_H

#include <libps.h>

#define SKY_TEX_MEM_ADDR  (0x800D7370)  // sky2.tim
#define SKY_ALT_TEX_MEM_ADDR (0x800A43E0)  // sky.tim


/************* FUNCTION PROTOTYPES *******************/
void InitialiseSky(u_long texMemAddr);
void DrawSky(GsOT *ot);
/*****************************************************/


#endif