/******************************************************************************
    NET YAROZE: ON-SCREEN KEYBOARD MODULE
    --------------------------
    Displays an on-screen keyboard for capturing input from the user

    This keyboard module uses the FONT FX MODULE to handle font parsing/rendering

    Controls:
        D-Pad       Move cursor
        Cross       Type selected character / confirm on OK key
        Square      Backspace
        Triangle    Toggle uppercase / lowercase
        Circle      Cancel
        Start       Confirm input
		
	special characters:
        '<' = backspace
        '>' = confirm/OK
        '_' = space
******************************************************************************/

#include "keyboard.h"
#include "font.h"
#include "colours.h"
#include "controller.h"

#include <string.h>

// Unpack RGB macros
#define SET_KB_KEY_BG(col)       {unsigned char _c[]={col}; kbTheme.keyButtonR=_c[0]; kbTheme.keyButtonG=_c[1]; kbTheme.keyButtonB=_c[2];}
#define SET_KB_SELECTED_BG(col)  {unsigned char _c[]={col}; kbTheme.selectedKeyButtonR=_c[0]; kbTheme.selectedKeyButtonG=_c[1]; kbTheme.selectedKeyButtonB=_c[2];}
#define SET_KB_TEXT(col)         {unsigned char _c[]={col}; kbTheme.textR=_c[0]; kbTheme.textG=_c[1]; kbTheme.textB=_c[2];}
#define SET_KB_INFO_TEXT(col)    {unsigned char _c[]={col}; kbTheme.infoTextR=_c[0]; kbTheme.infoTextG=_c[1]; kbTheme.infoTextB=_c[2];}
#define SET_KB_INPUT_TEXT(col)   {unsigned char _c[]={col}; kbTheme.inputTextR=_c[0]; kbTheme.inputTextG=_c[1]; kbTheme.inputTextB=_c[2];}
#define SET_KB_OK_TEXT(col)      {unsigned char _c[]={col}; kbTheme.okCharR=_c[0]; kbTheme.okCharG=_c[1]; kbTheme.okCharB=_c[2];}
#define SET_KB_BS_TEXT(col)      {unsigned char _c[]={col}; kbTheme.bsCharR=_c[0]; kbTheme.bsCharG=_c[1]; kbTheme.bsCharB=_c[2];}
#define SET_KB_BG_COL(col)       {unsigned char _c[]={col}; kbTheme.bgR=_c[0]; kbTheme.bgG=_c[1]; kbTheme.bgB=_c[2];}
#define SET_KB_INPUT_BG(col)     {unsigned char _c[]={col}; kbTheme.inputBgR=_c[0]; kbTheme.inputBgG=_c[1]; kbTheme.inputBgB=_c[2];}
#define SET_KB_WIDGET_BG(col)    {unsigned char _c[]={col}; kbTheme.wigetR=_c[0]; kbTheme.wigetG=_c[1]; kbTheme.wigetB=_c[2];}

// For manual R,G,B values
#define SET_KB_BG_RGB(r, g, b) {kbTheme.bgR = r; kbTheme.bgG = g; kbTheme.bgB = b;}

// Current keyboard theme
KBTheme kbTheme;

// Keyboard X/Y position
static int kbX = 8; 
static int kbY = 20;


// Uppercase char set
static const char kbUpper[KB_ROWS][KB_COLS + 1] =
{
    "1234567890",
    "QWERTYUIOP",
    "ASDFGHJKL<",
    "ZXCVBNM_>."
};


// Lowercase charset
static const char kbLower[KB_ROWS][KB_COLS + 1] =
{
    "1234567890",
    "qwertyuiop",
    "asdfghjkl<",
    "zxcvbnm_>."
};


/*****************************************************
Keyboard State
*****************************************************/
static char inputBuf[KB_MAX_INPUT + 1];
static int  inputLen;
static int  cursorCol;
static int  cursorRow;
static int  upperCase;
static int  blinkTimer;


/*****************************************************
DrawBox — helper to draw a filled rectangle
*****************************************************/
static void DrawBox(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b, GsOT *ot, int priority)
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
Keyboard Themes
*****************************************************/

// Default keyboard theme
void KB_DefaultTheme(KBTheme *s)
{
    s->textFontStyle      = FONT_STYLE_0;   // Uppercase style
	s->textLowerFontStyle = FONT_STYLE_3;   // Lowercase style
	s->infoFontStyle      = FONT_STYLE_0;
	
	// Widget background
	s->wigetR = 255;
    s->wigetG = 255;
    s->wigetB = 255;
	
	// Typed text background colour
    s->inputBgR = 30;
    s->inputBgG = 30;
    s->inputBgB = 30;

	// Typed text colour
    s->inputTextR = 255;
    s->inputTextG = 255;
    s->inputTextB = 255;

	// Keyboard background colour
    s->bgR = 20;
    s->bgG = 20;
    s->bgB = 40;

	// Key button colour (none selected)
    s->keyButtonR = 50;
    s->keyButtonG = 50;
    s->keyButtonB = 120;

	// Key button colour (selected)
    s->selectedKeyButtonR = 220;
    s->selectedKeyButtonG = 200;
    s->selectedKeyButtonB = 50;

	// OK text colour
    s->okCharR = 80;
    s->okCharG = 255;
    s->okCharB = 80;

	// Backspace text colour
    s->bsCharR = 255;
    s->bsCharG = 80;
    s->bsCharB = 80;

	// Button text colour
    s->textR = 255;
    s->textG = 255;
    s->textB = 255;

	// Info text colour
    s->infoTextR = 192;
    s->infoTextG = 192;
    s->infoTextB = 192;
}


// Create some pre-defined colour themes
void KB_SetPredefinedTheme(int themeID)
{
	/*****************************************************
	Uppercase font IDs: 0, 1, 2, 5
	Lowercase font IDs: 3, 4, 6
	*****************************************************/
	
	// Use the default theme as a base for all themes
    KB_DefaultTheme(&kbTheme);
	
    switch (themeID)
    {
        case KB_THEME_RED:
			// Fonts for uppercase and lowercase chars
			kbTheme.textFontStyle = FONT_STYLE_0;
			kbTheme.textLowerFontStyle = FONT_STYLE_3;
		
			// Colours
            SET_KB_BG_COL(COL_DEEPRED);
            SET_KB_INPUT_BG(COL_CHARCOAL);
            SET_KB_INPUT_TEXT(COL_WHITE);
            SET_KB_KEY_BG(COL_DARKRED);
            SET_KB_SELECTED_BG(COL_SCARLET);
            SET_KB_TEXT(COL_WHITE);
            break;

        case KB_THEME_DARK:
			// Fonts for uppercase and lowercase chars
			kbTheme.textFontStyle = FONT_STYLE_1;
			kbTheme.textLowerFontStyle = FONT_STYLE_4;
			
			// Colours
            SET_KB_BG_COL(COL_BLACK);
            SET_KB_INPUT_BG(COL_CHARCOAL);
            SET_KB_INPUT_TEXT(COL_CYAN_PASTEL);
            SET_KB_KEY_BG(COL_GUNMETAL);
            SET_KB_SELECTED_BG(COL_SLATE);
            SET_KB_TEXT(COL_WHITE);
            break;

        case KB_THEME_RETRO:
			// Fonts for uppercase and lowercase chars
			kbTheme.textFontStyle = FONT_STYLE_2;
			kbTheme.textLowerFontStyle = FONT_STYLE_6;
		
			// Colours
            SET_KB_BG_COL(COL_BLACK);
            SET_KB_INPUT_BG(COL_DEEPGREEN);
            SET_KB_INPUT_TEXT(COL_LIME);
            SET_KB_KEY_BG(COL_DARKGREEN);
            SET_KB_SELECTED_BG(COL_GREEN);
            SET_KB_TEXT(COL_MINT);
			SET_KB_OK_TEXT(COL_DEEPGREEN);
            SET_KB_BS_TEXT(COL_DEEPRED);
            break;

        case KB_THEME_YELLOW:
			// Fonts for uppercase and lowercase chars
			kbTheme.textFontStyle = FONT_STYLE_0;
			kbTheme.textLowerFontStyle = FONT_STYLE_3;
			
			// Colours
            SET_KB_BG_COL(COL_CHARCOAL);
            SET_KB_INPUT_BG(COL_CHARCOAL);
            SET_KB_INPUT_TEXT(COL_YELLOW);
            SET_KB_KEY_BG(COL_YELLOW_DARK);
            SET_KB_SELECTED_BG(COL_GOLD);
            SET_KB_TEXT(COL_BLACK); 
			SET_KB_OK_TEXT(COL_DARKGREEN);
            SET_KB_BS_TEXT(COL_DARKRED);
            break;

        case KB_THEME_WHITE:
			// Fonts for uppercase and lowercase chars
			kbTheme.textFontStyle = FONT_STYLE_0;
			kbTheme.textLowerFontStyle = FONT_STYLE_3;
			
			// Colours
            SET_KB_BG_COL(COL_OFFWHITE);
            SET_KB_INPUT_BG(COL_WHITE);
            SET_KB_INPUT_TEXT(COL_BLACK);
            SET_KB_KEY_BG(COL_SILVER);
            SET_KB_SELECTED_BG(COL_SKYBLUE);
            SET_KB_TEXT(COL_NAVY);
			SET_KB_OK_TEXT(COL_DEEPGREEN);
            SET_KB_BS_TEXT(COL_DEEPRED);
			SET_KB_WIDGET_BG(COL_CHARCOAL);
            break;

        case KB_THEME_PURPLE:
			// Fonts for uppercase and lowercase chars
			kbTheme.textFontStyle = FONT_STYLE_0;
			kbTheme.textLowerFontStyle = FONT_STYLE_3;
			
			// Colours
            SET_KB_BG_COL(COL_DEEPPURPLE);
            SET_KB_INPUT_BG(COL_CHARCOAL);
            SET_KB_INPUT_TEXT(COL_PLUM);
            SET_KB_KEY_BG(COL_INDIGO);
            SET_KB_SELECTED_BG(COL_VIOLET);
            SET_KB_TEXT(COL_WHITE);
            break;
            
        case KB_THEME_GREEN:
			// Fonts for uppercase and lowercase chars
			kbTheme.textFontStyle = FONT_STYLE_0;
			kbTheme.textLowerFontStyle = FONT_STYLE_3;
			
			// Colours
            SET_KB_BG_COL(COL_DEEPGREEN);
            SET_KB_INPUT_BG(COL_BLACK);
            SET_KB_INPUT_TEXT(COL_MINT);
            SET_KB_KEY_BG(COL_FORESTGREEN);
            SET_KB_SELECTED_BG(COL_EMERALD);
            SET_KB_TEXT(COL_WHITE);
            break;

        case KB_THEME_DEFAULT:
        default:
			// Fonts for uppercase and lowercase chars
			kbTheme.textFontStyle = FONT_STYLE_0;
			kbTheme.textLowerFontStyle = FONT_STYLE_3;
			
			// Colours
            SET_KB_BG_COL(COL_DEEPBLUE);
            SET_KB_INPUT_BG(COL_CHARCOAL);
            SET_KB_INPUT_TEXT(COL_ICEBLUE);
            SET_KB_KEY_BG(COL_MIDBLUE);
            SET_KB_SELECTED_BG(COL_ROYALBLUE);
            SET_KB_TEXT(COL_WHITE);
            break;
    }
}


/*****************************************************
Keyboard Functions
*****************************************************/

// Initialise the keyboard
void KB_Init(void) {
    KB_DefaultTheme(&kbTheme);
	kbX = 8;
    kbY = 20;
}


// Get the theme
KBTheme KB_GetTheme(void) {
    return kbTheme;
}


// Set the theme
void KB_SetTheme(KBTheme style)
{
    // Copy the contents of the passed struct into our global kbTheme
    memcpy(&kbTheme, &style, sizeof(KBTheme));
}


// Set the keyboard position on screen
void KB_SetPosition(int x, int y)
{
    kbX = x;
    kbY = y;
}


// Get the keyboard position
void KB_GetPosition(int *x, int *y)
{
    if (x) *x = kbX;
    if (y) *y = kbY;
}


// Get the input
const char *KB_GetInput(void) 
{ 
	return inputBuf; 
}


// Get the input length
int KB_GetInputLength(void) 
{ 
	return inputLen; 
}


// Open the keyboard
void KB_Open(void)
{
    memset(inputBuf, 0, sizeof(inputBuf));
    inputLen   = 0;
    cursorCol  = 0;
    cursorRow  = 1;
    upperCase  = 1;
    blinkTimer = 0;
}


/*****************************************************
Keyboard Update
*****************************************************/

int KB_Update(void)
{
	// Pointer to the currently active keyboard layout (lowercase or uppercase)
    const char (*layout)[KB_COLS + 1];
    
	// Stores the currently selected key character
	char key;

	// Increment cursor blink timer for UI animation
    blinkTimer++;

	// Select active keyboard layout based on case mode
    layout = upperCase ? kbUpper : kbLower;
	
	/*****************************************************
	D-Pad navigation
	*****************************************************/
	
    // Move cursor up
    if (BTN_PRESSED(PADup))
    {
        cursorRow--;
		
		// Wrap around to bottom row
        if (cursorRow < 0)
            cursorRow = KB_ROWS - 1;
    }

	// Move cursor down
    if (BTN_PRESSED(PADdown))
    {
        cursorRow++;
		
		// Wrap around to top row
        if (cursorRow >= KB_ROWS)
            cursorRow = 0;
    }

	// Move cursor left
    if (BTN_PRESSED(PADleft))
    {
        cursorCol--;
		
		// Wrap around to rightmost column
        if (cursorCol < 0)
            cursorCol = KB_COLS - 1;
    }

	// Move cursor right
    if (BTN_PRESSED(PADright))
    {
        cursorCol++;
		
		// Wrap around to leftmost column
        if (cursorCol >= KB_COLS)
            cursorCol = 0;
    }

	/*****************************************************
	Special button actions
	*****************************************************/

    // Triangle button toggles uppercase/lowercase mode
    if (BTN_PRESSED(PADtriangle))
        upperCase = !upperCase;

    // Circle button cancels keyboard input
    if (BTN_PRESSED(PADcircle))
        return KB_RESULT_CANCEL;

    // Start button confirms current input
    if (BTN_PRESSED(PADstart))
        return KB_RESULT_CONFIRM;

    // Square button performs backspace
    if (BTN_PRESSED(PADsquare))
    {
		// Remove last character if buffer is not empty
        if (inputLen > 0)
        {
            inputLen--;
            inputBuf[inputLen] = '\0';
        }
    }
	
	/*****************************************************
	Cross button types selected key
	*****************************************************/

    if (BTN_PRESSED(PADcross))
    {
		// Get the character at current cursor position
        key = layout[cursorRow][cursorCol];

		// '<' acts as backspace
        if (key == '<')
        {
            if (inputLen > 0)
            {
                inputLen--;
                inputBuf[inputLen] = '\0';
            }
        }
		// '>' acts as confirm/enter
        else if (key == '>')
        {
            return KB_RESULT_CONFIRM;
        }
		// '_' inserts a space character
        else if (key == '_')
        {
			// Prevent buffer overflow
            if (inputLen < KB_MAX_INPUT)
            {
                inputBuf[inputLen++] = ' ';
                inputBuf[inputLen]   = '\0';
            }
        }
		// Normal character input
        else
        {
			// Prevent buffer overflow
            if (inputLen < KB_MAX_INPUT)
            {
                inputBuf[inputLen++] = key;
                inputBuf[inputLen]   = '\0';
            }
        }
    }

    return KB_RESULT_NONE;
}


/*****************************************************
Keyboard Render
*****************************************************/

// Render the keyboard at the stored position
void KB_RenderAtStoredPos(GsOT *ot, int priority)
{
    KB_Render(kbX, kbY, ot, priority);
}


// Render the keyboard
void KB_Render(int x, int y, GsOT *ot, int priority)
{
    int  row, col;
    int  kx, ky;
    char keyStr[2];
	int celPadding = 6;

    const char (*layout)[KB_COLS + 1];
	
	// Calculate keyboard dimensions
    int kbW = (KB_COLS * (KB_KEY_W + KB_KEY_PAD)) - KB_KEY_PAD;
    int kbH = (KB_ROWS * (KB_KEY_H + KB_KEY_PAD)) - KB_KEY_PAD;
	
	// Keyboard background location
	int bgX = x - 2;
    int bgY = (y + FONT_CHAR_H + 4);
    int bgW = kbW + 4;
    int bgH = kbH + 4;
	
	// Display buffer for the user input
	char displayBuf[KB_MAX_INPUT + 1];
    int  di;
    int  displayStyle;
	int tx = x;

	// Deyermine the current character case
	layout = upperCase ? kbUpper : kbLower;
    keyStr[1] = '\0';
	
	// Widget background
	DrawBox(bgX -2, y - 4, bgW +4, bgH + FONT_CHAR_H + 10, kbTheme.wigetR, kbTheme.wigetG, kbTheme.wigetB, ot, priority + 4);
	
    // Keyboard background
	DrawBox(bgX, bgY, bgW, bgH, kbTheme.bgR, kbTheme.bgG, kbTheme.bgB, ot, priority + 3);
	
    // Input field background
	DrawBox(bgX, y - 2, bgW, FONT_CHAR_H + 4, kbTheme.inputBgR, kbTheme.inputBgG, kbTheme.inputBgB, ot, priority + 3);
	
	// Determine the font offset for the typed text (swich between uppercase and lowercase)
	for (di = 0; di < inputLen; di++)
	{
		char c = inputBuf[di];
		int  style;
		char keyStr[2];

		keyStr[0] = (c >= 'a' && c <= 'z') ? (c - 32) : c;
		keyStr[1] = '\0';

		// Choose correct font atlas (upper or lower)
		if (c >= 'a' && c <= 'z')
			style = kbTheme.textLowerFontStyle;
		else
			style = kbTheme.textFontStyle;

		// Print the user typed text
		FontFX_PrintWithoutEffects(tx, y, keyStr, style, kbTheme.inputTextR, kbTheme.inputTextG, kbTheme.inputTextB, ot, priority + 1);

		tx += FONT_CHAR_W;
	}

    // Blinking cursor using DrawBox instead of an underscore font glyph
	// This is because some font styles dont have an underscore char
    if ((blinkTimer / 20) % 2 == 0)
    {
        int cursorX = x + (inputLen * FONT_CHAR_W);
        int cursorY = y + FONT_CHAR_H - 2;

        // Small underscore-style rectangle
        DrawBox(cursorX, cursorY, FONT_CHAR_W - 1, 2, kbTheme.inputTextR, kbTheme.inputTextG, kbTheme.inputTextB, ot, priority + 2);
    }
	
	/*****************************************************
	Keyboard Grid
	*****************************************************/

	// Increment the Y past the input field
    ky = y + FONT_CHAR_H + 6;
	
    for (row = 0; row < KB_ROWS; row++)
    {
        kx = x;

        for (col = 0; col < KB_COLS; col++)
        {
            char key = layout[row][col];
            int  isSelected = (row == cursorRow && col == cursorCol);
			int activeFontStyle;

			// Use lowercase font only for lowercase alphabet characters
			// This keeps the numbers 0-9 instead of the symbols in lowercase mode
			if (key >= 'a' && key <= 'z')
				activeFontStyle = kbTheme.textLowerFontStyle;
			else
				activeFontStyle = kbTheme.textFontStyle;

            // Key background
            if (isSelected)
                DrawBox(kx, ky, KB_KEY_W, KB_KEY_H, kbTheme.selectedKeyButtonR, kbTheme.selectedKeyButtonG, kbTheme.selectedKeyButtonB, ot, priority + 2);
            else
                DrawBox(kx, ky, KB_KEY_W, KB_KEY_H, kbTheme.keyButtonR, kbTheme.keyButtonG, kbTheme.keyButtonB, ot, priority + 2);

            // Key label
            {
                int labelX = kx + (KB_KEY_W / 2) - (FONT_CHAR_W / 2);
                int labelY = ky + (KB_KEY_H / 2) - (FONT_CHAR_H / 2);

                if (key == '<')
                {
					// Backspace button
                    FontFX_PrintWithoutEffects(kx + 2, labelY, "BS", kbTheme.textFontStyle, kbTheme.bsCharR, kbTheme.bsCharG, kbTheme.bsCharB, ot, priority + 1);
                }
                else if (key == '>')
                {
					// OK button
                    FontFX_PrintWithoutEffects(kx + 2, labelY, "OK", kbTheme.textFontStyle, kbTheme.okCharR, kbTheme.okCharG, kbTheme.okCharB, ot, priority + 1);
                }
                else if (key == '_')
                {
					// Space button
					FontFX_PrintWithoutEffects(kx + 2, labelY, "SP", kbTheme.textFontStyle, kbTheme.textR, kbTheme.textG, kbTheme.textB, ot, priority + 1);
                }
                else
                {
					// Convert to uppercase for font lookup (all chars need to be uppercase, it is the offset that determines the character case)
					keyStr[0] = (key >= 'a' && key <= 'z') ? (key - 32) : key;
					
					
                    FontFX_PrintWithoutEffects(labelX, labelY, keyStr, activeFontStyle, kbTheme.textR, kbTheme.textG, kbTheme.textB, ot, priority + 1);
                }
            }
            kx += KB_KEY_W + KB_KEY_PAD;
        }
        ky += KB_KEY_H + KB_KEY_PAD;
    }

	// Controls legend
	/*
    ky += FONT_CHAR_H + 2;

	FontFX_PrintWithoutEffects(x, ky, "CROSS:TYPE", kbTheme.infoFontStyle, kbTheme.infoTextR, kbTheme.infoTextG, kbTheme.infoTextB, ot, priority + 1);
	ky += FONT_CHAR_H + 2;
	
	FontFX_PrintWithoutEffects(x, ky, "SQUARE:DEL", kbTheme.infoFontStyle, kbTheme.infoTextR, kbTheme.infoTextG, kbTheme.infoTextB, ot, priority + 1);
	ky += FONT_CHAR_H + 2;
	
	FontFX_PrintWithoutEffects(x, ky, "TRIANGLE:CASE", kbTheme.infoFontStyle, kbTheme.infoTextR, kbTheme.infoTextG, kbTheme.infoTextB, ot, priority + 1);
    ky += FONT_CHAR_H * 2;

    FontFX_PrintWithoutEffects(x, ky, "START:OK  O:CANCEL", kbTheme.infoFontStyle, kbTheme.infoTextR, kbTheme.infoTextG, kbTheme.infoTextB, ot, priority + 1);
	*/
}