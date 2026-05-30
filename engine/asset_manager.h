/* asset_manager.h */
#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include "memmap.h"

#define ASSET_MAX_SLOTS     16
#define ASSET_NAME_LEN      16

#define ASSET_TYPE_NONE     0
#define ASSET_TYPE_TIM      1   // Texture/sprite
#define ASSET_TYPE_TMD      2   // 3D model
#define ASSET_TYPE_RAW      3   // Raw binary data

typedef struct
{
    char          name[ASSET_NAME_LEN];
    int           type;
    unsigned char *address;     // Where in RAM
    unsigned int  size;         // Bytes
    int           inUse;
} AssetSlot;


/************* FUNCTION PROTOTYPES *******************/
void Asset_Init(void);
unsigned char *Asset_Load(const char *name, int type, unsigned int size);
void Asset_Free(const char *name);
void Asset_FreeAll(void);
unsigned char *Asset_GetAddress(const char *name);
int Asset_GetUsedKB(void);
int Asset_GetFreeKB(void);

// Debug (print all slots)
void Asset_PrintMap(void);
/*****************************************************/

#endif