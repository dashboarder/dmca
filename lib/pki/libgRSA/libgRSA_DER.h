/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * libgRSA_DER.h - DER encoding/decoding routines
 */
 
#ifndef	_LIBGRSA_LIBRSA_DER_H_
#define _LIBGRSA_LIBRSA_DER_H_

#include <libgRSA/libgRSA.h>
#include <libgRSA/libgRSA_config.h>
#include <libGiants/giantPlatform.h>
#include <libDER/DER_Encode.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * Decode PKCS1 form keys into RSA{Pub,Priv}GiantKey. 
 */
RSAStatus RSA_DecodePubKey(
	const unsigned char *pkcs1Key,
	gi_uint16			pkcs1KeyLen,
	RSAPubGiantKey		*pubKey);		/* RETURNED */
	
RSAStatus RSA_DecodePrivKey(
	const unsigned char *pkcs1Key,
	gi_uint16			pkcs1KeyLen,
	RSAPrivGiantKey		*privKey);		/* RETURNED */

RSAStatus RSA_DecodeKeyPair(
	const unsigned char *pkcs1Key,
	gi_uint16			pkcs1KeyLen,
	RSAFullGiantKey		*keyPair);		/* RETURNED */

/*
 * Decode an custom (to this library) public key with reciprocal.
 */
RSAStatus RSA_DecodeApplePubKey(
	const unsigned char *aplePubKey,
	gi_uint16			applePubKeyLen,
	RSAPubGiantKey		*pubKey);		/* RETURNED */

/* 
 * Generated key pair encoding support. 
 */
 
#define RSA_HALF_SIZE(keySizeInBytes) ((keySizeInBytes + 1) / 2)

/*
 * To allocate space on the stack for encoded public and private keys, 
 * use these macros to determine the max size needed. They are conservative;
 * they will almost certainly allocate more space than you really
 * end up needing for the encoded keys. 
 */
#define RSA_ENCODED_PRIV_KEY_SIZE(keySizeInBytes)							\
	DER_MAX_ENCODED_SIZE(													\
		DER_MAX_ENCODED_SIZE(1) +				/* version */				\
		DER_MAX_ENCODED_SIZE(keySizeInBytes) +	/* n */						\
		DER_MAX_ENCODED_SIZE(keySizeInBytes) +	/* e */						\
		DER_MAX_ENCODED_SIZE(keySizeInBytes) +	/* d */						\
		DER_MAX_ENCODED_SIZE(RSA_HALF_SIZE(keySizeInBytes)) +	/* p */		\
		DER_MAX_ENCODED_SIZE(RSA_HALF_SIZE(keySizeInBytes)) +	/* q */		\
		DER_MAX_ENCODED_SIZE(RSA_HALF_SIZE(keySizeInBytes)) +	/* dp */	\
		DER_MAX_ENCODED_SIZE(RSA_HALF_SIZE(keySizeInBytes)) +	/* dq */	\
		DER_MAX_ENCODED_SIZE(RSA_HALF_SIZE(keySizeInBytes)))	/* qInv */
		
#define RSA_ENCODED_PUB_KEY_SIZE(keySizeInBytes)				\
	DER_MAX_ENCODED_SIZE(										\
		DER_MAX_ENCODED_SIZE(keySizeInBytes) +		/* n */		\
		DER_MAX_ENCODED_SIZE(keySizeInBytes))		/* e */

#define RSA_ENCODED_APPLE_PUB_KEY_SIZE(keySizeInBytes)			\
	DER_MAX_ENCODED_SIZE(										\
		DER_MAX_ENCODED_SIZE(keySizeInBytes) +	/* n */		\
		DER_MAX_ENCODED_SIZE(keySizeInBytes) +	/* recip */	\
		GIANT_BYTES_PER_DIGIT +					/* recip: one extra digit */ \
		DER_MAX_ENCODED_SIZE(keySizeInBytes))	/* e */

/* 
 * Create PKCS1-formatted public key given a RSAFullGiantKey.
 * The inOutLen param contains the number of bytes available in
 * *pkcs1Key on entry, adn contains the number of bytes actually
 * written to *pkcs1Key on successful return. 
 */
RSAStatus RSA_EncodePubKey(
	RSAPubGiantKey		*giantKey,
	gi_uint8			*pkcs1Key,		/* data written here */
	gi_uint32			*inOutLen);		/* IN/OUT */
	
/* 
 * Create a PKCS1-formattted private key given a RSAFullGiantKey.
 * The inOutLen param contains the number of bytes available in
 * *pkcs1Key on entry, adn contains the number of bytes actually
 * written to *pkcs1Key on successful return.
 */
RSAStatus RSA_EncodeKeyPair(
	RSAFullGiantKey		*keyPair,
	gi_uint8			*pkcs1Key,		/* data written here */
	gi_uint32			*inOutLen);		/* IN/OUT */

/* 
 * Create a custom (to this library) DER-encoded public key including 
 * the reciprocal.
 * The inOutLen param contains the number of bytes available in
 * *pubKey on entry, adn contains the number of bytes actually
 * written to *pkcs1Key on successful return.
 */
RSAStatus RSA_EncodeApplePubKey(
	RSAPubGiantKey		*giantKey,
	gi_uint8			*pubKey,		/* data written here */
	gi_uint32			*inOutLen);		/* IN/OUT */

#ifdef __cplusplus
}
#endif

#endif	/* _LIBGRSA_LIBRSA_DER_H_ */

