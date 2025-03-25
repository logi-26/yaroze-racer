#include <libps.h>
#include "pad.h"

// low-level pad buffers: never need to touch
volatile u_char *bb0, *bb1;

// Call once only in program initialisation
void PadInit (void) {
	GetPadBuf(&bb0, &bb1);
}


u_long PadRead(void) {
	return(~(*(bb0+3) | *(bb0+2) << 8 | *(bb1+3) << 16 | *(bb1+2) << 24));
}