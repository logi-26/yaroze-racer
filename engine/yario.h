#ifndef YARIO_H
#define YARIO_H

#include <libps.h>
#include <sys/ioctl.h>

// Data bytes in each packet (between the start byte and the XOR checksum)
#define YARIO_DATA_SIZE 6

typedef struct {
    int ttyFD;					            // TTY file descriptor
    u_long localBufferIO;		            // IO buffer from the local console
    u_long RemoteBufferIO;  	            // IO buffer from the remote console
    u_char localData[YARIO_DATA_SIZE];		// Data bytes last sent
    u_char remoteData[YARIO_DATA_SIZE];		// Data bytes last received
    int remoteReceived;		                // 1 if a packet arrived in the last update
} YarioBuff;

/************* FUNCTION PROTOTYPES *******************/
void YarioInit(void);
void YarioUpdate(u_long outBuff);
void YarioUpdateData(const u_char *outData);
int YarioGetRemoteData(u_char *inData);
void YarioClose(void);
u_long YarioGetLocalBuff(void);
u_long YarioGetRemoteBuff(void);
/*****************************************************/

#endif // YARIO_H