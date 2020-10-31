/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * giantPort_C.h - Generic giant definitions routines, used when
 *			 no platform-specific version is available.
 *
 *			 This module works on any size giantDigit up to and including 
 *			 the native unsigned int, as long as the platform does indeed
 *			 supply a (2 x giantDigit) unsigned integer, defined (in 
 *			 giantPlatform.h) as a giantDouble.
 *
 * Created Aug. 8 2005 by Doug Mitchell.
 */

#include <libGiants/giantTypes.h>
#include <libGiants/giantIntegers.h>
#include <libGiants/giantPortCommon.h>
#include <libGiants/giantDebug.h>

#ifndef	GI_GIANT_PORT_INLINE
#error Please define GI_GIANT_PORT_INLINE.
#endif

#if	!GI_GIANT_PORT_INLINE
/*
 * Add two digits, return sum. Carry bit returned as an out parameter.
 */
giantDigit giantAddDigits(
	giantDigit dig1,
	giantDigit dig2,
	giantDigit *carry)			/* RETURNED, 0 or 1 */
{
	giantDigit sum = dig1 + dig2;

	GI_CHECK_SP("giantAddDigits");
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
void giantAddDouble(
	giantDigit *accLow,			/* IN/OUT */
	giantDigit *accHigh,		/* IN/OUT */
	giantDigit val)
{
	giantDigit sumLo = *accLow + val;

	GI_CHECK_SP("giantAddDouble");
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
giantDigit giantSubDigits(
	giantDigit a,
	giantDigit b,
	giantDigit *borrow)			/* RETURNED, 0 or 1 */
{
	giantDigit diff = a - b;

	GI_CHECK_SP("giantSubDigits");
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
void giantMulDigits(
	giantDigit	dig1,
	giantDigit	dig2,
 	giantDigit	*lowProduct,		/* RETURNED, low digit */
	giantDigit	*hiProduct)		/* RETURNED, high digit */
{
	giantDouble dprod;

	GI_CHECK_SP("giantMulDigits");
	dprod = (giantDouble)dig1 * (giantDouble)dig2;
	*hiProduct = (giantDigit)(dprod >> GIANT_BITS_PER_DIGIT);
	*lowProduct = (giantDigit)dprod;
}

#if 0
/* this is currently unused. */
/*
 * Multiply a vector of giantDigits, candVector, by a single giantDigit,
 * plierDigit, adding results into prodVector. Returns m.s. digit from
 * final multiply; only candLength digits of *prodVector will be written.
 */
giantDigit VectorMultiply(
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
#endif	/* 0 - VectorMultiply disabled */

#endif	/* !GI_GIANT_PORT_INLINE */
