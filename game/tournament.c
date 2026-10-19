#include <string.h>
#include <stdio.h>

#include "tournament.h"
#include "player.h"

TournamentState tournament;

// The tracks in race order (currently in numerical order, may make this more random later)
static const int trackOrder[TOURNAMENT_RACES] = { 0, 1, 2, 3, 4, 5 };


// Start a new tournament (no points, the first race)
void Tournament_Begin(int id)
{
    memset((unsigned char *)&tournament, 0, sizeof(tournament));
    tournament.active = 1;
    tournament.id = (id >= 0 && id < TOURNAMENT_COUNT) ? id : 0;
    tournament.night = (tournament.id == 1);
}


// The player and the AI racers keep the same vehicles for the whole tournament
// (they can only be chosen before the first race)
void Tournament_LockVehicles(void)
{
    if (!tournament.active || tournament.raceIndex > 0 || tournament.raceFinalised)
    {
        return;
    }

    tournament.playerVehicle = selectedVehicleIndex;
    ResolveAIVehicles(tournament.aiVehicle);
}


// End the tournament
void Tournament_Clear(void)
{
    memset((unsigned char *)&tournament, 0, sizeof(tournament));
}


int Tournament_IsActive(void)
{
    return tournament.active;
}


// The track of the race being run (or the next one)
int Tournament_Track(void)
{
    int race = tournament.raceIndex;

    if (race < 0 || race >= TOURNAMENT_RACES)
    {
        race = 0;
    }
    return trackOrder[race];
}


int Tournament_IsNight(void)
{
    return tournament.night;
}


// The points for a race position
int Tournament_RacePoints(int position)
{
    if (position < 1 || position > TOURNAMENT_COMPETITORS)
    {
        return 0;
    }
    return TOURNAMENT_COMPETITORS + 1 - position;
}


// Is the order every competitor once
static int IsValidOrder(const int order[TOURNAMENT_COMPETITORS])
{
    int seen[TOURNAMENT_COMPETITORS];
    int i;

    for (i = 0; i < TOURNAMENT_COMPETITORS; i++)
    {
        seen[i] = 0;
    }

    for (i = 0; i < TOURNAMENT_COMPETITORS; i++)
    {
        if (order[i] < 0 || order[i] >= TOURNAMENT_COMPETITORS || seen[order[i]])
        {
            return 0;
        }
        seen[order[i]] = 1;
    }
    return 1;
}


// The race is over (order is the competitor in each position)
// Each competitor gets the points for their position only once for each race
void Tournament_FinaliseRace(const int order[TOURNAMENT_COMPETITORS])
{
    int pos;

    if (!tournament.active || tournament.raceFinalised)
    {
        return;
    }

    if (!IsValidOrder(order))
    {
        printf("Tournament: invalid race order\n");
        return;
    }

    for (pos = 0; pos < TOURNAMENT_COMPETITORS; pos++)
    {
        int c = order[pos];
        tournament.results[tournament.raceIndex][c] = (unsigned char)(pos + 1);
        tournament.scores[c] += Tournament_RacePoints(pos + 1);
        tournament.lastOrder[pos] = c;
    }

    tournament.raceFinalised = 1;
    tournament.complete = (tournament.raceIndex == TOURNAMENT_RACES - 1);
}


// Is there another race after this one
int Tournament_HasNextRace(void)
{
    return tournament.active && tournament.raceIndex < TOURNAMENT_RACES - 1;
}


// On to the next race (once this one has been finalised)
void Tournament_AdvanceRace(void)
{
    if (!tournament.raceFinalised || !Tournament_HasNextRace())
    {
        return;
    }

    tournament.raceIndex++;
    tournament.raceFinalised = 0;
}


// The number of times a competitor finished in a position
static int PositionCount(int c, int position)
{
    int r, count = 0;

    for (r = 0; r < TOURNAMENT_RACES; r++)
    {
        if (tournament.results[r][c] == position)
        {
            count++;
        }
    }
    return count;
}


// Is the competitor A ahead of competitor B in the standings, follow this logic:
// 1. Most points
// 2. If tied on points, the player is ahead
// 3. If still tied, countback: most wins, then most 2nds, then most 3rds, etc
// 4. If still tied, the better position in the most recent race
// 5. If still tied, the competitor higher in the driver order
static int IsAhead(int a, int b)
{
    int pos, r;

    if (tournament.scores[a] != tournament.scores[b])
    {
        return tournament.scores[a] > tournament.scores[b];
    }

    if (a == COMPETITOR_PLAYER || b == COMPETITOR_PLAYER)
    {
        return a == COMPETITOR_PLAYER;
    }

    for (pos = 1; pos <= TOURNAMENT_COMPETITORS; pos++)
    {
        int countA = PositionCount(a, pos);
        int countB = PositionCount(b, pos);
        if (countA != countB)
        {
            return countA > countB;
        }
    }

    for (r = TOURNAMENT_RACES - 1; r >= 0; r--)
    {
        int posA = tournament.results[r][a];
        int posB = tournament.results[r][b];
        if (posA && posB && posA != posB)
        {
            return posA < posB;
        }
    }

    return a < b;
}


// The competitors in standings order
void Tournament_GetStandings(int order[TOURNAMENT_COMPETITORS])
{
    int i, j;

    for (i = 0; i < TOURNAMENT_COMPETITORS; i++)
    {
        int c = i;

        // Insert it after everyone ahead of it
        for (j = i; j > 0 && IsAhead(c, order[j - 1]); j--)
        {
            order[j] = order[j - 1];
        }
        order[j] = c;
    }
}
