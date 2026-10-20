#include <libps.h>
#include "calculations.h"

// Distances too large to square without overflowing
// Return the largest int (far away)
#define DISTANCE_MAX_AXIS (26754)    // 3 * 26754^2 < 2^31

int CalculateDistanceSquared(int x1, int y1, int z1, int x2, int y2, int z2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int dz = z2 - z1;
    if (dx > DISTANCE_MAX_AXIS || dx < -DISTANCE_MAX_AXIS ||
        dy > DISTANCE_MAX_AXIS || dy < -DISTANCE_MAX_AXIS ||
        dz > DISTANCE_MAX_AXIS || dz < -DISTANCE_MAX_AXIS)
        return 0x7FFFFFFF;
    return (dx*dx + dy*dy + dz*dz);
}

// A small random number generator
static unsigned long randomSeed = 1;

void SeedRandom(unsigned long seed) {
    randomSeed = seed;
}

// A random number from 0 to 32767
int RandomNumber(void) {
    randomSeed = randomSeed * 1103515245 + 12345;
    return (int)((randomSeed >> 16) & 0x7FFF);
}

void ResetMatrix(short m[3][3]) {
    m[0][0] = m[1][1] = m[2][2] = ONE;
    m[0][1] = m[0][2] = m[1][0] = m[1][2] = m[2][0] = m[2][1] = 0;
}