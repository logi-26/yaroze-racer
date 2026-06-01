#include "timer.h"


/*****************************************************
Stopwatch — stores and compares pure milliseconds via gameMs
gameMs is updated in GameUpdate() from the hardware VSync count
returned by UpdateGraphicsSystem(), so it tracks real elapsed time
*****************************************************/
int timerRunning = 0;
int timerPaused  = 0;

// Milisecond timers
static int timerStartMs  = 0;
static int timerPausedMs = 0;
static int timerFrozenMs = 0;

void TimerHardwarePoll(void) {}

// Countdown (frame-based)
int countdownRunning         = 0;
int countdownPaused          = 0;
int countdownTotalFrames     = 0;
int countdownRemainingFrames = 0;


void TimerStart(void)
{
    if (timerRunning || timerPaused) return;

    timerStartMs  = gameMs;
    timerPausedMs = 0;
    timerFrozenMs = 0;

    timerRunning = 1;
    timerPaused  = 0;
}


void TimerStop(void)
{
    if (!timerRunning && !timerPaused) return;

    timerFrozenMs = gameMs - timerStartMs + timerPausedMs;

    timerRunning = 0;
    timerPaused  = 0;
}


void TimerPause(void)
{
    if (!timerRunning) return;

    timerPausedMs += gameMs - timerStartMs;

    timerRunning = 0;
    timerPaused  = 1;
}


void TimerResume(void)
{
    if (!timerPaused) return;

    timerStartMs = gameMs;

    timerRunning = 1;
    timerPaused  = 0;
}


void TimerReset(void)
{
    timerStartMs  = gameMs;
    timerPausedMs = 0;
    timerFrozenMs = 0;

    timerRunning = 0;
    timerPaused  = 0;
}


void TimerUpdate(void) {}


int TimerGetElapsedFrames(void)
{
    return TimerGetElapsedMs() * FRAME_RATE / 1000;
}


int TimerGetElapsedMs(void)
{
    return timerRunning
        ? (gameMs - timerStartMs + timerPausedMs)
        : timerFrozenMs;
}


void CountdownStart(int frames)
{
    countdownTotalFrames     = frames;
    countdownRemainingFrames = frames;

    countdownRunning = 1;
    countdownPaused  = 0;
}


void CountdownStop(void)
{
    countdownRunning = 0;
    countdownPaused  = 0;
}


void CountdownPause(void)
{
    if (!countdownRunning) return;

    countdownRunning = 0;
    countdownPaused  = 1;
}


void CountdownResume(void)
{
    if (!countdownPaused) return;

    countdownRunning = 1;
    countdownPaused  = 0;
}


void CountdownReset(void)
{
    countdownRemainingFrames = countdownTotalFrames;

    countdownRunning = 0;
    countdownPaused  = 0;
}


void CountdownUpdate(void)
{
    if (!countdownRunning) return;

    if (countdownRemainingFrames > 0)
        countdownRemainingFrames--;
    else
        countdownRunning = 0;
}


int CountdownGetRemainingFrames(void)
{
    return countdownRemainingFrames;
}


int CountdownGetRemainingMs(void)
{
    return (countdownRemainingFrames * 1000) / FRAME_RATE;
}