/*!
** \file apd.h
** Author: Phil Brooks
**
** Description:
** DMX Linux base Header file
**
** Copyright (C) 2006 Musicnet Inc.
*/
/*
 * $Log: apd.h,v $
 * Revision 1.7  2006/05/02 22:17:17  pbrooks
 * Corrected socket command length.
 *
 * Revision 1.6  2006/05/02 17:45:17  yliu
 * handle multi commands in the same buffer
 *
 * Revision 1.5  2006/04/29 00:42:11  pbrooks
 * Audio play milestone.
 *
 * Revision 1.4  2006/04/27 01:00:21  pbrooks
 * First cut at embedded decode and play.
 *
 * Revision 1.3  2006/04/26 23:52:17  pbrooks
 * Changes for threaded decode and play.
 *
 * Revision 1.2  2006/04/21 16:42:03  pbrooks
 * Fixed Parse function.
 *
 * Revision 1.1.1.1  2006/04/21 16:30:29  pbrooks
 * Initial checkin of embedded player files.
 *
 *
 */
#ifndef APD_H
#define APD_H

#define BUFFER_SCALE 1

#define clone_flags (CLONE_VM|CLONE_FS|CLONE_FILES)


struct ACard;
#define APD_BASEPORT 8300  /* base port to listen on */
#define APD_PORTRANGE 10	/* can search up to 10 ports for available one */

#define APD_MESG_SIZE       1024
#define APD_CMD_SIZE        256
#define APD_CMD_COUNT       16
#define APD_DEFAULT_VOLUME  80

#define APD_DECODEERROR 	0
#define APD_FINISHED 	    1
#define APD_STOP		    3

#define APD_MSG_PLAY            "Play"
#define APD_MSG_STOP            "Stop"
#define APD_MSG_SETVOL          "SetVolume"
#define APD_MSG_GETVOL          "GetVolume"
#define APD_MSG_PING            "Ping"
#define APD_MSG_PAUSE           "Pause"
#define APD_MSG_SHUTDOWN        "Shutdown"
#define APD_MSG_SETCURRENT      "SetCurrent"
#define APD_MSG_GETCURRENT      "GetCurrent"
#define APD_MSG_CLEARCURRENT    "ClearCurrent"
#define APD_MSG_SETQUEUE        "SetQueue"
#define APD_MSG_GETQUEUE        "GetQueue"
#define APD_MSG_CLEARQUEUE      "ClearQueue"
#define APD_MSG_   ""
#define APD_MSG_   ""
#define APD_MSG_   ""
#define APD_MSG_   ""
#define APD_MSG_   ""
#define APD_MSG_   ""
#define APD_MSG_   ""
static char apd_msgname[][20] = { "SetCurrentSong",
				     "GetCurrentSong",
				     "ClearCurrentSong",
				     "PlaySong",
				     "SetQueue",
				     "GetQueue",
				     "ClearQueue",
				     "Stop",
				     "Pause",
				     "SetVolume",
				     "Crossfade",
				     "Status" };



#define MAX_BUFSIZE WMA_MAX_DATA_REQUESTED
#define MAX_SAMPLES 2048

typedef struct _WMAInfo {
	unsigned char pBuffer[WMA_MAX_DATA_REQUESTED];  /* wma buffer */
	tWMA_U32 cbBuffer;	                            /* WMA thing */
	tHWMAFileState state;		                    /* file state */
	unsigned char *wmabuf;	                        /*temp buf. for PCM data */
	int wmabuf_bytes;	                            /*# of bytes in wmabuf */
} WMAInfo;

typedef struct _SongInfo{

	int nValid;	        /* TRUE only if a decode thread is running */
	char sFilename[256]; /* filename of file to play */
	char sStagename[256];/* filename of staged file to play */
	int vol;	        /* vol level at which to play this content */
	int stopflag;	    /* if TRUE - decoding must stop and thread must exit */
	int quietstop;	    /* if TRUE - when stopping thread - do not send msg */
    int finishtrack;    /* flag to finish track */
    int pcmnumblocks;   /* total pcm blocks */
    int comms_fd;       /* socket fd */
	FILE *fd;           /* file we are decrypting */
    WMAInfo wma;        /* WMA Info */

} SongInfo;

typedef struct _PlayInfo {
    SongInfo *songinfo;
    ACard    *cardinfo;
} PlayInfo;


//int comms_fd = -1;
//int pcmnumblocks = 0;		/* a block is 8 bytes */

int apdDecode(PlayInfo *p);
int apdPlaySong(ACard *pCard, SongInfo *pInfo);
int apdMp3Decode(ACard *pCard, SongInfo *pInfo);

int  apdParseMesg(char *pRaw,char pVect[][APD_CMD_SIZE],char cSep);
int  apdPostMessage(char *pMesg, int nComfd);
int  apdEstablishSocket();
void apdProcessCommands(ACard *pCard, SongInfo *pSongInfo);
int  apdProcessMessage(ACard* pCard, SongInfo *pInfo, int nCmd, char pMesg[][APD_CMD_SIZE]);


#endif // APD_H
