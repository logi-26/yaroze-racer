/******************************************************************************
    NET YAROZE: MESSAGE DIALOGUE MODULE
    ------------------------------------
    Displays animated popup messages with optional prompts.
 
    Usage:
 
    Simple message (auto-dismisses):
        Message_Show("GAME SAVED SUCCESSFULLY");
 
    Yes/No prompt:
        Message_Prompt("OVERWRITE SAVE SLOT?", MSG_YES_NO);
 
    Check result each frame:
        int result = Message_GetResult();
        if (result == MSG_RESULT_YES)  { ... }
        if (result == MSG_RESULT_NO)   { ... }
        if (result == MSG_RESULT_OK)   { ... }
******************************************************************************/
 
#ifndef MESSAGE_H
#define MESSAGE_H
 
#include <libps.h>

#include "graphics.h"

// Config
#define MSG_MAX_LEN      	 64    				// maximum message string length
#define MSG_POPUP_W      	 220   				// popup width in pixels
#define MSG_POPUP_H      	 50    				// popup height for simple message
#define MSG_PROMPT_H     	 70   				// popup height for yes/no prompt
#define MSG_ANIM_FRAMES  	 10    				// number of frames for open animation
#define MSG_HOLD_FRAMES  	 70   				// frames a simple message stays visible
#define MSG_SCREEN_CX    	 SCREEN_WIDTH /2   	// horizontal centre of screen
#define MSG_SCREEN_CY    	 SCREEN_HEIGHT /2   // vertical centre of screen
#define MSG_TEXT_PADDING 	 8
#define MSG_BUTTON_PADDING 	 12

// Prompt types
#define MSG_NONE         0     // auto-dismiss message
#define MSG_YES_NO       1     // yes / no prompt
#define MSG_OK_CANCEL    2     // ok / cancel prompt
#define MSG_OK           3     // single OK button

// Results
#define MSG_RESULT_NONE   0    // no result yet — still active
#define MSG_RESULT_YES    1    // user selected Yes or OK
#define MSG_RESULT_NO     2    // user selected No or Cancel
#define MSG_RESULT_OK     3    // user dismissed a simple message

// Theme IDs
#define MSG_THEME_DEFAULT   0
#define MSG_THEME_RED       1
#define MSG_THEME_DARK      2
#define MSG_THEME_RETRO     3
#define MSG_THEME_WHITE     4
#define MSG_THEME_GREEN     5

#define MSG_MAX_THEMES      6 


/*****************************************************
MESSAGE THEME
*****************************************************/

typedef struct
{
    // Font styles
    int textFontStyle;
    int infoFontStyle;

    // Popup background
    unsigned char bgR;
    unsigned char bgG;
    unsigned char bgB;

    // Border
    unsigned char borderR;
    unsigned char borderG;
    unsigned char borderB;

    // Main message text
    unsigned char textR;
    unsigned char textG;
    unsigned char textB;

    // Selected button background
    unsigned char selectedButtonR;
    unsigned char selectedButtonG;
    unsigned char selectedButtonB;

    // Unselected button background
    unsigned char buttonR;
    unsigned char buttonG;
    unsigned char buttonB;

    // Selected button text
    unsigned char selectedTextR;
    unsigned char selectedTextG;
    unsigned char selectedTextB;

    // Normal button text
    unsigned char buttonTextR;
    unsigned char buttonTextG;
    unsigned char buttonTextB;

} MSGTheme;

extern MSGTheme msgTheme;


/************* FUNCTION PROTOTYPES *******************/
void Message_Init(void);
void Message_Show(const char *text);
void Message_Prompt(const char *text, int promptType);
void Message_Update(void);
void Message_Render(GsOT *ot, int priority);
int Message_IsActive(void);
int Message_GetResult(void);
void Message_Dismiss(void);

void Message_ShowTimed(const char *text, int frames);

// Theme functions
void MSG_DefaultTheme(MSGTheme *t);
void MSG_SetTheme(MSGTheme theme);
MSGTheme MSG_GetTheme(void);
void MSG_SetPredefinedTheme(int themeID);
/*****************************************************/

#endif /* MESSAGE_H */