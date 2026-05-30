/*
A SEQ track = multiple MIDI channels (0–15)
Instruments (like drums) are assigned to channels inside the sequence

*/

#ifndef AUDIO_H
#define AUDIO_H

#include <libps.h>

// RAM addresses for the audio files
#define VH_ADDR  ((unsigned char*)0x800C8000)
#define VB_ADDR  ((unsigned char*)0x800D7000)
#define SEQ_ADDR ((unsigned char*)0x800F9000)

// Volume constants
#define VOL_MIN     0
#define VOL_MAX     127
#define VOL_STEP    8
#define VOL_DEFAULT 100

// SFX volume constants
#define SFX_VOL_MIN     0
#define SFX_VOL_MAX     127
#define SFX_VOL_STEP    8
#define SFX_VOL_DEFAULT 100

// Tempo constants
#define TEMPO_MIN       30
#define TEMPO_MAX       240
#define TEMPO_STEP      10
#define TEMPO_DEFAULT   120

extern short vab;
extern short seq;

// Number of supported tracks
#define MAX_TRACKS 4

// Max MIDI channels
#define MAX_CHANNELS 16

typedef struct
{
    short seq;
    int active;

    int volL;
    int volR;

    int targetVolL;
    int targetVolR;

    int fadeSpeed;
	
	int channelVol[MAX_CHANNELS];
	int channelTargetVol[MAX_CHANNELS];
    int channelFadeSpeed[MAX_CHANNELS];
	
} AudioTrack;

static AudioTrack tracks[MAX_TRACKS];


/************* FUNCTION PROTOTYPES *******************/

// Playback functions
void PlayMusic(void);
void PauseMusic(void);
void ResumeMusic(void);
void RestartMusic(void);
void StopMusic(void);

// Main volume functions
void SetVolume(int left, int right);
void VolumeUp(void);
void VolumeDown(void);
void MuteMusic(void);
void UnmuteMusic(void);
int  GetVolume(void);

// SFX volume functions
void SetSFXVolume(int vol);
void SFXVolumeUp(void);
void SFXVolumeDown(void);
int  GetSFXVolume(void);

// Tempo functions
void SetTempo(int t);
void TempoUp(void);
void TempoDown(void);
void ResetTempo(void);
int  GetTempo(void);

// Reverb functions
void SetReverb(short type, int depth);
void ReverbOn(void);
void ReverbOff(void);

// Status functions
int MusicIsPlaying(void);
int MusicIsPaused(void);
int MusicIsMuted(void);

// SFX

// prog — the instrument number from the VAB file
// note — MIDI note number. 60 is middle C. Each step up or down is one semitone
// vol — volume from 0 to 127
// pan — stereo position. 0 is full left, 64 is centre, 127 is full right

void PlaySFX(int prog, int note, int vol, int pan);



// NEW FUNCTION PROTOTYPES (UNTESTED)
void Audio_Update(void);
void Audio_InitTracks(void);
int PlayTrack(int id, unsigned char *seqData, int vol);
void StopTrack(int id);
void SetTrackVolume(int id, int vol);
void FadeTrack(int id, int target, int speed);
void CrossfadeTracks(int from, int to, int speed);

void MuteChannel(int trackID, int channel);
void UnmuteChannel(int trackID, int channel);
void SetChannelVolume(int trackID, int channel, int vol);
void FadeChannel(int trackID, int channel, int target, int speed);

/*****************************************************/

#endif


/*
per-sequence volume
multi-sequence support
dynamic control
*/