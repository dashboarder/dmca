/*
 * Copyright (c) 2010-2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

// =============================================================================

#include <IOKit/IOTypes.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOUserClient.h>

#include "AppleEffaceableStorage.h"
#include "AppleEffaceableStorageKeys.h"
#include "AppleEffaceableStorageUserClient.h"

// =============================================================================

// XXX correct hard-coding of SHA1 buffer size
#define SHA1_HASH_SIZE 20

// =============================================================================

#define APPLE_EFFACEABLE_USER_CLIENT_DBGMSK_INIT  (1<<0)
#define APPLE_EFFACEABLE_USER_CLIENT_DBGMSK_ERR   (1<<1)
#define APPLE_EFFACEABLE_USER_CLIENT_DBGMSK_WARN  (1<<2)
#define APPLE_EFFACEABLE_USER_CLIENT_DBGMSK_INFO  (1<<3)
#define APPLE_EFFACEABLE_USER_CLIENT_DBGMSK_SPEW  (1<<4)

#define APPLE_EFFACEABLE_USER_CLIENT_DBGTAG_INIT  "INIT"
#define APPLE_EFFACEABLE_USER_CLIENT_DBGTAG_ERR   "ERROR"
#define APPLE_EFFACEABLE_USER_CLIENT_DBGTAG_WARN  "WARNING"
#define APPLE_EFFACEABLE_USER_CLIENT_DBGTAG_INFO  "INFO"
#define APPLE_EFFACEABLE_USER_CLIENT_DBGTAG_SPEW  "SPEW"

#define APPLE_EFFACEABLE_USER_CLIENT_DEBUG_ALL (APPLE_EFFACEABLE_USER_CLIENT_DBGMSK_INIT      | \
                                                APPLE_EFFACEABLE_USER_CLIENT_DBGMSK_ERR       | \
                                                APPLE_EFFACEABLE_USER_CLIENT_DBGMSK_WARN      | \
                                                APPLE_EFFACEABLE_USER_CLIENT_DBGMSK_INFO      | \
                                                APPLE_EFFACEABLE_USER_CLIENT_DBGMSK_SPEW)

#define APPLE_EFFACEABLE_USER_CLIENT_DEBUG_MOST (APPLE_EFFACEABLE_USER_CLIENT_DBGMSK_INIT      | \
                                                 APPLE_EFFACEABLE_USER_CLIENT_DBGMSK_ERR       | \
                                                 APPLE_EFFACEABLE_USER_CLIENT_DBGMSK_WARN      | \
                                                 APPLE_EFFACEABLE_USER_CLIENT_DBGMSK_INFO)

#define APPLE_EFFACEABLE_USER_CLIENT_DEBUG_DEFAULT (APPLE_EFFACEABLE_USER_CLIENT_DBGMSK_INIT     | \
                                                    APPLE_EFFACEABLE_USER_CLIENT_DBGMSK_ERR)

#ifdef DEBUG_BUILD
# define APPLE_EFFACEABLE_USER_CLIENT_DEBUG APPLE_EFFACEABLE_USER_CLIENT_DEBUG_MOST
#else
# define APPLE_EFFACEABLE_USER_CLIENT_DEBUG APPLE_EFFACEABLE_USER_CLIENT_DEBUG_DEFAULT
#endif

#if defined(APPLE_EFFACEABLE_USER_CLIENT_DEBUG) && APPLE_EFFACEABLE_USER_CLIENT_DEBUG
# define debug(fac, fmt, args...)                                       \
    do {                                                                \
        if (APPLE_EFFACEABLE_USER_CLIENT_DBGMSK_##fac &                 \
            APPLE_EFFACEABLE_USER_CLIENT_DEBUG) {                       \
            kprintf("AppleEffaceableStorageUserClient::%s():[%s] " fmt "\n", \
                    __FUNCTION__,                                       \
                    APPLE_EFFACEABLE_USER_CLIENT_DBGTAG_##fac ,         \
                    ##args );                                           \
        }                                                               \
    } while(0)
# define hexdump(fac, bytes, count)                                     \
    do {                                                                \
        if (APPLE_EFFACEABLE_USER_CLIENT_DBGMSK_##fac &                 \
            APPLE_EFFACEABLE_USER_CLIENT_DEBUG) {                       \
            uint32_t i, j;                                              \
            for (i = 0; i < count; i += 16) {                           \
                kprintf("AppleEffaceableStorageUserClient::%s():[%s] 0x%08x:", \
                        __FUNCTION__,                                   \
                        APPLE_EFFACEABLE_USER_CLIENT_DBGTAG_##fac ,     \
                        ((uint32_t) bytes) + i);                        \
                for (j = 0; (j < 16) && ((i + j) < count); j++)         \
                    kprintf(" %2.2x", ((uint8_t*)bytes)[i+j]);          \
                kprintf("\n");                                          \
            }                                                           \
        }                                                               \
    } while(0)
#else
# define debug(fac, fmt, args...)  do { /* nothing */ } while(0)
# define hexdump(fac, bytes, count)  do { /* nothing */ } while(0)
#endif

// =============================================================================

#ifdef super
#undef super
#endif
#define super IOUserClient

OSDefineMetaClassAndStructors(AppleEffaceableStorageUserClient, IOUserClient);

// =============================================================================

bool
AppleEffaceableStorageUserClient::start(IOService *provider)
{
    bool ok = false;

    _provider = OSDynamicCast(AppleEffaceableStorage, provider);

    if (NULL == _provider) {
        debug(ERR, "bug in kext matching");
    } else if (!super::start(provider)) {
        debug(ERR, "super failed to start");
    } else {
        debug(INFO, "started");
        ok = true;
    }

    return ok;
}

IOReturn
AppleEffaceableStorageUserClient::clientClose(void)
{
    terminate();
    return kIOReturnSuccess;
}

IOReturn
AppleEffaceableStorageUserClient::externalMethod(uint32_t selector,
                                                 IOExternalMethodArguments * arguments,
                                                 IOExternalMethodDispatch *dispatch,
                                                 OSObject *target,
                                                 void *reference)
{
    IOReturn ok = kIOReturnSuccess;
    bool    secure_root;

    // require administrator privilege
    if (kIOReturnSuccess != clientHasPrivilege(current_task(), kIOClientPrivilegeAdministrator)) {
        debug(ERR, "not authorized");
        return kIOReturnNotPrivileged;
    }

#ifdef DEBUG_BUILD
    secure_root = true;
#else
    // some operations require a secure root
    if (kIOReturnSuccess != (ok = _provider->getProvider()->callPlatformFunction("SecureRoot",
                                                                                 false,
                                                                                 _provider->getProvider(),
                                                                                 &secure_root,
                                                                                 0, 0))) {
        debug(ERR, "failed to determine root trust state");
        secure_root = false;
    }
#endif // DEBUG_BUILD

    switch (selector) {
    case kAppleEffaceableStorageMethodGetCapacity:

        if (arguments->scalarOutputCount != 1) {
            debug(ERR, "getCapacity: missing output argument");
            ok = kIOReturnBadArgument;
        } else {
            arguments->scalarOutput[0] = _provider->getCapacity();
        }
        break;

    case kAppleEffaceableStorageMethodIsFormatted:

        if (arguments->scalarOutputCount != 1) {
            debug(ERR, "isFormatted: missing output argument");
            ok = kIOReturnBadArgument;
        } else {
            arguments->scalarOutput[0] = _provider->isFormatted();
        }
        break;

    case kAppleEffaceableStorageMethodFormatStorage:

        if (!secure_root) {
            debug(ERR, "format attempt from untrusted root");
        } else {
            ok = _provider->formatStorage();
        }
        break;

    case kAppleEffaceableStorageMethodWipeStorage:

        if (!secure_root) {
            debug(ERR, "wipe attempt from untrusted root");
        } else {
            ok = _provider->wipeStorage();
        }
        break;

    case kAppleEffaceableStorageMethodGetLocker:

        if ((arguments->scalarInputCount != 1) ||
            (arguments->scalarOutputCount != 1)) {
            debug(ERR, "getLocker: missing input/output arguments");
            ok = kIOReturnBadArgument;
        } else {
            IOByteCount data_size = arguments->structureOutputSize;
            ok = _provider->getLocker(arguments->scalarInput[0],
                                      arguments->structureOutput,
                                      &data_size,
                                      !secure_root);
            arguments->scalarOutput[0] = data_size;
            // if we didn't clip the data, report the actual output size
            if (data_size < arguments->structureOutputSize)
                arguments->structureOutputSize = data_size;
        }
        break;

    case kAppleEffaceableStorageMethodSetLocker:

        if ((arguments->scalarInputCount != 1) ||
            (arguments->structureInputSize < 1)) {
            debug(ERR, "setLocker: missing input arguments");
            ok = kIOReturnBadArgument;
        } else {
            ok = _provider->setLocker(arguments->scalarInput[0],
                                      arguments->structureInput,
                                      arguments->structureInputSize,
                                      !secure_root);
        }
        break;

    case kAppleEffaceableStorageMethodSpace:

        if ((arguments->scalarInputCount != 1) ||
            (arguments->scalarOutputCount != 1)) {
            debug(ERR, "lockerSpace: missing input/output arguments");
            ok = kIOReturnBadArgument;
        } else {
            IOByteCount data_space;
            ok = _provider->spaceForLocker(arguments->scalarInput[0],
                                           &data_space);
            arguments->scalarOutput[0] = data_space;
        }
        break;

    case kAppleEffaceableStorageMethodEffaceLocker:

        if (arguments->scalarInputCount != 1) {
            debug(ERR, "effaceLocker: missing input argument");
            ok = kIOReturnBadArgument;
        } else if (!secure_root && arguments->scalarInput[0] == kAppleEffaceableStorageLockerWildcard) {
            debug(ERR, "effaceLocker: untrusted attempt to efface by wildcard");
            ok = kIOReturnNotPermitted;
        } else {
            ok = _provider->effaceLocker(arguments->scalarInput[0], !secure_root);
        }
        break;

    case kAppleEffaceableStorageMethodGenerateNonce:

        if (arguments->structureOutputSize != SHA1_HASH_SIZE) {
            debug(ERR, "generateNonce: missing output argument");
            ok = kIOReturnBadArgument;
        } else {
            ok = _provider->generateNonce(arguments->structureOutput);
            if (kIOReturnSuccess != ok) {
                arguments->structureOutputSize = 0;
            }
        }
        break;

//
// Methods below are not for production use and may only be used when kernel debug enabled via boot-args.
//

    case kAppleEffaceableStorageMethodGetBytes:

        if ((arguments->scalarInputCount != 1) ||
            (arguments->structureOutputSize < 1)) {
            ok = kIOReturnBadArgument;
        } else if (!PE_i_can_has_debugger(NULL)) {
            debug(ERR, "getBytes is only allowed when kernel debug is enabled");
            ok = kIOReturnUnsupported;
        } else {
            ok = _provider->getBytes(arguments->structureOutput, arguments->scalarInput[0], arguments->structureOutputSize);
            if (kIOReturnSuccess != ok) {
                debug(ERR, "getBytes from user client to service failed");
            } else {
                debug(SPEW, "read %u bytes from offset %llu", arguments->structureOutputSize, arguments->scalarInput[0]);
            }
        }
        break;

    default:
        debug(ERR, "bad selector %u", selector);
        ok = kIOReturnUnsupported;
        break;
    }

    return ok;
}
