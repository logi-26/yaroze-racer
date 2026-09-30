#include "link_race.h"
#include "../engine/link.h"
#include "../engine/font.h"
#include "../engine/colours.h"
#include "../engine/graphics.h"
#include "../engine/calculations.h"
#include "../engine/model.h"
#include "player.h"
#include "game.h"
#include "ground.h"
#include "ai_racer.h"

// Grid positions: player 1 at the front, player 2 behind (the first AI slot)
#define GRID_Y          (-200)
#define P1_START_X      3605
#define P1_START_Z      9273
#define P2_START_X      3000
#define P2_START_Z      8800

static int active = 0;          // This race is a link race
static int finishPlace;         // 0 while racing, then 1 or 2
static int opponentFinished;    // The other player has finished
static int connectionLost;      // The other console has gone


// Place the other player's car where its console says it is
static void MoveOpponent(long x, long z, int heading)
{
    MATRIX matTmp;

    player2.rotation.vx = 0;
    player2.rotation.vy = heading;
    player2.rotation.vz = 0;

    ResetMatrix(player2.gsObjectCoord.coord.m);
    RotMatrix(&player2.rotation, &matTmp);
    MulMatrix0(&player2.gsObjectCoord.coord, &matTmp, &player2.gsObjectCoord.coord);

    player2.gsObjectCoord.coord.t[0] = x;
    player2.gsObjectCoord.coord.t[2] = z;
    player2.gsObjectCoord.flg = 0;
}


// Collision detection for the remote player (its console does the same for its car)
static void CollideWithOpponent(void)
{
    long dx, dz, combinedRadius, absDx, absDz, approxDist;

    dx = player1.gsObjectCoord.coord.t[0] - player2.gsObjectCoord.coord.t[0];
    dz = player1.gsObjectCoord.coord.t[2] - player2.gsObjectCoord.coord.t[2];
    combinedRadius = player1.collisionRadius + player2.collisionRadius;

    if (dx > combinedRadius || dx < -combinedRadius || dz > combinedRadius || dz < -combinedRadius)
        return;
    if (dx * dx + dz * dz >= combinedRadius * combinedRadius)
        return;

    // Cheap approximation of the distance (as for the AI collisions)
    absDx = dx < 0 ? -dx : dx;
    absDz = dz < 0 ? -dz : dz;
    approxDist = ((absDx > absDz ? absDx : absDz) * 123 + (absDx > absDz ? absDz : absDx) * 51) >> 7;

    // Push our car out to the edge of the other one, and slow it down
    if (approxDist < 1)
    {
        player1.gsObjectCoord.coord.t[0] += combinedRadius;
    }
    else
    {
        player1.gsObjectCoord.coord.t[0] = player2.gsObjectCoord.coord.t[0] + (dx * combinedRadius) / approxDist;
        player1.gsObjectCoord.coord.t[2] = player2.gsObjectCoord.coord.t[2] + (dz * combinedRadius) / approxDist;
    }
    player1.gsObjectCoord.flg = 0;
    player1.speed /= 2;
    player1_lateralSpeed = 0;
}


// Send our car's state to the other console
static void SendState(void)
{
    long x = player1.gsObjectCoord.coord.t[0];
    long z = player1.gsObjectCoord.coord.t[2];

    // The track is well inside this range (clamp anything off the edge)
    if (x < 0) x = 0;
    if (x > LINK_RACE_MAX_POS) x = LINK_RACE_MAX_POS;
    if (z < 0) z = 0;
    if (z > LINK_RACE_MAX_POS) z = LINK_RACE_MAX_POS;

    Link_SetLocalData(LINK_RACE_STATUS(finishPlace != 0, player1.rotation.vy));
    Link_SetLocalExtra(LINK_RACE_EXTRA(x, z, selectedTrackIndex));
}


/*****************************************************
Public Interface
*****************************************************/

// Call when the race starts: it's a link race if the link is connected
void LinkRace_Begin(void)
{
    active = (Link_GetStatus() == LINK_CONNECTED && remoteVehicleIndex >= 0);
    finishPlace = 0;
    opponentFinished = 0;
    connectionLost = 0;
}


int LinkRace_IsActive(void)
{
    return active;
}


// This console's grid position (player 1's in a normal race)
void LinkRace_GetStart(long *x, long *z)
{
    if (active && Link_GetPlayerNumber() == 2)
    {
        *x = P2_START_X;
        *z = P2_START_Z;
    }
    else
    {
        *x = P1_START_X;
        *z = P1_START_Z;
    }
}


// Put the other player's car on its grid position
void LinkRace_InitOpponent(unsigned long *tmdAddr)
{
    long x, z;
    SVECTOR modelRot = {0, 0, 0, 0};

    if (Link_GetPlayerNumber() == 2)
    {
        x = P1_START_X;
        z = P1_START_Z;
    }
    else
    {
        x = P2_START_X;
        z = P2_START_Z;
    }

    InitialisePlayer(&player2, 2, x, GRID_Y, z, tmdAddr);
    RotModel(&player2.gsModelCoord, &modelRot, 3072, 2048, 0);
    InitialiseOpponentRaceProgress(x, z);
    SendState();
}


// Call once per frame after our car has moved
void LinkRace_Update(void)
{
    int remote;
    u_long extra;

    if (!active)
        return;

    if (!connectionLost && Link_GetStatus() != LINK_CONNECTED)
    {
        // Lost connection with the other console (the linked race has to end)
        connectionLost = 1;
        Link_Close();
    }

    if (!connectionLost)
    {
        remote = Link_GetRemoteData();
        extra = Link_GetRemoteExtra();

        // Its race state (it may still be starting: nothing yet, or no position)
        if (remote >= 0 && LINK_STATUS_STAGE(remote) == LINK_STAGE_RACE &&
            (LINK_RACE_X(extra) != 0 || LINK_RACE_Z(extra) != 0))
        {
            MoveOpponent(LINK_RACE_X(extra), LINK_RACE_Z(extra), LINK_RACE_HEADING(remote));
            if (LINK_RACE_FINISHED(remote))
                opponentFinished = 1;
        }
    }

    CollideWithOpponent();

    // Race positions between the two players
    UpdateOpponentRaceProgress(player2.gsObjectCoord.coord.t[0], player2.gsObjectCoord.coord.t[2]);
    UpdateLinkRacePositions(player2.gsObjectCoord.coord.t[0], player2.gsObjectCoord.coord.t[2]);

    // Crossing the line after the last lap: first or second
    if (!finishPlace && playerRaceLapCount >= NUM_RACE_LAPS)
        finishPlace = opponentFinished ? 2 : 1;
    if (finishPlace)
        playerRacePosition = finishPlace;

    if (!connectionLost)
        SendState();
}


// Leaving is allowed once we've finished or the other player has gone
int LinkRace_CanLeave(void)
{
    return active && (finishPlace != 0 || connectionLost);
}


// Leave the link race (closes the link)
void LinkRace_End(void)
{
    Link_Close();
    active = 0;
}


// The race result/messages over the HUD
void LinkRace_DrawHUD(GsOT *ot)
{
    if (!active)
        return;

    FontFX_FontBegin();
    FontFX_SetStyle(FONT_STYLE_2);
    FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);

    if (finishPlace)
    {
        FontFX_SetSize(2);
        FontFX_SetColour(finishPlace == 1 ? COL_GOLD : COL_WHITE);
        FontFX_SetOutline(COL_DARKGREY);
        FontFX_Print(20, 80, finishPlace == 1 ? "YOU WIN!" : "YOU LOSE", ot, OT_UI);
        FontFX_SetSize(1);
    }
    else if (connectionLost)
    {
        FontFX_SetColour(COL_WHITE);
        FontFX_SetOutline(COL_DARKGREY);
        FontFX_Print(20, 80, "THE OTHER PLAYER HAS LEFT", ot, OT_UI);
    }
    else if (opponentFinished)
    {
        FontFX_SetColour(COL_WHITE);
        FontFX_SetOutline(COL_DARKGREY);
        FontFX_Print(20, 80, "THE OTHER PLAYER HAS FINISHED", ot, OT_UI);
    }
    FontFX_FontEnd();

    if (LinkRace_CanLeave())
    {
        FontFX_FontBegin();
        FontFX_SetCenter(SCREEN_X_OFFSET, gScreenWidth);
        FontFX_SetColour(COL_WHITE);
        FontFX_SetOutline(COL_DARKGREY);
        FontFX_SetPulse(0, 255, 20);
        FontFX_Print(20, 110, "PRESS START", ot, OT_UI);
        FontFX_FontEnd();
    }
}
