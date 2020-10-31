/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include "WMRFeatures.h"
#if ENABLE_L2V_TREE
#include "L2V.h"


#if L2V_TRACK_VALID!=0

void _L2V_ValidUp(UInt32 vpn, UInt32 count)
{
    UInt32 sb = vpn / L2V.pagesPerSB;
    L2V.Valid[sb] += count;
    //l2v_assert_le(L2V.Valid[sb], sbsize);
}

void _L2V_ValidDown(UInt32 vpn, UInt32 count)
{
    UInt32 sb = vpn / L2V.pagesPerSB;
    //l2v_assert_ge(L2V.Valid[sb], count);
    L2V.Valid[sb] -= count;
}

#endif // track valid

#endif // ENABLE_L2V_TREE

