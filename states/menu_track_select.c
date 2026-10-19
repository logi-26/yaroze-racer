#include "menu_track_select.h"

#include "../engine/state_manager.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/controller.h"
#include "../engine/graphics.h"
#include "../engine/link.h"
#include "../engine/ui.h"
#include "../engine/lang.h"
#include "../engine/audio.h"
#include "../engine/message.h"
#include "../game/ground.h"
#include "../game/tournament.h"

static int stateInitialised = 0;
static int selectedItem = 0;
static int shownItem = 0;    // The track the preview shows (follows selectedItem, with a slide)

static const char *trackNames[NUM_TRACKS] = {
    "TRACK 1",
    "TRACK 2",
    "TRACK 3",
    "TRACK 4",
    "TRACK 5",
    "TRACK 6"
};

// The preview panel (each tracks map is fitted inside it)
#define PANEL_X 56
#define PANEL_Y 38
#define PANEL_W 208
#define PANEL_H 150
#define MAX_TILE_PX 8

// Sliding to the next/previous track (the same speed as the main menus slide)
#define SLIDE_FRAMES 10
static int slideFrames = 0;         // Frames of the slide left (0 = not sliding)
static int slideFrom = 0;           // The track sliding out
static int slideDir = 1;            // 1 = to the next track (comes in from the right), -1 = to the previous

// The arrows (they flash for a moment when pressed)
#define ARROW_SIZE 12
#define ARROW_FLASH 8
static int arrowFlash[2];           // Left, right

static int frameCount = 0;

// The map letters colours in the preview (-1 = grass, the panels colour)
enum { PREVIEW_ROAD, PREVIEW_LINE, PREVIEW_GRID, PREVIEW_SAND, PREVIEW_COLOURS };
static const unsigned char previewColour[PREVIEW_COLOURS][3] = {
    { 100, 100, 108 },      // Road
    { 240, 240, 240 },      // Start/finish line
    { 150, 150, 156 },      // Starting grid
    { 180, 160, 100 },      // Sand
};
static signed char letterColour[128];

// Where each tracks map is in the preview (the part of the map with track on it, scaled to fit the panel)
typedef struct {
    int row0, col0, rows, cols;     // The tiles shown
    int tilePx;                     // The size of a tile on the screen
    int x, y;                       // The top left of the map on the screen (when not sliding)
} TrackPreview;
static TrackPreview previews[NUM_TRACKS];


// The preview colour of each map letter
static void InitialiseLetterColours(void)
{
    static const char road[] = "126789acdbefijklmnopqrstu";
    int i;

    for (i = 0; i < 128; i++) 
    {
        letterColour[i] = -1;
    }

    for (i = 0; road[i]; i++) 
    {
        letterColour[(int)road[i]] = PREVIEW_ROAD;
    }

    letterColour['4'] = letterColour['5'] = PREVIEW_LINE;
    letterColour['g'] = letterColour['h'] = PREVIEW_GRID;
    letterColour['v'] = PREVIEW_SAND;
}


static int TileColour(char letter)
{
    return (letter > 0) ? letterColour[(int)letter] : -1;
}


// Fit a tracks map in the panel
static void InitialisePreview(int track)
{
    char (*tiles)[GROUND_MAX_X] = TrackTiles(track);
    TrackPreview *p = &previews[track];
    int r, c, r0 = GROUND_MAX_X, r1 = -1, c0 = GROUND_MAX_Z, c1 = -1, sizeW, sizeH;

    for (r = 0; r < GROUND_MAX_X; r++)
    {
        for (c = 0; c < GROUND_MAX_Z; c++)
        {
            if (TileColour(tiles[r][c]) >= 0)
            {
                if (r < r0) r0 = r;
                if (r > r1) r1 = r;
                if (c < c0) c0 = c;
                if (c > c1) c1 = c;
            }
        }
    }

    // No track (the whole map)
    if (r1 < 0)
    {
        r0 = c0 = 0;
        r1 = GROUND_MAX_X - 1;
        c1 = GROUND_MAX_Z - 1;
    }

    if (r0 > 0) r0--;
    if (c0 > 0) c0--;
    if (r1 < GROUND_MAX_X - 1) r1++;
    if (c1 < GROUND_MAX_Z - 1) c1++;

    p->row0 = r0;
    p->col0 = c0;
    p->rows = r1 - r0 + 1;
    p->cols = c1 - c0 + 1;

    // The biggest tile size this track fits the panel at (map rows go down the screen and columns across)
    sizeW = PANEL_W / p->cols;
    sizeH = PANEL_H / p->rows;
    p->tilePx = (sizeW < sizeH) ? sizeW : sizeH;
    
    if (p->tilePx > MAX_TILE_PX) 
    {
        p->tilePx = MAX_TILE_PX;
    }
    
    if (p->tilePx < 1) 
    {
        p->tilePx = 1;
    }
}


// All the tracks at the same tile size (the biggest that the largest track fits at), so the road
// is the same width on all of them, each centred in the panel
static void InitialisePreviews(void)
{
    int i, tilePx = MAX_TILE_PX;

    for (i = 0; i < NUM_TRACKS; i++)
    {
        InitialisePreview(i);
        if (previews[i].tilePx < tilePx)
        {
            tilePx = previews[i].tilePx;
        }
    }

    for (i = 0; i < NUM_TRACKS; i++)
    {
        TrackPreview *p = &previews[i];
        p->tilePx = tilePx;
        p->x = PANEL_X + (PANEL_W - p->cols * tilePx) / 2;
        p->y = PANEL_Y + (PANEL_H - p->rows * tilePx) / 2;
    }
}


static void StateInit(void)
{
    selectedItem = (selectedTrackIndex >= 0 && selectedTrackIndex < NUM_TRACKS) ? selectedTrackIndex : 0;

    // A tournament (its next track which can't be changed)
    if (Tournament_IsActive())
    {
        selectedItem = Tournament_Track();
        Message_Dismiss();
    }

    shownItem = selectedItem;
    slideFrames = 0;
    arrowFlash[0] = arrowFlash[1] = 0;
    stateInitialised = 1;
    SetBackgroundColor(20, 20, 40);

    InitialiseLetterColours();
    InitialisePreviews();
}


static int IsLinkGame(void)
{
    return Link_GetStatus() != LINK_OFF;
}


// Player 1 chooses the track in a link game
static int ChoosesTrack(void)
{
    return !IsLinkGame() || Link_GetPlayerNumber() == 1;
}


static void StartRace(int track, int night)
{
    selectedTrackIndex = track;
    nightMode = night;

    // Link game (tell the other console the race has started, on which track and day or night)
    // The race then keeps sending our cars position with it
    if (IsLinkGame())
    {
        Link_SetLocalData(LINK_RACE_STATUS(0, 0, 0));
        Link_SetLocalExtra(LINK_RACE_EXTRA(0, 0, LINK_RACE_SETUP(track, night)));
    }

    stateInitialised = 0;
    gameState = STATE_GAMEPLAY;
}


// Slide the preview to the selected track (when the player presses left/right, or player 2 follows player 1's choice)
static void UpdateSlide(void)
{
    if (slideFrames > 0)
    {
        slideFrames--;
    }

    if (selectedItem != shownItem)
    {
        slideDir    = ((shownItem + 1) % NUM_TRACKS == selectedItem) ? 1 : -1;
        slideFrom   = shownItem;
        shownItem   = selectedItem;
        slideFrames = SLIDE_FRAMES;
    }

    if (arrowFlash[0] > 0) 
    {
        arrowFlash[0]--;
    }
    
    if (arrowFlash[1] > 0) 
    {
        arrowFlash[1]--;
    }
}


// A tournament: its next track is shown (START RACE for the first race, then NEXT RACE)
// X or Start races on it, in the tournaments day or night. Before the first race Circle goes back
// to the vehicle select, after it Select asks to quit the tournament
static void UpdateTournament(void)
{
    int promptWasOpen = Message_IsActive();

    Message_Update();

    // While the quit prompt is open (and the frame it closes) the buttons are only for the prompt
    if (promptWasOpen)
    {
        // Yes: back to the main menu (which ends the tournament)
        if (!Message_IsActive() && Message_GetResult() == MSG_RESULT_YES)
        {
            stateInitialised = 0;
            gameState = STATE_MENU_MAIN;
            RestartMusic();
            PauseMusic();
        }
        return;
    }

    if (BTN_PRESSED(PADcross) || BTN_PRESSED(PADstart))
    {
        StartRace(Tournament_Track(), Tournament_IsNight());
        return;
    }

    if (tournament.raceIndex == 0)
    {
        if (BTN_PRESSED(PADcircle))
        {
            stateInitialised = 0;
            gameState = STATE_MENU_VEHICLE_SELECT;
        }
    }
    else if (BTN_PRESSED(PADselect))
    {
        MSG_SetPredefinedTheme(MSG_THEME_DEFAULT);
        Message_Prompt(TXT(TXT_QUIT_TOURNAMENT), MSG_YES_NO);
    }
}


static void UpdateMenuTrackSelect(void)
{
    int remote = Link_GetRemoteData();

    if (!stateInitialised)
    {
        StateInit();
    }

    if (Tournament_IsActive())
    {
        UpdateTournament();
        return;
    }

    // The connection with the other console has been lost, return to the lobby
    if (IsLinkGame() && Link_GetStatus() != LINK_CONNECTED)
    {
        stateInitialised = 0;
        gameState = STATE_MENU_LOBBY;
        return;
    }

    if (ChoosesTrack())
    {
        // Left/right (the previous/next track)
        if (BTN_PRESSED(PADLleft))
        {
            selectedItem = (selectedItem + NUM_TRACKS - 1) % NUM_TRACKS;
            arrowFlash[0] = ARROW_FLASH;
        }
        else if (BTN_PRESSED(PADLright))
        {
            selectedItem = (selectedItem + 1) % NUM_TRACKS;
            arrowFlash[1] = ARROW_FLASH;
        }
    }
    else if (remote >= 0 && LINK_STATUS_STAGE(remote) == LINK_STAGE_TRACK && LINK_STATUS_VALUE(remote) < NUM_TRACKS)
    {
        // Player 2 follows player 1's choice
        selectedItem = LINK_STATUS_VALUE(remote);
    }

    UpdateSlide();

    // Back to vehicle select (a link game is left: the other console goes back to the lobby)
    if (BTN_PRESSED(PADcircle))
    {
        stateInitialised = 0;
        if (IsLinkGame())
        {
            Link_Close();
            gameState = STATE_MENU_MAIN;
        }
        else
        {
            gameState = STATE_MENU_VEHICLE_SELECT;
        }
        return;
    }

    if (ChoosesTrack())
    {
        // X (or Start): a day race, Square: a night race
        if (BTN_PRESSED(PADcross) || BTN_PRESSED(PADstart))
        {
            StartRace(selectedItem, 0);
            return;
        }
        if (BTN_PRESSED(PADsquare))
        {
            StartRace(selectedItem, 1);
            return;
        }
    }
    else if (remote >= 0 && LINK_STATUS_STAGE(remote) == LINK_STAGE_RACE && LINK_RACE_TRACK(Link_GetRemoteExtra()) < NUM_TRACKS)
    {
        // Player 1 has chosen (race on that track, day or night)
        StartRace(LINK_RACE_TRACK(Link_GetRemoteExtra()), LINK_RACE_NIGHT(Link_GetRemoteExtra()));
        return;
    }

    // Tell the other console we're here, and which track is highlighted
    if (IsLinkGame())
    {
        Link_SetLocalData(LINK_STATUS(LINK_STAGE_TRACK, selectedItem, 0));
    }
}


// The player 1 start position
static void DrawStartMarker(int track, int offsetX, GsOT *ot)
{
    const TrackPreview *p = &previews[track];
    const StartDef *s = &TrackStarts(track)[START_PLAYER1];
    int px = p->tilePx;
    int sx = p->x + offsetX + (int)(((long)s->z - (long)p->col0 * SEPERATION) * px / SEPERATION);
    int sy = p->y + (int)(((long)s->x - (long)p->row0 * SEPERATION) * px / SEPERATION);
    int pulse = frameCount & 31;
    int glow = (pulse < 16) ? pulse : 31 - pulse;
    unsigned char g = (unsigned char)(160 + glow * 6);

    DrawRect(sx - 2, sy - 2, 4, 4, 255, g, 0, SOLID, ot, OT_UI + 1);
}


// A tracks preview (offsetX: where it is in the slide animation)
static void DrawPreview(int track, int offsetX, GsOT *ot)
{
    char (*tiles)[GROUND_MAX_X] = TrackTiles(track);
    const TrackPreview *p = &previews[track];
    int px = p->tilePx;
    int r, c, start, colour;

    // If it is off the screen, there is nothing to draw
    if (PANEL_X + offsetX >= gScreenWidth || PANEL_X + PANEL_W + offsetX <= 0) 
    {
        return;
    }

    // The frame and the grass
    DrawRect(PANEL_X + offsetX - 2, PANEL_Y - 2, PANEL_W + 4, PANEL_H + 4, 0, 0, 0, SOLID, ot, OT_UI + 4);
    DrawRect(PANEL_X + offsetX, PANEL_Y, PANEL_W, PANEL_H, 34, 84, 34, SOLID, ot, OT_UI + 3);

    // The track
    for (r = 0; r < p->rows; r++)
    {
        const char *row = tiles[p->row0 + r] + p->col0;
        c = 0;
        while (c < p->cols)
        {
            colour = TileColour(row[c]);
            if (colour < 0)
            {
                c++;
                continue;
            }
            start = c;
            while (c < p->cols && TileColour(row[c]) == colour) 
            {
                c++;
            }

            DrawRect(p->x + offsetX + start * px, p->y + r * px, (c - start) * px, px,
                     previewColour[colour][0], previewColour[colour][1], previewColour[colour][2],
                     SOLID, ot, OT_UI + 2);
        }
    }

    DrawStartMarker(track, offsetX, ot);
}


// An arrow beside the panel (dir: -1 = left, 1 = right), pulsing and bright when just pressed
static void DrawArrow(int dir, GsOT *ot)
{
    int flash = arrowFlash[dir > 0];
    int pulse = frameCount & 63;
    int glow = (pulse < 32) ? pulse : 63 - pulse;
    unsigned char level = (unsigned char)(flash ? 255 : 150 + glow * 3);
    int cy = PANEL_Y + PANEL_H / 2;
    int tipX = (dir < 0) ? PANEL_X - 26 : PANEL_X + PANEL_W + 26;
    int i;

    // One column of the triangle at a time (from the tip, getting taller)
    for (i = 0; i < ARROW_SIZE; i++)
    {
        int x = (dir < 0) ? tipX + i : tipX - i;
        DrawRect(x, cy - i, 1, i * 2 + 1, level, level, flash ? 0 : level, SOLID, ot, OT_UI);
    }
}


// A tournament: START RACE or NEXT RACE, the track (no arrows, it can't be changed)
// the tournament and which race it is and how to start it
static void RenderTournament(GsOT *ot)
{
    char text[64];
    TextID name = (tournament.id == 1) ? TXT_TOURNAMENT_2 : TXT_TOURNAMENT_1;

    FontFX_FontBegin();
    FontFX_SetStyle(FONT_STYLE_2);
    FontFX_SetSize(2);
    FontFX_SetColour(COL_GOLD);
    FontFX_SetOutline(COL_WHITE);
    FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
    FontFX_Print(20, 12, (char *)TXT(tournament.raceIndex == 0 ? TXT_START_RACE : TXT_NEXT_RACE), ot, OT_UI);
    FontFX_SetSize(1);
    FontFX_FontEnd();

    DrawPreview(shownItem, 0, ot);

    // The tracks name
    FontFX_FontBegin();
    FontFX_SetStyle(FONT_STYLE_2);
    FontFX_SetSize(2);
    FontFX_SetColour(COL_GREEN);
    FontFX_SetOutline(COL_DARKGREY);
    FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
    FontFX_Print(20, PANEL_Y + PANEL_H + 8, (char *)trackNames[shownItem], ot, OT_UI);
    FontFX_SetSize(1);
    FontFX_FontEnd();

    // The tournament and which of its races this is
    sprintf(text, "%s  %s %d/%d", TXT(name), TXT(TXT_RACE), tournament.raceIndex + 1, TOURNAMENT_RACES);
    FontFX_FontBegin();
    FontFX_SetColour(COL_WHITE);
    FontFX_SetOutline(COL_DARKGREY);
    FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
    FontFX_Print(20, PANEL_Y + PANEL_H + 28, text, ot, OT_UI);
    FontFX_FontEnd();

    // How to start the race (and go back or quit the tournament)
    FontFX_FontBegin();
    FontFX_SetColour(COL_WHITE);
    FontFX_SetOutline(COL_DARKGREY);
    FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
    FontFX_SetPulse(0, 255, 20);
    FontFX_Print(20, PANEL_Y + PANEL_H + 40, (char *)TXT(tournament.raceIndex == 0 ? TXT_TOURNAMENT_PROMPT_FIRST : TXT_TOURNAMENT_PROMPT_NEXT), ot, OT_UI);
    FontFX_FontEnd();

    Message_Render(ot, OT_POPUP);
}


static void RenderMenuTrackSelect(void)
{
    int i, dotsX;
    GsOT *ot = &WorldOrderingTable[activeBuffer];

    frameCount++;

    if (Tournament_IsActive())
    {
        RenderTournament(ot);
        return;
    }

    FontFX_FontBegin();
    FontFX_SetStyle(FONT_STYLE_2);
    FontFX_SetSize(2);
    FontFX_SetColour(COL_DARKGREEN);
    FontFX_SetOutline(COL_WHITE);
    FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
    FontFX_Print(20, 12, "SELECT TRACK", ot, OT_UI);
    FontFX_SetSize(1);
    FontFX_FontEnd();

    // The tracks preview (sliding in, with the last one sliding out, when it has just changed)
    if (slideFrames > 0)
    {
        int f = SLIDE_FRAMES - slideFrames;
        int eased = 256 * f * (2 * SLIDE_FRAMES - f) / (SLIDE_FRAMES * SLIDE_FRAMES);
        DrawPreview(slideFrom, -slideDir * eased * gScreenWidth / 256, ot);
        DrawPreview(shownItem, slideDir * (256 - eased) * gScreenWidth / 256, ot);
    }
    else
    {
        DrawPreview(shownItem, 0, ot);
    }

    // The arrows (only for the player choosing the track)
    if (ChoosesTrack())
    {
        DrawArrow(-1, ot);
        DrawArrow(1, ot);
    }

    // Display the tracks name
    FontFX_FontBegin();
    FontFX_SetStyle(FONT_STYLE_2);
    FontFX_SetSize(2);
    FontFX_SetColour(COL_GREEN);
    FontFX_SetOutline(COL_DARKGREY);
    FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
    FontFX_Print(20, PANEL_Y + PANEL_H + 8, (char *)trackNames[selectedItem], ot, OT_UI);
    FontFX_SetSize(1);
    FontFX_FontEnd();

    // Which of the tracks it is (a dot for each, the selected one lit)
    dotsX = gScreenWidth / 2 - (NUM_TRACKS * 10 - 4) / 2;
    for (i = 0; i < NUM_TRACKS; i++)
    {
        if (i == selectedItem)
        {
            DrawRect(dotsX + i * 10, PANEL_Y + PANEL_H + 28, 6, 6, COL_WHITE, SOLID, ot, OT_UI);
        }
        else
        {
            DrawRect(dotsX + i * 10, PANEL_Y + PANEL_H + 28, 6, 6, 70, 70, 90, SOLID, ot, OT_UI);
        }
    }

    FontFX_FontBegin();
    FontFX_SetColour(COL_WHITE);
    FontFX_SetOutline(COL_DARKGREY);
    FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
    if (ChoosesTrack())
    {
        // How to start the race (in the day or at night)
        FontFX_Print(20, PANEL_Y + PANEL_H + 40, "X: DAY  SQUARE: NIGHT  CIRCLE: BACK", ot, OT_UI);
    }
    else
    {
        // Link game (player 2 waits for player 1 to choose the track)
        FontFX_SetPulse(0, 255, 20);
        FontFX_Print(20, PANEL_Y + PANEL_H + 40, "PLAYER 1 IS CHOOSING THE TRACK", ot, OT_UI);
    }
    FontFX_FontEnd();
}


void StateMenuTrackSelect(void)
{
    UpdateMenuTrackSelect();
    RenderMenuTrackSelect();
}
