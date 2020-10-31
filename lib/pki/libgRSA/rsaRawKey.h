/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * rsaRawKey.h - Typedefs and declarations for RSA keys in raw format
 *
 * Created Nov. 29 2005 by Doug Mitchell.
 */

#ifndef	_LIBGRSA_RSA_RAW_KEY_H_
#define _LIBGRSA_RSA_RAW_KEY_H_

#include <libgRSA/libgRSA.h>
#include <libgRSA/rsaGiantKey.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Generic forms of RSA keys, public and private forms. 
 * All fields are in raw, unformatted binary format, as a byte pointer
 * and a length, with the most significant byte first.
 *
 * The recip field in RSAPubKey is optional; it's the reciprocal of 
 * the modulus n. Providing this field is an optimization; if it's not 
 * provided, it will be calculated on the fly at some runtime cost. The 
 * recip field is not sensitive; it can be distributed and stored with 
 * no more security concern than the modulus itself. Providing the recip
 * field yields approximately 28% speedup on public key operations.  
 * 
 * See libgRSAUtils/rsaKeyGen.[ch] or Tools/rsaKeyTool for info
 * on obtaining the reciprocal. 
 */
typedef struct {
	unsigned char		*n;			/* modulus */
	gi_uint16			nLen;
	unsigned char		*recip;		/* reciprocal of modulus */
	gi_uint16			recipLen;
	unsigned char		*e;			/* public exponent */
	gi_uint16			eLen;
} RSAPubKey;

/* Private key, CRT form  */
typedef struct {
	unsigned char		*p;
	gi_uint16			pLen;
	unsigned char		*q;
	gi_uint16			qLen;
	unsigned char		*dp;
	gi_uint16			dpLen;
	unsigned char		*dq;
	gi_uint16			dqLen;
	unsigned char		*qInv;
	gi_uint16			qInvLen;
} RSAPrivKey;

/* raw (n, e) --> public RSAPubGiantKey */
RSAStatus rsaInitPubGKey(
	const RSAPubKey		*apiKey,
	RSAPubGiantKey		*pubKey);

/* raw (p, q, dp, dq, qInv) --> private RSAPrivGiantKey */
RSAStatus rsaInitPrivGKey(
	const RSAPrivKey	*apiKey,
	RSAPrivGiantKey	*privKey);
	

#ifdef __cplusplus
}
#endif

#endif	/* _LIBGRSA_RSA_CORE_H_ */
