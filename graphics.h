#ifndef GRAPHICS_H
#define GRAPHICS_H

#define ORDERING_TABLE_LENGTH (12)
#define SCREEN_WIDTH (320)
#define SCREEN_HEIGHT (240)

extern GsOT othWorld[2]; 									
extern GsOT_TAG otWorld[2][1<<ORDERING_TABLE_LENGTH]; 

extern int fontID_1;

/************* FUNCTION PROTOTYPES *******************/
void InitialiseGraphics();
void InitialiseFontSystem();
/*****************************************************/

#endif  // GRAPHICS_H