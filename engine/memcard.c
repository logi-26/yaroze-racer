#include "memcard.h"
#include <string.h>
#include <stdio.h>

// Global data
static SaveData saveData;
static SaveInfo saveList[MAX_SAVES];
static int saveCount = 0;


char *GetCardPath(int card)
{
	/******************************************************************************
    Builds the full PlayStation memory card file path for a given card slot
	SAVE_PREFIX defines the game identifier string
	******************************************************************************/
	
	// Static buffer so the returned pointer remains valid after function exit
    static char path[32];
	
	// Construct memory card file path
    sprintf(path, "%s%sSAVE", (card == 0) ? "bu00:" : "bu10:", SAVE_PREFIX);

	// Return pointer to static path buffer
    return path;
}


int CardLoad(int card)
{
	/******************************************************************************
    Read the full SaveData structure from the memory card into RAM
	******************************************************************************/
	
    int fd;
	
	// Build full memory card file path
    char *path = GetCardPath(card);

	// Open file for reading
    fd = open(path, 0x0001);

	// Failed to open save file
    if (fd < 0)
        return 0;

	// Read entire save structure into RAM
    read(fd, (char*)&saveData, sizeof(SaveData));

	// Close file after reading
    close(fd);

	// Validate save file header contains the magic "SC"
    if (saveData.header[0] != 'S' || saveData.header[1] != 'C')
        return 0;

	// Load successful
    return 1;
}


void CardSave(int card)
{
	/******************************************************************************
    Writes the full SaveData structure to the memory card
	******************************************************************************/
	
    int fd;
	
	// Get full memory card file path
    char *path = GetCardPath(card);

	// Delete existing save file to ensure clean overwrite
    delete(path);

	// Create/open memory card file
    fd = open(path, 0x0202 | (SAVE_BLOCKS << 16));

	// Return if the file failed to create/open
    if (fd < 0)
	{
		printf("Failed to open the card!\n");
        return;
	}

	// Write full save structure to memory card
    write(fd, (char*)&saveData, sizeof(SaveData));
    
	// Close file after writing
	close(fd);
	
    printf("CardSave write complete\n");
	
	// ************************
	// DEBUG ONLY:
	// ************************
	
	// Clear local save data to verify that reload works correctly
	memset(&saveData, 0, sizeof(SaveData));

	// Reload save file from memory card for verification
	CardLoad(card);

	// Print verification results
	printf("loaded used=%d\n", saveData.slots[0].used);
	printf("loaded value=%d\n", saveData.slots[0].value);
}


/*
void CardWriteSlot(int card, int slot, int value, const char *title)
{
	/******************************************************************************
	Writes data to a save slot on the selected memory card
	******************************************************************************/
	/*
	int free_blocks = 0;
	
	if(slot < 0 || slot >= SAVE_SLOT_COUNT)
        return;

	// Attempt to load an existing save file
    if (!CardLoad(card))
    {
		// Create a new memory card save header
        printf("no existing save, building header...\n");
        BuildSaveHeader();
    }

	// Write the slot data into the save structure
	WriteSlot(slot, value, (char*)title);
	
	// Save the updated data back to the memory card
    CardSave(card);
	
	// Update the free block count
	free_blocks = CardFreeBlocks(card);
}
*/




void CardWriteSlot(int card, int slot, const SaveSlot *data)
{
	/******************************************************************************
	Writes data to a save slot on the selected memory card
	******************************************************************************/

	int free_blocks = 0;
	
	if(slot < 0 || slot >= SAVE_SLOT_COUNT)
        return;

	// Attempt to load an existing save file
    if (!CardLoad(card))
    {
		// Create a new memory card save header
        printf("no existing save, building header...\n");
        BuildSaveHeader();
    }
	
	// Write slot data
    WriteSlot(slot, data);
	
	// Save to memory card
    CardSave(card);

    // Update free block count
    free_blocks = CardFreeBlocks(card);
}




int CardReadSlot(int card, int slot)
{
	/******************************************************************************
	Loads the memory card save file and returns the value from the save slot

	Returns:
	- Slot value on success
	- -1 if the slot index is invalid or the save file failed to load
	******************************************************************************/
	
	// Prevent accessing memory outside the slot array
    if (slot < 0 || slot >= SAVE_SLOT_COUNT)
        return -1;

	// Load the save file from the memory card
    if (!CardLoad(card))
        return -1;

	// Return the stored value from the requested slot
    return saveData.slots[slot].value;
}


int CardFreeBlocks(int card)
{
	/******************************************************************************
	Calculate free memory card blocks
	Scans all save files and calculates how many memory card blocks are in use

	Returns:
	- Number of free blocks remaining on the card
	******************************************************************************/
	
    struct DIRENTRY  entry;
    struct DIRENTRY *found;

    SaveInfo info;

    char path[64];

    char *device;
    char *pattern;

	// Total number of used blocks found on the card
    int usedBlocks = 0;
	
	// Select the memory card device path
    device = (card == 0) ? "bu00:" : "bu10:";

	// File search pattern for the selected card
    pattern = (card == 0) ? "bu00:*" : "bu10:*";

	// Begin searching files on the memory card
    found = firstfile(pattern, &entry);

	// Loop through every file found on the card
    while (found != 0)
    {
		printf("save file found: %p\n", found);
		
		// Build the full file path
        strcpy(path, device);

        strcat(path, entry.name);

		// Read save information and add its block usage to the total
        if (CardReadSaveInfo(path, &info))
            usedBlocks += info.blocks;

		// Move to the next file on the card
        found = nextfile(&entry);
    }

	// Return remaining free blocks
    return CARD_TOTAL_BLOCKS - usedBlocks;
}


int CardReadSaveInfo(char *path, SaveInfo *info)
{
	/******************************************************************************
	Reads the memory card save header and extracts:
	- Save filename
	- Save title
	- Number of memory card blocks used
	- Number of animated icon frames
	- Icon CLUT (palette)
	- Icon image frame data
	******************************************************************************/
	
    unsigned char header[0x200];
    int fd;
    int i;

	// Clear the output structure before filling it
    memset(info, 0, sizeof(SaveInfo));

	// Open the memory card save file for reading
    fd = open(path, 0x0001);

	// Return if the file failed to open
    if (fd < 0)
        return 0;

	// Read the first 512 bytes (memory card save header)
    read(fd, (char*)header, sizeof(header));

	// Close the file
    close(fd);

	// Validate the save file header contains the magic "SC"
    if (header[0x00] != 'S' || header[0x01] != 'C')
        return 0;

	// Store the filename/path
    strncpy(info->filename, path, 63);

	// Ensure null termination
    info->filename[63] = '\0';

	// Copy the save title from the header (begins at offset 0x04)
    memcpy(info->title, &header[0x04], SAVE_TITLE_LEN);

	// Ensure null termination
    info->title[SAVE_TITLE_LEN] = '\0';
	
	// Read the number of memory card blocks used (header offset 0x03)
    info->blocks = (int)header[0x03];
	
	// Extract the number of icon animation frames (lower 4 bits of byte 0x02)
    info->iconFrames = (int)(header[0x02] & 0x0F);

	// Copy the icon CLUT (begins at offset 0x60)
    memcpy(info->clut, &header[0x60], ICON_CLUT_SIZE);

	// Copy each icon animation frame (frames begin at offset 0x80)
    for (i = 0; i < info->iconFrames && i < ICON_MAX_FRAMES; i++)
    {
        memcpy(info->icon[i], &header[0x80 + (i * ICON_FRAME_SIZE)], ICON_FRAME_SIZE);
    }

	// Successfully read save info
    return 1;
}


int ExtractTim(unsigned char *tim, unsigned char *clutOut, unsigned char *iconOut)
{
	/******************************************************************************
    Extracts the CLUT and icon pixel data from a TIM loaded into memory

    The PlayStation memory card icon format requires:
    - 32 bytes of CLUT data
    - 128 bytes of 4-bit image data

    This function assumes the TIM file contains:
    - A valid TIM header
    - A CLUT section
    - A 4-bit image section
    ******************************************************************************/
	
    unsigned long *p;
    unsigned long flags;
    unsigned long size;

	// Treat the TIM data as 32-bit values for easier parsing
    p = (unsigned long*)tim;

	// Validate TIM magic number
    if (p[0] != 0x10)
        return 0;

	// Read TIM flags
    flags = p[1];

	// Ensure the TIM contains a CLUT
    if ((flags & 0x08) == 0)
        return 0;

	// Move past TIM header
    p += 2;

	// Read CLUT block size
    size = p[0];

	// Copy CLUT data
    memcpy(clutOut, ((unsigned char*)p) + 12, 32);

	// Move to image data block
    p = (unsigned long*)(((unsigned char*)p) + size);

	// Read image block size
    size = p[0];

	// Copy icon pixel data
    memcpy(iconOut, ((unsigned char*)p) + 12, 128);

    return 1;
}


void BuildSaveHeader(void)
{
	/******************************************************************************
    Build the save file header
	******************************************************************************/
	
	// Variables to store the 3 TIM icon files and the CLUT
    unsigned char clut[32];
    unsigned char frame1[128];
    unsigned char frame2[128];
    unsigned char frame3[128];

    memset(&saveData, 0, sizeof(SaveData));

	// Load the TIM files with the CLUT
    ExtractTim(ICON1_ADDR, clut, frame1);
    ExtractTim(ICON2_ADDR, clut, frame2);
    ExtractTim(ICON3_ADDR, clut, frame3);

	// Savedata header magic 
    saveData.header[0x00] = 'S';
    saveData.header[0x01] = 'C';
	
	// Using a 3-frame animated icon
    saveData.header[0x02] = 0x13;

    saveData.header[0x03] = SAVE_BLOCKS;

	// Set the save game title
    strncpy((char*)&saveData.header[0x04], "NET YAROZE SAVE", 64);
	
	// Add the CLUT to the header
    memcpy(&saveData.header[0x60],  clut,   32);
	
	// Add the 3 icons
    memcpy(&saveData.header[0x080], frame1, 128);
    memcpy(&saveData.header[0x100], frame2, 128);
    memcpy(&saveData.header[0x180], frame3, 128);
}


/*
void WriteSlot(int slot, int value, char *title)
{
	/******************************************************************************
    Write the save slot data
	******************************************************************************/
	/*
    SaveSlot *s;

	// Validate the slot index (prevents writing outside the save slot array)
    if (slot < 0 || slot >= SAVE_SLOT_COUNT)
        return;

	// Get a pointer to the requested save slot
    s = &saveData.slots[slot];
	
	// Clear the entire slot before writing new data
    memset(s, 0, sizeof(SaveSlot));
	
	// Mark the slot as used
	s->used = 1;
	
	// Store the slot number
    s->slotNumber = slot;
	
	// Store the save value (just an int for now)
    s->value = value;

	// Copy save title into the slot (leave room for the null terminator)
    strncpy(s->title, title, 31);

	// Ensure the title string is always null terminated
    s->title[31] = '\0';
}
*/




void WriteSlot(int slot, const SaveSlot *data)
{
	/******************************************************************************
    Write the save slot data
	******************************************************************************/

    SaveSlot *s;

	// Validate the slot index (prevents writing outside the save slot array)
    if (slot < 0 || slot >= SAVE_SLOT_COUNT)
        return;

	// Get a pointer to the requested save slot
    s = &saveData.slots[slot];
	
	// Clear the entire slot before writing new data
    memset(s, 0, sizeof(SaveSlot));
	
	// Copy the entire structure
    *s = *data;
	
	// Ensure slot number is correct
    s->slotNumber = slot;
	
	// Mark the slot as used
	s->used = 1;
	
	// Ensure the title string is always null terminated
    s->title[31] = '\0';
}











SaveSlot* GetSlot(int slot)
{
	/******************************************************************************
	Returns a pointer to a valid, used save slot inside the saveData struct
	This is used by menu systems and game loading code to access save entries
	
	Returns:
	- Pointer to SaveSlot if valid and used
	- NULL if: Slot index is out of range or is marked as unused
	******************************************************************************/
	
    SaveSlot *s;

	// Validate slot index range
    if (slot < 0 || slot >= SAVE_SLOT_COUNT)
        return NULL;

	// Get pointer to requested slot in save data
    s = &saveData.slots[slot];

	// Check if slot contains valid save data
    if (s->used != 1)
    {
        printf("slot unused\n");
        return NULL;
    }

	// Return valid slot pointer
    return s;
}


void PrintCardStatus(int card, long status)
{
	/******************************************************************************
    Print the card status for debugging
	******************************************************************************/
	
    int freeBlocks;

    switch (status)
    {
        case 0:
            FntPrint(-1, "~c900Card %d: None", card + 1);
            break;

        case 1:
            freeBlocks = CardFreeBlocks(card);
            FntPrint(-1, "~c090Card %d: Present (%d free blocks)", card + 1, freeBlocks);
            break;

        case 2:
            FntPrint(-1, "~c090Card %d: New", card + 1);
            break;

        case 3:
            FntPrint(-1, "~c900Card %d: Error", card + 1);
            break;

        case 4:
            FntPrint(-1, "~c990Card %d: Unformatted", card + 1);
            break;
    }
}


void PrintSlot(int slot)
{
	/******************************************************************************
    Print slot info for debugging
	******************************************************************************/
	
    SaveSlot *s;

    if (slot < 0 || slot >= SAVE_SLOT_COUNT)
        return;

    s = &saveData.slots[slot];
	
	if (s->used != 1)
        return;

    FntPrint(-1, "~c007SLOT %d  TITLE : %s  VALUE : %d\n\n", s->slotNumber, s->title, s->value);
}