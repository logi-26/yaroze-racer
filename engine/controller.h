#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <libps.h>


/*****************************************************
Button definitions - PadRead() bitmasks
*****************************************************/
#define PADLup     (1 << 12)
#define PADLdown   (1 << 14)
#define PADLleft   (1 << 15)
#define PADLright  (1 << 13)
#define PADRup     (1 <<  4)
#define PADRdown   (1 <<  6)
#define PADRleft   (1 <<  7)
#define PADRright  (1 <<  5)
#define PADi       (1 <<  9)
#define PADj       (1 << 10)
#define PADk       (1 <<  8)
#define PADl       (1 <<  3)
#define PADm       (1 <<  1)
#define PADn       (1 <<  2)
#define PADo       (1 <<  0)
#define PADh       (1 << 11)

// Named aliases
#define PADL1       PADn
#define PADL2       PADo
#define PADR1       PADl
#define PADR2       PADm
#define PADstart    PADh
#define PADselect   PADk
#define PADcross    PADRdown
#define PADcircle   PADRright
#define PADsquare   PADRleft
#define PADtriangle PADRup
#define PADup       PADLup
#define PADdown     PADLdown
#define PADleft     PADLleft
#define PADright    PADLright


/*****************************************************
Terminal types
*****************************************************/
#define CTRL_MOUSE       0x1
#define CTRL_NEGCON      0x2
#define CTRL_NORMAL      0x4
#define CTRL_ANALOG_JOY  0x5
#define CTRL_GUNCON      0x6
#define CTRL_ANALOG      0x7


/*****************************************************
Status/sentinel values
*****************************************************/
#define CTRL_DISCONNECTED 0xFF
#define PAD_NOKEY         0xFFFF
#define PAD_BAD           0xFF


/*****************************************************
Button definitions
*****************************************************/
#define PAD_SELECT    (1 <<  0)
#define PAD_L3        (1 <<  1)
#define PAD_R3        (1 <<  2)
#define PAD_START     (1 <<  3)
#define PAD_UP        (1 <<  4)
#define PAD_RIGHT     (1 <<  5)
#define PAD_DOWN      (1 <<  6)
#define PAD_LEFT      (1 <<  7)
#define PAD_L2        (1 <<  8)
#define PAD_R2        (1 <<  9)
#define PAD_L1        (1 << 10)
#define PAD_R1        (1 << 11)
#define PAD_TRIANGLE  (1 << 12)
#define PAD_CIRCLE    (1 << 13)
#define PAD_CROSS     (1 << 14)
#define PAD_SQUARE    (1 << 15)

// Mouse
#define MOUSE_RIGHT   (1 << 2)
#define MOUSE_LEFT    (1 << 3)
#define MOUSE_NOKEY   0xFC

// NeGcon
#define NEGCON_UP     (1 << 4)
#define NEGCON_RIGHT  (1 << 5)
#define NEGCON_DOWN   (1 << 6)
#define NEGCON_LEFT   (1 << 7)
#define NEGCON_R      (1 << 11)
#define NEGCON_B      (1 << 12)
#define NEGCON_A      (1 << 13)
#define NEGCON_START  (1 <<  3)

// GunCon
#define GUNCON_A       (1 <<  3)
#define GUNCON_TRIGGER (1 << 13)
#define GUNCON_B       (1 << 14)

// Quit shortcut
#define CTRL_QUIT (PAD_PRESS(ctrlBuffer1, PAD_SELECT) && PAD_PRESS(ctrlBuffer1, PAD_START))


/*****************************************************
Macros — cntrl_data interface
*****************************************************/
#define PAD_PRESS(buf, btn)      (~(buf)->data.pad & (btn))
#define MOUSE_PRESS(buf, btn)    (~(buf)->data.mouse.buttons & (btn))
#define MOUSE_X(buf)             ((buf)->data.mouse.x_offset)
#define MOUSE_Y(buf)             ((buf)->data.mouse.y_offset)
#define NEGCON_PRESS(buf, btn)   (~(buf)->data.negcon.buttons & (btn))
#define NEGCON_TWIST(buf)        ((buf)->data.negcon.twist)
#define NEGCON_BTN_I(buf)        ((buf)->data.negcon.button_I)
#define NEGCON_BTN_II(buf)       ((buf)->data.negcon.button_II)
#define NEGCON_BTN_L(buf)        ((buf)->data.negcon.button_L)
#define GUNCON_PRESS(buf, btn)   (~(buf)->data.guncon.buttons & (btn))
#define GUNCON_X(buf)            ((buf)->data.guncon.guncon_x)
#define GUNCON_Y(buf)            ((buf)->data.guncon.guncon_y)
#define ANALOG_LX(buf)           ((buf)->data.analog.left_x)
#define ANALOG_LY(buf)           ((buf)->data.analog.left_y)
#define ANALOG_RX(buf)           ((buf)->data.analog.right_x)
#define ANALOG_RY(buf)           ((buf)->data.analog.right_y)
#define CTRL_CONNECTED(buf)      ((buf)->status != CTRL_DISCONNECTED)
#define CTRL_TYPE(buf)           ((buf)->type >> 4)


/*****************************************************
Button State Helpers
*****************************************************/

// True on the frame the button is first pressed
#define BTN_PRESSED(btn) ((pad & (btn)) && !(oldPad & (btn)))

// True every frame the button is held
#define BTN_HELD(btn) (pad & (btn))

// True on the frame the button is released
#define BTN_RELEASED(btn) (!(pad & (btn)) && (oldPad & (btn)))


/*****************************************************
Structures
*****************************************************/
typedef u_short pad_data;


// Analog struct
typedef struct {
    u_short buttons;
    u_char right_x;
    u_char right_y;
    u_char left_x;
    u_char left_y;
} analog_data;


// Mouse struct
typedef struct {
    char not_used;
    char buttons;
    signed char x_offset;
    signed char y_offset;
} mouse_data;


// NeGcon struct
typedef struct {
    u_short buttons;
    u_char twist;
    u_char button_I;
    u_char button_II;
    u_char button_L;
} negcon_data;


// GunCon struct
typedef struct {
    u_short buttons;
    u_short guncon_x;
    u_short guncon_y;
} guncon_data;


// Controller data struct
typedef struct {
    u_char status;
    u_char type;
    union {
        pad_data pad;
		analog_data analog;
        mouse_data mouse;
        negcon_data negcon;
        guncon_data guncon;
    } data;
} cntrl_data, *cntrl_ptr;


// Globals
extern volatile u_char *bb0, *bb1;  // PadRead() low-level buffers  
extern cntrl_data *ctrlBuffer1;     // GetPadBuf() port 1           
extern cntrl_data *ctrlBuffer2;    	// GetPadBuf() port 2           
extern u_long pad;            		// Current frame button state   
extern u_long oldPad;         		// Previous frame button state  
 

/************* FUNCTION PROTOTYPES *******************/
void  Ctrl_Init(void);
void  Ctrl_Update(void);
u_long Ctrl_Read(void);
int   Ctrl_IsConnected(cntrl_data *buf);
const char *Ctrl_GetTypeName(int type);
int Ctrl_IsGamepad(int port);
/*****************************************************/

#endif