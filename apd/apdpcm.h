/*!
** \file pcm.h
** Author: 
**
** Description:
**
** Copyright (C) 2006, Inc.
*/
/*
 * $Log: apdpcm.h,v $
 * Revision 1.13  2006/05/23 19:35:17  pbrooks
 * Added support for GetTime, State, GetBitrate, GetAudioParams, GetState,
 *        GetCurrent, SetCurrent, ClearCurrent.
 *
 * Revision 1.12  2006/05/10 22:54:53  pbrooks
 * Cleaned up volume and buffering method.
 *
 * Revision 1.11  2006/05/09 21:38:37  pbrooks
 * Helix decoder milestone.
 *
 * Revision 1.10  2006/05/05 21:10:45  pbrooks
 * Fixed Stop bugs.
 *
 * Revision 1.9  2006/05/05 16:52:14  pbrooks
 * Added define for sample rate.
 *
 * Revision 1.8  2006/05/05 16:35:46  pbrooks
 * Even bigger buffers and using 22050 sample rate on output.
 *
 * Revision 1.7  2006/05/05 01:20:26  pbrooks
 * Optimization changes.
 *
 * Revision 1.6  2006/05/03 21:11:26  pbrooks
 * Integration with libmad now plays music correctly on the desktop. Arm version
 * choppy, needs optimization.
 *
 * Revision 1.5  2006/05/03 16:24:18  pbrooks
 * Now playing scratchy noise.
 *
 * Revision 1.4  2006/05/03 01:28:51  pbrooks
 * First cut at libmad decoder.
 *
 * Revision 1.3  2006/04/29 00:42:11  pbrooks
 * Audio play milestone.
 *
 * Revision 1.2  2006/04/26 23:52:17  pbrooks
 * Changes for threaded decode and play.
 *
 * Revision 1.1.1.1  2006/04/21 16:30:29  pbrooks
 * Initial checkin of embedded player files.
 *
 *
 */
#ifndef PCM_H
#define PCM_H

#include <pthread.h>

#define clone_flags (CLONE_VM|CLONE_FS|CLONE_FILES)

#define BUF_SIZE 1152

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 1
#endif

#ifndef FAILURE
#define FAILURE 0
#endif

#define DECODE_STOP         0
#define DECODE_FINISHED     1
#define DECODE_DECODEERROR  2

#define PCM_QUICKFADELEN    1
#define PCM_FADELEN         4
#define PCM_FADESTEPS       10 

//#define PCM_SAMPLERATE      22050
#define PCM_SAMPLERATE      44100

#define PCM_LONGSLEEP		100000	/* tenth of a second */
#define PCM_SHORTSLEEP		10000	/* hundredth of a second */
#define PCM_DEFAULTBUFFERSIZE 	8 	/* default size of buffer (in frames) */
#define PCM_MAXBUFFERSIZE 	44100	/* this is per channel number of FRAMES */
//#define PCM_FRAMESIZE		(PCM_SAMPLERATE/11025)	/*  one PCM frame  */
#define PCM_FRAMESIZE		2	/*  one PCM frame  */
#define MADPCM_SAMPLES 		(1152)	/* samples per frame  */

#define PCM_NUMPCMBUF		200	/* Just under a second of audio for stereo stream
					   & just under 2 seconds for mono stream */

#define PCM_NONE	-1
#define PCM_LEFT 	0
#define PCM_RIGHT 	1
#define PCM_BOTH 	2

typedef struct _StereoVolume{
	unsigned char left;
	unsigned char right;
} StereoVolume;

typedef struct _ACard {
	int handle;                     /* ret on open of dsp dev */
	int available;                  /* whether card was available on init */
	int active_channel[2];          /* active channels flag array */
	int buffer_pending[2];          /* num of buffers pending  */
	unsigned char *xferbuf[2];      /* alloc on successful opening of card */
	unsigned char *pcmbuf;          /* buffer used when writing PCM data out */
    int pcmBufferSize;              /* buffer size */
	int xferbuf_pos[2];	            /*pos in xferbuf of next buffer */
//	DFX_PORT_HANDLE *dfx_handle[2]; /* 2 handles for DFX processing */ 	
    StereoVolume pcmVol;            /* pcm Volume */
    int bNewSong;
    int nBytesElapsed;                /* to calc progress */
} ACard;

/* function definitions */
/* mixer functions */
void pcmFadeVolume(int length);
int  pcmSetSysVolume(StereoVolume vol);
int  pcmSetPcmVolume(StereoVolume vol);

void pcmPause();
void pcmUnPause();
int pcmInit(ACard *pCard, int buffer_size); 	
int pcmOpen(ACard *pCard);                           /* open a stream */
int pcmSubmitBuffer(ACard *pCard,unsigned char *frame_data,int nFrameSize); 
int pcmClose(ACard *pCard);	 	                     /* close down PCM stream */
int pcmShutDown(ACard *pCard);				         /* shut down PCM */
int pcmFlush(ACard *pCard);		                     /* flushes pending data */ 
int pcmSendAudioThread(ACard *pCard);                /* writes to audio HW */

#ifdef _DFXENABLED
void pcm_setdfx(int setting);  			/* DFX value between 900000 917777 */
						/* 9<on||off><fid><amb><dyn><bass> */
#endif

#ifdef PCM_C

int gPcmPaused = FALSE; /* is the PCM subsystem paused? */
int gPcmbuffersize = 0; /* number of frames in the buffer (frame = 8bytes) */

/* function definitions */
void *pcmCardUpdate(); /* thread that runs for each opened card device */

#endif

#endif
