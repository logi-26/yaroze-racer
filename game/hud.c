#include "hud.h"
#include "gear.h"
#include "player.h"
#include "ground.h"
#include "../engine/graphics.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/ui.h"
#include "../engine/timer.h"

static int lapStarted = 0;
static int lastLapMs = 0;
static int bestLapMs = 0;
static int wasOnStartLine = 0;


static void FormatLapTime(char *buf, int ms) {
    int mins = ms / 60000;
    int secs = (ms % 60000) / 1000;
    int tenths = (ms % 1000) / 100;
    sprintf(buf, "%d:%02d.%d", mins, secs, tenths);
}


static void UpdateLapTimer(void) {
	
    // Require forward motion to prevent backward crossings from triggering
    int onStartLine = (player1.speed > 20) && IsOnStartLine(player1.gsObjectCoord.coord.t[0], player1.gsObjectCoord.coord.t[2]);

    if (onStartLine && !wasOnStartLine) 
	{
		if (!lapStarted) 
		{
            // First crossing: start the clock
            TimerReset();
            TimerStart();
            lapStarted = 1;
        } else 
		{
            // Completed a lap: record time and restart
            lastLapMs = TimerGetElapsedMs();
            
			if (bestLapMs == 0 || lastLapMs < bestLapMs) 
			{
                bestLapMs = lastLapMs;
			}
			
            TimerReset();
            TimerStart();
        }
    }
    wasOnStartLine = onStartLine;
}


void DrawGameplayHUD(GsOT *ot) {
    char hudStr[32];
    char timeStr[12];
	char gearStr[32];
	char transStr[32];

    TimerHardwarePoll();
    UpdateLapTimer();
	
	FontFX_FontBegin();
    FontFX_SetStyle(FONT_STYLE_2);
    FontFX_SetColour(COL_BLACK);
	
	// Lap timer — current lap
    FormatLapTime(timeStr, TimerGetElapsedMs());
    sprintf(hudStr, "LAP:%s", timeStr);
    FontFX_Print(20, 20, hudStr, ot, OT_UI);
	
	FontFX_SetColour(COL_DARKGREY);

    // Last lap times
	FormatLapTime(timeStr, lastLapMs);
	sprintf(hudStr, "LST:%s", timeStr);
	FontFX_Print(20, 30, hudStr, ot, OT_UI);
	
	FontFX_SetColour(COL_DARKGREEN);

	// Best lap times
	FormatLapTime(timeStr, bestLapMs);
	sprintf(hudStr, "BST:%s", timeStr);
	FontFX_Print(20, 40, hudStr, ot, OT_UI);
	
	FontFX_SetColour(COL_WHITE);

	// Game speed display
    sprintf(hudStr, "%d MPH", (int)player1.speed / 2);
    FontFX_Print(gScreenWidth - 60, gScreenHeight - 60, hudStr, ot, OT_UI);
	
	// Gear display
	if (currentGear == 0) 
	{
		sprintf(gearStr, "GEAR:R");
	}
	else 
	{
		sprintf(gearStr, "GEAR:%d", currentGear);
	}

	sprintf(transStr, "%s", (gearboxMode == GEAR_MANUAL) ? "MANUAL" : "AUTO");

	FontFX_Print(gScreenWidth - 60, gScreenHeight - 40, gearStr, ot, OT_UI);
	FontFX_Print(gScreenWidth - 60, gScreenHeight - 30, transStr, ot, OT_UI);

	// RPM display
    if (currentGear >= 1 && currentGear <= 5) 
	{
        FontFX_Print(gScreenWidth - 150, gScreenHeight - 20, "RPM", ot, OT_UI);
	}
	
	// Rev counter: green zone (0-70%) + red zone (70-100%)
    if (currentGear >= 1 && currentGear <= 5) 
	{
        int revPct = GetGearRevPct(player1.speed > 0L ? player1.speed : 0L, (long)activeVehicle->maxSpeed);
        int greenVal = revPct < 70 ? revPct : 70;
        int redVal = revPct > 70 ? revPct - 70 : 0;
        int barX = gScreenWidth - 145 + FONT_CHAR_W * 3 + 2;

        // 7 green segs × 10px + 6 × 1px gap = 76px total
        DrawBarSegmented(barX, gScreenHeight - 20, 76, 7, greenVal, 70, 7, 1, COL_DARKGREEN, COL_VERYDARKGREY, ot, OT_UI);
        
		// 3 red segs × 10px + 2 × 1px gap = 32px total
        DrawBarSegmented(barX + 78, gScreenHeight - 20, 32, 7, redVal, 30, 3, 1, COL_RED, COL_VERYDARKGREY, ot, OT_UI);
    }

    FontFX_FontEnd();
}