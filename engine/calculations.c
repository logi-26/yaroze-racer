#include <libps.h>
#include "calculations.h"

int CalculateDistanceSquared(int x1, int y1, int z1, int x2, int y2, int z2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int dz = z2 - z1;
    return (dx*dx + dy*dy + dz*dz);
}

void ResetMatrix(short m[3][3]) {
    m[0][0] = m[1][1] = m[2][2] = ONE;
    m[0][1] = m[0][2] = m[1][0] = m[1][2] = m[2][0] = m[2][1] = 0;
}