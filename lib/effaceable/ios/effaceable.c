/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <stdlib.h>
#include <err.h>
#include <sys/types.h>
#include <sys/random.h>
#include <unistd.h>
#include <fcntl.h>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

#include "AppleEffaceableStorageKeys.h"
#include "AppleEffaceableStorageFormat.h"
#include "effaceable.h"

// =============================================================================

// XXX correct hard-coding of SHA1 buffer size
#define SHA1_HASH_SIZE 20

// =============================================================================

bool
connectClient(const char * name, io_service_t * service, io_connect_t * connection)
{
    bool            ok = false;
    mach_port_t     master;
    CFDictionaryRef match;
    io_service_t    found_service;
    io_connect_t    found_connection;

    if (KERN_SUCCESS != IOMasterPort(MACH_PORT_NULL, &master)) {
        warnx("IOMasterPort failed");
    } else if (IO_OBJECT_NULL == (match = IOServiceMatching(name))) {
        warnx("IOServiceMatching failed for %s", name);
    } else if (IO_OBJECT_NULL == (found_service = IOServiceGetMatchingService(master, match))) {
        warnx("IOServiceGetMatchingService failed");
    } else if (KERN_SUCCESS != IOServiceOpen(found_service, mach_task_self(), 0, &found_connection)) {
        warnx("IOServiceOpen failed for class '%s'", name);
    } else {
        *service = found_service;
        *connection = found_connection;
        ok = true;
    }

    if (!ok) {
        // XXX clean up
    }

    return ok;
}

bool
wipeStorage(io_connect_t connection)
{
    bool ok = false;

    if (KERN_SUCCESS != IOConnectCallScalarMethod(connection,
                                                  kAppleEffaceableStorageMethodWipeStorage,
                                                  NULL,
                                                  0,
                                                  NULL,
                                                  NULL)) {
        errx(1, "wipe failed");
    } else {
        ok = true;
    }
    return ok;
}

bool
getCapacity(io_connect_t connection, IOByteCount * capacity)
{
    bool ok = false;
    const uint32_t expected_count = 1;
    uint32_t output_count = expected_count;
    uint64_t output[output_count];

    if (KERN_SUCCESS != IOConnectCallScalarMethod(connection,
                                                  kAppleEffaceableStorageMethodGetCapacity,
                                                  NULL,
                                                  0,
                                                  output,
                                                  &output_count)) {
        warnx("IOConnectCallScalarMethod failed");
    } else if (expected_count != output_count) {
        warnx("unexpected scalar output count");
    } else {
        *capacity = (IOByteCount) output[0];
        ok = true;
    }
    return ok;
}

bool
getBytes(io_connect_t connection, void * buf, IOByteCount offset, IOByteCount count)
{
    bool ok = false;
    uint64_t input;
    size_t output_count = count;

    input = (uint64_t)offset;

    if (KERN_SUCCESS != IOConnectCallMethod(connection,
                                            kAppleEffaceableStorageMethodGetBytes,
                                            &input,
                                            1,
                                            NULL,
                                            0,
                                            NULL,
                                            NULL,
                                            buf,
                                            &output_count)) {
        warnx("IOConnectCallMethod failed");
    } else if (count != output_count) {
        warnx("unexpected output struct count");
    } else {
        ok = true;
    }
    return ok;
}

bool
setBytes(io_connect_t connection, const void * buf, IOByteCount offset, IOByteCount count)
{
    bool ok = false;
    uint64_t input;

    input = (uint64_t)offset;

    if (KERN_SUCCESS != IOConnectCallMethod(connection,
                                            kAppleEffaceableStorageMethodSetBytes,
                                            &input,
                                            1,
                                            buf,
                                            count,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL)) {
        warnx("IOConnectCallMethod failed");
    } else {
        ok = true;
    }
    return ok;
}

bool
isFormatted(io_connect_t connection, bool * is_formatted)
{
    bool ok = false;
    const uint32_t expected_count = 1;
    uint32_t output_count = expected_count;
    uint64_t output[output_count];

    if (KERN_SUCCESS != IOConnectCallScalarMethod(connection,
                                                  kAppleEffaceableStorageMethodIsFormatted,
                                                  NULL,
                                                  0,
                                                  output,
                                                  &output_count)) {
        warnx("IOConnectCallScalarMethod failed");
    } else if (expected_count != output_count) {
        warnx("unexpected scalar output count");
    } else {
        *is_formatted = (bool) output[0];
        ok = true;
    }
    return ok;
}

bool
formatStorage(io_connect_t connection)
{
    bool ok = false;

    if (KERN_SUCCESS != IOConnectCallScalarMethod(connection,
                                                  kAppleEffaceableStorageMethodFormatStorage,
                                                  NULL,
                                                  0,
                                                  NULL,
                                                  NULL)) {
        warnx("IOConnectCallScalarMethod failed");
    } else {
        ok = true;
    }
    return ok;
}

// =============================================================================

int
main(int argc, char *argv[])
{
    io_service_t    service;
    io_connect_t    connection;

    if (0 != getuid()) {
        errx(1, "must be run as root (uid = %d)", getuid());
    }

    if (!connectClient(kAppleEffaceableStorageClassName, &service, &connection)) {
        errx(1, "unable to connect to service");
    }

    // crude command parser...really only intended as test vehicle
    if (1 >= argc) {
        printUsage(argc, argv);
    } else {
        if (0 == strcmp("format", argv[1])) {
            if (formatStorage(connection)) {
                printf("format succeeded\n");
            } else {
                printf("format failed\n");
            }
        } else if (0 == strcmp("wipe", argv[1])) {
            wipeStorage(connection);
        } else if (0 == strcmp("test", argv[1])) {
            quickTest(connection);
        } else if (0 == strcmp("lockers", argv[1])) {
            lockerList(connection);
        } else if (0 == strcmp("nonce", argv[1])) {
            generateNonce(connection);
        } else if (2 >= argc) {
            printUsage(argc, argv);
        } else if (0 == strcmp("efface", argv[1])) {
            lockerEfface(connection, argv[2]);
        } else if (3 >= argc) {
            printUsage(argc, argv);
        } else if (0 == strcmp("set", argv[1])) {
            lockerSet(connection, argv[2], argv[3]);
        } else if (0 == strcmp("get", argv[1])) {
            lockerGet(connection, argv[2], argv[3]);
        } else {
            printUsage(argc, argv);
            errx(1, "unrecognized subcommand");
        }
    }

    // XXX clean up service and connection

    return 0;
}

void
printUsage(int argc, char *argv[])
{
    printf("USAGE: %s <subcmd>\n\n", ((0 < argc) ? argv[0] : "?"));
    printf("  Where <subcmd> is one of following\n\n");
    printf("    format  - format storage\n");
    printf("    wipe    - wipe storage\n");
    printf("    test    - perform quick test\n");
    printf("    lockers - list lockers and contents\n");
    printf("    nonce - generate a nonce and report its SHA1 hash\n");
    printf("    set <tag> <file> - write <file> into locker <tag>\n");
    printf("    get <tag> <file> - write <file> from locker <tag>\n");
    printf("\n");
}

// =============================================================================

void
hexdump(const void * buf, IOByteCount count)
{
    int i, j;
    for (i = 0; i < count; i += 16) {
        printf("0x%08x:", ((unsigned) buf) + i);
        for (j = 0; (j < 16) && ((i + j) < count); j++) {
            printf(" %2.2x", ((uint8_t*)buf)[i+j]);
        }
        printf("\n");
    }
}

void
quickTest(io_connect_t connection)
{
    const IOByteCount count = 64;
    IOByteCount offset;
    IOByteCount capacity;
    void * sbuf = NULL;
    void * gbuf = NULL;
    unsigned i;

    if (!getCapacity(connection, &capacity)) {
        errx(1, "unable to get capacity");
    }
    printf("capacity reported as %u; expected 960\n", (unsigned) capacity);

    offset = capacity - count - 1;
    sbuf = malloc(count);
    gbuf = malloc(count);

    if ((NULL == sbuf) || (NULL == gbuf)) {
        errx(1, "failed to allocate buffers");
    }

    if (!getBytes(connection, gbuf, offset, count)) {
        errx(1, "unable to get bytes");
    }
    printf("read %u bytes from %u offset\n", (unsigned) count, (unsigned) offset);
    hexdump(gbuf, count);

    srandom(((unsigned *)gbuf)[0]);
    for (i = 0; i < (count / sizeof(long)); i++) {
        ((long *)sbuf)[i] = random();
    }

    if (!setBytes(connection, sbuf, offset, count)) {
        errx(1, "unable to set bytes");
    }
    printf("wrote %u bytes to %u offset\n", (unsigned) count, (unsigned) offset);
    hexdump(sbuf, count);

    if (!getBytes(connection, gbuf, offset, count)) {
        errx(1, "unable to get bytes");
    }
    printf("read %u bytes from %u offset\n", (unsigned) count, (unsigned) offset);
    hexdump(gbuf, count);

    if (0 != bcmp(sbuf, gbuf, count)) {
        errx(1, "bytes read back differ from those written");
    }
    printf("test passed\n");

    if (NULL != sbuf) {
        free(sbuf);
    }
    if (NULL != gbuf) {
        free(gbuf);
    }
}

// =============================================================================

#define UNTAG(x)                (int)(((x) >> 24) & 0xff),(int)(((x) >> 16) & 0xff),(int)(((x) >> 8) & 0xff),(int)((x) & 0xff)

void
lockerList(io_connect_t connection)
{
    uint8_t *buf, *cursor;
    IOByteCount size;
    AppleEffaceableStorageLockerHeader *header;

    if (!getCapacity(connection, &size))
        errx(1, "unable to get capacity");
    if (size != kAppleEffaceableStorageLockerSize)
        warnx("size %lu not %lu as expected", size, kAppleEffaceableStorageLockerSize);
    buf = malloc(size);

    if (!getBytes(connection, buf, 0, size))
        errx(1, "unable to read");

    cursor = buf;
    for (;;) {
        header = (AppleEffaceableStorageLockerHeader *)cursor;
        if (header->magic != kAppleEffaceableStorageLockerMagic) {
            warnx("unexpected magic 0x%04x at %u, expected 0x%04x", header->magic, cursor - buf, kAppleEffaceableStorageLockerMagic);
            break;
        }
        if (header->type_id == kAppleEffaceableStorageLockerSentinel) {
            printf("<END>\n");
            break;
        }
        printf("0x%08x/0x%04x %c%c%c%c %s\n",
               (unsigned)header->type_id, header->data_size, UNTAG(header->type_id),
               (header->type_id & kAppleEffaceableStorageLockerProtected) ? "(protected)" : "");
        cursor += sizeof(*header);
        if (header->data_size > 0) {
            if ((cursor + header->data_size) > (buf + size)) {
                warnx("data overflows buffer");
                break;
            }
            hexdump(cursor, header->data_size);
            cursor += header->data_size;
        }
        if ((cursor + sizeof(*header)) > (buf + size)) {
            warnx("header overflows buffer");
            break;
        }
    }

}

void
lockerGet(io_connect_t connection, char *tag, char *file)
{
    UInt32      tagVal;
    UInt8       tagStr[4];
    UInt8       buf[1024];
    UInt64      input, output;
    uint32_t    output_count;
    UInt32      output_size;
    int         fd;

    // get the tag
    memset(tagStr, 0, 4);
    strncpy((char *)tagStr, tag, 4);
    tagVal = ((UInt32)tagStr[0] << 24) + ((UInt32)tagStr[1] << 16) + ((UInt32)tagStr[2] << 8) + (UInt32)tagStr[3];

    // get the locker
    input = tagVal;
    output_count = 1;
    output_size = sizeof(buf);
    if (KERN_SUCCESS != IOConnectCallMethod(connection,                             // connection
                                            kAppleEffaceableStorageMethodGetLocker, // selector
                                            &input,                                 // input
                                            1,                                      // input count
                                            NULL,                                   // input struct
                                            0,                                      // input struct size
                                            &output,                                // output
                                            &output_count,                          // output count
                                            &buf,                                   // output struct
                                            &output_size)) {                        // output struct size
        errx(1, "getLocker call failed");
    }

    // write the file
    if ((fd = open(file, O_CREAT | O_TRUNC | O_WRONLY, 0666)) < 0)
        err(1, "failed to create %s", file);
    warnx("using descriptor %d", fd);
    if (write(fd, buf, output_size) != output_size)
        err(1, "failed writing %lu bytes to %s (fd %d)", output_size, file, fd);
    close(fd);

    exit(0);
}

void
lockerSet(io_connect_t connection, char *tag, char *file)
{
    UInt32      tagVal;
    UInt8       tagStr[4];
    UInt8       buf[1024];
    UInt64      input;
    int         fd, size;

    // get the tag
    memset(tagStr, 0, 4);
    strncpy((char *)tagStr, tag, 4);
    tagVal = ((UInt32)tagStr[0] << 24) + ((UInt32)tagStr[1] << 16) + ((UInt32)tagStr[2] << 8) + (UInt32)tagStr[3];

    // read the file
    if ((fd = open(file, O_RDONLY)) < 0)
        err(1, "failed to open %s", file);
    if ((size = read(fd, buf, sizeof(buf))) < 0)
        err(1, "failed to read from %s", file);
    close(fd);


    // make the call
    input = tagVal;
    if (KERN_SUCCESS != IOConnectCallMethod(connection,
                                            kAppleEffaceableStorageMethodSetLocker,
                                            &input,
                                            1,
                                            &buf,
                                            size,
                                            NULL,
                                            0,
                                            NULL,
                                            NULL)) {
        errx(1, "setLocker call failed");
    }

    exit(0);
}

void
lockerEfface(io_connect_t connection, char *tag)
{
    UInt32      tagVal;
    UInt8       tagStr[4];
    UInt64      input;

    // get the tag
    memset(tagStr, 0, 4);
    strncpy((char *)tagStr, tag, 4);
    tagVal = ((UInt32)tagStr[0] << 24) + ((UInt32)tagStr[1] << 16) + ((UInt32)tagStr[2] << 8) + (UInt32)tagStr[3];

    // make the call
    input = tagVal;
    if (KERN_SUCCESS != IOConnectCallMethod(connection,
                                            kAppleEffaceableStorageMethodEffaceLocker,
                                            &input,
                                            1,
                                            NULL,
                                            0,
                                            NULL,
                                            0,
                                            NULL,
                                            NULL)) {
        errx(1, "effaceLocker call failed");
    }
    exit(0);
}

void
generateNonce(io_connect_t connection)
{

    UInt8   buf[SHA1_HASH_SIZE];
    UInt32  output_size = sizeof(buf);
    UInt32  idx;

    memset(buf, 0, output_size);
    if (KERN_SUCCESS != IOConnectCallMethod(connection,                                 // connection
                                            kAppleEffaceableStorageMethodGenerateNonce, // selector
                                            NULL,                                       // input
                                            0,                                          // input count
                                            NULL,                                       // input struct
                                            0,                                          // input struct size
                                            NULL,                                       // output
                                            NULL,                                       // output count
                                            &buf,                                       // output struct
                                            &output_size)) {                            // output struct size
        errx(1, "generateNonce call failed");
    }

    // report the nonce SHA1 hash
    for (idx = 0; idx < output_size; idx++) {
        printf("%s%02X", (idx ? " " : ""), buf[idx]);
    }
    printf("\n");

    exit(0);
}
