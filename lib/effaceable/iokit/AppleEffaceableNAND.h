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

#ifndef _APPLE_EFFACEABLE_NAND_H
#define _APPLE_EFFACEABLE_NAND_H

#include <IOKit/IOService.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/storage/flash/IOFlashStoragePartition.h>
#include "AppleEffaceableStorage.h"

#include "effaceable_contract.h"

class AppleEffaceableNAND : public AppleEffaceableStorage
{
    OSDeclareDefaultStructors(AppleEffaceableNAND);

public:
    virtual bool start(IOService * provider);

    friend uint32_t getPageSizeHook(effaceable_nand_hal_t * nand);
    friend uint32_t getPageCountHook(effaceable_nand_hal_t * nand);
    friend uint32_t getBlockCountHook(effaceable_nand_hal_t * nand);
    friend uint32_t getBankCountHook(effaceable_nand_hal_t * nand);
    friend bool isBlockBadHook(effaceable_nand_hal_t * nand, uint32_t bank, uint32_t block);
    friend EffaceableReturn eraseBlockHook(effaceable_nand_hal_t * nand, uint32_t bank, uint32_t block);
    friend EffaceableReturn writePageHook(effaceable_nand_hal_t * nand, uint32_t bank, uint32_t block, uint32_t page, const void * buf, uint32_t length);
    friend EffaceableReturn readPageHook(effaceable_nand_hal_t * nand, uint32_t bank, uint32_t block, uint32_t page, void * buf, uint32_t length);
    friend bool requestPartitionTableDiffHook(effaceable_nand_hal_t * nand, void * buf, uint32_t length);
    friend bool providePartitionTableDiffHook(effaceable_nand_hal_t * nand, void * buf, uint32_t length);

protected:
    effaceable_nand_hal_t * nand(void);

    virtual void setupDeviceContract(void);

private:
    void setupNandContract(void);

    uint32_t getPageSizeNand(void);
    uint32_t getPageCountNand(void);
    uint32_t getBlockCountNand(void);
    uint32_t getBankCountNand(void);
    bool isBlockBadNand(uint32_t bank, uint32_t block);
    EffaceableReturn eraseBlockNand(uint32_t bank, uint32_t block);
    EffaceableReturn writePageNand(uint32_t bank, uint32_t block, uint32_t page, const void * buf, uint32_t length);
    EffaceableReturn readPageNand(uint32_t bank, uint32_t block, uint32_t page, void * buf, uint32_t length);
    bool requestPartitionTableDiffNand(void * buf, uint32_t length);
    bool providePartitionTableDiffNand(void * buf, uint32_t length);

    IOFlashStoragePartition * _partition;
    effaceable_nand_hal_t _nand_hal;
};

#endif /* _APPLE_NAND_EFFACEABLE_STORAGE_H */
