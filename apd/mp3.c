/*!
** \file mp3.c
** Author: Phil Brooks
**
** Description:
** mp3 decode
**
** Copyright (C) 2006 by Musicnet Inc.
*/
/*
 * $Log: mp3.c,v $
 * Revision 1.13  2007/09/05 23:39:12  pbrooks
 * Added alsa versions.
 *
 * Revision 1.12  2006/05/23 19:35:17  pbrooks
 * Added support for GetTime, State, GetBitrate, GetAudioParams, GetState,
 *        GetCurrent, SetCurrent, ClearCurrent.
 *
 * Revision 1.11  2006/05/09 21:38:37  pbrooks
 * Helix decoder milestone.
 *
 * Revision 1.10  2006/05/05 21:10:45  pbrooks
 * Fixed Stop bugs.
 *
 * Revision 1.9  2006/05/05 16:35:01  pbrooks
 * Now using 22050 sample rate on soundcard.
 *
 * Revision 1.8  2006/05/05 01:20:26  pbrooks
 * Optimization changes.
 *
 * Revision 1.7  2006/05/04 17:20:28  pbrooks
 * Fixed bug for mono content. Scale doesn't clip.
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
 * Revision 1.3  2006/05/02 17:15:14  pbrooks
 * Initial support for libmad.
 *
 * Revision 1.2  2006/04/29 00:42:11  pbrooks
 * Audio play milestone.
 *
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
#include "ampdll.h"
#include "apdpcm.h"
#include "apd.h"
#include "apdutil.h"

#include "../libmad/mad.h"
#include "../libmad/decoder.h"
#include "../libmad/fixed.h"
#include "../helix-mp3dec/mp3dec.h"
#include "../helix-mp3dec/helix_test/debug.h"
#include "../helix-mp3dec/helix_test/timing.h"

#define READBUF_SIZE		(1024*16)	/* feel free to change this, but keep big enough for >= one frame at high bitrates */
#define MAX_ARM_FRAMES		100
#define ARMULATE_MUL_FACT	1


ACard *pCardTmp;

//#ifndef IPOD
#ifdef DO_AMP

int apdMp3decode(ACard *pCard, SongInfo *pInfo) 
{

    AMPHEADER amph; 
    BOOL FINISHED, SETUP;
    int dstnumblocks;
    int srcnumblocks;
    int nFrameSize;
    UINT srcsize, destsize;
    char logstr[256] = "";

    srcsize = amp_default_srcsize() * BUFFER_SCALE;
    destsize = amp_default_destsize() * BUFFER_SCALE;
    srcnumblocks = srcsize / 8;
    dstnumblocks = destsize / 8;

    amph.id = amp_open_stream(NULL);
    amph.src = (BYTE *) malloc(sizeof(BYTE[srcsize]));
    amph.src_bytes_used = fread(amph.src,1, srcnumblocks, pInfo->fd);
    amph.dest = (BYTE *) malloc(sizeof(BYTE[destsize]));
    amph.play_mode = AMP_PM_NORMAL;
    amph.next = NULL;

    FINISHED = SETUP = FALSE;
    pInfo->finishtrack = FALSE;

    nFrameSize = srcsize;
    
    while (!FINISHED) 
    {
        
        /* check if we need to quit the thread */
        if (pInfo->stopflag) 
        {
            pInfo->stopflag = FALSE;
            amp_close_stream(amph.id);
            if (amph.src != NULL) 
            {
                free(amph.src);
            }
            free(amph.dest);
            return DECODE_STOP;
        }           

        /* check if we have to finish track */
        if (pInfo->finishtrack) 
        {
            pInfo->finishtrack = FALSE;
            amp_close_stream(amph.id);
            if (amph.src != NULL) 
            {
                free(amph.src);
            }
            free(amph.dest);
            return DECODE_FINISHED;
        }
        
        switch (amp_decode_buffer(&amph)) 
        {
            case AMP_SEX:
                amph.src_bytes_used =fread(amph.src,1, srcnumblocks, pInfo->fd);
                if (amph.src_bytes_used == 0) 
                {
                    free(amph.src);
                    amph.src = NULL;
                }
                break;
            case AMP_DEX:
                if (!pcmSubmitBuffer(pCard,amph.dest,nFrameSize)) 
                {
                    sprintf(logstr, "Error writing PCM buffer");
                    apdLogPrint("PF29", APD_LOG_ERROR, logstr);
                    amp_close_stream(amph.id);
                    if (amph.src != NULL) 
                    {
                        free(amph.src);
                    }
                    free(amph.dest);
                    return DECODE_DECODEERROR;
                }
                break;
            case AMP_AUD:
                /* This is intentionally simplified - audio is setup once only */
                /* Normally, you could reset it every time AMP_AUD is returned */
                if (SETUP == TRUE) 
                    break;
                else
                    SETUP = FALSE;
                break;
            case AMP_FIN:
                /* pad the rest of the buffer with 0s */
                if (amph.dest != NULL) {
                    memset(amph.dest+amph.dest_bytes_used, 0, destsize-amph.dest_bytes_used);
                    /* write to PCM stream */
                    if (!pcmSubmitBuffer(pCard,amph.dest,nFrameSize)) 
                    {
                        sprintf(logstr, "Error writing PCM buffer");
                        apdLogPrint("PF30", APD_LOG_ERROR, logstr);                  
                        amp_close_stream(amph.id);
                        if (amph.src != NULL) 
                        {
                            free(amph.src);
                        }
                        free(amph.dest);
                        return DECODE_DECODEERROR;
                    }
                }
                FINISHED = TRUE;
                break;
        }
    }
    amp_close_stream(amph.id);
    if (amph.src != NULL) 
    {
        free(amph.src);
    }
    free(amph.dest);
    return DECODE_FINISHED;
}
#endif // DO_AMP

static int FillReadBuffer(unsigned char *readBuf, unsigned char *readPtr, int bufSize, int bytesLeft, FILE *infile)
{
	int nRead;

	/* move last, small chunk from end of buffer to start, then fill with new data */
	memmove(readBuf, readPtr, bytesLeft);				
	nRead = fread(readBuf + bytesLeft, 1, bufSize - bytesLeft, infile);
	/* zero-pad to avoid finding false sync word after last frame (from old data in readBuf) */
	if (nRead < bufSize - bytesLeft)
		memset(readBuf + bytesLeft + nRead, 0, bufSize - bytesLeft - nRead);	

	return nRead;
}

/* 
 * Get mp3 frame data, samplerate, nChans, also calcs total seconds. 
 */
int apdHelixGetMp3FrameData(ACard *pCard, SongInfo *pInfo) 
{
	int bytesLeft, nRead, err, offset, outOfData, eofReached;
	unsigned char readBuf[READBUF_SIZE], *readPtr;
	unsigned char sBuf[MAX_NCHAN * MAX_NGRAN * MAX_NSAMP * 2];
	short outBuf[MAX_NCHAN * MAX_NGRAN * MAX_NSAMP];
	FILE *infile, *outfile;
	MP3FrameInfo mp3FrameInfo;
    int nTotalBytes;
	HMP3Decoder hMP3Decoder;
	int startTime, endTime, diffTime, totalDecTime, nFrames;
    int nSamples;
    int nHalfSample;
    int i,j;
    struct stat pbuf;
#ifdef ARM_ADS	
	float audioSecs;
#endif
    nHalfSample = TRUE; 
    nSamples = MAX_NCHAN * MAX_NGRAN * MAX_NSAMP;

	infile = fopen(pInfo->sFilename, "rb");
	if (!infile) 
    {
		printf("file open error\n");
		return -1;
	}

    fstat(fileno(infile), &pbuf);

    nTotalBytes = pbuf.st_size;

	if ( (hMP3Decoder = MP3InitDecoder()) == 0 )
		return -2;

	bytesLeft = 0;
	outOfData = 0;
	eofReached = 0;
	readPtr = readBuf;
	nRead = 0;
	totalDecTime = 0;
	nFrames = 0;
	if ( infile )
    {

		/* somewhat arbitrary trigger to refill buffer - should always be enough for a full frame */
		if (bytesLeft < 2*MAINBUF_SIZE && !eofReached) {
			nRead = FillReadBuffer(readBuf, readPtr, READBUF_SIZE, bytesLeft, infile);
			bytesLeft += nRead;
			readPtr = readBuf;
			if (nRead == 0)
				eofReached = 1;
		}

		/* find start of next MP3 frame - assume EOF if no sync found */
		offset = MP3FindSyncWord(readPtr, bytesLeft);
		readPtr += offset;
		bytesLeft -= offset;


		/* decode one MP3 frame - if offset < 0 then bytesLeft was less than a full frame */
 		err = MP3Decode(hMP3Decoder, &readPtr, &bytesLeft, outBuf, 0);
 		nFrames++;

        startTime = sizeof(short);
		MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);
         j = 0;

        pInfo->bitrate = mp3FrameInfo.bitrate;
        pInfo->channels = mp3FrameInfo.nChans;
        pInfo->samplerate = mp3FrameInfo.samprate;
        

        pInfo->total_sec = (nTotalBytes /(pInfo->bitrate / 8));
 		
	} 


#ifdef ARM_ADS	
	MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);
	audioSecs = ((float)nFrames * mp3FrameInfo.outputSamps) / ( (float)mp3FrameInfo.samprate * mp3FrameInfo.nChans);
	printf("\nTotal clock ticks = %d, MHz usage = %.2f\n", totalDecTime, ARMULATE_MUL_FACT * (1.0f / audioSecs) * totalDecTime * GetClockDivFactor() / 1e6f);
	printf("nFrames = %d, output samps = %d, sampRate = %d, nChans = %d\n", nFrames, mp3FrameInfo.outputSamps, mp3FrameInfo.samprate, mp3FrameInfo.nChans);
#endif

	MP3FreeDecoder(hMP3Decoder);

	fclose(infile);

	return DECODE_FINISHED;
}
int apdHelixDecode(ACard *pCard, SongInfo *pInfo) 
{
	int bytesLeft, nRead, err, offset, outOfData, eofReached;
	unsigned char readBuf[READBUF_SIZE], *readPtr;
	unsigned char sBuf[MAX_NCHAN * MAX_NGRAN * MAX_NSAMP * 2];
	short outBuf[MAX_NCHAN * MAX_NGRAN * MAX_NSAMP];
	FILE *infile, *outfile;
	MP3FrameInfo mp3FrameInfo;
    int nTotalBytes;
	HMP3Decoder hMP3Decoder;
	int startTime, endTime, diffTime, totalDecTime, nFrames;
    int nSamples;
    int nHalfSample;
    int i,j;
#ifdef ARM_ADS	
	float audioSecs;
#endif
    nHalfSample = TRUE; 
    nSamples = MAX_NCHAN * MAX_NGRAN * MAX_NSAMP;

	infile = fopen(pInfo->sFilename, "rb");
	if (!infile) 
    {
		printf("file open error\n");
		return -1;
	}
		
//    outfile = fopen("/tmp/apd_debug_out","wb");

#ifdef TIMER_TESTS 
	InitTimer();
#endif // ARM
	

	if ( (hMP3Decoder = MP3InitDecoder()) == 0 )
		return -2;

//	DebugMemCheckEndPoint();

//        apdLogPrint("XXX01", APD_LOG_INFO, "Helix dbg 1");

	bytesLeft = 0;
	outOfData = 0;
	eofReached = 0;
	readPtr = readBuf;
	nRead = 0;
	totalDecTime = 0;
	nFrames = 0;
	do {
          /* check if we need to quit the thread */
          if (pInfo->stopflag) 
          {
//             pInfo->stopflag = FALSE;
               outOfData = 1;;
               break;
          }           

		/* somewhat arbitrary trigger to refill buffer - should always be enough for a full frame */
		if (bytesLeft < 2*MAINBUF_SIZE && !eofReached) {
			nRead = FillReadBuffer(readBuf, readPtr, READBUF_SIZE, bytesLeft, infile);
			bytesLeft += nRead;
			readPtr = readBuf;
			if (nRead == 0)
				eofReached = 1;
		}

		/* find start of next MP3 frame - assume EOF if no sync found */
		offset = MP3FindSyncWord(readPtr, bytesLeft);
		if (offset < 0) {
			outOfData = 1;
			break;
		}
		readPtr += offset;
		bytesLeft -= offset;


		/* decode one MP3 frame - if offset < 0 then bytesLeft was less than a full frame */
#ifdef ARM
//		startTime = ReadTimer();
#endif
 		err = MP3Decode(hMP3Decoder, &readPtr, &bytesLeft, outBuf, 0);
 		nFrames++;

        startTime = sizeof(short);
		MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);
//        nSamples = mp3FrameInfo.Samps;
         j = 0;
        for( i = 0; i < nSamples; i++)
        {

            short sample;
            sample = outBuf[i];
            if ( mp3FrameInfo.nChans == 1 )
            {
                if ( nHalfSample )
                {
                    if (!(i%4))
                    {
                        sample = outBuf[i/2];
                        sBuf[(j*4)] = ((sample >> 0) & 0xff);
                        sBuf[(j*4)+1] = ((sample >> 8) & 0xff);
                        sBuf[(j*4)+2] = ((sample >> 0) & 0xff);
                        sBuf[(j*4)+3] = ((sample >> 8) & 0xff);
                        j++;
                    }
                }
                else
                {
                    if (!(i%2))
                    {
                        sample = outBuf[j];
                        sBuf[(j*4)] = ((sample >> 0) & 0xff);
                        sBuf[(j*4)+1] = ((sample >> 8) & 0xff);
                        sBuf[(j*4)+2] = ((sample >> 0) & 0xff);
                        sBuf[(j*4)+3] = ((sample >> 8) & 0xff);
                        j++;
                    }
                }
            }
            else
            {
                if ( nHalfSample )
                {
                    if (!(i%2))
                    {
                        sample = outBuf[i];
                        sBuf[(j*2)] = ((sample >> 0) & 0xff);
                        sBuf[(j*2)+1] = ((sample >> 8) & 0xff);
                        j++;
                    }
                }
                else
                {
                    sample = outBuf[i];
                    sBuf[(i*2)] = ((sample >> 0) & 0xff);
                    sBuf[(i*2)+1] = ((sample >> 8) & 0xff);
                }
            }

        }

//        apdLogPrint("XXX02", APD_LOG_INFO, "Helix dbg 2");
        pcmSubmitBuffer(pCard,sBuf,nSamples*2); 
//        pcmSubmitBuffer(pCard,outBuf); 
 		
#ifdef TIMER_TESTS
 		endTime = ReadTimer();
 		diffTime = CalcTimeDifference(startTime, endTime);
#endif // TIMER_TESTS
		totalDecTime += diffTime;

#ifdef TIMER_TESTS 	
		printf("frame %5d  start = %10d, end = %10d elapsed = %10d ticks\r", 
			nFrames, startTime, endTime, diffTime);
		fflush(stdout);
#endif

		if (err) {
			/* error occurred */
			switch (err) {
			case ERR_MP3_INDATA_UNDERFLOW:
				outOfData = 1;
				break;
			case ERR_MP3_MAINDATA_UNDERFLOW:
				/* do nothing - next call to decode will provide more mainData */
				break;
			case ERR_MP3_FREE_BITRATE_SYNC:
			default:
				outOfData = 1;
				break;
			}
		} else {
			/* no error */
			if (outfile)
				fwrite(outBuf, mp3FrameInfo.bitsPerSample / 8, mp3FrameInfo.outputSamps, outfile);
		}

#if defined ARM_ADS && defined MAX_ARM_FRAMES
		if (nFrames >= MAX_ARM_FRAMES)
			break;
#endif
	} while (!outOfData);


#ifdef ARM_ADS	
	MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);
	audioSecs = ((float)nFrames * mp3FrameInfo.outputSamps) / ( (float)mp3FrameInfo.samprate * mp3FrameInfo.nChans);
	printf("\nTotal clock ticks = %d, MHz usage = %.2f\n", totalDecTime, ARMULATE_MUL_FACT * (1.0f / audioSecs) * totalDecTime * GetClockDivFactor() / 1e6f);
	printf("nFrames = %d, output samps = %d, sampRate = %d, nChans = %d\n", nFrames, mp3FrameInfo.outputSamps, mp3FrameInfo.samprate, mp3FrameInfo.nChans);
#endif

	MP3FreeDecoder(hMP3Decoder);

	fclose(infile);
#ifdef ARM 
//	FreeTimer();
#endif
//	DebugMemCheckFree();

	return DECODE_FINISHED;
}
#ifdef DO_MAD
int apdMadDecode(ACard *pCard, SongInfo *pInfo) 
{

    BOOL FINISHED, SETUP;
    UINT srcsize, destsize;
    char logstr[256] = "";
    int fd1;
    struct stat stat;
    void *fdm;
    int nTotal;
    int nProcessed;
    int nChunk;
    int nRemaining;
    int nCount = 0;

    FINISHED = SETUP = FALSE;
    pInfo->finishtrack = FALSE;

    fd1 = open(pInfo->sFilename,O_RDONLY);

    nChunk = BUF_SIZE;

    if (fstat(fd1, &stat) == -1 ||
        stat.st_size == 0)
    {
        sprintf(logstr,"Mad decode failed to open file %s.",pInfo->sFilename);
        apdLogPrint("APD29", APD_LOG_ERROR, logstr);
        return 2;
    }

    nTotal = stat.st_size;
    nRemaining = nTotal;
    nProcessed = 0;

    
    while ( nRemaining ) 
    {
        
        /* check if we need to quit the thread */
        if (pInfo->stopflag) 
        {
//            pInfo->stopflag = FALSE;
            nRemaining = 0;
            return APD_STOP;
        }           

        if (!(nCount%2))
            printf("Decode loop %d\n",nCount);

         nCount++;
        /* check if we have to finish track */
        if (pInfo->finishtrack) 
        {
            pInfo->finishtrack = FALSE;
            return DECODE_FINISHED;
        }

//        if ( nRemaining < nChunk )
//        {
//            nChunk = nRemaining; 
//        }
//        nRemaining -= nChunk;
        
        fdm = mmap(0, nRemaining, PROT_READ, MAP_SHARED, fd1, 0);
        if (fdm == MAP_FAILED)
        {
            printf("minimad mmap failed!\n");
            return 3;
        }

//        decode(fdm, nChunk, pCard, pInfo);
        decode(fdm, nRemaining, pCard, pInfo);

        nProcessed += nChunk;
        
//        munmap(fdm, nChunk);
        munmap(fdm, nRemaining);

        nRemaining = 0;

 /* pad the rest of the buffer with 0s */
//  memset(amph.dest+amph.dest_bytes_used, 0, destsize-amph.dest_bytes_used);
    }
    printf("MadDecode returning DECODE_FINISHED\n");
    return DECODE_FINISHED;
}
/*
 * This is perhaps the simplest example use of the MAD high-level API.
 * Standard input is mapped into memory via mmap(), then the high-level API
 * is invoked with three callbacks: input, output, and error. The output
 * callback converts MAD's high-resolution PCM samples to 16 bits, then
 * writes them to standard output in little-endian, stereo-interleaved
 * format.
 */

int decode(unsigned char const *, unsigned long, ACard *pCard);

/*
 * This is a private message structure. A generic pointer to this structure
 * is passed to each of the callback functions. Put here any data you need
 * to access from within the callbacks.
 */

struct buffer {
  unsigned char const *start;
  unsigned long length;
};

/*
 * This is the input callback. The purpose of this callback is to (re)fill
 * the stream buffer which is to be decoded. In this example, an entire file
 * has been mapped into memory, so we just call mad_stream_buffer() with the
 * address and length of the mapping. When this callback is called a second
 * time, we are finished decoding.
 */

static
enum mad_flow input(void *data,
		    struct mad_stream *stream)
{
    struct buffer *buffer = data;

    if (!buffer->length)
        return MAD_FLOW_STOP;

    mad_stream_buffer(stream, buffer->start, buffer->length);

    buffer->length = 0;

    return MAD_FLOW_CONTINUE;
}

/*
 * The following utility routine performs simple rounding, clipping, and
 * scaling of MAD's high-resolution samples down to 16 bits. It does not
 * perform any dithering or noise shaping, which would be recommended to
 * obtain any exceptional audio quality. It is therefore not recommended to
 * use this routine if high-quality output is desired.
 */

static inline
signed int scale(mad_fixed_t sample)
{
   signed int r1;
   int r2;

  /* round */
  r1 = (1L << (MAD_F_FRACBITS - 16));
  sample += r1;

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  r2 =  (MAD_F_FRACBITS + 1 - 16);
//  return sample >> (MAD_F_FRACBITS + 1 - 16);
  sample =  (sample >> r2);
  return sample;
}

/*
 * This is the output callback function. It is called after each frame of
 * MPEG audio data has been completely decoded. The purpose of this callback
 * is to output (or play) the decoded PCM audio.
 */

static
enum mad_flow output(void *data,
		     struct mad_header const *header,
		     struct mad_pcm *pcm)
{
    unsigned int nChannels, nSamples;
    mad_fixed_t const *left_ch, *right_ch;
    int i;
    signed int r1;
    static int nCount = 0;
    unsigned char pbuf[MADPCM_SAMPLES*PCM_FRAMESIZE];
//    signed int pbuf[2400];

    /* pcm->samplerate contains the sampling frequency */

    nChannels = pcm->channels;
    nSamples  = pcm->length;
    left_ch   = pcm->samples[0];
    right_ch  = pcm->samples[1];
    r1 = (1L << (MAD_F_FRACBITS - 16));

//    if (!(nCount % 100))
//        printf("output %d\n",nCount/100);
//    nCount++;

    for(i = 0; i < (nSamples/2); i++)
    { 
        signed int sample;

        /* output sample(s) in 16-bit signed little-endian PCM */

//        sample = scale(*(left_ch+=2));
        sample = (*(left_ch+(i*2)) + r1);
        sample = (sample >> 13);
        pbuf[i*4] = ((sample >> 0) & 0xff);
        pbuf[(i*4)+1] = ((sample >> 8) & 0xff);

        if ( nChannels == 2 )
        {
            sample = (*(right_ch+(i*2)) + r1);
            sample = (sample >> 13);
            pbuf[(i*4)+2] = ((sample >> 0) & 0xff);
            pbuf[(i*4)+3] = ((sample >> 8) & 0xff);
        }
        else
        {
            pbuf[(i*4)+2] = pbuf[i*4];
            pbuf[(i*4)+3] = pbuf[(i*4)+1];
        }

    }
    pcmSubmitBuffer(pCardTmp,pbuf,nSamples*2); 

    if ( pcmGetThreadFlag() )
    {
       return MAD_FLOW_STOP;
    }

    return MAD_FLOW_CONTINUE;
}

/*
 * This is the error callback function. It is called whenever a decoding
 * error occurs. The error is indicated by stream->error; the list of
 * possible MAD_ERROR_* errors can be found in the mad.h (or stream.h)
 * header file.
 */

static enum mad_flow error(void *data,
		    struct mad_stream *stream,
		    struct mad_frame *frame)
{
  struct buffer *buffer = data;
  char logstr[128];

  fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n",
	  stream->error, mad_stream_errorstr(stream),
	  stream->this_frame - buffer->start);

  /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

  return MAD_FLOW_CONTINUE;
}

/*
 * This is the function called by main() above to perform all the decoding.
 * It instantiates a decoder object and configures it with the input,
 * output, and error callback functions above. A single call to
 * mad_decoder_run() continues until a callback function returns
 * MAD_FLOW_STOP (to stop decoding) or MAD_FLOW_BREAK (to stop decoding and
 * signal an error).
 */


int decode(unsigned char const *start, unsigned long length, ACard *pCard)
{
    struct buffer buffer;
    struct mad_decoder decoder;
    int result;

    /* initialize our private message structure */

    //  printf("Starting minimad decode.\n");

    buffer.start  = start;
    buffer.length = length;

    // pass the cheese please
    pCardTmp = pCard;

    /* configure input, output, and error functions */

    mad_decoder_init(&decoder, &buffer,
		     input, 0 /* header */, 0 /* filter */, output,
		     error, 0 /* message */);

  /* start decoding */

  result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

  /* release the decoder */

  mad_decoder_finish(&decoder);

  return result;
}
#endif // DO_MAD
