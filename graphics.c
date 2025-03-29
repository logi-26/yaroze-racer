#include <libps.h>
#include "graphics.h"

GsOT OTable_Header[4]; 
GsOT_TAG OTable_Array[4][1<<OTABLE_LENGTH];

int fontID_1;
int fontID_2;


void InitialiseGraphics() {
	
	if (IS_PAL_USED) {
		// Define screen as PAL
		SetVideoMode(MODE_PAL);                           
		GsInitGraph(PAL_WIDTH, PAL_HEIGHT, GsINTER|GsOFSGPU, 1, 0);											  
		GsDefDispBuff(0, 0, 0, PAL_HEIGHT);               
														  
	}
	else {
		// Define screen as NTSC
		SetVideoMode(MODE_NTSC);                          
		GsInitGraph(NTSC_WIDTH, NTSC_HEIGHT, GsINTER|GsOFSGPU, 1, 0);
		GsDefDispBuff(0, 0, 0, NTSC_HEIGHT);              
	};
	
	// Initialise 3D graphics
	GsInit3D();                                         
				
	// Set the ordering table header length
	OTable_Header[0].length = OTABLE_LENGTH;            
	OTable_Header[1].length = OTABLE_LENGTH;            
	OTable_Header[2].length = OTABLE_LENGTH;
	OTable_Header[3].length = OTABLE_LENGTH;
	
	// Set the ordering table header arrays
	OTable_Header[0].org = OTable_Array[0];             
	OTable_Header[1].org = OTable_Array[1];             
	OTable_Header[2].org = OTable_Array[2];
	OTable_Header[3].org = OTable_Array[3];
	
	// Clear the ordering table
	GsClearOt(0, 0, &OTable_Header[0]);              
	GsClearOt(0, 0, &OTable_Header[1]);               
	GsClearOt(0, 0, &OTable_Header[2]);
	GsClearOt(0, 0, &OTable_Header[3]);
};


void InitialiseFontSystem() {
	FntLoad(960, 256);
	fontID_1 = FntOpen(-150, -100, PAL_WIDTH, PAL_HEIGHT, 0, 512);
	
	fontID_2 = FntOpen(-150, 0, PAL_WIDTH, PAL_HEIGHT, 0, 512);
}