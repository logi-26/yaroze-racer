#ifndef LANG_H
#define LANG_H


typedef enum
{
    /*****************************************************
    MAIN MENU
    *****************************************************/
    TXT_ONE_PLAYER,
	TXT_TWO_PLAYER,
    TXT_LINK_GAME,
    TXT_OPTIONS,
    TXT_QUIT_GAME,
    TXT_GAME_TITLE,

    /*****************************************************
    MEMORY CARD MENU
    *****************************************************/
    TXT_MEMCARD_SLOT_A,
    TXT_MEMCARD_SLOT_B,
    TXT_MEMCARD_DONT_LOAD,
    TXT_MEMCARD_LOAD_WHICH,

    /*****************************************************
    MEMORY CARD LOAD MENU
    *****************************************************/
    TXT_LOAD_TITLE,
    TXT_NO_SAVE_DATA,

    /*****************************************************
    MEMORY CARD SAVE MENU
    *****************************************************/
    TXT_SAVE_TITLE,
    TXT_EMPTY_SLOT,
    TXT_FAILED_READ_MEMCARD,
    TXT_OVERWRITE_SAVE_SLOT,
    TXT_CREATING_SAVE_FILE,

    /*****************************************************
    OPTIONS MENU
    *****************************************************/
    TXT_OPTIONS_TITLE,
	TXT_LANGUAGE,
    TXT_MUSIC_VOLUME,
    TXT_SFX_VOLUME,
	TXT_LOAD_GAME,

    TXT_COUNT

} TextID;

extern int currentLanguage;

const char* TXT(TextID id);


typedef enum
{
    LANG_ENGLISH,
    LANG_FRENCH,
	LANG_DUTCH,
    LANG_GERMAN,

    LANG_COUNT

} Language;


#endif