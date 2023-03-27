/*!
** \file apdutil.c
** Author: Phil Brooks
**
** Description:
** Utility module.  Date and time, logging functions, etc.
**
** Copyright (C) 2006 Musicnet, Inc.
*/
/*
 * $Log: apdutil.c,v $
 * Revision 1.1.1.1  2006/04/21 16:30:29  pbrooks
 * Initial checkin of embedded player files.
 *
 *
 */
#define _APDUTIL_C

#include <time.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _LINUX
#include <dirent.h>
#include <sys/types.h>
#endif // _LINUX
#include "apdutil.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define SUCCESS TRUE
#define FAILURE FALSE

int apdSetLogDate() 
{
    /* set the stored log date */
    apdGetDate(&gDate);
    return TRUE;
}
/* logging functions */
int apdLogSetFile(char *sLogfile) 
{
    /* set log filename for process */
    if (strlen(sLogfile) > 0) 
    {
        strcpy(gLogFile, sLogfile);
        return TRUE;
    } 
    else 
    {
        return FALSE;
    }
}

int apdLogPrint(char *logcode, int severity, char *logmessage) 
        
{

    FILE *rfd = NULL;
    FILE *wfd = NULL;
    FILE *tfd = NULL;
    char severity_ch;
    char sStr[256];
    int fpos = 0;
    char ch = ' ';
    char buffer[1024];
    int numbytes = 0;
    char tmp_sLogfile[256];
    
    sprintf(tmp_sLogfile, "%s.tmp", gLogFile);
    
    if (severity == APD_LOG_INFO) 
    {
        severity_ch = 'I';
    } 
    else if (severity == APD_LOG_WARNING) 
    {
        severity_ch = 'W';
    } 
    else if (severity == APD_LOG_ERROR) 
    {
        severity_ch = 'E';
    } 
    sprintf(sStr,"%s", logmessage);
    if (sStr[strlen(sStr)-1] == '\n') 
    { 
        /* remove possible newline character */
        sStr[strlen(sStr)-1] = '\0';
    }
    /* open and append */
    wfd = fopen(gLogFile, "a");
    if (wfd) 
    {
        /* append message */
        apdGetDate(&gDate);
        fprintf(wfd, "%c %s %04d/%02d/%02d %02d:%02d:%02d %s\n",
            severity_ch, logcode, gDate.year, gDate.month, gDate.dayinmonth,
            gDate.hours, gDate.minutes, gDate.seconds, sStr);
    
        /* do we need to trim the logfile? */

        fpos = ftell(wfd);
        if (fpos > APD_MAXLOGSIZE) 
        {
            /* create new file of size approx APD_TRIMLOGSIZE */
            tfd = fopen(tmp_sLogfile, "wb");
            if (tfd) 
            {
                fclose(wfd);
                wfd = fopen(gLogFile, "rb");
                if (wfd) 
                {
                    /* seek to a position in the file */
                    fseek(wfd, fpos - APD_TRIMLOGSIZE, SEEK_SET);
                    while (ch != '\n') 
                    {
                        /* read to \n */
                        fread(&ch, 1, 1, wfd); 
                    }   
                    /* copy data */
                    while((numbytes=fread((void *)&(buffer[0]),1,1024,wfd))>0) 
                    {
                        (int)fwrite((void *)&(buffer[0]),1,numbytes,tfd); 
                    } 
                }
            }
        
            /* rename the temporary file to the actual filename */
            rename(tmp_sLogfile, gLogFile);
        }
        /* close the files */
        if ( tfd )
            fclose(tfd);
        if ( wfd )
            fclose(wfd);
    }
    return TRUE;
}

int apdSongLogSetFile(char *sLogfile) 
{
    /* set log filename for process */
    if (strlen(sLogfile) > 0) 
    {
        strcpy(gSongLogFile, sLogfile);
        return TRUE;
    } 
    else 
    {
        return FALSE;
    }
}

/* gets a date for a time which is seconds seconds into the future */
int apdGetDate(apd_date_ptr pDate) 
{
    time_t currenttime;
    struct tm *tm_ptr = NULL;
    
    /* get the time */
    time(&currenttime);
    
    tm_ptr = localtime(&currenttime);
    
    /* check that the year is at least 100 (2000) */
    if (tm_ptr->tm_year < 100) 
    {
        /* just set the year to 2000 */
        pDate->year = 2000;
    } 
    else 
    {
        pDate->year = tm_ptr->tm_year + 1900; 
    }
    
    /* fill in the rest of the data structure */
    pDate->month = tm_ptr->tm_mon + 1;
    pDate->dayinmonth = tm_ptr->tm_mday;
    pDate->dayinweek = tm_ptr->tm_wday;
    pDate->hours = tm_ptr->tm_hour;
    pDate->minutes = tm_ptr->tm_min;
    pDate->seconds = tm_ptr->tm_sec;
    pDate->abstime = currenttime;
    pDate->abshours = currenttime / 60 / 60;

    return SUCCESS;
}

/* parse the standard YYYYMMDD date into a date structure */
int apdDateParse(apd_date_ptr pDate, char *sDatestr) {

    char sTmp[9];
    int nVal = 0;
    int nRet;

    nRet = FALSE;

    if (strlen(sDatestr) == 8) 
    {
    
        /* blank the date structure */
        pDate->year = 2000;
        pDate->month = 1;
        pDate->dayinmonth = 1;
        pDate->dayinweek = 1;
        pDate->hours = 0;
        pDate->minutes = 0;
        pDate->seconds = 0;
        pDate->abstime = 0;
        pDate->abshours = 0;

        /* grab the year out of the string */
        strncpy(sTmp, sDatestr, 4);
        sTmp[4] = '\0';
        nVal = atoi(sTmp);
        pDate->year = nVal;
    
        /* grab the month out of the string */
        strncpy(sTmp, &(sDatestr[4]), 2);
        sTmp[2] = '\0';
        nVal = atoi(sTmp);
        pDate->month = nVal;
    
        /* grab the day in the month out of the string */
        strncpy(sTmp, &(sDatestr[6]), 2);
        sTmp[2] = '\0';
        nVal = atoi(sTmp);
        pDate->dayinmonth = nVal;
  
        nRet = SUCCESS;
    }

    return nRet;
}

