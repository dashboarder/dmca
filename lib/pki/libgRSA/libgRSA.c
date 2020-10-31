/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * libgRSA.h - public API for RSA library based on libGiants
 *
 * Created Aug. 9 2005 by Doug Mitchell.
 */
 
#include <libgRSA/libgRSA.h>
#include <libgRSA/libgRSA_config.h>
#include <libgRSA/rsaPadding.h>
#include <libgRSA/libgRSA_priv.h>
#include <libGiants/giantIntegers.h>
#include <libGiants/giantExternal.h>
#include <libGiants/giantMod.h>
#include <libGiants/giantDebug.h>

#define SIG_DEBUG	0
#if SIG_DEBUG
static void showGiant(const char *label, giant g)
{
	printf("%s: ", label);
	printGiantHex(g);
}
#else
#define showGiant(l, g)
#endif

#if		RSA_PUB_KEY_ENABLE

/* lazy evaluation RSAPubGiantKey->recip */
static void rsaEnsurePubRecip(
	RSAPubGiantKey *pubKey)
{
	GI_CHECK_SP("rsaEnsurePubRecip");
	if(pubKey->recip.g.sign != 0) {
		return;
	}
	GI_LOG_SP("rsaEnsurePubRecip calling make_recip");
	make_recip(&pubKey->n.g, &pubKey->recip.g);
}

#endif	/* RSA_PUB_KEY_ENABLE */

#if		RSA_PRIV_KEY_ENABLE

/* lazy evaluation RSAPrivGiantKey->{pRecip,qRecip} */
static void rsaEnsurePrivRecip(
	RSAPrivGiantKey *privKey)
{
	GI_CHECK_SP("rsaEnsurePrivRecip");
	if(privKey->pRecip.g.sign != 0) {
		return;
	}
	GI_LOG_SP("rsaEnsurePrivbRecip calling make_recip");
	make_recip(&privKey->p.g, &privKey->pRecip.g);
	make_recip(&privKey->q.g, &privKey->qRecip.g);
}

#endif	/* RSA_PRIV_KEY_ENABLE */

#if RSA_SIGN_ENABLE

RSAStatus RSA_Sign(
	RSAPrivGiantKey				*privKey,		/* private key */
	RSAPadding					padding,		/* RP_None or RP_PKCS1 */
	const gi_uint8				*dataToSign,	/* signature over this data */
	gi_uint16					dataToSignLen,	/* length of dataToSign */
	#if RSA_SIG_SHARE_GIANT
	RSASignBuffer				*signBuffer,	/* signature RETURNED here */
	#else
	gi_uint8					*sig,			/* signature, RETURNED */
	#endif
	gi_uint16					*sigLen)		/* IN/OUT if !RSA_SIG_SHARE_GIANT */
												/* OUT only if RSA_SIG_SHARE_GIANT */
{
	RSAStatus rtn = RSA_Success;
	lgiant ptSigGiant;				/* in/out for powermodCRT */
	gi_uint16 modSize;
	GIReturn grtn;
	#if		!RSA_SIG_SHARE_GIANT
	/* powermodCRT needs this one way or another */
	lgiant	powermodScratch;
	#endif
	
	if((dataToSign == NULL) || (sigLen == NULL) || (privKey == NULL)) {
		return RSA_ParamErr;
	}
	#if		RSA_SIG_SHARE_GIANT
	if(signBuffer == NULL) {
		return RSA_ParamErr;
	}
	#else
	if(sig == NULL) {
		return RSA_ParamErr;
	}
	localGiantAlloc(&powermodScratch);
	#endif	/* RSA_SIG_SHARE_GIANT */
	
	GI_CHECK_SP("RSA_Sign");
	localGiantAlloc(&ptSigGiant);
	
	rsaEnsurePrivRecip(privKey);
	modSize = ((bitlen(&privKey->p.g) + bitlen(&privKey->q.g)) + 7) / 8;
	switch(padding) {
		case RP_None:
			rtn = rsaAddNoPadding(dataToSign, dataToSignLen, &ptSigGiant.g);
			break;
		
		#if		RSA_PADDING_PKCS1_ENABLE
		case RP_PKCS1:
			rtn = rsaAddPkcs1Padding(dataToSign, dataToSignLen,
				modSize, RSA_PKCS1_PAD_SIGN, 
				NULL,				/* no RNG for signing */
				&ptSigGiant.g);
			break;
		#endif	/* RSA_PADDING_PKCS1_ENABLE */
		
		default:
			rtn = RSA_Unimplemented;
			break;
	}
	if(rtn) {
		rsaDebug1("RSA_Sign: AddPadding returned %s\n", rsaStatusStr(rtn));
		goto errOut;
	}

	#if		RSA_SIG_SHARE_GIANT
	/* 
	 * We init this. Not caller, not powermodCRT. But not before
	 * we're done with the signature buffer, which could be the same
	 * as dataToSign.
	 */
	localGiantAlloc(&signBuffer->g);
	#endif	/* RSA_SIG_SHARE_GIANT */
	
	/* sig := (ptext ^ d) mod n */
	showGiant("sign ptGiant", &ptSigGiant.g);
	powermodCRT(&ptSigGiant.g, 
		&privKey->p.g, &privKey->pRecip.g,
		&privKey->q.g, &privKey->qRecip.g,
		&privKey->dp.g, &privKey->dq.g,
		&privKey->qInv.g,
		#if RSA_SIG_SHARE_GIANT
		&signBuffer->g.g,
		#else
		&powermodScratch.g,
		#endif
		&ptSigGiant.g);
	showGiant("sign sig    ", &ptSigGiant.g);
	
	#if		RSA_SIG_SHARE_GIANT
	/* our sigLen is not IN/OUT, but the arg to serializeGiant is */
	*sigLen = sizeof(signBuffer->signature);
	grtn = serializeGiant(&ptSigGiant.g, signBuffer->signature, sigLen);
	#else
	grtn = serializeGiant(&ptSigGiant.g, sig, sigLen);
	#endif	/* RSA_SIG_SHARE_GIANT */
	if(grtn) {
		rtn = giantStatusToRSA(grtn);
		rsaDebug1("RSA_Sign: serializeGiant returned %s\n", rsaStatusStr(rtn));
	}
errOut:
	clearGiant(&ptSigGiant.g);
	return rtn;
}
#endif	/* RSA_SIGN_ENABLE */

#if		RSA_VERIFY_ENABLE

extern RSAStatus RSA_SigVerify(
	RSAPubGiantKey		*pubKey,		/* public key */
	RSAPadding			padding,		/* RP_None or RP_PKCS1 */
	const gi_uint8		*signedData,	/* signature over this data */
	gi_uint16			signedDataLen,	/* length of dataToSign */
	const gi_uint8		*sig,			/* signature */
	gi_uint16			sigLen)			/* length of signature */
{
	RSAStatus rtn = RSA_Success;
	lgiant ptSigGiant;		/* plain and decrypted signature */
	GIReturn grtn;
	
	GI_CHECK_SP("RSA_SigVerify");
	GI_LOG_SP("RSA_SigVerify top");
	if((signedData == NULL) || (sig == NULL)) {
		return RSA_ParamErr;
	}
	if(pubKey == NULL) {
		return RSA_ParamErr;
	}

	localGiantAlloc(&ptSigGiant);
	rsaEnsurePubRecip(pubKey);
	
	/* signature --> giant */
	grtn = deserializeGiant(&ptSigGiant.g, sig, sigLen);
	if(grtn) {
		rtn = giantStatusToRSA(grtn);
		rsaDebug1("RSA_SigVerify: deserializeGiant (sig) returned %s\n", 
			rsaStatusStr(rtn));
		return rtn;
	}

	/* decrytped sig := (sig ^ e) mod n */
	/* powermodg allows base and result to reside in same giant */
	GI_LOG_SP("calling powermodg");
	showGiant("vfy  ptGiant", &ptSigGiant.g);
	
	powermodg(&ptSigGiant.g, &pubKey->e.g, 
		&pubKey->n.g, &pubKey->recip.g, 
		&ptSigGiant.g);

	showGiant("vfy  sig    ", &ptSigGiant);
	
	/* check and strip padding, verify data */
	switch(padding) {
		case RP_None:
			rtn = rsaCheckNoPaddingForSigVfy(&ptSigGiant.g, signedData, signedDataLen);
			break;
		
		#if		RSA_PADDING_PKCS1_ENABLE
		case RP_PKCS1:
			rtn = rsaCheckPkcs1PaddingForSigVfy(&ptSigGiant.g, signedData, signedDataLen);
			break;
		#endif	/* RSA_PADDING_PKCS1_ENABLE */
		
		default:
			rtn = RSA_Unimplemented;
			break;
	}
	if(rtn) {
		rsaDebug1("RSA_SigVerify: CheckPadding returned %s\n", rsaStatusStr(rtn));
	}

	clearGiant(&ptSigGiant.g);
	return rtn;
}
#endif	/* RSA_VERIFY_ENABLE */

#if		RSA_ENCRYPT_ENABLE

extern RSAStatus RSA_Encrypt(
	RSAPubGiantKey	*pubKey,			/* public key */
	RSAPadding		padding,			/* RP_None, RP_PKCS1, RP_OAEP */
	RSARngCallback	*rngCallback,		/* required for RP_PKCS1, RP_OAEP */
	const gi_uint8	*plainText,
	gi_uint16		plainTextLen,		/* length of plainText */
	gi_uint8		*cipherText,	
	gi_uint16		*cipherTextLen)		/* IN/OUT */
{
	RSAStatus rtn = RSA_Success;
	lgiant ptCtGiant;
	GIReturn grtn;
	gi_uint16 modSize;
	
	if((plainText == NULL) || (cipherText == NULL) || (cipherTextLen == NULL)) {
		return RSA_ParamErr;
	}
	if(pubKey == NULL) {
		return RSA_ParamErr;
	}
	
	GI_CHECK_SP("RSA_Encrypt");
	localGiantAlloc(&ptCtGiant);
	rsaEnsurePubRecip(pubKey);

	modSize = giantNumBytes(&pubKey->n.g);
	switch(padding) {
		case RP_None:
			rtn = rsaAddNoPadding(plainText, plainTextLen, &ptCtGiant.g);
			break;
		
		#if		RSA_PADDING_PKCS1_ENABLE
		case RP_PKCS1:
			rtn = rsaAddPkcs1Padding(plainText, plainTextLen,
				modSize, RSA_PKCS1_PAD_ENCRYPT, 
				rngCallback,
				&ptCtGiant.g);
			break;
		#endif	/* RSA_PADDING_PKCS1_ENABLE */
		
		/* OEAP goes here eventually */
		
		default:
			rtn = RSA_Unimplemented;
			break;
	}
	if(rtn) {
		rsaDebug1("RSA_Encrypt: AddPadding returned %s\n", rsaStatusStr(rtn));
		goto errOut;
	}

	/* ctext := (ptext ^ e) mod n */
	/* ptext and ctext can share the same giant */
	powermodg(&ptCtGiant.g, &pubKey->e.g, 
		&pubKey->n.g, &pubKey->recip.g, 
		&ptCtGiant.g);
	
	/* out to caller */
	grtn = serializeGiant(&ptCtGiant.g, cipherText, cipherTextLen);
	if(grtn) {
		rtn = giantStatusToRSA(grtn);
		rsaDebug1("RSA_Encrypt: serializeGiant returned %s\n", rsaStatusStr(rtn));
	}
errOut:
	clearGiant(&ptCtGiant.g);
	return rtn;
}
#endif	/* RSA_ENCRYPT_ENABLE */

#if		RSA_DECRYPT_ENABLE

RSAStatus RSA_Decrypt(
	RSAPrivGiantKey				*privKey,			/* private key */
	RSAPadding					padding,			/* RP_None, RP_PKCS1, RP_OAEP */
	const gi_uint8				*cipherText,
	gi_uint16					cipherTextLen,		/* length of cipherText */
	gi_uint8					*plainText,	
	gi_uint16					*plainTextLen)		/* IN/OUT */
{
	RSAStatus rtn = RSA_Success;
	lgiant ptCtGiant;	
	GIReturn grtn;
	lgiant	powermodScratch;

	if((plainText == NULL) || (cipherText == NULL) || (plainTextLen == NULL) || (privKey == NULL)) {
		return RSA_ParamErr;
	}

	GI_CHECK_SP("RSA_Decrypt");
	localGiantAlloc(&ptCtGiant);
	localGiantAlloc(&powermodScratch);
	
	rsaEnsurePrivRecip(privKey);

	/* ciphertext --> giant */
	grtn = deserializeGiant(&ptCtGiant.g, cipherText, cipherTextLen);
	if(grtn) {
		rtn = giantStatusToRSA(grtn);
		rsaDebug1("RSA_Decrypt: deserializeGiant (sig) returned %s\n", 
			rsaStatusStr(rtn));
		goto errOut;
	}

	/* plaintext := (ciphertext ^ d) mod n */
	powermodCRT(&ptCtGiant.g, 
		&privKey->p.g, &privKey->pRecip.g,
		&privKey->q.g, &privKey->qRecip.g,
		&privKey->dp.g, &privKey->dq.g,
		&privKey->qInv.g,
		&powermodScratch.g,
		&ptCtGiant.g);	
	
	/* check and strip padding */
	switch(padding) {
		case RP_None:
			rtn = rsaCheckNoPadding(&ptCtGiant.g, plainText, plainTextLen);
			break;
		
		#if		RSA_PADDING_PKCS1_ENABLE
		case RP_PKCS1:
			rtn = rsaCheckPkcs1Padding(&ptCtGiant.g, RSA_PKCS1_PAD_ENCRYPT,
				plainText, plainTextLen);
			break;
		#endif	/* RSA_PADDING_PKCS1_ENABLE */
		
		/* OAEP here */
		
		default:
			rtn = RSA_Unimplemented;
			break;
	}
	if(rtn) {
		rsaDebug1("RSA_Decrypt: CheckPadding returned %s\n", rsaStatusStr(rtn));
	}

errOut:
	clearGiant(&ptCtGiant.g);
	return rtn;
}

#endif	/* RSA_DECRYPT_ENABLE */

