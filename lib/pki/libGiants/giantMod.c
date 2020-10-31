/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * giantMod.c - modular arithmetic function declarations.
 *
 * Created Nov. 29 2005 by Doug Mitchell.
 */

#include <libGiants/giantMod.h>
#include <libGiants/giantDebug.h>


#ifndef	GI_SMALL_CRT_ENABLE
#error Please define GI_SMALL_CRT_ENABLE.
#endif

#if GI_SMALL_CRT_ENABLE
/*
 * Custom version of the tgiant for GI_SMALL_CRT_ENABLE use.
 * Normally a tgiant has 2n+2 digits. The corresponding "small tgiant"
 * has n+2 digits. 
 */
#define GIANT_SMALL_TRIPLE_DIGITS	(MAX_PRIME_SIZE_DIGITS + 2)

typedef struct {	
	giantStruct	g;
	giantDigit	_n_[GIANT_SMALL_TRIPLE_DIGITS];
} smallTGiantStruct;
typedef smallTGiantStruct stgiant;	

static GI_INLINE void localSmallTGiantAlloc(stgiant *g)
{
	initGiant(&g->g, GIANT_SMALL_TRIPLE_DIGITS, g->_n_);
}

static void rsquareSmall(
	giant x, 
	giant n, 
	giant recip);
static void rmulgSmall(
	giant x, 
	giant y, 
	giant n, 
	giant recip);
static void modg_via_recipSmall(
	giant 	d,
	giant 	r,
	giant 	n);
static void powermodgSmall(
	giant		base,
	giant		expo,
	giant		n,
	giant		recip,
	giant		result);
#endif

/* for lots of grungy low-level debugging */
#define DEBUG_MODG	0
#if		DEBUG_MODG

#include <stdio.h>
#include <libGiantsUtils/printGiant.h>

static void dispGiant(
	const char *label,
	giant g)
{
	printf("--- %s : ", label);
	printGiant(g);
}

static void dispInt(
	const char *label,
	unsigned i)
{
	printf("--- %s : %u\n", label, i);
}

static void dispHeader(
	const char *label)
{
	printf("--- %s ---\n", label);
}
static void dispDelim(void)
{
	printf("-------------\n");
}
#else	/* DEBUG_MODG */
#define dispGiant(l, g)
#define dispInt(l, i)
#define dispDelim()
#define dispHeader(l)
#endif	/* DEBUG_MODG */

#if		GI_MODG_ENABLE


/* returns 1 if g has exactly one bit set, else returns zero */
static int isPowerOfTwo(
	giant g)
{
	int foundBit = 0;
	gi_uint16 dex;
	giantDigit currDigit;
	giantDigit mask;
	
	GI_CHECK_SP("isPowerOfTwo");
	if(isZero(g)) {
		return 0;
	}
	
	/* see if m.s. digit has exactly one bit set */
	currDigit = g->n[g->sign - 1];
	/* mask shifts out to zero when done */
	for(mask=0x01; mask != 0; mask <<= 1) {
		if(currDigit & mask) {
			if(foundBit) {
				/* found second bit set: done  */
				return 0;
			}
			else {
				/* first '1' bit seen */
				foundBit = 1;
			}
		}
	}
	GIASSERT(foundBit != 0);

	/* now we return true iff all the rest of the digits are zero */
	for(dex=0; dex<g->sign-1; dex++) {
		if(g->n[dex] != 0) {
			return 0;
		}
	}
	return 1;
}

/*
 * Calculate the reciprocal of a demonimator.
 */
void make_recip(
	giant d, 
	giant recip)
/* r becomes the steady-state reciprocal
   2^(2b)/d, where b = bit-length of d-1. */
{
	int b;
	
	tgiant tmp1;
	bgiant tmp2;
	tgiant tmp3;
	
	GI_CHECK_SP("make_recip (1)");
	if (isZero(d)) {
		GIASSERT(0);
		return;
	}
	if(isPowerOfTwo(d)) {
		gtog(d, recip);
		return;
	}
	
	localTriGiantAlloc(&tmp1);
	localBigGiantAlloc(&tmp2);
	localTriGiantAlloc(&tmp3);
	//localGiantAlloc(&tmpRecip);
	
	GI_CHECK_SP("make_recip (2)");
	
    b = bitlen(d); 
	int_to_giant(1, recip); 
	if((b & (GIANT_BITS_PER_DIGIT - 1)) == 0) {
		/* digit-aligned */
		gshiftwordsleft(b >> GI_GIANT_LOG2_BITS_PER_DIGIT, recip);
	}
	else {
		gshiftleft(b, recip); 
	}
	gtog(recip, &tmp2.g);
	
	while(1) {
		grammarSquare_common(recip, &tmp1.g);
		gshiftright(b, &tmp1.g);
		mulg_common(d, &tmp1.g, &tmp3.g);
		gshiftright(b, &tmp3.g);
		addg(recip, recip); 
		normal_subg(&tmp3.g, recip);
		if(gcompg(recip, &tmp2.g) <= 0) {
			break;
		}
		gtog(recip, &tmp2.g);
	}
	mulg_common(recip, d, &tmp1.g);
	int_to_giant(1, &tmp2.g);
	while(bitlen(&tmp1.g) > (2 * b)) {
		normal_subg(&tmp2.g, recip);
		normal_subg(d, &tmp1.g);
	}
    CHECK_GIANT_OFLOW(recip);
}

#define MODG_CTR_LIMIT	3

/*
 * Optimized modg.
 * n := n % d, where r is the precalculated steady-state reciprocal of d.
 */
void modg_via_recip(
	giant 	d,
	giant 	r,
	giant 	n)
{
    int        s = bitlen(r)-1;
    bgiant     tmp1;
    tgiant     tmp2;

    //dispGiant("d", d);
    //dispGiant("r", r);
    //dispGiant("n", n);

    GI_CHECK_SP("modg_via_recip(1)");
    GIASSERT(!isZero(r));

    if(s == 0) {
        int_to_giant(0, n);
        return;
    }
	if(gcompg(n, d) < 0) {
		/* this mod is a nop: done. */
		return;
	}
    localBigGiantAlloc(&tmp1);
    localTriGiantAlloc(&tmp2);

	do {
		unsigned ct = 0;
		gtog(n, &tmp1.g);
		gshiftright(s-1, &tmp1.g);

		mulg_common(r, &tmp1.g, &tmp2.g);
		gshiftright(s+1, &tmp2.g);
		mulg_common(d, &tmp2.g, &tmp1.g);
		normal_subg(&tmp1.g, n);

		while(++ct <= MODG_CTR_LIMIT) {
			if(gcompg(n,d) >= 0) {
				normal_subg(d,n);
			}
			else {
				return;
			}
		}
    } while(1);
}

/* Both n and div are written by this function, with new values:
   n   := n mod d,
   div := floor(n/d).
 */
void divmodg_via_recip(
	giant 	d,
	giant 	r,
	giant 	n,
    giant   dv)
{
    int        s = bitlen(r)-1;
    tgiant     tmp1;
    tgiant     tmp2;

    GI_CHECK_SP("divmodg_via_recip(1)");
    GIASSERT(!isZero(r));

    if(s == 0) {
        int_to_giant(0, n);
        gtog(n, dv);
        return;
    }
    int_to_giant(0, dv);
	if(gcompg(n, d) < 0) {
		/* this mod is a nop: done. */
		return;
	}
    localTriGiantAlloc(&tmp1);
    localTriGiantAlloc(&tmp2);
    GI_CHECK_SP("divmodg_via_recip(2)");

	do {
		unsigned ct = 0;
		gtog(n, &tmp1.g);
		gshiftright(s-1, &tmp1.g);
		mulg_common(r, &tmp1.g, &tmp2.g);
		gshiftright(s+1, &tmp2.g);
		mulg_common(d, &tmp2.g, &tmp1.g);
		normal_subg(&tmp1.g, n);
        addg(&tmp2.g, dv);
		while(++ct <= 3) {
			if(gcompg(n,d) >= 0) {
				normal_subg(d,n);
                iaddg(1, dv);
			}
			else {
				return;
			}
		}
    } while(1);
}

/* x := (x^2) mod n with known reciprocal of n */
void rsquare(
	giant x, 
	giant n, 
	giant recip)
{
	bgiant sq;
	
	GI_CHECK_SP("rsquare(1)");
	localBigGiantAlloc(&sq);
	GI_CHECK_SP("rsquare(2)");
	grammarSquare_common(x, &sq.g);
	modg_via_recip(n, recip, &sq.g);
	gtog(&sq.g, x);
}

/* y := (x * y) mod n with known reciprocal of n */
void rmulg(
	giant x, 
	giant y, 
	giant n, 
	giant recip)
{
	bgiant prod;
	
	GI_CHECK_SP("rmulg");
	localBigGiantAlloc(&prod);
	mulg_common(x, y, &prod.g);
	modg_via_recip(n, recip, &prod.g);
	gtog(&prod.g, y);
}

/* 
 * result becomes base^expo (mod n). 
 * Caller knows and gives us reciprocal of n as recip.
 * base and result can share the same giant. 
 */
void powermodg(
	giant		base,
	giant		expo,
	giant		n,
	giant		recip,
	giant		result)
{
	int 		len, pos;
	lgiant		bmod;			/* saved base mod n */
	
	GI_CHECK_SP("powermodg");
	GI_LOG_SP("top of powermodg");
    if(isZero(expo)) {
		int_to_giant(1, result); 
		return; 
	}
             
	localGiantAlloc(&bmod);	
	
	if(base != result) {
		gtog(base, result);
	}
	if(gcompg(result, n) >= 0) {
		modg_via_recip(n, recip, result);
	}
	gtog(result, &bmod.g);
	
	len = bitlen(expo);
	if(len == 1) {
		return;
	}
	for(pos = len-2; pos >= 0; pos--) {
        rsquare(result, n, recip);	// New ring-square.
		if (bitval(expo, pos)) {
			rmulg(&bmod.g, result, n, recip);
		}
	}
	return;
}

void modg(
	giant 	d,
	giant 	n)
/* n becomes n%d. n is arbitrary, but the denominator d must be positive! */
{
	lgiant recip;
	
	GI_CHECK_SP("modg");
	localGiantAlloc(&recip);
	make_recip(d, &recip.g);
	modg_via_recip(d, &recip.g, n);
}

/*
 * power/mod using Chinese Remainder Theorem. 
 *
 * result becomes base^d (mod n). 
 *
 * ...but we don't know or need d or n. What we have are
 *
 * p, q such that n = p*q
 * reciprocals of p, q
 * dp = d mod (p-1)
 * dq = d mod (q-1)
 * qinv = q^(1) mod p
 */
void powermodCRT(
	giant		x,			/* base (plaintext/ciphertext) */
	giant		p,			/* p*q = n */
	giant		pRecip,		/* reciprocal of p */
	giant		q,
	giant		qRecip,		/* reciprocal of p */
	giant		dp,			/* d mod (p-1) */
	giant		dq,			/* d mod (q-1) */
	giant		qinv,
	giant		sq,			/* scratch, provided and init'd by caller */
	giant		result)		/* OUTPUT */
{
	bgiant	sp;
	int		m;
	
	GI_CHECK_SP("powermodCRT");
	localBigGiantAlloc(&sp);
	GIASSERT(sq->capacity >= GIANT_LITTLE_DIGITS);
	
	/* 
	 * sp := x^dp mod p; 
	 * sq := x^dq mod q; 
	 */
	#if GI_SMALL_CRT_ENABLE
	powermodgSmall(x, dp, p, pRecip, &sp.g);
	powermodgSmall(x, dq, q, qRecip, sq);
	#else
	powermodg(x, dp, p, pRecip, &sp.g);
	powermodg(x, dq, q, qRecip, sq);
	#endif
	
	/*
	 * m = gcompg(sp, sq);
	 * if(m > = 0) 
	 *     result = sp-sq;
	 * else
	 *     result = sq-sp;    // Note unsigned subtract always;
	 */
	m = gcompg(&sp.g, sq);
	if(m >= 0) {
		gtog(&sp.g, result);
		normal_subg(sq, result);
	}
	else {
		gtog(sq, result);
		normal_subg(&sp.g, result);
	}
	
	/* 
	 * result = (qinv * result) mod p; 
	 * done with sp, use as scratch now...
	 */
	mulg_common(result, qinv, &sp.g);
	modg_via_recip(p, pRecip, &sp.g);
	gtog(&sp.g, result);
	
	/*
	 * if( ! isZero(result)) if(m < 0) result = p - result;  
	 */
	if(!isZero(result)) {
		if(m < 0) {
			gtog(p, &sp.g);
			normal_subg(result, &sp.g);
			gtog(&sp.g, result);
		}
	}
	
    /* 
     * result *= q;
     * result += sq;
	 */
	mulg_common(result, q, &sp.g);
	addg(sq, &sp.g);
	gtog(&sp.g, result);
}

#endif	/* GI_MODG_ENABLE */

#if		GI_INVERSE_MOD_ENABLE

/*
 * Given x, y coprime with x > 0, y > 1, this routine forces
 * x := x^(-1) mod modulus.
 */
void ginverseMod(
	giant modulus,
	giant recip,		/* reciprocal of modulus */
	giant x)			/* IN/OUT */
{
	lgiant u, v, a, q, tmp;
	unsigned int k = 0;
	lgiant uRecip;
	
	GI_CHECK_SP("ginverseMod");
	localGiantAlloc(&u);
	localGiantAlloc(&v);
	localGiantAlloc(&a);
	localGiantAlloc(&q);
	localGiantAlloc(&tmp);
	localGiantAlloc(&uRecip);
	
	dispHeader("ginverseMod top");
	dispGiant("x", x);
	dispGiant("modulus", modulus);
	dispGiant("recip", recip);

	/* 
	 * 1) (u, v, a, x, k) = (x mod modulus, modulus, 0, 1, 0);
	 */
	gtog(x, &u.g);
	modg_via_recip(modulus, recip, &u.g);
	gtog(modulus, &v.g);
	/* a is already zero */
	int_to_giant(1, x);
	
	dispHeader("ginverseMod before loop");
	dispGiant("x", x);
	dispGiant("u", &u.g);
	dispGiant("v", &v.g);
	dispGiant("a", &a.g);
	dispInt("k", k);

	/* 
	 * 2) while(u > 1) {
	 *         q = v div u;
	 *         (u, v, a, x, k) = (v mod u, u, x, (a + q*x) mod y, k + 1);
	 *    }
	 */
	while(!isZero(&u.g) && !isone(&u.g)) {
		/* (q,u) := (v div u, v mod u */
		make_recip(&u.g, &uRecip.g);
		gtog(&u.g, &tmp.g);			/* for a const modulus in divmodg_via_recip() */
		gtog(&v.g, &u.g);			/* divisor, then result of mod */
		divmodg_via_recip(&tmp.g, &uRecip.g, &u.g, &q.g);

		/* v := old u */
		gtog(&tmp.g, &v.g);
		
		/* intermediate: *old* x */
		gtog(x, &tmp.g);

		/* calc new x := a + q*x mod y */
		rmulg(&q.g, x, modulus, recip);
		addg(&a.g, x);
		modg_via_recip(modulus, recip, x);	/* just in case */

		/* new a := old x */
		gtog(&tmp.g, &a.g);
		k++;
		
		dispDelim();
		dispGiant("x", x);
		dispGiant("q", &q.g);
		dispGiant("u", &u.g);
		dispGiant("v", &v.g);
		dispGiant("a", &a.g);
		dispInt("k", k);
	}
	
	/*
	 * 3) if(k odd) x = y - x;    // Unsigned (normal_subg) subtract.
	 */
	if(k & 1) {
		gtog(modulus, &u.g);
		normal_subg(x, &u.g);
		gtog(&u.g, x);
	}
}

#endif	/* GI_INVERSE_MOD_ENABLE */

#if GI_SMALL_CRT_ENABLE

/*
 * Optional "small giant" routines used only by powermodCRT when 
 * performing its initial powermodg operations with p and q as the 
 * modulus; p and q are half the size of MAX_PRIME_SIZE_DIGITS, so
 * we implement the following functions with corresponding small
 * local giants. 
 * On PPC, this saves 384 bytes of stack space, at the cost of 
 * 1407 extra bytes of code. 
 */
static void rsquareSmall(
	giant x, 
	giant n, 
	giant recip)
{
	stgiant sq;
	
	GI_CHECK_SP("rsquareSmall(1)");
	localSmallTGiantAlloc(&sq);
	GI_CHECK_SP("rsquareSmall(2)");
	grammarSquare_common(x, &sq.g);
	modg_via_recipSmall(n, recip, &sq.g);
	gtog(&sq.g, x);
}

/* y := (x * y) mod n with known reciprocal of n */
static void rmulgSmall(
	giant x, 
	giant y, 
	giant n, 
	giant recip)
{
	stgiant prod;
	
	GI_CHECK_SP("rmulgSmall");
	localSmallTGiantAlloc(&prod);
	mulg_common(x, y, &prod.g);
	modg_via_recipSmall(n, recip, &prod.g);
	gtog(&prod.g, y);
}

/*
 * Optimized modg.
 * n := n % d, where r is the precalculated steady-state reciprocal of d.
 */
static void modg_via_recipSmall(
	giant 	d,
	giant 	r,
	giant 	n)
{
    int        s = bitlen(r)-1;
    stgiant    tmp1;
    stgiant    tmp2;

    GI_CHECK_SP("modg_via_recipSmall(1)");
    GIASSERT(!isZero(r));

    if(s == 0) {
        int_to_giant(0, n);
        return;
    }
	if(gcompg(n, d) < 0) {
		/* this mod is a nop: done. */
		return;
	}
    localSmallTGiantAlloc(&tmp1);
    localSmallTGiantAlloc(&tmp2);

	do {
		unsigned ct = 0;
		gtog(n, &tmp1.g);
		gshiftright(s-1, &tmp1.g);

		mulg_common(r, &tmp1.g, &tmp2.g);
		gshiftright(s+1, &tmp2.g);
		mulg_common(d, &tmp2.g, &tmp1.g);
		normal_subg(&tmp1.g, n);

		while(++ct <= MODG_CTR_LIMIT) {
			if(gcompg(n,d) >= 0) {
				normal_subg(d,n);
			}
			else {
				return;
			}
		}
    } while(1);
}

/* 
 * result becomes base^expo (mod n). 
 * Caller knows and gives us reciprocal of n as recip.
 * base and result can share the same giant. 
 *
 * This is only used by powermodCRT, which conveniently hands us 
 * two initialized scratch giants, one bgiant and one stgiant
 * so neither we nor any of our callers have to allocate any
 * local giants. 
 */
static void powermodgSmall(
	giant		base,
	giant		expo,
	giant		n,
	giant		recip,
	giant		result)
{
	int 		len, pos;
	lgiant		bmod;			/* saved base mod n */
	
	GI_CHECK_SP("powermodgSmall");
	GI_LOG_SP("top of powermodgSmall");
    if(isZero(expo)) {
		int_to_giant(1, result); 
		return; 
	}
             
	localGiantAlloc(&bmod);	
	
	if(base != result) {
		gtog(base, result);
	}
	if(gcompg(result, n) >= 0) {
		modg_via_recipSmall(n, recip, result);
	}
	gtog(result, &bmod.g);
	
	len = bitlen(expo);
	if(len == 1) {
		return;
	}
	for(pos = len-2; pos >= 0; pos--) {
        rsquareSmall(result, n, recip);	// New ring-square.
		if (bitval(expo, pos)) {
			rmulgSmall(&bmod.g, result, n, recip);
		}
	}
	return;
}

#endif	/* GI_SMALL_CRT_ENABLE */
