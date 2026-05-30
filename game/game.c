#include "game.h"
#include "game/player.h"
#include "../engine/graphics.h"
#include "../engine/calculations.h"

long player1_lateralSpeed = 0;
long player2_lateralSpeed = 0;
int  player1_isBraking    = 0;
int  player2_isBraking    = 0;

/* Scale all vertices in a TMD by dividing by 'divisor'.
   Must be called AFTER InitialisePlayer (GsMapModelingData has run,
   so the vertex pointer in the object table is already absolute). */
void ScaleTmdVertices(unsigned long *tmdAddr, int divisor) {
	unsigned long vert_p_abs;
	unsigned long n_vert;
	short *v;
	int i;
	/* Object table starts at word index 3 (after id, flags, nobject).
	   vert_p is at index 5, n_vert at index 6. */
	vert_p_abs = tmdAddr[5];
	n_vert     = tmdAddr[6];
	v = (short *)vert_p_abs;
	for (i = 0; i < (int)n_vert; i++) {
		v[0] = (short)(v[0] / divisor); /* vx */
		v[1] = (short)(v[1] / divisor); /* vy */
		v[2] = (short)(v[2] / divisor); /* vz */
		/* v[3] is padding, leave it */
		v += 4; /* 4 shorts = 8 bytes per vertex */
	}
}

void InitialiseTrackerViewPlayer1(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ) {
	// This is the distance between the eye and the imaginary projection screen
	GsSetProjection(200);

	// Set the eye position or center of projection
	view->vpx = nVPX;
	view->vpy = nVPY;
	view->vpz = nVPZ;

	// Set the look at position
	view->vrx = nVRX;
	view->vry = nVRY;
	view->vrz = nVRZ;

	// Set which way is up
	view->rz =- nRZ;

	// Set the origin of the coord system in this case the car
	view->super = &player1.gsObjectCoord;

	// Activate view
	GsSetRefView2(view);
}

void InitialiseTrackerViewPlayer2(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ) {
	// This is the distance between the eye and the imaginary projection screen
	GsSetProjection(nProjDist);

	// Set the eye position or center of projection
	view->vpx = nVPX;
	view->vpy = nVPY;
	view->vpz = nVPZ;

	// Set the look at position
	view->vrx = nVRX;
	view->vry = nVRY;
	view->vrz = nVRZ;

	// Set which way is up
	view->rz =- nRZ;

	// Set the origin of the coord system in this case the car
	view->super = &player2.gsObjectCoord;

	// Activate view
	GsSetRefView2(view);
}

void InitialiseFrontViewPlayer1(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ) {
	// This is the distance between the eye and the imaginary projection screen
	GsSetProjection(nProjDist);

	// Set the eye position or center of projection
	view->vpx = 0;  // dont touch
	view->vpy = -400;
	view->vpz = -600;

	// Set the look at position
	view->vrx = 0; // dont touch
	view->vry = -500;
	view->vrz = 100;

	// Set which way is up
	view->rz =- nRZ;

	// Set the origin of the coord system in this case the car
	view->super = &player1.gsObjectCoord;

	// Activate view
	GsSetRefView2(view);
}


void InitialiseTopDownView(GsRVIEW2 *view, int nProjDist, int nHeight, int nVPX, int nVPZ) {
    // This is the distance between the eye and the imaginary projection screen
    GsSetProjection(nProjDist);

    // Set the eye position (camera position)
    // Place the camera directly above the car at a specified height
    view->vpx = 0;
	view->vpy = -3000; // height
	view->vpz = -1000;

    // Set the look-at position (car's position)
    iew->vrx = 12000;
	view->vry = 800;
	view->vrz = 0; // angle

    // Set which way is up
    view->rz = -1000;

    // Set the origin of the coordinate system (the car)
    view->super = &player1.gsObjectCoord;

    // Activate view
    GsSetRefView2(view);
}


void InitialiseStaticView(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPX, int nVPY, int nVPZ, int nVRX, int nVRY, int nVRZ) {
	// This is the distance between the eye and the imaginary projection screen
	GsSetProjection(nProjDist);

	// Set the eye position or center of projection
	view->vpx = 0;
	view->vpy = -32000; // height
	view->vpz = -1000;

	// Set the look at position
	view->vrx = 12000;
	view->vry = 800;
	view->vrz = 18000; // angle

	// Set which way is up
	view->rz =- nRZ;

	// Set the origin of the coord system in this case the WORLD
	view->super = WORLD;

	// Activate view
	GsSetRefView2(view);
}

void RotateModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, int nRX, int nRY, int nRZ, long *speed, long *lateralSpeed) {
    MATRIX matTmp;
    long absSpeed;
    long absLateral;
    long gripLimit;
    long maxLateral;
    int steeringResponse;
    int maxRotation;
    int originalNRY;

    maxRotation = ONE / 8;
    absSpeed = abs(*speed);
    absLateral = abs(*lateralSpeed);
    originalNRY = nRY;

    if (absSpeed > 0) {
        // Speed-sensitive steering: full rotation authority at low speed, reduced at high speed
        steeringResponse = STEERING_RESPONSE - (absSpeed * (STEERING_RESPONSE - MIN_STEERING_RESPONSE) / MAX_SPEED);
        if (steeringResponse < MIN_STEERING_RESPONSE) steeringResponse = MIN_STEERING_RESPONSE;

        // Grip limit decreases with speed: same turn input breaks loose sooner at high speed
        gripLimit = MAX_GRIP - (absSpeed * (MAX_GRIP - MIN_GRIP) / MAX_SPEED);
        if (gripLimit < MIN_GRIP) gripLimit = MIN_GRIP;

        // Momentum transfer: rotating the heading while moving converts forward velocity
        // into lateral velocity — the physical reason why fast corners cause slides.
        if (*speed > 0) {
            maxLateral = MAX_SPEED / 3;
            if (absLateral > gripLimit) {
                // Traction limit exceeded: oversteer
                *lateralSpeed -= (*speed * originalNRY * 2) / TURN_RADIUS_FACTOR;
                nRY = nRY / 3;
            } else {
                // Gripped: normal momentum transfer from cornering
                *lateralSpeed -= (*speed * originalNRY) / TURN_RADIUS_FACTOR;
            }
            if (*lateralSpeed > maxLateral) *lateralSpeed = maxLateral;
            if (*lateralSpeed < -maxLateral) *lateralSpeed = -maxLateral;
        }

        // Scale rotation by steering authority and clamp to maximum rotation per frame
        nRX = (nRX * steeringResponse) / 100;
        nRY = (nRY * steeringResponse) / 100;
        nRZ = (nRZ * steeringResponse) / 100;
        nRX = (nRX > maxRotation) ? maxRotation : (nRX < -maxRotation) ? -maxRotation : nRX;
        nRY = (nRY > maxRotation) ? maxRotation : (nRY < -maxRotation) ? -maxRotation : nRY;
        nRZ = (nRZ > maxRotation) ? maxRotation : (nRZ < -maxRotation) ? -maxRotation : nRZ;

        rotateVector->vx = (rotateVector->vx + nRX) % ONE;
        rotateVector->vy = (rotateVector->vy + nRY) % ONE;
        rotateVector->vz = (rotateVector->vz + nRZ) % ONE;

        ResetMatrix(gsObjectCoord->coord.m);
        RotMatrix(rotateVector, &matTmp);
        MulMatrix0(&gsObjectCoord->coord, &matTmp, &gsObjectCoord->coord);
        gsObjectCoord->flg = 0;
    }
}

// Handle acceleration/deceleration
void AdvanceModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotateVector, long *speed, long *lateralSpeed, int movementDirection, int isBraking) {
    MATRIX matTmp;
    SVECTOR forwardBasis = {0, 0, ONE};
    SVECTOR rightBasis = {ONE, 0, 0};
    SVECTOR forwardDir;
    SVECTOR lateralDir;
    long absLateral;
    long absSpeed;
    long gripLimit;

    // Accelerating forward
    if (movementDirection > 0) {
        if (*speed < 0) {
            *speed += BRAKE_DECELERATION * 2;
        } else {
            *speed += ACCELERATION;
            if (*speed > MAX_SPEED) *speed = MAX_SPEED;
        }
    }
    // Braking or reversing
    else if (movementDirection < 0) {
        if (*speed > 0) {
            *speed -= BRAKE_DECELERATION * 2;
            if (*speed < 0) *speed = 0;
        } else {
            *speed -= ACCELERATION / 2;
            if (*speed < MAX_REVERSE_SPEED) *speed = MAX_REVERSE_SPEED;
        }
    }
    // No throttle
    else {
        if (*speed > 0) {
            *speed -= isBraking ? BRAKE_DECELERATION : DECELERATION;
            if (*speed < 0) *speed = 0;
        }
        else if (*speed < 0) {
            *speed += isBraking ? BRAKE_DECELERATION : DECELERATION;
            if (*speed > 0) *speed = 0;
        }
    }

    // Friction: gripped tires snap back quickly; sliding tires recover slowly
    absLateral = abs(*lateralSpeed);
    absSpeed = abs(*speed);
    gripLimit = MAX_GRIP - (absSpeed * (MAX_GRIP - MIN_GRIP) / MAX_SPEED);
    if (gripLimit < MIN_GRIP) gripLimit = MIN_GRIP;

    if (absLateral > gripLimit) {
        *lateralSpeed = (*lateralSpeed * SLIDING_FRICTION) / 1000;
    } else {
        *lateralSpeed = (*lateralSpeed * LATERAL_FRICTION) / 1000;
    }
    if (*lateralSpeed > -2 && *lateralSpeed < 2) *lateralSpeed = 0;

    // Move along both forward and lateral axes
    if (*speed != 0 || *lateralSpeed != 0) {
        RotMatrix(rotateVector, &matTmp);
        ApplyMatrixSV(&matTmp, &forwardBasis, &forwardDir);
        ApplyMatrixSV(&matTmp, &rightBasis, &lateralDir);

        gsObjectCoord->coord.t[0] += (forwardDir.vx * (*speed) + lateralDir.vx * (*lateralSpeed)) / ONE;
        gsObjectCoord->coord.t[1] += (forwardDir.vy * (*speed) + lateralDir.vy * (*lateralSpeed)) / ONE;
        gsObjectCoord->coord.t[2] += (forwardDir.vz * (*speed) + lateralDir.vz * (*lateralSpeed)) / ONE;

        gsObjectCoord->flg = 0;
    }
}

// Initialise single-screen mode
void InitSingleScreen() {

  SplitScreenInfo[0].clip.x = 0;
  SplitScreenInfo[0].clip.y = 0;
  SplitScreenInfo[0].clip.w = 320;
  SplitScreenInfo[0].clip.h = 240;
  SplitScreenInfo[0].ofs[0] = 0 + (320 / 2);
  SplitScreenInfo[0].ofs[1] = 0 + (240 / 2);

  SplitScreenInfo[1].clip.x = 0;
  SplitScreenInfo[1].clip.y = 240;
  SplitScreenInfo[1].clip.w = 320;
  SplitScreenInfo[1].clip.h = 240;
  SplitScreenInfo[1].ofs[0] = 0 + (320 / 2);
  SplitScreenInfo[1].ofs[1] = 0 + (240 / 2) + 240;
};

// Initialise the split-screen co-ordinates
void InitSplitScreen(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {

  SplitScreenInfo[0].clip.x = x1;
  SplitScreenInfo[0].clip.y = y1;
  SplitScreenInfo[0].clip.w = w1;
  SplitScreenInfo[0].clip.h = h1;
  SplitScreenInfo[0].ofs[0] = x1 + (w1 / 2);
  SplitScreenInfo[0].ofs[1] = y1 + (h1 / 2);

  SplitScreenInfo[1].clip.x = x1;
  SplitScreenInfo[1].clip.y = y1 + 240;
  SplitScreenInfo[1].clip.w = w1;
  SplitScreenInfo[1].clip.h = h1;
  SplitScreenInfo[1].ofs[0] = x1 + (w1 / 2);
  SplitScreenInfo[1].ofs[1] = y1 + (h1 / 2) + 240;

  SplitScreenInfo[2].clip.x = x2;
  SplitScreenInfo[2].clip.y = y2;
  SplitScreenInfo[2].clip.w = w2;
  SplitScreenInfo[2].clip.h = h2;
  SplitScreenInfo[2].ofs[0] = x2 + (w2 / 2);
  SplitScreenInfo[2].ofs[1] = y2 + (h2 / 2);

  SplitScreenInfo[3].clip.x = x2;
  SplitScreenInfo[3].clip.y = y2 + 240;
  SplitScreenInfo[3].clip.w = w2;
  SplitScreenInfo[3].clip.h = h2;
  SplitScreenInfo[3].ofs[0] = x2 + (w2 / 2);
  SplitScreenInfo[3].ofs[1] = y2 + (h2 / 2) + 240;
};

/* Sort a red 8x8 TILE into the OT using direct struct init + manual OT insertion.
   Avoids AddPrim/setTile from libgpu (not linked in this build). */
static void sortBrakeLightTile(GsOT *ot, int otz, TILE *t, int sx, int sy) {
	u_long *entry;
	if (otz <= 0 || otz >= (1 << OT_LENGTH)) return;
	entry  = (u_long *)&ot->org[otz];
	t->r0  = 255; t->g0 = 0; t->b0 = 0; t->code = 0x60; /* P_TILE */
	t->x0  = (short)(sx - 4);
	t->y0  = (short)(sy - 4);
	t->w   = 8;
	t->h   = 8;
	t->tag = (3UL << 24) | (*entry & 0x00FFFFFFUL); /* 3 data words, chain to prev */
	*entry = (u_long)t & 0x00FFFFFFUL;              /* OT entry -> this tile */
}

void DrawBrakeLights(PlayerStruct *player, GsOT *ot, int otIdx) {
	static TILE brakeLightPrims[4][4]; /* [otIdx][lightIdx] - double-buffered */
	MATRIX tmplw, tmpls;
	SVECTOR lv, cv;
	long camX, camY, camZ;
	int screenX, screenY, otz;
	int baseIdx;

	baseIdx = (player->playerNumber - 1) * 2;

	GsGetLws(player->gsObjectHandler.coord2, &tmplw, &tmpls);

	/* Left brake light  -- car local pos (-60, 0, -150) */
	lv.vx = -60; lv.vy = 0; lv.vz = -150; lv.pad = 0;
	ApplyMatrixSV(&tmpls, &lv, &cv);
	camX = (long)cv.vx + tmpls.t[0];
	camY = (long)cv.vy + tmpls.t[1];
	camZ = (long)cv.vz + tmpls.t[2];
	if (camZ > 100) {
		screenX = (int)((camX * 200L) / camZ); /* perspective: proj dist 200 */
		screenY = (int)((camY * 200L) / camZ);
		otz = (int)(camZ / 8); /* depth: camZ >> 3, matches GsSortObject4 shift=3 */
		if (otz < 1) otz = 1;
		if (otz >= (1 << OT_LENGTH)) otz = (1 << OT_LENGTH) - 1;
		sortBrakeLightTile(ot, otz, &brakeLightPrims[otIdx][baseIdx], screenX, screenY);
	}

	/* Right brake light -- car local pos (60, 0, -150) */
	lv.vx = 60; /* vy and vz unchanged from above */
	ApplyMatrixSV(&tmpls, &lv, &cv);
	camX = (long)cv.vx + tmpls.t[0];
	camY = (long)cv.vy + tmpls.t[1];
	camZ = (long)cv.vz + tmpls.t[2];
	if (camZ > 100) {
		screenX = (int)((camX * 200L) / camZ);
		screenY = (int)((camY * 200L) / camZ);
		otz = (int)(camZ / 8); /* depth: camZ >> 3, matches GsSortObject4 shift=3 */
		if (otz < 1) otz = 1;
		if (otz >= (1 << OT_LENGTH)) otz = (1 << OT_LENGTH) - 1;
		sortBrakeLightTile(ot, otz, &brakeLightPrims[otIdx][baseIdx + 1], screenX, screenY);
	}
}