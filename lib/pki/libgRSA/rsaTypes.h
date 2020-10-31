/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * rsaTypes.h - typedefs for libgRSA
 *
 * Created Nov. 30 2005 by Doug Mitchell.
 */
 
#ifndef	_LIBRSA_RSA_TYPES_H_
#define _LIBRSA_RSA_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libGiants/giantTypes.h>

/* 
 * Error returns from this library. 
 */
typedef enum {
	RSA_Success	= 0,
	RSA_BadKeyData = RSA_RETURN_BASE,	/* bad raw key data */
	RSA_ParamErr,						/* parameter error */
	RSA_IncompleteKey,					/* one or more RSAKey fields missing */
	RSA_VerifyFail,						/* signature verification failed */
	RSA_BadSigFormat,					/* improperly formatted signature */
	RSA_BadDataFormat,					/* improperly formatted ciphertext */
	RSA_BadKeyFormat,					/* improperly formatted public key */
	RSA_InternalError,					/* internal error */
	RSA_Overflow,						/* insufficent buffer provided */
	RSA_Unimplemented,					/* unimplemented in this configuration */
	RSA_IllegalKeySize,					/* Unsupported key size */
	RSA_OtherError
} RSAStatus;

/* 
 * Padding styles
 */
typedef enum {
	RP_None = 0,		/* no padding */
	RP_PKCS1,			/* PKCS1, for signing and encrypting */
	RP_OAEP				/* OAEP, for encrypting only */
} RSAPadding;

/* 
 * Random number callback function. The caller of RSA_Encrypt or
 * RSA_GenKeyPair supplies pseudorandom data for OAEP or PKCS1 padding
 * generation or for key generation, via this callback. 
 * 
 * The destination of the random data is *randBuf, which is memory owned
 * and maintained by the caller of this function.
 *
 * This function should return nonzero if it failed to obtain the number of
 * random bytes requested.
 */
typedef int (RSARngCallback)(
	gi_uint8 *randBuf,			/* random data goes here */
	gi_uint16 randSize);		/* byte count */

#ifdef __cplusplus
}
#endif

#endif	/* _LIBRSA_RSA_TYPES_H_ */

