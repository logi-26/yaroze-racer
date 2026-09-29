#ifndef LINK_H
#define LINK_H

// Link play: the connection between two consoles over the link cable 
// (YarIO, which uses the tty on the serial port)

typedef enum
{
    LINK_OFF,           // Not open
    LINK_WAITING,       // Open, nothing heard from the other console yet
    LINK_HANDSHAKE,     // Heard the other console, waiting for it to hear us
    LINK_CONNECTED      // Both consoles have heard each other
} LinkStatus;


/************* FUNCTION PROTOTYPES *******************/
void Link_Open(void);
void Link_Close(void);
void Link_Update(void);
LinkStatus Link_GetStatus(void);
int Link_GetPlayerNumber(void);
/*****************************************************/

#endif
