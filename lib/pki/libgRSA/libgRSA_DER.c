/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * libgRSA_DER.c - DER encoding/decoding routines
 */
 
#include <libgRSA/libgRSA_DER.h>
#include <libgRSA/libgRSA_priv.h>
#include <libgRSA/libgRSA_config.h>
#include <libGiants/giantPlatform.h>
#include <libGiants/giantExternal.h>
#include <libGiants/giantMod.h>
#include <libGiants/giantDebug.h>
#include <libDER/DER_Keys.h>
#include <libDER/asn1Types.h>

#ifndef	RSA_PKCS1_KEY_ENABLE
#error Please define RSA_PKCS1_KEY_ENABLE.
#endif

#if		RSA_PKCS1_KEY_ENABLE

#if		RSA_PUB_KEY_ENABLE || RSA_PRIV_KEY_ENABLE

/* init an lgiant, fill it with incoming bytes */
static RSAStatus rsaSetGiant(
	lgiant *g,
	DERItem *item)
{
	localGiantAlloc(g);
	if(deserializeGiant(&g->g, item->data, item->length)) {
		return RSA_BadKeyData;
	}
	return RSA_Success;
}

#endif	/* RSA_PUB_KEY_ENABLE || RSA_PRIV_KEY_ENABLE */

/* 
 * Decode PKCS1 form keys into RSA{Pub,Priv}GiantKey. 
 */

#if		RSA_PUB_KEY_ENABLE

RSAStatus RSA_DecodePubKey(
	const unsigned char *pkcs1Key,
	gi_uint16			pkcs1KeyLen,
	RSAPubGiantKey		*pubKey)		/* RETURNED */
{
	DERReturn drtn;
	DERItem keyItem = {(DERByte *)pkcs1Key, pkcs1KeyLen};
	DERRSAPubKeyPKCS1 decodedKey;
	RSAStatus rrtn;
	
	GI_CHECK_SP("RSA_DecodePubKey");
	Bzero(pubKey, sizeof(*pubKey));
	drtn = DERParseSequence(&keyItem,
		DERNumRSAPubKeyPKCS1ItemSpecs, DERRSAPubKeyPKCS1ItemSpecs,
		&decodedKey, sizeof(decodedKey));
	if(drtn) {
		return RSA_BadKeyData;
	}
	
	/* make_recip called lazily when the key is actually used */
	localGiantAlloc(&pubKey->recip);
	
	rrtn = rsaSetGiant(&pubKey->n, &decodedKey.modulus);
	if(rrtn) {
		return rrtn;
	}
	return rsaSetGiant(&pubKey->e, &decodedKey.pubExponent);
}

RSAStatus RSA_DecodeApplePubKey(
	const unsigned char *pkcs1Key,
	gi_uint16			pkcs1KeyLen,
	RSAPubGiantKey		*pubKey)		/* RETURNED */
{
	DERReturn drtn;
	DERItem keyItem = {(DERByte *)pkcs1Key, pkcs1KeyLen};
	DERRSAPubKeyApple decodedKey;
	RSAStatus rrtn;
	
	Bzero(pubKey, sizeof(*pubKey));
	drtn = DERParseSequence(&keyItem,
		DERNumRSAPubKeyAppleItemSpecs, DERRSAPubKeyAppleItemSpecs,
		&decodedKey, sizeof(decodedKey));
	if(drtn) {
		return RSA_BadKeyData;
	}
	
	rrtn = rsaSetGiant(&pubKey->e, &decodedKey.pubExponent);
	if(rrtn) {
		return rrtn;
	}
	rrtn = rsaSetGiant(&pubKey->n, &decodedKey.modulus);
	if(rrtn) {
		return rrtn;
	}
	rrtn = rsaSetGiant(&pubKey->recip, &decodedKey.reciprocal);
	if(rrtn) {
		return rrtn;
	}
	return RSA_Success;
}

#endif	/* RSA_PUB_KEY_ENABLE */

#if		RSA_PRIV_KEY_ENABLE

/* init an rsaSgiant, fill it with incoming bytes */
static RSAStatus rsaSetSGiant(
	rsaSgiant *g,
	DERItem *item)
{
	localSmallGiantAlloc(g);
	if(deserializeGiant(&g->g, item->data, item->length)) {
		return RSA_BadKeyData;
	}
	return RSA_Success;
}

RSAStatus RSA_DecodePrivKey(
	const unsigned char *pkcs1Key,
	gi_uint16			pkcs1KeyLen,
	RSAPrivGiantKey		*privKey)		/* RETURNED */
{
	DERReturn drtn;
	DERItem keyItem = {(DERByte *)pkcs1Key, pkcs1KeyLen};
	DERRSAPrivKeyCRT decodedKey;

	Bzero(privKey, sizeof(*privKey));
	drtn = DERParseSequence(&keyItem,
		DERNumRSAPrivKeyCRTItemSpecs, DERRSAPrivKeyCRTItemSpecs,
		&decodedKey, sizeof(decodedKey));
	if(drtn) {
		return RSA_BadKeyData;
	}
	if(rsaSetSGiant(&privKey->p, &decodedKey.p)) {
		return RSA_BadKeyData;
	}
	if(rsaSetSGiant(&privKey->q, &decodedKey.q)) {
		return RSA_BadKeyData;
	}
	if(rsaSetSGiant(&privKey->dp, &decodedKey.dp)) {
		return RSA_BadKeyData;
	}
	if(rsaSetSGiant(&privKey->dq, &decodedKey.dq)) {
		return RSA_BadKeyData;
	}
	if(rsaSetSGiant(&privKey->qInv, &decodedKey.qInv)) {
		return RSA_BadKeyData;
	}
	
	/* pRecip, qRecip evaluated lazily */
	localSmallGiantAlloc(&privKey->pRecip);
	localSmallGiantAlloc(&privKey->qRecip);

	return RSA_Success;
}

RSAStatus RSA_DecodeKeyPair(
	const unsigned char *pkcs1Key,
	gi_uint16			pkcs1KeyLen,
	RSAFullGiantKey		*keyPair)		/* RETURNED */
{
	DERReturn drtn;
	DERItem keyItem = {(DERByte *)pkcs1Key, pkcs1KeyLen};
	DERRSAKeyPair decodedKey;

	Bzero(keyPair, sizeof(*keyPair));
	drtn = DERParseSequence(&keyItem,
		DERNumRSAKeyPairItemSpecs, DERRSAKeyPairItemSpecs,
		&decodedKey, sizeof(decodedKey));
	if(drtn) {
		return RSA_BadKeyData;
	}
	if(rsaSetGiant(&keyPair->pub.n, &decodedKey.n)) {
		return RSA_BadKeyData;
	}
	if(rsaSetGiant(&keyPair->pub.e, &decodedKey.e)) {
		return RSA_BadKeyData;
	}
	if(rsaSetGiant(&keyPair->d, &decodedKey.d)) {
		return RSA_BadKeyData;
	}
	if(rsaSetSGiant(&keyPair->priv.p, &decodedKey.p)) {
		return RSA_BadKeyData;
	}
	if(rsaSetSGiant(&keyPair->priv.q, &decodedKey.q)) {
		return RSA_BadKeyData;
	}
	if(rsaSetSGiant(&keyPair->priv.dp, &decodedKey.dp)) {
		return RSA_BadKeyData;
	}
	if(rsaSetSGiant(&keyPair->priv.dq, &decodedKey.dq)) {
		return RSA_BadKeyData;
	}
	if(rsaSetSGiant(&keyPair->priv.qInv, &decodedKey.qInv)) {
		return RSA_BadKeyData;
	}
	
	/* recip, pRecip, qRecip evaluated lazily */
	localGiantAlloc(&keyPair->pub.recip);
	localSmallGiantAlloc(&keyPair->priv.pRecip);
	localSmallGiantAlloc(&keyPair->priv.qRecip);

	return RSA_Success;
}

#endif	/* RSA_PRIV_KEY_ENABLE */

#if		RSA_KEY_GENERATE_ENABLE

#define RSA_MAX_GIANT_SIZE	MAX_SERIALIZED_GIANT_SIZE(GIANT_LITTLE_DIGITS)

/* 
 * Munge a giant (from e.g. a RSAFullGiantKey) into a gi_uint8 array
 * and then into a DERItem suitable for encoding. 
 */
static RSAStatus rsaGiantToDerItem(
	giant		g,
	gi_uint8	*cp,
	gi_uint16	cpLen,
	DERItem		*derItem)
{
	GIReturn	grtn;
	gi_uint16	ioLen = cpLen;
	gi_uint16	dex;
	
	/* serialize the giant into DER content */
	grtn = serializeGiant(g, cp, &ioLen);
	if(grtn) {
		return giantStatusToRSA(grtn);
	}
	
	/* convert the result into a DERItem */
	derItem->data = cp;
	derItem->length = ioLen;

	/* trim off leading zeroes (but leave one zero if that's all there is) */
	for(dex=0; dex<(ioLen-1); dex++) {
		if(*(derItem->data) != 0) {
			break;
		}
		derItem->data++;
		derItem->length--;
	}
	return RSA_Success;
}

#if		RSA_PUB_KEY_ENABLE

/* DER encode a public key, PKCS1 format */
RSAStatus RSA_EncodePubKey(
	RSAPubGiantKey		*giantKey,
	gi_uint8			*pkcs1Key,		/* data written here */
	gi_uint32			*inOutLen)		/* IN/OUT */
{
	DERRSAPubKeyPKCS1	derKey;
	DERReturn			drtn;
	RSAStatus			rrtn;
	DERByte				*der = pkcs1Key;
	DERSize				ioLen = *inOutLen;
	
	/* serialized giant byte arrays */
	gi_uint8	nChars[RSA_MAX_GIANT_SIZE];
	gi_uint8	eChars[RSA_MAX_GIANT_SIZE];
	
	/* giants --> DERItems */
	rrtn = rsaGiantToDerItem(&giantKey->n.g, nChars, RSA_MAX_GIANT_SIZE, 
		&derKey.modulus);
	if(rrtn) {
		return rrtn;
	}
	rrtn = rsaGiantToDerItem(&giantKey->e.g, eChars, RSA_MAX_GIANT_SIZE, 
		&derKey.pubExponent);
	if(rrtn) {
		return rrtn;
	}
	
	/* encode */
	drtn = DEREncodeSequence(ASN1_CONSTR_SEQUENCE,
		&derKey,
		DERNumRSAPubKeyPKCS1ItemSpecs, DERRSAPubKeyPKCS1ItemSpecs,
		der,
		&ioLen);
	if(drtn) {
		return derStatusToRSA(drtn);
	}
	
	/* success */
	*inOutLen = ioLen;
	return RSA_Success;
}

/* DER encode a public key, custom (to this library) format with reciprocal */
RSAStatus RSA_EncodeApplePubKey(
	RSAPubGiantKey		*giantKey,
	gi_uint8			*pubKey,		/* data written here */
	gi_uint32			*inOutLen)		/* IN/OUT */
{
	DERRSAPubKeyApple	derKey;
	DERReturn			drtn;
	RSAStatus			rrtn;
	DERByte				*der = pubKey;
	DERSize				ioLen = *inOutLen;
	bgiant				recip;
	
	/* serialized giant byte arrays */
	gi_uint8	nChars[RSA_MAX_GIANT_SIZE];
	gi_uint8	rChars[RSA_MAX_GIANT_SIZE];
	gi_uint8	eChars[RSA_MAX_GIANT_SIZE];
	
	/* calculate the reciprocal */
	localBigGiantAlloc(&recip);
	make_recip(&giantKey->n.g, &recip.g);
	
	/* giants --> DERItems */
	rrtn = rsaGiantToDerItem(&giantKey->n.g, nChars, RSA_MAX_GIANT_SIZE, 
		&derKey.modulus);
	if(rrtn) {
		return rrtn;
	}
	rrtn = rsaGiantToDerItem(&recip.g, rChars, RSA_MAX_GIANT_SIZE, 
		&derKey.reciprocal);
	if(rrtn) {
		return rrtn;
	}
	rrtn = rsaGiantToDerItem(&giantKey->e.g, eChars, RSA_MAX_GIANT_SIZE, 
		&derKey.pubExponent);
	if(rrtn) {
		return rrtn;
	}
	
	/* encode */
	drtn = DEREncodeSequence(ASN1_CONSTR_SEQUENCE,
		&derKey,
		DERNumRSAPubKeyAppleItemSpecs, DERRSAPubKeyAppleItemSpecs,
		der,
		&ioLen);
	if(drtn) {
		return derStatusToRSA(drtn);
	}
	
	/* success */
	*inOutLen = ioLen;
	return RSA_Success;
}

#endif	/* RSA_PUB_KEY_ENABLE */

#if		RSA_PRIV_KEY_ENABLE

/* 
 * Create a PKCS1-formattted private key given a RSAFullGiantKey.
 * The inOutLen param contains the number of bytes available in
 * *pkcs1Key on entry, adn contains the number of bytes actually
 * written to *pkcs1Key on successful return.
 */
RSAStatus RSA_EncodeKeyPair(
	RSAFullGiantKey		*keyPair,
	gi_uint8			*pkcs1Key,		/* data written here */
	gi_uint32			*inOutLen)		/* IN/OUT */
{
	DERRSAKeyPair		derKey;
	DERReturn			drtn;
	RSAStatus			rrtn;
	DERByte				*der = pkcs1Key;
	DERSize				ioLen = *inOutLen;
	DERByte				version = 0;
	
	/* serialized giant byte arrays */
	gi_uint8	nChars[RSA_MAX_GIANT_SIZE];
	gi_uint8	eChars[RSA_MAX_GIANT_SIZE];
	gi_uint8	dChars[RSA_MAX_GIANT_SIZE];
	gi_uint8	pChars[RSA_MAX_GIANT_SIZE];
	gi_uint8	qChars[RSA_MAX_GIANT_SIZE];
	gi_uint8	dpChars[RSA_MAX_GIANT_SIZE];
	gi_uint8	dqChars[RSA_MAX_GIANT_SIZE];
	gi_uint8	qInvChars[RSA_MAX_GIANT_SIZE];
	
	derKey.version.data = &version;
	derKey.version.length = 1;
	
	/* giants --> DERItems */
	rrtn = rsaGiantToDerItem(&keyPair->pub.n.g, nChars, RSA_MAX_GIANT_SIZE, &derKey.n);
	if(rrtn) {
		return rrtn;
	}
	rrtn = rsaGiantToDerItem(&keyPair->pub.e.g, eChars, RSA_MAX_GIANT_SIZE, &derKey.e);
	if(rrtn) {
		return rrtn;
	}
	rrtn = rsaGiantToDerItem(&keyPair->d.g, dChars, RSA_MAX_GIANT_SIZE, &derKey.d);
	if(rrtn) {
		return rrtn;
	}
	rrtn = rsaGiantToDerItem(&keyPair->priv.p.g, pChars, RSA_MAX_GIANT_SIZE, &derKey.p);
	if(rrtn) {
		return rrtn;
	}
	rrtn = rsaGiantToDerItem(&keyPair->priv.q.g, qChars, RSA_MAX_GIANT_SIZE, &derKey.q);
	if(rrtn) {
		return rrtn;
	}
	rrtn = rsaGiantToDerItem(&keyPair->priv.dp.g, dpChars, RSA_MAX_GIANT_SIZE, &derKey.dp);
	if(rrtn) {
		return rrtn;
	}
	rrtn = rsaGiantToDerItem(&keyPair->priv.dq.g, dqChars, RSA_MAX_GIANT_SIZE, &derKey.dq);
	if(rrtn) {
		return rrtn;
	}
	rrtn = rsaGiantToDerItem(&keyPair->priv.qInv.g, qInvChars, RSA_MAX_GIANT_SIZE, &derKey.qInv);
	if(rrtn) {
		return rrtn;
	}
	
	/* encode */
	drtn = DEREncodeSequence(ASN1_CONSTR_SEQUENCE,
		&derKey,
		DERNumRSAKeyPairItemSpecs, DERRSAKeyPairItemSpecs,
		der,
		&ioLen);
	if(drtn) {
		return derStatusToRSA(drtn);
	}
	
	/* success */
	*inOutLen = ioLen;
	return RSA_Success;
}

#endif	/* RSA_PRIV_KEY_ENABLE */

#endif	/* RSA_KEY_GENERATE_ENABLE */

#endif	/* RSA_PKCS1_KEY_ENABLE */

