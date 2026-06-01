#ifndef TIMER_H
#define TIMER_H

#include "state_manager.h"
#include "graphics.h"


// Stopwatch
extern int timerRunning;
extern int timerPaused;

// Countdown
extern int countdownRunning;
extern int countdownPaused;
extern int countdownTotalFrames;
extern int countdownRemainingFrames;


/************* FUNCTION PROTOTYPES *******************/
void TimerHardwarePoll(void);

void TimerStart(void);
void TimerStop(void);
void TimerPause(void);
void TimerResume(void);
void TimerReset(void);
void TimerUpdate(void);

int TimerGetElapsedMs(void);
int TimerGetElapsedFrames(void);

// Countdown
void CountdownStart(int frames);
void CountdownStop(void);
void CountdownPause(void);
void CountdownResume(void);
void CountdownReset(void);
void CountdownUpdate(void);

int CountdownGetRemainingMs(void);
int CountdownGetRemainingFrames(void);
/*****************************************************/

#endif // TIMER_H