/*!
** \file ampmp3.h
** Author: 
**
** Description:
** 
**
** Copyright (C) 2006 Musicnet Inc.
*/
/*
 * $Log: mp3.h,v $
 * Revision 1.1.1.1  2006/04/21 16:30:29  pbrooks
 * Initial checkin of embedded player files.
 *
 *
 */
// This file is a part of the amp MPEG audio decoder
// Copyright (C) 1996-1998 Tomislav Uzelac
// All rights reserved.

#ifndef AMPMP3_H
#define AMPMP3_H

#if defined(WIN32)
#include "windows.h"
#endif

#if defined(LINUX)
#define	UINT	unsigned int
#define BYTE	unsigned char
#define BOOL	int
#define TRUE	1
#define FALSE	0
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/* struct
*/
typedef struct {
	UINT	sample_index;
	UINT	byte_index;
	UINT	_offset;
} SRC_INDEX;

typedef struct {
	UINT	id;

	BYTE	*src;
	UINT	src_bytes_used;
	BYTE	*dest;
	UINT	dest_bytes_used;

	UINT	play_mode;

	SRC_INDEX	current;
	SRC_INDEX	*next;

	UINT	dest_index;

	UINT    seek_to_sample;

} AMPHEADER;

typedef struct {
	UINT 	paramID;
	UINT	paramValue;
	void	*paramXt;
} AMPPARAM;

typedef struct {
	int		data[2][10];
	float	base;
	int		boost;
	float	decay;
} ANALIZER_STRUCT;

/* prototypes
*/
int amp_open_stream(AMPPARAM *paramlist);
int amp_close_stream(int id);
int amp_decode_buffer(AMPHEADER* hdr);

UINT amp_default_srcsize(void);
UINT amp_default_destsize(void);
UINT amp_msec_to_samples(int id, int msec);

int amp_supports_parameter(UINT paramID);
int amp_set_parameter(int id,AMPPARAM *param);
int amp_get_parameter(int id,AMPPARAM *param);

UINT amp_version_info(void);

/* error codes
*/
#define AMP_ERR_GENERAL	-1
#define AMP_ERR_NOMEM	-2
#define AMP_ERR_INVARG	-3

/* status notifications
*/
#define AMP_SEX		0x1
#define AMP_DEX		0x2
#define AMP_AUD		0x3
#define AMP_FIN		0x4
#define AMP_FSE		0x5

/* play modes
*/
#define AMP_PM_NORMAL	0x1
#define AMP_PM_SEEK		0x2

/* parameters
*/
#define AMP_PS_SRCSIZE		0x1
#define AMP_PS_DESTSIZE		0x2
#define AMP_PS_PRECISION	0x3
#define AMP_PS_UNSIGNED		0x4
#define AMP_PS_INVENDIAN	0x5
#define AMP_PS_DOWNMIX		0x6
#define AMP_PS_PRIORITY		0x7

#define AMP_PD_DOWNMIX2		0x11
#define AMP_PD_INVSTEREO	0x12
#define	AMP_PD_SAMPLERATE	0x13
#define	AMP_PD_CHANNELS		0x14
#define AMP_PD_BITRATE		0x15
#define AMP_PD_LAYER		0x16
#define AMP_PD_FRAMENO		0x17
#define AMP_PD_EQU			0x18
#define AMP_PD_PREPLAY		0x19
#define AMP_PD_PREPLAY_M1	0x1a
#define AMP_PD_PREPLAY_M2	0x1b
#define AMP_PD_TEMPO		0x1c
#define AMP_PD_TEMPO_CHG	0x1d
#define AMP_PD_GAIN			0x1e
#define AMP_PD_TEMPO_CHGABS	0x1f
#define	AMP_PD_DEST_OFFSET	0x20
#define	AMP_PD_TEMPOF		0x21
#define	AMP_PD_ANALIZER		0x22

#if defined(__cplusplus)
}
#endif

#endif // AMPMP3_H
