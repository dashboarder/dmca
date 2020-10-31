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
#include "L2V_Forget.h"
#include "L2V_Funcs.h"
#include "L2V_Mem.h"
#include "L2V_Assert.h"

static UInt32 ChooseBiggest(void);
static void Forget(lNode_t *node);
static void ReplaceRoot(UInt32 tree);

void L2V_ForgetBiggestTree()
{
    UInt32 tree;

    tree = ChooseBiggest();

    // Trying to repack a rootptr?  Abort...
    if (((UInt32)~0 == tree) || (!LPTR_IS_NODEPTR(L2V.Tree[tree]))) {
        return;
    }

    Forget(L2V_IDX_TO_NODE(LPTR_GET_NODEIDX(L2V.Tree[tree])));

    ReplaceRoot(tree);
}

static UInt32 ChooseBiggest()
{
    UInt32 i, biggest, biggestSize;

    biggest = ~0;
    biggestSize = 0;

    for (i = 0; i < L2V.numRoots; i++) {
        if (L2V.TreeNodes[i] > biggestSize) {
            biggest = i;
            biggestSize = L2V.TreeNodes[i];
        }
    }

    return biggest;
}

static void Forget(lNode_t *node)
{
    UInt32 nOfs, nodeSize; // Byte offset within the node and size
    lPtr_t thisPtr;
    ContigUnpacked_t cu;

    // Iterate through the node,
    _L2V_NODE_ITERATE(node, nOfs, nodeSize, cu, thisPtr) {
        if (cu.isNodePtr) {
            // Recurse
            Forget(L2V_IDX_TO_NODE(cu.u.nodeIdx));
        }
    } _L2V_NODE_ITERATE_END(nOfs);

    // Free the node
    _L2V_FreeNode(node);
}

static void ReplaceRoot(UInt32 tree)
{
    ContigUnpacked_t cu;

    // Set up contig
    cu.isNodePtr = 0;
    cu.hasSpanOF = 0;
    cu.u.n.vpn = L2V_VPN_MISS;
    cu.span = 0; // Just so it's consistent... value doesn't matter in root

    // Update tree
    L2V.Tree[tree] = LPTR_PACK_NAND(cu);
    L2V.TreeNodes[tree] = 0;
    L2V_root_depth[tree] = 0;
}

#endif // ENABLE_L2V_TREE

