#include "profiler.h"
#include "font.h"
#include "ui.h"
#include "colours.h"
#include "graphics.h"
#include "memmap.h"
#include "asset_manager.h"

#include <string.h>
#include <stdio.h>

#define VERTCLE_PADDING 9

// Global instance
Profiler prof;


// Initialise the profiler
void Profiler_Init(void)
{
    memset(&prof, 0, sizeof(Profiler));

	prof.page = 0;
    prof.visible = 0;

    prof.sections[PROF_FRAME_TOTAL].name = "TOTAL";
    prof.sections[PROF_UPDATE].name      = "UPDATE";
    prof.sections[PROF_RENDER].name      = "RENDER";
    prof.sections[PROF_GAMEPLAY].name    = "GAMEPLAY";
    prof.sections[PROF_PHYSICS].name     = "PHYSICS";
    prof.sections[PROF_AUDIO].name       = "AUDIO";
    prof.sections[PROF_MEMCARD].name     = "MEMCARD";
    prof.sections[PROF_MISC].name        = "MISC";
}


// Profiler toggle
void Profiler_Toggle(void)
{
    prof.visible = !prof.visible;
}


// Switch profiler section
void Profiler_NameSection(int id, const char *name)
{
    if (id >= 0 && id < PROF_MAX_SECTIONS)
        prof.sections[id].name = name;
}


// Begin frame
void Profiler_BeginFrame(void)
{
    int i;

    // Mark frame start
    prof.sections[PROF_FRAME_TOTAL].startVSync = prof.vsyncCount;
    prof.sections[PROF_FRAME_TOTAL].active = 1;

    // Clear section elapsed counts
    for (i = 1; i < PROF_MAX_SECTIONS; i++)
        prof.sections[i].elapsed = 0;
}


// Vsync tick conuter
void Profiler_TickVSync(void)
{
    prof.vsyncCount++;
}


// End frame
void Profiler_EndFrame(void)
{
    int elapsed;

    elapsed = prof.vsyncCount - prof.sections[PROF_FRAME_TOTAL].startVSync;
    prof.sections[PROF_FRAME_TOTAL].elapsed = elapsed;
    prof.sections[PROF_FRAME_TOTAL].active  = 0;

    if (elapsed > prof.sections[PROF_FRAME_TOTAL].elapsedMax)
        prof.sections[PROF_FRAME_TOTAL].elapsedMax = elapsed;

    prof.frameElapsed = elapsed;
    if (elapsed > prof.frameElapsedMax)
        prof.frameElapsedMax = elapsed;

    prof.frameHistory[prof.frameHistoryHead] = elapsed;
    prof.frameHistoryHead = (prof.frameHistoryHead + 1) % PROF_HISTORY;
	
	// FPS 
    prof.fpsCounter++;
    prof.fpsTimer++;
	
    if (prof.fpsTimer >= FRAME_RATE)
    {
        prof.fps = prof.fpsCounter;
        prof.fpsCounter = 0;
        prof.fpsTimer   = 0;
    }
}


// Section begin
void Profiler_SectionBegin(int id)
{
    if (id < 0 || id >= PROF_MAX_SECTIONS) return;
    prof.sections[id].startVSync = prof.vsyncCount;
    prof.sections[id].active = 1;
}


// Section end
void Profiler_SectionEnd(int id)
{
    int elapsed;
    if (id < 0 || id >= PROF_MAX_SECTIONS) return;
    if (!prof.sections[id].active) return;

    elapsed = prof.vsyncCount - prof.sections[id].startVSync;
    prof.sections[id].elapsed += elapsed;
    prof.sections[id].active   = 0;

    if (elapsed > prof.sections[id].elapsedMax)
        prof.sections[id].elapsedMax = elapsed;
}


// Stack pointer
int Profiler_GetStackPointer(void)
{
    int sp;
    asm("move %0, $sp" : "=r"(sp));
    return sp;
}


// Get used RAM
int Profiler_GetRAMUsedKB(void)
{
    int sp   = Profiler_GetStackPointer();
    int used = sp - (int)PS1_CODE_BASE;

    // Clamp to sane range
    if (used < 0) used = 0;
    if (used > PS1_RAM_TOTAL) used = PS1_RAM_TOTAL;

    return used / 1024;
}


int Profiler_GetRAMFreeKB(void)
{
    return PS1_RAM_TOTAL_KB - Profiler_GetRAMUsedKB();
}


// Render profile screen
static void RenderTimingPage(GsOT *ot, int priority)
{
	// Frame positions
    int   i;
    int   x  = 4;
    int   y  = 4;
	int   pw = 240;
	int   ph = 210;
    char  buf[40];
	
	// Memory values
	int usedKB = Profiler_GetRAMUsedKB();
    int freeKB = Profiler_GetRAMFreeKB();
    int pct    = (usedKB * 100) / PS1_RAM_TOTAL_KB;
    int sp     = Profiler_GetStackPointer();
	
    // Panel
	DrawRect(x - 2, y - 2, pw, ph, COL_MIDBLUE, SOLID, ot, priority + 3);
	
	DrawRect(x - 2, y - 2, pw, 1, COL_DARKGREY,  SOLID, ot, priority + 2);
	DrawRect(x - 2, y + ph - 3, pw, 1, COL_DARKGREY,  SOLID, ot, priority + 2);
	
    // Title
	FontFX_PrintWithoutEffects(x +6, y +2, "PROFILER", FONT_STYLE_0, 255, 220, 0, ot, priority);
    y += (VERTCLE_PADDING +1);

    // FPS row
    sprintf(buf, "FPS:%d  FRAMES:%d  MAX:%d", prof.fps, prof.frameElapsed, prof.frameElapsedMax);
    {
        unsigned char cr = (prof.frameElapsed > 1) ? 255 : 80;
        unsigned char cg = (prof.frameElapsed > 1) ? 60  : 220;
		FontFX_PrintWithoutEffects(x +6, y +2, buf, FONT_STYLE_0, cr, cg, 0, ot, priority);
    }
    y += (VERTCLE_PADDING + 6);

    // Section rows
	FontFX_PrintWithoutEffects(x +6, y, "SECTION  CUR PEAK BAR", FONT_STYLE_0, 140, 140, 140, ot, priority);
    y += VERTCLE_PADDING;

    for (i = 0; i < PROF_MAX_SECTIONS; i++)
    {
        ProfSection *s   = &prof.sections[i];
        int          cur = s->elapsed;
        int          pk  = s->elapsedMax;

        unsigned char cr, cg;
        if (cur > 1)      { cr = 255; cg = 60;  }
        else if (cur > 0) { cr = 255; cg = 180; }
        else              { cr = 80;  cg = 200; }

        // Name
        sprintf(buf, "%-8s", s->name ? s->name : "?");
		FontFX_PrintWithoutEffects(x +6, y, buf, FONT_STYLE_0, 200, 200, 200, ot, priority);

        // Current
        sprintf(buf, "%2d", cur);
		FontFX_PrintWithoutEffects(x + 68, y +6, buf, FONT_STYLE_0, cr, cg, 0, ot, priority);

        // Peak
        sprintf(buf, "%2d", pk);
		FontFX_PrintWithoutEffects(x + 100, y +6, buf, FONT_STYLE_0, 160, 100, 100, ot, priority);

        // Bar (max 4 VSyncs shown)
		DrawBarSolid(x + 150, y + 7, 80, 6, cur, 4, cr, cg, 0, COL_VERYDARKGREY, ot, priority);

        y += VERTCLE_PADDING;
    }
	
	y += VERTCLE_PADDING;
	
	FontFX_PrintWithoutEffects(x +6, y, "MEMORY", FONT_STYLE_0, 255, 220, 0, ot, priority);
    y += (VERTCLE_PADDING +1);

    // RAM total
	FontFX_PrintWithoutEffects(x +6, y, "RAM  2048KB TOTAL", FONT_STYLE_0, 160, 160, 255, ot, priority);
    y += (VERTCLE_PADDING +1);

    // Used / free
    {
        unsigned char cr = (pct > 80) ? 255 : (pct > 60) ? 255 : 80;
        unsigned char cg = (pct > 80) ? 0   : (pct > 60) ? 150 : 220;
        sprintf(buf, "USED %dKB  %d%%", usedKB, pct);
		FontFX_PrintWithoutEffects(x +6, y, buf, FONT_STYLE_0, cr, cg, 0, ot, priority);
        y += VERTCLE_PADDING;

        sprintf(buf, "FREE %dKB", freeKB);
		FontFX_PrintWithoutEffects(x +6, y, buf, FONT_STYLE_0, 100, 200, 100, ot, priority);
        y += VERTCLE_PADDING;
		
		// Draw the RAM usage bar
		DrawBarSolid(x +6, y+1, pw - 14, 8, usedKB, PS1_RAM_TOTAL_KB, cr, cg, 0, COL_VERYDARKGREY, ot, priority);
        y += 13;
    }

    // Address map
	FontFX_PrintWithoutEffects(x +6, y, "ADDRESS MAP", FONT_STYLE_0, 160, 160, 160, ot, priority);
    y += VERTCLE_PADDING;

    sprintf(buf, "CODE BASE  %08X", (unsigned int)PS1_CODE_BASE);
	FontFX_PrintWithoutEffects(x +6, y, buf, FONT_STYLE_0, 120, 200, 120, ot, priority);
    y += VERTCLE_PADDING;

    sprintf(buf, "STACK PTR  %08X", (unsigned int)sp);
	FontFX_PrintWithoutEffects(x +6, y, buf, FONT_STYLE_0, 200, 200, 120, ot, priority);
    y += VERTCLE_PADDING;

    // VRAM note
	FontFX_PrintWithoutEffects(x +6, y, "VRAM 1MB  1024X512X16BPP", FONT_STYLE_0, 160, 160, 255, ot, priority);
}


static void RenderMemMapSection(GsOT *ot, int priority)
{
    char buf[40];
    int  x  = 4;
    int  y  = 4;      // plain int, not a pointer
    int  pw = 240;
    int  ph = 210;
	
    // Panel
    DrawRect(x - 2, y - 2, pw, ph, COL_MIDBLUE, SOLID, ot, priority + 3);
    DrawRect(x - 2, y - 2, pw, 1, COL_DARKGREY, SOLID, ot, priority + 2);
    DrawRect(x - 2, y + ph - 3, pw, 1, COL_DARKGREY, SOLID, ot, priority + 2);

    // Title
    FontFX_PrintWithoutEffects(x + 6, y, "RAM MAP", FONT_STYLE_0, 255, 220, 0, ot, priority);
    y += VERTCLE_PADDING;

    // Column header
    FontFX_PrintWithoutEffects(x + 6, y, "NAME     ADDR       KB", FONT_STYLE_0, 140, 140, 140, ot, priority);
    y += VERTCLE_PADDING;

    // Fixed regions
    {
        struct 
		{ 
			const char *name; 
			unsigned int addr; 
			unsigned int size;
			unsigned char r, g, b; 
		} regions[] =
        {
            { "KERNEL  ", MEM_KERNEL_START,    MEM_KERNEL_SIZE,       100, 100, 180 },
            { "AUDIO VH", MEM_VH_ADDR,         MEM_VH_SIZE,           180, 100, 180 },
            { "AUDIO VB", MEM_VB_ADDR,         MEM_VB_SIZE,           180, 100, 180 },
            { "SEQ     ", MEM_SEQ_ADDR,        MEM_SEQ_SIZE,          180, 100, 180 },
            { "FONT    ", MEM_FONT_ADDR,        MEM_FONT_SIZE,        100, 180, 100 },
            { "ICON1   ", MEM_ICON1_ADDR,       MEM_ICON1_SIZE,       100, 180, 100 },
            { "ICON2   ", MEM_ICON2_ADDR,       MEM_ICON2_SIZE,       100, 180, 100 },
            { "ICON3   ", MEM_ICON3_ADDR,       MEM_ICON3_SIZE,       100, 180, 100 },
            { "POOL    ", MEM_ASSET_POOL_START, MEM_ASSET_POOL_SIZE,  200, 180,  80 },
        };

        int numRegions = sizeof(regions) / sizeof(regions[0]);
        int i;

        for (i = 0; i < numRegions; i++)
        {
            int  barW  = pw - 14;
            int  fillW = (regions[i].size / 1024 * barW) / PS1_RAM_TOTAL_KB;
            if (fillW < 1) fillW = 1;

            sprintf(buf, "%s %08X %4dKB", regions[i].name, regions[i].addr, regions[i].size / 1024);
            FontFX_PrintWithoutEffects(x + 6, y, buf, FONT_STYLE_0, regions[i].r, regions[i].g, regions[i].b, ot, priority);

            DrawBarSolid(x + 6, y + VERTCLE_PADDING, fillW, 3, fillW, fillW, regions[i].r, regions[i].g, regions[i].b, COL_VERYDARKGREY, ot, priority);

            y += VERTCLE_PADDING + 8;
        }
    }

    // Asset pool
    {
        int poolTotalKB = MEM_ASSET_POOL_SIZE / 1024;
        int usedKB = Asset_GetUsedKB();
        int freeKB = Asset_GetFreeKB();
        int pct    = poolTotalKB > 0 ? (usedKB * 100) / poolTotalKB : 0;

        unsigned char cr = (pct > 80) ? 255 : (pct > 60) ? 255 : 80;
        unsigned char cg = (pct > 80) ? 0   : (pct > 60) ? 150 : 220;

        sprintf(buf, "POOL %dKB USED  %dKB FREE %d%%", usedKB, freeKB, pct);
        FontFX_PrintWithoutEffects(x + 6, y, buf, FONT_STYLE_0, cr, cg, 0, ot, priority);
        y += VERTCLE_PADDING;

        DrawBarSolid(x + 6, y, pw - 14, 6, usedKB, poolTotalKB, cr, cg, 0, COL_VERYDARKGREY, ot, priority);
    }
}


void Profiler_SetPage(int page)
{
    prof.page = page % PROF_PAGE_COUNT;
}


void Profiler_Render(GsOT *ot, int priority)
{
    if (!prof.visible) return;

    if (prof.page == PROF_PAGE_TIMING)
        RenderTimingPage(ot, priority);
    else
        RenderMemMapSection(ot, priority);
}