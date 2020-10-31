/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * rsaPadding.h - PKCS1 and OAEP padding support.
 *
 * Created Aug. 10 2005 by Doug Mitchell.
 */

#ifndef	_LIBGRSA_RSA_PADDING_H_
#define _LIBGRSA_RSA_PADDING_H_

#include <libgRSA/libgRSA.h>
#include <libGiants/giantIntegers.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PKCS1 padding markers */
#define RSA_PKCS1_PAD_SIGN		0x01
#define RSA_PKCS1_PAD_ENCRYPT	0x02

/* 
 * RP_None padding routines - just check length and convert between
 * byte array and giant 
 */
RSAStatus rsaAddNoPadding(
	const unsigned char	*plainText,
	gi_uint16			plainTextLen,	
	giant				result);		/* RETURNED */

RSAStatus rsaCheckNoPadding(
	giant				decryptResult,
	unsigned char		*plainText,		/* data RETURNED here */
	gi_uint16			*plainTextLen);	/* IN/OUT */

/*
 * Specialized "no padding" check for signature verify operation.
 * We don't deserialize the decrypted giant; we do a byte-for-
 * byte compare of the expected decrypted value against memory
 * passed in to us, in-place in the decrypted giant. 
 * This saves two stack-allocated byte arrays, each of 
 * MAX_PRIME_SIZE_BYTES - one here, and one in RSA_SigVerify().
 */
RSAStatus rsaCheckNoPaddingForSigVfy(
	giant				decryptResult,
	const unsigned char	*expectPlainText,
	gi_uint16			expectPlainTextLen);

RSAStatus rsaAddPkcs1Padding(
	const unsigned char	*plainText,
	gi_uint16			plainTextLen,	
	gi_uint16			totalLength,	/* intended total length */
	unsigned char		padMarker,		/* RSA_PKCS1_PAD_{SIGN,ENCRYPT} */
	RSARngCallback		*rngCallback,	/* required for RP_PKCS1, RP_OAEP */
	giant				result);
	
/* 
 * Given decrypted plaintext in the form of a giant, which should 
 * be the same size as n, verify proper PKCS1 padding and copy
 * resulting plaintext and length into caller-supplied buffer. 
 * On entry, *plainTextLen indicates the size of the available
 * plaintext buffer. On return, *plainTextLen indicates the 
 * number of valid bytes of plaintext recovered. 
 */
RSAStatus rsaCheckPkcs1Padding(
	giant				decryptResult,
	unsigned char		padMarker,		/* RSA_PKCS1_PAD_{SIGN,ENCRYPT} */
	unsigned char		*plainText,		/* data RETURNED here */
	gi_uint16			*plainTextLen);	/* IN/OUT */
	
/* 
 * Specialized PKCS1 padding check for signature verify operation.
 * We don't deserialize the decrypted giant; we check for padding 
 * in place (in the decrypted giant's n[]), AND we do a byte-for-
 * byte compare of the expected decrypted value against memory
 * passed in to us. This saves two stack-allocated byte arrays,
 * each of MAX_PRIME_SIZE_BYTES - one here, and one in RSA_SigVerify().
 *
 * Returns:
 *  -- RSA_Success, padding good and result compares OK
 *  -- RSA_VerifyFail, padding good but data miscompare
 *  -- RSA_BadSigFormat, bad padding
 *  -- RSA_Overflow, expectPlainText too big for this key with PKCS1 padding
 *  -- else usual gross errors
 */
RSAStatus rsaCheckPkcs1PaddingForSigVfy(
	giant				decryptResult,
	const unsigned char	*expectPlainText,
	gi_uint16			expectPlainTextLen);

#ifdef __cplusplus
}
#endif

#endif	/* _LIBGRSA_RSA_PADDING_H_ */

