/* Copyright (c) 1998-2007 Apple Inc.  All Rights Reserved. */

/*

   giantIntegers.c - library for large-integer arithmetic.

 Revision History
 ----------------
 12/04/98	dmitch/R. Crandall
 	Fixed a==b bug in addg(). 
 10/06/98		ap
 	Changed to compile with C++.
 13 Apr 98	Fixed shiftright(1) bug in modg_via_recip.
 09 Apr 98	Doug Mitchell at Apple
 	Major rewrite of core arithmetic routines to make this module
		independent of size of giantDigit.
 	Removed idivg() and radixdiv().
 20 Jan 98 	Doug Mitchell at Apple
 	Deleted FFT arithmetic; simplified mulg().
 09 Jan 98	Doug Mitchell and Richard Crandall at Apple
 	gshiftright() optimization.
 08 Jan 98	Doug Mitchell at Apple
 	newGiant() returns NULL on malloc failure
 24 Dec 97	Doug Mitchell and Richard Crandall at Apple
 	New grammarSquare(); optimized modg_via_recip()
 11 Jun 97	Doug Mitchell and Richard Crandall at Apple
 	Added modg_via_recip(), divg_via_recip(), make_recip()
	Added new multiple giant stack mechanism
	Fixed potential packing/alignment bug in copyGiant()
	Added profiling for borrowGiant(), returnGiant()
	Deleted obsolete ifdef'd code
	Deleted newgiant()
	All calls to borrowGiant() now specify required size (no more
		borrowGiant(0) calls)
 08 May 97	Doug Mitchell at Apple
 	Changed size of giantstruct.n to 1 for Mac build
 05 Feb 97	Doug Mitchell at Apple
 	newGiant() no longer modifies CurrentMaxShorts or giant stack
	Added modg profiling
 01 Feb 97	Doug Mitchell at NeXT
 	Added iszero() check in gcompg
 17 Jan 97	Richard Crandall, Doug Mitchell at NeXT
 	Fixed negation bug in gmersennemod()
  	Fixed n[words-1] == 0 bug in extractbits()
	Cleaned up lots of static declarations
 19 Sep 96	Doug Mitchell at NeXT
 	Fixed --size underflow bug in normal_subg().
  4 Sep 96	Doug Mitchell at NeXT
  	Fixed (b<n), (sign<0) case in gmersennemod() to allow for arbitrary n.
  9 Aug 96	Doug Mitchell at NeXT
  	Fixed sign-extend bug in data_to_giant().
  7 Aug 96	Doug Mitchell at NeXT
  	Changed precision in newtondivide().
	Removed #ifdef UNUSED code.
 24 Jul 96	Doug Mitchell at NeXT
 	Added scompg().
      1991	Richard Crandall at NeXT
      	Created.

*/

#include <libGiants/giantTypes.h>
#include <libGiants/giantIntegers.h>
#include <libGiants/giantDebug.h>
#include <libGiants/giantPortCommon.h>
#include <stdlib.h>

#ifndef	GI_GIANT_LOG2_BITS_PER_DIGIT
#error	Please define GI_GIANT_LOG2_BITS_PER_DIGIT
#endif

#if		GIANTS_DEBUG
/*
 * counter, acccumulates calls to (giantDigit x giantDigit) multiply. 
 */
unsigned numGiantDigitMuls;
unsigned numMulgCommon;
unsigned numGSquareCommon;
#endif

/*
 * Local (heap-allocated) giant support.
 */

#if		!GI_INLINE_SMALL_FUNCS && !GI_MACRO_SMALL_FUNCTIONS

void initGiant(
	giant g, 
	gi_uint16 capacity,
	giantDigit *digits)
{
	GI_CHECK_SP("initGiant");
	g->sign = 0;
	g->capacity = capacity;
	g->n = digits;
}

/* call this after declaring a local lgiant and before using it */
void localGiantAlloc(lgiant *g)
{
	initGiant(&g->g, GIANT_LITTLE_DIGITS, g->_n_);
	
}

/* ditto, for a bgiant */
extern void localBigGiantAlloc(bgiant *g)
{
	initGiant(&g->g, GIANT_BIG_DIGITS, g->_n_);
}

/* ditto, for a bgiant */
void localTriGiantAlloc(tgiant *g)
{
	initGiant(&g->g, GIANT_TRIPLE_DIGITS, g->_n_);
}

#endif	/* !GI_INLINE_SMALL_FUNCS && !GI_MACRO_SMALL_FUNCTIONS */

gi_uint16 bitlen(giant n) {
    gi_uint16 	b = GIANT_BITS_PER_DIGIT;
    giantDigit 	c = 1 << (GIANT_BITS_PER_DIGIT - 1);
    giantDigit 	w;

	GI_CHECK_SP("bitlen");
    if (isZero(n)) {
    	return(0);
    }
    w = n->n[n->sign - 1]; 
    GIASSERT (w != 0);
    while((w&c) == 0) {
        b--;
        c >>= 1;
    }
    return(GIANT_BITS_PER_DIGIT * (n->sign - 1) + b);
}

giantDigit bitval(giant n, gi_uint16 pos) 
{
    gi_uint16 i = pos >> GI_GIANT_LOG2_BITS_PER_DIGIT;
    giantDigit c = 1 << (pos & (GIANT_BITS_PER_DIGIT - 1));

	GI_CHECK_SP("bitval");
    return((n->n[i]) & c);
}

gi_uint16 giantNumBytes(giant n) 
{
	gi_uint16 bits = bitlen(n);
	GI_CHECK_SP("giantNumBytes");
	return (bits + 7) / 8;
}

/*
 * Adjust sign for possible leading (m.s.) zero digits
 */
void gtrimSign(giant g)
{
	int i = g->sign-1;

	GI_CHECK_SP("gtrimSign");
	if(g->sign == 0) {
		return;
	}
    if(g->n[i] != 0) return;
	for(--i; i>=0; i--) {
		if(g->n[i] != 0) break;
	}
	g->sign = i+1;
}

#if		!GI_INLINE_SMALL_FUNCS && !GI_MACRO_SMALL_FUNCTIONS
int isone(giant g) {
	GI_CHECK_SP("isone");
    return((g->sign == 1) && (g->n[0] == 1));
}

int isZero(giant thegiant) {
/* Returns TRUE if thegiant == 0.  */
    int count;
    int length = thegiant->sign;
    giantDigit *numpointer;

	GI_CHECK_SP("isZero");
    if (length) {
        numpointer = thegiant->n;

        for(count = 0; count<length; ++count,++numpointer) {
            if (*numpointer != 0 ) {
				return 0;
			}
		}
		/* why wasn't this trimmed? */
		GIASSERT(0);
    }
    return 1;
}

#endif	/* !GI_INLINE_SMALL_FUNCS && !GI_MACRO_SMALL_FUNCTIONS */

/* Returns the number of trailing zero bits in g. */
gi_uint16 numtrailzeros(
	giant g)
{
	gi_uint16 numDigits = g->sign, j, bcount=0;
	giantDigit dig, c;

	for (j=0;j<numDigits;j++) {
		dig = g->n[j];
		c = 1;
		for (bcount=0;bcount<GIANT_BITS_PER_DIGIT; bcount++) {
			if (c & dig) {
				break;
			}
			c <<= 1;
		}
		if (bcount<GIANT_BITS_PER_DIGIT)
			break;
	}
	return(bcount + GIANT_BITS_PER_DIGIT*j);
}

int gcompg(giant a, giant b)
/* returns -1,0,1 if a<b, a=b, a>b, respectively */
{
    gi_uint16 sa = a->sign;
    int j;
    gi_uint16 sb = b->sign;
    giantDigit va;
    giantDigit vb;

    if(isZero(a) && isZero(b)) return 0;
    if(sa > sb) return(1);
    if(sa < sb) return(-1);
	
    for(j = sa-1; j >= 0; j--) {
        va = a->n[j]; vb = b->n[j];
		if (va > vb) return 1;
		if (va < vb) return -1;
    }
    return 0;
}

/* destgiant becomes equal to srcgiant */
void gtog(giant srcgiant, giant destgiant) {

    unsigned numbytes;

    GIASSERT(srcgiant != NULL);
    GIASSERT(destgiant != NULL);
    GIASSERT(srcgiant->sign <= destgiant->capacity);
	
	GI_CHECK_SP("gtog");
    numbytes =  srcgiant->sign * GIANT_BYTES_PER_DIGIT;
    Memcpy(destgiant->n, srcgiant->n, numbytes);
    destgiant->sign = srcgiant->sign;
    CHECK_GIANT_OFLOW(destgiant);
}

void int_to_giant(gi_uint32 i, giant g) {
/* The giant g becomes set to the integer value i. */
    gi_size dex;

	GI_CHECK_SP("int_to_giant");
    g->sign = 0;
    if (i == 0) {
		g->n[0] = 0;
		return;
    }

    if(sizeof(giantDigit) >= sizeof(i)) {
		/* guaranteed trivial case */
    	g->n[0] = i;
		g->sign = 1;
    }
    else {
		/* one loop per digit */
		gi_uint16 scnt = GIANT_BITS_PER_DIGIT;	

		for(dex=0; ; dex++) {
			g->n[dex] = i & GIANT_DIGIT_MASK;
			i >>= scnt;
			g->sign++;
			if(i == 0) {
				break;
			}
		}
    }
}

/* The integer i becomes set to the giant value g.  Returns GR_Overflow if
   the giant is too big.  */
GIReturn giant_to_int(giant g, gi_uint32 *i) {
    gi_uint16 dex;
    gi_uint32 value = 0;

	GI_CHECK_SP("giant_to_int");

    if (g->sign * GIANT_BYTES_PER_DIGIT > sizeof(*i))
        return GR_Overflow;

    for (dex = g->sign; dex-- > 0;) {
        value <<= GIANT_BITS_PER_DIGIT;
        value |= g->n[dex];
    }

    *i = value;
    return GR_Success;
}

/*------------- Arithmetic --------------*/

/* 
 * Two implementations of these. Which one is faster depends highly on your 
 * implementation of giantPortCommon.h and your platform's giantDouble performance. 
 * If giantDouble arithmetic is fast then use GI_ADDG_INLINE. Otherwise try 
 * !GI_ADDG_INLINE, measure the two, and pick the best one. 
 */
#if		GI_ADDG_INLINE

void iaddg(giantDigit i, giant g) {  
    gi_size j;
    giantDigit carry;
    gi_size size = g->sign;
	giantDouble tmp;
	
    if (isZero(g)) {
    	int_to_giant(i,g);
    }
    else {
		carry = i;
    	for(j=0; ((j<size) && (carry != 0)); j++) {
            tmp = (giantDouble)(g->n[j]) + (giantDouble)carry;
			carry = tmp >> GIANT_BITS_PER_DIGIT;
			g->n[j] = tmp;		// implicit truncate to giantDigit 
        }
		if(carry) {
			++g->sign;
			CHECK_GIANT_OFLOW(g);
			g->n[size] = carry;
		}
    }
}

void imulg(giantDigit a, giant g)		
{
	giantDigit	carry = 0;
	gi_size		size = g->sign;
	gi_size		j;
	giantDigit	*digit = g->n;
	giantDouble	k;
	
	GIASSERT((size == 0) || (g->n[size - 1] != 0));		// caller should trim!
	GI_CHECK_SP("imulg");
	
	for (j=0; j<size; ++j)
	{
		k = ((giantDouble)(*digit) * (giantDouble)a) + (giantDouble)carry;
		carry = k >> GIANT_BITS_PER_DIGIT;
		*digit = k;		// implicit truncation 
		++digit;
	}
	if (carry)
	{
		*digit = carry;
		g->sign = size+1;
		return;
	}
	g->sign = size;
    CHECK_GIANT_OFLOW(g);
}

void
normal_addg(
	giant			a,
	giant			b
)
{
	giantDigit	carry = 0;
	gi_size		asize = a->sign, bsize = b->sign;
	gi_size		j=0;
	giantDigit	*aptr = a->n, *bptr = b->n;
	giantDouble	k;
	
	GI_CHECK_SP("normal_addg");
	if (asize < bsize)
	{
		for (j=0; j<asize; j++)
		{
			k = (giantDouble)(*aptr++) + (giantDouble)(*bptr) + (giantDouble)carry;
			carry = k >> GIANT_BITS_PER_DIGIT;
			*bptr++ = k; 
		}
		for (j=asize; j<bsize; j++)
		{
			k = (giantDouble)(*bptr) + (giantDouble)carry;
			carry = k >> GIANT_BITS_PER_DIGIT;
			*bptr++ = k; 
		}
	}
	else
	{
		for (j=0; j<bsize; j++)
		{
			k = (giantDouble)(*aptr++) + (giantDouble)(*bptr) + (giantDouble)carry;
			carry = k >> GIANT_BITS_PER_DIGIT;
			*bptr++ = k;
		}
		for (j=bsize; j<asize; j++)
		{
			k = (giantDouble)(*aptr++) + (giantDouble)carry;
			carry = k >> GIANT_BITS_PER_DIGIT;
			*bptr++ = k;
		}
	}
	if (carry)
	{
		*bptr = 1; ++j;
	}
	b->sign = j;
    CHECK_GIANT_OFLOW(b);
}

#else	/* !GI_ADDG_INLINE */

/*
 * Use these versions if your giantPortCommon.h is faster than your platform's giantDouble
 * arithmetic
 */
 
void iaddg(giantDigit i, giant g) {  
    gi_size j;
    giantDigit carry;
    gi_size size = g->sign;

    if (isZero(g)) {
    	int_to_giant(i,g);
    }
    else {
    	carry = i;
		giantDigit *n = g->n;
    	for(j=0; ((j<size) && (carry != 0)); j++) {
            /* tmp = g->n[j] + carry;
               carry = tmp >> GIANT_BITS_PER_DIGIT;
			   g->n[j] = tmp;		-- implicit truncate to giantDigit */
			giantDigit sum;
			sum = giantAddDigits(*n, carry, &carry);
			*n++ = sum;
        }
		if(carry) {
			++g->sign;
			CHECK_GIANT_OFLOW(g);
			g->n[size] = carry;
		}
    }
}

void imulg(giantDigit a, giant g)		
{
	giantDigit		carry = 0;
	gi_size		size = g->sign;
	gi_size		j;
	giantDigit		*digit = g->n;

	GI_CHECK_SP("imulg");
	for (j=0; j<size; ++j)
	{
		giantDigit lowProd;
		giantDigit hiProd;
		
		/* k := (*digit * a) + carry; */
		giantMulDigits(*digit, a, &lowProd, &hiProd);
		giantAddDouble(&lowProd, &hiProd, carry);
		carry = hiProd;
		*digit = lowProd;
		++digit;
	}
	if (carry)
	{
		*digit = carry;
		g->sign = size+1;
		return;
	}
	g->sign = size;
}

void
normal_addg(
	giant			a,
	giant			b
)
{
	giantDigit	carry = 0;
	gi_size		asize = a->sign, bsize = b->sign;
	gi_size		j=0;
	giantDigit	*aptr = a->n, *bptr = b->n;
	giantDigit	sumHi;
	bgiant		tmpA;
	
	GI_CHECK_SP("normal_addg");
	if(a == b) {
		/* this routine cannot handle an add in place - make a copy */
		localBigGiantAlloc(&tmpA);
		gtog(a, &tmpA.g);
		aptr = tmpA.g.n;
	}
		
	if (asize < bsize)
	{
		for (j=0; j<asize; j++)
		{
			sumHi = 0;
			if(carry) {
				/* bring in previous carry */
				giantAddDouble(bptr, &sumHi, carry);
			}
			/* now the addend, accumulating carry */
			carry = sumHi;
			giantAddDouble(bptr++, &carry, *aptr++);
		}
		for (j=asize; j<bsize; j++)
		{
			if(carry == 0) {
				j = bsize;
				break;
			}
			sumHi = 0;
			giantAddDouble(bptr++, &sumHi, carry);
			carry = sumHi;
		}
	}
	else
	{
		for (j=0; j<bsize; j++)
		{
			sumHi = 0;
			if(carry) {
				/* bring in previous carry */
				giantAddDouble(bptr, &sumHi, carry);
			}
			/* now the addend, accumulating carry */
			carry = sumHi;
			giantAddDouble(bptr++, &carry, *aptr++);

		}
		for (j=bsize; j<asize; j++)
		{
			giantDigit sumLo = carry;
			sumHi = 0;
			giantAddDouble(&sumLo, &sumHi, *aptr++);
			carry = sumHi;
			*bptr++ = sumLo;
		}
	}
	if (carry)
	{
		*bptr = 1; ++j;
	}
	b->sign = j;
    CHECK_GIANT_OFLOW(b);
}

#endif	/* GI_ADDG_INLINE */

void normal_subg(giant a, giant b)
/* b := b - a; requires b, a non-negative and b >= a. */
{
    int j;
    int size = b->sign;
    giantDigit tmp;
    giantDigit borrow1 = 0;
    giantDigit borrow2 = 0;
    giantDigit *an = a->n;
    giantDigit *bn = b->n;

	GI_CHECK_SP("normal_subg");
	GIASSERT(gcompg(b, a) >= 0);	/* else we'd come up with a negative result... */
    if(a->sign == 0) {
    	return;
    }

    for (j=0; j<a->sign; ++j) {
		if(borrow1 || borrow2) {
			tmp = giantSubDigits(bn[j], (giantDigit)1, &borrow1);
		}
		else {
			tmp = bn[j];
			borrow1 = 0;
		}
		bn[j] = giantSubDigits(tmp, an[j], &borrow2);
    }
    if(borrow1 || borrow2) {
    	/* propagate borrow thru remainder of bn[] */
    	borrow1 = 1;
		for (j=a->sign; j<size; ++j) {
			if(borrow1) {
				bn[j] = giantSubDigits(bn[j], (giantDigit)1, &borrow1);
			}
			else {
				break;
			}
		}
    }

    /* adjust sign for leading zero digits */
    while((size-- > 0) && (b->n[size] == 0))
    	;
    b->sign = (b->n[size] == 0)? 0 : size+1;
	CHECK_GIANT_OFLOW(b);
}

/* return the number of leading zeroes in the m.s. digit */
static gi_uint16 gleadzeroes(giant g)
{
	giantDigit dig = g->n[g->sign - 1];
	gi_uint16 numZeroes = 0;
	giantDigit mask = 1 << (GIANT_BITS_PER_DIGIT - 1);
	
	GIASSERT(!isZero(g));
	
	while((dig & mask) == 0) {
		numZeroes++;
		mask >>= 1;
		if(mask == 0) {
			/* all zeroes, shouldn't happen */
			GIASSERT(0);
			return GIANT_BITS_PER_DIGIT;
		}
	}
	return numZeroes;
}

/*
 * The new gshiftleft costs an extra 280 bytes of code space, but has no measurable
 * increase in performance (on G4, gcc 3.3). 
 */
#define NEW_SHIFTLEFT	0

#if		NEW_SHIFTLEFT


void gshiftleft(gi_uint16 bits, giant g )
{
	/* Number of full words/digits to leftshift */
	gi_uint16 wordshift	 = bits >> GI_GIANT_LOG2_BITS_PER_DIGIT;
	/* Number of bits to shift left */
	gi_uint16 leftshift	 = bits & (GIANT_BITS_PER_DIGIT-1);	
	gi_uint16 rightshift = GIANT_BITS_PER_DIGIT - leftshift;
	gi_uint16 olddigits	 = g->sign;
	/* Leading 0's of high word */
	gi_uint16 lead; 
	gi_uint16 newdigits;
	gi_uint16 j = olddigits - 1;
	giantDigit remainder;
	gi_uint16 k;

	if (!bits || !olddigits) {
		return;
	}
	lead = gleadzeroes(g);
	newdigits = ((bits + (GIANT_BITS_PER_DIGIT-lead) - 1) >> GI_GIANT_LOG2_BITS_PER_DIGIT) + olddigits;
	k = newdigits;
	
	if (!leftshift) {
		/* Case 1: Whole word moves */
		Memcpy(g->n + wordshift, g->n, olddigits * GIANT_BYTES_PER_DIGIT);
		k = wordshift;
	} else if (leftshift <= lead) {
		/* Case 2: No bit-wrapping on the high end */
		giantDigit next = g->n[j];
		giantDigit load;
		while( j > 0 ) {
			load = next;
			next = g->n[j-1];
			remainder = next >> rightshift;
			g->n[--k] = (load << leftshift) | remainder;
			j--;
		}
		g->n[--k] = next << leftshift; // next == g->n[0]
	} else {
		/* Case 3: When we left-shift the high bits, they will spill over and use olddigits+wordshift+1 words */
		giantDigit load = g->n[j];
		remainder = 0;
		while( j > 0 ) {
			g->n[--k] = (load >> rightshift) | remainder;
			remainder = load << leftshift;
			load = g->n[--j];
		}
		g->n[--k] = (load >> rightshift) | remainder;
		remainder = load << leftshift;
		g->n[--k] = remainder;
	}
	Bzero(g->n, k * GIANT_BYTES_PER_DIGIT ); // Shift in 0's on the right
	g->sign = newdigits;
    CHECK_GIANT_OFLOW(g);
	return;
}

#else	/* OLD shiftleft */

void gshiftleft(gi_uint16 bits, giant g) {
/* shift g left bits bits.  Equivalent to g = g*2^bits */
    int 	rem = bits & (GIANT_BITS_PER_DIGIT - 1);
    int 	crem = GIANT_BITS_PER_DIGIT - rem;
    int 	digits = 1 + (bits >> GI_GIANT_LOG2_BITS_PER_DIGIT);
    int 	size = g->sign;
    int 	j;
    int 	k;
    giantDigit 	carry;
    giantDigit 	dat;
	gi_uint16	lead;
	gi_uint16	newdigits;
	
	GI_CHECK_SP("gshiftleft");
    if(!bits) return;
    if(!size) return;
	
    /* rem=0 means we're shifting strictly by digits, no bit shifts. */
    if(rem == 0) {
		gshiftwordsleft(digits-1, g);
		return;
    }
	lead = gleadzeroes(g);
	newdigits = ((bits + (GIANT_BITS_PER_DIGIT-lead) - 1) >> 
		GI_GIANT_LOG2_BITS_PER_DIGIT) + size;

	/* 
	 * due to hack of clearing g->n[k] below we can only shift such that
	 * the result is one digit less than the giant's capacity 
	 */
    //GIASSERT((size+digits) <= (int)g->capacity);
    k = size - 1 + digits;	// (MSD of result + 1)
    carry = 0;
	
	/*
	 * normal unaligned case
	 * FIXME - this writes past g->n[size-1] the first time thru!
	 */
	for(j=size-1; j>=0; j--) {
		dat = g->n[j];
		if(k < newdigits) {
			g->n[k--] = (dat >> crem) | carry;
		}
		else {
			k--;
		}
		carry = (dat << rem);
	}
	do{
		g->n[k--] = carry;
		carry = 0;
	} while(k>=0);

    g->sign = newdigits;
    CHECK_GIANT_OFLOW(g);
}
#endif	/* NEW_SHIFTLEFT */

void gshiftright(gi_uint16 bits, giant g) {
	/* shift g right bits bits.  Equivalent to g = g/2^bits */
    gi_size j;
    gi_size size = g->sign;
    giantDigit carry;
    gi_size digits = bits >> GI_GIANT_LOG2_BITS_PER_DIGIT;
    gi_size remain = bits & (GIANT_BITS_PER_DIGIT - 1);
    gi_size cremain = GIANT_BITS_PER_DIGIT - remain;

	GI_CHECK_SP("gshiftright");
    if(bits==0) return;
    if (digits >= size) {
        g->sign = 0;
        return;
    }

    size -= digits;

	/* Begin OPT: 9 Jan 98 REC. */
    if(remain == 0) {
		g->sign = size;
		/* FIXME we can do better than this with memmove or pointers... */
        for(j=0; j<size; j++) {
			g->n[j] = g->n[j+digits];
		}
        return;
    }
	/* End OPT: 9 Jan 98 REC. */

    for(j=0;j<size;++j) {
        if (j==size-1) {
			carry = 0;
		}
        else {
			carry = (g->n[j+digits+1]) << cremain;
		}
        g->n[j] = ((g->n[j+digits]) >> remain ) | carry;
    }
    if (g->n[size-1] == 0) {
    	--size;
    }
    g->sign = size;
    CHECK_GIANT_OFLOW(g);
}

// Right-shift z by s giantDigits 
void gshifltwordsright(
	gi_uint16 digits, 
	giant g)
{
    gi_size size = g->sign;
	gi_size numBytes;
	
	GI_CHECK_SP("gshifltwordsright");
    if(digits == 0) {
		return;
	}
	/* not necessary, we won't do anything if it's zero 
    if(isZero(g)) {
		return;
	}
	*/
    if (digits >= size) {
        g->sign = 0;
        return;
    }

    size -= digits;
	g->sign = size;
	numBytes = size * GIANT_BYTES_PER_DIGIT;
	Memcpy(&g->n[0], &g->n[digits], numBytes);
}

/* shift left by s giantDigits */
void gshiftwordsleft(
	gi_uint16 digits, 
	giant g)
{
	gi_size dex;
	giantDigit *src;
	giantDigit *dst;
	
	GI_CHECK_SP("gshiftwordsleft");
    if(digits == 0) {
		return;
	}
	
	/* start copying from m.s. digit */
	src = &g->n[g->sign - 1];
	dst = src + digits;
	for(dex=0; dex<g->sign; dex++) {
		*dst-- = *src--;
	}
	Bzero(g->n, digits * GIANT_BYTES_PER_DIGIT);
	g->sign += digits;
    CHECK_GIANT_OFLOW(g);
}

/* dest *= a * b */
void
mulg(
	giant			a,
	giant			b)
{
	bgiant	dest;
	
	localBigGiantAlloc(&dest);
	GI_CHECK_SP("mulg");
	mulg_common(a, b, &dest.g);
	gtog(&dest.g, b);
}

/* dest *= a * b */
void
mulg_common(
	giant			a,
	giant			b,
	giant			dest)		/* typically a bgiant */
{
	gi_size		i,j;
	giantDouble 	prod1;
	giantDouble 	prod2;
	giantDouble		carry1=0;
	giantDouble		carry2=0;
	gi_size 		asize = a->sign, bsize = b->sign;
	giantDigit		*aptr,*bptr,*destptr;
	giantDigit		curmul;
	giantDigit		prevmul;
	giantDigit		mult1;
	giantDigit		mult2;
	giantDouble		prevDigit;
	giantDouble		prodsum;

	INCR_NUM_MULGS;
	GIASSERT(dest->capacity >= (a->sign + b->sign));
	GI_CHECK_SP("mulg_common");
	
	if(isZero(a) || isZero(b)) {
		dest->sign = 0;
		return;
	}
	Bzero(dest->n, (asize+bsize) * GIANT_BYTES_PER_DIGIT);

	bptr = b->n;
	for (i=0; i<bsize-1; i += 2)
	{
		mult1 = *(bptr++);
		mult2 = *(bptr++);

		prevmul = 0;
		carry1 = 0;
		carry2 = 0;
		aptr = &(a->n[0]);
		destptr = &(dest->n[i]);		
		prevDigit = *destptr;
		
		for (j=0; j<asize; ++j)	{
			//prod = *(aptr++) * mult + *destptr + carry;
			curmul = (giantDouble)(*aptr++);
			
			prevDigit += carry1 + carry2;
							
			prod1 = (giantDouble)curmul*mult1;
			prod2 = (giantDouble)prevmul*mult2;
			
			INCR_DIGIT_MULS;
			INCR_DIGIT_MULS;
			
			carry1 = prod1 >> GIANT_BITS_PER_DIGIT;
			carry2 = prod2 >> GIANT_BITS_PER_DIGIT;

			prod1 &= GIANT_DIGIT_MASK;
			prod2 &= GIANT_DIGIT_MASK;

			prodsum = prod1 + prod2 + prevDigit;
			carry1 += prodsum >> GIANT_BITS_PER_DIGIT;
			prevDigit = *(destptr+1);
			*(destptr++) = prodsum;		// implicit truncate
			prevmul = curmul;				
		}

		prod1 = prevDigit + carry1;
		prod1 += (giantDouble)prevmul*mult2;
		prod1 += carry2;
		carry1 = prod1 >> GIANT_BITS_PER_DIGIT;
		*(destptr++) = prod1;		// implicit truncate
		*destptr = carry1;
	}

	if (i<bsize) {
		mult1 = *(bptr++);
		if (mult1)	{
			carry1 = 0;
			aptr = &(a->n[0]);
			destptr = &(dest->n[i]);
			for (j=0; j<asize; ++j)	{
				//prod = *(aptr++) * mult + *destptr + carry;
				prod1 = (giantDouble)(*aptr++);
				prod1 *= mult1;
				INCR_DIGIT_MULS;
				prod1 += *destptr;
				prod1 += carry1;
				*(destptr++) = prod1;		// implicit truncate
				carry1 = prod1 >> GIANT_BITS_PER_DIGIT;
			}
			*destptr = carry1;
		}
	}
	
	bsize += asize;
	if (bsize && !carry1) {
		--bsize;
	}
	dest->sign = bsize;
}

void
grammarSquare (
	giant a)
{
	bgiant	dest;
	
	localBigGiantAlloc(&dest);
	GI_CHECK_SP("grammarSquare");
	grammarSquare_common(a, &dest.g);
	gtog(&dest.g, a);
}

void
grammarSquare_common (
	giant a,
	giant dest)		/* typically a bgiant */
{
	gi_size		i,j;
	giantDigit		currentDigit;
	giantDouble 	prod;
	giantDouble		carry=0;
	gi_size 		asize = a->sign;
	giantDigit		*aptr,*bptr,*destptr;
	giantDigit		mult;

	INCR_NUM_GSQUARE;
	GI_CHECK_SP("grammarSquare_common");
	if(asize == 0) {
		/* 
		 * the rest of this routine really is not equipped to handle this, hence
		 * this trivial special casse
		 */
		dest->sign = 0;
		return;
	}
	destptr = dest->n;
	
	aptr = a->n;
	
	for (i=0; i<asize; ++i)
	{
		giantDouble	prodx;
		
		 mult = *aptr++;
		
		 prodx = ((giantDouble)mult) * mult;
		
		*destptr++ = prodx;
		*destptr++ = prodx >> GIANT_BITS_PER_DIGIT;
	}

	bptr = a->n;
	
	for (i=0; i<asize-1; ++i)
	{

		mult = *bptr++;
		
		//if (mult)
		if (1)
		{
			giantDouble		carried;
			giantDouble		produced;
			aptr = &(a->n[i+1]);
			destptr = &(dest->n[2*i+1]);
			carry = 0;
						
			for (j=i+1; j<asize; ++j)
			{
				//prod = *(aptr++) * mult + *destptr + carry;
				prod = (giantDouble)(*aptr++);
				
				produced = prod * mult;

				carried = (produced >> GIANT_BITS_PER_DIGIT) << 1;
				produced = (produced & GIANT_DIGIT_MASK) << 1;
				
				currentDigit = *destptr;
				prod = currentDigit + carry;
				prod += produced;
				
				*(destptr++) = prod;		// implicit truncate
				carry = prod >> GIANT_BITS_PER_DIGIT;
				
				carry += carried;
			}
			
			do {
				currentDigit = *destptr;
			
				carried = carry >> GIANT_BITS_PER_DIGIT;
				
				prod = (carry & GIANT_DIGIT_MASK) + currentDigit;

				*destptr++ = prod;	// implicit truncate
				
				carry = carried + (prod >> GIANT_BITS_PER_DIGIT);				
			} while (carry);
		}
	}

	asize += asize;
	
	currentDigit = dest->n[asize-1];
	currentDigit += carry;
	dest->n[asize-1] = currentDigit;
	
	if (asize && !currentDigit)
		--asize;
	dest->sign = asize;
}


/*
 * Clear all of a giant's data fields, for secure erasure of sensitive data.
 */
void clearGiant(giant g)
{
	GI_CHECK_SP("clearGiant");
	Bzero(g->n, g->capacity * GIANT_BYTES_PER_DIGIT);
    g->sign = 0;
}

