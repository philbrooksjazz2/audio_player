/*!
** \file apdutil.h
** Author: Phil Brooks
**
** Description:
** apd utility functions, logging, etc.
**
** Copyright (C) 2006 Musicnet, Inc.
*/
/*
 * $Log: apdutil.h,v $
 * Revision 1.1.1.1  2006/04/21 16:30:29  pbrooks
 * Initial checkin of embedded player files.
 *
 *
 */
/* UTILITY module */
#ifndef _APDUTIL_H
#define _APDUTIL_H

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


#define APD_MAXLOGSIZE 1024*1024	/*one meg maximum size for a log file */
#define APD_TRIMLOGSIZE 800*1024	/* when too big - trim to approx 800K */

#define APD_LOG_INFO 		0
#define APD_LOG_WARNING 	1
#define APD_LOG_ERROR		2

/* logging functions for apd*/
int apdLogSetFile(char *sLogfile); 	
int apdLogPrint(char *sLogcode, int nSeverity,char *sLogmessage);
int apdSongLogSetFile(char *logfile); 	
int apdSongLogPrint(char *logmessage);
int apdSetLogDate();	

/* generic logging function */
int apdFileLogPrint(char *filename,char *logcode,int severity,char *logmessage);
		
/* general utility functions that dont really belong anywhere else */

typedef struct {

	unsigned int year; 		/* 2000+*/
	unsigned char month;		/* 1-12 */
	unsigned char dayinmonth;	/* 1-31 */
	unsigned char dayinweek;	/* 0-6 (Sunday through Monday) */
	unsigned char hours;		/* 0-23 */
	unsigned char minutes;		/* 0-59 */
	unsigned char seconds;		/* 0-59 */
	
	unsigned int abstime;		/* O/S absolute seconds */
	unsigned int abshours; 		/* abstime / 60 / 60 */
	
} apddate, *apd_date_ptr;

/* date functions */
int apdGetDate(apd_date_ptr date);
int apdDateParse(apd_date_ptr date,  char *datestr);

#ifdef _APDUTIL_C

/* globals */
static apddate gDate;
static char gLogFile[256];
static char gSongLogFile[256];


#endif // _APD_C

#endif // _APDUTIL_H
