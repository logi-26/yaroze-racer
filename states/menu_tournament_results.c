#include "menu_tournament_results.h"
#include "menu_main.h"

#include "../engine/state_manager.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/controller.h"
#include "../engine/audio.h"
#include "../engine/graphics.h"
#include "../engine/ui.h"
#include "../engine/lang.h"
#include "../game/tournament.h"

#include "../game/ai_drivers.h"

/*****************************************************
The race results (each drivers position, their points for the race and their points so far)
After the last race X or Start then shows the final standings (each drivers position in the tournament and their points)
X or Start goes on to the next race or back to the main menu after the final standings
*****************************************************/

enum { PAGE_RACE, PAGE_FINAL };

// The rows slide in from the right one after the other
#define ROW_DELAY      4                                // Frames between each row starting to slide in
#define ROW_SLIDE      10                               // Frames a row takes to slide in
#define ANIM_FRAMES    (ROW_DELAY * (TOURNAMENT_COMPETITORS - 1) + ROW_SLIDE)

// The table
#define TITLE_Y        12
#define SUBTITLE_Y     34
#define HEADER_Y       56
#define ROW_Y          72
#define ROW_H          16
#define TABLE_X        20
#define TABLE_W        280
#define COL_POS        28
#define COL_NAME       60
#define COL_COUNTRY    188
#define COL_POINTS     224
#define COL_TOTAL      260
#define WINNER_Y       (ROW_Y + TOURNAMENT_COMPETITORS * ROW_H + 10)
#define PROMPT_Y       216

static int stateInitialised = 0;
static int page;
static int frames;                  // Frames since the page was shown (for the rows sliding in)


static void StateInit(void)
{
    page = PAGE_RACE;
    frames = 0;
    SetBackgroundColor(20, 20, 40);
    stateInitialised = 1;
}


static void StateDeinitialise(void)
{
    stateInitialised = 0;
}


static void UpdateResults(void)
{
    // The first frame only sets up the screen (the button that ended the race does nothing here)
    if (!stateInitialised)
    {
        StateInit();
        return;
    }

    // There is no race result to show (back to the main menu)
    if (!Tournament_IsActive() || !tournament.raceFinalised)
    {
        StateDeinitialise();
        gameState = STATE_MENU_MAIN;
        return;
    }

    if (frames < ANIM_FRAMES)
    {
        frames++;
    }

    if (!BTN_PRESSED(PADcross) && !BTN_PRESSED(PADstart))
    {
        return;
    }

    // While the rows are sliding in the first press shows them all
    if (frames < ANIM_FRAMES)
    {
        frames = ANIM_FRAMES;
        return;
    }

    if (page == PAGE_RACE)
    {
        if (Tournament_HasNextRace())
        {
            // On to the next race (NEXT RACE on the track screen)
            Tournament_AdvanceRace();
            StateDeinitialise();
            gameState = STATE_MENU_TRACK_SELECT;
        }
        else
        {
            // The last race (the final standings)
            page = PAGE_FINAL;
            frames = 0;
        }
        return;
    }

    // The tournament is over (back to the one player game modes which ends it)
    StateDeinitialise();
    MenuMainReturnFromTournament();
    gameState = STATE_MENU_MAIN;
    RestartMusic();
    PauseMusic();
}


// How far a row is to the right while it slides in (0 once it is in place)
static int RowOffset(int row)
{
    int left = ROW_DELAY * row + ROW_SLIDE - frames;

    if (left <= 0)
    {
        return 0;
    }

    if (left > ROW_SLIDE)
    {
        left = ROW_SLIDE;
    }

    return left * gScreenWidth / ROW_SLIDE;
}


static void PrintText(int x, int y, char *text, unsigned char r, unsigned char g, unsigned char b, GsOT *ot)
{
    FontFX_FontBegin();
    FontFX_SetStyle(FONT_STYLE_0);
    FontFX_SetColour(r, g, b);
    FontFX_SetOutline(COL_DARKGREY);
    FontFX_Print(x, y, text, ot, OT_UI);
    FontFX_FontEnd();
}


static void PrintCentred(int y, int size, char *text, unsigned char r, unsigned char g, unsigned char b, int pulse, GsOT *ot)
{
    FontFX_FontBegin();
    FontFX_SetStyle(FONT_STYLE_2);
    FontFX_SetSize(size);
    FontFX_SetColour(r, g, b);
    FontFX_SetOutline(COL_DARKGREY);
    FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);

    if (pulse)
    {
        FontFX_SetPulse(0, 255, 20);
    }
    
    FontFX_Print(20, y, text, ot, OT_UI);
    FontFX_SetSize(1);
    FontFX_FontEnd();
}


// A competitors name (the players in the current language)
static char *CompetitorName(int c)
{
    return (char *)((c == COMPETITOR_PLAYER) ? TXT(TXT_PLAYER) : Tournament_CompetitorName(c));
}


// One row of the table: position, name, country, the race points (the race page only) and the total
static void DrawRow(int row, int c, int racePoints, GsOT *ot)
{
    static const char *places[] = { "1ST", "2ND", "3RD", "4TH", "5TH", "6TH" };
    char text[8];
    int x = RowOffset(row);
    int y = ROW_Y + row * ROW_H;
    unsigned char r = 255, g = 255, b = 255;

    // Off the screen (still to slide in)
    if (x >= gScreenWidth)
    {
        return;
    }

    // The players row is highlighted, the others alternate
    if (c == COMPETITOR_PLAYER)
    {
        DrawRect(TABLE_X + x, y - 4, TABLE_W, ROW_H - 1, 40, 60, 140, SOLID, ot, OT_UI + 2);
    }
    else
    {
        DrawRect(TABLE_X + x, y - 4, TABLE_W, ROW_H - 1, (row & 1) ? 34 : 44, (row & 1) ? 34 : 44, (row & 1) ? 60 : 72, SOLID, ot, OT_UI + 2);
    }

    // 1st place is displayed in gold
    if (row == 0)
    {
        r = 255; g = 215; b = 0;
    }

    PrintText(COL_POS + x, y, (char *)places[row], r, g, b, ot);
    PrintText(COL_NAME + x, y, CompetitorName(c), r, g, b, ot);
    
    if (c != COMPETITOR_PLAYER)
    {
        PrintText(COL_COUNTRY + x, y, (char *)aiDrivers[c - 1].country, r, g, b, ot);
    }

    if (racePoints > 0)
    {
        sprintf(text, "%d", racePoints);
        PrintText(COL_POINTS + x, y, text, r, g, b, ot);
    }

    sprintf(text, "%d", tournament.scores[c]);
    PrintText(COL_TOTAL + x, y, text, r, g, b, ot);
}


static void DrawHeader(int racePage, GsOT *ot)
{
    PrintText(COL_POS, HEADER_Y, (char *)TXT(TXT_POS), 150, 150, 180, ot);
    PrintText(COL_NAME, HEADER_Y, (char *)TXT(TXT_DRIVER), 150, 150, 180, ot);

    if (racePage)
    {
        PrintText(COL_POINTS, HEADER_Y, (char *)TXT(TXT_POINTS), 150, 150, 180, ot);
    }

    PrintText(COL_TOTAL, HEADER_Y, (char *)TXT(TXT_TOTAL), 150, 150, 180, ot);
}


static void RenderResults(void)
{
    GsOT *ot = &WorldOrderingTable[activeBuffer];
    TextID name = (tournament.id == 1) ? TXT_TOURNAMENT_2 : TXT_TOURNAMENT_1;
    char text[64];
    int order[TOURNAMENT_COMPETITORS];
    int row;

    if (!stateInitialised || !Tournament_IsActive())
    {
        return;
    }

    PrintCentred(TITLE_Y, 2, (char *)TXT(page == PAGE_RACE ? TXT_RACE_RESULTS : TXT_FINAL_STANDINGS), COL_GOLD, 0, ot);

    // The tournament (and the race)
    if (page == PAGE_RACE)
    {
        sprintf(text, "%s  %s %d/%d", TXT(name), TXT(TXT_RACE), tournament.raceIndex + 1, TOURNAMENT_RACES);
    }
    else
    {
        sprintf(text, "%s", TXT(name));
    }
    PrintCentred(SUBTITLE_Y, 1, text, COL_WHITE, 0, ot);

    DrawHeader(page == PAGE_RACE, ot);

    if (page == PAGE_RACE)
    {
        // The race: in finishing order with their points for it
        for (row = 0; row < TOURNAMENT_COMPETITORS; row++)
        {
            DrawRow(row, tournament.lastOrder[row], Tournament_RacePoints(row + 1), ot);
        }
    }
    else
    {
        // The tournament: in standings order
        Tournament_GetStandings(order);
        for (row = 0; row < TOURNAMENT_COMPETITORS; row++)
        {
            DrawRow(row, order[row], 0, ot);
        }

        // The winner (once all the rows are in)
        if (frames >= ANIM_FRAMES)
        {
            if (order[0] == COMPETITOR_PLAYER)
            {
                PrintCentred(WINNER_Y, 1, (char *)TXT(TXT_PLAYER_WINS), COL_GOLD, 1, ot);
            }
            else
            {
                sprintf(text, "%s: %s", TXT(TXT_WINNER), CompetitorName(order[0]));
                PrintCentred(WINNER_Y, 1, text, COL_GOLD, 1, ot);
            }
        }
    }

    if (frames >= ANIM_FRAMES)
    {
        PrintCentred(PROMPT_Y, 1, (char *)TXT(TXT_PRESS_X_OR_START), COL_WHITE, 1, ot);
    }
}


void StateMenuTournamentResults(void)
{
    UpdateResults();
    RenderResults();
}
