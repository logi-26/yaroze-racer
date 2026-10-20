#ifndef MEMCARD_CONTEXT_H
#define MEMCARD_CONTEXT_H

#include "state_manager.h"

// Loading or saving
#define MEMCARD_MODE_LOAD 0
#define MEMCARD_MODE_SAVE 1


// Memory card context to store the selected slot
typedef struct
{
    int selectedSlot;
    int saveValue;
    int mode;                   // MEMCARD_MODE_LOAD or MEMCARD_MODE_SAVE
    GameState returnState;      // Where to go back to after the memory card menus
} MemcardContext;


extern MemcardContext memcardContext;


/************* FUNCTION PROTOTYPES *******************/
void Memcard_Open(int mode, GameState returnState);
GameState Memcard_ReturnState(void);
/*****************************************************/


#endif