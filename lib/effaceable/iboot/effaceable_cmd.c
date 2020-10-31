/*
 * Copyright (c) 2010-11 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

// =============================================================================

#include <AppleEffaceableStorageFormat.h>
#include <AppleEffaceableStorageKeys.h>

#include <lib/effaceable.h>

#include <debug.h>
#include <sys/menu.h>
#include <drivers/sha1.h>

// =============================================================================

static int do_effaceable(int argc, struct cmd_arg *args);
static void printUsage(int argc, struct cmd_arg *args);
static void effaceable_hexdump(const void * buf, uint32_t count);
static int lockerList(void);

#if DEBUG_BUILD
static int consumeNonce(void);
#endif

// =============================================================================

static int
do_effaceable(int argc, struct cmd_arg *args)
{
    int err;

    if (1 >= argc) {
        printUsage(argc, args);
        err = 0;
    } else if (0 == strcmp("lockers", args[1].str)) {
        err = lockerList();
#if DEBUG_BUILD
    } else if (0 == strcmp("nonce", args[1].str)) {
        err = consumeNonce();
#endif
    } else {
        printUsage(argc, args);
        printf("unrecognized subcommand\n");
        err = -1;
    }

    return err;
}

// =============================================================================

static void
printUsage(int argc, struct cmd_arg *args)
{
    printf("USAGE: %s <subcmd>\n\n", ((0 < argc) ? args[0].str : "?"));
    printf("  Where <subcmd> is one of following\n\n");
    printf("    lockers - list lockers and contents\n");

#if DEBUG_BUILD
    printf("    nonce - consume nonce; then, report its SHA1 hash and value\n");
#endif

    printf("    ...(more to come)...\n\n");
}

// =============================================================================

static void
effaceable_hexdump(const void * buf, uint32_t count)
{
    uint32_t i, j;
    for (i = 0; i < count; i += 16) {
        printf("0x%08x:", ((uint32_t) buf) + i);
        for (j = 0; (j < 16) && ((i + j) < count); j++) {
            printf(" %02.2x", ((uint8_t*)buf)[i+j]);
        }
        printf("\n");
    }
}

// =============================================================================

#define UNTAG(x)                                    \
                                                    \
        (uint8_t)(((x) >> 24) & 0xff),              \
        (uint8_t)(((x) >> 16) & 0xff),              \
        (uint8_t)(((x) >> 8) & 0xff),               \
        (uint8_t)((x) & 0xff)

// =============================================================================

static int
lockerList(void)
{
    uint8_t *buf, *cursor;
    uint32_t size;
    AppleEffaceableStorageLockerHeader *header;
    
    size = effaceable_get_capacity();
    if (0 == size) {
        printf("ERROR: unable to get capacity\n");
        return -1;
    }

    buf = malloc(size);
    
    if (!effaceable_get_bytes(buf, 0, size)) {
        printf("ERROR: unable to read\n");
        return -1;
    }
    
    cursor = buf;
    for (;;) {
        header = (AppleEffaceableStorageLockerHeader *)cursor;
        if (header->magic != kAppleEffaceableStorageLockerMagic) {
            printf("WARNING: unexpected magic 0x%04x at %u, expected 0x%04x\n", 
                   header->magic, cursor - buf, kAppleEffaceableStorageLockerMagic);
            break;
        }
        if (header->type_id == kAppleEffaceableStorageLockerSentinel) {
            printf("<END>\n");
            break;
        }
        printf("0x%08x/0x%04x %c%c%c%c %s\n",
               header->type_id, 
               header->data_size, 
               UNTAG(header->type_id & ~kAppleEffaceableStorageLockerProtected),
               (header->type_id & kAppleEffaceableStorageLockerProtected) ? "(protected)" : "");
        cursor += sizeof(*header);
        if (header->data_size > 0) {
            if ((cursor + header->data_size) > (buf + size)) {
                printf("WARNING: data overflows buffer\n");
                break;
            }
            effaceable_hexdump(cursor, header->data_size);
            cursor += header->data_size;
        }
        if ((cursor + sizeof(*header)) > (buf + size)) {
            printf("WARNING: header overflows buffer\n");
            break;
        }
    }
    return 0;
}

// =============================================================================

// XXX correct hard-coding of SHA1 buffer size
#define SHA1_HASH_SIZE 20

static int
consumeNonce(void)
{
    uint64_t nonce;
    uint8_t hash[SHA1_HASH_SIZE];
    int idx;

    if (!effaceable_consume_nonce(&nonce)) {
        printf("ERROR: unable to consume nonce\n");
        return -1;
    }

    sha1_calculate(&nonce, sizeof(nonce), hash);

    for (idx = 0; idx < SHA1_HASH_SIZE; idx++) {
        printf("%s%02X", (idx ? " " : ""), hash[idx]);
    }
    printf(" (0x%016llX)\n", nonce);

    return 0;
}

// =============================================================================

MENU_COMMAND_DEBUG(effaceable, do_effaceable, "effaceable storage utilities", NULL);

// =============================================================================
