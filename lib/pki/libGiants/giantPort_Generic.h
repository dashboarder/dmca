/* Copyright (c) 1998,2006-2007 Apple Inc.  All Rights Reserved.
 *
 * NOTICE: USE OF THE MATERIALS ACCOMPANYING THIS NOTICE IS SUBJECT
 * TO THE TERMS OF THE SIGNED "FAST ELLIPTIC ENCRYPTION (FEE) REFERENCE
 * SOURCE CODE EVALUATION AGREEMENT" BETWEEN APPLE COMPUTER, INC. AND THE
 * ORIGINAL LICENSEE THAT OBTAINED THESE MATERIALS FROM APPLE COMPUTER,
 * INC.  ANY USE OF THESE MATERIALS NOT PERMITTED BY SUCH AGREEMENT WILL
 * EXPOSE YOU TO LIABILITY.
 ***************************************************************************
 *
 * giantPort_Generic.h - Generic giant definitions routines, implemented
 *			 as static inlines, used when no platform-specific version is available.
 *
 *			 This module works on any size giantDigit up to and including 
 *			 the native unsigned int, as long as the platform does indeed
 *			 supply a (2 x giantDigit) unsigned integer, defined (in 
 *			 giantPlatform.h) as a giantDouble.
 *
 *			 The module giantPort_C.c is functionaly identical to this
 *			 except that its routines are actual C functions instead of 
 *			 inlines. 
 *
 * Revision History
 * ----------------
 * 06 Apr 1998	Doug Mitchell at Apple
 *	Created.
 */

#ifndef	_GIANT_PORT_GENERIC_H_
#define _GIANT_PORT_GENERIC_H_

#include <libGiants/giantTypes.h>
#include <libGiants/giantIntegers.h>
#include <libGiants/giantDebug.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Add two digits, return sum. Carry bit returned as an out parameter.
 */
static GI_INLINE giantDigit giantAddDigits(
	giantDigit dig1,
	giantDigit dig2,
	giantDigit *carry)			/* RETURNED, 0 or 1 */
{
	giantDigit sum = dig1 + dig2;

	if((sum < dig1) || (sum < dig2)) {
	 	*carry = 1;
	}
	else {
		*carry = 0;
	}
	return sum;
}

/*
 * Add a single digit value to a double digit accumulator in place.
 * Carry out of the MSD of the accumulator is not handled.
 */
static GI_INLINE void giantAddDouble(
	giantDigit *accLow,			/* IN/OUT */
	giantDigit *accHigh,		/* IN/OUT */
	giantDigit val)
{
	giantDigit sumLo = *accLow + val;

	if((sumLo < *accLow) || (sumLo < val)) {
	    (*accHigh)++;
		/* if it's zero we just overflowed */
	    GIASSERT(*accHigh != 0);
	}
	*accLow = sumLo;
}

/*
 * Subtract a - b, return difference. Borrow bit returned as an out parameter.
 */
static GI_INLINE giantDigit giantSubDigits(
	giantDigit a,
	giantDigit b,
	giantDigit *borrow)			/* RETURNED, 0 or 1 */
{
	giantDigit diff = a - b;

	if(a < b) {
		*borrow = 1;
	}
	else {
		*borrow = 0;
	}
	return diff;
}

/*
 * Multiply two digits, return two digits.
 */
static GI_INLINE void giantMulDigits(
	giantDigit	dig1,
	giantDigit	dig2,
 	giantDigit	*lowProduct,		/* RETURNED, low digit */
	giantDigit	*hiProduct)		/* RETURNED, high digit */
{
	giantDouble dprod;

	dprod = (giantDouble)dig1 * (giantDouble)dig2;
	*hiProduct = (giantDigit)(dprod >> GIANT_BITS_PER_DIGIT);
	*lowProduct = (giantDigit)dprod;
}

#if 0
/* this is not currently used or needed */

/*
 * Multiply a vector of giantDigits, candVector, by a single giantDigit,
 * plierDigit, adding results into prodVector. Returns m.s. digit from
 * final multiply; only candLength digits of *prodVector will be written.
 */
static GI_INLINE giantDigit VectorMultiply(
	giantDigit plierDigit,
	giantDigit *candVector,
	gi_size	candLength,
	giantDigit *prodVector)
{
	gi_size candDex;		// index into multiplicandVector
	giantDigit lastCarry = 0;
	giantDigit prodLo;
	giantDigit prodHi;

	for(candDex=0; candDex<candLength; ++candDex) {
	    /*
	     * prod = *(candVector++) * plierDigit + *prodVector + lastCarry
	     */
	    giantMulDigits(*(candVector++),
		plierDigit,
		&prodLo,
		&prodHi);
	    giantAddDouble(&prodLo, &prodHi, *prodVector);
	    giantAddDouble(&prodLo, &prodHi, lastCarry);

	    /*
	     * *(destptr++) = prodHi;
	     * lastCarry = prodLo;
	     */
	    *(prodVector++) = prodLo;
	    lastCarry = prodHi;
	}

	return lastCarry;
}

#endif	

#ifdef __cplusplus
}
#endif

#endif	/*_GIANT_PORT_GENERIC_H_*/
