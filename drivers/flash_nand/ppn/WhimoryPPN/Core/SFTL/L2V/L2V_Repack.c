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
#if REPACK_DATA_GATHERING
    UInt32 repackedTree[REPACK_TREE_LOG_SIZE];
    UInt32 updSinceRepack[REPACK_TREE_LOG_SIZE];
    UInt32 numNodesBef[REPACK_TREE_LOG_SIZE];
    UInt32 numNodesAft[REPACK_TREE_LOG_SIZE];
    UInt32 numNodeContBef[REPACK_TREE_LOG_SIZE];
    UInt32 numNodeContAft[REPACK_TREE_LOG_SIZE];
    UInt32 numNandCont[REPACK_TREE_LOG_SIZE];
    UInt32 timeREP[REPACK_TREE_LOG_SIZE];
    UInt32 timeCD[REPACK_TREE_LOG_SIZE];
    UInt32 timeREB[REPACK_TREE_LOG_SIZE];
    UInt32 repackTreeLogIdx;
#endif
} RepackCtx_t;


// Static definitions
static void CopyAndDestroy(RepackCtx_t *r, lNode_t *node);
static int  Rebuild(RepackCtx_t *r);
static void ReplaceRoot(RepackCtx_t *r, UInt32 tree);
static int CopyAndDestroyOneNode(RepackCtx_t *r, lNode_t *node, int nodeCount);


UInt32 L2V_ForceRepack(void)
{
    UInt32 best;
    UInt32 merit, bestMerit, i;

    // Find best candidate; merit==updates
    bestMerit = 0;
    best = 0xffffffff;
    for (i = 0; i < L2V.numRoots; i++) {
        if((L2V.Root[i].rootContig != ROOT_CONTIG_DEALLOC) && (L2V.Root[i].numNodes > 0) && (L2V.Root[i].updatesSinceRepack > 0))
        {
            merit = L2V.Root[i].updatesSinceRepack;
            if (merit >= bestMerit) {
                bestMerit = merit;
                best = i;
            }
        }
    }

    if(L2V_CriticalMem)
    {
        l2v_assert_ne(best, 0xffffffff);
        l2v_assert_ne(bestMerit, 0);
    }

    if(best != 0xffffffff)
    {
        L2V_Repack(best);
    }
    return best;
}

void L2V_PeriodicRepack()
{
    // Check if it's appropriate or not
    if (L2V.RepackCounter < L2V_REPACK_PERIOD) {
        return;
    }
    L2V.RepackCounter = 0;

    L2V_ForceRepack();
}


// WARNING: NOT REENTRANT
static RepackCtx_t r;
void L2V_Repack(UInt32 tree)
{
    lNode_t *root;
    RootContig_t rc;
#if REPACK_DATA_GATHERING
    UInt64 ticks, ticks_CD, ticks_REB;

    ticks = WMR_CLOCK_TICKS();

    // Log which tree we repacked
    if (r.repackTreeLogIdx >= REPACK_TREE_LOG_SIZE) {
        r.repackTreeLogIdx = 0;
    }
    r.repackedTree[r.repackTreeLogIdx] = tree;
    r.updSinceRepack[r.repackTreeLogIdx] = L2V.Root[tree].updatesSinceRepack;
    r.numNodesBef[r.repackTreeLogIdx] = L2V.Root[tree].numNodes;
    r.numNodeContBef[r.repackTreeLogIdx] = 0;
    r.numNodeContAft[r.repackTreeLogIdx] = 0;
    r.numNandCont[r.repackTreeLogIdx] = 0;

#endif

    rc = L2V.Root[tree].rootContig;

    // Trying to repack a rootptr?  Abort...
    if (ROOT_CONTIG_DEALLOC == rc) {
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
    root = L2V_IDX_TO_NODE(ROOT_CONTIG_GET_NODEIDX(rc));
#if REPACK_DATA_GATHERING
    ticks_CD = WMR_CLOCK_TICKS();
#endif
    CopyAndDestroy(&r, root);
#if REPACK_DATA_GATHERING
    ticks_CD = WMR_CLOCK_TICKS() - ticks_CD;
#endif

    l2v_assert_ne(r.bottom_nOfs, 0);

#if REPACK_DATA_GATHERING
    ticks_REB = WMR_CLOCK_TICKS();
#endif
    // Rebuild hierarchy
    L2V.Root[tree].numNodes = Rebuild(&r);
#if REPACK_DATA_GATHERING
    ticks_REB = WMR_CLOCK_TICKS() - ticks_REB;
#endif

    // Replace root
    ReplaceRoot(&r, tree);

    // Clear repack counter
    L2V.Root[tree].updatesSinceRepack = 0;

#if REPACK_DATA_GATHERING
    ticks = WMR_CLOCK_TICKS() - ticks;

    r.numNodesAft[r.repackTreeLogIdx] = L2V.Root[tree].numNodes;
    r.timeREP[r.repackTreeLogIdx] = (UInt32)(ticks / WMR_GET_TICKS_PER_US());
    r.timeCD[r.repackTreeLogIdx] = (UInt32)(ticks_CD / WMR_GET_TICKS_PER_US());
    r.timeREB[r.repackTreeLogIdx] = (UInt32)(ticks_REB / WMR_GET_TICKS_PER_US());

    r.repackTreeLogIdx++;

#endif

    WMR_TRACE_0(Repack, END);
}


static int CopyAndDestroyOneNode(RepackCtx_t *r, lNode_t *node, int nodeCount)
{
    UInt32         nOfs, nodeSize; // Byte offset within the node
    ContigUnpacked_t cu;
    lPtr_t           thisPtr;
    
    // Iterate through the node,
    _L2V_NODE_ITERATE(node, nOfs, nodeSize, cu, thisPtr) {
        // Would be full?  Move along...
        if ((r->bottom_nOfs + _L2V_CONTIG_SIZE(cu)) > r->bottom_nodeSize) {
            r->bottomIdx++;
            l2v_assert_lt(r->bottomIdx, REPACK_MAX_NODES);
        }
        
        // Allocate node if necessary
        if (NULL == r->bottom[r->bottomIdx]) {
            // Allocate...
            r->bottom[r->bottomIdx] = _L2V_AllocNode();
            nodeCount++;
            r->bottom_nOfs          = 0;
            r->bottom_nodeSize      = L2V_NODE_SIZE;
        }
        
        // Copy out
        _L2V_NODE_PUSH_CONTIG(r->bottom[r->bottomIdx], r->bottom_nOfs, r->bottom_nodeSize, cu);
        r->bottomSpan[r->bottomIdx] += cu.span;
        l2v_assert_le(cu.span, L2V_TREE_SIZE);
    } _L2V_NODE_ITERATE_END(nOfs);
    
    // Free the node
    _L2V_FreeNode(node);
    nodeCount--;
    return nodeCount;
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
#if REPACK_DATA_GATHERING
            r->numNodeContBef[r->repackTreeLogIdx]++;
#endif
            CopyAndDestroy(r, L2V_IDX_TO_NODE(cu.u.nodeIdx));
        } else {
#if REPACK_DATA_GATHERING
            r->numNandCont[r->repackTreeLogIdx]++;
#endif
            // Would be full?  Move along...
            if ((r->bottom_nOfs + _L2V_CONTIG_SIZE(cu)) > r->bottom_nodeSize) {
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
    lNode_t *tempBottomNode;
    
    nodeCount = r->bottomIdx + 1;
    
    while (r->bottomIdx > 0) {
        thisBranch = _L2V_AllocNode();
        nodeCount++;
        nOfs = 0;
        nodeSize = L2V_NODE_SIZE;
        thisSpan = 0;
        branchIdx = 0;

        // Rebuild this layer
        for (i = 0; i < r->bottomIdx; i++) {
            cu.isNodePtr = 1;
            cu.u.nodeIdx = L2V_NODE_TO_IDX(r->bottom[i]);
            cu.span = r->bottomSpan[i];

            if ((nOfs + _L2V_CONTIG_SIZE(cu)) > nodeSize) {
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
#if REPACK_DATA_GATHERING
            r->numNodeContAft[r->repackTreeLogIdx]++;
#endif
        }

        l2v_assert_ne(nOfs, 0);

        r->bottom[branchIdx] = thisBranch;
        r->bottomSpan[branchIdx] = thisSpan;
        tempBottomNode = r->bottom[r->bottomIdx];
        // Get ready for the next round
        r->bottomIdx = branchIdx;
        r->bottom_nOfs = nOfs;
        r->bottom_nodeSize = nodeSize;
        WMR_MEMSET(&(r->bottom[branchIdx + 1]), 0, (REPACK_MAX_NODES - (branchIdx + 1)) * sizeof(r->bottom[0]));
        WMR_MEMSET(&(r->bottomSpan[branchIdx + 1]), 0, (REPACK_MAX_NODES - (branchIdx + 1)) * sizeof(r->bottomSpan[0]));
        
        // repack the remainder to ensure we do not waste extra node
        nodeCount = CopyAndDestroyOneNode(r, tempBottomNode, nodeCount);
        
        // Make sure the repack didn't munge the root node's spans
        if (0 == r->bottomIdx) {
            l2v_assert_eq(r->bottomSpan[r->bottomIdx], L2V_TREE_SIZE);
        }
    }

    return nodeCount;
}


static void ReplaceRoot(RepackCtx_t *r, UInt32 tree)
{
    ContigUnpacked_t cu;
    l2v_assert_eq(r->bottomIdx, 0);
    cu.u.nodeIdx = L2V_NODE_TO_IDX(r->bottom[0]);
    cu.isNodePtr = 1;
    L2V.Root[tree].rootContig = ROOT_CONTIG_PACK(cu);
    if(ROOT_CONTIG_DEALLOC == L2V.Root[tree].rootContig)
    {
        L2V_root_depth[tree] = 0;
    }
}

