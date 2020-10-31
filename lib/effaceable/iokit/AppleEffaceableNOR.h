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

#ifndef _APPLE_EFFACEABLE_NOR_H
#define _APPLE_EFFACEABLE_NOR_H

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>
#include <IOKit/platform/AppleARMNORFlashDevice.h>
#include "AppleEffaceableStorage.h"

#include "effaceable_contract.h"

class AppleEffaceableNOR : public AppleEffaceableStorage
{
    OSDeclareAbstractStructors(AppleEffaceableNOR);

public:
    virtual bool start(IOService * provider);

    friend uint32_t getRegionCountHook(effaceable_nor_hal_t * nor);
    friend uint32_t getRegionSizeHook(effaceable_nor_hal_t * nor, uint32_t index);
    friend EffaceableReturn eraseRegionHook(effaceable_nor_hal_t * nor, uint32_t index, uint32_t length);
    friend uint32_t writeRegionHook(effaceable_nor_hal_t * nor, uint32_t index, const void * buf, uint32_t length);
    friend bool readRegionHook(effaceable_nor_hal_t * nor, uint32_t index, void * buf, uint32_t length);

protected:
    effaceable_nor_hal_t * nor(void);

    virtual void setupDeviceContract(void);

private:
    void setupNorContract(void);

    uint32_t getRegionCountNor(void);
    uint32_t getRegionSizeNor(uint32_t index);
    EffaceableReturn eraseRegionNor(uint32_t index, uint32_t length);
    uint32_t writeRegionNor(uint32_t index, const void * buf, uint32_t length);
    bool readRegionNor(uint32_t index, void * buf, uint32_t length);

    AppleARMNORFlashDevice *  _nor_flash;
    bool _is_started;
    effaceable_nor_hal_t _nor_hal;
};

#endif /* _APPLE_EFFACEABLE_NOR_H */
