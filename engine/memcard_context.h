#ifndef MEMCARD_CONTEXT_H
#define MEMCARD_CONTEXT_H


// Memory card context to store the selected slot
typedef struct
{
    int selectedSlot;
    int saveValue;
} MemcardContext;


extern MemcardContext memcardContext;


#endif