/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * giantExternal.c - convert between giant integer and external representation
 *
 * Created Aug. 5 2005 by Doug Mitchell.
 */

#include <libGiants/giantTypes.h>
#include <libGiants/giantExternal.h>
#include <libGiants/giantDebug.h>
#include <stdlib.h>

/* 
 * Precalculate actual size in bytes needed to serialize a given giant.
 */
gi_uint16 serializeGiantBytes(const giant g)
{
	gi_size		numDigits, numZeros, digitByte;
    giantDigit  digit;

	gtrimSign(g);
	numDigits = g->sign;
	if(numDigits == 0) {
		return 0;
	}
    /* Count the number of leading 0 bytes in the m.s. digit. */
    numZeros = 0;
    digit = g->n[numDigits - 1];
    for(digitByte=0; digitByte<GIANT_BYTES_PER_DIGIT; digitByte++) {
        /* one loop per byte in the digit */
        gi_uint8 byte = (gi_uint8)digit;
        digit >>= 8;
        if (byte) {
            numZeros = 0;
        } else {
            numZeros++;
        }
    }
    return (numDigits * GIANT_BYTES_PER_DIGIT) - numZeros;
}

/*
 * Serialize, deserialize giants's n[] to/from byte stream.
 * First byte of byte stream is the m.s. byte of the resulting giant,
 * regardless of the size of giantDigit.
 *
 * No assumption is made about the alignment of cp.
 *
 * The numBytes argument indicates the space available on input;
 * on return it indicates the actual number of bytes written. 
 */
GIReturn serializeGiant(
	const giant g,
	gi_uint8 *cp,
	gi_uint16 *numBytes)		/* IN/OUT */
{
	gi_size     digitDex, numDigits, digitByte, totalBytes, bytesLeft;
	gi_uint8	*currCp;
	giantDigit  digit;

	GI_CHECK_SP("serializeGiant");
	if((g == NULL) || (cp == NULL) || (numBytes == NULL)) {
		return GR_IllegalArg;
	}
    totalBytes = serializeGiantBytes(g);
	if(totalBytes == 0) {
		*numBytes = 0;
		return GR_Success;
	}
	numDigits = g->sign;
	if(totalBytes > *numBytes) {
		dbgLog("serializeGiant: Insufficient space\n");
		return GR_Overflow;
	}
	
	/*
	 * Emit bytes starting at the far end of the outgoing byte
	 * stream, which is the l.s. byte of giant data. In order to prevent
     * writing out leading zeros, we special case the m.s. digit.
	 */
	currCp = cp + totalBytes - 1;
	for(digitDex=0;	digitDex<numDigits - 1; digitDex++) {
	    /* one loop per giant digit */
	    digit = g->n[digitDex];
		
	    for(digitByte=0; digitByte<GIANT_BYTES_PER_DIGIT; digitByte++) {
	        /* one loop per byte in the digit */
	    	*currCp-- = (gi_uint8)digit;
			digit >>= 8;
	    }
	}

    /* Handle the m.s. digit, by writing out only as many bytes as are left.
       Since we already wrote out numDigits - 1 digits above the answer is: */
    bytesLeft = totalBytes - (numDigits - 1) * GIANT_BYTES_PER_DIGIT;
    digit = g->n[digitDex];
    for(digitByte=0; digitByte<bytesLeft; digitByte++) {
        /* one loop per byte in the digit */
        *currCp-- = (gi_uint8)digit;
        digit >>= 8;
    }

	*numBytes = totalBytes;
	return GR_Success;
}

/* 
 * Initialize a giant with specified data. Does not have to be
 * aligned to giantDigits (as the output of serializeGiant() always 
 * is). 
 */
GIReturn deserializeGiant(
	giant g,
	const gi_uint8 *cp,
	gi_uint16 numBytes)
{
	gi_size 	numDigits;
	giantDigit 	digit;
	giantDigit	tempDigit;
	gi_size		digitDex;
	gi_size		digitByte;
	gi_size		dex;
	const gi_uint8	*currCp;
	
	GI_CHECK_SP("deserializeGiant");
	if(g == NULL) {
		return GR_IllegalArg;
	}
	if(numBytes == 0) {
		g->sign = 0;
		return GR_Success;
	}
    if(cp == NULL) {
		return GR_IllegalArg;
	}

	/* 
	 * Trim off leading zeroes, sometimes placed there during DER
	 * encoding to ensure the number does not appear negative. We 
	 * don't support negative numbers in this module...
	 */
	
	while((*cp == 0) && (numBytes != 0)) {
		cp++;
		numBytes--;
	}	
	if(numBytes == 0) {
		g->sign = 0;
		return GR_Success;
	}
	
	numDigits = BYTES_TO_GIANT_DIGITS(numBytes);
	if(numDigits > g->capacity) {
		return GR_Overflow;
	}

	/*
	 * Start at l.s. byte, at the far end of the byte stream.
	 */
	digitDex = 0;				/* index into g->n[] */
	digit = 0;					/* accumulator */
	digitByte = 0;				/* byte index into digit */
	currCp = cp + numBytes - 1;
	
	for(dex=0; dex<numBytes; dex++) {
		/* add in current byte */
		tempDigit = *currCp--;
		tempDigit <<= (8 * digitByte);
		digit |= tempDigit;
		digitByte++;
		
		/* have a complete digit, or reached the last byte? */
		if((digitByte == GIANT_BYTES_PER_DIGIT) ||	/* full digit */
		   (dex == (numBytes - 1))) {				/* no more data */
			g->n[digitDex++] = digit;
			
			/* reset for next digit */
			digit = 0;
			digitByte = 0;
		} 
	}

	/*
	 * Infer sign/size from non-zero n[] elements
	 */
	g->sign = numDigits;
	gtrimSign(g);
	return GR_Success;
}

