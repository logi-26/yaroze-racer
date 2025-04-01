#include <libps.h>
#include "pad.h"

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