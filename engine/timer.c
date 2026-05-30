#include "timer.h"

// Stopwatch
int timerRunning = 0;
int timerPaused = 0;
int timerStartFrame = 0;
int timerPausedAccum = 0;
int timerElapsedFrames = 0;

// Countdown
int countdownRunning = 0;
int countdownPaused = 0;
int countdownTotalFrames = 0;
int countdownRemainingFrames = 0;


void TimerStart(void)
{
    if (timerRunning || timerPaused) return;

    timerStartFrame = frameNumber;
    timerPausedAccum = 0;
    timerElapsedFrames = 0;

    timerRunning = 1;
    timerPaused = 0;
}


void TimerStop(void)
{
    if (!timerRunning && !timerPaused) return;

    timerElapsedFrames =
        (frameNumber - timerStartFrame) + timerPausedAccum;

    timerRunning = 0;
    timerPaused = 0;
}


void TimerPause(void)
{
    if (!timerRunning) return;

    timerPausedAccum =
        (frameNumber - timerStartFrame) + timerPausedAccum;

    timerRunning = 0;
    timerPaused = 1;
}


void TimerResume(void)
{
    if (!timerPaused) return;

    timerStartFrame = frameNumber;

    timerRunning = 1;
    timerPaused = 0;
}


void TimerReset(void)
{
    timerStartFrame = frameNumber;
    timerPausedAccum = 0;
    timerElapsedFrames = 0;

    timerRunning = 0;
    timerPaused = 0;
}


void TimerUpdate(void)
{
    if (timerRunning)
    {
        timerElapsedFrames =
            (frameNumber - timerStartFrame) + timerPausedAccum;
    }
}


int TimerGetElapsedFrames(void)
{
    return timerElapsedFrames;
}


int TimerGetElapsedMs(void)
{
    return (timerElapsedFrames * 1000) / FRAME_RATE;
}


void CountdownStart(int frames)
{
    countdownTotalFrames = frames;
    countdownRemainingFrames = frames;

    countdownRunning = 1;
    countdownPaused = 0;
}


void CountdownStop(void)
{
    countdownRunning = 0;
    countdownPaused = 0;
}


void CountdownPause(void)
{
    if (!countdownRunning) return;

    countdownRunning = 0;
    countdownPaused = 1;
}


void CountdownResume(void)
{
    if (!countdownPaused) return;

    countdownRunning = 1;
    countdownPaused = 0;
}


void CountdownReset(void)
{
    countdownRemainingFrames = countdownTotalFrames;

    countdownRunning = 0;
    countdownPaused = 0;
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