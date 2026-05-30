#include "asset_manager.h"
#include <string.h>
#include <stdio.h>

static AssetSlot slots[ASSET_MAX_SLOTS];
static unsigned char *poolPtr = (unsigned char*)MEM_ASSET_POOL_START;


void Asset_Init(void)
{
    memset(slots, 0, sizeof(slots));
    poolPtr = (unsigned char*)MEM_ASSET_POOL_START;
}


unsigned char *Asset_Load(const char *name, int type, unsigned int size)
{
    int i;
    unsigned char *addr;

    // Check if already loaded
    for (i = 0; i < ASSET_MAX_SLOTS; i++)
    {
        if (slots[i].inUse && strcmp(slots[i].name, name) == 0)
            return slots[i].address;
    }

    // Find a free slot
    for (i = 0; i < ASSET_MAX_SLOTS; i++)
    {
        if (!slots[i].inUse)
            break;
    }

    if (i >= ASSET_MAX_SLOTS)
    {
        printf("Asset_Load: no free slots for %s\n", name);
        return 0;
    }

    // Check pool has room
    if (poolPtr + size > (unsigned char*)MEM_ASSET_POOL_END)
    {
        printf("Asset_Load: pool full, cannot load %s (%d bytes)\n", name, size);
        return 0;
    }

    addr = poolPtr;
    poolPtr += size;

    // Align to 4 bytes
    poolPtr = (unsigned char*)(((unsigned int)poolPtr + 3) & ~3);

    strncpy(slots[i].name, name, ASSET_NAME_LEN - 1);
    slots[i].name[ASSET_NAME_LEN - 1] = '\0';
    slots[i].type    = type;
    slots[i].address = addr;
    slots[i].size    = size;
    slots[i].inUse   = 1;

    printf("Asset_Load: %s at %08X (%d bytes)\n", name, (unsigned int)addr, size);

    return addr;
}


void Asset_Free(const char *name)
{
    /* Note: this is a simple pool — freeing individual assets
       does not reclaim memory unless it is the last allocation.
       For a Net Yaroze game, free everything between levels
       using Asset_FreeAll rather than freeing individual assets. */
    int i;
    for (i = 0; i < ASSET_MAX_SLOTS; i++)
    {
        if (slots[i].inUse && strcmp(slots[i].name, name) == 0)
        {
            slots[i].inUse = 0;
            printf("Asset_Free: %s released from slot %d\n", name, i);
            return;
        }
    }
}


void Asset_FreeAll(void)
{
    memset(slots, 0, sizeof(slots));
    poolPtr = (unsigned char*)MEM_ASSET_POOL_START;
    printf("Asset_FreeAll: pool reset\n");
}


unsigned char *Asset_GetAddress(const char *name)
{
    int i;
    for (i = 0; i < ASSET_MAX_SLOTS; i++)
    {
        if (slots[i].inUse && strcmp(slots[i].name, name) == 0)
            return slots[i].address;
    }
    return 0;
}


int Asset_GetUsedKB(void)
{
    return ((unsigned int)poolPtr - MEM_ASSET_POOL_START) / 1024;
}


int Asset_GetFreeKB(void)
{
    return (MEM_ASSET_POOL_END - (unsigned int)poolPtr) / 1024;
}


void Asset_PrintMap(void)
{
    int i;
    printf("=== ASSET POOL MAP ===\n");
    printf("Pool: %08X - %08X (%dKB total)\n", MEM_ASSET_POOL_START, MEM_ASSET_POOL_END, MEM_ASSET_POOL_SIZE / 1024);
    printf("Used: %dKB  Free: %dKB\n", Asset_GetUsedKB(), Asset_GetFreeKB());

    for (i = 0; i < ASSET_MAX_SLOTS; i++)
    {
        if (slots[i].inUse)
        {
            printf("  [%2d] %08X %5dB  %s\n", i, (unsigned int)slots[i].address, slots[i].size, slots[i].name);
        }
    }
    printf("======================\n");
}