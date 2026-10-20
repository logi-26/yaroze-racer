#include "memcard_context.h"

// Used to pass the selected slot between the memory card states
MemcardContext memcardContext;


// Go to the memory card menus to load or save and come back to returnState afterwards
void Memcard_Open(int mode, GameState returnState)
{
    memcardContext.mode = mode;
    memcardContext.returnState = returnState;
    gameState = STATE_MENU_MEMCARD;
}


// Where to go back to after the memory card menus (the main menu if it hasn't been set)
GameState Memcard_ReturnState(void)
{
    return (memcardContext.returnState == STATE_INIT) ? STATE_MENU_MAIN : memcardContext.returnState;
}