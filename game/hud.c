#include "hud.h"
#include "gear.h"
#include "player.h"
#include "ground.h"
#include "ai_racer.h"
#include "link_race.h"
#include "../engine/graphics.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/ui.h"
#include "../engine/timer.h"

static int lapStarted = 0;
static int lastLapMs = 0;
static int bestLapMs = 0;
static int wasOnStartLine = 0;
static int lapsTimed = 0;       // The lap count when the clock was last restarted


static void FormatLapTime(char *buf, int ms) {
    int mins = ms / 60000;
    int secs = (ms % 60000) / 1000;
    int tenths = (ms % 1000) / 100;
    sprintf(buf, "%d:%02d.%d", mins, secs, tenths);
}


// Clear the lap times (at the start of a race)
void ResetLapTimer(void) {
    lapStarted = 0;
    lastLapMs = 0;
    bestLapMs = 0;
    wasOnStartLine = 0;
    lapsTimed = 0;
    TimerReset();
}


static void UpdateLapTimer(void) {
	
    // Require forward motion to prevent backward crossings from triggering
    int onStartLine = (player1.speed > 20) && IsOnStartLine(player1.gsObjectCoord.coord.t[0], player1.gsObjectCoord.coord.t[2]);

    if (!lapStarted)
	{
		// First crossing of the line (start the clock)
		if (onStartLine && !wasOnStartLine)
		{
            TimerReset();
            TimerStart();
            lapStarted = 1;
            lapsTimed = playerRaceLapCount;
        }
    }
	else if (playerRaceLapCount > lapsTimed)
	{
		// Completed a lap (record the time and restart the clock)
		// Only when the lap counter goes up, which it doesn't for a lap that skipped
		// part of the track or for going back and forth over the line
		lastLapMs = TimerGetElapsedMs();

		if (bestLapMs == 0 || lastLapMs < bestLapMs)
		{
			bestLapMs = lastLapMs;
		}

		TimerReset();
		TimerStart();
		lapsTimed = playerRaceLapCount;
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
	
	// Lap timer (current lap)
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
	
	// Player position and lap number
	FontFX_SetSize(2);
	FontFX_SetColour(COL_AMBER);

    sprintf(hudStr, "%d/%d", playerRacePosition, LinkRace_IsActive() ? 2 : 1 + NUM_AI_RACERS);
    FontFX_Print(gScreenWidth - 55, 20, hudStr, ot, OT_UI);

	FontFX_SetSize(1);
	
	// (after the last lap: stays on the last one)
	sprintf(hudStr, "LAP:%d/%d", playerRaceLapCount < NUM_RACE_LAPS ? playerRaceLapCount + 1 : NUM_RACE_LAPS, NUM_RACE_LAPS);
    FontFX_Print(gScreenWidth - 65, 40, hudStr, ot, OT_UI);
	
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