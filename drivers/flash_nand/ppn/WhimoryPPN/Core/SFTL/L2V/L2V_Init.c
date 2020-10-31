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

#include "L2V_Mem.h"

// Declare our L2V context structure here
L2V_t L2V;
UInt32 L2V_nodepool_mem = 0xffffffff;
UInt8 *L2V_root_depth;
tempNodePool_t tempNodes;

void L2V_Nuke(void)
{
    UInt32 i;
    Int32 j;

    // Set tree version
    L2V.treeVersion = 0;

    // Empty roots; default to miss
    for (i = 0; i < L2V.numRoots; i++) {
        L2V.Root[i].rootContig = ROOT_CONTIG_DEALLOC;
        L2V_root_depth[i] = 0;
    }

    // Reset periodic repacker variables
    L2V.RepackCounter = 0;

    // Finally, put all nodes in the free pool
    L2V.Pool.FreePtr = NULL;
    L2V.Pool.FreeCount = 0;
    for (j = L2V_NODEPOOL_COUNT-1; j >= 0; j--) {
        _L2V_FreeNode(&L2V.Pool.Node[j]);
    }
}

// Functions
BOOL32 L2V_Init(UInt32 numLbas, UInt32 max_sb, UInt32 vbas_per_sb)
{
    // Set up bits
    // ... nand lptrs
    L2V.bits.vba = WMR_LOG2((2 * max_sb * vbas_per_sb) - 1);
    WMR_ASSERT(L2V.bits.vba <= 30);
    L2V.bits.nand_span = 32 - 2 - L2V.bits.vba;

    // ... node lptrs
    L2V.bits.nodeIdx = WMR_LOG2((2 * L2V_NODEPOOL_COUNT) - 1);
    WMR_ASSERT(L2V.bits.nodeIdx <= 30);
    L2V.bits.node_span = 32 - 2 - L2V.bits.nodeIdx;

    // Set up special vbas
    L2V.vba.dealloc = (1 << L2V.bits.vba) - 1;
    L2V.vba.special = L2V.vba.dealloc;

    // Set number of superblocks for valid accounting division
    L2V.max_sb = max_sb;
    L2V.vbas_per_sb = vbas_per_sb;
    if ((max_sb * vbas_per_sb) > (L2V.vba.special+1)) {
        WMR_PANIC("Tree bitspace not sufficient for geometry %dx%d: please re-adjust", max_sb, vbas_per_sb);
    }

    // Calculate roots
    L2V.numRoots = (numLbas >> L2V_TREE_BITS) + 1;

    // Allocate arrays
    L2V.Root = WMR_MALLOC(L2V.numRoots * sizeof(Root_t));
    l2v_assert_ne(L2V.Root , NULL);
    L2V_root_depth = WMR_MALLOC(L2V.numRoots);
    l2v_assert_ne(L2V_root_depth , NULL);
    
    tempNodes.tempNodeEntries = NULL;
    tempNodes.isEntryAvailable = NULL;
    tempNodes.tempNodeEntries = WMR_MALLOC(sizeof(lNodeBig_t) * L2V_MAX_TREE_DEPTH);
    l2v_assert_ne(tempNodes.tempNodeEntries, NULL);
    tempNodes.isEntryAvailable = WMR_MALLOC(L2V_MAX_TREE_DEPTH);
    l2v_assert_ne(tempNodes.isEntryAvailable, NULL);
    tempNodes.poolSize = L2V_MAX_TREE_DEPTH;
    WMR_MEMSET(tempNodes.isEntryAvailable, 0xff, tempNodes.poolSize);
    
    if(L2V_nodepool_mem != 0xffffffff)
    {
        L2V.Pool.Node = (lNode_t*)WMR_MALLOC(L2V_nodepool_mem);
        if (NULL == L2V.Pool.Node) {
            return FALSE32;
        }
    }
    else
    {
        return FALSE32;
    }

    L2V_Nuke();

    return TRUE32;
}

