#include "message.h"
#include "font.h"
#include "controller.h"
#include "colours.h"

#include <string.h>


// State
static int msgActive = 0;
static int msgPromptType = MSG_NONE;
static int msgResult = MSG_RESULT_NONE;

// Animation/timer
static int msgAnimFrame = 0;
static int msgHoldTimer = 0;
static int msgHoldLimit = MSG_HOLD_FRAMES;

// Store the users selection (0 = left option, 1 = right option)
static int msgSelection = 0;

// Message text
static char msgText[MSG_MAX_LEN + 1];

// Unpack RGB macros
#define SET_MSG_BG(col) {unsigned char _c[]={col}; msgTheme.bgR=_c[0]; msgTheme.bgG=_c[1]; msgTheme.bgB=_c[2];}
#define SET_MSG_BORDER(col) {unsigned char _c[]={col}; msgTheme.borderR=_c[0]; msgTheme.borderG=_c[1]; msgTheme.borderB=_c[2];}
#define SET_MSG_TEXT(col) {unsigned char _c[]={col}; msgTheme.textR=_c[0]; msgTheme.textG=_c[1]; msgTheme.textB=_c[2];}
#define SET_MSG_BUTTON(col) {unsigned char _c[]={col}; msgTheme.buttonR=_c[0]; msgTheme.buttonG=_c[1]; msgTheme.buttonB=_c[2];}
#define SET_MSG_SELECTED(col) {unsigned char _c[]={col}; msgTheme.selectedButtonR=_c[0]; msgTheme.selectedButtonG=_c[1]; msgTheme.selectedButtonB=_c[2];}

// Current message theme
MSGTheme msgTheme;


/*****************************************************
DrawBox — helper to draw the message dialogue box
*****************************************************/
static void Message_DrawBox(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b, GsOT *ot, int priority)
{
    GsBOXF box;

    box.attribute = 0;
    box.x = (short)x;
    box.y = (short)y;
    box.w = (unsigned short)w;
    box.h = (unsigned short)h;

    box.r = r;
    box.g = g;
    box.b = b;

    GsSortBoxFill(&box, ot, priority);
}


/*****************************************************
Message Dialogue Themes
*****************************************************/

// Default theme
void MSG_DefaultTheme(MSGTheme *t)
{
    t->textFontStyle = FONT_STYLE_0;
    t->infoFontStyle = FONT_STYLE_0;

    // Popup background
    t->bgR = 20;
    t->bgG = 20;
    t->bgB = 40;

    // Border
    t->borderR = 255;
    t->borderG = 255;
    t->borderB = 255;

    // Main text
    t->textR = 255;
    t->textG = 255;
    t->textB = 255;

    // Button (none selected)
    t->buttonR = 50;
    t->buttonG = 50;
    t->buttonB = 120;

	// Button  (selected)
    t->selectedButtonR = 220;
    t->selectedButtonG = 200;
    t->selectedButtonB = 50;

    // Button text
    t->buttonTextR = 255;
    t->buttonTextG = 255;
    t->buttonTextB = 255;

    // Selected button text
    t->selectedTextR = 0;
    t->selectedTextG = 0;
    t->selectedTextB = 0;
}


// Create some pre-defined colour themes
void MSG_SetPredefinedTheme(int themeID)
{
    MSG_DefaultTheme(&msgTheme);

    switch (themeID)
    {
        case MSG_THEME_RED:
			// Fonts style
            msgTheme.textFontStyle = FONT_STYLE_0;

			// Colours
            SET_MSG_BG(COL_DEEPRED);
            SET_MSG_BORDER(COL_WHITE);
            SET_MSG_BUTTON(COL_DARKRED);
            SET_MSG_SELECTED(COL_SCARLET);
            break;

        case MSG_THEME_DARK:
			// Fonts style
            msgTheme.textFontStyle = FONT_STYLE_1;

			// Colours
            SET_MSG_BG(COL_BLACK);
            SET_MSG_BORDER(COL_GUNMETAL);
            SET_MSG_BUTTON(COL_GUNMETAL);
            SET_MSG_SELECTED(COL_SLATE);
            break;

        case MSG_THEME_RETRO:
			// Fonts style
            msgTheme.textFontStyle = FONT_STYLE_2;

			// Colours
            SET_MSG_BG(COL_BLACK);
            SET_MSG_BORDER(COL_GREEN);
            SET_MSG_TEXT(COL_LIME);
            SET_MSG_BUTTON(COL_DARKGREEN);
            SET_MSG_SELECTED(COL_GREEN);
            break;

        case MSG_THEME_WHITE:
			// Fonts style
            msgTheme.textFontStyle = FONT_STYLE_0;
			
			// Colours
            SET_MSG_BG(COL_OFFWHITE);
            SET_MSG_BORDER(COL_CHARCOAL);
            SET_MSG_TEXT(COL_BLACK);
            SET_MSG_BUTTON(COL_SILVER);
            SET_MSG_SELECTED(COL_SKYBLUE);
            break;

        case MSG_THEME_GREEN:
			// Fonts style
            msgTheme.textFontStyle = FONT_STYLE_0;

            SET_MSG_BG(COL_DEEPGREEN);
            SET_MSG_BORDER(COL_MINT);
            SET_MSG_BUTTON(COL_FORESTGREEN);
            SET_MSG_SELECTED(COL_EMERALD);
            break;

        case MSG_THEME_DEFAULT:
        default:
			// Fonts style
            msgTheme.textFontStyle = FONT_STYLE_0;

			// Colours
            SET_MSG_BG(COL_DEEPBLUE);
            SET_MSG_BORDER(COL_WHITE);
            SET_MSG_BUTTON(COL_MIDBLUE);
            SET_MSG_SELECTED(COL_ROYALBLUE);
            break;
    }
}


// Get the theme
MSGTheme MSG_GetTheme(void)
{
    return msgTheme;
}


// Set the theme
void MSG_SetTheme(MSGTheme theme)
{
    memcpy(&msgTheme, &theme, sizeof(MSGTheme));
}


/*****************************************************
Message Functions
*****************************************************/

// Initialise the message dialogue
void Message_Init(void)
{
	MSG_DefaultTheme(&msgTheme);
}


// Open the message
static void Message_Open(const char *text, int promptType, int holdFrames)
{
    strncpy(msgText, text, MSG_MAX_LEN);
    msgText[MSG_MAX_LEN] = '\0';

    msgActive = 1;
    msgPromptType = promptType;
    msgResult = MSG_RESULT_NONE;

    msgAnimFrame = 0;
    msgHoldTimer = 0;
    msgHoldLimit = holdFrames; // Store the custom duration

    msgSelection = 0;
}


// Auto-dismiss popup
void Message_Show(const char *text)
{
    Message_Open(text, MSG_NONE, MSG_HOLD_FRAMES);
}


// Show a timed message
void Message_ShowTimed(const char *text, int frames)
{
    Message_Open(text, MSG_NONE, frames);
}


// Prompt popup
void Message_Prompt(const char *text, int promptType)
{
    Message_Open(text, promptType, MSG_HOLD_FRAMES);
}


// Returns 1 if popup active
int Message_IsActive(void)
{
    return msgActive;
}


// Returns current result
int Message_GetResult(void)
{
    return msgResult;
}


// Manually dismiss popup
void Message_Dismiss(void)
{
    msgActive = 0;
}


// Set the button text colour
static void Message_SetButtonTextColourForIndex(int index)
{
    if (msgSelection == index)
        FontFX_SetColour(msgTheme.selectedTextR, msgTheme.selectedTextG, msgTheme.selectedTextB);
    else
        FontFX_SetColour(msgTheme.buttonTextR, msgTheme.buttonTextG, msgTheme.buttonTextB);
}


/*****************************************************
Message Update
*****************************************************/

void Message_Update(void)
{
    if (!msgActive)
        return;

    // Opening animation
    if (msgAnimFrame < MSG_ANIM_FRAMES)
    {
        msgAnimFrame++;
        return;
    }

	/*****************************************************
	Auto-dismiss message
	*****************************************************/
    if (msgPromptType == MSG_NONE)
    {
        msgHoldTimer++;

        // Cross dismisses immediately
        if (BTN_PRESSED(PADcross))
        {
            msgResult = MSG_RESULT_OK;
            msgActive = 0;
            return;
        }

        // Auto dismiss
        if (msgHoldTimer >= msgHoldLimit)
        {
            msgResult = MSG_RESULT_OK;
            msgActive = 0;
        }

        return;
    }


	/*****************************************************
	Prompt input
	*****************************************************/

    // Toggle selection
    if (BTN_PRESSED(PADleft) || BTN_PRESSED(PADright))
        msgSelection ^= 1;

    // Confirm selection
    if (BTN_PRESSED(PADcross))
    {
        if (msgPromptType == MSG_YES_NO)
            msgResult = (msgSelection == 0) ? MSG_RESULT_YES : MSG_RESULT_NO;
        else if (msgPromptType == MSG_OK_CANCEL)
            msgResult = (msgSelection == 0) ? MSG_RESULT_OK : MSG_RESULT_NO;
        else if (msgPromptType == MSG_OK)
            msgResult = MSG_RESULT_OK;

        msgActive = 0;
    }

    // Circle acts as cancel/no
    if (BTN_PRESSED(PADcircle))
    {
        msgResult = MSG_RESULT_NO;
        msgActive = 0;
    }
}


/*****************************************************
Render message
*****************************************************/

void Message_Render(GsOT *ot, int priority)
{
    int popupW;
    int popupH;
    int popupX;
    int popupY;
    int scale;

    if (!msgActive)
        return;

    // Select popup height
    if (msgPromptType == MSG_NONE)
        popupH = MSG_POPUP_H;
    else
        popupH = MSG_PROMPT_H;

    popupW = MSG_POPUP_W;

    // Animation scale
    scale = (msgAnimFrame * 100) / MSG_ANIM_FRAMES;
    if (scale > 100)
        scale = 100;

    popupW = (popupW * scale) / 100;
    popupH = (popupH * scale) / 100;

    popupX = MSG_SCREEN_CX - (popupW / 2);
    popupY = MSG_SCREEN_CY - (popupH / 2);

    // Message box shadow
    Message_DrawBox(popupX + 2, popupY + 2, popupW, popupH, 0, 0, 0, ot, priority + 4);

    // Main message background
    Message_DrawBox(popupX, popupY, popupW, popupH,
        msgTheme.bgR, msgTheme.bgG, msgTheme.bgB,
        ot, priority + 3);

    // Borders
    Message_DrawBox(popupX, popupY, popupW, 2, msgTheme.borderR, msgTheme.borderG, msgTheme.borderB, ot, priority + 2);
    Message_DrawBox(popupX, popupY + popupH - 2, popupW, 2, msgTheme.borderR, msgTheme.borderG, msgTheme.borderB, ot, priority + 2);
    Message_DrawBox(popupX, popupY, 2, popupH, msgTheme.borderR, msgTheme.borderG, msgTheme.borderB, ot, priority + 2);
    Message_DrawBox(popupX + popupW - 2, popupY, 2, popupH, msgTheme.borderR, msgTheme.borderG, msgTheme.borderB, ot, priority + 2);

    if (msgAnimFrame < MSG_ANIM_FRAMES)
        return;

    // Message text
    FontFX_FontBegin();
    FontFX_SetStyle(msgTheme.textFontStyle);
    FontFX_SetColour(msgTheme.textR, msgTheme.textG, msgTheme.textB);
    FontFX_SetCenter(popupX + MSG_TEXT_PADDING, popupX + popupW - MSG_TEXT_PADDING);
    FontFX_Print(0, popupY + 10, msgText, ot, priority);
    FontFX_FontEnd();


    /******************************************************************
    YES / NO PROMPT (EQUAL WIDTH + CENTERED GROUP)
    ******************************************************************/
    if (msgPromptType == MSG_YES_NO)
    {
        const char *yesText = "YES";
        const char *noText = "NO";

        int yesLen = strlen(yesText);
        int noLen  = strlen(noText);
        int charW = FONT_CHAR_W * fontScale;
        int yesTextW = yesLen * charW;
        int noTextW  = noLen  * charW;
        int buttonW = (yesTextW > noTextW ? yesTextW : noTextW) + (MSG_BUTTON_PADDING * 2);
        int buttonH = 16;
        int gap = 10;
        int totalW = (buttonW * 2) + gap;
        int startX = popupX + (popupW - totalW) / 2;
        int y = popupY + 36;
        int yesX = startX;
        int noX  = startX + buttonW + gap;

        // Yes button
        Message_DrawBox(
            yesX, y, buttonW, buttonH,
            (msgSelection == 0) ? msgTheme.selectedButtonR : msgTheme.buttonR,
            (msgSelection == 0) ? msgTheme.selectedButtonG : msgTheme.buttonG,
            (msgSelection == 0) ? msgTheme.selectedButtonB : msgTheme.buttonB,
            ot, priority + 1
        );

        // No button
        Message_DrawBox(
            noX, y, buttonW, buttonH,
            (msgSelection == 1) ? msgTheme.selectedButtonR : msgTheme.buttonR,
            (msgSelection == 1) ? msgTheme.selectedButtonG : msgTheme.buttonG,
            (msgSelection == 1) ? msgTheme.selectedButtonB : msgTheme.buttonB,
            ot, priority + 1
        );

        // Yes text
        FontFX_FontBegin();
        FontFX_SetStyle(msgTheme.textFontStyle);
		Message_SetButtonTextColourForIndex(0);
        FontFX_SetCenter(yesX, yesX + buttonW);
        FontFX_Print(0, popupY + 40, "YES", ot, priority);
        FontFX_FontEnd();

        // No text
        FontFX_FontBegin();
        FontFX_SetStyle(msgTheme.textFontStyle);
		Message_SetButtonTextColourForIndex(1);
        FontFX_SetCenter(noX, noX + buttonW);
        FontFX_Print(0, popupY + 40, "NO", ot, priority);
        FontFX_FontEnd();
    }


    /******************************************************************
    OK / CANCEL PROMPT (SAME SYSTEM)
    ******************************************************************/
    else if (msgPromptType == MSG_OK_CANCEL)
    {
        const char *okText = "OK";
        const char *cancelText = "CANCEL";

        int charW = FONT_CHAR_W * fontScale;
        int okW = strlen(okText) * charW;
        int cancelW = strlen(cancelText) * charW;
        int buttonW = (okW > cancelW ? okW : cancelW) + (MSG_BUTTON_PADDING * 2);
        int buttonH = 16;
        int gap = 10;
        int totalW = (buttonW * 2) + gap;
        int startX = popupX + (popupW - totalW) / 2;
        int y = popupY + 36;
        int okX = startX;
        int cancelX = startX + buttonW + gap;

        // OK button
        Message_DrawBox(okX, y, buttonW, buttonH,
            (msgSelection == 0) ? msgTheme.selectedButtonR : msgTheme.buttonR,
            (msgSelection == 0) ? msgTheme.selectedButtonG : msgTheme.buttonG,
            (msgSelection == 0) ? msgTheme.selectedButtonB : msgTheme.buttonB,
            ot, priority + 1);

        // Cancel button
        Message_DrawBox(cancelX, y, buttonW, buttonH,
            (msgSelection == 1) ? msgTheme.selectedButtonR : msgTheme.buttonR,
            (msgSelection == 1) ? msgTheme.selectedButtonG : msgTheme.buttonG,
            (msgSelection == 1) ? msgTheme.selectedButtonB : msgTheme.buttonB,
            ot, priority + 1);

        // OK text
        FontFX_FontBegin();
        FontFX_SetStyle(msgTheme.textFontStyle);
		Message_SetButtonTextColourForIndex(0);
        FontFX_SetCenter(okX, okX + buttonW);
        FontFX_Print(0, popupY + 40, "OK", ot, priority);
        FontFX_FontEnd();

        // Cancel text
        FontFX_FontBegin();
        FontFX_SetStyle(msgTheme.textFontStyle);
		Message_SetButtonTextColourForIndex(1);
        FontFX_SetCenter(cancelX, cancelX + buttonW);
        FontFX_Print(0, popupY + 40, "CANCEL", ot, priority);
        FontFX_FontEnd();
    }

    /******************************************************************
    SINGLE OK
    ******************************************************************/
    else if (msgPromptType == MSG_OK)
    {
        const char *okText = "OK";

        int charW = FONT_CHAR_W * fontScale;
        int buttonW = strlen(okText) * charW + (MSG_BUTTON_PADDING * 2);
        int buttonH = 16;

        int okX = popupX + (popupW - buttonW) / 2;
        int y = popupY + 36;

        Message_DrawBox(okX, y, buttonW, buttonH, msgTheme.selectedButtonR, msgTheme.selectedButtonG, msgTheme.selectedButtonB, ot, priority + 1);

        FontFX_FontBegin();
        FontFX_SetStyle(msgTheme.textFontStyle);
        FontFX_SetColour(msgTheme.selectedTextR, msgTheme.selectedTextG, msgTheme.selectedTextB);
        FontFX_SetCenter(okX, okX + buttonW);
        FontFX_Print(0, popupY + 40, "OK", ot, priority);
        FontFX_FontEnd();
    }
}