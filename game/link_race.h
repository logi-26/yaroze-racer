#ifndef LINK_RACE_H
#define LINK_RACE_H

#include <libps.h>

/****************************************
Link race: two players, one on each console, and no AI racers. 
Each console drives its own car (player1) and draws the other 
player's car (player2) where the other console says it is.
****************************************/


/************* FUNCTION PROTOTYPES *******************/
void LinkRace_Begin(void);
int LinkRace_IsActive(void);
void LinkRace_GetStart(long *x, long *z);
void LinkRace_InitOpponent(unsigned long *tmdAddr);
void LinkRace_Update(void);
int LinkRace_CanLeave(void);
void LinkRace_End(void);
void LinkRace_DrawHUD(GsOT *ot);
/*****************************************************/

#endif
