/*!
** \file wma.c
** Author: Phil Brooks
**
** Description:
** WMA decode
**
** Copyright (C) 2006 by Musicnet Inc.
*/
/*
 * $Log: wma.c,v $
 * Revision 1.1.1.1  2006/04/21 16:30:29  pbrooks
 * Initial checkin of embedded player files.
 *
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "WMA_Dec_Emb_x86.h"
//#include "wmaudio.h"
#include "ampdll.h"
#include "apdpcm.h"
#include "apd.h"
#include "apdutil.h"

#include "../libmad/mad.h"
#include "../libmad/decoder.h"
#include "../libmad/fixed.h"

tHWMAFileState fstate;

#ifdef DO_WMA
int apdWmadecode(ACard *pCard, SongInfo *pInfo) 
{
    /* WMA stuff */
    tWMAFileHdrState hdrstate;
    tWMAFileHeader hdr;
    tWMAFileStatus rc;
    int code = 0;
    char logstr[256] = "";
    short pLeft [MAX_SAMPLES * 2];
    short pRight [MAX_SAMPLES * 2];
    short pOut [MAX_SAMPLES * 4];
    unsigned char *pBuf;
    int nMBRTargetStream = 1;
    long long pTime = 0;
    tWMA_U32 nSamples;
    FILE *fp;
    int i;
    
    printf("wma 1\n");
    /* initialise WMA stuff */
    memset ((void *)&hdrstate, 0, sizeof(hdrstate));
//    memset ((void *)&fstate), 0, sizeof(fstate));
//    memset ((void *)&(zoneinfo[zoneindex].state), 0, sizeof(zoneinfo[zoneindex].state));
    memset ((void *)&hdr, 0, sizeof(hdr));

    /* test the checking API */
    rc = WMAFileSetTargetMBRAudioStream(&hdrstate, nMBRTargetStream);
    if(rc != cWMA_NoErr)
    {
//        fprintf(stderr, "** Error while setting target stream number.\n");
        sprintf(logstr, "Error while setting target stream number for  %s", pInfo->sFilename);
//        return DECODE_DECODEERROR;
    }

    fp = fopen("/home/pmb/tout.wav","wb");

    printf("wma 2\n");
    rc = WMAFileIsWMA (&hdrstate);
    if(rc != cWMA_NoErr) 
    {
        sprintf(logstr, "File is not encrypted WMA: %s", pInfo->sFilename);
        apdLogPrint("PF52", APD_LOG_ERROR, logstr);
//        return DECODE_DECODEERROR;
    }
    rc = WMAFileDecodeCreate (&fstate);
    if(rc != cWMA_NoErr)
    {
        sprintf(logstr, "Decode Create failed");
        apdLogPrint("PF53", APD_LOG_ERROR, logstr);
//        return DECODE_DECODEERROR;
    }

        
    printf("wma 3\n");
    rc = WMAFileDecodeInit (fstate,1);
    if(rc != cWMA_NoErr) 
    {
        sprintf(logstr, "Unable to initialise WMA decoder");
        apdLogPrint("PF53", APD_LOG_ERROR, logstr);
//        return DECODE_DECODEERROR;
    }

    /* get header information */
    rc = WMAFileDecodeInfo (fstate, &hdr);
    if(rc != cWMA_NoErr) 
    {
        sprintf(logstr, "Unable to get WMA header info: %s", pInfo->sFilename);
        apdLogPrint("PF55", APD_LOG_ERROR, logstr);
        /* close WMA stuff */
        WMAFileDecodeClose(&fstate);
//        return DECODE_DECODEERROR;
    }

#ifdef NOT
    /* check to make sure we don't have DRM in the file - PF doesn't handle DRM yet */
    if(hdr.has_DRM) 
    {
        sprintf(logstr, "Encrytped WMA file has DRM - cannot handle: %s", pInfo->sFilename);
        apdLogPrint("PF57", APD_LOG_ERROR, logstr);
        /* close WMA stuff */
        WMAFileDecodeClose(&fstate);
        return DECODE_DECODEERROR;
    }

    /* allocate a WMA PCM pre-buffer */
    pInfo->wma.wmabuf = (unsigned char *) malloc(MADPCM_SAMPLES * 8);
    if (pInfo->wma.wmabuf == NULL) {
        apdLogPrint("PF58",APD_LOG_ERROR, "Cannot allocate WMA PCM prebuffer");
        /* close WMA stuff */
        WMAFileDecodeClose(&fstate);
        return DECODE_DECODEERROR;
    }
    pInfo->wma.wmabuf_bytes = 0;
    
#endif /*NOT*/
    /* now do the actual decoding */
    tWMA_U32 nCount = 0;
    while (TRUE) 
    {
        /* check to see if this thread needs to stop */
        if (pInfo->stopflag) {
            pInfo->stopflag = FALSE;
            code = 2;
            break;
        }
    
        rc = WMAFileDecodeData (fstate, &nSamples);
        if(rc != cWMA_NoErr) 
        {
            if ( rc == cWMA_NoMoreFrames || rc == cWMA_Failed ) {
                code = 0;       // normal exit
            } else {
                code = 1;       // error decoding data
            }
            break;
        }
    
        while (TRUE) 
        {
         

            tWMA_U32 num_samples;
            tWMA_U32 nRet;
            num_samples = WMAFileGetPCM (fstate, 
                             pLeft, pRight,sizeof(pLeft),MAX_SAMPLES,&pTime);

            if (num_samples == 0) 
            {
//                printf("decode break\n",num_samples);
                /* no more, so on with the decoding... */
                break;
            }
//            printf("Stuffing %d samples to playpfc_pcmdata\n",num_samples);
            nCount++;

//            printf("fwrite ret %d\n",nRet);

//              pBuf = (unsigned char *) pLeft;
              // interleave the buffers into a stereo output.
              for (i = 0; i < num_samples; i++)
              {
                  pOut[i*2] = pRight[i];
                  pOut[i*2 + 1] = pLeft[i];
              }

            nRet = fwrite(pOut,2,num_samples*2,fp);
              pcmSubmitBuffer(pCard, pOut, ((sizeof(short) * num_samples * hdr.num_channels))); 
//            playpfc_pcmdata(pInfo, pCard, (unsigned char *) pLeft, sizeof(short) * num_samples * hdr.num_channels); 
//              usleep(1000);
        }
    }

    printf("wma done a\n");

    /* make sure we flush any remaining WMA PCM data sitting in the prebuffer */
    playpfc_flushpcm(pInfo, pCard);
    
    /* close WMA stuff */
    WMAFileDecodeClose(&fstate);
        
    /* check return code of the decode to see how function should return */
    if (code == 0) 
    {
        return DECODE_FINISHED;
    } 
    else if (code == 1) 
    {
        return DECODE_DECODEERROR;
    } 
    else if (code == 2) 
    {
        return DECODE_STOP;
    } 
    else 
    {
        return DECODE_FINISHED;
    }
}
#endif // DO_WMA
