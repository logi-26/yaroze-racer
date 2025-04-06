#ifndef SKYBOX_H
#define SKYBOX_H

#include <libps.h>

typedef struct {
    GsSPRITE sprite;
    u_short tpage;
    SVECTOR position;
    int scale;
    u_long tim_addr;
} Skybox;

extern Skybox skybox;

/************* FUNCTION PROTOTYPES *******************/
void Skybox_Init(Skybox* skybox, u_long tim_addr);
void Skybox_Draw(Skybox* skybox, GsRVIEW2* view, GsOT* ot);
/*****************************************************/

#define SKYBOX_TIM_ADDR (0x801A4000) 

#endif