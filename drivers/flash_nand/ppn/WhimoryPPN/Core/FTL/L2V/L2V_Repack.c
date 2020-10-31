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

#define AND_TRACE_LAYER FTL

#include "WMRFeatures.h"
#if ENABLE_L2V_TREE
#include "L2V_Repack.h"
#include "L2V.h"
#include "L2V_Types.h"
#include "L2V_Mem.h"
#include "L2V_Funcs.h"


#define REPACK_MAX_NODES (L2V_TREE_SIZE/L2V_MIN_CONTIG_PER_NODE)
#define REPACK_TREE_LOG_SIZE 32

typedef struct {
    lNode_t *bottom[REPACK_MAX_NODES];
    UInt32 bottomSpan[REPACK_MAX_NODES];
    UInt32 bottomIdx;
    UInt32 bottom_nOfs;
    UInt32 bottom_nodeSize;
    UInt32 repackTreeLog[REPACK_TREE_LOG_SIZE];
    UInt32 repackTreeLogIdx;
} RepackCtx_t;


// Static definitions
static void CopyAndDestroy(RepackCtx_t *r, lNode_t *node);
static int  Rebuild(RepackCtx_t *r);
static void ReplaceRoot(RepackCtx_t *r, UInt32 tree);


void L2V_PeriodicRepack()
{
    Int32 best;
    UInt32 merit, bestMerit, i;

    // Check if it's appropriate or not
    if (L2V.RepackCounter < L2V_REPACK_PERIOD) {
        return;
    }
    L2V.RepackCounter = 0;

    // Find best candidate; merit==updates
    bestMerit = 0;
    best = -1;
    for (i = 0; i < L2V.numRoots; i++) {
        merit = L2V.UpdatesSinceRepack[i];
        if (merit >= bestMerit) {
            bestMerit = merit;
            best = i;
        }
    }

    l2v_assert_ne(best, -1);

    L2V_Repack(best);
}


// WARNING: NOT REENTRANT
static RepackCtx_t r;
void L2V_Repack(UInt32 tree)
{
    lNode_t *root;
    ContigUnpacked_t cu;

    LPTR_UNPACK(L2V.Tree[tree], cu);

    // Trying to repack a rootptr?  Abort...
    if (!cu.isNodePtr) {
        return;
    }

    WMR_TRACE_1(Repack, START, tree);

    // Up tree version for search-update coherency
    L2V.treeVersion++;

    // Create list of new bottom layer
    r.bottomIdx = 0;
    r.bottom_nOfs = 0;
    r.bottom_nodeSize = L2V_NODE_SIZE;
    WMR_MEMSET(r.bottom, 0, REPACK_MAX_NODES * sizeof(r.bottom[0]));
    WMR_MEMSET(r.bottomSpan, 0, REPACK_MAX_NODES * sizeof(r.bottomSpan[0]));

    // Recurse and destroy
    root = L2V_IDX_TO_NODE(cu.u.nodeIdx);
    CopyAndDestroy(&r, root);

    l2v_assert_ne(r.bottom_nOfs, 0);

    // Fill rest with "empty" markers
    _L2V_NODE_FILL(r.bottom[r.bottomIdx], r.bottom_nOfs, r.bottom_nodeSize);

    // Rebuild hierarchy
    L2V.TreeNodes[tree] = Rebuild(&r);

    // Replace root
    ReplaceRoot(&r, tree);

    // Clear repack counter
    L2V.UpdatesSinceRepack[tree] = 0;


    // Log which tree we repacked
    if (r.repackTreeLogIdx >= REPACK_TREE_LOG_SIZE) {
        r.repackTreeLogIdx = 0;
    }
    r.repackTreeLog[r.repackTreeLogIdx] = tree;
    r.repackTreeLogIdx++;

    WMR_TRACE_0(Repack, END);
}


static void CopyAndDestroy(RepackCtx_t *r, lNode_t *node)
{
    UInt32 nOfs, nodeSize; // Byte offset within the node
    ContigUnpacked_t cu;
    lPtr_t thisPtr;

    // Iterate through the node,
    _L2V_NODE_ITERATE(node, nOfs, nodeSize, cu, thisPtr) {
        if (cu.isNodePtr) {
            // Recurse
            CopyAndDestroy(r, L2V_IDX_TO_NODE(cu.u.nodeIdx));
        } else {
            // Would be full?  Move along...
            if ((r->bottom_nOfs + _L2V_CONTIG_SIZE(cu)) > r->bottom_nodeSize) {
                // Fill rest with "empty" markers
                _L2V_NODE_FILL(r->bottom[r->bottomIdx], r->bottom_nOfs, r->bottom_nodeSize);
                
                r->bottomIdx++;
                l2v_assert_lt(r->bottomIdx, REPACK_MAX_NODES);
            }

            // Allocate node if necessary
            if (NULL == r->bottom[r->bottomIdx]) {
                // Allocate...
                r->bottom[r->bottomIdx] = _L2V_AllocNode();
                r->bottom_nOfs = 0;
                r->bottom_nodeSize = L2V_NODE_SIZE;
            }

            // Copy out
            _L2V_NODE_PUSH_CONTIG(r->bottom[r->bottomIdx], r->bottom_nOfs, r->bottom_nodeSize, cu);
            r->bottomSpan[r->bottomIdx] += cu.span;
            l2v_assert_le(cu.span, L2V_TREE_SIZE);
        }
    } _L2V_NODE_ITERATE_END(nOfs);

    // Free the node
    _L2V_FreeNode(node);
}


static int Rebuild(RepackCtx_t *r)
{
    lNode_t *thisBranch;
    UInt32 nOfs, nodeSize, thisSpan, i, branchIdx;
    ContigUnpacked_t cu;
    int nodeCount;

    nodeCount = r->bottomIdx+1;
    while (r->bottomIdx > 0) {
        thisBranch = _L2V_AllocNode();
        nodeCount++;
        nOfs = 0;
        nodeSize = L2V_NODE_SIZE;
        thisSpan = 0;
        branchIdx = 0;

        // Rebuild this layer
        for (i = 0; i <= r->bottomIdx; i++) {
            cu.isNodePtr = 1;
            cu.u.nodeIdx = L2V_NODE_TO_IDX(r->bottom[i]);
            cu.span = r->bottomSpan[i];

            if ((nOfs + _L2V_CONTIG_SIZE(cu)) > nodeSize) {
                // Fill rest with "empty" markers
                _L2V_NODE_FILL(thisBranch, nOfs, nodeSize);

                // Place it down and get a new one
                r->bottom[branchIdx] = thisBranch;
                r->bottomSpan[branchIdx] = thisSpan;
                branchIdx++;
                thisBranch = _L2V_AllocNode();
                nodeCount++;
                nOfs = 0;
                nodeSize = L2V_NODE_SIZE;
                thisSpan = 0;
            }

            _L2V_NODE_PUSH_CONTIG(thisBranch, nOfs, nodeSize, cu);
            thisSpan += cu.span;
        }

        l2v_assert_ne(nOfs, 0);

        r->bottom[branchIdx] = thisBranch;
        r->bottomSpan[branchIdx] = thisSpan;
        // Fill rest with "empty" markers
        _L2V_NODE_FILL(thisBranch, nOfs, nodeSize);

        // Make sure the repack didn't munge the root node's spans
        if (0 == branchIdx) {
            l2v_assert_eq(thisSpan, L2V_TREE_SIZE);
        }

        // Get ready for the next round
        r->bottomIdx = branchIdx;
    }

    return nodeCount;
}


static void ReplaceRoot(RepackCtx_t *r, UInt32 tree)
{
    ContigUnpacked_t cu;
    l2v_assert_eq(r->bottomIdx, 0);
    cu.u.nodeIdx = L2V_NODE_TO_IDX(r->bottom[0]);
    cu.hasSpanOF = 0;
    cu.span = 0;
    L2V.Tree[tree] = LPTR_PACK_NODE(cu);
    if (!LPTR_IS_NODEPTR(L2V.Tree[tree])) {
        L2V_root_depth[tree] = 0;
    }
}

#endif // ENABLE_L2V_TREE

