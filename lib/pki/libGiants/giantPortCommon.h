/* Copyright (c) 1998,2006-2007 Apple Inc.  All Rights Reserved.
 *
 * NOTICE: USE OF THE MATERIALS ACCOMPANYING THIS NOTICE IS SUBJECT
 * TO THE TERMS OF THE SIGNED "FAST ELLIPTIC ENCRYPTION (FEE) REFERENCE
 * SOURCE CODE EVALUATION AGREEMENT" BETWEEN APPLE COMPUTER, INC. AND THE
 * ORIGINAL LICENSEE THAT OBTAINED THESE MATERIALS FROM APPLE COMPUTER,
 * INC.  ANY USE OF THESE MATERIALS NOT PERMITTED BY SUCH AGREEMENT WILL
 * EXPOSE YOU TO LIABILITY.
 ***************************************************************************

   giantPortCommon.h - common header used to specify and access
         platform-dependent giant digit routines.

 Revision History
 ----------------
 1 Sep 98	Doug Mitchell at Apple
 	Created.

*******************************/

#ifndef _GIANT_PORT_COMMON_H_
#define _GIANT_PORT_COMMON_H_

#include <libGiants/giantTypes.h>

/* 
 * This is a platform-dependent symbol that must be defined elsewhere,
 * typically in giantPlatform.h.
 */
#ifndef	GI_GIANT_PORT_INLINE
#error Please define GI_GIANT_PORT_INLINE.
#endif

#if		GI_GIANT_PORT_INLINE

/*
 * Vanilla C version using static inlines, should work on any platform,
 * though may not be the fastest possible implementation.
 */
#include <libGiants/giantPort_Generic.h>

#else	/* GI_GIANT_PORT_INLINE */

/*
 * A platform-dependent implementation of these functions is required.
 */

/*
 * Add two digits, return sum. Carry bit returned as an out parameter.
 */
extern giantDigit giantAddDigits(
	giantDigit dig1,
	giantDigit dig2,
	giantDigit *carry);			/* RETURNED, 0 or 1 */

/*
 * Add a single digit value to a double digit accumulator in place.
 * Carry out of the MSD of the accumulator is not handled.
 */
extern void giantAddDouble(
	giantDigit *accLow,			/* IN/OUT */
	giantDigit *accHigh,		/* IN/OUT */
	giantDigit val);

/*
 * Subtract a - b, return difference. Borrow bit returned as an out parameter.
 */
extern giantDigit giantSubDigits(
	giantDigit a,
	giantDigit b,
	giantDigit *borrow);		/* RETURNED, 0 or 1 */

/*
 * Multiply two digits, return two digits.
 */
extern void giantMulDigits(
	giantDigit	dig1,
	giantDigit	dig2,
 	giantDigit	*lowProduct,		/* RETURNED, low digit */
	giantDigit	*hiProduct);		/* RETURNED, high digit */

#if 0
/* this is not currently used or needed */
/*
 * Multiply a vector of giantDigits, candVector, by a single giantDigit,
 * plierDigit, adding results into prodVector. Returns m.s. digit from
 * final multiply; only candLength digits of *prodVector will be written.
 */
extern giantDigit VectorMultiply(
	giantDigit plierDigit,
	giantDigit *candVector,
	gi_size   candLength,
	giantDigit *prodVector);
#endif

#endif	/* GI_GIANT_PORT_INLINE */

#endif	/* _GIANT_PORT_COMMON_H_ */
