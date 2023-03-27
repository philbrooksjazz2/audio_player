/*!
** \file apd.c
** Author: Phil Brooks
**
** Description:
** main() function for player is in here.  
** This module also handles the decoding (via Amp) 
** of the audio files.c
**
** Copyright (C) 2006 by Musicnet Inc.
*/
/*
 * $Log: stand_alond_apd.c,v $
 * Revision 1.1  2006/05/03 21:04:51  pbrooks
 * Stand alone hack changes.
 *
 * Revision 1.9  2006/05/03 01:28:51  pbrooks
 * First cut at libmad decoder.
 *
 * Revision 1.8  2006/05/02 22:16:56  pbrooks
 * Corrected socket command length.
 *
 * Revision 1.7  2006/05/02 17:45:17  yliu
 * handle multi commands in the same buffer
 *
 * Revision 1.6  2006/04/29 00:42:11  pbrooks
 * Audio play milestone.
 *
 * Revision 1.5  2006/04/27 01:00:20  pbrooks
 * First cut at embedded decode and play.
 *
 * Revision 1.4  2006/04/26 23:52:17  pbrooks
 * Changes for threaded decode and play.
 *
 * Revision 1.3  2006/04/21 22:33:04  pbrooks
 * Corrected apdParseMesg calls.
 *
 * Revision 1.2  2006/04/21 16:40:54  pbrooks
 * Cleaned up some initial compile errors.
 *
 * Revision 1.1.1.1  2006/04/21 16:30:29  pbrooks
 * Initial checkin of embedded player files.
 *
 *
 *
 */
#define APD_C

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <sys/resource.h>
#include <sched.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include <errno.h>
#include <sys/mman.h>
#include <pthread.h>

/*
#include <linux/soundcard.h>
*/

#include "WMA_Dec_Emb_x86.h"
#include "apdpcm.h"
#include "apdutil.h"
#include "apd.h"

#define UINT unsigned int
#define BYTE unsigned char
#define BOOL int
#define TRUE 1
#define FALSE 0
#include "ampdll.h"

#define SCHED_ALG SCHED_RR

int main(int argc, char *argv[])
{
    int dstnumblocks;
    int srcnumblocks;
    UINT srcsize, destsize;
    struct sched_param schedp;
    int sockfd = -1;
    unsigned int sin_size;
    struct sockaddr_in their_addr;     
    int index;
    int nPcmStatus;
    ACard Card;
    SongInfo Info;

    nPcmStatus = 0;


    apdLogSetFile("/tmp/player.log");
    apdLogPrint("PF00", APD_LOG_INFO, "Player Starting");


//    mlockall(MCL_CURRENT | MCL_FUTURE);

    /* setup real time scheduling
       - be sure to set it up _before_ calling amp_open_stream()
             so that priorities are inherited properly.
    */
   // memset(&schedp,0,sizeof(schedp));
   // schedp.sched_priority = sched_get_priority_min(SCHED_ALG);
   // sched_setscheduler(0, SCHED_ALG, &schedp);

#ifdef AMP 
    srcsize = amp_default_srcsize() * BUFFER_SCALE;
    destsize = amp_default_destsize() * BUFFER_SCALE;
    srcnumblocks = srcsize / 8;
    dstnumblocks = destsize / 8;

    if (srcsize % 8 != 0) 
    {
        apdLogPrint("PF01", APD_LOG_ERROR, "Amp Error");
//        exit(-1);
    }
    if (destsize % 8 != 0) 
    {
        apdLogPrint("PF02", APD_LOG_ERROR, "Amp Error");
//        exit(-1);
    }
#else
    srcsize = MADPCM_SAMPLES;
//    destsize = amp_default_destsize() * BUFFER_SCALE;
    srcnumblocks = srcsize / 8;
    dstnumblocks = MADPCM_SAMPLES;
#endif
    nPcmStatus = pcmInit(&Card, dstnumblocks);

    Info.pcmnumblocks = dstnumblocks;    /* store for later */

    /* ensure we have a connection */
    while (sockfd = apdEstablishSocket(), sockfd == -1) { sleep(1); }

    /* loop forever */
    while (TRUE) 
    {
        apdProcessCommands(&Card,&Info);

        /* listen on the port for one connection only */
        if (listen(sockfd, 1) == -1) 
        {
            apdLogPrint("PF03", APD_LOG_ERROR, "Network Listen error");
            close(sockfd);
            /* ensure we have a connection */
            while (sockfd = apdEstablishSocket(), sockfd == -1) { sleep(1); }
            usleep(500000);
            continue;
        }

        sin_size = sizeof(struct sockaddr);
        Info.comms_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (Info.comms_fd == -1) 
        {
            apdLogPrint("PF04", APD_LOG_ERROR, "Network Accept Error");
            close(sockfd);
            while (sockfd = apdEstablishSocket(), sockfd == -1) { sleep(1); }
            usleep(500000);
            continue;
        }

        /* set socket to nonblocking */
        fcntl(Info.comms_fd, F_SETFL, O_NONBLOCK);

        /* continue reading and writing until some error occurs */
        apdProcessCommands(&Card,&Info);

        /* close the socket */
//      close(comms_fd);
        shutdown(Info.comms_fd, 2);
        
        Info.comms_fd = -1;
    }

    exit(0);
}

/* process commands until some error occurs reading or writing.  Then exit function
and connection will be re-established */
void apdProcessCommands(ACard *pCard, SongInfo *pSongInfo) 
{
    int retval;
    int index;
    int nCmd;
    char pMsgBuf[APD_MESG_SIZE];
    char pMesg[APD_CMD_COUNT][APD_CMD_SIZE];
     static int nTest = TRUE;

    printf("A\n");
    while (TRUE) 
    {

        /* see if there is a message to receive */
    printf("AA\n");
        
        /* non blocking receive call on the socket */
        if ( nTest == FALSE )
        retval = recv(pSongInfo->comms_fd, (char *) pMsgBuf, APD_MESG_SIZE, 0);

    printf("AAA\n");
        if (retval == -1 && errno != EAGAIN) 
        {
            apdLogPrint("PF05", APD_LOG_ERROR, "Failed to receive message from Manager");
            return;
        }
        
    printf("AAAA\n");
        if (retval == 0) 
        {
//            apdLogPrint("APD08", APD_LOG_ERROR, "Connection to Client is dead - restarting Player");
            //exit(8);
//            return;
        }

//        if (retval != -1) 
        if (nTest == TRUE) 
        {
            printf("B\n");
            nTest = FALSE;
//            nCmd = apdParseMesg(pMsgBuf,pMesg,'\n');
            if (!apdProcessMessage(pCard, pSongInfo, nCmd, pMesg)) 
            { 
                return; 
            }
        }
        /* sleep for a little while */
        usleep(100000);
    }
}

int apdParseMesg(char *pRaw,char pVect[][APD_CMD_SIZE] ,char cSeperator)
{

    int i,j;
    int nCmd;
    int nLen;

    fprintf(stderr, "recved: %s\n", pRaw);

    j = 0;
    nCmd = 0;
    nLen = strlen(pRaw);

    for( i = 0; i <= nLen; i++)
    {
        if (pRaw[i] == cSeperator)
        {
            pVect[nCmd][j] = '\0';
            nCmd++;
            i++;
            j = 0;
        }
        pVect[nCmd][j] = pRaw[i];
        j++;

    }
    return nCmd;
}

/* process the message that came in from the management process */
int apdProcessMessage(ACard *pCard, SongInfo *pSongInfo, int nCmd, char pMesg[][APD_CMD_SIZE]) 
{
    int index;
    int nVol;
    apddate date;
    char logstr[256] = "";
    StereoVolume pVol;
    int i;
    char pCommands[APD_CMD_COUNT][APD_CMD_SIZE];
    char pRetMesg[128];
    static int nTest = TRUE;

    

            printf("BB\n");

    nCmd = 1;

    apdGetDate(&date);

    for(i=0; i<nCmd; i++) {

      sprintf(logstr,"Received %s message from client",pMesg[i]);
      apdLogPrint("APD05", APD_LOG_INFO, logstr);


      /* do we have a PING message? */
      if (!strcmp(pMesg[i],APD_MSG_PING)) 
	{
	  apdLogPrint("APD01", APD_LOG_INFO, "Received PING message");
	  /* send the message right back! */
	  if (!apdPostMessage(APD_MSG_PING,pSongInfo->comms_fd)) 
	    {
	      apdLogPrint("PF08", APD_LOG_ERROR, "Ping Ack Failed");
	      return FALSE;
	    }
	  return TRUE;
	}
    
      /* do we have a sysvol message? */
      if (!strncmp(pMesg[i],APD_MSG_SETVOL,strlen(APD_MSG_SETVOL)))
	{
	  apdLogPrint("APD01", APD_LOG_INFO, "Received SETVOL message");
	  apdParseMesg(pMesg[i],pCommands,'"');
	  sscanf(pCommands[1],"%d",&nVol);
	  sprintf(logstr, "Setting system volume to %d", nVol); 
	  apdLogPrint("APD47", APD_LOG_INFO, logstr);
	  pVol.left = nVol;
	  pVol.right = nVol;
	  if (!pcmSetPcmVolume(pVol)) 
	    {
	      sprintf(logstr, "Could not set volume %d", nVol); 
	      apdLogPrint("PF09", APD_LOG_ERROR, logstr);
	      return TRUE;        
	    }
	}
      if (!strcmp(pMesg[i],APD_MSG_GETVOL))
	{
	  apdLogPrint("APD01", APD_LOG_INFO, "Received GETVOL message");
	  if (!pcmGetSysVolume(&pVol)) 
	    {
	      sprintf(logstr, "Could not get volume"); 
	      apdLogPrint("PF09", APD_LOG_ERROR, logstr);
	      return TRUE;        
	    }
	  sprintf(pRetMesg,"%d",pVol.left);
	  if (!apdPostMessage(pRetMesg,pSongInfo->comms_fd)) 
	    {
	      apdLogPrint("APD11", APD_LOG_ERROR, "APD_MSG_GETVOL failed");
	      return FALSE;
	    }
	}

      if (!strcmp(pMesg[i],APD_MSG_STOP)) 
	{
	  apdLogPrint("APD01", APD_LOG_INFO, "Received STOP message");
	  apdLogPrint("PF90", APD_LOG_INFO, "Stopping all zones");
	  pcmFadeVolume(2);     
	  apdLogPrint("PF91", APD_LOG_INFO, "All playback zones stopped");     
	}
      if (!strncmp(pMesg[i],APD_MSG_PAUSE,strlen(APD_MSG_PAUSE))) 
	{
	  apdParseMesg(pMesg[i],pCommands,'"');
	  apdLogPrint("APD01", APD_LOG_INFO, "Received PAUSE message");
	  if ( atoi(pCommands[1]) )
            pcmPause();
	  else
            pcmUnPause();
	}

      /* do we want to play a song?  */
//      if (!strncmp(pMesg[i],APD_MSG_PLAY,strlen(APD_MSG_PLAY))) 
      if (nTest == TRUE) 
	{
      nTest = FALSE;
	  apdParseMesg(pMesg[i],pCommands,'"');
	  apdLogPrint("APD01", APD_LOG_INFO, "Received PLAY message");
	  /* check if we are currently playing something on that zone */
	  /* stop playback on that zone */
	  pSongInfo->quietstop = TRUE;
	  pSongInfo->stopflag = TRUE;
    
	  /* set up the new filename and pcm level of the new song */
	  pSongInfo->vol = APD_DEFAULT_VOLUME;
	  pSongInfo->quietstop = FALSE;
	  pSongInfo->stopflag = FALSE;
	  sprintf(pSongInfo->sFilename,"%s",pCommands[1]);
	  sprintf(pSongInfo->sFilename,"%s","/tmp/xxx.mp3"); 
	  sprintf(logstr,"Received %s for play queue",pSongInfo->sFilename);
	  apdLogPrint("APD10", APD_LOG_INFO, logstr);
	  strcpy(pSongInfo->sStagename, "");
#ifdef USE_WMA
	  /* handle WMA buffers */
	  pSongInfo->wma.wmabuf_bytes = 0;
	  if (pSongInfo->wmabuf != NULL) 
	    {
	      free((void *)pSongInfo->wmabuf);
	      pSongInfo->wmabuf = NULL;
	    }
#endif // USEWMA
	  apdPlaySong(pCard,pSongInfo);
	  //        return TRUE;
	}

      /* do we want to stop a zone? */
      if (!strcmp(pMesg[i],APD_MSG_STOP)) 
	{
	  apdLogPrint("APD01", APD_LOG_INFO, "Received STOP message");

	  /* check if we are playing anything on that zone */
	  /* something is running - stop it */
	  /* stop playback on that zone */
	  /* stop only tracks that are music */
	  /* check if we need to do anything about the pending file */

	  /* check if we need to do anything about the playing file */
	  sleep(1); //Both sleep is needed to make the fading works
	  pcmFadeVolume(1);
	  sleep(1); //Both sleep is needed
	  // Do what it takes to stop
	  //return TRUE;
	}

      /* do we want to stop all the zones? */
      if (!strcmp(pMesg[i],APD_MSG_SHUTDOWN)) 
	{
	  apdLogPrint("APD01", APD_LOG_INFO, "Received SHUTDOWN message");
	  apdLogPrint("PF13", APD_LOG_INFO, "Shutting down all zones");
	  pcmFadeVolume(1);
          
	  apdLogPrint("PF14", APD_LOG_INFO, "Shutting down PCM subsystem");
	  pcmShutDown(pCard);

	  apdLogPrint("PF15", APD_LOG_INFO, "Exiting apd");
	  exit(0);
	  //        return TRUE;
	}
      // service play buffers and return, allowing new messages to
      // be processed.
      // apdDecode(pCard, pSongInfo); 
    }

    return TRUE;
}

/* function to post a message to be sent to the client process */
int apdPostMessage(char *pMesg,int com_fd) 
{
    int retval;
    int nRet;
    apddate date;

    if (com_fd != -1) 
    {
    
        apdGetDate(&date);
    
        /* send the message to the socket */
        retval = send(com_fd, (void *)pMesg, strlen(pMesg), 0x40); 

        if (retval == -1) 
        {
            if (errno == EAGAIN) 
            {
                apdLogPrint("PF18", APD_LOG_ERROR, "Send function wants to block");
                return FALSE;
            } 
            else 
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}

/* function to establish a socket bound to the port APD_PORT in
a range APD_BASEPORT -> (APD_BASEPORT + PLAYPFC_PORTRANGE - 1) */
int apdEstablishSocket() 
{

    int port = APD_BASEPORT;
    struct sockaddr_in my_addr;     
    int sockfd;
    char logstr[256] = "";
    int opt = 1;

    /* get a socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0); /* do some error checking! */
    if (sockfd == -1) 
    {
        apdLogPrint("PF19", APD_LOG_ERROR, "Network socket error");
        return -1;
    }

    /* set socket option so that we can re-bind to this port if restarted */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &opt, sizeof(int)) != 0) 
    {
        apdLogPrint("PF86", APD_LOG_ERROR, "Cannot set socket options");
        return -1;
    }


    for (port = APD_BASEPORT; 
         port < APD_PORTRANGE + APD_BASEPORT;
         port++) 
    {
         
            /* try to bind to port */
        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(port);
        my_addr.sin_addr.s_addr = htons(INADDR_ANY);
        bzero(&(my_addr.sin_zero), 8);

        if (bind(sockfd, (struct sockaddr *)&my_addr, 
                sizeof(struct sockaddr)) == -1) 
        {
            sprintf(logstr, "Cannot bind to port %d", port);
            apdLogPrint("PF20", APD_LOG_WARNING, logstr);
        } 
        else 
        {
            sprintf(logstr, "Bound to port %d", port);
            apdLogPrint("PF32", APD_LOG_INFO, logstr);
            return sockfd;
        }
    }

    apdLogPrint("PF33", APD_LOG_ERROR, "Could not bind to any port");
    
    /* obviously could not bind to any port */
    close(sockfd);
    return -1;
}

apdPlaySong(ACard *pCard, SongInfo *pInfo)
{
    int dstack_sz;
    char *dstack;
    PlayInfo pPlayInfo;
    pthread_t th1;

    dstack_sz = 0x40000;

    dstack = malloc(dstack_sz);

    pPlayInfo.cardinfo = pCard;
    pPlayInfo.songinfo = pInfo;
//    testfunc(&pPlayInfo);
  
#ifdef IPOD
    clone((&apdDecode),(dstack+dstack_sz),clone_flags,&pPlayInfo);
#else
    pthread_create((&th1),NULL,apdDecode,&pPlayInfo);
#endif // IPOD
    
}
testfunc(PlayInfo *p)
{

     printf("Playing song %s\n",p->songinfo->sFilename);


}

#ifdef DO_WMA
void WMADebugMessage(const char* pszFmt, ... )
{
}

tWMA_U32 WMAFileCBGetData (
    tHWMAFileState hstate,
    tWMA_U32 offset,
    tWMA_U32 num_bytes,
    unsigned char **ppData) 
{
    tWMA_U32 ret;
    int index;
 
    /* find zoneindex for this thread */
    index = findzonebythread(getpid());
    if (index == -1) 
    {
        return 0;
    }

    tWMA_U32 nWanted = num_bytes <= (tWMA_U32) MAX_BUFSIZE ? num_bytes : (tWMA_U32) MAX_BUFSIZE;
    if(num_bytes != nWanted)
    {
        fprintf(stderr, "** WMAFileCBGetData: Requested too much (%lu).\n",
                num_bytes);
    }

    ret = aeisecure_wmafread(pSongInfo->pBuffer, offset, nWanted, pSongInfo.fd);

    pSongInfo->cbBuffer = ret;

    *ppData = pSongInfo->pBuffer;
    return ret;
}


tWMA_U32 WMAFileCBGetLicenseData (
    tHWMAFileState *state,
    tWMA_U32 offset,
    tWMA_U32 num_bytes,
    unsigned char **ppData) 
{
    return 0;
}
#endif // DO_WMA
/* this used to be playerthread */
/* Decode and buffer */
int apdDecode(PlayInfo *p) 
{

    char logstr[256] = "";
    struct sched_param schedp;
    int ret = 0;
    int keepgoing = TRUE;
    int nCardStatus;
    StereoVolume pVol;
    FILE *temp_fp = NULL;

    
#ifndef IPOD
    mlockall(MCL_CURRENT | MCL_FUTURE);
#endif // IPOD

    /* open the PCM stream */
    nCardStatus = pcmOpen(p->cardinfo); 
    if (!nCardStatus)
    {
        sprintf(logstr, "%s","Cannot open stream for audio channel");
        apdLogPrint("PF23", APD_LOG_ERROR, logstr);
    }

//    /* send the started message */
//    make_message(APD_MSG_STARTEDPLAY, zoneindex, &taskmsg);
//    if (!post_message(&taskmsg)) {
//        apdLogPrint("PF26", APD_LOG_ERROR, "Cannot send Started Play message");
//    }

    sprintf(logstr, "Playing %s on audio device", p->songinfo->sFilename);
    apdLogPrint("PF27", APD_LOG_INFO, logstr);

    do 
    {
        /* attempt to open the input (encrypted) file */
        temp_fp = fopen(p->songinfo->sFilename,"rb"); 
        p->songinfo->fd = temp_fp; 
        sprintf(logstr, "file pointer =  %p",temp_fp);
        apdLogPrint("XXX32", APD_LOG_INFO, logstr);
        if (p->songinfo->fd == NULL) 
        {
            sprintf(logstr, "Cannot open input file %s",p->songinfo->sFilename);
            apdLogPrint("PF21", APD_LOG_ERROR, logstr);
            p->songinfo->quietstop = FALSE;
            break;                      
        }
        apdLogPrint("XXX1", APD_LOG_INFO, "A");

        /* flush the PCM buffers (so volume change occurs as close as possible to transition) */
        //pcmFlush(p->cardinfo);
        apdLogPrint("XXX2", APD_LOG_INFO, "b");

        /* send the mixer a command to change the PCM volume for this zone */
        pVol.left = p->songinfo->vol;
        pVol.right = p->songinfo->vol;
        if (!pcmSetPcmVolume(pVol)) 
        {
            sprintf(logstr, "Could not set PCM volume %d",
                pVol.left); 
            apdLogPrint("PF25", APD_LOG_ERROR, logstr);
        }
        apdLogPrint("XXX3", APD_LOG_INFO, "c");

        /* make sure quietstop flag is okay */
        p->songinfo->quietstop = FALSE;

//        ret = apdMp3decode(p->cardinfo, p->songinfo);
        ret = apdMadDecode(p->cardinfo, p->songinfo);

#ifdef DO_WMA
        /* delete any WMA buffer we might have */
        if (pSongInfo->wma.wmabuf != NULL) {
            free((void *)pSongInfo->wma.wmabuf);
            pSongInfo->wma.wmabuf = NULL;
        }
        pSongInfo->wma.wmabuf_bytes = 0;

        /*  open AMP to decode or initialise WMA decoding engine */
        if (aeisecure_wmafile(pSongInfo->fd)) 
        {
            ret = playpfc_wmadecode();
        } 
        else 
        {
            ret = apdMp3decode();
        }
        
        /* close the secure file */
        aeisecure_fclose(pSongInfo->fd);
#endif // DO_WMA
        
        apdLogPrint("XXX4", APD_LOG_INFO, "d");
        /* check the return code */
        if (ret == APD_DECODEERROR || ret == APD_FINISHED) 
        {
            /* check to see if something is staged */
            
            if (p->songinfo->sStagename[0] != '\0') 
            {
                strncpy(p->songinfo->sFilename, p->songinfo->sStagename, 255);
                strcpy(p->songinfo->sStagename, "");
        
                /* send the MOVED message back to the Management thing */
                //make_message(APD_MSG_MOVEDPLAY, zoneindex, &taskmsg);
                sprintf(logstr, "Playing %s",p->songinfo->sFilename); 
                apdLogPrint("PF31", APD_LOG_INFO, logstr);
            } 
            else 
            {
                keepgoing = FALSE;
                p->songinfo->quietstop = FALSE;
            }
        } 
        else if (ret == APD_STOP) 
        {
            keepgoing = FALSE;      
        }       
        apdLogPrint("XXX5", APD_LOG_INFO, "e");
    } while (keepgoing);

    pcmFlush(p->cardinfo);
    pcmClose(p->cardinfo);   

//    if (!pSongInfo->quietstop) 
//    {
//        make_message(APD_MSG_STOPPEDPLAY, zoneindex, &taskmsg);
//        if (!post_message(&taskmsg)) 
//        {
//            apdLogPrint("PF37", APD_LOG_ERROR, "Cannot send Stopped play message");
//        }
//    }
//    pSongInfo->threadexit = TRUE;
//    pthread_exit(NULL);
    return FALSE;
}

/* Only used by wma, so out for now */
#ifdef DO_WMA
/* prebuffer a load of data (maybe more, maybe less than pcmnumblocks size */
int playpfc_pcmdata(int zoneindex, unsigned char *src, int numbytes) 
{

    int bytesleft = numbytes;
    int offset = 0;
    int pcmbufsize = pcmnumblocks * 8;
    int copybytes = 0;
    void *destptr = NULL;
    void *srcptr = NULL;
    char logstr[256] = "";
    
    while (bytesleft > 0) 
    {
        /* calculate max number of bytes we can copy into wmabuf */
        copybytes = pcmbufsize - pSongInfo->wmabuf_bytes;
        
        /* do we have less than this number of bytes to copy? */
        if (bytesleft < copybytes) 
        {
            copybytes = bytesleft;
        }
        
        /* copy the data in wmabuf */
        destptr = (void *) ((unsigned int) pSongInfo->wmabuf + SongInfo->wmabuf_bytes);
        srcptr = (void *) ((unsigned int) src + offset);
        memcpy(destptr, srcptr, copybytes);
        /* adjust numbers */
        offset += copybytes;
        bytesleft -= copybytes;
        pSongInfo->wmabuf_bytes += copybytes;
        
        /* do we have a full wmabuf? */
        if (pSongInfo->wmabuf_bytes >= pcmbufsize) 
        {
            pSongInfo->wmabuf_bytes = 0;
            /* submit data to pcm handler */
            if (!pcmSubmitBuffer(pCard, (void *) SongInfo->wmabuf)) 
            {
                sprintf(logstr, "Error writing PCM buffer for card %d on channel %d",
                    pSongInfo->card_num, SongInfo->channel_mode);
                apdLogPrint("PF59", APD_LOG_ERROR, logstr);
                return FALSE;
            }
        }
    }
    return TRUE;
}
/* fill rest of buffer with zeros and submit to PCM handler */
void playpfc_flushpcm() 
{
    int bytesneeded;
    int pcmbufsize = pcmnumblocks * 8;
    void *dstptr = NULL;

    if (pSongInfo->wmabuf_bytes == 0) 
    {
        return;
    }

    /* how many zeros do we need? */
    bytesneeded = pcmbufsize - pSongInfo->wmabuf_bytes;
    
    /* copy the zeros in there */
    dstptr = (void *) ((unsigned int) pSongInfo->wmabuf + SongInfo->wmabuf_bytes);
    memset(dstptr, 0, bytesneeded);
    
    /* reset number of bytes in buffer */
    pSongInfo->wmabuf_bytes = 0;
    
    /* submit the PCM buffer */
    pcmSubmitBuffer(pSongInfo->pcmhandle, (void *)pSongInfo->wma.wmabuf);
    
    return;
}

#endif // DO_WMA
