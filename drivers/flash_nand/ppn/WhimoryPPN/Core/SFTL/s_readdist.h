/*
 * Copyright (c) 2008-2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

// Author:  Daniel J. Post (djp), dpost@apple.com

#ifndef __S_READDIST_H__
#define __S_READDIST_H__

#include "s_internal.h"

#define s_readDist_add(vba, amount) \
do { \
    UInt32 ____temp1; \
    ____temp1 = s_g_vba_to_sb(vba); \
    sftl.sb[____temp1].reads += s_g_div_vbas_per_stripe((amount) + s_g_vbas_per_stripe - 1); \
    if (sftl.sb[____temp1].reads >= S_READDIST_LIMIT_PER_SB) { \
        s_readCount_enq(vba); \
    } \
} while(0)

extern void s_readDist_init(void);
extern void s_readDist_close(void);
extern void s_readRefresh_enq(UInt32 vba);
extern void s_readCount_enq(UInt32 vba);
extern void s_readDist_end_of_read_handle(void);

#endif // __S_READDIST_H__

