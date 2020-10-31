/*
 * Copyright (C) 2007-2008 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <platform.h>

#include <libDER/DER_CertCrl.h>
#include <libDER/DER_Keys.h>
#include <libDER/DER_Digest.h>
#include <libDER/oids.h>
#include <libDER/asn1Types.h>

#ifndef WITH_HW_PKE
#include <libgRSA/libgRSA.h>
#include <libgRSA/libgRSA_DER.h>
#include <libgRSA/libgRSA_priv.h>
#include <libGiants/giantDebug.h>
#endif

#include <debug.h>
#if DEBUG_BUILD
#ifndef DEBUG_ASSERT_PRODUCTION_CODE
#define DEBUG_ASSERT_PRODUCTION_CODE 0
#endif
#endif
#include <AssertMacros.h>

#include <drivers/pke.h>
#include <drivers/sha1.h>
#include <lib/pki.h>

#if PKI_APPLE_ROOT_CA
#include <apple_ca.h>
#else
#include <fake_ca.h>
#endif

typedef struct {
    DERItem subjectKeyID;
    DERItem authorityKeyID;
    DERItem appleSpecBlob;
    DERItem appleSpecTicketBlob;
} DERExtensions;

static int extension_octet_string_value(DERExtension *extn, bool critical, 
        DERItem *value)
{
    DERDecodedInfo extn_info;
#if 0
    if (critical) {
        /* this is more important to be enforced when issued */
        bool critical;
        require_noerr(DERParseBoolean(&extn->critical, false, &critical), out);
        require(critical, out);
    }
#endif
    /* get value */
    require_noerr(DERDecodeItem(&extn->extnValue, &extn_info), out);
    require(extn_info.tag == ASN1_OCTET_STRING, out);
    *value = extn_info.content;

    return 0;
out:
    return -1;
}

static int extension_sequence_item(DERExtension *extn, bool critical, 
        DERItem *value)
{
    DERDecodedInfo extn_info;
#if 0
    if (critical) {
        /* this is more important to be enforced when issued */
        bool critical;
        require_noerr(DERParseBoolean(&extn->critical, false, &critical), out);
        require(critical, out);
    }
#endif
    /* get the sequence */
    require_noerr(DERDecodeItem(&extn->extnValue, &extn_info), out);
    require(extn_info.tag == ASN1_CONSTR_SEQUENCE, out);
    *value = extn->extnValue;

    return 0;
out:
    return -1;
}

/**
 * Find extension with oidAppleSecureBootCertSpec oid and return octet string.
 * It's not an error to not find it, but general parse errors are flagged.
 */
static int parse_extensions(DERTBSCert *tbsCert, DERExtensions *extensions)
{
    DERSequence derSeq;
    DERTag tag;
    DERDecodedInfo currDecoded;
    DERExtension extn;

    bzero(extensions, sizeof(*extensions));
    if (tbsCert->extensions.length) {
        require_noerr(DERDecodeSeqInit(&tbsCert->extensions, &tag, &derSeq), out);
        require(tag == ASN1_CONSTR_SEQUENCE, out);
        while (DERDecodeSeqNext(&derSeq, &currDecoded) == DR_Success) {
            require(currDecoded.tag == ASN1_CONSTR_SEQUENCE, out);
            require_noerr(DERParseSequenceContent(&currDecoded.content,
                        DERNumExtensionItemSpecs, DERExtensionItemSpecs,
                        &extn, sizeof(extn)), out);

            if (DEROidCompare(&oidAppleSecureBootTicketCertSpec, &extn.extnID)) {
                require_noerr(extension_sequence_item(&extn, false, 
                    &extensions->appleSpecTicketBlob), out);
                dprintf(DEBUG_SPEW, "found apple spec (ticket) extension (%p, %u)\n",
                        extensions->appleSpecTicketBlob.data,
                        extensions->appleSpecTicketBlob.length);
            }
            else if (DEROidCompare(&oidAppleSecureBootCertSpec, &extn.extnID)) {
                require_noerr(extension_octet_string_value(&extn, false, 
                    &extensions->appleSpecBlob), out);
                dprintf(DEBUG_SPEW, "found apple spec (image3) extension (%p, %d)\n",
                        extensions->appleSpecBlob.data,
                        extensions->appleSpecBlob.length);
            }
#if PKI_CHECK_KEY_IDS
            else if (DEROidCompare(&oidSubjectKeyIdentifier, &extn.extnID)) {
                require_noerr(extension_octet_string_value(&extn, false, 
                    &extensions->subjectKeyID), out);
                dprintf(DEBUG_SPEW, "found subject key id extension (%p, %d)\n",
                        extensions->subjectKeyID.data,
                        extensions->subjectKeyID.length);
            }
            else if (DEROidCompare(&oidAuthorityKeyIdentifier, &extn.extnID)) {
                DERAuthorityKeyIdentifier authority_key_id_seq;
                require_noerr(DERParseSequence(&extn.extnValue,
                            DERNumAuthorityKeyIdentifierItemSpecs,
                            DERAuthorityKeyIdentifierItemSpecs,
                            &authority_key_id_seq, 
                            sizeof(authority_key_id_seq)), out);
                if (authority_key_id_seq.keyIdentifier.length) {
                    extensions->authorityKeyID = 
                        authority_key_id_seq.keyIdentifier; 
                    dprintf(DEBUG_SPEW, "found auth key id extension (%p, %d)\n",
                            extensions->authorityKeyID.data,
                            extensions->authorityKeyID.length);
                }
            }
#endif
        }
    }

    return 0;

out:
    return -1;
}

static inline int dercmp(const DERItem a, const DERItem b)
{
    if (a.length != b.length)
        return -1;
    if (memcmp(a.data, b.data, a.length))
        return -1;
    return 0;
}

/* Take a byte string and reverse it: giants layout on little endian */
static int rsa_set_giant(uint8_t *destination, size_t *dest_len, 
        uint8_t *source, size_t length, bool should_trim)
{
    size_t len = length;
    uint8_t *end = source;
    uint8_t *dst = destination;

    if(should_trim) {
        /* ASN.1 integers may start with a zero to avoid sign interpretation */
	if( len && !(*end) ) {
	    end++;
	    len--;
	}
    }

    /* If it won't fit or zero we won't continue */
    if (!len || len > *dest_len) {
        *dest_len = 0;
        return -1;
    }

    /* Reverse bytes: bignums start with lsb, word size doesn't matter for 
       little endian */
    uint8_t *src = source+length-1;
    while (src >= end)
        *dst++ = *src--;

    *dest_len = len;
    return 0;
}

#if WITH_HW_PKE
/* Verify signed encoded digest info with rsa pubKey against signature */
static int verify_pkcs1_sig(const DERItem *pubKey /* PKCS1 format */,
        const DERItem *digest_info, const DERItem *sig)
{
    DERItem keyItem = {(DERByte *)pubKey->data, pubKey->length};
    DERRSAPubKeyPKCS1 decodedKey;
    struct {
        uint8_t pmod[256];
        uint8_t base[256];
        uint8_t expn[256];
        uint8_t mods[256];
        size_t pmod_length;
        size_t base_length;
        size_t expn_length;
        size_t mods_length;
        size_t key_length;
    } request;

    require_noerr(DERParseSequence(&keyItem,
            DERNumRSAPubKeyPKCS1ItemSpecs, DERRSAPubKeyPKCS1ItemSpecs,
            &decodedKey, sizeof(decodedKey)), out);

    bzero(&request, sizeof(request));
    request.pmod_length = sizeof(request.pmod);
    request.base_length = sizeof(request.base);
    request.expn_length = sizeof(request.expn);
    request.mods_length = sizeof(request.mods);

    //Modulus and expn can have a leading zero padded to indicate a positive num. 
    //So trim those zeros out.
    require_noerr(rsa_set_giant(request.mods, &request.mods_length, 
        decodedKey.modulus.data, decodedKey.modulus.length, true), out);
    require_noerr(rsa_set_giant(request.expn, &request.expn_length, 
        decodedKey.pubExponent.data, decodedKey.pubExponent.length, true), out);
    require_noerr(rsa_set_giant(request.base, &request.base_length, 
        sig->data, sig->length, false), out);
    request.key_length = request.mods_length * 8;

    require(pke_do_exp(request.pmod, &request.pmod_length, 
        request.key_length,
        request.base, request.base_length,
        request.expn, request.expn_length,
        request.mods, request.mods_length), out);

    /* verify padding: 0x00, 0x01, 0xff**0xff, 0x00, {algid, hash}: */

    /* minimal pad length; digest_info->length although passed in is fixed. */
    require(request.pmod_length >= digest_info->length + 11, out);

    uint8_t *ptr = request.pmod + request.pmod_length - 1;
    uint8_t *end = request.pmod;
    uint8_t  result = 0;

    /* start with 0 */
    result |= *ptr-- ^ 0;
    require(ptr>end, out);

    /* block type 1 */
    result |= *ptr-- ^ 1;
    require(ptr>end, out);

    /* all 0xff */
    while (*ptr == 0xff) {
	    require(ptr>end, out);
	    ptr--;
    }

    /* terminated by 0 */
    result |= *ptr-- ^ 0;
    require(ptr>end, out);

    /* digest_info->length bytes left */
    require(end + digest_info->length - 1 == ptr, out);
    uint8_t *hash = digest_info->data;
    /* match SHA-1 hash */
    while (ptr>=end) {
	    result |= *ptr-- ^ *hash++;
    }
    require(result == 0, out);

    return 0;

out:
    return -1;
}
#else /* !WITH_HW_PKE */
/*
 * Verify a PKCS1 signature with an RSA key. Caller is responsible for 
 * EncodedDigestInfo formatting.
 */
static int verify_pkcs1_sig(
        const DERItem	*pubKey,	/* PKCS1 format */
        const DERItem	*toVerify,
        const DERItem	*sig)
{
    RSAPubGiantKey rsaPubKey;
    RSAStatus rrtn;

    GI_CHECK_SP("verifyPkcs1Sig");

    /* 
     * Convert PKCS1 format key to native libgRSA key.
     * Ideally we'd like to have the key in Apple Custom form, 
     * with the reciprocal, obtainable from cert, 
     * Someday maybe. 
     */
    GI_LOG_SP("calling RSA_DecodePubKey");
    rrtn = RSA_DecodePubKey(pubKey->data, pubKey->length, &rsaPubKey);
    require_noerr(rrtn, out);

    GI_LOG_SP("calling RSA_SigVerify");
    rrtn = RSA_SigVerify(&rsaPubKey, RP_PKCS1,
            (const gi_uint8 *)toVerify->data, (gi_uint16)toVerify->length,
            (const gi_uint8 *)sig->data, (gi_uint16)sig->length);
    require_noerr(rrtn, out);

    return 0;

out:
    return -1;
}
#endif

/*
 * Given data to sign or verify, take the SHA1 digest of it and encode the result in
 * an EncodedDigestInfo. The result is DER_SHA1_DIGEST_INFO_LEN bytes, allocated by the 
 * caller. 
 */
static int sha1DigestInfo(const DERItem *ptext, unsigned char *digestInfo)
    /* DER_SHA1_DIGEST_INFO_LEN bytes RETURNED */
{
    DERByte digest[DER_SHA1_DIGEST_LEN];
    DERReturn resultLen = DER_SHA1_DIGEST_INFO_LEN;

    /* take the SHA1 digest */
    sha1_calculate(ptext->data, ptext->length, digest);

    /* now roll it into an EncodedDigestInfo */
    return DEREncodeSHA1DigestInfo(digest, DER_SHA1_DIGEST_LEN,
            digestInfo, &resultLen);
}

/* decode an algorithm ID */
static int decodeAlgId(const DERItem *encodedAlgId, DERAlgorithmId *decodedAlgId)
{
    return DERParseSequenceContent(encodedAlgId, 
            DERNumAlgorithmIdItemSpecs, DERAlgorithmIdItemSpecs, 
            decodedAlgId, sizeof(*decodedAlgId));
}

/*
 * Obtain a PKCS1-format public key from a SubjectPubKeyInfo (tbs.subjectPubKey).
 */
static int decodePubKey(DERItem *pubKeyInfoContent, DERItem *pubKeyPkcs1)
{
    DERSubjPubKeyInfo pubKeyInfo;
    DERByte numUnused;
    DERAlgorithmId algId;

    /* sequence we're given: encoded DERSubjPubKeyInfo */
    require_noerr(DERParseSequenceContent(pubKeyInfoContent,
                DERNumSubjPubKeyInfoItemSpecs, DERSubjPubKeyInfoItemSpecs,
                &pubKeyInfo, sizeof(pubKeyInfo)), out);

    /* verify that this is an RSA key by decoding the AlgId */
    require_noerr(decodeAlgId(&pubKeyInfo.algId, &algId), out);
    require(DEROidCompare(&algId.oid, &oidRsa), out);

    /* 
     * The contents of pubKeyInfo.pubKey is a bit string whose contents
     * are a PKCS1 format RSA key. 
     */
    require_noerr(DERParseBitString(&pubKeyInfo.pubKey, pubKeyPkcs1, &numUnused), out);

    return 0;
out:
    return -1;
}

static int crack_chain(DERItem *certblob, DERItem chain[], DERShort chain_len)
{
    DERItem der_iter = *certblob;
    DERShort num = 0;
    DERSize used;
    DERDecodedInfo topDecode;

    dprintf(DEBUG_SPEW, "PKI: break certificate chain into certs\n");
    
    do {
        require_noerr(DERDecodeItem(&der_iter, &topDecode), out);

        used = topDecode.content.data + topDecode.content.length - der_iter.data;

        require(used < (128 * 1024), out);   //no single cert larger than 128k
        require(der_iter.length >= used, out);

        require(num < chain_len, out);

        chain[num].length = used;
        chain[num].data = der_iter.data;

        num++;

        der_iter.length -= used;
        der_iter.data += used;

    } while (der_iter.length > 0);
    
#if PKI_CHECK_ANCHOR_BY_SHA1
    /* require 3 element long chain */
    require(num == chain_len, out);

    /* check SHA-1 of (whole) root certificate (Apple root CA) */
    dprintf(DEBUG_SPEW, "PKI: check SHA-1 of root\n");
    unsigned char sha1[DER_SHA1_DIGEST_LEN];
    sha1_calculate(chain[0].data, chain[0].length, sha1);
    require_noerr(memcmp(ROOT_CA_SHA1_HASH, sha1, DER_SHA1_DIGEST_LEN), out);
#else
    /* make room for root CA */
    if (num == chain_len - 1) { 
        DERShort index;
        for (index = chain_len - 1; index > 0; index--)
            chain[index] = chain[index - 1];
    }
    else
        require(num == chain_len, out);

    /* use embedded root ca; override if one was passed in */
    dprintf(DEBUG_SPEW, "PKI: anchor chain to %s CA\n", 
        PKI_APPLE_ROOT_CA ? "Apple" : "fake");
    chain[0].data = (u_int8_t*)&ROOT_CA_CERTIFICATE;
    chain[0].length = sizeof(ROOT_CA_CERTIFICATE);
#endif /* PKI_CHECK_ANCHOR_BY_SHA1 */

    return 0;
out:
    return -1;
}

static int parse_chain(DERItem chain[], DERShort chain_len,
        DERSignedCertCrl chainElements[], DERTBSCert tbsCerts[], 
        DERItem derPubKey[], DERExtensions extensions[])
{
    DERShort num = 0;


    /* top level decode */
    for (num = 0; num < chain_len; num++) {

        dprintf(DEBUG_SPEW, "PKI: parse cert %d of %d\n", num + 1, chain_len);

        require_noerr(DERParseSequence(&chain[num], 
                    DERNumSignedCertCrlItemSpecs, DERSignedCertCrlItemSpecs, 
                    &chainElements[num], sizeof(*chainElements)), out);

        require_noerr(DERParseSequence(&chainElements[num].tbs, 
                    DERNumTBSCertItemSpecs, DERTBSCertItemSpecs,
                    &tbsCerts[num], sizeof(*tbsCerts)), out);

        require_noerr(decodePubKey(&tbsCerts[num].subjectPubKey, &derPubKey[num]), out);

        require_noerr(parse_extensions(&tbsCerts[num], &extensions[num]), out);
    }
    return 0;
out:
    return -1;
}

static int verify_chain_signatures(
        DERItem *issuerPubKey,
        DERSignedCertCrl *signedCert)
{
    DERAlgorithmId	algId;
    unsigned char	digestInfo[DER_SHA1_DIGEST_INFO_LEN];
    DERByte		numUnused;
    DERItem		toVerify = { digestInfo, DER_SHA1_DIGEST_INFO_LEN };
    DERItem		sigBytes;

    /* figure out which digest to use from the sig algId */
    require_noerr(decodeAlgId(&signedCert->sigAlg, &algId), out);

    /* get the encodedDigestInfo from the digest of the subject's TBSCert */
    require(DEROidCompare(&algId.oid, &oidSha1Rsa), out);

    /* generate digest info from plaintext */
    require_noerr(sha1DigestInfo(&signedCert->tbs, digestInfo), out);

    /* get contents of sig, a bit string, as raw bytes */
    require_noerr(DERParseBitString(&signedCert->sig, &sigBytes, &numUnused), out);

    require_noerr(verify_pkcs1_sig(issuerPubKey, &toVerify, &sigBytes), out);

    return 0;

out:
    return -1;
}

/* find first value for oid */
static int find_content_by_oid(const DERItem *rdnSetContent, 
        const DERItem *oid, DERItem *content) 
{
    DERReturn drtn;
    DERSequence rdn;
    DERDecodedInfo atvContent;
    DERAttributeTypeAndValue atv;

    require_noerr(DERDecodeSeqContentInit(rdnSetContent, &rdn), out);

    while ((drtn = DERDecodeSeqNext(&rdn, &atvContent)) == DR_Success) {

        require(atvContent.tag == ASN1_CONSTR_SEQUENCE, out);

        require_noerr(DERParseSequenceContent(&atvContent.content,
                    DERNumAttributeTypeAndValueItemSpecs,
                    DERAttributeTypeAndValueItemSpecs,
                    &atv, sizeof(atv)), out);

        if (DEROidCompare(oid, &atv.type)) {
		*content = atv.value;
		return 0;
        }
    }
    return drtn;
out:
    return -1;
}


static int parse_common_name(const DERItem *subject, const DERItem *oid, DERItem *content)
{
    DERSequence derSeq;
    DERDecodedInfo currDecoded;

    require_noerr(DERDecodeSeqContentInit(subject, &derSeq), out);

    while (DERDecodeSeqNext(&derSeq, &currDecoded) == DR_Success) {

        require(currDecoded.tag == ASN1_CONSTR_SET, out);

        /* only if we reached the end of a sequence do we keep looking */
        int status = find_content_by_oid(&currDecoded.content, oid, content);
        if (status != DR_EndOfSequence)
            return status;
    }
out:
    return -1;
}


static int verify_signature_with_hash(DERItem *public_key_pkcs1_rsa,
        unsigned char *sig_blob, unsigned sig_blob_len,
        unsigned char *hash_blob, unsigned hash_blob_len)
{
    unsigned char digest_data[DER_SHA1_DIGEST_INFO_LEN];
    DERItem signature = { sig_blob, sig_blob_len };
    DERItem digest_info = { digest_data, sizeof(digest_data) };

    /* generate digest info blob from hash */
    require(hash_blob_len == DER_SHA1_DIGEST_LEN, out);

    require_noerr(DEREncodeSHA1DigestInfo(hash_blob, hash_blob_len, 
                digest_info.data, &digest_info.length), out);

    /* verify signature */
    return verify_pkcs1_sig(public_key_pkcs1_rsa, &digest_info, &signature);

out:
    return -1;
}

int verify_signature_with_chain(void *chain_blob, size_t chain_blob_length,
        void *sig_blob, size_t sig_blob_len,
        void *hash_blob, size_t hash_blob_len,
        void **img3_spec_blob, size_t *img3_spec_blob_len,
        void **ticket_spec_blob, size_t *ticket_spec_blob_len)
{
    DERShort index;
    const uint8_t chain_len = 3;
    DERItem chain = { chain_blob, chain_blob_length };
    DERItem certs[chain_len];
    DERSignedCertCrl signedCert[chain_len];
    DERTBSCert tbsCert[chain_len];
    DERItem pubKeyPkcs1[chain_len];
    DERExtensions extensions[chain_len];

    require_noerr(crack_chain(&chain, certs, chain_len), out);

    require_noerr(parse_chain(certs, chain_len, signedCert, tbsCert, 
        pubKeyPkcs1, extensions), out);

    /* check linkage */
    index = 1;
    while (index < chain_len) {
        dprintf(DEBUG_SPEW, "PKI: verify cert %d was issued by %d\n", index, index-1);
        require_noerr(dercmp(tbsCert[index-1].subject, 
            tbsCert[index].issuer), out);
#if PKI_CHECK_KEY_IDS
        require_noerr(dercmp(extensions[index-1].subjectKeyID, 
            extensions[index].authorityKeyID), out);
#endif        
        require_noerr(verify_chain_signatures(&pubKeyPkcs1[index-1], &signedCert[index]), out);
        index++;
    }

    /* check common name intermediary cert to restrict to secure boot sub CA */
    DERItem commonName = { NULL, 0 };
    require_noerr(parse_common_name(&tbsCert[1].subject, &oidCommonName, &commonName), out);
    // first character (0x13) - ASN1_PRINTABLE_STRING
    // second character (0x29) - Length (copied over a cert)
    static const DERByte sboot_common_name[] = "\x13\x29""Apple Secure Boot Certification Authority";
    static const DERItem sboot_item = {
	    .data = (DERByte *)sboot_common_name,
	    .length = 0x2b
    };
    dprintf(DEBUG_SPEW, "PKI: check intermediate cert for common name '%s'\n", sboot_common_name);
    require(DEROidCompare(&sboot_item, &commonName), out);

    dprintf(DEBUG_SPEW, "PKI: check payload hash with signature\n");
    require_noerr(verify_signature_with_hash(&pubKeyPkcs1[chain_len-1],
            sig_blob, sig_blob_len, hash_blob, hash_blob_len), out);

    /* return pointer to leaf blob in cert chain if it exists */
    if (extensions[chain_len-1].appleSpecTicketBlob.data &&
                extensions[chain_len-1].appleSpecTicketBlob.length) {
        if (ticket_spec_blob != NULL && ticket_spec_blob_len != NULL) {
            *ticket_spec_blob = extensions[chain_len-1].appleSpecTicketBlob.data;
            *ticket_spec_blob_len = extensions[chain_len-1].appleSpecTicketBlob.length;
            dprintf(DEBUG_SPEW, "PKI: parse apple spec id extension (ticket) in leaf: (%p, %zu)\n", *ticket_spec_blob, *ticket_spec_blob_len);
        }
    } 
    else if (extensions[chain_len-1].appleSpecBlob.data &&
                extensions[chain_len-1].appleSpecBlob.length) {
        if (img3_spec_blob != NULL && img3_spec_blob_len != NULL) {
            *img3_spec_blob = extensions[chain_len-1].appleSpecBlob.data;
            *img3_spec_blob_len = extensions[chain_len-1].appleSpecBlob.length;
            dprintf(DEBUG_SPEW, "PKI: parse apple spec id extension (image3) in leaf: (%p, %zu)\n", *img3_spec_blob, *img3_spec_blob_len);
        }
    }
#if NO_DEFAULT_PLATFORM_PROVIDE_SPEC_BLOB
    else /* ask the platform to provide a leaf blob */
    {
        DERTBSCert *leaf = &tbsCert[chain_len-1];
        DERItem leafCommonName = { NULL, 0 };

        require_noerr(parse_common_name(&leaf->subject, &oidCommonName, &leafCommonName), out);

        require(platform_provide_spec_blob(leafCommonName.data, leafCommonName.length, 
                leaf->serialNum.data, leaf->serialNum.length, 
                img3_spec_blob, img3_spec_blob_len), out);
    }
#endif // NO_DEFAULT_PLATFORM_PROVIDE_SPEC_BLOB

    return 0;

out:
    return -1;
}
