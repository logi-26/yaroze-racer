#include <libps.h>
#include "link.h"
#include "yario.h"
#include "state_manager.h"
#include "graphics.h"

/*
Handshake: each console sends HELLO with a random ID every frame. When it
receives the other console's HELLO (or ACK) it sends ACK instead, and once
it receives the other console's ACK both have heard each other: connected.
The console with the higher ID is player 1 (equal IDs: both pick new ones).
While connected, ACK keeps being sent; if nothing arrives for LINK_TIMEOUT
frames the connection is lost and the handshake starts again.

YarIO sends 16 bits per packet (and nothing for 0): the top two bits are the
message type, the other 14 the ID.
*/
#define MSG_HELLO       0x4000
#define MSG_ACK         0x8000
#define MSG_TYPE_MASK   0xC000
#define MSG_ID_MASK     0x3FFF

#define LINK_TIMEOUT    (FRAME_RATE * 2)

static LinkStatus linkStatus = LINK_OFF;
static int localId;
static int remoteId;
static int playerNumber;
static int framesSinceHeard;
static u_long randomSeed;


// Small random number generator (libps has no rand)
static int NewId(void)
{
    int id;
    do {
        randomSeed = randomSeed * 1103515245 + 12345;
        id = (randomSeed >> 16) & MSG_ID_MASK;
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
    u_long received;
    int type, id;

    if (linkStatus == LINK_OFF)
        return;

    YarioUpdate((linkStatus == LINK_WAITING ? MSG_HELLO : MSG_ACK) | localId);
    received = YarioGetRemoteBuff();

    type = received & MSG_TYPE_MASK;
    id = received & MSG_ID_MASK;

    if (received == 0 || received > 0xFFFF || (type != MSG_HELLO && type != MSG_ACK) || id == 0)
    {
        // Nothing (valid) this frame
        if (linkStatus != LINK_WAITING && ++framesSinceHeard > LINK_TIMEOUT)
            StartHandshake();
        return;
    }
    framesSinceHeard = 0;

    // Both picked the same ID: pick another (the other console does too)
    if (id == localId)
    {
        localId = NewId();
        linkStatus = LINK_WAITING;
        return;
    }

    // Heard from a different console than before (it restarted): start again
    if (remoteId != 0 && id != remoteId)
    {
        StartHandshake();
        return;
    }

    remoteId = id;
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
