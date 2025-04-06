#include <libps.h>
#include "pad.h"

volatile u_char *bb0 = NULL;
volatile u_char *bb1 = NULL;
u_char PAD1lh, PAD1lv, PAD1rh, PAD1rv;
u_char PAD2lh, PAD2lv, PAD2rh, PAD2rv;

// Initialise the pad buffer (call once only in program initialisation)
void PadInit (void) {
	GetPadBuf(&bb0, &bb1);
}

// Read the pad buffer data
u_long PadRead(void) {
	
	// Get port 1 analog stick positions
	PAD1rh = *(bb0+4);
	PAD1rv = *(bb0+5);
	PAD1lh = *(bb0+6);
	PAD1lv = *(bb0+7);
	
	// Get port 2 analog stick positions
	PAD2rh = *(bb1+4);
	PAD2rv = *(bb1+5);
	PAD2lh = *(bb1+6);
	PAD2lv = *(bb1+7);
	
	// Return the pad buffer
	return(~(*(bb0+3) | *(bb0+2) << 8 | *(bb1+3) << 16 | *(bb1+2) << 24));
}

int CheckPort1(void) {
	
	if (*(bb0) == PAD_BAD) {
		FntPrint("PAD port-1 removed.\n\n");
		return 0;
	}
	else if ((*(bb0+1) >> 4) == MOUSE) {
		FntPrint("Mouse port-1 connected.\n\n");
		return 1;
	}
	else if ((*(bb0+1) >> 4) == NEGCON) {
		FntPrint("NeGcon port-1 connected.\n\n");
		return 2;
	}
	else if ((*(bb0+1) >> 4) == NORMAL) {
		FntPrint("Standard PAD port-1 connected.\n\n");
		return 3;
	}
	else if ((*(bb0+1) >> 4) == ANALOG) {
		FntPrint("Analog PAD port-1 connected.\n\n"); 
		return 4;
	}
	else if ((*(bb0+1) >> 4) == ANALOG_JOY) {
		FntPrint("Joystick port-1 connected.\n\n");
		return 5;
	}
	else {
		FntPrint("No controller in port 1\n\n");
		return 0;
	}
}

int CheckPort2(void) {
	
	if (*(bb1) == PAD_BAD) {
		FntPrint("PAD port-2 removed.\n\n");
		return 0;
	}
	else if ((*(bb1+1) >> 4) == MOUSE) {
		FntPrint("Mouse port-2 connected.\n\n");
		return 1;
	}
	else if ((*(bb1+1) >> 4) == NEGCON) {
		FntPrint("NeGcon port-2 connected.\n\n");
		return 2;
	}
	else if ((*(bb1+1) >> 4) == NORMAL) {
		FntPrint("Standard PAD port-2 connected.\n\n");
		return 3;
	}
	else if ((*(bb1+1) >> 4) == ANALOG) {
		FntPrint("Analog PAD port-2 connected.\n\n"); 
		return 4;
	}
	else if ((*(bb1+1) >> 4) == ANALOG_JOY) {
		FntPrint("Joystick port-2 connected.\n\n");
		return 5;
	}
	else {
		FntPrint("No controller in port 2\n\n");
		return 0;
	}
}