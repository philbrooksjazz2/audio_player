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
int playpfc_wmadecode(int zoneindex) 
{

    /* WMA stuff */
    tWMAFileHdrState hdrstate;
    tWMAFileHeader hdr;
    tWMAFileStatus rc;
    int code = 0;
    char logstr[256] = "";
    short pLeft [MAX_SAMPLES * 2];
    short *pRight = NULL;
    
    /* initialise WMA stuff */
    memset ((void *)&hdrstate, 0, sizeof(hdrstate));
    memset ((void *)&(zoneinfo[zoneindex].state), 0, sizeof(zoneinfo[zoneindex].state));
    memset ((void *)&hdr, 0, sizeof(hdr));

    rc = WMAFileIsWMA (&hdrstate);
    if(rc != cWMA_NoErr) 
    {
        sprintf(logstr, "File is not encrypted WMA: %s", zoneinfo[zoneindex].filename);
        apdLogPrint("PF52", AEIU_LOG_ERROR, logstr);
        return PLAYPFC_DECODEERROR;
    }
        
    rc = WMAFileDecodeInit (&(zoneinfo[zoneindex].state));
    if(rc != cWMA_NoErr) 
    {
        sprintf(logstr, "Unable to initialise WMA decoder");
        apdLogPrint("PF53", AEIU_LOG_ERROR, logstr);
        return PLAYPFC_DECODEERROR;
    }

    /* get header information */
    rc = WMAFileDecodeInfo (zoneinfo[zoneindex].state, &hdr);
    if(rc != cWMA_NoErr) 
    {
        sprintf(logstr, "Unable to get WMA header info: %s", zoneinfo[zoneindex].filename);
        apdLogPrint("PF55", AEIU_LOG_ERROR, logstr);
        /* close WMA stuff */
        WMAFileDecodeClose(&(zoneinfo[zoneindex].state));
        return PLAYPFC_DECODEERROR;
    }

    /* check to make sure we don't have DRM in the file - PF doesn't handle DRM yet */
    if(hdr.has_DRM) 
    {
        sprintf(logstr, "Encrytped WMA file has DRM - cannot handle: %s", zoneinfo[zoneindex].filename);
        apdLogPrint("PF57", AEIU_LOG_ERROR, logstr);
        /* close WMA stuff */
        WMAFileDecodeClose(&(zoneinfo[zoneindex].state));
        return PLAYPFC_DECODEERROR;
    }

    /* allocate a WMA PCM pre-buffer */
    zoneinfo[zoneindex].wmabuf = (unsigned char *) malloc(pcmnumblocks * 8);
    if (zoneinfo[zoneindex].wmabuf == NULL) {
        apdLogPrint("PF58", AEIU_LOG_ERROR, "Cannot allocate WMA PCM prebuffer");
        /* close WMA stuff */
        WMAFileDecodeClose(&(zoneinfo[zoneindex].state));
        return PLAYPFC_DECODEERROR;
    }
    zoneinfo[zoneindex].wmabuf_bytes = 0;
    
    /* now do the actual decoding */
    while (TRUE) 
    {
        /* check to see if this thread needs to stop */
        if (zoneinfo[zoneindex].stopflag) {
            zoneinfo[zoneindex].stopflag = FALSE;
            code = 2;
            break;
        }
    
        rc = WMAFileDecodeData (zoneinfo[zoneindex].state);
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
            num_samples = WMAFileGetPCM (zoneinfo[zoneindex].state, 
                             pLeft, pRight, MAX_SAMPLES);

            if (num_samples == 0) 
            {
                /* no more, so on with the decoding... */
                break;
            }

            playpfc_pcmdata(zoneindex, (unsigned char *) pLeft, sizeof(short) * num_samples * hdr.num_channels); 
        }
    }

    /* make sure we flush any remaining WMA PCM data sitting in the prebuffer */
    playpfc_flushpcm(zoneindex);
    
    /* close WMA stuff */
    WMAFileDecodeClose(&(zoneinfo[zoneindex].state));
        
    /* check return code of the decode to see how function should return */
    if (code == 0) 
    {
        return PLAYPFC_FINISHED;
    } 
    else if (code == 1) 
    {
        return PLAYPFC_DECODEERROR;
    } 
    else if (code == 2) 
    {
        return PLAYPFC_STOP;
    } 
    else 
    {
        return PLAYPFC_FINISHED;
    }
}
