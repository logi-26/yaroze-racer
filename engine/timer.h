#ifndef TIMER_H
#define TIMER_H

#include "state_manager.h"

// Stopwatch timer
extern int timerRunning;
extern int timerPaused;
extern int timerStartFrame;
extern int timerPausedAccum;
extern int timerElapsedFrames;

// Countdown timer
extern int countdownRunning;
extern int countdownPaused;
extern int countdownTotalFrames;
extern int countdownRemainingFrames;

/************* FUNCTION PROTOTYPES *******************/
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