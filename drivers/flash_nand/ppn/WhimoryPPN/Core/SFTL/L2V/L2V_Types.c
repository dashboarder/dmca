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

#include "L2V_Defines.h"
#include "L2V_Types.h"
#include "L2V.h"


BOOL32 _L2V_NodeBig_PushContig(lNodeBig_t *big, ContigUnpacked_t *cu)
{
    Int32 last = ((Int32)big->max)-1;
    l2v_assert_ne(cu->span, 0);
    
    if (!cu->isNodePtr && (last >= 0) && !big->cu[last].isNodePtr && (
        ((cu->u.vba < L2V_VBA_SPECIAL) && ((big->cu[last].u.vba+big->cu[last].span) == cu->u.vba))
        || ((cu->u.vba >= L2V_VBA_SPECIAL) && (big->cu[last].u.vba == cu->u.vba))))
    {
        big->cu[last].span += cu->span;
        return TRUE32;
    }

    big->cu[big->max] = *cu;
    big->max++;

    if (cu->isNodePtr) {
        l2v_assert_lt(cu->u.nodeIdx, L2V_NODEPOOL_COUNT);
    }

    return FALSE32;
}


BOOL32 _L2V_NodeBig_PushContig_Node(lNodeBig_t *big, UInt32 nodeIdx, UInt32 span)
{
    big->cu[big->max].isNodePtr = 1;
    big->cu[big->max].u.nodeIdx = nodeIdx;
    big->cu[big->max].span = span;
    big->max++;

    return FALSE32;
}


BOOL32 _L2V_NodeBig_PushContig_Nand(lNodeBig_t *big, UInt32 vba, UInt32 span)
{
    Int32 last = ((Int32)big->max)-1;
    l2v_assert_ne(span, 0);
    
    if ((last >= 0) && !big->cu[last].isNodePtr && (
        ((vba < L2V_VBA_SPECIAL) && ((big->cu[last].u.vba+big->cu[last].span) == vba))
        || ((vba >= L2V_VBA_SPECIAL) && (big->cu[last].u.vba == vba))))
    {
        big->cu[last].span += span;
        return TRUE32;
    }

    big->cu[big->max].isNodePtr = 0;
    big->cu[big->max].u.vba = vba;
    big->cu[big->max].span = span;
    big->max++;

    return FALSE32;
}

