/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * libgRSA.h - public API for RSA library based on libGiants
 */
 
#ifndef	_LIBGRSA_LIBRSA_H_
#define _LIBGRSA_LIBRSA_H_

#include <libgRSA/rsaTypes.h>
#include <libgRSA/rsaGiantKey.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * This file declares the main public functions in the libgRSA library. 
 * Keys for all of these functions are expressed in "RSAGiantKey" format, 
 * defined in rsaGiantKey.h. This is the native key format for this 
 * library. RSAGiantKeys can be obtained in one of two ways:
 *
 * -- from DER-encoded, PKCS1-style key blobs, using the interface in 
 *    libgRSA_DER.h.
 * -- from raw byte strings representing individual key components, using 
 *    the interface in rsaRawKey.h. 
 */
 
/*
 * RSA_SIG_SHARE_GIANT, when true, is an optimization to reduce the stack
 * requirements when performing an RSA_Sign operation. When true, caller
 * allocates an RSASignBuffer union which contains both a sratch giant
 * for use by lower level code and the buffer into which a signature
 * will be written at the completion of RSA_Sign(). The input, 
 * dataToSign, can also be in this RSASignBuffer (i.e., dataToSign
 * can equal RSASignBuffer.signature).
 */
#define RSA_SIG_SHARE_GIANT		0
#if		RSA_SIG_SHARE_GIANT

typedef union {
	lgiant		g;
	gi_uint8	signature[MAX_PRIME_SIZE_BYTES];
} RSASignBuffer;

#endif	/* RSA_SIG_SHARE_GIANT */


/* 
 * Given a private key and data to sign, generate a digital signature. 
 *
 * If the padding argument is RP_PKCS1, PKCS1 padding will be performed
 * prior to signing. If this argument is RP_None, the incoming data
 * will be signed "as is". (OAEP padding is not supported for signing.)
 * Note when PKCS1 padding is performed, the maximum length of data that 
 * can be signed is RSAKey.nLen-11 where RSAKey.nLen is the length of 
 * the modulus. 
 *
 * NOTE: using RP_None will not work if the first byte of dataToSign 
 * is zero; there is no way to verify leading zeroes as they got chopped 
 * off during the calculation. 

 *
 * If you want a proper PKCS1 style signature, with DER encoding of the 
 * incoming data - and the data is a SHA1 digest - use RSA_EncodeDigest
 * to format the data in an encoded DigestInfo struct before calling 
 * this routine. 
 *
 * If RSA_SIG_SHARE_GIANT is zero, caller allocates sig, the output buffer, 
 * into which the signature will be written. This must be at least as large 
 * as the key's modulus. On input, *sigLen is the size of the available buffer. 
 * On output it's the size of the signature actually written to sig. 
 *
 * If RSA_SIG_SHARE_GIANT is nonzero, caller allocates the signBuffer of
 * type RSASignBuffer; the signature will be written into 
 * signBuffer->signature and *sigLen will equal the size of the
 * signature actually written to signBuffer->signature.
 *
 * When memory usage is a critical issue, note that the input buffer
 * (dataToSign) can be the same as the output buffer (sig, or 
 * signBuffer->signature, depending on RSA_SIG_SHARE_GIANT). 
 *
 * Returns an RSAStatus as described in libgRSA.h. 
 */ 
 
#if		RSA_SIG_SHARE_GIANT

extern RSAStatus RSA_Sign(
	RSAPrivGiantKey				*privKey,		/* private key */
	RSAPadding					padding,		/* RP_None or RP_PKCS1 */
	const gi_uint8				*dataToSign,	/* signature over this data */
	gi_uint16					dataToSignLen,	/* length of dataToSign */
	RSASignBuffer				*signBuffer,	/* signature RETURNED here */
	gi_uint16					*sigLen);		/* signature RETURNED here */

#else	/* !RSA_SIG_SHARE_GIANT */

extern RSAStatus RSA_Sign(
	RSAPrivGiantKey				*privKey,		/* private key */
	RSAPadding					padding,		/* RP_None or RP_PKCS1 */
	const gi_uint8				*dataToSign,	/* signature over this data */
	gi_uint16					dataToSignLen,	/* length of dataToSign */
	gi_uint8					*sig,			/* signature, RETURNED */
	gi_uint16					*sigLen);		/* IN/OUT */

#endif	/* RSA_SIG_SHARE_GIANT */

/* 
 * Given a public key, data which has been signed, and a signature,
 * verify the signature. 
 *
 * If the padding argument is RP_PKCS1, PKCS1 padding will be performed
 * prior to verification. If this argument is RP_None, the incoming data
 * will be verified "as is". (OAEP padding is not supported for verifying.)
 * Note when PKCS1 padding is performed, the maximum length of data that 
 * can be signed and verified is RSAKey.nLen-11 where RSAKey.nLen is the 
 * length of the modulus. 
 *
 * If you are verifying a proper PKCS1 style signature, with DER encoding 
 * of the incoming data - and the data is a SHA1 digest - use 
 * DEREncodeSHA1DigestInfo to format the data in an encoded DigestInfo 
 * struct before calling this routine. 
 *
 * Returns an RSAStatus as described above. 
 */ 
extern RSAStatus RSA_SigVerify(
	RSAPubGiantKey			*pubKey,		/* public key */
	RSAPadding				padding,		/* RP_None or RP_PKCS1 */
	const gi_uint8			*signedData,	/* signature over this data */
	gi_uint16				signedDataLen,	/* length of dataToSign */
	const gi_uint8			*sig,			/* signature */
	gi_uint16				sigLen);		/* length of signature */
	
/* 
 * Encrypt a block of plaintext with a public key. 
 * Resulting ciphertext is the same size as the key's modulus
 * (RSAKey.nLen) and is allocated by the caller. 
 * The cipherTextLen argument, on input, specifies how much 
 * space is available for the ciphertext; on return, it
 * is the actual number of ciphertext bytes written.
 *
 * When performing RP_PKCS1 or RP_OAEP padding, you must supply 
 * a callback to a pseudorandom number generator via the rngFcn 
 * argument. 
 *
 * When memory usage is a critical issue, note that the input buffer
 * (plainText) can be the same as the output buffer (cipherText). 
 *
 * Returns an RSAStatus as described above. 
 */ 
extern RSAStatus RSA_Encrypt(
	RSAPubGiantKey		*pubKey,			/* public key */
	RSAPadding			padding,			/* RP_None, RP_PKCS1, RP_OAEP */
	RSARngCallback		*rngCallback,		/* required for RP_PKCS1, RP_OAEP */
	const gi_uint8		*plainText,
	gi_uint16			plainTextLen,		/* length of plainText */
	gi_uint8			*cipherText,	
	gi_uint16			*cipherTextLen);	/* IN/OUT */

/* 
 * Decrypt a block of ciphertext with a private key. The
 * ciphertext must be equal in length to the size of the 
 * key's modulus. The plaintext buffer is allocated by
 * the caller' en entry, *plainTextLen is the size of the 
 * available plaintext buffer; on return, it is the number
 * of plaintext bytes actually written. 
 *
 * When memory usage is a critical issue, note that the input buffer
 * (cipherText) can be the same as the output buffer (plainText). 
 *
 * Returns an RSAStatus as described above. 
 */ 
extern RSAStatus RSA_Decrypt(
	RSAPrivGiantKey				*privKey,			/* private key */
	RSAPadding					padding,			/* RP_None, RP_PKCS1, RP_OAEP */
	const gi_uint8				*cipherText,
	gi_uint16					cipherTextLen,		/* length of cipherText */
	gi_uint8					*plainText,	
	gi_uint16					*plainTextLen);		/* IN/OUT */


#ifdef __cplusplus
}
#endif

#endif	/* _LIBRSA_LIBRSA_H_ */

