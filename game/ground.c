#include <libps.h>
#include "ground.h"
#include "map_1.h"
#include "map_2.h"
#include "map_3.h"
#include "map_4.h"
#include "map_5.h"
#include "map_6.h"
#include "game.h"
#include "../engine/graphics.h"
#include "../engine/model.h"

int selectedTrackIndex = 0;
int nightMode = 1;

static char (*activeMap)[GROUND_MAX_X] = groundDataMap1;

const Waypoint *trackWaypoints = waypointsMap1;
int numTrackWaypoints = NUM_WAYPOINTS_MAP1;
long trackArenaX = 0;
long trackArenaZ = 0;
long trackArenaSize = 30 * SEPERATION;
const BarrierDef *trackBarriers = barriersMap1;
int numTrackBarriers = NUM_BARRIERS_MAP1;
const BuildingDef *trackBuildings = buildingsMap1;
int numTrackBuildings = NUM_BUILDINGS_MAP1;
const StartDef *trackStarts = startsMap1;

// The tracks, in the order of the track select menu
static const struct {
	char (*tiles)[GROUND_MAX_X];
	const Waypoint *waypoints;
	int numWaypoints;
	long arenaX;
	long arenaZ;
	long arenaSize;
	const BarrierDef *barriers;
	int numBarriers;
	const BuildingDef *buildings;
	int numBuildings;
	const StartDef *starts;
} tracks[NUM_TRACKS] = {
	{ groundDataMap1, waypointsMap1, NUM_WAYPOINTS_MAP1, ARENA_X_MAP1, ARENA_Z_MAP1, ARENA_SIZE_MAP1,
	  barriersMap1, NUM_BARRIERS_MAP1, buildingsMap1, NUM_BUILDINGS_MAP1, startsMap1 },
	{ groundDataMap2, waypointsMap2, NUM_WAYPOINTS_MAP2, ARENA_X_MAP2, ARENA_Z_MAP2, ARENA_SIZE_MAP2,
	  barriersMap2, NUM_BARRIERS_MAP2, buildingsMap2, NUM_BUILDINGS_MAP2, startsMap2 },
	{ groundDataMap3, waypointsMap3, NUM_WAYPOINTS_MAP3, ARENA_X_MAP3, ARENA_Z_MAP3, ARENA_SIZE_MAP3,
	  barriersMap3, NUM_BARRIERS_MAP3, buildingsMap3, NUM_BUILDINGS_MAP3, startsMap3 },
	{ groundDataMap4, waypointsMap4, NUM_WAYPOINTS_MAP4, ARENA_X_MAP4, ARENA_Z_MAP4, ARENA_SIZE_MAP4,
	  barriersMap4, NUM_BARRIERS_MAP4, buildingsMap4, NUM_BUILDINGS_MAP4, startsMap4 },
	{ groundDataMap5, waypointsMap5, NUM_WAYPOINTS_MAP5, ARENA_X_MAP5, ARENA_Z_MAP5, ARENA_SIZE_MAP5,
	  barriersMap5, NUM_BARRIERS_MAP5, buildingsMap5, NUM_BUILDINGS_MAP5, startsMap5 },
	{ groundDataMap6, waypointsMap6, NUM_WAYPOINTS_MAP6, ARENA_X_MAP6, ARENA_Z_MAP6, ARENA_SIZE_MAP6,
	  barriersMap6, NUM_BARRIERS_MAP6, buildingsMap6, NUM_BUILDINGS_MAP6, startsMap6 },
};


/*****************************************************
The start line

The line is painted across the middle of the start line tiles ('4' / '5', on a road along Z).
The laps (and the race positions) change where the waypoints wrap round to waypoint 0, so the
race uses a copy of the track's waypoints with one added where they cross the line, as the last one.
*****************************************************/
#define MAX_LAP_WAYPOINTS (512)
static Waypoint lapWaypoints[MAX_LAP_WAYPOINTS + 1];

static int hasStartLine = 0;
static long lineZ;                  // The start/finish line
static long lineX0, lineX1;         // From X0 to X1
static int lineDir = 1;             // The way the race crosses it (+1 = towards +Z, -1 = towards -Z)


// Find the start/finish line on the map, and set the race's waypoints
static void FindStartLine(const Waypoint *wps, int num) {
	int r, c, i, k;
	int col = -1, rowMin = GROUND_MAX_Z, rowMax = -1;

	hasStartLine = 0;
	lineDir = 1;
	trackWaypoints = wps;
	numTrackWaypoints = num;

	// The line tiles (one column of them across the road)
	for (r = 0; r < GROUND_MAX_Z; r++) {
		for (c = 0; c < GROUND_MAX_X; c++) {
			if ((activeMap[r][c] == '4' || activeMap[r][c] == '5') && (col < 0 || c == col)) {
				col = c;
				if (r < rowMin) rowMin = r;
				if (r > rowMax) rowMax = r;
			}
		}
	}
	if (col < 0)
		return;
	lineZ = (long)col * SEPERATION + SEPERATION / 2;
	lineX0 = (long)rowMin * SEPERATION;
	lineX1 = (long)(rowMax + 1) * SEPERATION;
	hasStartLine = 1;

	if (num < 2 || num > MAX_LAP_WAYPOINTS)
		return;

	// The waypoint segment that crosses the line
	for (i = 0; i < num; i++) {
		const Waypoint *a = &wps[i];
		const Waypoint *b = &wps[(i + 1) % num];
		long dz = b->z - a->z;
		long t, x, toA, toB;
		int start, insert;

		if (dz == 0 || !((a->z < lineZ && b->z >= lineZ) || (a->z > lineZ && b->z <= lineZ)))
			continue;
		t = (lineZ - a->z) * 1024 / dz;
		x = a->x + (b->x - a->x) * t / 1024;
		if (x < lineX0 - SEPERATION || x > lineX1 + SEPERATION)
			continue;
		lineDir = (dz > 0) ? 1 : -1;

		// From the first waypoint past the line round to the line itself
		// (a waypoint already on the line is used as it is)
		toA = (lineZ > a->z) ? lineZ - a->z : a->z - lineZ;
		toB = (lineZ > b->z) ? lineZ - b->z : b->z - lineZ;
		if (toA < 50) {
			start = (i + 1) % num;
			insert = 0;
		} else if (toB < 50) {
			start = (i + 2) % num;
			insert = 0;
		} else {
			start = (i + 1) % num;
			insert = 1;
		}
		for (k = 0; k < num; k++)
			lapWaypoints[k] = wps[(start + k) % num];
		if (insert) {
			lapWaypoints[num].x = x;
			lapWaypoints[num].z = lineZ;
		}
		trackWaypoints = lapWaypoints;
		numTrackWaypoints = num + insert;
		return;
	}
}


// Whether a car has just crossed the start/finish line (it's within a tile past the line)
int IsOnStartLine(long worldX, long worldZ) {
	long past;

	if (!hasStartLine || worldX < lineX0 || worldX >= lineX1)
		return 0;
	past = (worldZ - lineZ) * lineDir;
	return past >= 0 && past < SEPERATION;
}


// Return the type of terrain (detect when player is off the track)
TerrainType GetTerrainType(long worldX, long worldZ) {
    int tileRow = (int)(worldX / SEPERATION);
    int tileCol = (int)(worldZ / SEPERATION);
    char tile;

    if (tileRow < 0) tileRow = 0;
    if (tileRow >= GROUND_MAX_Z) tileRow = GROUND_MAX_Z - 1;
    if (tileCol < 0) tileCol = 0;
    if (tileCol >= GROUND_MAX_X) tileCol = GROUND_MAX_X - 1;

    tile = activeMap[tileRow][tileCol];
    if (tile == '3') return TERRAIN_GRASS;
    if (tile == 'v') return TERRAIN_SAND;
    return TERRAIN_TRACK;
}


void InitialiseGroundTextures() {

	// Load ground textures into vram
	LoadTexture(LINE_L_TEX_MEM_ADDR);					
	LoadTexture(LINE_R_TEX_MEM_ADDR);					
	LoadTexture(STRAIGHT_L_1_TEX_MEM_ADDR);								
	LoadTexture(TURN_L_1_TEX_MEM_ADDR);						
	LoadTexture(TURN_R_1_TEX_MEM_ADDR);	
	LoadTexture(GRID_TEX_MEM_ADDR);				
	LoadTexture(STRAIGHT_L_01_TEX_MEM_ADDR);							
	LoadTexture(GRASS_TEX_MEM_ADDR); 
	LoadTexture(TURN_00_TEX_MEM_ADDR); 
	LoadTexture(TURN_01_TEX_MEM_ADDR); 
	LoadTexture(TURN_02_TEX_MEM_ADDR);
	
	LoadTexture(SAND_TEX_MEM_ADDR); 
}


/*
1 = STRAIGHT_L_1_MEM_ADDR 					data\track\st_l_1\st_l_1
2 = STRAIGHT_L_1_MEM_ADDR + rotate 180 		*data\track\st_l_1\st_l_1
3 = GRASS_MEM_ADDR          				data\track\grass_1\grass_1
4 = LINE_L_MEM_ADDR           				data\track\line_l\line_l
5 = LINE_L_MEM_ADDR + rotate 180           	*data\track\line_l\line_l
6 = STRAIGHT_L_1_MEM_ADDR + rotate 90		*data\track\st_l_1\st_l_1
7 = STRAIGHT_L_1_MEM_ADDR + rotate 270		*data\track\st_l_1\st_l_1
8 = TURN_L_1_MEM_ADDR 						data\track\t_l_1\t_l_1
9 = TURN_R_1_MEM_ADDR 						data\track\t_r_1\t_r_1
a = TURN_L_1_MEM_ADDR + rotate 90			*data\track\t_l_1\t_l_1
b = TURN_00_MEM_ADDR 						data\track\t_00\t_00
c = TURN_L_1_MEM_ADDR + rotate 180			*data\track\t_l_1\t_l_1
d = TURN_L_1_MEM_ADDR + rotate 270			*data\track\t_l_1\t_l_1
e = TURN_00_MEM_ADDR + rotate 90			data\track\t_00\t_00
f = TURN_00_MEM_ADDR + rotate 180			*data\track\t_00\t_00
g = GRID_MEM_ADDR         				    data\track\grid\grid
h = GRID_MEM_ADDR + rotate 180        	    *data\track\grid_l\grid_l
i = STRAIGHT_L_01_MEM_ADDR					data\track\st_l_01\st_l_01
j = STRAIGHT_L_01_MEM_ADDR + rotate 90		*data\track\st_l_01\st_l_01
k = STRAIGHT_L_01_MEM_ADDR + rotate 180		*data\track\st_l_01\st_l_01
l = STRAIGHT_L_01_MEM_ADDR + rotate 270		*data\track\st_l_01\st_l_01
m = TURN_00_MEM_ADDR + rotate 270			*data\track\t_00\t_00
n = TURN_01_MEM_ADDR						data\track\t_01\t_01
o = TURN_02_MEM_ADDR						data\track\t_02\t_02
p = TURN_01_MEM_ADDR + rotate 90			*data\track\t_01\t_01
q = TURN_01_MEM_ADDR + rotate 180			*data\track\t_01\t_01
r = TURN_01_MEM_ADDR + rotate 270			*data\track\t_01\t_01
s = TURN_02_MEM_ADDR + rotate 90			*data\track\t_02\t_02
t = TURN_02_MEM_ADDR + rotate 180			*data\track\t_02\t_02
u = TURN_02_MEM_ADDR + rotate 270			*data\track\t_02\t_02
v = 
x = 
y = 
z = 
*/

/*****************************************************
Drawing the ground

The map holds one letter per tile (map_1.h, map_2.h). 
Each tile model is set up once, as a drawable shared by all its tiles.
Each frame only the tiles around the car are looked at, and each one in view 
is drawn by placing the shared drawable there (position and quarter turns) and sorting it.
*****************************************************/

// The tile models
enum {
	TILE_LINE, TILE_STRAIGHT, TILE_TURN, TILE_BLANK, TILE_GRID, TILE_STRAIGHT_01,
	TILE_GRASS, TILE_TURN_00, TILE_TURN_01, TILE_TURN_02, TILE_SAND, TILE_MODELS
};

static const unsigned long tileModelAddr[TILE_MODELS] = {
	LINE_L_MEM_ADDR, STRAIGHT_L_1_MEM_ADDR, TURN_L_1_MEM_ADDR, TURN_R_1_MEM_ADDR, GRID_MEM_ADDR,
	STRAIGHT_L_01_MEM_ADDR, GRASS_MEM_ADDR, TURN_00_MEM_ADDR, TURN_01_MEM_ADDR, TURN_02_MEM_ADDR,
	SAND_MEM_ADDR
};

// What each map letter is (a tile model and its quarter turns)
#define TILE(model, quarters) ((model) | ((quarters) << 4))
#define NO_TILE 0xFF

static const struct { char letter; unsigned char kind; } tileLetters[] = {
	{ '1', TILE(TILE_STRAIGHT, 0) },    { '2', TILE(TILE_STRAIGHT, 2) },
	{ '6', TILE(TILE_STRAIGHT, 1) },    { '7', TILE(TILE_STRAIGHT, 3) },
	{ '3', TILE(TILE_GRASS, 0) },
	{ '4', TILE(TILE_LINE, 0) },        { '5', TILE(TILE_LINE, 2) },
	{ '8', TILE(TILE_TURN, 0) },        { 'a', TILE(TILE_TURN, 1) },
	{ 'c', TILE(TILE_TURN, 2) },        { 'd', TILE(TILE_TURN, 3) },
	{ '9', TILE(TILE_BLANK, 0) },
	{ 'g', TILE(TILE_GRID, 2) },        { 'h', TILE(TILE_GRID, 0) },
	{ 'i', TILE(TILE_STRAIGHT_01, 0) }, { 'j', TILE(TILE_STRAIGHT_01, 1) },
	{ 'k', TILE(TILE_STRAIGHT_01, 2) }, { 'l', TILE(TILE_STRAIGHT_01, 3) },
	{ 'b', TILE(TILE_TURN_00, 0) },     { 'e', TILE(TILE_TURN_00, 1) },
	{ 'f', TILE(TILE_TURN_00, 2) },     { 'm', TILE(TILE_TURN_00, 3) },
	{ 'n', TILE(TILE_TURN_01, 0) },     { 'p', TILE(TILE_TURN_01, 1) },
	{ 'q', TILE(TILE_TURN_01, 2) },     { 'r', TILE(TILE_TURN_01, 3) },
	{ 'o', TILE(TILE_TURN_02, 0) },     { 's', TILE(TILE_TURN_02, 1) },
	{ 't', TILE(TILE_TURN_02, 2) },     { 'u', TILE(TILE_TURN_02, 3) },
	{ 'v', TILE(TILE_SAND, 0) },
};

static unsigned char tileKind[128];         
static GsDOBJ2 tileModels[TILE_MODELS];     // One drawable per tile model
static GsCOORDINATE2 tileCoord;             // Where the tile being drawn is
static MATRIX tileRotation[4];              // 0, 90, 180, 270 degrees rotation (to reuse the same model for all quarter turns)


// Point the track data (tiles, waypoints, arena, objects and start positions) at the selected track
// (done first when a race starts, before anything is placed on the track)
void SelectTrack() {
	int track = (selectedTrackIndex >= 0 && selectedTrackIndex < NUM_TRACKS) ? selectedTrackIndex : 0;

	activeMap = tracks[track].tiles;
	FindStartLine(tracks[track].waypoints, tracks[track].numWaypoints);
	trackArenaX = tracks[track].arenaX;
	trackArenaZ = tracks[track].arenaZ;
	trackArenaSize = tracks[track].arenaSize;
	trackBarriers = tracks[track].barriers;
	numTrackBarriers = tracks[track].numBarriers;
	trackBuildings = tracks[track].buildings;
	numTrackBuildings = tracks[track].numBuildings;
	trackStarts = tracks[track].starts;
}


void InitialiseGround() {
	int i;

	SelectTrack();

	// Map letters
	for (i = 0; i < 128; i++)
		tileKind[i] = NO_TILE;
	for (i = 0; i < (int)(sizeof(tileLetters) / sizeof(tileLetters[0])); i++)
		tileKind[(int)tileLetters[i].letter] = tileLetters[i].kind;

	// Map each TMD (past its ID) and link it to its drawable
	GsInitCoordinate2(WORLD, &tileCoord);
	for (i = 0; i < TILE_MODELS; i++) {
		unsigned long *tmd = (unsigned long *)tileModelAddr[i] + 1;
		GsMapModelingData(tmd);
		GsLinkObject4((unsigned long)(tmd + 2), &tileModels[i], 0);
		tileModels[i].coord2 = &tileCoord;
		tileModels[i].attribute = 0;
	}

	// Exact quarter turns
	for (i = 0; i < 4; i++) {
		SVECTOR turn;
		turn.vx = 0;
		turn.vy = i * 1024;
		turn.vz = 0;
		RotMatrix(&turn, &tileRotation[i]);
		tileRotation[i].t[0] = tileRotation[i].t[1] = tileRotation[i].t[2] = 0;
	}
}


// Polygon subdivision of the ground tiles by distance from the camera
static const DivisionStep groundDivision[] = {
	{ 1800, GsDIV2 }, { 3600, GsDIV1 }, { 0, 0 }
};


void DrawGround(PlayerStruct *currentPlayer, GsOT *ot) {
	MATRIX tmpls, tmplw;
	MATRIX *m = &currentPlayer->gsObjectCoord.coord;
	long px = currentPlayer->gsObjectCoord.coord.t[0];
	long pz = currentPlayer->gsObjectCoord.coord.t[2];
	long reachAhead = VIEW_AHEAD > VIEW_BEHIND ? VIEW_AHEAD : VIEW_BEHIND;
	long reachSide = VIEW_SIDE + (((VIEW_AHEAD + VIEW_CAMERA_BACK) * VIEW_SPREAD) >> 10);
	long ex, ez;
	int row0, row1, col0, col1, row, col;

	// Only select the tiles that are in view (based on the culling around the players position/camera)
	ex = (reachAhead * abs(m->m[0][2]) + reachSide * abs(m->m[0][0])) / 4096 + SEPERATION;
	ez = (reachAhead * abs(m->m[2][2]) + reachSide * abs(m->m[2][0])) / 4096 + SEPERATION;
	row0 = (px - ex) / SEPERATION;
	row1 = (px + ex) / SEPERATION;
	col0 = (pz - ez) / SEPERATION;
	col1 = (pz + ez) / SEPERATION;
	if (row0 < 0) row0 = 0;
	if (col0 < 0) col0 = 0;
	if (row1 >= GROUND_MAX_Z) row1 = GROUND_MAX_Z - 1;
	if (col1 >= GROUND_MAX_X) col1 = GROUND_MAX_X - 1;

	for (row = row0; row <= row1; row++) {
		for (col = col0; col <= col1; col++) {
			unsigned char kind = tileKind[activeMap[row][col] & 127];
			// The middle of the tile (the tile models are centred on their origin
			long x = (long)row * SEPERATION + SEPERATION / 2;
			long z = (long)col * SEPERATION + SEPERATION / 2;
			GsDOBJ2 *model;

			if (kind == NO_TILE || !IsPointInView(currentPlayer, x, z, VIEW_AHEAD, VIEW_BEHIND, VIEW_SIDE))
				continue;
			model = &tileModels[kind & 15];

			// Place the ground tile (using its position and rotation)
			tileCoord.coord = tileRotation[kind >> 4];
			tileCoord.coord.t[0] = x;
			tileCoord.coord.t[1] = 0;
			tileCoord.coord.t[2] = z;
			tileCoord.flg = 0;

			GsGetLws(&tileCoord, &tmplw, &tmpls);
			GsSetLightMatrix(&tmplw);
			GsSetLsMatrix(&tmpls);

			// Subdivide the tiles near the camera
			SetDivisionByDistance(model, x - px, z - pz, groundDivision);

			GsSortObject4(model, ot, 2, (u_long *)getScratchAddr(0));
		}
	}
}
