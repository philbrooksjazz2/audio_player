/* ***** BEGIN LICENSE BLOCK ***** 
 * Version: RCSL 1.0/RPSL 1.0 
 *  
 * Portions Copyright (c) 1995-2002 RealNetworks, Inc. All Rights Reserved. 
 *      
 * The contents of this file, and the files included with this file, are 
 * subject to the current version of the RealNetworks Public Source License 
 * Version 1.0 (the "RPSL") available at 
 * http://www.helixcommunity.org/content/rpsl unless you have licensed 
 * the file under the RealNetworks Community Source License Version 1.0 
 * (the "RCSL") available at http://www.helixcommunity.org/content/rcsl, 
 * in which case the RCSL will apply. You may also obtain the license terms 
 * directly from RealNetworks.  You may not use this file except in 
 * compliance with the RPSL or, if you have a valid RCSL with RealNetworks 
 * applicable to this file, the RCSL.  Please see the applicable RPSL or 
 * RCSL for the rights, obligations and limitations governing use of the 
 * contents of the file.  
 *  
 * This file is part of the Helix DNA Technology. RealNetworks is the 
 * developer of the Original Code and owns the copyrights in the portions 
 * it created. 
 *  
 * This file, and the files included with this file, is distributed and made 
 * available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER 
 * EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS ALL SUCH WARRANTIES, 
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS 
 * FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. 
 * 
 * Technology Compatibility Kit Test Suite(s) Location: 
 *    http://www.helixcommunity.org/content/tck 
 * 
 * Contributor(s): 
 *  
 * ***** END LICENSE BLOCK ***** */ 

/**************************************************************************************
 * Fixed-point MP3 decoder
 * Jon Recker (jrecker@real.com), Ken Cooke (kenc@real.com)
 * June 2003
 *
 * assembly.h - assembly language functions and prototypes for supported platforms
 *
 * - inline rountines with access to 64-bit multiply results 
 * - x86 (_WIN32) and ARM (ARM_ADS, _WIN32_WCE) versions included
 * - some inline functions are mix of asm and C for speed
 * - some functions are in native asm files, so only the prototype is given here
 *
 * MULSHIFT32(x, y)    signed multiply of two 32-bit integers (x and y), returns top 32 bits of 64-bit result
 * FASTABS(x)          branchless absolute value of signed integer x
 * CLZ(x)              count leading zeros in x
 * MADD64(sum, x, y)   (Windows only) sum [64-bit] += x [32-bit] * y [32-bit]
 * SHL64(sum, x, y)    (Windows only) 64-bit left shift using __int64
 * SAR64(sum, x, y)    (Windows only) 64-bit right shift using __int64
 */
/*
 * $Log: assembly.h,v $
 * Revision 1.2  2006/05/10 22:49:31  pbrooks
 * Fixed MULSHIFT32.
 *
 * Revision 1.1  2006/05/09 23:33:45  pbrooks
 * Added Helix fixed point codec to apd.
 *
 */
#ifndef _ASSEMBLY_H
#define _ASSEMBLY_H

#if defined(__GNUC__) && defined(__i386__)

static __inline int MULSHIFT32(int x, int y)	
{
    long long z;
    z = (long long)((long long)x * (long long)y);
    return (z >> 32);
}

/*
 * THE BELOW DOESN'T COMPILE ON gcc 4.0 !!
 */

//static __inline int MULSHIFT32(int x, int y)	
//{
//	int z;
//	__asm__ volatile ("imull %3" : "=d,d" (z), "+a,a" (x): "%1,1" (x), "m,r" (y)) ;
//	__asm__ ("imull %3" : "=d,d" (z), "+a,a" (x): "%1,1" (x), "m,r" (y)) ;
//	return z;
//}

static __inline int FASTABS(int x) 
{
	int sign;

	sign = x >> (sizeof(int) * 8 - 1);
	x ^= sign;
	x -= sign;

	return x;
}

static __inline int CLZ(int x)
{
	int numZeros;

	if (!x)
		return (sizeof(int) * 8);

	numZeros = 0;
	while (!(x & 0x80000000)) {
		numZeros++;
		x <<= 1;
	} 

	return numZeros;
}

/* MADD64, SHL64, SAR64:
 * write in assembly to avoid dependency on run-time lib for 64-bit shifts, muls
 *  (sometimes compiler thunks to function calls instead of code generating)
 * required for Symbian emulator
 */
typedef long long __int64; 
 
static __inline __int64 MADD64(__int64 sum, int x, int y)
{
	return (sum + ((__int64)x * y));
}

static __inline __int64 SHL64(__int64 x, int n)
{
	return x << n;
}

static __inline __int64 SAR64(__int64 x, int n)
{
	return x >> n;
}

#elif (defined _WIN32) && (defined _WIN32_WCE)

/* use asm function for now (EVC++ 3.0 does horrible job compiling __int64 version) */
#define MULSHIFT32	xmp3_MULSHIFT32
int MULSHIFT32(int x, int y);

static __inline int FASTABS(int x) 
{
	int sign;

	sign = x >> (sizeof(int) * 8 - 1);
	x ^= sign;
	x -= sign;

	return x;
}

static __inline int CLZ(int x)
{
	int numZeros;

	if (!x)
		return (sizeof(int) * 8);

	numZeros = 0;
	while (!(x & 0x80000000)) {
		numZeros++;
		x <<= 1;
	} 

	return numZeros;
}

#elif defined ARM_ADS

static __inline int MULSHIFT32(int x, int y)
{
    /* important rules for smull RdLo, RdHi, Rm, Rs:
     *     RdHi and Rm can't be the same register
     *     RdLo and Rm can't be the same register
     *     RdHi and RdLo can't be the same register
     * Note: Rs determines early termination (leading sign bits) so if you want to specify
     *   which operand is Rs, put it in the SECOND argument (y)
	 * For inline assembly, x and y are not assumed to be R0, R1 so it shouldn't matter
	 *   which one is returned. (If this were a function call, returning y (R1) would 
	 *   require an extra "mov r0, r1")
     */
    int zlow;
    __asm {
    	smull zlow,y,x,y
   	}

    return y;
}

static __inline int FASTABS(int x) 
{
	int t;

	__asm {
		eor	t, x, x, asr #31
		sub	t, t, x, asr #31
	}

	return t;
}

static __inline int CLZ(int x)
{
	int numZeros;

	if (!x)
		return (sizeof(int) * 8);

	numZeros = 0;
	while (!(x & 0x80000000)) {
		numZeros++;
		x <<= 1;
	} 

	return numZeros;
}

#elif defined(__GNUC__) && defined(__arm__)

static __inline int MULSHIFT32(int x, int y)
{
    /* important rules for smull RdLo, RdHi, Rm, Rs:
     *     RdHi and Rm can't be the same register
     *     RdLo and Rm can't be the same register
     *     RdHi and RdLo can't be the same register
     * Note: Rs determines early termination (leading sign bits) so if you want to specify
     *   which operand is Rs, put it in the SECOND argument (y)
	 * For inline assembly, x and y are not assumed to be R0, R1 so it shouldn't matter
	 *   which one is returned. (If this were a function call, returning y (R1) would 
	 *   require an extra "mov r0, r1")
     */
    int zlow;
    __asm__ volatile ("smull %0,%1,%2,%3" : "=&r" (zlow), "=r" (y) : "r" (x), "1" (y)) ;

    return y;
}

static __inline int FASTABS(int x) 
{
	int t;

	__asm__ volatile (
		"eor %0,%2,%2, asr #31;"
		"sub %0,%1,%2, asr #31;"
		: "=&r" (t) 
		: "0" (t), "r" (x)
	 );

	return t;
}

static __inline int CLZ(int x)
{
	int numZeros;

	if (!x)
		return (sizeof(int) * 8);

	numZeros = 0;
	while (!(x & 0x80000000)) {
		numZeros++;
		x <<= 1;
	} 

	return numZeros;
}

#else

#error Unsupported platform in assembly.h

#endif	/* platforms */

#endif /* _ASSEMBLY_H */
