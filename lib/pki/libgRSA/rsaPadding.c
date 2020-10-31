/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * rsaPadding.c - PKCS1 and OAEP padding support.
 *
 * Created Aug. 10 2005 by Doug Mitchell.
 */

#include <libgRSA/rsaPadding.h>
#include <libgRSA/libgRSA_config.h>
#include <libgRSA/libgRSA_priv.h>
#include <libGiants/giantExternal.h>
#include <libGiants/giantDebug.h>
#include <AssertMacros.h>

/* 
 * RP_None padding routines - just check length and convert between
 * byte array and giant 
 */
RSAStatus rsaAddNoPadding(
	const unsigned char	*plainText,
	gi_uint16			plainTextLen,	
	giant				result)			/* RETURNED */
{
	GIReturn rtn = deserializeGiant(result, plainText, plainTextLen);
	return giantStatusToRSA(rtn);
}

RSAStatus rsaCheckNoPadding(
	giant				decryptResult,
	unsigned char		*plainText,		/* data RETURNED here */
	gi_uint16			*plainTextLen)	/* IN/OUT */
{
	GIReturn rtn = serializeGiant(decryptResult, 
		(gi_uint8 *)plainText, plainTextLen);
	return giantStatusToRSA(rtn);
}

#if		RSA_VERIFY_ENABLE

/* 
 * Decrement digit ptr and byte index thru a giant's n[],
 * going from m.s. byte to l.s. byte.
 * -- n[0] is the l.s. digit of a giant
 * -- (n[0] & 0xff) is the l.s. byte of that digit
 */
static inline void decrGiantPtrs(
	giantDigit **digitPtr,
	gi_uint8 *byteDex)
{
	if(*byteDex == 0) {
		/* roll to m.s. byte of next l.s. digit */
		(*digitPtr)--;
		*byteDex = GIANT_BYTES_PER_DIGIT - 1;
	}
	else {
		(*byteDex)--;
	}
}

/* fetch specified byte from giant's n[] array */
static inline gi_uint8 getGiantByte(
	giantDigit *digitPtr,
	gi_uint8 byteDex)
{
	giantDigit digit = *digitPtr;
	digit >>= (byteDex * 8);
	return (gi_uint8)digit;
}
	
/* 
 * How many bytes are left between current digit/byte markers and
 * the start of the giant's data?
 */
static inline gi_size bytesLeftInGiant(
	giant g,
	giantDigit *digitPtr,
	gi_uint8 byteDex)
{
	GIASSERT(digitPtr >= g->n);
	return ((digitPtr - g->n) * GIANT_BYTES_PER_DIGIT) + byteDex + 1;
}
	

/*
 * Specialized "no padding" check for signature verify operation.
 * We don't deserialize the decrypted giant; we do a byte-for-
 * byte compare of the expected decrypted value against memory
 * passed in to us, in-place in the decrypted giant. 
 * This saves two stack-allocated byte arrays, each of 
 * MAX_PRIME_SIZE_BYTES - one here, and one in RSA_SigVerify().
 *
 * NOTE: using "no padding" option will not work if the first byte
 * (the m.s. byte of the decryptResult giant) is zero; there is 
 * no way to verify leading zeroes as they got chopped off during
 * the calculation. 
 */
RSAStatus rsaCheckNoPaddingForSigVfy(
	giant				decryptResult,
	const unsigned char	*expectPlainText,
	gi_uint16			expectPlainTextLen)
{
	unsigned dex;
	giantDigit *digitPtr;
	gi_uint8 byteDex;
	gi_uint8 giantByte;
	gi_size bytesLeft;

	if((expectPlainText == NULL) || (decryptResult == NULL)) {
		return RSA_InternalError;
	}
	if(expectPlainTextLen > MAX_PRIME_SIZE_BYTES) {
		return RSA_Overflow;
	}
	if(decryptResult->sign == 0) {
		if(expectPlainTextLen == 0) {
			/* Well OK */
			return RSA_Success;
		}
		else {
			rsaDebug("rsaCheckNoPaddingForSigVfy: empty decryptResult\n");
			return RSA_VerifyFail;
		}
	}
	/* 
	 * Prepare for scanning thru giant data, starting at the m.s. byte of the 
	 * m.s. digit.
	 */
	digitPtr = &decryptResult->n[decryptResult->sign - 1];
	byteDex = GIANT_BYTES_PER_DIGIT - 1;
	bytesLeft = bytesLeftInGiant(decryptResult, digitPtr, byteDex);
	GIASSERT(bytesLeft != 0);
	
	/* find first non-zero byte */
	for(;;) {
		giantByte = getGiantByte(digitPtr, byteDex);
		if(giantByte != 0) {
			break;
		}
		decrGiantPtrs(&digitPtr, &byteDex);
		if(--bytesLeft == 0) {
			rsaDebug("rsaCheckNoPaddingForSigVfy: non nonzero data found\n");
			return RSA_VerifyFail;
		}
	}
	
	if(bytesLeft != expectPlainTextLen) {
		rsaDebug("rsaCheckNoPaddingForSigVfy: length mismatch\n");
		return RSA_VerifyFail;
	}
	
	for(dex=0; dex<bytesLeft; dex++) {
		giantByte = getGiantByte(digitPtr, byteDex);
		decrGiantPtrs(&digitPtr, &byteDex);
		if(giantByte != expectPlainText[dex]) {
			rsaDebug("rsaCheckNoPaddingForSigVfy: data miscompare\n");
			return RSA_VerifyFail;
		}
	}

	/* TA DA */
	return RSA_Success;

}

#endif	/* RSA_VERIFY_ENABLE */

#if		RSA_PADDING_PKCS1_ENABLE

#define PKCS1_MIN_PAD_LEN	11		/* minumum length of padding */

#if		RSA_SIGN_ENABLE || RSA_ENCRYPT_ENABLE

/* Write randSize bytes of nonzero data into randBuf. */
static RSAStatus nonZeroRandom(
	RSARngCallback		*rngCallback,
	gi_uint8			*randBuf,			/* random data goes here */
	gi_uint16			randSize)			/* byte count */
{
	RSAStatus rstatus = RSA_Success;

	gi_uint8 scratch[randSize];
	gi_uint16 dix, six = randSize;
	for (dix = 0; dix < randSize; ++dix) {
		for (;;) {
			if (six >= randSize) {
				require_noerr(rstatus = rngCallback(scratch, randSize), errOut);
				six = 0;
			}

			gi_uint8 b = scratch[six++];
			if (b) {
				randBuf[dix] = b;
				break;
			}
		}
	}

errOut:
	/* Clear out scratch cause we like being paranoid. */
	Bzero(scratch, randSize);
	return rstatus;
}

/* 
 * Given plaintext or data to be signed, add PKCS1 padding and place
 * the result in a giant.
 */
RSAStatus rsaAddPkcs1Padding(
	const unsigned char	*plainText,
	gi_uint16			plainTextLen,	
	gi_uint16			totalLength,	/* intended total length */
	unsigned char		padMarker,		/* RSA_PKCS1_PAD_{SIGN,ENCRYPT} */
	RSARngCallback		*rngCallback,	/* required for RP_PKCS1, RP_OAEP */
	giant				result)
{
	unsigned char giantBytes[MAX_PRIME_SIZE_BYTES];
	unsigned char *cp = giantBytes;
	unsigned dex;
	GIReturn grtn;
	RSAStatus rrtn = RSA_Success;
	unsigned psLen;			/* length of variable length padding string */
	
	if((plainText == NULL) || (result == NULL)) {
		return RSA_InternalError;
	}
	if(totalLength > MAX_PRIME_SIZE_BYTES) {
		return RSA_Overflow;
	}
	if(plainTextLen > (totalLength - PKCS1_MIN_PAD_LEN)) {
		rsaDebug("rsaAddPkcs1Padding: insuffient space for padding\n");
		return RSA_Overflow;
	}
	if((padMarker == RSA_PKCS1_PAD_ENCRYPT) && (rngCallback == NULL)) {
		rsaDebug("rsaAddPkcs1Padding: RNG required\n");
		return RSA_ParamErr;
	}
	
	*cp++ = 0;
	*cp++ = padMarker;
	
	/* 
	 * RSA_PKCS1_PAD_SIGN: pad with 0xff
	 * RSA_PKCS1_PAD_ENCRYPT: pad with random nonzero data
	 */
	psLen = totalLength - plainTextLen - 3;
	if(padMarker == RSA_PKCS1_PAD_ENCRYPT) {
		/* Fill cp with psLen random nonzero bytes. */
		rrtn = nonZeroRandom(rngCallback, cp, psLen);
		if (rrtn) {
			rsaDebug1("rsaAddPkcs1Padding: nonZeroRandom returned: %s\n",
				rsaStatusStr(rrtn));
			return rrtn;
		}

		cp += psLen;
	} else {
		for(dex=0; dex<psLen; dex++) {
			*cp++ = 0xff;
		}
	}
	*cp++ = 0;
	Memcpy(cp, plainText, plainTextLen);
	RSAASSERT((cp + plainTextLen) == (giantBytes + totalLength));
	
	grtn = deserializeGiant(result, giantBytes, totalLength);
	if(grtn) {
		rrtn = giantStatusToRSA(grtn);
		rsaDebug1("rsaAddPkcs1Padding: deserializeGiant returned %s\n",
			rsaStatusStr(rrtn));
	}
	return rrtn;
}

#endif		/* RSA_SIGN_ENABLE || RSA_ENCRYPT_ENABLE */

#if		RSA_DECRYPT_ENABLE
/* 
 * Given decrypted plaintext in the form of a giant, which should 
 * be the same size as n, verify proper PKCS1 padding and copy
 * resulting plaintext and length into caller-supplied buffer. 
 * ON entry, *plainTextLen indicates the size of the available
 * plaintext buffer. ON return, *plainTextLen indicates the 
 * number of valid plaintext recovered. 
 *
 * FIXME don't need padMarker any more...
 */
RSAStatus rsaCheckPkcs1Padding(
	giant				decryptResult,
	unsigned char		padMarker,		/* RSA_PKCS1_PAD_{SIGN,ENCRYPT} */
	unsigned char		*plainText,		/* data RETURNED here */
	gi_uint16			*plainTextLen)	/* IN/OUT */
{
	unsigned char giantBytes[MAX_PRIME_SIZE_BYTES];
	gi_uint16 giantBytesLen = MAX_PRIME_SIZE_BYTES;
	unsigned char *cp = giantBytes;
	GIReturn grtn;
	RSAStatus rrtn = RSA_Success;
	RSAStatus formatRtn;
	unsigned char *giantEnd;
	gi_uint16 validData;
	unsigned dex;
	
	if(padMarker == RSA_PKCS1_PAD_SIGN) {
		formatRtn = RSA_BadSigFormat;
	}
	else {
		formatRtn = RSA_BadDataFormat;
	}
	if((plainText == NULL) || (decryptResult == NULL)) {
		return RSA_InternalError;
	}
	if(*plainTextLen < (MAX_PRIME_SIZE_BYTES - PKCS1_MIN_PAD_LEN)) {
		return RSA_Overflow;
	}
	grtn = serializeGiant(decryptResult, giantBytes, &giantBytesLen);
	if(grtn) {
		rrtn = giantStatusToRSA(grtn);
		rsaDebug1("rsaCheckPkcs1Padding: serializeGiant returned %s\n",
			rsaStatusStr(rrtn));
		return rrtn;
	}
	
	/* 
	 * Leading zeroes are optional:
	 *
	 * 1. If the key's modulus size is not aligned to a giantDigit and the m.s. 
	 *    byte of decryptResult - which was set to zero when the padding was
	 *    created - is the only byte in the m.s. giantDigit, OR giantDigit is
	 *    one byte, then the leading zero will not appear in the serialized
	 *    giantBytes.
	 * 2. If the key's modulus size is not aligned to a giantDigit, and the
	 *    m.s. byte of decryptResult is neither the l.s. or the m.s. byte 
	 *    of the m.s. giantDigit, then we'll see extra zeroes at the m.s.
	 *    end of the serialized giantBytes. 
	 */
	for(dex=0; dex<GIANT_BYTES_PER_DIGIT-1; dex++) {
		if(*cp != 0) {
			break;
		}
		cp++;
	}
	if(*cp++ != padMarker) {
		rsaDebug("rsaCheckPkcs1Padding: bad padMarker\n");
		rrtn = formatRtn;
		goto errOut;
	}

	/* 
	 * RSA_PKCS1_PAD_SIGN: pad with 0xff
	 * RSA_PKCS1_PAD_ENCRYPT: pad with random nonzero data
	 */
	 
	/* mark end of valid decrypted data */
	giantEnd = giantBytes + giantBytesLen;
	do {
		unsigned char b = *cp++;
		if(b == 0) {
			/* normal termination */
			break;
		}
		if(padMarker == RSA_PKCS1_PAD_SIGN) {
			/* verify in this case else skip */
			if(b != 0xff) {
				rsaDebug("rsaCheckPkcs1Padding: SIGN pad\n");
				rrtn = formatRtn;
				goto errOut;
			}
		}
	} while (cp < giantEnd);
	
	validData = giantEnd - cp;
	*plainTextLen = validData;
	if(validData != 0) {
		Memcpy(plainText, cp, validData);
	}
errOut:
	Bzero(giantBytes, MAX_PRIME_SIZE_BYTES);
	return rrtn;
	
}
#endif	/* RSA_DECRYPT_ENABLE */

#endif	/* RSA_PADDING_PKCS1_ENABLE */

#if RSA_PADDING_PKCS1_ENABLE && RSA_VERIFY_ENABLE

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
	gi_uint16			expectPlainTextLen)
{
	unsigned dex;
	giantDigit *digitPtr;
	gi_uint8 byteDex;
	gi_uint8 giantByte;
	gi_size bytesLeft;
	gi_size numFFs = 0;
	
	if((expectPlainText == NULL) || (decryptResult == NULL)) {
		return RSA_InternalError;
	}
	if(expectPlainTextLen > (MAX_PRIME_SIZE_BYTES - PKCS1_MIN_PAD_LEN)) {
		return RSA_Overflow;
	}
	GI_CHECK_SP("rsaCheckPkcs1PaddingForSigVfy");
	
	/* 
	 * Prepare for scanning thru giant data, starting at the m.s. byte of the 
	 * m.s. digit.
	 */
	digitPtr = &decryptResult->n[decryptResult->sign - 1];
	byteDex = GIANT_BYTES_PER_DIGIT - 1;
	
	/* 
	 * Padding of the pre-encrypted data is like so:
	 *
	 * one byte zero
	 * one byte pad marker (RSA_PKCS1_PAD_SIGN)
	 * 8 bytes of 0xff
	 * one byte of zero
	 * then the to-be-encrypted data
	 *
	 * Leading zeroes are optional:
	 *
	 * 1. If:
	 *    -- The key's modulus size is not aligned to a giantDigit, AND 
	 *       -- the m.s. byte of decryptResult - which was set to zero when 
	 *          the padding was created - is the only byte in the m.s. 
	 *          giantDigit (i.e., num_bytes_of_decrypted_data mod 
	 *          sizeof(giantDigit) equals one, 
	 *       OR
	 *       -- sizeof(giantDigit) == 1 byte
	 *
	 *    THEN the leading zero will not appear in decryptResult.
	 *
	 * 2. If:
	 *    -- The key's modulus size is not aligned to a giantDigit, AND
	 *    -- The m.s. byte of decryptResult is neither the l.s. or the 
	 *       m.s. byte of the m.s. giantDigit,
	 *
	 *    THEN then we'll see extra zeroes at the m.s. digit in decryptResult.
	 */
	for(;;) {
		giantByte = getGiantByte(digitPtr, byteDex);
		if(giantByte != 0) {
			break;
		}
		decrGiantPtrs(&digitPtr, &byteDex);
		if(digitPtr == decryptResult->n) {
			/* 
			 * rolled back to first digit, not enough for 11 bytes of 
			 * padding.
			 * FIXME rewrite this when sizeof(giantDigit) > 11. :-)
			 */
			rsaDebug("rsaCheckPkcs1PaddingForSigVfy: no padMarker\n");
			return RSA_BadSigFormat;
		}
	}
	/* we're at the first nonzero byte */
	if(giantByte != RSA_PKCS1_PAD_SIGN) {
		rsaDebug("rsaCheckPkcs1PaddingForSigVfy: bad padMarker\n");
		return RSA_BadSigFormat;
	}
	decrGiantPtrs(&digitPtr, &byteDex);
	bytesLeft = bytesLeftInGiant(decryptResult, digitPtr, byteDex);
	if(bytesLeft < 9) {
		rsaDebug("rsaCheckPkcs1PaddingForSigVfy: no space left for padding\n");
		return RSA_BadSigFormat;
	}
	
	/*
	 * Now skip over 0xff bytes and then one byte of zero. 
	 */
	do {
		giantByte = getGiantByte(digitPtr, byteDex);
		decrGiantPtrs(&digitPtr, &byteDex);
		bytesLeft--;
		if(giantByte == 0xff) {
			numFFs++;
			continue;
		}
		else if(giantByte == 0) {
			if(numFFs < 8) {
				rsaDebug("rsaCheckPkcs1PaddingForSigVfy: insufficient padding\n");
				return RSA_BadSigFormat;
			}
			/* start of decrypted data */
			break;
		}
		else {
			rsaDebug("rsaCheckPkcs1PaddingForSigVfy: bad padding (not ff)\n");
			return RSA_BadSigFormat;
		}
	} while(bytesLeft != 0);
	
	/* We're at the first decrypted data; compare with caller's expected data */
	if(bytesLeft != expectPlainTextLen) {
		rsaDebug("rsaCheckPkcs1PaddingForSigVfy: length mismatch\n");
		return RSA_VerifyFail;
	}
	
	for(dex=0; dex<bytesLeft; dex++) {
		giantByte = getGiantByte(digitPtr, byteDex);
		decrGiantPtrs(&digitPtr, &byteDex);
		if(giantByte != expectPlainText[dex]) {
			rsaDebug("rsaCheckPkcs1PaddingForSigVfy: data miscompare\n");
			return RSA_VerifyFail;
		}
	}

	/* TA DA */
	return RSA_Success;
	
}

#endif	/* RSA_PADDING_PKCS1_ENABLE && RSA_VERIFY_ENABLE */
