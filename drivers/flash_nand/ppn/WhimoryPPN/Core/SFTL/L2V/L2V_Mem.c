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

#include "L2V.h"
#include "L2V_Mem.h"
#include "L2V_Funcs.h"
#include "L2V_Assert.h"


void _L2V_FreeNode(lNode_t *node)
{
    static int freeCount = 0;

    freeCount++;

    l2v_assert_ge(node, &L2V.Pool.Node[0]);
    l2v_assert_le(node, &L2V.Pool.Node[L2V_NODEPOOL_COUNT]);

    node->nextFreePtr = L2V.Pool.FreePtr;
    L2V.Pool.FreePtr = node;
    L2V.Pool.FreeCount++;
}


lNode_t *_L2V_AllocNode()
{
    lNode_t *ret;
    static int allocCount = 0;

    allocCount++;

    ret = (lNode_t*)L2V.Pool.FreePtr;
    if (NULL != ret) {
        L2V.Pool.FreePtr = L2V.Pool.FreePtr->nextFreePtr;
        L2V.Pool.FreeCount--;
        _L2V_NODE_FILL(ret, 0, L2V_NODE_SIZE);
    }
    l2v_assert_ne(ret, NULL);

    return ret;
}

