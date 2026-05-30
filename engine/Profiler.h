#ifndef PROFILER_H
#define PROFILER_H

#include <libps.h>

// Config
#define PROF_MAX_SECTIONS   8
#define PROF_HISTORY        60

// Page toggle
#define PROF_PAGE_TIMING  0
#define PROF_PAGE_MEMMAP  1
#define PROF_PAGE_COUNT   2

// Section ID's (used with profile-begin and profile-end)
#define PROF_FRAME_TOTAL    0
#define PROF_UPDATE         1
#define PROF_RENDER         2
#define PROF_GAMEPLAY       3
#define PROF_PHYSICS        4
#define PROF_AUDIO          5
#define PROF_MEMCARD        6
#define PROF_MISC           7

// Memory layout
#define PS1_RAM_TOTAL_KB    2048
#define PS1_RAM_TOTAL       (PS1_RAM_TOTAL_KB * 1024)
#define PS1_RAM_START       0x80000000
#define PS1_RAM_END         0x80200000
#define PS1_CODE_BASE       0x80140000
#define PS1_STACK_BASE      0x801FFF00

// Macros
#define PROFILE_BEGIN(id)   Profiler_SectionBegin(id)
#define PROFILE_END(id)     Profiler_SectionEnd(id)


typedef struct
{
    const char *name;
    int startVSync;     // vsyncCount when section began
    int elapsed;        // VSyncs elapsed last measured frame
    int elapsedMax;     // peak VSyncs ever measured
    int active;
} ProfSection;


typedef struct
{
    // Frame timing in VSyncs
    int frameElapsed;
    int frameElapsedMax;
    int frameHistory[PROF_HISTORY];
    int frameHistoryHead;

    // Running VSync counter
    int vsyncCount;

    // FPS
    int fps;
    int fpsCounter;
    int fpsTimer;

    // Sections
    ProfSection  sections[PROF_MAX_SECTIONS];

    // Display state
    int visible;
	
	int page;
	
} Profiler;

extern Profiler prof;

/************* FUNCTION PROTOTYPES *******************/
void Profiler_Init(void);
void Profiler_Toggle(void);

void Profiler_BeginFrame(void);
void Profiler_TickVSync(void);
void Profiler_EndFrame(void);

void Profiler_SectionBegin(int id);
void Profiler_SectionEnd(int id);
void Profiler_NameSection(int id, const char *name);

// RAM queries
int Profiler_GetStackPointer(void);
int Profiler_GetRAMUsedKB(void);
int Profiler_GetRAMFreeKB(void);

// Render the overlay
void Profiler_Render(GsOT *ot, int priority);

void Profiler_SetPage(int page);
/*****************************************************/

#endif