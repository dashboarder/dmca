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
#include <drivers/sha1.h>
#include <lib/fs.h>
#include <lib/blockdev.h>
#include <lib/env.h>
#include <lib/image.h>
#include <lib/mib.h>
#include <platform.h>
#include <stdlib.h>
#include <sys/menu.h>
#include <AssertMacros.h>
#include <lib/ticket.h>
#include <lib/DERApTicket.h>

/* -------------------------------------------------------------------------- */

/* <rdar://problem/9447707> Disable TICKET_LOOSE_VERIFICATION for the release build */
#if !RELEASE_BUILD
#define TICKET_LOOSE_VERIFICATION 1
#endif

#define MAX_TICKET_LENGTH 8192

/* -------------------------------------------------------------------------- */

static uint8_t *gRootTicketBuffer = NULL;
static size_t gRootTicketLength = 0;
static DERApTicket *gRootTicket = NULL;
static uint8_t gRootTicketHashBuffer[CCSHA1_OUTPUT_SIZE];
static bool gRootTicketIsRestore = false;

/* -------------------------------------------------------------------------- */

size_t sprint_hex(const uint8_t *data, unsigned data_len, char *buf, unsigned buf_len)
{
    static const char kAsciiHexChars[] = "0123456789ABCDEF";
    unsigned i, j;

    /* ensure the output buffer is large enough to contain the hex string */
    if (buf_len < (data_len * 2) + 1) {
        j = -1;
        goto exit;
    }

    j = 0;
    for ( i = 0; i < data_len; ++i ) {
        uint8_t octet = data[i];
        buf[j++] = kAsciiHexChars[((octet & 0xF0) >> 4)];
        buf[j++] = kAsciiHexChars[(octet & 0x0F)];
    }

    buf[j] = '\0';
    
exit:
    return j;
}

/* -------------------------------------------------------------------------- */

typedef struct {
    uint32_t type;
    uint8_t  boot_mode;
    uint8_t  hash_tag;
    uint8_t  trust_tag;
    uint8_t  build_tag;
} image3_ticket_tags;

#define kTicketBootRestore      (0x1U)
#define kTicketBootNormal       (0x2U)
#define kTicketBootAny          (kTicketBootRestore | kTicketBootNormal)

static const image3_ticket_tags kImage3TicketTags[] = {
/*    type                          boot_mode            hash_tag  trust_tag   build_tag   */
    { IMAGE_TYPE_DIAG,              kTicketBootAny,           11,        52,          0,   },
    { IMAGE_TYPE_LLB,               kTicketBootNormal,       228,       231,          6,   },
    { IMAGE_TYPE_IBOOT,             kTicketBootAny,            7,        48,          0,   },
    { IMAGE_TYPE_DEVTREE,           kTicketBootNormal,         9,        50,          0,   },
    { IMAGE_TYPE_KERNELCACHE,       kTicketBootNormal,        10,        51,          0,   },
    { IMAGE_TYPE_LOGO,              kTicketBootNormal,         8,        49,          0,   },
    { IMAGE_TYPE_NEEDSERVICE,       kTicketBootNormal,        17,        58,          0,   },
    { IMAGE_TYPE_GLYPHCHRG,         kTicketBootNormal,        12,        53,          0,   },
    { IMAGE_TYPE_GLYPHPLUGIN,       kTicketBootNormal,        13,        54,          0,   },
    { IMAGE_TYPE_BATTERYCHARGING0,  kTicketBootNormal,        78,        84,          0,   },
    { IMAGE_TYPE_BATTERYCHARGING1,  kTicketBootNormal,        79,        85,          0,   },
    { IMAGE_TYPE_BATTERYLOW0,       kTicketBootNormal,        14,        55,          0,   },
    { IMAGE_TYPE_BATTERYLOW1,       kTicketBootNormal,        15,        56,          0,   },
    { IMAGE_TYPE_BATTERYFULL,       kTicketBootNormal,        80,        86,          0,   },
    { IMAGE_TYPE_RECMODE,           kTicketBootAny,           16,        57,          0,   },
    { IMAGE_TYPE_IBSS,              kTicketBootRestore,      229,       232,         20,   },
    { IMAGE_TYPE_IBEC,              kTicketBootRestore,      230,       233,         22,   },
    { IMAGE_TYPE_LOGO,              kTicketBootRestore,       23,        59,          0,   },
    { IMAGE_TYPE_DEVTREE,           kTicketBootRestore,       24,        60,          0,   },
    { IMAGE_TYPE_KERNELCACHE,       kTicketBootRestore,       25,        61,          0,   },
    { IMAGE_TYPE_RAMDISK,           kTicketBootRestore,       26,        62,          0,   },
};

/* -------------------------------------------------------------------------- */

static const image3_ticket_tags *
ticket_get_image3_tags( uint32_t type )
{
    /*
     * Infer whether this is a normal boot or a restore.
     */
    unsigned boot_mode;
    uint32_t product = mib_get_u32(kMIBTargetApplicationProduct);

    if ((product == MIB_APP_PROD_IBOOT) || (product == MIB_APP_PROD_LLB)) {
        boot_mode = (type == IMAGE_TYPE_IBEC) ? kTicketBootRestore : kTicketBootNormal;
    } else {
        boot_mode = kTicketBootRestore;
    }
    
    TICKETLOG( "      boot mode: %s", (boot_mode == kTicketBootRestore) ? "restore" : "normal" );

    unsigned i;
    unsigned count = sizeof(kImage3TicketTags) / sizeof(kImage3TicketTags[0]);
    const image3_ticket_tags *found = NULL;
    
    for ( i = 0; i < count; ++i ) {
        if ( kImage3TicketTags[i].type == type && kImage3TicketTags[i].boot_mode & boot_mode ) {
            found = &kImage3TicketTags[i];
            break;
        }
    }

    return found;
}

/* -------------------------------------------------------------------------- */

bool
ticket_validation_required()
{
    bool required;

    /*
     * <rdar://problem/9391858> Revised iBoot ticket policy
     * <rdar://problem/9414797> Don't allow replay of old iBECs on customer units
     * <rdar://problem/9790456> Modify ticket validation policy for K66
     */

#if RELEASE_BUILD
     required = !( platform_get_chip_id() == 0x8930 && platform_get_board_id() == 0x10 );
#else
     required = false;
#endif

     return required;
}

/* -------------------------------------------------------------------------- */

bool 
ticket_copy_tag_data( DERTag tag, uint8_t *buffer, size_t *ioLength )
{
    int status = -1;
    DERReturn derstat;
    DERSize dataLength = (DERSize) *ioLength;

    require_action( gRootTicket != NULL, exit, TICKETLOG("no ticket") );

    derstat = DERApTicketCopyTagData( &(gRootTicket->body), tag, buffer, &dataLength );
    require( derstat == DR_Success, exit );
    
    *ioLength = (size_t) dataLength;
    
    status = 0;

exit:
    return (status == 0);
}

/* -------------------------------------------------------------------------- */

bool
ticket_validate_image3( uint32_t type, uint32_t expectedType, uint8_t *hashBuffer, size_t hashBufferSize, bool *outIsUntrusted )
{
    bool valid = false;
    const image3_ticket_tags *ticketTags;
    uint8_t ticketEntryHashBuffer[CCSHA1_OUTPUT_SIZE];
    size_t ticketEntryHashSize = sizeof(ticketEntryHashBuffer);
    uint32_t trustedEncoding;
    size_t trustedEncodingSize = sizeof(trustedEncoding);
    char hashLog[2 * CCSHA1_OUTPUT_SIZE + 1];
    
    TICKETLOG( "examining image: type=0x%08x ('%c%c%c%c')", type, (type >> 24) & 0xFF, (type >> 16) & 0xFF, (type >> 8) & 0xFF, type & 0xFF );
    
    if ( expectedType != IMAGE_TYPE_ANY && expectedType != type ) {
        TICKETLOG( " expected image: type=0x%08x ('%c%c%c%c')", expectedType, (expectedType >> 24) & 0xFF, (expectedType >> 16) & 0xFF, (expectedType >> 8) & 0xFF, expectedType & 0xFF );
        goto exit;
    }

    ticketTags = ticket_get_image3_tags(type);
    if (ticketTags == NULL) {
        TICKETLOG("this image isn't supported by the ticket model");
        goto exit;
    }

    if (ticketTags->boot_mode == kTicketBootRestore && gRootTicket != NULL && !gRootTicketIsRestore) {
        TICKETLOG("a restore ticket is required");
        goto exit;
    }

    if (!ticket_copy_tag_data(ticketTags->hash_tag, ticketEntryHashBuffer, &ticketEntryHashSize)) {
        TICKETLOG("this tag (%d) isn't in the ticket", ticketTags->hash_tag);
        goto exit;
    }
    
    if (ticketEntryHashSize != sizeof(ticketEntryHashBuffer)) {
        TICKETLOG("unrecognized ticket encoding for hash tag (%d)", ticketTags->hash_tag);
        goto exit;
    }
        
    sprint_hex(ticketEntryHashBuffer, sizeof(ticketEntryHashBuffer), hashLog, sizeof(hashLog));
    TICKETLOG("    ticket hash: %s", hashLog);
    
    sprint_hex(hashBuffer, hashBufferSize, hashLog, sizeof(hashLog));
    TICKETLOG("     image hash: %s", hashLog);
    
    /* compare hash_buffer to the hash of the image3 */
    if (memcmp(hashBuffer, ticketEntryHashBuffer, sizeof(ticketEntryHashBuffer)) != 0) {
        TICKETLOG("this image doesn't match the one specified by ticket tag (%d)", ticketTags->hash_tag);
        goto exit;
    }

    if (!ticket_copy_tag_data(ticketTags->trust_tag, (void *) &trustedEncoding, &trustedEncodingSize)) {
        TICKETLOG("ticket lacks trust tag (%d) for this image; assuming untrusted", ticketTags->trust_tag);
        *outIsUntrusted = true;
    }
    else {
        if (trustedEncodingSize != sizeof(trustedEncoding)) {
            TICKETLOG("unrecognized ticket encoding for trust tag (%d)", ticketTags->trust_tag);
            goto exit;
        }

        TICKETLOG(" explicit trust: %s", trustedEncoding ? "yes" : "no");
        *outIsUntrusted = trustedEncoding ? false : true;
    }

    valid = true;

exit:
    return valid;
}

/* -------------------------------------------------------------------------- */

int
ticket_set( const uint8_t *buffer, size_t length, bool isRestore, size_t *outTicketLength )
{
    int status = -1;
    DERReturn derstat;
    DERItem ticketItem;
    size_t ticketLength;
    uint8_t *ticketBuffer = NULL;
    DERApTicket *apTicket = NULL;
        
    require( buffer != NULL, exit );
    require( length > 0, exit );

    /* Parse just enough of the buffer to determine the ticket length */
    derstat = DERApTicketParseLengthFromBuffer( buffer, length, &ticketLength );
    require_action( derstat == DR_Success, exit, TICKETLOG("can't parse ticket length") );
    require_action( ticketLength > 0 && ticketLength < MAX_TICKET_LENGTH, exit, TICKETLOG("unreasonable ticket length: %d bytes", ticketLength) );

    /* Copy the ticket data to a separate buffer */
    ticketBuffer = (uint8_t *) malloc( ticketLength );
    require_action( ticketBuffer != NULL, exit, TICKETLOG("failed to alloc ticket buffer") );
    memcpy( ticketBuffer, buffer, ticketLength );

    /* Allocate a ticket structure */
    apTicket = (DERApTicket *) calloc( 1, sizeof(DERApTicket) );
    require_action( apTicket != NULL, exit, TICKETLOG("failed to alloc ticket structure") );

    /* Decode */
    ticketItem.data = (DERByte *) ticketBuffer;
    ticketItem.length = (DERSize) ticketLength;
    derstat = DERApTicketDecode( &ticketItem, apTicket, (DERSize *) &ticketLength );
    require_action( derstat == DR_Success, exit, TICKETLOG("malformed ticket") );

    /* Validate */
    derstat = DERApTicketValidate( apTicket, isRestore );

#if TICKET_LOOSE_VERIFICATION
    derstat = DR_Success;
#endif

    require_action( derstat == DR_Success, exit, TICKETLOG("ticket rejected") );

    /* Destroy any older ticket */
    if ( gRootTicketBuffer != NULL ) {
        free( gRootTicketBuffer );
        gRootTicketBuffer = NULL;
        gRootTicketLength = 0;
    }
    
    if ( gRootTicket != NULL ) {
        free( gRootTicket );
        gRootTicket = NULL;
    }
    
    /* Cache this ticket */
    gRootTicketBuffer = ticketBuffer;
    gRootTicketLength = ticketLength;
    gRootTicket = apTicket;
    gRootTicketIsRestore = isRestore;

    /* Cache the new ticket digest */
    sha1_calculate( gRootTicketBuffer, gRootTicketLength, gRootTicketHashBuffer );

    /* Debug logging */
    {
        char hashStr[2 * CCSHA1_OUTPUT_SIZE + 1];
        sprint_hex(gRootTicketHashBuffer, sizeof(gRootTicketHashBuffer), hashStr, sizeof(hashStr));
        TICKETLOG( "cached ticket (%u bytes) hash: %s", ticketLength, hashStr );
    }

    if ( outTicketLength != NULL ) {
        *outTicketLength = ticketLength;
    }

    /* success */
    status = 0;
    
exit:
    if ( status != 0 ) {
        if ( ticketBuffer != NULL ) {
            free( ticketBuffer );
        }
        
        if ( apTicket != NULL ) {
            free( apTicket );
        }
    }
    
    return status;
}

/* -------------------------------------------------------------------------- */

/*
 *  "ticket" command handler
 *  This command allows the host to specify a temporary RAM-based ticket.
 */
int
do_ticket( int argc, struct cmd_arg *argv )
{
    int status = -1;
    size_t ticketLength;
    size_t bufferLength;

    bufferLength = env_get_uint( "filesize", 0 );
    
    require_action( bufferLength > 0, exit, status = -1; TICKETLOG("filesize invalid or not set") );    
    require_action( bufferLength < MAX_TICKET_LENGTH, exit, status = -1; TICKETLOG("filesize too large") );
    check( MAX_TICKET_LENGTH <= mib_get_size(kMIBTargetDefaultLoadSize) );

    status = ticket_set( (uint8_t *) mib_get_addr(kMIBTargetDefaultLoadAddress), bufferLength, true, &ticketLength );
    require( status == 0, exit );
    require_action( ticketLength == bufferLength, exit, status = -1; TICKETLOG("garbage following ticket") );
    
    /* success */
    status = 0;

exit:
    return status;
}


/* -------------------------------------------------------------------------- */

#if DEBUG_BUILD
int
do_ticket_dump( int argc, struct cmd_arg *argv )
{
    int status = -1;
    
    if ( gRootTicketBuffer == NULL ) {
        printf( "no ticket\n");
        goto exit;
    }

    hexdump( gRootTicketBuffer, gRootTicketLength );

    status = 0;
    
exit:
    return status;
}
#endif


/* -------------------------------------------------------------------------- */

bool
ticket_get_hash( uint8_t *buffer, size_t length )
{
    bool copied;
    
    if ( gRootTicketBuffer != NULL && length >= sizeof(gRootTicketHashBuffer) ) {
        memcpy( buffer, gRootTicketHashBuffer, sizeof(gRootTicketHashBuffer) );
        copied = true;
    }
    else {
        copied = false;
    }
    
    return copied;
}

/* ========================================================================== */


/*
 * Load a ticket from a file.
 */
int
ticket_load_file( const char *filePath, uintptr_t bufferAddress, size_t bufferLength )
{
    int status = -1;

    size_t fileLength = bufferLength;
    size_t ticketLength;

    status = fs_load_file(filePath, bufferAddress, &fileLength);
    require_action( status == 0, exit, status = -1; TICKETLOG("failed to load ticket file: %s", filePath) );

    status = ticket_set( (uint8_t *) bufferAddress, fileLength, true, &ticketLength );
    require_action( status == 0, exit, status = -1; TICKETLOG("unable to set ticket") );

    require_action( ticketLength == fileLength, exit, status = -1; TICKETLOG("garbage following ticket") );

    status = 0;
    TICKETLOG("successfully loaded ticket file: %s", filePath);

exit:
    return status;
}

int
do_ticket_file( int argc, struct cmd_arg *args )
{
    int status = -1;

    require_action( argc == 2, exit, printf("%s <path>\n", args[0].str) );

    status = ticket_load_file( args[1].str,
                               mib_get_addr(kMIBTargetDefaultLoadAddress),
                               mib_get_size(kMIBTargetDefaultLoadSize) );
    require( status == 0, exit );

    status = 0;

exit:
    return status;
}


/* -------------------------------------------------------------------------- */

int
ticket_load()
{
    int status = -1;

    struct image_info *imageInfo;
    uint8_t *dataBuffer;
    size_t dataLength;
    size_t ticketLength;
    size_t objectLength;
    uint8_t *objectBuffer = NULL;
    bool restoreTicket;

    /* Find the ticket image */
    imageInfo = image_find( 'SCAB' );
    require_action( imageInfo != NULL, exit, TICKETLOG("no ticket found") );
    require_action( imageInfo->imagePrivateMagic == IMAGE3_IMAGE_INFO_MAGIC, exit, TICKETLOG("ticket is not image3") );

    /* 
     * Allocate a buffer for the image and load it into the buffer
     * without attempting to validate it.
     */
    objectLength = imageInfo->imageAllocation;
    objectBuffer = (uint8_t *) malloc( objectLength );
    imageInfo->imageOptions |= IMAGE_OPTION_JUST_LOAD;

    /*
     * Accept any type (pass NULL)
     */
    if ( image_load(imageInfo, NULL, 0, NULL, (void **)&objectBuffer, &objectLength) ) {
        TICKETLOG( "failed to load ticket image3" );
        goto exit;
    }
    
    /* 
     * The ticket is always supplied in an image3 object with tags in
     * the order TYPE, DATA, resulting in an offset from the buffer to
     * the actual ticket data of 0x40 bytes.
     *
     * We don't try to introspect or validate the image here -
     * ticket_set below will reject the data if it is not a
     * well-formed ticket.
     */
    if (objectLength < 0x40)
	    goto exit;
    dataBuffer = objectBuffer + 0x40;
    dataLength = objectLength - 0x40;
    if (dataLength > MAX_TICKET_LENGTH)
	    goto exit;

    /* If the ticket comes from a memory blockdev,
       use the stricter restore ticket validation */
    restoreTicket = (imageInfo->imageOptions & IMAGE_OPTION_MEMORY) != 0;

    /*
     * Parse the ticket data.
     */
    status = ticket_set( dataBuffer, dataLength, restoreTicket, &ticketLength );

exit:

    if ( objectBuffer != NULL ) {
        free( objectBuffer );
    }
    
    TICKETLOG( "exiting" );
    return status;
}

/* -------------------------------------------------------------------------- */

int
do_ticket_load( int argc, struct cmd_arg *argv )
{
    int status = -1;

    status = ticket_load();
    require( status == 0, exit );
    
    /* success */
    status = 0;

exit:
    return status;
}

/* -------------------------------------------------------------------------- */
