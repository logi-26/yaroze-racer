#ifndef SKY_H
#define SKY_H

#include <libps.h>

#define SKY_TEX_MEM_ADDR  (0x80136000)  // sky2.tim
#define SKY_ALT_TEX_MEM_ADDR (0x800E4000)  // sky.tim


/************* FUNCTION PROTOTYPES *******************/
void InitialiseSky(u_long texMemAddr);
void DrawSky(GsOT *ot);
/*****************************************************/


#endif