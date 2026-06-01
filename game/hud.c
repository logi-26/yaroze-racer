#include "hud.h"
#include "gear.h"
#include "player.h"
#include "ground.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/ui.h"
#include "../engine/timer.h"

static int lapStarted     = 0;
static int lastLapMs      = 0;
static int bestLapMs      = 0;
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

    if (onStartLine && !wasOnStartLine) {
        if (!lapStarted) {
			
            // First crossing: start the clock
            TimerReset();
            TimerStart();
            lapStarted = 1;
        } else {
			
            // Completed a lap: record time and restart */
            lastLapMs = TimerGetElapsedMs();
            
			if (bestLapMs == 0 || lastLapMs < bestLapMs)
                bestLapMs = lastLapMs;
			
            TimerReset();
            TimerStart();
        }
    }
    wasOnStartLine = onStartLine;
}


void DrawGameplayHUD(GsOT *ot) {
    char hudStr[32];
    char timeStr[12];

    TimerHardwarePoll();
    UpdateLapTimer();

    FontFX_FontBegin();
    FontFX_SetStyle(FONT_STYLE_2);
    FontFX_SetColour(COL_DARKRED);

    sprintf(hudStr, "SPEED:%d MPH", (int)player1.speed / 2);
    FontFX_Print(20, 20, hudStr, ot, OT_UI);

    if (currentGear == 0)
        sprintf(hudStr, "GEAR:R %s", (gearboxMode == GEAR_MANUAL) ? "[M]" : "[A]");
    else
        sprintf(hudStr, "GEAR:%d %s", currentGear, (gearboxMode == GEAR_MANUAL) ? "[M]" : "[A]");
    
	FontFX_Print(20, 30, hudStr, ot, OT_UI);

    if (currentGear >= 1 && currentGear <= 5)
        FontFX_Print(20, 42, "RPM", ot, OT_UI);

    // Lap timer — current lap
    FormatLapTime(timeStr, TimerGetElapsedMs());
    sprintf(hudStr, "LAP:%s", timeStr);
    FontFX_Print(20, 56, hudStr, ot, OT_UI);

    // Last and best lap times (shown after the first complete lap)
    if (lastLapMs > 0) {
        FormatLapTime(timeStr, lastLapMs);
        sprintf(hudStr, "LST:%s", timeStr);
        FontFX_Print(20, 66, hudStr, ot, OT_UI);

        FormatLapTime(timeStr, bestLapMs);
        sprintf(hudStr, "BST:%s", timeStr);
        FontFX_Print(20, 76, hudStr, ot, OT_UI);
    }

    sprintf(hudStr, "X:%d Y:%d Z:%d",
        (int)player1.gsObjectCoord.coord.t[0],
        (int)player1.gsObjectCoord.coord.t[1],
        (int)player1.gsObjectCoord.coord.t[2]);
    FontFX_Print(20, 200, hudStr, ot, OT_UI);

    FontFX_FontEnd();

    // Rev counter: green zone (0-70%) + red zone (70-100%)
    if (currentGear >= 1 && currentGear <= 5) {
        int revPct   = GetGearRevPct(player1.speed > 0L ? player1.speed : 0L,
                                     (long)activeVehicle->maxSpeed);
        int greenVal = revPct < 70 ? revPct : 70;
        int redVal   = revPct > 70 ? revPct - 70 : 0;
        int barX     = 20 + FONT_CHAR_W * 3 + 2;

        // 7 green segs × 10px + 6 × 1px gap = 76px total
        DrawBarSegmented(barX,      44, 76, 7, greenVal, 70, 7, 1, COL_DARKGREEN, COL_VERYDARKGREY, ot, OT_UI);
        
		// 3 red segs × 10px + 2 × 1px gap = 32px total
        DrawBarSegmented(barX + 78, 44, 32, 7, redVal, 30, 3, 1, COL_RED, COL_VERYDARKGREY, ot, OT_UI);
    }
}