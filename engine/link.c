#include <libps.h>
#include "link.h"
#include "yario.h"
#include "state_manager.h"
#include "graphics.h"

/*
Handshake: each console sends HELLO with a random ID every frame. When it
receives the other console's HELLO (or ACK) it sends ACK instead, and once
it receives the other console's ACK (or DATA) both have heard each other:
connected. The console with the higher ID is player 1 (equal IDs: both pick
new ones).

While connected, each console sends DATA every frame: a 14-bit status word
set by the game (Link_SetLocalData), which the other console reads with
Link_GetRemoteData. If nothing arrives for LINK_TIMEOUT frames the connection
is lost and the handshake starts again.

Each YarIO packet carries 6 bytes: a 16-bit message (the top two bits are the
message type, the other 14 the ID for HELLO/ACK or the status word for DATA),
then 32 bits of extra data for DATA (Link_SetLocalExtra: the race state).
*/
#define MSG_HELLO       0x4000
#define MSG_ACK         0x8000
#define MSG_DATA        0xC000
#define MSG_TYPE_MASK   0xC000
#define MSG_VALUE_MASK  0x3FFF

#define LINK_TIMEOUT    (FRAME_RATE * 2)

static LinkStatus linkStatus = LINK_OFF;
static int localId;
static int remoteId;
static int playerNumber;
static int framesSinceHeard;
static int localData;
static int remoteData;
static u_long localExtra;
static u_long remoteExtra;
static u_long randomSeed;


// Small random number generator (libps has no rand)
static int NewId(void)
{
    int id;
    do {
        randomSeed = randomSeed * 1103515245 + 12345;
        id = (randomSeed >> 16) & MSG_VALUE_MASK;
    } while (id == 0);
    return id;
}


static void StartHandshake(void)
{
    linkStatus = LINK_WAITING;
    localId = NewId();
    remoteId = 0;
    playerNumber = 0;
    framesSinceHeard = 0;
    localData = 0;
    remoteData = -1;
    localExtra = 0;
    remoteExtra = 0;
}


// The packet to send this frame
static u_long OutgoingPacket(void)
{
    switch (linkStatus)
    {
        case LINK_WAITING:   return MSG_HELLO | localId;
        case LINK_HANDSHAKE: return MSG_ACK | localId;
        default:             return MSG_DATA | localData;
    }
}


/*****************************************************
Public Interface
*****************************************************/

// Open the link (the tty) and start looking for the other console
void Link_Open(void)
{
    if (linkStatus != LINK_OFF)
        return;

    // Seed from the time since the console started: differs between consoles
    randomSeed = (u_long)VSync(-1) * 31 + frameNumber;

    YarioInit();
    StartHandshake();
}


// Close the link
void Link_Close(void)
{
    if (linkStatus == LINK_OFF)
        return;

    YarioClose();
    linkStatus = LINK_OFF;
}


// Send/receive this frame's packet: call once per frame while the link is open
void Link_Update(void)
{
    u_char packet[YARIO_DATA_SIZE];
    u_long message, received;
    int type, value;

    if (linkStatus == LINK_OFF)
        return;

    message = OutgoingPacket();
    packet[0] = (message >> 8) & 0xFF;
    packet[1] = message & 0xFF;
    packet[2] = (localExtra >> 24) & 0xFF;
    packet[3] = (localExtra >> 16) & 0xFF;
    packet[4] = (localExtra >> 8) & 0xFF;
    packet[5] = localExtra & 0xFF;
    YarioUpdateData(packet);

    // Nothing arrived this frame: 0
    received = 0;
    if (YarioGetRemoteData(packet))
        received = ((u_long)packet[0] << 8) | packet[1];

    type = received & MSG_TYPE_MASK;
    value = received & MSG_VALUE_MASK;

    // DATA: the other console is connected
    if (type == MSG_DATA && linkStatus != LINK_WAITING)
    {
        framesSinceHeard = 0;
        remoteData = value;
        remoteExtra = ((u_long)packet[2] << 24) | ((u_long)packet[3] << 16) | ((u_long)packet[4] << 8) | packet[5];
        linkStatus = LINK_CONNECTED;
        return;
    }

    if (received == 0 || (type != MSG_HELLO && type != MSG_ACK) || value == 0)
    {
        // Nothing (valid) this frame
        if (linkStatus != LINK_WAITING && ++framesSinceHeard > LINK_TIMEOUT)
            StartHandshake();
        return;
    }
    framesSinceHeard = 0;

    // Both picked the same ID: pick another (the other console does too)
    if (value == localId)
    {
        localId = NewId();
        linkStatus = LINK_WAITING;
        return;
    }

    // Heard from a different console than before (it restarted): start again
    if (remoteId != 0 && value != remoteId)
    {
        StartHandshake();
        return;
    }

    remoteId = value;
    playerNumber = (localId > remoteId) ? 1 : 2;

    if (type == MSG_ACK)
        linkStatus = LINK_CONNECTED;        // it has heard us
    else if (linkStatus == LINK_WAITING)
        linkStatus = LINK_HANDSHAKE;        // we have heard it
}


LinkStatus Link_GetStatus(void)
{
    return linkStatus;
}


// 1 or 2 once connected (0 before)
int Link_GetPlayerNumber(void)
{
    return (linkStatus == LINK_CONNECTED) ? playerNumber : 0;
}


// The status word sent to the other console every frame while connected (14 bits)
void Link_SetLocalData(int data)
{
    localData = data & MSG_VALUE_MASK;
}


// The other console's status word (-1 until one has arrived)
int Link_GetRemoteData(void)
{
    return (linkStatus == LINK_CONNECTED) ? remoteData : -1;
}


// 32 bits of extra data sent with the status word while connected
void Link_SetLocalExtra(u_long extra)
{
    localExtra = extra;
}


// The other console's extra data (0 until some has arrived)
u_long Link_GetRemoteExtra(void)
{
    return (linkStatus == LINK_CONNECTED) ? remoteExtra : 0;
}
