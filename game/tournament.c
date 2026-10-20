#include <string.h>
#include <stdio.h>

#include "tournament.h"
#include "ai_drivers.h"
#include "player.h"

TournamentState tournament;

// The tracks in race order (currently in numerical order, may make this more random later)
static const int trackOrder[TOURNAMENT_RACES] = { 0, 1, 2, 3, 4, 5 };

// The vehicles (0-14: 3 models in 5 colours)
#define TOURNAMENT_VEHICLES 15

// A tournament saved on the memory card (between two of its races)
#define TOURNAMENT_SAVE_MAGIC   0x54524E59      // "TRNY"
#define TOURNAMENT_SAVE_VERSION 1

typedef struct {
    int magic;
    int version;
    int id;                                                             // Tournament 1 or 2 (0 or 1)
    int raceIndex;                                                      // The next race (1-5: saved after a race)
    int playerVehicle;
    int aiVehicle[NUM_AI_RACERS];
    int scores[TOURNAMENT_COMPETITORS];
    unsigned char results[TOURNAMENT_RACES][TOURNAMENT_COMPETITORS];
    int checksum;                                                       // Checksum of everything above
} TournamentSave;


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


// A competitors name
const char *Tournament_CompetitorName(int c)
{
    if (c > COMPETITOR_PLAYER && c < TOURNAMENT_COMPETITORS)
    {
        return aiDrivers[c - 1].name;
    }
    return "PLAYER";
}


// Print the last races result and the standings (for debugging, on the TTY)
void Tournament_PrintResults(void)
{
    int order[TOURNAMENT_COMPETITORS];
    int pos;

    printf("Tournament %d race %d:\n", tournament.id + 1, tournament.raceIndex + 1);
    for (pos = 0; pos < TOURNAMENT_COMPETITORS; pos++)
    {
        int c = tournament.lastOrder[pos];
        printf("  %d %-16s +%d = %d\n", pos + 1, Tournament_CompetitorName(c), Tournament_RacePoints(pos + 1), tournament.scores[c]);
    }

    Tournament_GetStandings(order);
    printf("Standings:\n");
    for (pos = 0; pos < TOURNAMENT_COMPETITORS; pos++)
    {
        printf("  %d %-16s %d\n", pos + 1, Tournament_CompetitorName(order[pos]), tournament.scores[order[pos]]);
    }
}


// Can the tournament be saved now (between two of its races: after a race, before the next one)
int Tournament_CanSave(void)
{
    return tournament.active && !tournament.raceFinalised && tournament.raceIndex > 0 && tournament.raceIndex < TOURNAMENT_RACES;
}


// The checksum of a save (of everything before its checksum)
static int SaveChecksum(const TournamentSave *save)
{
    const unsigned char *bytes = (const unsigned char *)save;
    unsigned long sum = 0x5A17;
    int i;

    for (i = 0; i < (int)((const unsigned char *)&save->checksum - bytes); i++)
    {
        sum = sum * 31 + bytes[i];
    }

    return (int)(sum & 0x7FFFFFFF);
}


// Put the tournament into a save slots data (returns the size used, or 0 if it can't be saved now)
int Tournament_ToSave(unsigned char *data, int size)
{
    TournamentSave save;
    int i;

    if (!Tournament_CanSave() || size < (int)sizeof(TournamentSave))
    {
        return 0;
    }

    memset((unsigned char *)&save, 0, sizeof(save));
    save.magic = TOURNAMENT_SAVE_MAGIC;
    save.version = TOURNAMENT_SAVE_VERSION;
    save.id = tournament.id;
    save.raceIndex = tournament.raceIndex;
    save.playerVehicle = tournament.playerVehicle;

    for (i = 0; i < NUM_AI_RACERS; i++)
    {
        save.aiVehicle[i] = tournament.aiVehicle[i];
    }

    for (i = 0; i < TOURNAMENT_COMPETITORS; i++)
    {
        save.scores[i] = tournament.scores[i];
    }

    memcpy((unsigned char *)save.results, (unsigned char *)tournament.results, sizeof(save.results));
    save.checksum = SaveChecksum(&save);

    memcpy(data, (unsigned char *)&save, sizeof(save));
    return sizeof(save);
}


// Is a saved tournament complete and correct (the races before the next one each have every
// position once, the others none, and the points match the positions)
static int IsValidSave(const TournamentSave *save)
{
    int r, c;

    if (save->magic != TOURNAMENT_SAVE_MAGIC || save->version != TOURNAMENT_SAVE_VERSION || save->checksum != SaveChecksum(save))
    {
        return 0;
    }

    if (save->id < 0 || save->id >= TOURNAMENT_COUNT || save->raceIndex < 1 || save->raceIndex >= TOURNAMENT_RACES)
    {
        return 0;
    }

    if (save->playerVehicle < 0 || save->playerVehicle >= TOURNAMENT_VEHICLES)
    {
        return 0;
    }

    for (c = 0; c < NUM_AI_RACERS; c++)
    {
        if (save->aiVehicle[c] < 0 || save->aiVehicle[c] >= TOURNAMENT_VEHICLES)
        {
            return 0;
        }
    }

    for (r = 0; r < TOURNAMENT_RACES; r++)
    {
        int seen[TOURNAMENT_COMPETITORS + 1];

        for (c = 0; c <= TOURNAMENT_COMPETITORS; c++)
        {
            seen[c] = 0;
        }

        for (c = 0; c < TOURNAMENT_COMPETITORS; c++)
        {
            int pos = save->results[r][c];

            if (r >= save->raceIndex)
            {
                if (pos != 0)
                {
                    return 0;
                }
            }
            else if (pos < 1 || pos > TOURNAMENT_COMPETITORS || seen[pos]++)
            {
                return 0;
            }
        }
    }

    for (c = 0; c < TOURNAMENT_COMPETITORS; c++)
    {
        int points = 0;

        for (r = 0; r < save->raceIndex; r++)
        {
            points += Tournament_RacePoints(save->results[r][c]);
        }

        if (points != save->scores[c])
        {
            return 0;
        }
    }

    return 1;
}


// Carry on a tournament from a save slots data (returns 0, and changes nothing, if the save isn't a valid tournament)
// It carries on at the next race (NEXT RACE on the track screen)
int Tournament_FromSave(const unsigned char *data)
{
    TournamentSave save;
    int i, c;

    memcpy((unsigned char *)&save, (unsigned char *)data, sizeof(save));
    if (!IsValidSave(&save))
    {
        return 0;
    }

    Tournament_Begin(save.id);
    tournament.raceIndex = save.raceIndex;
    tournament.playerVehicle = save.playerVehicle;

    for (i = 0; i < NUM_AI_RACERS; i++)
    {
        tournament.aiVehicle[i] = save.aiVehicle[i];
    }
    
    for (c = 0; c < TOURNAMENT_COMPETITORS; c++)
    {
        tournament.scores[c] = save.scores[c];

        // The last race's order (from the positions in it)
        tournament.lastOrder[save.results[save.raceIndex - 1][c] - 1] = c;
    }
    memcpy((unsigned char *)tournament.results, (unsigned char *)save.results, sizeof(tournament.results));

    selectedVehicleIndex = tournament.playerVehicle;
    return 1;
}
