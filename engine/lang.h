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
    TXT_GAME_TITLE,

    /*****************************************************
    GAME MODE MENU (one player)
    *****************************************************/
    TXT_SINGLE_RACE,
    TXT_TOURNAMENT,
    TXT_TOURNAMENT_1,
    TXT_TOURNAMENT_2,

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

    /*****************************************************
    VEHICLE SELECT MENU
    *****************************************************/
    TXT_VEHICLE_SELECT_TITLE,
	TXT_VEHICLE_MODEL,
	TXT_VEHICLE_SPEED,
	TXT_VEHICLE_ACCELERATION,
	TXT_VEHICLE_BRAKES,
	TXT_VEHICLE_GRIP,
	TXT_VEHICLE_HATCHBACK,
	TXT_VEHICLE_SALOON,
	TXT_VEHICLE_AMERICAN,

    /*****************************************************
    RACE
    *****************************************************/
	TXT_WRONG_WAY,
	TXT_GO,
	TXT_WAITING_FOR_PLAYER,

    /*****************************************************
    TOURNAMENT
    *****************************************************/
	TXT_START_RACE,
	TXT_NEXT_RACE,
	TXT_RACE,
	TXT_TOURNAMENT_PROMPT_FIRST,
	TXT_TOURNAMENT_PROMPT_NEXT,
	TXT_QUIT_TOURNAMENT,
	TXT_RACE_RESULTS,
	TXT_FINAL_STANDINGS,
	TXT_POS,
	TXT_DRIVER,
	TXT_POINTS,
	TXT_TOTAL,
	TXT_PLAYER,
	TXT_WINNER,
	TXT_PLAYER_WINS,
	TXT_PRESS_X_OR_START,

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