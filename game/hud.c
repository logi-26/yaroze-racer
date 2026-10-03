#include "hud.h"
#include "gear.h"
#include "player.h"
#include "game.h"
#include "ground.h"
#include "ai_racer.h"
#include "link_race.h"
#include "car_controls.h"
#include "../engine/graphics.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/ui.h"
#include "../engine/timer.h"

// Each players lap times (two in a two player race). The timer runs for the whole race, and each
// players lap is timed from when they last crossed the line
typedef struct {
	int started;            // Crossed the line for the first time (the clock is running)
	int lastMs, bestMs;
	int wasOnStartLine;
	int lapsTimed;          // The lap count when the lap was last started
	int lapStartMs;         // The race timer when the lap started
} LapTimer;

static LapTimer lapTimers[2];


static void FormatLapTime(char *buf, int ms) {
    int mins = ms / 60000;
    int secs = (ms % 60000) / 1000;
    int tenths = (ms % 1000) / 100;
    sprintf(buf, "%d:%02d.%d", mins, secs, tenths);
}


// Clear the lap times and start the races timer (at the start of a race)
void ResetLapTimer(void) {
	int i;

	for (i = 0; i < 2; i++) {
		lapTimers[i].started = 0;
		lapTimers[i].lastMs = 0;
		lapTimers[i].bestMs = 0;
		lapTimers[i].wasOnStartLine = 0;
		lapTimers[i].lapsTimed = 0;
		lapTimers[i].lapStartMs = 0;
	}
    TimerReset();
    TimerStart();
}


static void UpdateLapTimer(LapTimer *t, PlayerStruct *car, int laps) {
	int now = TimerGetElapsedMs();

    // Require forward motion to prevent backward crossings from triggering
    int onStartLine = (car->speed > 20) && IsOnStartLine(car->gsObjectCoord.coord.t[0], car->gsObjectCoord.coord.t[2]);

    if (!t->started)
	{
		// First crossing of the line (start the clock)
		if (onStartLine && !t->wasOnStartLine)
		{
            t->started = 1;
            t->lapsTimed = laps;
            t->lapStartMs = now;
        }
    }
	else if (laps > t->lapsTimed)
	{
		// Completed a lap (record the time and restart the clock)
		// Only when the lap counter goes up, which it doesn't for a lap that skipped
		// part of the track or for going back and forth over the line
		t->lastMs = now - t->lapStartMs;

		if (t->bestMs == 0 || t->lastMs < t->bestMs)
		{
			t->bestMs = t->lastMs;
		}

		t->lapStartMs = now;
		t->lapsTimed = laps;
	}
    t->wasOnStartLine = onStartLine;
}


// A players HUD: player 1 or 2, their position (1 to the number of cars), drawn into ot
// (one player: the whole screen. Two players: their half of the screen)
void DrawGameplayHUD(int playerNo, int position, GsOT *ot) {
    char hudStr[32];
    char timeStr[12];
	char gearStr[32];
	char transStr[32];
	PlayerStruct *car = (playerNo == 2) ? &player2 : &player1;
	LapTimer *t = &lapTimers[(playerNo == 2) ? 1 : 0];
	int laps = (playerNo == 2) ? OpponentRaceLapCount() : playerRaceLapCount;
	int gear = (playerNo == 2) ? Player2Gear() : currentGear;
	GearboxMode gearbox = (playerNo == 2) ? Player2Gearbox() : gearboxMode;
	VehicleAttributes *vehicle = (playerNo == 2) ? Player2Vehicle() : activeVehicle;
	int cars = (LinkRace_IsActive() || NumberOfPlayers == 2) ? 2 : 1 + NUM_AI_RACERS;
	int split = (NumberOfPlayers == 2);
	int height = split ? gScreenHeight / 2 : gScreenHeight;

    TimerHardwarePoll();
    UpdateLapTimer(t, car, laps);

	FontFX_FontBegin();
    FontFX_SetStyle(FONT_STYLE_2);
    FontFX_SetColour(COL_BLACK);

	// Lap timer (current lap)
    FormatLapTime(timeStr, t->started ? TimerGetElapsedMs() - t->lapStartMs : 0);
    sprintf(hudStr, "LAP:%s", timeStr);
    FontFX_Print(split ? 12 : 20, split ? 6 : 20, hudStr, ot, OT_UI);

	// Last lap times (not in two player, there is less room)
	if (!split)
	{
		FontFX_SetColour(COL_DARKGREY);
		FormatLapTime(timeStr, t->lastMs);
		sprintf(hudStr, "LST:%s", timeStr);
		FontFX_Print(20, 30, hudStr, ot, OT_UI);
	}

	FontFX_SetColour(COL_DARKGREEN);

	// Best lap times
	FormatLapTime(timeStr, t->bestMs);
	sprintf(hudStr, "BST:%s", timeStr);
	FontFX_Print(split ? 12 : 20, split ? 16 : 40, hudStr, ot, OT_UI);

	// Player position and lap number
	FontFX_SetSize(2);
	FontFX_SetColour(COL_AMBER);

    sprintf(hudStr, "%d/%d", position, cars);
    FontFX_Print(gScreenWidth - 55, split ? 6 : 20, hudStr, ot, OT_UI);

	FontFX_SetSize(1);

	// (after the last lap: stays on the last one)
	sprintf(hudStr, "LAP:%d/%d", laps < NUM_RACE_LAPS ? laps + 1 : NUM_RACE_LAPS, NUM_RACE_LAPS);
    FontFX_Print(gScreenWidth - 65, split ? 24 : 40, hudStr, ot, OT_UI);

	FontFX_SetColour(COL_WHITE);

	// Game speed display
    sprintf(hudStr, "%d MPH", (int)car->speed / 2);
    FontFX_Print(gScreenWidth - 60, height - (split ? 34 : 60), hudStr, ot, OT_UI);

	// Gear display
	if (gear == 0)
	{
		sprintf(gearStr, "GEAR:R");
	}
	else
	{
		sprintf(gearStr, "GEAR:%d", gear);
	}

	sprintf(transStr, "%s", (gearbox == GEAR_MANUAL) ? "MANUAL" : "AUTO");

	if (split)
	{
		FontFX_Print(gScreenWidth - 60, height - 24, gearStr, ot, OT_UI);
	}
	else
	{
		FontFX_Print(gScreenWidth - 60, height - 40, gearStr, ot, OT_UI);
		FontFX_Print(gScreenWidth - 60, height - 30, transStr, ot, OT_UI);
	}

	// RPM display
    if (gear >= 1 && gear <= 5)
	{
        FontFX_Print(gScreenWidth - 150, height - (split ? 12 : 20), "RPM", ot, OT_UI);
	}

	// Rev counter: green zone (0-70%) + red zone (70-100%)
    if (gear >= 1 && gear <= 5)
	{
        int savedGear = currentGear;
        int revPct, greenVal, redVal, barX, barY;

		// The rev counter reads the gear from currentGear
        currentGear = gear;
        revPct = GetGearRevPct(car->speed > 0L ? car->speed : 0L, (long)vehicle->maxSpeed);
        currentGear = savedGear;

        greenVal = revPct < 70 ? revPct : 70;
        redVal = revPct > 70 ? revPct - 70 : 0;
        barX = gScreenWidth - 145 + FONT_CHAR_W * 3 + 2;
		barY = height - (split ? 12 : 20);

        // 7 green segs × 10px + 6 × 1px gap = 76px total
        DrawBarSegmented(barX, barY, 76, 7, greenVal, 70, 7, 1, COL_DARKGREEN, COL_VERYDARKGREY, ot, OT_UI);

		// 3 red segs × 10px + 2 × 1px gap = 32px total
        DrawBarSegmented(barX + 78, barY, 32, 7, redVal, 30, 3, 1, COL_RED, COL_VERYDARKGREY, ot, OT_UI);
    }

    FontFX_FontEnd();
}
