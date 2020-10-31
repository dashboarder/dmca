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

#ifndef _APPLE_EFFACEABLE_STORAGE_KEYS_H
#define _APPLE_EFFACEABLE_STORAGE_KEYS_H

__BEGIN_DECLS

// =============================================================================

// callPlatformFunction methods
//
// GetLocker
//      Returns the contents of the locker type_id, if it exists.
//
// SetLocker
//      Updates or creates the locker type_id.
//
// Space
//      Returns the largest amount of data that could be stored in a locker
//      with type_id, accounting for overwriting any current value.
//
// Efface
//      Destroys the locker type_id and all its contents.
//
// GenerateNonce
//      Populate private nonce locker with random 64-bit value and return its hash.
//
// type_id
//      Unique identifier for the locker.  Typically a four-byte ASCII value.
//
// data
//      Pointer to read/write data buffer.
//
// data_size
//      Buffer/operation size in bytes.  Note that for GetLocker this is a
//      pointer to an IOByteCount that is updated to the actual size of the
//      locker, and for Space this is the amount of space available for
//      a locker with type_id.
//
// untrusted
//      True if the client is untrusted and should not be allowed to operate
//      on protected lockers.
//
// hash
//      Pointer to 20 byte buffer into which the nonce hash will be copied.
//
#define kAppleEffaceableStorageFunctionGet           "GetLocker"     // (type_id, *data, *data_size, untrusted)
#define kAppleEffaceableStorageFunctionSet           "SetLocker"     // (type_id, *data, data_size, untrusted)
#define kAppleEffaceableStorageFunctionEfface        "EffaceLocker"  // (type_id, untrusted)
#define kAppleEffaceableStorageFunctionGenerateNonce "GenerateNonce" // (*hash)
#define kAppleEffaceableStorageFunctionSpace         "Space"         // (type_id, *free_size)

// When set on a request's type_id, the locker can only be read, overwritten and
// effaced by a kernel client.  Cannot be set by a user client.
#define kAppleEffaceableStorageLockerProtected  (1<<31)
// Special type for manipulating nonce locker, which is permitted
// only by the effaceable storage driver
#define kAppleEffaceableStorageLockerPrivate    (1<<23)

// These bits must not be set in a locker's type_id.
#define kAppleEffaceableStorageLockerReserved   ((1<<15) | (1<<7))

// This type_id is considered a wildcard; passed to Efface it will cause
// every locker to be effaced.
#define kAppleEffaceableStorageLockerWildcard   '****'

// =============================================================================

#define kAppleEffaceableStorageClassName              "AppleEffaceableStorage"

// DeviceTree properties

#define kAppleEffaceableStoragePropertyVersionMajor   "VersionMajor"
#define kAppleEffaceableStoragePropertyVersionMinor   "VersionMinor"
#define kAppleEffaceableStoragePropertyIsFormatted    "IsFormatted"

// UserClient methods

enum _AppleEffaceableStorageMethod
{
    // Maintenance interface.
    kAppleEffaceableStorageMethodGetCapacity,
    kAppleEffaceableStorageMethodGetBytes,
    kAppleEffaceableStorageMethodSetBytes,
    kAppleEffaceableStorageMethodIsFormatted,
    kAppleEffaceableStorageMethodFormatStorage,

    // Userclient locker methods.
    //
    // The type_id argument is always passed in scalarInput[0].
    //
    // GetLocker returns the actual size of the locker in scalarOutput[0],
    // and if structureOutput/structureOutputSize describe a buffer, as much
    // of the locker data as will fit is returned into the buffer.
    //
    // SetLocker creates or replaces an existing locker with the contents of
    // structureInput/structureInputSize.
    //
    // EffaceLocker destroys the locker.
    //
    kAppleEffaceableStorageMethodGetLocker,
    kAppleEffaceableStorageMethodSetLocker,
    kAppleEffaceableStorageMethodEffaceLocker,
    kAppleEffaceableStorageMethodSpace,

    // Debug-only interface.
    kAppleEffaceableStorageMethodWipeStorage,

    // Nonce support
    kAppleEffaceableStorageMethodGenerateNonce,

    kAppleEffaceableStorageMethodCount
};

typedef enum _AppleEffaceableStorageMethod AppleEffaceableStorageMethod;

// =============================================================================

__END_DECLS

#endif /* _APPLE_EFFACEABLE_STORAGE_KEYS_H */
