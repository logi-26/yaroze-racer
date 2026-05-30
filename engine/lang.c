#include "lang.h"

#define EMPTY_SLOT_TEXT TXT(TXT_EMPTY_SLOT)

int currentLanguage = LANG_ENGLISH;


/*****************************************************
ENGLISH
*****************************************************/
static const char* strings_english[TXT_COUNT] =
{
    /*****************************************************
    MAIN MENU
    *****************************************************/
    "ONE PLAYER",
	"TWO PLAYER",
    "LINK GAME",
    "OPTIONS",
    "QUIT GAME",
    "YAROZE ENGINE",

    /*****************************************************
    MEMORY CARD MENU
    *****************************************************/
    "Slot A",
    "Slot B",
    "Do not load",
    "Load from which Memory Card?",

    /*****************************************************
    MEMORY CARD LOAD MENU
    *****************************************************/
    "LOAD DATA",
    "NO SAVE DATA",

    /*****************************************************
    MEMORY CARD SAVE MENU
    *****************************************************/
    "SAVE DATA",
    "EMPTY SLOT",
    "FAILED TO READ THE MEMORY CARD",
    "OVERWRITE SAVE SLOT?",
    "CREATING SAVE FILE...",

    /*****************************************************
    OPTIONS MENU
    *****************************************************/
    "OPTIONS",
	"LANGUAGE",
    "MUSIC VOLUME",
    "SFX VOLUME",
	"LOAD GAME"
};


/*****************************************************
FRENCH
*****************************************************/
static const char* strings_french[TXT_COUNT] =
{
    /*****************************************************
    MAIN MENU
    *****************************************************/
    "UN JOUEUR",
	"DEUX JOUEURS",
    "PARTIE EN RESEAU",
    "OPTIONS",
    "QUITTER",
    "MOTEUR YAROZE",

    /*****************************************************
    MEMORY CARD MENU
    *****************************************************/
    "Slot A",
    "Slot B",
    "Ne pas charger",
    "Charger depuis quelle Carte Memoire ?",

    /*****************************************************
    MEMORY CARD LOAD MENU
    *****************************************************/
    "CHARGEMENT",
    "AUCUNE SAUVEGARDE",

    /*****************************************************
    MEMORY CARD SAVE MENU
    *****************************************************/
    "SAUVEGARDE",
    "EMPLACEMENT VIDE",
    "ECHEC DE LECTURE DE LA CARTE MEMOIRE",
    "ECRASER LA SAUVEGARDE ?",
    "CREATION DE LA SAUVEGARDE...",

    /*****************************************************
    OPTIONS MENU
    *****************************************************/
    "OPTIONS",
	"LANGUE",
    "VOLUME MUSIQUE",
    "VOLUME EFFETS",
	"CHARGER"
};


/*****************************************************
DUTCH
*****************************************************/
static const char* strings_dutch[TXT_COUNT] =
{
    /*****************************************************
    MAIN MENU
    *****************************************************/
    "EEN SPELER",
	"TWEE SPELERS",
    "LINKSPEL",
    "OPTIES",
    "AFSLUITEN",
    "YAROZE-ENGINE",

    /*****************************************************
    MEMORY CARD MENU
    *****************************************************/
    "Slot A",
    "Slot B",
    "Niet laden",
    "Laden vanaf welke Memory Card?",

    /*****************************************************
    MEMORY CARD LOAD MENU
    *****************************************************/
    "LADEN",
    "GEEN SAVE DATA",

    /*****************************************************
    MEMORY CARD SAVE MENU
    *****************************************************/
    "OPSLAAN",
    "LEEG SLOT",
    "FOUT BIJ LEZEN VAN MEMORY CARD",
    "SAVE OVERSCHRIJVEN?",
    "SAVE BESTAND AANMAKEN...",

    /*****************************************************
    OPTIONS MENU
    *****************************************************/
    "OPTIES",
    "TAAL",
    "MUZIEK VOLUME",
    "SFX VOLUME",
	"LADEN"
};


/*****************************************************
GERMAN
*****************************************************/
static const char* strings_german[TXT_COUNT] =
{
    /*****************************************************
    MAIN MENU
    *****************************************************/
    "EIN SPIELER",
	"ZWEI SPIELER",
    "VERBINDUNGSSPIEL",
    "OPTIONEN",
    "SPIEL BEENDEN",
    "YAROZE-ENGINE",

    /*****************************************************
    MEMORY CARD MENU
    *****************************************************/
    "Slot A",
    "Slot B",
    "Nicht laden",
    "Von welcher Memory Card laden?",

    /*****************************************************
    MEMORY CARD LOAD MENU
    *****************************************************/
    "LADEN",
    "KEINE SPEICHERDATEN",

    /*****************************************************
    MEMORY CARD SAVE MENU
    *****************************************************/
    "SPEICHERN",
    "LEERER SLOT",
    "FEHLER BEIM LESEN DER MEMORY CARD",
    "SPEICHERSTAND ÜBERSCHREIBEN?",
    "SPEICHERDATEI WIRD ERSTELLT...",

    /*****************************************************
    OPTIONS MENU
    *****************************************************/
    "OPTIONEN",
    "SPRACHE",
    "MUSIK LAUTSTARKE",
    "SFX LAUTSTARKE",
	"SPIEL LADEN"
};


// Language tables
static const char** languageTables[LANG_COUNT] =
{
    strings_english,
    strings_french,
	strings_dutch,
	strings_german
};


// Return the string based on the ID and current language
const char* TXT(TextID id)
{
    if (id < 0 || id >= TXT_COUNT)
        return "TXT_ERR";

    return languageTables[currentLanguage][id];
}
