#ifndef MEMCARD_H
#define MEMCARD_H

#include <libps.h>

#include "memmap.h"

// Memory card values
#define BLOCK_SIZE 8192
#define CARD_TOTAL_BLOCKS 15
#define CARD_MAX_FILES CARD_TOTAL_BLOCKS

#define SAVE_BLOCKS 1
#define SAVE_SIZE BLOCK_SIZE

#define HEADER_SIZE 512

#define MAX_SAVES 6
#define SAVE_SLOT_COUNT MAX_SAVES
#define SAVE_SLOT_SIZE 1280

// Savefile identifiers
#define SAVE_REGION      "BE"
#define SAVE_PRODCODE    "NETYA"
#define SAVE_PREFIX      SAVE_REGION SAVE_PRODCODE "-"

// TIM memory card icon addresses (defined in memmap.h)
#define ICON1_ADDR MEM_ICON1_PTR
#define ICON2_ADDR MEM_ICON2_PTR
#define ICON3_ADDR MEM_ICON3_PTR

// Save header definitions
#define SAVE_TITLE_LEN   64
#define ICON_FRAME_SIZE  128
#define ICON_CLUT_SIZE   32
#define ICON_MAX_FRAMES  3


/******************************************************************************
SAVE SLOT STRUCTURE
******************************************************************************/
/*
typedef struct
{
    int used;
    int slotNumber;
    int value;
    char title[32];

	// Padding to fill the slot data to 1280 bytes
    unsigned char padding[
        SAVE_SLOT_SIZE
        - sizeof(int)   // used
        - sizeof(int)   // slotNumber
        - sizeof(int)   // value
        - 32            // title
    ];
} SaveSlot;
*/

typedef struct
{
    int used;
    int slotNumber;
    int value1;
    int value2;
    int value3;
    char title[32];

	// Padding to fill the slot data to 1280 bytes
    unsigned char padding[
        SAVE_SLOT_SIZE
        - sizeof(int) * 5   // used, slotNumber, value1, value2, value3
        - 32                // title
    ];
} SaveSlot;


/******************************************************************************
FULL SAVE FILE STRUCTURE
******************************************************************************/

typedef struct
{
    unsigned char header[HEADER_SIZE];
    SaveSlot slots[SAVE_SLOT_COUNT];
} SaveData;


/******************************************************************************
SAVE INFO STRUCTURE
******************************************************************************/

typedef struct
{
    char          filename[64];
    char          title[SAVE_TITLE_LEN + 1];
    int           blocks;
    int           iconFrames;
    unsigned char clut[ICON_CLUT_SIZE];
    unsigned char icon[ICON_MAX_FRAMES][ICON_FRAME_SIZE];

} SaveInfo;


/************* FUNCTION PROTOTYPES *******************/
char* GetCardPath(int card);
int CardLoad(int card);
void CardSave(int card);


//void CardWriteSlot(int card, int slot, int value, const char *title);
void CardWriteSlot(int card, int slot, const SaveSlot *data);


//void WriteSlot(int slot, int value, char *title);
void WriteSlot(int slot, const SaveSlot *data);


int CardReadSlot(int card, int slot);
int CardFreeBlocks(int card);
int CardReadSaveInfo(char *path, SaveInfo *info);
void PrintCardStatus(int card, long status);
void BuildSaveHeader(void);
void PrintSlot(int slot);
SaveSlot* GetSlot(int slot);
/*****************************************************/

#endif // MEMCARD_H