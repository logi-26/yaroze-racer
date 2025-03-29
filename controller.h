#ifndef CONTROLLER_H
#define CONTROLLER_H

// Global Variables
extern u_char PAD1lh, PAD1lv, PAD1rh, PAD1rv;                  // Analog axis
extern u_char PAD2lh, PAD2lv, PAD2rh, PAD2rv;                  // Analog axis

extern GsBOXF box[2];
extern GsBOXF stick[2];

extern unsigned long PLAYING;

/************* FUNCTION PROTOTYPES *******************/
void ProcessUserInput();
/*****************************************************/

#endif