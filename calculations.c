
int CalculateDistanceSquared(int x1, int y1, int z1, int x2, int y2, int z2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int dz = z2 - z1;
    return (dx*dx + dy*dy + dz*dz);
}