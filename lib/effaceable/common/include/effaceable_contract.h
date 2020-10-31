/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef _EFFACEABLE_CONTRACT_H_
#define _EFFACEABLE_CONTRACT_H_

// =============================================================================

#include <sys/types.h>
#include <stdarg.h>

/* horrible that this is necessary */
#ifndef __cplusplus
# ifndef false
#  include <stdbool.h>
# endif
#endif

// =============================================================================

enum _EffaceableReturn
{
    kEffaceableReturnSuccess = 0,
    kEffaceableReturnError,

    kEffaceableReturnNotFound,
    kEffaceableReturnUnsupported,
    kEffaceableReturnNotPrivileged,
    kEffaceableReturnNotPermitted,
    kEffaceableReturnBadArgument,
    kEffaceableReturnBadMedia,
    kEffaceableReturnUnformattedMedia,
    kEffaceableReturnIOError,
    kEffaceableReturnNoMemory,
    kEffaceableReturnNoSpace,
    kEffaceableReturnInternalError,

    kEffaceableReturnInvalid
};

typedef enum _EffaceableReturn EffaceableReturn;

// =============================================================================

struct _effaceable_storage;
struct _effaceable_device;
struct _effaceable_system;
struct _effaceable_nor_hal;
struct _effaceable_nand_hal;

// -----------------------------------------------------------------------------

typedef struct _effaceable_storage   effaceable_storage_t;
typedef struct _effaceable_device    effaceable_device_t;

typedef struct _effaceable_system    effaceable_system_t;
typedef struct _effaceable_nor_hal   effaceable_nor_hal_t;
typedef struct _effaceable_nand_hal  effaceable_nand_hal_t;

// =============================================================================

struct _effaceable_storage
{
    void * opaque;

    /* client api for use externally */
    EffaceableReturn  (* getLocker        )(effaceable_storage_t * storage, uint32_t type_id, void *data, uint32_t *data_size, bool untrusted);
    EffaceableReturn  (* setLocker        )(effaceable_storage_t * storage, uint32_t type_id, const void *data, uint32_t data_size, bool untrusted);
    EffaceableReturn  (* spaceForLocker   )(effaceable_storage_t * storage, uint32_t type_id, uint32_t *data_space);
    EffaceableReturn  (* effaceLocker     )(effaceable_storage_t * storage, uint32_t type_id, bool untrusted);
    EffaceableReturn  (* generateNonce    )(effaceable_storage_t * storage, void * hash);
    EffaceableReturn  (* consumeNonce     )(effaceable_storage_t * storage, uint64_t * nonce);

    /* management api for use externally */
    EffaceableReturn  (* getBytes         )(effaceable_storage_t * storage, void * client_buf, uint32_t offset, uint32_t count);
    EffaceableReturn  (* wipeStorage      )(effaceable_storage_t * storage, void * tmp_buf);
    uint32_t          (* getCapacity      )(effaceable_storage_t * storage);
    bool              (* isFormatted      )(effaceable_storage_t * storage);
    EffaceableReturn  (* formatStorage    )(effaceable_storage_t * storage);

    /* spi for use by device-specific controller */
    uint32_t          (* getCopiesPerUnit )(effaceable_storage_t * storage);
    uint32_t          (* getCopySize      )(effaceable_storage_t * storage);
};

// -----------------------------------------------------------------------------

struct _effaceable_device
{
    void * opaque;

    /* device controller must provide concrete implementation of these operations */
    uint32_t           (* getGroupCount            )(effaceable_device_t * device);
    uint32_t           (* getUnitsPerGroup         )(effaceable_device_t * device);
    uint32_t           (* getUnitSize              )(effaceable_device_t * device);
    EffaceableReturn   (* eraseGroup               )(effaceable_device_t * device, uint32_t group);
    EffaceableReturn   (* writeUnit                )(effaceable_device_t * device, const void * buf, uint32_t group, uint32_t unit);
    EffaceableReturn   (* readUnit                 )(effaceable_device_t * device, void * buf, uint32_t group, uint32_t unit);

    /* device controller may optionally extend these device; if NULL, code should not call */
    void               (* cleanBuffers             )(effaceable_device_t * device);
    void               (* foundCurrentCloneInGroup )(effaceable_device_t * device, uint32_t group);
    void               (* updateDynamicProperties  )(effaceable_device_t * device);
};

// -----------------------------------------------------------------------------

struct _effaceable_system
{
    void * opaque;

    /* operating system specific operations must be supplied when starting device */
    void *           (* allocMem    )(effaceable_system_t * system, uint32_t size);
    void             (* freeMem     )(effaceable_system_t * system, void * buf, uint32_t size);
    void *           (* setMem      )(effaceable_system_t * system, void * buf, uint8_t val, uint32_t size);
    void *           (* moveMem     )(effaceable_system_t * system, void * dst, const void * src, uint32_t size);
    int              (* cmpMem      )(effaceable_system_t * system, const void * lhs, const void * rhs, uint32_t size);
    void             (* readRandom  )(effaceable_system_t * system, void * buf, uint32_t size);
    void             (* calcSHA1    )(effaceable_system_t * system, const void * buf, uint32_t size, void * hash);
    uint32_t         (* crc32       )(effaceable_system_t * system, uint32_t crc, const void * buf, uint32_t size, bool finish);
    bool             (* setProperty )(effaceable_system_t * system, const char * key, uint32_t value);
    void             (* panicSys    )(effaceable_system_t * system, const char * msg);
    int              (* vlogf       )(effaceable_system_t * system, const char * fmt, va_list ap);
};

// -----------------------------------------------------------------------------

/* contract for NOR HAL operations */
struct _effaceable_nor_hal
{
    void * opaque;

    /* NOR HAL operations must be supplied when starting device */
    uint32_t           (* getRegionCount )(effaceable_nor_hal_t * nor);
    uint32_t           (* getRegionSize  )(effaceable_nor_hal_t * nor, uint32_t index);
    EffaceableReturn   (* eraseRegion    )(effaceable_nor_hal_t * nor, uint32_t index, uint32_t length);
    uint32_t           (* writeRegion    )(effaceable_nor_hal_t * nor, uint32_t index, const void * buf, uint32_t length);
    bool               (* readRegion     )(effaceable_nor_hal_t * nor, uint32_t index, void * buf, uint32_t length);
};

// -----------------------------------------------------------------------------

/* contract for NAND HAL operations */
struct _effaceable_nand_hal
{
    void * opaque;

    /* NAND HAL operations must be supplied when starting device */
    uint32_t           (* getPageSize               )(effaceable_nand_hal_t * nand);
    uint32_t           (* getPageCount              )(effaceable_nand_hal_t * nand);
    uint32_t           (* getBlockCount             )(effaceable_nand_hal_t * nand);
    uint32_t           (* getBankCount              )(effaceable_nand_hal_t * nand);
    bool               (* isBlockBad                )(effaceable_nand_hal_t * nand, uint32_t bank, uint32_t block);
    EffaceableReturn   (* eraseBlock                )(effaceable_nand_hal_t * nand, uint32_t bank, uint32_t block);
    EffaceableReturn   (* writePage                 )(effaceable_nand_hal_t * nand, uint32_t bank, uint32_t block, uint32_t page, const void * buf, uint32_t length);
    EffaceableReturn   (* readPage                  )(effaceable_nand_hal_t * nand, uint32_t bank, uint32_t block, uint32_t page, void * buf, uint32_t length);
    bool               (* requestPartitionTableDiff )(effaceable_nand_hal_t * nand, void * buf, uint32_t length);
    bool               (* providePartitionTableDiff )(effaceable_nand_hal_t * nand, void * buf, uint32_t length);
};

// =============================================================================

__BEGIN_DECLS

// -----------------------------------------------------------------------------

extern void startEffaceableNOR(effaceable_device_t * device, effaceable_system_t * system, effaceable_nor_hal_t * nor, effaceable_storage_t * storage);
extern void startEffaceableNAND(effaceable_device_t * device, effaceable_system_t * system, effaceable_nand_hal_t * nor, effaceable_storage_t * storage);

// -----------------------------------------------------------------------------

__END_DECLS

// =============================================================================

#endif /* _EFFACEABLE_CONTRACT_H_ */
