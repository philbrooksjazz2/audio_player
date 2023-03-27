
/*file apdpcm.c
** Author: Phil Brooks
**
** Description:
** Handles the distribution of PCM (uncompressed) audio 
** to the soundcard.   
**
** Copyright (C) 2006 Musicnet, Inc.
*/
/*
 * $Log: apdpcm.c,v $
 * Revision 1.15  2006/05/23 19:35:17  pbrooks
 * Added support for GetTime, State, GetBitrate, GetAudioParams, GetState,
 *        GetCurrent, SetCurrent, ClearCurrent.
 *
 * Revision 1.14  2006/05/12 23:54:36  pbrooks
 * Additional cleanup.
 *
 * Revision 1.13  2006/05/12 00:47:06  yliu
 * fix bugs related to threading and memory
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
 * Revision 1.9  2006/05/05 16:56:24  pbrooks
 * Added additional defines.
 *
 * Revision 1.8  2006/05/05 01:20:26  pbrooks
 * Optimization changes.
 *
 * Revision 1.7  2006/05/03 21:11:26  pbrooks
 * Integration with libmad now plays music correctly on the desktop. Arm version
 * choppy, needs optimization.
 *
 * Revision 1.6  2006/05/03 16:24:18  pbrooks
 * Now playing scratchy noise.
 *
 * Revision 1.5  2006/05/03 01:28:51  pbrooks
 * First cut at libmad decoder.
 *
 * Revision 1.4  2006/04/29 00:42:11  pbrooks
 * Audio play milestone.
 *
 * Revision 1.3  2006/04/27 01:00:21  pbrooks
 * First cut at embedded decode and play.
 *
 * Revision 1.2  2006/04/26 23:52:17  pbrooks
 * Changes for threaded decode and play.
 *
 * Revision 1.1.1.1  2006/04/21 16:30:29  pbrooks
 * Initial checkin of embedded player files.
 *
 *
 *
 */

#define PCM_C

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/soundcard.h> 
#include <string.h>
#include <sys/mman.h>
#include <math.h>
//#include <DfxPortSdk.h>  
#include <sys/time.h>
#include <sys/resource.h>     
#include <sched.h>


#include "apdpcm.h"
#include "apdutil.h"

int bThreadStop = FALSE;

int pcmGetThreadFlag()
{
    return bThreadStop;
}
void pcmStop()
{
   bThreadStop = TRUE;
}
void pcmGo()
{
   bThreadStop = FALSE;
}

void pcmPause() 
{
    gPcmPaused = TRUE;
}
void pcmUnPause() 
{
    gPcmPaused = FALSE;
}

int pcmGetPcmVolume(StereoVolume *pVol) 
{

    int fd = -1;
    char filename[256];

    /* open the mixer device for the card */
    sprintf(filename,"%s","/dev/mixer");
    fd = open(filename, O_RDWR, 0);
    if (fd == -1) 
    {
        return FALSE;
    }

    if (ioctl(fd, SOUND_MIXER_READ_PCM, pVol) != 0) 
    {
        /* failed */
        close(fd);
        return FALSE;           
    }   
    close(fd);
    return TRUE;
}

int pcmGetSysVolume(StereoVolume *pVol) 
{

    int fd = -1;
    char filename[256] = "";
    StereoVolume vol;

    /* open the mixer device for the card */
    sprintf(filename,"%s", "/dev/mixer" );
    fd = open(filename, O_RDWR, 0);
    if (fd == -1) 
    {
        return FALSE;
    }

    if (ioctl(fd, SOUND_MIXER_READ_VOLUME, pVol) != 0) 
    {
        /* failed */
        close(fd);
        return FALSE;           
    }   
    close(fd);
    return TRUE;
}
int pcmPlaySong(ACard *pCard)
{
    int nBufSize;
    int nSndParam;
    int nResult;
    char *tstack;
    unsigned int tstack_sz;
    char pArg[64];
    pthread_t th2;
    
}

int pcmSetPcmVolume(StereoVolume vol) 
{

    int fd = -1;
    char filename[256] = "";

    /* open the mixer device for the card */
    sprintf(filename, "/dev/mixer" );
    fd = open(filename, O_RDWR, 0);
    if (fd == -1) 
    {
        return FALSE;
    }

    /* write to the mixer device the new volumes */
    if (ioctl(fd, SOUND_MIXER_WRITE_PCM, &vol) != 0) 
    {
        /* failed */
        close(fd);
        return FALSE;           
    }
    
    close(fd);
    return TRUE;
}
#ifndef IPOD
/* function to provide a volume level for a particular time within a fade */
int pcmFadeLevel(double time, int start_level, double total_time) 
{

    double y = 0;
    double x = 0;
    int volume = 0;

    if (time < 0 || time > total_time) { return 0; }
    if (start_level < 0 || start_level > 100) { return 0; }

    /* get x value between 0 and 1.1 (covers us down to (cos x) ^ 4 */
    x = time / total_time * 1.1;
    y = pow(cos(x), 4);   /* y = (cos x) ^ 4 */

    /* get y between 0 and start_level */
    y = y * start_level;
    
    /* round off */
    volume = (int) y;

    /* minimum check */
    if (volume < 5) { volume = 0; }
    
    return volume;
}
#endif // IPOD
/* blocking function to fade volume */
void pcmFadeVolume(int length) 
{
    int index = 0;
    int level;
//    double time = 0;
    int leftvol = 0;
    int rightvol = 0;
    int card_num = 0;
    StereoVolume vol1;
    StereoVolume vol2;
    int nStep;

    pcmGetPcmVolume(&vol1);
    level = vol1.left;

    printf("read vol %d\n",level);

    nStep = vol1.left / PCM_FADESTEPS;

    if (nStep == 0)
        nStep = 1;

    
    /* loop round to do a length seconds fade */
    for (index = 0; index < PCM_FADESTEPS; index++) 
    {
//        time = 1.0 * index / PCM_FADESTEPS * length;
        vol1.left = (level - (nStep*(index+1)));
        if ( vol1.left <= 5 )
            vol1.left = 2;
        vol1.right = vol1.left;
        
//        vol1.left = pcmFadeLevel(time, levels[0], length);
//        vol1.right = pcmFadeLevel(time, levels[1], length);
        pcmSetPcmVolume(vol1); 
        usleep((length * 30000) / PCM_FADESTEPS);
    }
    vol1.left = 1;
    vol1.right = 1;
    pcmSetPcmVolume(vol1); 
    pcmSetPcmVolume(vol1); 
    
}

int pcmSetSysVolume(StereoVolume vol) 
{
    int fd = -1;
    char filename[256] = "";
    int nRet;
    
    /* open the mixer device for the card */
    sprintf(filename,"%s","/dev/mixer");
    fd = open(filename, O_RDWR, 0);
    if (fd != -1) 
    {
        /* write to the mixer device the new volumes */
        nRet = ioctl(fd, SOUND_MIXER_WRITE_VOLUME, &vol); 
        close(fd);
    }
    if( nRet == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* initialise the fist PCM_MAXCARDS dsp devices for use by the PCM Subsystem */
int pcmInit(ACard *pCard, int buffer_size) 
{

    int index = 0;
    int tmphandle = -1;
    char audio_device[32];
    int dummy;
    int nBufSize;
    int nSndParam;
    int nResult;
    char *tstack;
    unsigned int tstack_sz;
    char pArg[64];
    pthread_t th2;
    

//    gPcmPaused = TRUE;
    
    sprintf(pArg,"%s","Uno");

    apdLogPrint("APD32", APD_LOG_INFO, "pcmInit 1");

    tstack_sz = 0x10000;

    tstack = malloc(tstack_sz);

    /* what is the buffer size? */
    if (buffer_size < 1) 
    {
        buffer_size = 1;
    }
    if (buffer_size > PCM_MAXBUFFERSIZE) 
    {
        buffer_size = PCM_MAXBUFFERSIZE;
    }
    pCard->pcmBufferSize = buffer_size;

    /* initialise internal data structures */
    
    pCard->handle = -1;
    pCard->available = TRUE;
    pCard->active_channel[0] = FALSE;
    pCard->active_channel[1] = FALSE;
    pCard->buffer_pending[0] = 0;
    pCard->buffer_pending[1] = 0;
    pCard->xferbuf[0] = NULL;
    pCard->xferbuf[1] = NULL;
    pCard->xferbuf_pos[0] = 0;
    pCard->xferbuf_pos[1] = 0;
    pCard->pcmbuf = NULL;
//    pCard->dfx_handle[0] = NULL;   
//    pCard->dfx_handle[1] = NULL;  
    pCard->pcmVol.left = 0;
    pCard->pcmVol.right = 0;

    /* try and open the device   */
    sprintf(audio_device,"%s","/dev/dsp");   
    tmphandle = open(audio_device,O_WRONLY);
        
    nBufSize = pCard->pcmBufferSize * PCM_FRAMESIZE * PCM_NUMPCMBUF;

    if (tmphandle != -1) 
    {
        apdLogPrint("APD32", APD_LOG_INFO, "pcmInit 2");
        /* set sound format - 16bit Little Endian */
        nSndParam = AFMT_S16_LE;
        ioctl(tmphandle, SNDCTL_DSP_SETFMT, &nSndParam);  
        nSndParam = 1;   /* PCM handler only supports stereo  */
        ioctl(tmphandle, SNDCTL_DSP_STEREO, &nSndParam); 
        nSndParam = PCM_SAMPLERATE;
        ioctl(tmphandle, SNDCTL_DSP_SPEED, &nSndParam);  
        /* device opened successfully */
        /* note that in the case of USB, we get here even if we were unable to initialize card*/
        pCard->handle = tmphandle;
        pCard->available = TRUE;
      
        /* allocate buffers - PCM and Transfer - pCard->pcmBufferSize is in  */
        /* frames (PCM_FRAMESIZE Bytes), and pCard->pcmBufferSize is for ONE  */
        /* channel */
        pCard->xferbuf[0] =(unsigned char *)malloc(nBufSize);
        pCard->xferbuf[1] =(unsigned char *)malloc(nBufSize);        
        pCard->pcmbuf = (unsigned char *) malloc (nBufSize * 2);
    }
    
    //#ifdef IPOD
    //clone((&pcmSendAudioThread),(tstack+tstack_sz),clone_flags,pCard);
    //#else
    pthread_create(&th2,NULL,(pcmSendAudioThread),pCard);
    //#endif // IPOD

    apdLogPrint("APD35", APD_LOG_INFO, "pcmInit 3");
//    nResult = clone(&pcmSendAudioThread,tstack,0,(void *)(pCard));
    return SUCCESS;
}

/*open stream*/
int pcmOpen(ACard *pCard) 
{
    int nRet = FALSE;
    char logstr[256];
    int nCount = 0;
    apdLogPrint("APD20", APD_LOG_INFO, "pcmOpen entry");

    /* make sure card is available before doing anything */
    if (pCard->available) 
    {
        apdLogPrint("APD21", APD_LOG_INFO, "pcmOpen entry 2");

        /* make sure we are able to use the channel specified on the card */
        if (pCard->active_channel[0] ||
            pCard->active_channel[1]) 
        {
            apdLogPrint("APD51", APD_LOG_INFO, "pcmOpen entry 3");
            nRet = FALSE;
        } 
        else 
        {
            apdLogPrint("APD53", APD_LOG_INFO, "pcmOpen entry 4");
            pCard->active_channel[0] = TRUE;
            pCard->active_channel[1] = TRUE;
            pCard->buffer_pending[0] = 0;
            pCard->buffer_pending[1] = 0;
            pCard->xferbuf_pos[0] = 0;
            pCard->xferbuf_pos[1] = 0;
        }           
        nRet = TRUE;
    
        /* clear the memory for the stream */
        memset(pCard->xferbuf[0],0,pCard->pcmBufferSize*PCM_FRAMESIZE*PCM_NUMPCMBUF); 

    }
    /* all done, so unlock card */

    return nRet;
}

/* close down the PCM stream so the handler for the card is no longer */
/* expecting data for that stream */
int pcmClose(ACard *pCard) 
{
    
    /* make the stream available to another user */
    pCard->active_channel[0] = FALSE;
    pCard->active_channel[1] = FALSE;   
    pCard->xferbuf_pos[0] = 0;
    pCard->xferbuf_pos[1] = 0;
    pCard->buffer_pending[0] = 0;
    pCard->buffer_pending[1] = 0;

    memset(pCard->xferbuf[0],0,pCard->pcmBufferSize*PCM_FRAMESIZE*PCM_NUMPCMBUF); 
    return SUCCESS;
}

/* flush pending data in the buffers for a stream */
int pcmFlush(ACard *pCard) 
{
    /* loop until pending buffer is empty */
    char log[128];

    sprintf(log,"buffer pernding is %d",pCard->buffer_pending[0]);
    apdLogPrint("APD40", APD_LOG_INFO, log);

    pCard->buffer_pending[0] = 0;
        
    while (TRUE) 
    {
        if (pCard->buffer_pending[0] <= 0) 
        {        
            break;
        }       
        usleep(PCM_LONGSLEEP);
    }
    return SUCCESS;
}
    
/* Shut down entire PCM subsystem.  Free all memory, close all devices.. */
int pcmShutDown(ACard *pCard) 
{
    int index;
    int timer = 0;

    /* was the card ever available? */
    if (pCard->available) 
    {
        /* make the card unavailable */
        pCard->available = FALSE;
        
#ifndef PCM_NOWRITE     
        /* close the audio device */
        close(pCard->handle);
#endif // PCM_NOWRITE
        /* free internal buffers */
        free((void *)pCard->xferbuf[0]);
        free((void *)pCard->xferbuf[1]);       
        free((void *)pCard->pcmbuf);
    
        /* reset stuff */
        pCard->active_channel[0] = FALSE;
        pCard->active_channel[1] = FALSE;
        pCard->buffer_pending[0] = 0;
        pCard->buffer_pending[1] = 0;
    }
    return SUCCESS;
}

/* submit PCM data to a stream - pcm_buffer_size frames are passed on one go */
int pcmSubmitBuffer(ACard *pCard, unsigned char *frame_data, int nFrameSize) 
{
    int good_stream = TRUE;
    unsigned char *ptr = NULL;
    int index = 0;
    int bufpos = 0;
    int nRet = TRUE;

//    pCard->pcmBufferSize = nFrameSize;


    // block until the player has consumed enough data to free up 
    // a buffer.

    while (TRUE) 
    {
        /* check both channels for this stream */
        if (pCard->buffer_pending[0] < PCM_NUMPCMBUF) 
        {
            break;
        }

        usleep(PCM_LONGSLEEP);
    }
        
    if (pCard->buffer_pending[0] <= 0) 
    {
        bufpos = 0;
        pCard->xferbuf_pos[0] = 0;
    } 
    else 
    {
        bufpos = pCard->xferbuf_pos[0] + 
             pCard->buffer_pending[0];
        if (bufpos >= PCM_NUMPCMBUF) 
        {
            bufpos -= PCM_NUMPCMBUF;
        }
    }
    ptr = (unsigned char *) (pCard->xferbuf[0] 
        + pCard->pcmBufferSize * (PCM_FRAMESIZE) * bufpos);
//    ptr = (unsigned char *) (pCard->xferbuf[0] 
//        + pCard->pcmBufferSize * bufpos);
    
    /* check that the stream specified is currently active & copy data */
    if ((pCard->active_channel[0] || pCard->active_channel[1]) && pCard->available ) 
    {
//        memcpy(ptr, frame_data, pCard->pcmBufferSize );
        memcpy(ptr, frame_data, pCard->pcmBufferSize * (PCM_FRAMESIZE));
        pCard->buffer_pending[0]++;
        nRet = TRUE;
    }
    else
    {
        nRet = FALSE;
    }
    return nRet;
}
#define ZEROBUF_SZ  8192
/* **************** PRIVATE FUNCTIONS *************** */

/* Performs actual writing to PCM devices */
int pcmSendAudioThread(ACard *pCard) 
{
    int nNumbytes = 0;
    unsigned char *xferptr[2] = {NULL, NULL};
    int xferindex[2] = {0, 0};
    struct sched_param schedp;
    unsigned char zerobuf[ZEROBUF_SZ];
    int i;
    int nCount = 0;
    static int nInitialBuf = (PCM_NUMPCMBUF - 2);
    char sLogMsg[256];


//    zerobuf = malloc(ZEROBUF_SZ);

    apdLogPrint("APD30", APD_LOG_INFO, "SendAudio entry");
//    gPcmPaused = TRUE;

    memset(zerobuf, 0, ZEROBUF_SZ);
    
    while (1)
    {
        if ( pCard->bNewSong )
        {
             nInitialBuf = (PCM_NUMPCMBUF - 2);
             pCard->bNewSong = FALSE;
        }
        if (gPcmPaused) 
        {
            write(pCard->handle, zerobuf, ZEROBUF_SZ); 
        } 
        else 
        {
            nCount++;
            /* check to see if we have any active channels */
            if (pCard->active_channel[0] &&
                pCard->active_channel[0]) 
            {
                nCount++;
                if (!(nCount%200))
                {
                   /* check to see if any buffer received any new data */
                   sprintf(sLogMsg,"Buffer pending = %d\n",pCard->buffer_pending[0]);
                   apdLogPrint("APD44", APD_LOG_INFO, sLogMsg);
                }
                if (pCard->buffer_pending[0] >= nInitialBuf)
                {
                    nInitialBuf = 1;
                    /* calculate pointers to current xfer buffers */

                    xferindex[0] = pCard->xferbuf_pos[0];
                    xferptr[0] = pCard->xferbuf[0] 

                        + xferindex[0] * pCard->pcmBufferSize * PCM_FRAMESIZE;
                    /* we have pending data on this channel */
                    xferindex[0]++;
                    if (xferindex[0] >= PCM_NUMPCMBUF) 
                    {
                       xferindex[0] = 0;
                    }
                    pCard->xferbuf_pos[0] = xferindex[0];

                    /* adjust number of buffers pending */
                    if (pCard->buffer_pending[0] > 0) 
                    {
                        pCard->buffer_pending[0]--;
                    }       
            
                    /* copy  data  */
                    memcpy((void *)pCard->pcmbuf, 
                         xferptr[0], pCard->pcmBufferSize * PCM_FRAMESIZE);
        
                    /* write data to PCM card */
                    nNumbytes = write(pCard->handle, pCard->pcmbuf, 
                       pCard->pcmBufferSize * PCM_FRAMESIZE); 
                    pCard->nBytesElapsed += nNumbytes;
                } 
            }        
        } 
        nCount++;
        usleep(1000);
    }
//    pthread_exit(NULL);
    return nNumbytes;
}

