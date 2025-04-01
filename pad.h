#ifndef PAD_H
#define PAD_H

// Low-level pad buffers
volatile u_char *bb0, *bb1;

// Analog axis for each controller
extern u_char PAD1lh, PAD1lv, PAD1rh, PAD1rv;                  
extern u_char PAD2lh, PAD2lv, PAD2rh, PAD2rv;

// Function prototypes
void PadInit();
u_long PadRead();

// Definitions for controller port 1 buttons
#define PAD1up       (1<<12)
#define PAD1down     (1<<14)
#define PAD1left     (1<<15)
#define PAD1right    (1<<13)
#define PAD1triangle (1<< 4)
#define PAD1cross    (1<< 6)
#define PAD1square   (1<< 7)
#define PAD1circle   (1<< 5)
#define PAD1select   (1<< 8)
#define PAD1start    (1<<11)
#define PAD1R1       (1<< 3)
#define PAD1R2       (1<< 1)
#define PAD1L1       (1<< 2)
#define PAD1L2       (1<< 0)
#define PAD1L3       (1<< 9)
#define PAD1R3       (1<<10)

// Definitions for controller port 2 buttons
#define PAD2up       (1<<28)
#define PAD2down     (1<<30)
#define PAD2left     (1<<31)
#define PAD2right    (1<<29)
#define PAD2triangle (1<<20)
#define PAD2cross    (1<<22)
#define PAD2square   (1<<23)
#define PAD2circle   (1<<21)
#define PAD2select   (1<<24)
#define PAD2start    (1<<27)
#define PAD2R1       (1<<19)
#define PAD2R2       (1<<17)
#define PAD2L1       (1<<18)
#define PAD2L2       (1<<16)
#define PAD2L3       (1<<25)
#define PAD2R3       (1<<26)

#endif // PAD_H