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
    "YAROZE ENGINE",

    /*****************************************************
    GAME MODE MENU (one player)
    *****************************************************/
    "SINGLE RACE",
    "TOURNAMENT",
    "TOURNAMENT 1 - DAY",
    "TOURNAMENT 2 - NIGHT",

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
	"LOAD GAME",
	
	/*****************************************************
    VEHICLE SELECT MENU
    *****************************************************/
    "SELECT VEHICLE",
	"MODEL",
	"SPEED",
	"ACCEL",
	"BRAKE",
	"GRIP",
	"HATCHBACK",
	"SALOON",
	"AMERICAN",

	/*****************************************************
    RACE
    *****************************************************/
	"WRONG WAY",
	"GO",
	"WAITING FOR THE OTHER PLAYER",

	/*****************************************************
    TOURNAMENT
    *****************************************************/
	"START RACE",
	"NEXT RACE",
	"RACE",
	"X/START: RACE  CIRCLE: BACK",
	"X/START: RACE  SELECT: QUIT",
	"QUIT THE TOURNAMENT",
	"RACE RESULTS",
	"FINAL STANDINGS",
	"POS",
	"DRIVER",
	"PTS",
	"TOTAL",
	"PLAYER",
	"WINNER",
	"YOU WIN THE TOURNAMENT",
	"PRESS X OR START",

	/*****************************************************
    TOURNAMENT SAVES
    *****************************************************/
	"TRIANGLE: SAVE",
	"Save to which Memory Card",
	"Do not save",
	"CAN NOT LOAD THIS SAVE"
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
    "MOTEUR YAROZE",

    /*****************************************************
    GAME MODE MENU (one player)
    *****************************************************/
    "COURSE SIMPLE",
    "TOURNOI",
    "TOURNOI 1 - JOUR",
    "TOURNOI 2 - NUIT",

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
	"CHARGER",
	
	/*****************************************************
    VEHICLE SELECT MENU
    *****************************************************/
    "SELECTIONNER UN VEHICULE",
	"MODELE",
	"VITESSE",
	"ACCEL",
	"FREIN",
	"GRIP",
	"COMPACTE",
	"BERLINE",
	"AMERICAINE",

	/*****************************************************
    RACE
    *****************************************************/
	"MAUVAIS SENS",
	"PARTEZ",
	"EN ATTENTE DE L'AUTRE JOUEUR",

	/*****************************************************
    TOURNAMENT
    *****************************************************/
	"LANCER LA COURSE",
	"COURSE SUIVANTE",
	"COURSE",
	"X/START: COURSE  ROND: RETOUR",
	"X/START: COURSE  SELECT: QUITTER",
	"QUITTER LE TOURNOI",
	"RESULTATS",
	"CLASSEMENT FINAL",
	"POS",
	"PILOTE",
	"PTS",
	"TOTAL",
	"JOUEUR",
	"VAINQUEUR",
	"VOUS GAGNEZ LE TOURNOI",
	"APPUYEZ SUR X OU START",

	/*****************************************************
    TOURNAMENT SAVES
    *****************************************************/
	"TRIANGLE: SAUVEGARDER",
	"Sauvegarder sur quelle Carte Memoire",
	"Ne pas sauvegarder",
	"CHARGEMENT IMPOSSIBLE"
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
    "YAROZE-ENGINE",

    /*****************************************************
    GAME MODE MENU (one player)
    *****************************************************/
    "ENKELE RACE",
    "TOERNOOI",
    "TOERNOOI 1 - DAG",
    "TOERNOOI 2 - NACHT",

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
	"LADEN",
	
	/*****************************************************
    VEHICLE SELECT MENU
    *****************************************************/
    "SELECTEER VOERTUIG",
	"MODEL",
	"SNELHEID",
	"ACCEL",
	"REM",
	"GRIP",
	"HATCHBACK",
	"SEDAN",
	"AMERIKAANS",

	/*****************************************************
    RACE
    *****************************************************/
	"VERKEERDE RICHTING",
	"START",
	"WACHTEN OP DE ANDERE SPELER",

	/*****************************************************
    TOURNAMENT
    *****************************************************/
	"START RACE",
	"VOLGENDE RACE",
	"RACE",
	"X/START: RACE  CIRKEL: TERUG",
	"X/START: RACE  SELECT: STOPPEN",
	"TOERNOOI STOPPEN",
	"RACE UITSLAG",
	"EINDSTAND",
	"POS",
	"COUREUR",
	"PNT",
	"TOTAAL",
	"SPELER",
	"WINNAAR",
	"JIJ WINT HET TOERNOOI",
	"DRUK OP X OF START",

	/*****************************************************
    TOURNAMENT SAVES
    *****************************************************/
	"DRIEHOEK: OPSLAAN",
	"Opslaan op welke Memory Card",
	"Niet opslaan",
	"KAN SAVE NIET LADEN"
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
    "YAROZE-ENGINE",

    /*****************************************************
    GAME MODE MENU (one player)
    *****************************************************/
    "EINZELRENNEN",
    "TURNIER",
    "TURNIER 1 - TAG",
    "TURNIER 2 - NACHT",

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
	"SPIEL LADEN",
	
	/*****************************************************
    VEHICLE SELECT MENU
    *****************************************************/
    "FAHRZEUG AUSWAHLEN",
	"MODELL",
	"TEMPO",
	"BESCHL.",
	"BREMSE",
	"GRIP",
	"SCHRAGHECK",
	"LIMOUSINE",
	"AMERIKANISCH",

	/*****************************************************
    RACE
    *****************************************************/
	"FALSCHE RICHTUNG",
	"LOS",
	"WARTEN AUF DEN ANDEREN SPIELER",

	/*****************************************************
    TOURNAMENT
    *****************************************************/
	"RENNEN STARTEN",
	"NACHSTES RENNEN",
	"RENNEN",
	"X/START: RENNEN  KREIS: ZURUCK",
	"X/START: RENNEN  SELECT: BEENDEN",
	"TURNIER BEENDEN",
	"RENNERGEBNIS",
	"ENDSTAND",
	"POS",
	"FAHRER",
	"PKT",
	"GESAMT",
	"SPIELER",
	"SIEGER",
	"DU GEWINNST DAS TURNIER",
	"DRUCKE X ODER START",

	/*****************************************************
    TOURNAMENT SAVES
    *****************************************************/
	"DREIECK: SPEICHERN",
	"Auf welcher Memory Card speichern",
	"Nicht speichern",
	"LADEN NICHT MOGLICH"
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
