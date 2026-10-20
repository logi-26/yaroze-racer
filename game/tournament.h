#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include "ai_racer.h"

/*****************************************************
Tournament Mode:
- Six races across tracks 1 to 6
- Tournament 1: All races take place during the day
- Tournament 2: All races take place at night
- Compete against five AI drivers (six drivers in total)
- Points are awarded after each race:
  1st = 6 points, 2nd = 5, 3rd = 4, 4th = 3, 5th = 2, 6th = 1
- Points carry over and accumulate across all six races
- Final standings are determined by total points, with tie-breakers applied if necessary
*****************************************************/

#define TOURNAMENT_COUNT 2
#define TOURNAMENT_RACES 6
#define TOURNAMENT_COMPETITORS  (1 + NUM_AI_RACERS)

// Competitor 0 is the player, competitor 1 + i is AI racer i
#define COMPETITOR_PLAYER 0

// A memory card save slot holding a tournament (between two of its races)
#define SAVE_TYPE_TOURNAMENT 1


typedef struct {
    int active;                                                         // A tournament is in progress
    int id;                                                             // 0 = tournament 1 (day), 1 = tournament 2 (night)
    int night;                                                          // Its races are at night
    int raceIndex;                                                      // The race being run (or the next one), 0-5
    int raceFinalised;                                                  // This races points have been given (so they can't be given twice)
    int complete;                                                       // The last race has been finalised
    int playerVehicle;                                                  // The players vehicle
    int aiVehicle[NUM_AI_RACERS];                                       // Each AI racers vehicle
    int scores[TOURNAMENT_COMPETITORS];                                 // Each competitors points so far
    unsigned char results[TOURNAMENT_RACES][TOURNAMENT_COMPETITORS];    // Each competitors position in each race (0 = not raced)
    int lastOrder[TOURNAMENT_COMPETITORS];                              // The last race: the competitor in each position
} TournamentState;


extern TournamentState tournament;


/************* FUNCTION PROTOTYPES *******************/
void Tournament_Begin(int id);
void Tournament_LockVehicles(void);
void Tournament_Clear(void);
int Tournament_IsActive(void);
int Tournament_Track(void);
int Tournament_IsNight(void);
void Tournament_FinaliseRace(const int order[TOURNAMENT_COMPETITORS]);
int Tournament_HasNextRace(void);
void Tournament_AdvanceRace(void);
void Tournament_GetStandings(int order[TOURNAMENT_COMPETITORS]);
int Tournament_RacePoints(int position);
const char *Tournament_CompetitorName(int c);
void Tournament_PrintResults(void);
int Tournament_CanSave(void);
int Tournament_ToSave(unsigned char *data, int size);
int Tournament_FromSave(const unsigned char *data);
/*****************************************************/

#endif // TOURNAMENT_H
