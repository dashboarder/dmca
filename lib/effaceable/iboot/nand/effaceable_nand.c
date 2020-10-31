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

// =============================================================================

#include <lib/effaceable.h>
#include <lib/effaceable_impl.h>

#include <effaceable_contract.h>
#include <effaceable_debug.h>

#include <nand_part.h>
#include <nand_part_interface.h>

#include <lib/heap.h>

// =============================================================================

struct _context
{
    NandPartInterface *  npi;
    uint32_t             idx;
};

typedef struct _context context_t;

// =============================================================================

static inline context_t * context(effaceable_nand_hal_t * nand)
{
    return (context_t*)nand->opaque;
}

// =============================================================================

#define delegateNPI(_me, _fn, ...)  context(_me)->npi->_fn(context(_me)->npi->core, context(_me)->idx, ## __VA_ARGS__)

// =============================================================================

static void initContext(effaceable_nand_hal_t * nand, NandPartInterface * npi, uint32_t idx)
{
    context_t * ctx = (context_t*)malloc(sizeof(context_t));
    
    ctx->npi = npi;
    ctx->idx = idx;

    nand->opaque = ctx;
}

// -----------------------------------------------------------------------------

static uint32_t getPageSizeHook(effaceable_nand_hal_t * nand)
{
    return delegateNPI(nand, get_bytes_per_page);
}

static uint32_t getPageCountHook(effaceable_nand_hal_t * nand)
{
    return delegateNPI(nand, get_pages_per_block);
}

static uint32_t getBlockCountHook(effaceable_nand_hal_t * nand)
{
    return delegateNPI(nand, get_block_depth);
}

static uint32_t getBankCountHook(effaceable_nand_hal_t * nand)
{
    return delegateNPI(nand, get_bank_width);
}

static bool isBlockBadHook(effaceable_nand_hal_t * nand, uint32_t bank, uint32_t block)
{
    return delegateNPI(nand, is_block_bad, bank, block);
}

static EffaceableReturn eraseBlockHook(effaceable_nand_hal_t * nand, uint32_t bank, uint32_t block)
{
    return (delegateNPI(nand, erase_block, bank, block) ? kEffaceableReturnSuccess : kEffaceableReturnError);
}

static EffaceableReturn writePageHook(effaceable_nand_hal_t * nand, uint32_t bank, uint32_t block, uint32_t page, const void * buf, uint32_t length)
{
    return (delegateNPI(nand, write_page, bank, block, page, (void*)buf) ? kEffaceableReturnSuccess : kEffaceableReturnError);
}

static EffaceableReturn readPageHook(effaceable_nand_hal_t * nand, uint32_t bank, uint32_t block, uint32_t page, void * buf, uint32_t length)
{
    return (delegateNPI(nand, read_page, bank, block, page, buf, (bool*)0) ? kEffaceableReturnSuccess : kEffaceableReturnError);
}

static bool requestPartitionTableDiffHook(effaceable_nand_hal_t * nand, void * buf, uint32_t length)
{
    return delegateNPI(nand, request_ptab_diff, buf, length);
}

static bool providePartitionTableDiffHook(effaceable_nand_hal_t * nand, void * buf, uint32_t length)
{
    return delegateNPI(nand, provide_ptab_diff, buf, length);
}

// -----------------------------------------------------------------------------

static void initContract(effaceable_nand_hal_t * nand)
{
    nand->getPageSize               = getPageSizeHook;
    nand->getPageCount              = getPageCountHook;
    nand->getBlockCount             = getBlockCountHook;
    nand->getBankCount              = getBankCountHook;
    nand->isBlockBad                = isBlockBadHook;
    nand->eraseBlock                = eraseBlockHook;
    nand->writePage                 = writePageHook;
    nand->readPage                  = readPageHook;
    nand->requestPartitionTableDiff = requestPartitionTableDiffHook;
    nand->providePartitionTableDiff = providePartitionTableDiffHook;
}

// =============================================================================

extern void
effaceable_nand_init(NandPartInterface * npi, uint32_t idx)
{
    effaceable_nand_hal_t * nand = (effaceable_nand_hal_t *)malloc(sizeof(effaceable_nand_hal_t));
    effaceable_device_t * device = (effaceable_device_t *)malloc(sizeof(effaceable_device_t));
    effaceable_system_t * system = (effaceable_system_t *)malloc(sizeof(effaceable_system_t));
    effaceable_storage_t * storage = (effaceable_storage_t *)malloc(sizeof(effaceable_storage_t));

    initContext(nand, npi, idx);
    initContract(nand);

    if (kEffaceableReturnSuccess == setupEffaceableSystemContract(system)) {

        startEffaceableNAND(device, system, nand, storage);
        registerEffaceableStorage(storage);
    }
}

// =============================================================================
