#ifndef GRAPHICS_H
#define GRAPHICS_H

#define true          (1)
#define false         (0)

#define OTABLE_LENGTH (12)                            		
#define MAX_PACKETS   (124000)                        		
#define IS_PAL_USED   (true)                          		
#define PAL_WIDTH     (320)                           		
#define PAL_HEIGHT    (240)
#define NTSC_WIDTH    (320)                           		
#define NTSC_HEIGHT   (256)

// Distance threshold for graphics culling
#define DISTANCE_THRESHOLD_SQUARED (80000*80000)

// Define 4 ordering tables, 4 memory packets and 4 draw environments
// This will give use 2 screen buffers for each view (split screen)
extern GsOT OTable_Header[4];                        
extern GsOT_TAG OTable_Array[4][1<<OTABLE_LENGTH];       
extern PACKET Packet_Memory[4][MAX_PACKETS];            		
DRAWENV SplitScreenInfo[4];                       		                               

extern int fontID_1;
extern int fontID_2;

/************* FUNCTION PROTOTYPES *******************/
void InitialiseGraphics();
void InitialiseFontSystem();
/*****************************************************/

#endif  // GRAPHICS_H