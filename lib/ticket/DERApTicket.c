/*
 * Copyright (C) 2011-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <lib/DERApTicket.h>
#include <lib/mib.h>
#include <lib/ticket.h>
#include <lib/pki.h>
#include <libDER/asn1Types.h>
#include <libDER/DER_Decode.h>
#include <libDER/DER_Keys.h>
#include <libDER/oids.h>
#include <drivers/sha1.h>
#include <arch.h>
#include <platform.h>
#include <AssertMacros.h>

/* -------------------------------------------------------------------------- */

extern size_t sprint_hex(const uint8_t *data, unsigned data_len, char *buf, unsigned buf_len);

static DERReturn DERApTicketValidateCertExtension( const DERItem *contents );

/* -------------------------------------------------------------------------- */

const DERItemSpec DERApTicketItemSpecs[] =
{
    { DER_OFFSET(DERApTicket, signatureAlgorithm),
            ASN1_CONSTR_SEQUENCE,
            DER_DEC_NO_OPTS },
    { DER_OFFSET(DERApTicket, body),
            ASN1_CONSTR_SET,
            DER_DEC_NO_OPTS | DER_DEC_SAVE_DER },
    { DER_OFFSET(DERApTicket, signature),
            ASN1_OCTET_STRING,
            DER_DEC_NO_OPTS },
    { DER_OFFSET(DERApTicket, certificates),
            ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 1,
            DER_DEC_NO_OPTS }
};

const DERSize DERNumApTicketItemSpecs = sizeof(DERApTicketItemSpecs) / sizeof(DERItemSpec);

/* -------------------------------------------------------------------------- */

const DERItemSpec DERApTicketCertExtensionItemSpecs[] = 
{
    { DER_OFFSET(DERApTicketCertExtension, version),
            ASN1_CONTEXT_SPECIFIC | 0,
            DER_DEC_OPTIONAL },
    { DER_OFFSET(DERApTicketCertExtension, override),
            ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 1,
            DER_DEC_NO_OPTS },
    { DER_OFFSET(DERApTicketCertExtension, append),
            ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 2,
            DER_DEC_OPTIONAL },
    { DER_OFFSET(DERApTicketCertExtension, remove),
            ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 3,
            DER_DEC_OPTIONAL }
};

const DERSize DERNumApTicketCertExtensionItemSpecs = sizeof(DERApTicketCertExtensionItemSpecs) / sizeof(DERItemSpec);

/* -------------------------------------------------------------------------- */

static DERReturn
GetBuildStringTagForRunningInstance( DERTag *outTag, bool isRestore )
{
#if !TICKET_UNITTEST
    uint32_t app_prod = mib_get_u32(kMIBTargetApplicationProduct);
#else
    uint32_t app_prod = MIB_APP_PROD_IBOOT;
#endif

    switch (app_prod) {
        case MIB_APP_PROD_LLB:
            /* <rdar://problem/9383890> LLB should not attempt to self-verify against a restore ticket */
            if ( !isRestore ) {
                *outTag = 6;
                return DR_Success;
            }
            return -1;

        case MIB_APP_PROD_IBSS:
            *outTag = 20;
            return DR_Success;

        case MIB_APP_PROD_IBEC:
            /* <rdar://problem/9384142> iBEC Should self-verify */
            *outTag = 22;
            return DR_Success;

        default:
            /* iBoot has no build string tag */
            return -1;
    }
}

/* -------------------------------------------------------------------------- */

static DERReturn
DERFindItemWithTag( const DERItem *container, DERTag tag, DERItem *outItem )
{
    DERReturn derstat = -1;
    DERSequence sequence;
    DERTag containerTag;
    DERDecodedInfo info;
//  char debugBuffer[128];

    derstat = DERDecodeSeqInit( container, &containerTag, &sequence );
    require_action( derstat == DR_Success, exit, TICKETLOG("can't parse top-level container") );
    
    int i = 0;
    while ( true ) {
        derstat = DERDecodeSeqNext( &sequence, &info );
        require( derstat == DR_Success, exit );
        
//      sprint_hex(info.content.data, info.content.length, debugBuffer, sizeof(debugBuffer));
//      TICKETLOG( "tag %u length=%u: %s", info.tag & ~ASN1_CONTEXT_SPECIFIC, info.content.length, debugBuffer );
        
        if ( info.tag == tag ) {
            break;
        }
        
        ++i;
    }

    *outItem = info.content;
    derstat = DR_Success;

exit:
    return derstat;
}

/* -------------------------------------------------------------------------- */

static DERReturn
DERApTicketFindAndValidateTag( const DERItem *container, DERTag tag, void *requiredValue, unsigned requiredLength )
{
    DERReturn derstat = -1;
    DERItem foundItem;

    derstat = DERFindItemWithTag( container, tag, &foundItem );
    require_action( derstat == DR_Success, exit, TICKETLOG("missing tag (%u)", tag) );

    require_action( foundItem.length == requiredLength, exit, derstat = -1;
		    TICKETLOG("tag (%llu) value has incorrect length (%u bytes, expected %u)", (uint64_t) tag, foundItem.length, requiredLength) );

    if ( memcmp(foundItem.data, requiredValue, requiredLength) != 0 ) {
      TICKETLOG( "tag (%llu) value is invalid", (uint64_t) tag );
        derstat = -1;
        goto exit;
    }
    
    derstat = DR_Success;
    
exit:
    return derstat;
}

/* -------------------------------------------------------------------------- */

DERReturn
DERApTicketCopyTagData( const DERItem *container, DERTag tag, uint8_t *buffer, DERSize *ioLength )
{
    DERReturn derstat = -1;
    DERItem item;
    
    derstat = DERFindItemWithTag( container, tag | ASN1_CONTEXT_SPECIFIC, &item );
    require( derstat == DR_Success, exit );
    
    require_action( *ioLength >= item.length, exit, derstat = -1;
        TICKETLOG("item data length (%u) is larger than provided buffer length (%u)", item.length, *ioLength) );
    
    memcpy( buffer, item.data, item.length );
    *ioLength = item.length;
    
    derstat = DR_Success;

exit:
    return derstat;
}

/* -------------------------------------------------------------------------- */

/*
 *  Decode the AP ticket and verify the signature.
 *  If the ticket structure is malformed or the signature is invalid, returns error.
 */
DERReturn 
DERApTicketDecode( const DERItem *contents, DERApTicket *outTicket, DERSize *outNumUsedBytes )
{
    DERReturn derstat = -1;
    int error;
    DERDecodedInfo ticketInfo;
    DERAlgorithmId algId;
    DERItem appleSpec;
    uint8_t bodyHash[CCSHA1_OUTPUT_SIZE];

    bzero( &appleSpec, sizeof(appleSpec) );
    
    /*
     * Validate the ticket structure and cert chain
     */
    derstat = DERDecodeItem( contents, &ticketInfo );
    require_action( derstat == DR_Success, exit, TICKETLOG("malformed ticket") );
    
    derstat = DERParseSequenceContent( &(ticketInfo.content), DERNumApTicketItemSpecs, DERApTicketItemSpecs, outTicket, 0 );
    require_action( derstat == DR_Success, exit, TICKETLOG("malformed ticket") );
    
    derstat = DERParseSequenceContent( &(outTicket->signatureAlgorithm), DERNumAlgorithmIdItemSpecs, DERAlgorithmIdItemSpecs, &algId, sizeof(algId) );
    require_action( derstat == DR_Success, exit, TICKETLOG("malformed ticket") );

    if ( !DEROidCompare( &algId.oid, &oidSha1Rsa) || (algId.params.data != NULL)) {
        TICKETLOG( "mismatched algorithm" );
        derstat = -1;
        goto exit;
    }

    sha1_calculate( outTicket->body.data, outTicket->body.length, bodyHash );

    error = verify_signature_with_chain( 
        outTicket->certificates.data, outTicket->certificates.length,
        outTicket->signature.data, outTicket->signature.length,
        bodyHash, sizeof(bodyHash),
        NULL, NULL,
        (void **) &(appleSpec.data), (size_t *) &(appleSpec.length) );

    if ( error != 0 ) {
        TICKETLOG( "failed signature validation" );
        derstat = -1;
        goto exit;
    }

//  TICKETLOG( "cert extension: 0x%08x, length=%u", appleSpec.data, appleSpec.length );
//  hexdump( appleSpec.data, appleSpec.length );

    require_action( appleSpec.data != NULL, exit, derstat = -1; TICKETLOG("missing cert extension") );

    derstat = DERApTicketValidateCertExtension( &appleSpec );
    require_action( derstat == DR_Success, exit, TICKETLOG("incompatible cert") );

    derstat = DR_Success;

exit:
    if ( derstat == DR_Success ) {
        /*
         * "ticketInfo" describes a pointer and length to content within "contents->data".
         * The difference of that pointer and "contents->data" is the number of bytes used to describe the content.
         * The overall ticket length is the content length summed with the number of bytes used to describe it.
         */
        *outNumUsedBytes = ticketInfo.content.length + (ticketInfo.content.data - contents->data);
    }
    else {
        *outNumUsedBytes = 0;
    }
    
    return derstat;
}

/* -------------------------------------------------------------------------- */

/*
 * Validate the build string
 * 
 * The root ticket includes "build string" entries for LLB and iBSS that indicate the build 
 * submission tag (e.g., iBoot-1194) of the iBoot project that was submitted to the build and 
 * was included in its nomination.
 * 
 * This build string is also stamped into the binary and is accessible via build_tag_string.
 * 
 * To guard against "mix and match" attacks, the executing instance's build string is compared
 * against the one in the root ticket; that is, the executing firmware is bound to the ticket 
 * being validated.
 */ 
static DERReturn
_DERApTicketValidateBuildString( const DERApTicket *ticket, bool isRestore )
{
    DERReturn derstat = -1;
    DERTag tmp;
    DERItem tmpItem;

    if ( GetBuildStringTagForRunningInstance(&tmp, isRestore) == DR_Success ) {
        unsigned i, length;
        char ticketBuildString[128];

        derstat = DERFindItemWithTag( &(ticket->body), tmp | ASN1_CONTEXT_SPECIFIC, &tmpItem );
        if ( derstat != DR_Success ) {
            TICKETLOG( "missing build string %u", tmp );
            derstat = -1;
            goto exit;
        }

        require_action( tmpItem.length < sizeof(ticketBuildString), exit, derstat = -1; TICKETLOG("malformed build string") );
        
        /* 
         * Remove any trailing build string turds.
         * <rdar://problem/9694009> Manifest "BuildString" entries have an undesired suffix
         */
        length = strlcpy( ticketBuildString, (const char *) tmpItem.data, sizeof(ticketBuildString) );
        for ( i = 0; i < length; ++i ) {
            if ( ticketBuildString[i] == '~' ) {
                ticketBuildString[i] = '\0';
                break;
            }
        }

        if ( strncmp(build_tag_string, ticketBuildString, sizeof(ticketBuildString)) != 0 ) {
            TICKETLOG( "mismatched build string\n     ticket: %s\n    current: %s", ticketBuildString, build_tag_string );
            derstat = -1;
            goto exit;
        }
    }
    
    derstat = DR_Success;
    goto exit;
    
exit:
    return derstat;
}

/* -------------------------------------------------------------------------- */

static DERReturn
DERApTicketValidateProductionMode( const DERItem *container )
{
    DERReturn derstat = -1;
    uint32_t tmp32 = platform_get_raw_production_mode();
#if !TICKET_UNITTEST
    bool acceptGlobalTicket = mib_get_bool(kMIBTargetAcceptGlobalTicket);
#else
    bool acceptGlobalTicket = false;
#endif

    if (acceptGlobalTicket) {
        DERItem tmpItem;
        derstat = DERFindItemWithTag( container, 4 | ASN1_CONTEXT_SPECIFIC, &tmpItem );
        require( derstat == DR_Success, exit );
    }
    if (!acceptGlobalTicket || tmp32)
    {
        derstat = DERApTicketFindAndValidateTag( container, 4 | ASN1_CONTEXT_SPECIFIC, &tmp32, sizeof(tmp32) );
        require( derstat == DR_Success, exit );
    }

    derstat = DR_Success;

exit:
    return derstat;
}

/* -------------------------------------------------------------------------- */

static DERReturn
DERApTicketValidateCertExtension( const DERItem *contents )
{
    DERReturn derstat = -1;
    uint32_t tmp32;
    DERItem tmpItem;
    DERDecodedInfo extensionInfo;
    DERApTicketCertExtension extension;

    bzero( &extension, sizeof(extension) );

    derstat = DERDecodeItem( contents, &extensionInfo );
    require_action( derstat == DR_Success, exit, TICKETLOG("malformed cert extension") );
    
    derstat = DERParseSequenceContent( &(extensionInfo.content), DERNumApTicketCertExtensionItemSpecs, DERApTicketCertExtensionItemSpecs, &extension, 0 );
    require_action( derstat == DR_Success, exit, TICKETLOG("malformed cert extension") );

    /* We only know how to handle version 0 (in which the version tag isn't provided) */
    require_action( extension.version.data == NULL, exit, derstat = -1; TICKETLOG("unsupported extension version") );
    
    /* These are provided for future expansion.  For now, forbid them.*/
    require_action( extension.append.data == NULL, exit, derstat = -1; TICKETLOG("unsupported extension") );
    require_action( extension.remove.data == NULL, exit, derstat = -1; TICKETLOG("unsupported extension") );
    
    /* An override is required */
    require_action( extension.override.data != NULL, exit, derstat = -1; TICKETLOG("incomplete extension") );

    /* Validate the chipid (tag=2) */
    tmp32 = platform_get_chip_id();
    derstat = DERApTicketFindAndValidateTag( &(extension.override), 2 | ASN1_CONTEXT_SPECIFIC, &tmp32, sizeof(tmp32) );
    require( derstat == DR_Success, exit );
    
    /* Validate the production mode (tag=4) */
    derstat = DERApTicketValidateProductionMode( &(extension.override) );
    require_action( derstat == DR_Success, exit, derstat = -1; TICKETLOG("invalid production mode") );
    
    /* Validate the security domain (tag=5) */
    tmp32 = platform_get_security_domain();
    derstat = DERApTicketFindAndValidateTag( &(extension.override), 5 | ASN1_CONTEXT_SPECIFIC, &tmp32, sizeof(tmp32) );
    require( derstat == DR_Success, exit );

    /* Validate the certificate epoch (tag=235) */
    derstat = DERFindItemWithTag( &(extension.override), 235 | ASN1_CONTEXT_SPECIFIC, &tmpItem );
    require_action( derstat == DR_Success, exit, TICKETLOG("missing cert epoch") );
    require_action( tmpItem.length == sizeof(tmp32), exit, derstat = -1; TICKETLOG("malformed cert epoch") );
    memcpy( &tmp32, tmpItem.data, sizeof(tmp32) );
    require_action( platform_get_hardware_epoch() <= tmp32, exit, derstat = -1; TICKETLOG("expired cert epoch") );

    derstat = DR_Success;

exit:
    return derstat;
}

/* -------------------------------------------------------------------------- */

DERReturn
DERApTicketValidate( const DERApTicket *ticket, bool isRestore )
{
    DERReturn derstat = -1;
    DERItem tmpItem;
    uint32_t tmp32;
    uint64_t tmp64;
    uint8_t  nonceHash[CCSHA1_OUTPUT_SIZE];
#if !TICKET_UNITTEST
    bool acceptGlobalTicket = mib_get_u32(kMIBTargetAcceptGlobalTicket);
#else
    bool acceptGlobalTicket = false;
#endif

    /* Validate the ecid (tag=1) */
    derstat = DERFindItemWithTag( &(ticket->body), 1 | ASN1_CONTEXT_SPECIFIC, &tmpItem );
    if (!acceptGlobalTicket) {
          require( derstat == DR_Success, exit );
    }

    if (derstat == DR_Success )
    {
        tmp64 = platform_get_ecid_id();
        derstat = DERApTicketFindAndValidateTag( &(ticket->body), 1 | ASN1_CONTEXT_SPECIFIC, &tmp64, sizeof(tmp64) );
        require( derstat == DR_Success, exit );
    }
    
    /* Validate the chipid (tag=2) */
    tmp32 = platform_get_chip_id();
    derstat = DERApTicketFindAndValidateTag( &(ticket->body), 2 | ASN1_CONTEXT_SPECIFIC, &tmp32, sizeof(tmp32) );
    require( derstat == DR_Success, exit );
    
    /* Validate the boardid (tag=3) */
    tmp32 = platform_get_board_id();
    derstat = DERApTicketFindAndValidateTag( &(ticket->body), 3 | ASN1_CONTEXT_SPECIFIC, &tmp32, sizeof(tmp32) );
    require( derstat == DR_Success, exit );
    
    /* Validate the production mode (tag=4) */
    derstat = DERApTicketValidateProductionMode( &(ticket->body) );
    require( derstat == DR_Success, exit );
    
    /* Validate the security domain (tag=5) */
    tmp32 = platform_get_security_domain();
    derstat = DERApTicketFindAndValidateTag( &(ticket->body), 5 | ASN1_CONTEXT_SPECIFIC, &tmp32, sizeof(tmp32) );
    require( derstat == DR_Success, exit );

    /* Validate the build tag */
    derstat = _DERApTicketValidateBuildString( ticket, isRestore );
    require( derstat == DR_Success, exit );

    /*
     * Restore validation is a superset of normal boot validation
     */
    if ( isRestore ) {

        /*
         * Validate the nonce (tag=18)
         */
        derstat = DERFindItemWithTag( &(ticket->body), 18 | ASN1_CONTEXT_SPECIFIC, &tmpItem );
        if ( derstat == DR_Success ) {
            if ( tmpItem.length != sizeof(nonceHash) ) {
                TICKETLOG( "malformed nonce" );
                derstat = -1;
                goto exit;
            }

            tmp64 = platform_get_nonce();
            sha1_calculate( &tmp64, sizeof(tmp64), nonceHash );

            if ( memcmp(nonceHash, tmpItem.data, sizeof(nonceHash)) != 0 ) {
#if TICKETLOG_ENABLED
                char hash1[2 * CCSHA1_OUTPUT_SIZE + 1];
                char hash2[2 * CCSHA1_OUTPUT_SIZE + 1];
                sprint_hex( tmpItem.data, tmpItem.length, hash1, sizeof(hash1) );
                sprint_hex( nonceHash, sizeof(nonceHash), hash2, sizeof(hash2) );
#endif

                TICKETLOG( "mismatched nonce\n    ticket:  %s\n    current: %s", hash1, hash2 );
                derstat = -1;
                goto exit;
            }
        } else {
            if (!acceptGlobalTicket) {
                TICKETLOG( "missing nonce" );
                derstat = -1;
                goto exit;
            }
        }
    }

    derstat = DR_Success;

exit:    
    return derstat;
}

/* -------------------------------------------------------------------------- */

DERReturn
DERApTicketParseLengthFromBuffer( const uint8_t *buffer, size_t length, size_t *outTicketLength )
{
    DERReturn derstat = -1;
    DERItem ticketItem;
    DERDecodedInfo ticketInfo;
    
    require( buffer != NULL, exit );
    require( outTicketLength != NULL, exit );
    
    /* Parse just enough of the buffer to determine the ticket length */
    ticketItem.data = (DERByte *) buffer;
    ticketItem.length = (DERSize) length;

    derstat = DERDecodeItem( &ticketItem, &ticketInfo );
    require_action( derstat == DR_Success, exit, TICKETLOG("failed to decode buffer") );

        /*
         * "ticketInfo" describes a pointer and length to content within "buffer".
         * The difference of that pointer and "buffer" is the number of bytes used to describe the content.
         * The overall ticket length is the content length summed with the number of bytes used to describe it.
         */
        *outTicketLength = (size_t) ticketInfo.content.length + (ticketInfo.content.data - buffer);

    derstat = DR_Success;

exit:
    return derstat;
}

/* -------------------------------------------------------------------------- */
