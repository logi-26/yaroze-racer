#include "yario.h"
#ifdef YARIO_EMU
#include "yario_emu.h"
#endif

YarioBuff yarioBuff;

// Accessors for the yario buffers
u_long YarioGetLocalBuff() { return yarioBuff.localBufferIO; }
u_long YarioGetRemoteBuff() { return yarioBuff.RemoteBufferIO; }

static int InitSerial(void) {
    int tty = open("tty:", 2); 				// O_RDWR
    if (tty < 0) return -1;

	ioctl(tty, TIOCRAW, 1);                 // Disable XON/XOFF
    ioctl(tty, TIOCBAUD, 115200 );          // Set baud rate
    ioctl(tty, TIOCLEN, (1 << 16) | 3);     // 8N1
    ioctl(tty, TIOCPARITY, 0);              // No parity
    ioctl(tty, FIOCNBLOCK, 1);              // Non-blocking
    ioctl(tty, TIOCFLUSH, 1);               // Flush
    ioctl(tty, TIOERRRST, 1);               // Reset errors

    return tty;
}

// Send 8-byte packet: 0xAA + 6 data bytes + XOR checksum
static void SendData(int tty, const u_char *data) {
    unsigned char packet[8];
    int i;
    packet[0] = 0xAA; 						// Start byte
    packet[7] = 0;
    for (i = 0; i < YARIO_DATA_SIZE; i++) {
        packet[1 + i] = data[i];			// Data bytes
        packet[7] ^= data[i];				// XOR checksum
    }
    write(tty, packet, 8);
}

// Receive 8-byte packet: copies its 6 data bytes to 'data'
static int ReceiveData(int tty, u_char *data) {
    static unsigned char buf[8];
    static int index = 0;
    unsigned char temp[8];
    int bytesRead, i;
    unsigned char check, byte;

    // Try to read up to 8 bytes at once
    bytesRead = read(tty, temp, 8 - index);

    if (bytesRead <= 0) return 0;

    // Process each byte
    for (i = 0; i < bytesRead; i++) {
        byte = temp[i];

        // Check for start byte
        if (index == 0 && byte != 0xAA) continue;

        buf[index++] = byte;

        // Process when the full 8-byte packet is received
        if (index == 8) {

			// Perform XOR
            check = buf[1] ^ buf[2] ^ buf[3] ^ buf[4] ^ buf[5] ^ buf[6];

			// If the XOR has passed, put the data in the buffer
			if (buf[7] == check) {
                memcpy(data, &buf[1], YARIO_DATA_SIZE);
                index = 0;
                return 1;
            }
            index = 0; // Reset on invalid packet
        }
    }
    return 0;
}

void YarioInit(void) {
    memset(&yarioBuff, 0, sizeof(YarioBuff));
#ifdef YARIO_EMU
    YarioEmuInstall();  // Emulator build: tty over the emulated serial port
#endif
    yarioBuff.ttyFD = InitSerial();
    yarioBuff.localBufferIO = 0xFFFFFFFF;
    yarioBuff.RemoteBufferIO = 0xFFFFFFFF;
}

// Send 6 data bytes (none if outData is 0), and receive the latest packet
// from the other console
void YarioUpdateData(const u_char *outData) {
    u_char latest[YARIO_DATA_SIZE];

    yarioBuff.remoteReceived = 0;

    if (yarioBuff.ttyFD >= 0) {
        // Read the data (keep the latest packet)
        while (ReceiveData(yarioBuff.ttyFD, latest)) {
            memcpy(yarioBuff.remoteData, latest, YARIO_DATA_SIZE);
            yarioBuff.remoteReceived = 1;
        }

        // Send the data
        if (outData) {
            SendData(yarioBuff.ttyFD, outData);
            memcpy(yarioBuff.localData, outData, YARIO_DATA_SIZE);
        }
    }
}

// The 6 data bytes last received; returns 1 if they arrived in the last update
int YarioGetRemoteData(u_char *inData) {
    memcpy(inData, yarioBuff.remoteData, YARIO_DATA_SIZE);
    return yarioBuff.remoteReceived;
}

// Send/receive 16 bits (the first two data bytes, nothing is sent for 0)
void YarioUpdate(u_long outBuff) {
	// Default to last state
    u_long latestRemoteBuff = yarioBuff.RemoteBufferIO;
    u_char data[YARIO_DATA_SIZE];

    if (yarioBuff.ttyFD >= 0) {
        memset(data, 0, YARIO_DATA_SIZE);
        data[0] = (outBuff >> 8) & 0xFF;    // Pad high byte (bits 8–15)
        data[1] = outBuff & 0xFF;           // Pad low byte (bits 0–7)

        // Read the data, and send the data
		if (outBuff != 0) {
            YarioUpdateData(data);
            yarioBuff.localBufferIO = outBuff;
        } else {
            YarioUpdateData(0);
        }

		// If we received data, store it in the buffer
        if (yarioBuff.remoteReceived) {
            latestRemoteBuff = ((u_long)yarioBuff.remoteData[0] << 8) | (u_long)yarioBuff.remoteData[1];
            yarioBuff.RemoteBufferIO = latestRemoteBuff;
        } else {
            yarioBuff.RemoteBufferIO = 0;
        }
    }
}

void YarioClose(void) {
    if (yarioBuff.ttyFD >= 0) {
        close(yarioBuff.ttyFD);
    }
}
