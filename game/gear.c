#include "gear.h"

int currentGear = 1;
GearboxMode gearboxMode = GEAR_AUTO;

// Top speed of each gear as percentage of vehicle maxSpeed
static const int gearTopSpeedPct[5] = { 22, 40, 60, 80, 100 };

// Acceleration multiplier per gear as percentage — lower gear gives more torque
static const int gearAccelPct[5] = { 160, 130, 100, 75, 55 };

// Auto upshift: engage next gear when speed exceeds this percentage of maxSpeed
static const int upshiftPct[5] = { 20, 38, 57, 76, 100 };

// Auto downshift: drop a gear when speed falls below this percentage of maxSpeed
static const int downshiftPct[5] = { 0, 8, 18, 35, 52 };


void UpdateGearbox(long speed, long maxSpeed, int movementDirection) {
    long threshold;
    if (maxSpeed <= 0L) return;

    if (gearboxMode == GEAR_AUTO) {
        
		// Engage R when going backward or pressing reverse at zero speed
        if (speed < 0L || (movementDirection < 0 && speed <= 0L)) {
            currentGear = 0;
            return;
        }
		
        // Exit R: switch to 1st when stopped and no longer reversing
        if (currentGear == 0) {
            if (speed >= 0L && movementDirection >= 0)
                currentGear = 1;
            return;
        }
		
        // Normal forward auto-shift
        if (movementDirection > 0 && currentGear < 5) {
            threshold = (long)maxSpeed * upshiftPct[currentGear - 1] / 100L;
            if (speed >= threshold)
                currentGear++;
        }
        if (currentGear > 1) {
            threshold = (long)maxSpeed * downshiftPct[currentGear - 1] / 100L;
            if (speed <= threshold)
                currentGear--;
        }
    }
}


long GetGearTopSpeed(long maxSpeed) {
    if (currentGear < 1 || currentGear > 5) return maxSpeed;
    return maxSpeed * gearTopSpeedPct[currentGear - 1] / 100L;
}


void ApplyGearAccel(long *massedAccel, long speed, long maxSpeed) {
    long gearBottom, gearTop, gearRange, posInGear, powerBand, baseAccel;

    if (maxSpeed <= 0L || currentGear < 1 || currentGear > 5) return;

    gearBottom = (currentGear > 1)
               ? ((long)maxSpeed * gearTopSpeedPct[currentGear - 2] / 100L)
               : 0L;
    gearTop   = (long)maxSpeed * gearTopSpeedPct[currentGear - 1] / 100L;
    gearRange = gearTop - gearBottom;

    if (speed < gearBottom) {
        
		// Under-revving (lugging): power falls as (speed/gearBottom)^2
        // 1% at speed 0, 100% at gearBottom — steep penalty at low speeds
        if (gearBottom > 0L) {
            long speedRatio = speed * 100L / gearBottom;  /* 0..100 */
            powerBand = 1L + (99L * speedRatio * speedRatio / 10000L);
        } else {
            powerBand = 100L;
        }
    } else {
		
        // Within gear range: full torque at gear bottom, 55% at redline
        posInGear = speed - gearBottom;
        if (posInGear > gearRange) posInGear = gearRange;
        if (gearRange > 0L) {
            powerBand = 100L - (45L * posInGear / gearRange);
        } else {
            powerBand = 100L;
        }
    }

    baseAccel = (long)*massedAccel * gearAccelPct[currentGear - 1] / 100L;
    baseAccel = baseAccel * powerBand / 100L;
    if (baseAccel < 1L) baseAccel = 1L;
    *massedAccel = baseAccel;
}