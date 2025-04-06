#ifndef PAD_H
#define PAD_H

#include <libps.h> 

// Low-level pad buffers
extern volatile u_char *bb0, *bb1;

// Analog axis for each controller
extern u_char PAD1lh, PAD1lv, PAD1rh, PAD1rv;                  
extern u_char PAD2lh, PAD2lv, PAD2rh, PAD2rv;

typedef u_short pad_data;

typedef struct negcon_tag { 
  u_short buttons;		   // LEFT, RIGHT, DOWN, UP, START, A, B, and R
  u_char twist;			   // the twist value
  u_char button_I; 		   // I button
  u_char button_II;		   // II button
  u_char button_L;		   // L button
  
} negcon_data;

typedef struct analog_tag {
   u_short buttons;   	  // 16 buttons
   u_char right_x;		  // Movement on right stick x direction: 0~255
   u_char right_y;		  // Movement on right stick y direction: 0~255
   u_char left_x;		  // Movement on left stick x direction: 0~255
   u_char left_y;		  // Movement on left stick y direction: 0~255
} analog_data;

typedef struct packet_tag {
	u_char status; // 0xff: no controller, 0x00 controller connected
	u_char type;   // Upper 4 bits: terminal type, lower 4 bits: size of data received / 2

	union {
		pad_data pad;
		analog_data analog;
		negcon_data negcon;
    } data;		

} cntrl_data, *cntrl_ptr;

/************* FUNCTION PROTOTYPES *******************/
void PadInit(void);
u_long PadRead(void);
int CheckPort1(void);
int CheckPort2(void);
/*****************************************************/

#define PAD_NOKEY     	(0xFFFF)
#define PAD_BAD		  	(0xFF)

// Definitions for controller types
#define MOUSE 		  	(0x1)
#define NEGCON		  	(0x2)
#define NORMAL 		  	(0x4)
#define ANALOG_JOY    	(0x5)
#define GUNCON        	(0x6)
#define ANALOG        	(0x7)

// Definitions for controller port 1 buttons
#define PAD1up       	(1<<12)
#define PAD1down     	(1<<14)
#define PAD1left     	(1<<15)
#define PAD1right    	(1<<13)
#define PAD1triangle 	(1<< 4)
#define PAD1cross    	(1<< 6)
#define PAD1square   	(1<< 7)
#define PAD1circle   	(1<< 5)
#define PAD1select   	(1<< 8)
#define PAD1start    	(1<<11)
#define PAD1R1       	(1<< 3)
#define PAD1R2       	(1<< 1)
#define PAD1L1       	(1<< 2)
#define PAD1L2       	(1<< 0)
#define PAD1L3       	(1<< 9)
#define PAD1R3       	(1<<10)

// Definitions for controller port 2 buttons
#define PAD2up       	(1<<28)
#define PAD2down     	(1<<30)
#define PAD2left     	(1<<31)
#define PAD2right    	(1<<29)
#define PAD2triangle 	(1<<20)
#define PAD2cross    	(1<<22)
#define PAD2square   	(1<<23)
#define PAD2circle   	(1<<21)
#define PAD2select   	(1<<24)
#define PAD2start    	(1<<27)
#define PAD2R1       	(1<<19)
#define PAD2R2       	(1<<17)
#define PAD2L1       	(1<<18)
#define PAD2L2       	(1<<16)
#define PAD2L3       	(1<<25)
#define PAD2R3       	(1<<26)

// Definitions for neGcon port 1 buttons
#define NEGCON1_LEFT	(1<< 7)
#define NEGCON1_RIGHT  	(1<< 5)
#define NEGCON1_DOWN	(1<< 6)
#define NEGCON1_UP	  	(1<< 4)
#define NEGCON1_A	  	(1<<13)
#define NEGCON1_B	  	(1<<12)
#define NEGCON1_START  	(1<< 3)
#define NEGCON1_R	  	(1<<11)

#endif // PAD_H