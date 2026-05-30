#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <libps.h>

// Config
#define KB_ROWS 4
#define KB_COLS 10
#define KB_MAX_INPUT 16
#define KB_KEY_W 18
#define KB_KEY_H 14
#define KB_KEY_PAD 2

// Results
#define KB_RESULT_NONE 0
#define KB_RESULT_CONFIRM 1
#define KB_RESULT_CANCEL 2

// Theme IDs
#define MAX_THEMES 8

#define KB_THEME_DEFAULT   0
#define KB_THEME_RED       1
#define KB_THEME_DARK      2
#define KB_THEME_RETRO     3
#define KB_THEME_YELLOW    4
#define KB_THEME_WHITE     5
#define KB_THEME_PURPLE    6
#define KB_THEME_GREEN     7


/*****************************************************
Keyboard Theme
*****************************************************/
typedef struct
{
	// Font style
    int textFontStyle;
	int textLowerFontStyle;
    int infoFontStyle;
	
	// Widget background colour
    unsigned char wigetR;
    unsigned char wigetG;
    unsigned char wigetB;

	// Input background colour
    unsigned char inputBgR;
    unsigned char inputBgG;
    unsigned char inputBgB;

	// Input text colour
    unsigned char inputTextR;
    unsigned char inputTextG;
    unsigned char inputTextB;
	
	// Keyboard background colour
    unsigned char bgR;
    unsigned char bgG;
    unsigned char bgB;

	// Key button background colour (none selected)
    unsigned char keyButtonR;
    unsigned char keyButtonG;
    unsigned char keyButtonB;

	// Key button background colour (selected)
    unsigned char selectedKeyButtonR;
    unsigned char selectedKeyButtonG;
    unsigned char selectedKeyButtonB;

	// Standard character text colour
    unsigned char textR;
    unsigned char textG;
    unsigned char textB;
	
	// OK character text colour
    unsigned char okCharR;
    unsigned char okCharG;
    unsigned char okCharB;

	// Backspace character text colour
    unsigned char bsCharR;
    unsigned char bsCharG;
    unsigned char bsCharB;

	// Info text
	unsigned char infoTextR;
    unsigned char infoTextG;
    unsigned char infoTextB;

} KBTheme;

extern KBTheme kbTheme;


/************* FUNCTION PROTOTYPES *******************/

// Core functions
void KB_Init(void);
void KB_Open(void);
int KB_Update(void);
void KB_Render(int x, int y, GsOT *ot, int priority);
void KB_RenderAtStoredPos(GsOT *ot, int priority);

// Input functions
const char *KB_GetInput(void);
int KB_GetInputLength(void);

// Position functions
void KB_SetPosition(int x, int y);
void KB_GetPosition(int *x, int *y);

// Theme functions
KBTheme KB_GetTheme(void);
void KB_SetTheme(KBTheme style);
void KB_SetPredefinedTheme(int themeID);
void KB_DefaultTheme(KBTheme *s);

/*****************************************************/

#endif