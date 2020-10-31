/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * libgRSA_config.h - #defines controlling what features are enabled in a
 *					  given implementation of this library.
 *
 * Created Aug. 9 2005 by Doug Mitchell.
 */
 
#ifndef	_LIBGRSA_LIBRSA_CONFIG_H_
#define _LIBGRSA_LIBRSA_CONFIG_H_

/* one flag for each basic RSA operation */
#define RSA_SIGN_ENABLE				1
#define RSA_VERIFY_ENABLE			1
#define RSA_ENCRYPT_ENABLE			1
#define RSA_DECRYPT_ENABLE			1

/* values inferred from above four flags - do not modify */
#define RSA_SIGVFY_ENABLE		(RSA_SIGN_ENABLE || RSA_VERIFY_ENABLE)
#define RSA_ENCRDECR_ENABLE		(RSA_ENCRYPT_ENABLE || RSA_DECRYPT_ENABLE)
#define RSA_PUB_KEY_ENABLE		(RSA_VERIFY_ENABLE || RSA_ENCRYPT_ENABLE)
#define RSA_PRIV_KEY_ENABLE		(RSA_SIGN_ENABLE || RSA_DECRYPT_ENABLE)

/* 
 * Enables DER encoding of DigestInfo for proper PKCS1 signatures
 * This is always disabled if !RSA_SIGVFY_ENABLE.
 */
#if		RSA_SIGVFY_ENABLE
#define RSA_DER_DIGEST_ENABLE		1	/* your platform's value here */ 
#else
#define RSA_DER_DIGEST_ENABLE		0	/* hard coded - do not change */ 
#endif

/* enable support for PKCS1 and OEAP padding */
#define RSA_PADDING_PKCS1_ENABLE	1
#define RSA_PADDING_OAEP_ENABLE		0	/* not supported yet */

/* enable support for PKCS1 format keys */
#define RSA_PKCS1_KEY_ENABLE		1

/* enable support for raw keys (rsaRawKey.[ch]) */
#define RSA_RAW_KEY_ENABLE			1

/* enable key pair generation */
#define RSA_KEY_GENERATE_ENABLE		1

#endif	/* _LIBGRSA_LIBRSA_CONFIG_H_ */
