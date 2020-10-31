/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * rsaRawKey.h - Typedefs and declarations for RSA keys in raw format
 *
 * Created Nov. 29 2005 by Doug Mitchell.
 */

#include <libgRSA/rsaRawKey.h>
#include <libGiants/giantExternal.h>
#include <libGiants/giantMod.h>
#include <libgRSA/libgRSA_config.h>

#ifndef	RSA_RAW_KEY_ENABLE
#error Please define RSA_RAW_KEY_ENABLE.
#endif

#if		RSA_RAW_KEY_ENABLE
#if		RSA_PUB_KEY_ENABLE || RSA_PRIV_KEY_ENABLE

/* init an lgiant, fill it with incoming bytes */
static RSAStatus rsaSetGiant(
	lgiant			*g,
	unsigned char	*c,
	gi_uint16		cLen)
{
	localGiantAlloc(g);
	if(deserializeGiant(&g->g, c, cLen)) {
		return RSA_BadKeyData;
	}
	return RSA_Success;
}

#endif	/* RSA_PUB_KEY_ENABLE || RSA_PRIV_KEY_ENABLE */

#if		RSA_PUB_KEY_ENABLE

/* raw (n, e) in API form --> public RSAGiantKey */
RSAStatus rsaInitPubGKey(
	const RSAPubKey	*apiKey,
	RSAPubGiantKey	*pubKey)
{
	if((apiKey == NULL) || (pubKey == NULL)) {
		return RSA_ParamErr;
	}
	if((apiKey->e == NULL) || (apiKey->n == NULL)) {
		return RSA_IncompleteKey;
	}
	if(rsaSetGiant(&pubKey->n, apiKey->n, apiKey->nLen)) {
		return RSA_BadKeyData;
	}
	if(rsaSetGiant(&pubKey->e, apiKey->e, apiKey->eLen)) {
		return RSA_BadKeyData;
	}
	
	localGiantAlloc(&pubKey->recip);
	if(apiKey->recip) {
		/* caller provided reciprocal */
		if(deserializeGiant(&pubKey->recip.g, apiKey->recip, apiKey->recipLen)) {
			return RSA_BadKeyData;
		}
	}
	/* else make_recip called lazily when the key is actually used */
	return RSA_Success;
}

#endif	/* RSA_PUB_KEY_ENABLE */

#if		RSA_PRIV_KEY_ENABLE

/* init an rsaSgiant, fill it with incoming bytes */
static RSAStatus rsaSetSGiant(
	rsaSgiant		*g,
	unsigned char	*c,
	gi_uint16		cLen)
{
	localSmallGiantAlloc(g);
	if(deserializeGiant(&g->g, c, cLen)) {
		return RSA_BadKeyData;
	}
	return RSA_Success;
}

/* raw (p, q, dp, dq, qInv) --> private RSAPrivCRTGiantKey */
RSAStatus rsaInitPrivGKey(
	const RSAPrivKey	*apiKey,
	RSAPrivGiantKey		*privKey)
{
	if((apiKey == NULL) || (privKey == NULL)) {
		return RSA_ParamErr;
	}
	if((apiKey->p == NULL) || (apiKey->q == NULL) || (apiKey->dp == NULL) ||
	   (apiKey->dq == NULL) || (apiKey->qInv == NULL)) {
		return RSA_ParamErr;
	}
	if(rsaSetSGiant(&privKey->p, apiKey->p, apiKey->pLen)) {
		return RSA_BadKeyData;
	}
	if(rsaSetSGiant(&privKey->q, apiKey->q, apiKey->qLen)) {
		return RSA_BadKeyData;
	}
	if(rsaSetSGiant(&privKey->dp, apiKey->dp, apiKey->dpLen)) {
		return RSA_BadKeyData;
	}
	if(rsaSetSGiant(&privKey->dq, apiKey->dq, apiKey->dqLen)) {
		return RSA_BadKeyData;
	}
	if(rsaSetSGiant(&privKey->qInv, apiKey->qInv, apiKey->qInvLen)) {
		return RSA_BadKeyData;
	}
	
	/* pRecip, qRecip evaluated lazily */
	localSmallGiantAlloc(&privKey->pRecip);
	localSmallGiantAlloc(&privKey->qRecip);

	return RSA_Success;
}

#endif	/* RSA_PRIV_KEY_ENABLE */
#endif	/* RSA_RAW_KEY_ENABLE */
