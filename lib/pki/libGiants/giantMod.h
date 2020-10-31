/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * giantMod.h - modular arithmetic function declarations.
 *
 * Created Nov. 29 2005 by Doug Mitchell.
 */

#ifndef	_GIANT_MOD_H_
#define _GIANT_MOD_H_

#include <libGiants/giantIntegers.h>

/* this must be defined in giantPlatform.h */
#ifndef	GI_MODG_ENABLE
#error	Please define GI_MODG_ENABLE one way or the other. 
#endif

#if		GI_MODG_ENABLE

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * Calculate the reciprocal of a demonimator to be used in 
 * modg_via_recip().
 */
void make_recip(giant d, giant recip);

/*
 * n := n % d, where r is the precalculated
 * steady-state reciprocal of d. 
 */
void modg_via_recip(
	giant 	d,
	giant 	r,
	giant 	n);

/* Both n and div are written by this function, with new values:
   n   := n mod d,
   div := floor(n/d).
 */
void divmodg_via_recip(
	giant 	d,
	giant 	r,
	giant 	n,
    giant   dv);

/* x := (x^2) mod n with known reciprocal of n */
void rsquare(
	giant x, 
	giant n, 
	giant recip);

/* y := (x * y) mod n with known reciprocal of n */
void rmulg(
	giant x, 
	giant y, 
	giant n, 
	giant recip);

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
	giant		result);

/* n becomes n%d. n is arbitrary, but the denominator d must be positive! */
void modg(giant	d,giant	n);

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
 *
 * Plus one scratch giant, initialized by caller with the capacity at
 * least as big as an lgiant. 
 *
 * The base and result arguments can be identical, i.e., an in/out
 * parameter. 
 */
void powermodCRT(
	giant		base,		/* base (plaintext/ciphertext) */
	giant		p,			/* p*q = n */
	giant		pRecip,		/* reciprocal of p */
	giant		q,
	giant		qRecip,		/* reciprocal of q */
	giant		dp,			/* d mod (p-1) */
	giant		dq,			/* d mod (q-1) */
	giant		qinv,
	giant		scratchLgiant,
	giant		result);	/* OUTPUT */

/*
 * Given x, y coprime with x > 0, y > 1, this routine forces
 * x := x^(-1) mod y.
 */
void ginverseMod(
	giant modulus,
	giant recip,		/* reciprocal of modulus */
	giant x);			/* IN/OUT */

#ifdef	__cplusplus
}
#endif

#endif	/* GI_MODG_ENABLE */

#endif	/* _GIANT_MOD_H_ */
