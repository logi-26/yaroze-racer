// low-level pad buffers
volatile u_char *bb0, *bb1;

// Function Prototypes
void PadInit();
u_long PadRead();

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
#define PAD1i        (1<< 9)
#define PAD1j        (1<<10)

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
#define PAD2i        (1<<25)
#define PAD2j        (1<<26)