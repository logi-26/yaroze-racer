#ifndef LINK_H
#define LINK_H

#include <libps.h>

// Link play: the connection between two consoles over the link cable 
// (YarIO, which uses the tty on the serial port)

typedef enum
{
    LINK_OFF,           // Not open
    LINK_WAITING,       // Open, nothing heard from the other console yet
    LINK_HANDSHAKE,     // Heard the other console, waiting for it to hear us
    LINK_CONNECTED      // Both consoles have heard each other
} LinkStatus;


// Status word the menus exchange while connected (Link_SetLocalData):
// stage in bits 0-1, value (vehicle/track) in bits 2-5, flag (ready/chosen) in bit 6
#define LINK_STAGE_VEHICLE  0   // vehicle select: value = vehicle, flag = ready
#define LINK_STAGE_TRACK    1   // track select: value = track, flag = chosen
#define LINK_STAGE_RACE     2   // racing: value = track

#define LINK_STATUS(stage, value, flag)  ((stage) | (((value) & 15) << 2) | (((flag) & 1) << 6))
#define LINK_STATUS_STAGE(status)        ((status) & 3)
#define LINK_STATUS_VALUE(status)        (((status) >> 2) & 15)
#define LINK_STATUS_FLAG(status)         (((status) >> 6) & 1)

// In the race the status word holds the stage (bits 0-1), finished (bit 2) and
// the car's heading / 2 (bits 3-13); the extra data (Link_SetLocalExtra) holds
// the car's position / 4 (X bits 0-13, Z bits 14-27) and the track (bits 28-31)
#define LINK_RACE_STATUS(finished, heading)  (LINK_STAGE_RACE | (((finished) & 1) << 2) | ((((heading) & 4095) >> 1) << 3))
#define LINK_RACE_FINISHED(status)           (((status) >> 2) & 1)
#define LINK_RACE_HEADING(status)            ((((status) >> 3) & 2047) << 1)

#define LINK_RACE_MAX_POS                    (0x3FFF << 2)
#define LINK_RACE_EXTRA(x, z, track)         ((((u_long)(x) >> 2) & 0x3FFF) | ((((u_long)(z) >> 2) & 0x3FFF) << 14) | (((u_long)(track) & 15) << 28))
#define LINK_RACE_X(extra)                   ((long)((extra) & 0x3FFF) << 2)
#define LINK_RACE_Z(extra)                   ((long)(((extra) >> 14) & 0x3FFF) << 2)
#define LINK_RACE_TRACK(extra)               ((int)(((extra) >> 28) & 15))


/************* FUNCTION PROTOTYPES *******************/
void Link_Open(void);
void Link_Close(void);
void Link_Update(void);
LinkStatus Link_GetStatus(void);
int Link_GetPlayerNumber(void);
void Link_SetLocalData(int data);
int Link_GetRemoteData(void);
void Link_SetLocalExtra(u_long extra);
u_long Link_GetRemoteExtra(void);
/*****************************************************/

#endif
