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

// Author:  Daniel J. Post (djp), dpost@apple.com

#include "WMROAM.h"


void *WMR_MALLOC(UInt32 size)
{
    void * ptr = malloc(size);
    if (ptr)
    {
        WMR_MEMSET(ptr, 0, size);
    }
    return ptr;
}

