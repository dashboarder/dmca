/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * rsaGiantKey.h - internal format of RSA keys, constructors for them.
 */
 
#ifndef	_LIBGRSA_RSA_GIANT_KEY_H_
#define _LIBGRSA_RSA_GIANT_KEY_H_

#include <libgRSA/libgRSA_config.h>
#include <libGiants/giantIntegers.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * A custom "small" giant used to represent some individual fields in a
 * RSAFullGiantKey. These fields are typically half the size of the 
 * modulus. We use MAX_PRIME_SIZE/2 + 1 digits for these. 
 */
#define RSA_SMALL_GIANT_DIGITS		((MAX_PRIME_SIZE_DIGITS + 3) / 2)

typedef struct {	
	giantStruct	g;
	giantDigit	_n_[RSA_SMALL_GIANT_DIGITS];
} rsaSmallGiantStruct;

typedef rsaSmallGiantStruct rsaSgiant;		/* for heap allocation */

/* call this after declaring a local lgiant and before using it */
extern void localSmallGiantAlloc(rsaSgiant *g);

/* RSA public key form native to this library */
typedef struct {
	lgiant	e;			/* public exponent */
	lgiant	n;			/* modulus */
	lgiant	recip;		/* reciprocal of n */	
} RSAPubGiantKey;

/* RSA private key form native to this library */
typedef struct {
	rsaSgiant			p;
	rsaSgiant			q;
	rsaSgiant			dp;
	rsaSgiant			dq;
	rsaSgiant			qInv;
	
	/* reciprocals of p and q */
	rsaSgiant			pRecip;
	rsaSgiant			qRecip;
} RSAPrivGiantKey;

/* 
 * A full RSA key pair. 
 */
typedef struct {
	RSAPubGiantKey pub;
	lgiant		d;		/* private exponent */
	RSAPrivGiantKey priv;
} RSAFullGiantKey;

/* clear an RSA*GiantKey */
void rsaClearPubGKey(
	RSAPubGiantKey		*gKey);
	
void rsaClearPrivGKey(
	RSAPrivGiantKey		*gKey);
	
void rsaClearFullGiantKey(
	RSAFullGiantKey		*gKey);
	
#ifdef __cplusplus
}
#endif

#endif	/* _LIBGRSA_RSA_GIANT_KEY_H_ */

