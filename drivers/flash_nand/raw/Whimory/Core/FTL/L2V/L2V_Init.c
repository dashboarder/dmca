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
#include "L2V_Mem.h"


// Declare our L2V context structure here
L2V_t L2V;
UInt8 *L2V_root_depth;

// Functions
BOOL32 L2V_Init(UInt32 numLbas, UInt32 numSBs, UInt32 pagesPerSB)
{
    UInt32 i;
    Int32 j;
    ContigUnpacked_t cu;

    // Set number of superblocks for valid accounting division
    L2V.numSBs = numSBs;
    L2V.pagesPerSB = pagesPerSB;
    if ((numSBs * pagesPerSB) >= L2V_VPN_SPECIAL) {
        WMR_PANIC("Tree bitspace not sufficient for geometry %dx%d: please re-adjust", numSBs, pagesPerSB);
    }

    // Set tree version
    L2V.treeVersion = 0;

    // Calculate roots
    L2V.numRoots = (numLbas >> L2V_TREE_BITS) + 1;

    // Allocate arrays
    L2V.Tree = WMR_MALLOC(L2V.numRoots * sizeof(lPtr_t));
    l2v_assert_ne(L2V.Tree, NULL);
    L2V.TreeNodes = WMR_MALLOC(L2V.numRoots * sizeof(UInt32));
    l2v_assert_ne(L2V.TreeNodes, NULL);
    L2V.UpdatesSinceRepack = WMR_MALLOC(L2V.numRoots * sizeof(UInt32));
    l2v_assert_ne(L2V.UpdatesSinceRepack, NULL);
    L2V_root_depth = WMR_MALLOC(L2V.numRoots);
    l2v_assert_ne(L2V_root_depth , NULL);
    for (i = 0; i < L2V.numRoots; i++) {
        L2V_root_depth[i] = 0;
    }

    // Empty roots; default to miss
    cu.hasSpanOF = 0;
    cu.u.n.vpn = L2V_VPN_MISS;
    cu.span = 0;
    for (i = 0; i < L2V.numRoots; i++) {
        L2V.Tree[i] = LPTR_PACK_NAND(cu);
    }

    // Reset periodic repacker variables
    L2V.RepackCounter = 0;

    // Allocate the node pool
    L2V.Pool.Node = (lNode_t*)WMR_MALLOC(L2V_NODEPOOL_MEM);
    if (NULL == L2V.Pool.Node) {
        return FALSE32;
    }

    // Finally, put all nodes in the free pool
    L2V.Pool.FreePtr = NULL;
    L2V.Pool.FreeCount = 0;
    for (j = L2V_NODEPOOL_COUNT-1; j >= 0; j--) {
        _L2V_FreeNode(&L2V.Pool.Node[j]);
    }

    return TRUE32;
}

#endif // ENABLE_L2V_TREE

