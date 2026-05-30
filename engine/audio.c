#include "audio.h"

// Audio handles
short vab;
short seq;

// RAM pointers
unsigned char *vh = VH_ADDR;
unsigned char *vb = VB_ADDR;
unsigned char *s  = SEQ_ADDR;

// Internal state
static int musicStarted = 0;
static int musicPaused  = 0;
static int musicMuted   = 0;
static int currentVolL  = VOL_DEFAULT;
static int currentVolR  = VOL_DEFAULT;
static int currentTempo = TEMPO_DEFAULT;
static int sfxVol       = SFX_VOL_DEFAULT;


/*****************************************************
Playback Functions
*****************************************************/

void PlayMusic(void)
{
    if (musicStarted)
        return;

    if (vh[0] == 0x00 && vh[1] == 0x00)
		return;

    SsSetMVol(currentVolL, currentVolR);

    vab = SsVabTransfer(vh, vb, -1, 1);
    seq = SsSeqOpen((u_long*)s, vab);
    SsSeqPlay(seq, SSPLAY_PLAY, SSPLAY_INFINITY);

    musicStarted = 1;
    musicPaused  = 0;
}


void PauseMusic(void)
{
    if (musicStarted && !musicPaused)
    {
        SsSeqPause(seq);
        musicPaused = 1;
    }
}


void ResumeMusic(void)
{
    if (musicStarted && musicPaused)
    {
        SsSeqReplay(seq);
        musicPaused = 0;
    }
}


void RestartMusic(void)
{
    if (!musicStarted && !musicPaused)
    {
        PlayMusic();
        return;
    }

    SsSeqStop(seq);
    SsSeqClose(seq);
    seq = SsSeqOpen((u_long*)s, vab);
    SsSeqPlay(seq, SSPLAY_PLAY, SSPLAY_INFINITY);
    musicPaused = 0;
}


void StopMusic(void)
{
    SsSeqStop(seq);
    SsSeqClose(seq);
    SsVabClose(vab);

    musicStarted = 0;
    musicPaused  = 0;
}


/*****************************************************
Main Volume Functions
*****************************************************/
void SetVolume(int left, int right)
{
    if (left  < VOL_MIN) left  = VOL_MIN;
    if (left  > VOL_MAX) left  = VOL_MAX;
    if (right < VOL_MIN) right = VOL_MIN;
    if (right > VOL_MAX) right = VOL_MAX;

    currentVolL = left;
    currentVolR = right;

    if (!musicMuted)
        SsSetMVol((short)currentVolL, (short)currentVolR);
}

void VolumeUp(void)
{
    SetVolume(currentVolL + VOL_STEP, currentVolR + VOL_STEP);
}

void VolumeDown(void)
{
    SetVolume(currentVolL - VOL_STEP, currentVolR - VOL_STEP);
}

void MuteMusic(void)
{
    if (musicMuted)
        return;

    SsSetMVol(0, 0);
    musicMuted = 1;
}

void UnmuteMusic(void)
{
    if (!musicMuted)
        return;

    SsSetMVol((short)currentVolL, (short)currentVolR);
    musicMuted = 0;
}

int GetVolume(void)
{
    return currentVolL;
}


/*****************************************************
SFX Volume Functions
*****************************************************/
void SetSFXVolume(int vol)
{
    if (vol < SFX_VOL_MIN) vol = SFX_VOL_MIN;
    if (vol > SFX_VOL_MAX) vol = SFX_VOL_MAX;
    sfxVol = vol;
}


void SFXVolumeUp(void)
{
    SetSFXVolume(sfxVol + SFX_VOL_STEP);
}


void SFXVolumeDown(void)
{
    SetSFXVolume(sfxVol - SFX_VOL_STEP);
}


int GetSFXVolume(void)
{
    return sfxVol;
}


/*****************************************************
Tempo Functions
*****************************************************/
void SetTempo(int t)
{
    if (t < TEMPO_MIN) t = TEMPO_MIN;
    if (t > TEMPO_MAX) t = TEMPO_MAX;

    currentTempo = t;

    if (musicStarted)
        SsSetTempo(seq, 0, (short)currentTempo);
}


void TempoUp(void)
{
    SetTempo(currentTempo + TEMPO_STEP);
}


void TempoDown(void)
{
    SetTempo(currentTempo - TEMPO_STEP);
}


void ResetTempo(void)
{
    SetTempo(TEMPO_DEFAULT);
}


int GetTempo(void)
{
    return currentTempo;
}


/*****************************************************
Reverb Functions
*****************************************************/
void SetReverb(short type, int depth)
{
    SsUtSetReverbType(type);
    SsUtSetReverbDepth((short)depth, (short)depth);
}


void ReverbOn(void)
{
    SsUtReverbOn();
}


void ReverbOff(void)
{
    SsUtReverbOff();
}


/*****************************************************
Status Functions
*****************************************************/
int MusicIsPlaying(void)
{
    return musicStarted && !musicPaused;
}


int MusicIsPaused(void)
{
    return musicPaused;
}


int MusicIsMuted(void)
{
    return musicMuted;
}

/*===========================================================================
 * SFX
 * Triggers a one-shot note from the loaded VAB.
 *  prog — instrument program number
 *  note — MIDI note (60 = middle C)
 *  vol  — velocity 0-127
 *  pan  — 0=left  64=centre  127=right
 *===========================================================================*/

void PlaySFX(int prog, int note, int vol, int pan)
{
    int scaledVol;

    if (!musicStarted)
        return;

    // Scale requested vol by sfxVol
    scaledVol = (vol * sfxVol) / SFX_VOL_MAX;

    SsUtKeyOn(vab, prog, 0, note, 0, scaledVol, pan);
}







// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NEW FUNCTIONS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Call this once after loading VAB
void Audio_InitTracks(void)
{
    int i, c;

    for (i = 0; i < MAX_TRACKS; i++)
    {
        tracks[i].seq = -1;
        tracks[i].active = 0;

        tracks[i].volL = 0;
        tracks[i].volR = 0;

        tracks[i].targetVolL = 0;
        tracks[i].targetVolR = 0;

        tracks[i].fadeSpeed = 0;

        for (c = 0; c < MAX_CHANNELS; c++)
		{
			tracks[i].channelVol[c]       = 127;
			tracks[i].channelTargetVol[c] = 127;
			tracks[i].channelFadeSpeed[c] = 0;
		}
    }
}


// PLAY TRACK (layered)
int PlayTrack(int id, unsigned char *seqData, int vol)
{
	int c;
	
    if (id < 0 || id >= MAX_TRACKS)
        return -1;

    tracks[id].seq = SsSeqOpen((u_long*)seqData, vab);

    SsSeqPlay(tracks[id].seq, SSPLAY_PLAY, SSPLAY_INFINITY);

    tracks[id].volL = vol;
    tracks[id].volR = vol;

    tracks[id].targetVolL = vol;
    tracks[id].targetVolR = vol;

	// Set the vloume for each track
    for (c = 0; c < MAX_CHANNELS; c++)
	{
		//SsSeqSetCVol(tracks[id].seq, c, 127, 127);
	}
	
    tracks[id].active = 1;

    return tracks[id].seq;
}


// STOP TRACK
void StopTrack(int id)
{
    if (id < 0 || id >= MAX_TRACKS)
        return;

    if (!tracks[id].active)
        return;

    SsSeqStop(tracks[id].seq);
    SsSeqClose(tracks[id].seq);

    tracks[id].active = 0;
}



// SET TRACK VOLUME (instant)
void SetTrackVolume(int id, int vol)
{
    if (!tracks[id].active)
        return;

    tracks[id].volL = vol;
    tracks[id].volR = vol;

    tracks[id].targetVolL = vol;
    tracks[id].targetVolR = vol;

    SsSeqSetVol(tracks[id].seq, vol, vol);
}



// FADE TRACK
void FadeTrack(int id, int target, int speed)
{
    if (!tracks[id].active)
        return;

    tracks[id].targetVolL = target;
    tracks[id].targetVolR = target;
    tracks[id].fadeSpeed  = speed;
}





// UPDATE (CALL EVERY FRAME)
void Audio_Update(void)
{
    int i, c;

    for (i = 0; i < MAX_TRACKS; i++)
    {
        if (!tracks[i].active)
            continue;

        /* =========================
           TRACK VOLUME FADE
        ========================= */

        if (tracks[i].volL < tracks[i].targetVolL)
        {
            tracks[i].volL += tracks[i].fadeSpeed;
            if (tracks[i].volL > tracks[i].targetVolL)
                tracks[i].volL = tracks[i].targetVolL;
        }
        else if (tracks[i].volL > tracks[i].targetVolL)
        {
            tracks[i].volL -= tracks[i].fadeSpeed;
            if (tracks[i].volL < tracks[i].targetVolL)
                tracks[i].volL = tracks[i].targetVolL;
        }

        if (tracks[i].volR < tracks[i].targetVolR)
        {
            tracks[i].volR += tracks[i].fadeSpeed;
            if (tracks[i].volR > tracks[i].targetVolR)
                tracks[i].volR = tracks[i].targetVolR;
        }
        else if (tracks[i].volR > tracks[i].targetVolR)
        {
            tracks[i].volR -= tracks[i].fadeSpeed;
            if (tracks[i].volR < tracks[i].targetVolR)
                tracks[i].volR = tracks[i].targetVolR;
        }

        SsSeqSetVol(tracks[i].seq, tracks[i].volL, tracks[i].volR);

        /* =========================
           CHANNEL VOLUME FADE
        ========================= */

        for (c = 0; c < MAX_CHANNELS; c++)
        {
            int cur = tracks[i].channelVol[c];
            int tgt = tracks[i].channelTargetVol[c];
            int spd = tracks[i].channelFadeSpeed[c];

            if (cur == tgt || spd == 0)
                continue;

            if (cur < tgt)
            {
                cur += spd;
                if (cur > tgt)
                    cur = tgt;
            }
            else
            {
                cur -= spd;
                if (cur < tgt)
                    cur = tgt;
            }

            tracks[i].channelVol[c] = cur;

            //SsSeqSetCVol(tracks[i].seq, c, cur, cur);
        }
    }
}




// CROSSFADE (PS1-style music switching)
void CrossfadeTracks(int from, int to, int speed)
{
    FadeTrack(from, 0, speed);
    FadeTrack(to,   VOL_MAX, speed);
}






// MUTE A CHANNEL
void MuteChannel(int trackID, int channel)
{
    if (!tracks[trackID].active)
        return;

    if (channel < 0 || channel >= MAX_CHANNELS)
        return;

    tracks[trackID].channelVol[channel] = 0;

    //SsSeqSetCVol(tracks[trackID].seq, channel, 0, 0);
}




// UNMUTE A CHANNEL
void UnmuteChannel(int trackID, int channel)
{
    if (!tracks[trackID].active)
        return;

    if (channel < 0 || channel >= MAX_CHANNELS)
        return;

    tracks[trackID].channelVol[channel] = 127;

    //SsSeqSetCVol(tracks[trackID].seq, channel, 127, 127);
}




// SET CHANNEL VOLUME (fine control)
void SetChannelVolume(int trackID, int channel, int vol)
{
    if (!tracks[trackID].active)
        return;

    if (channel < 0 || channel >= MAX_CHANNELS)
        return;

    if (vol < 0) vol = 0;
    if (vol > 127) vol = 127;

    tracks[trackID].channelVol[channel] = vol;

    //SsSeqSetCVol(tracks[trackID].seq, channel, vol, vol);
}



void FadeChannel(int trackID, int channel, int target, int speed)
{
    if (!tracks[trackID].active)
        return;

    if (channel < 0 || channel >= MAX_CHANNELS)
        return;

    if (target < 0)   target = 0;
    if (target > 127) target = 127;

    tracks[trackID].channelTargetVol[channel] = target;
    tracks[trackID].channelFadeSpeed[channel] = speed;
}









/*
EXAMPLE USAGE

Start layered music:
PlayTrack(0, SEQ_MAIN, 127);     // base music
PlayTrack(1, SEQ_DRUMS, 0);      // hidden layer

Trigger intensity:
FadeTrack(1, 127, 2); // fade in drums

Calm down:
FadeTrack(1, 0, 2);


Smooth drum fade-in
FadeChannel(0, 9, 127, 2);  // drums fade in

Smooth drum fade-out
FadeChannel(0, 9, 0, 2);    // drums fade out





You MUST call: Audio_Update(); every frame (like your FontFX_Update())

You now need multiple SEQ blobs:
unsigned char *SEQ_MAIN;
unsigned char *SEQ_DRUMS;

VAB stays shared. All tracks use the same: vab

*/





































