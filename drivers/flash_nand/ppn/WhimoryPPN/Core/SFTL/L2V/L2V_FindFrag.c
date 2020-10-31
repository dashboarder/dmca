/*
 * Copyright (c) 2009-2010 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include "L2V.h"
#include "L2V_Funcs.h"

typedef struct {
    UInt32 lba;
    UInt32 span;
} fragItem_t;

static struct {
    UInt32 curLba;
    fragItem_t item[L2V_FIND_FRAGS_MAX];
    UInt32 nextIdx;
    UInt32 idx;
    UInt32 cnt;
    UInt32 sum;
    UInt32 minSum;
    UInt32 minSumLba;

    fragItem_t listItem[L2V_LIST_FRAGS_MAX+1];
    UInt32 listSize;

    Int32 curTree;
} frag;

static void TreeFrag(lNode_t *node);

void L2V_FindFrag(UInt32 *d_lba, UInt32 *d_span)
{
    lNode_t *root;
    UInt32 tree;
    UInt32 lba, span;
    UInt32 curIdx;
    RootContig_t rc;

    if (frag.listSize < (L2V_LIST_FRAGS_MAX>>2)) {
        // Initialize structures
        frag.idx = 0;
        frag.cnt = 0;
        frag.sum = 0;
        frag.minSum = 0xffffffff;
        frag.minSumLba = 0xffffffff;

        // Iterate over roots
        for (tree = 0; tree < L2V.numRoots; tree++, frag.curTree--) {
            if (frag.curTree < 0) {
                frag.curTree = L2V.numRoots-1;
            }
            frag.curLba = frag.curTree << L2V_TREE_BITS;

            rc = L2V.Root[frag.curTree].rootContig;

            // No fragments if there's no tree!
            if (ROOT_CONTIG_DEALLOC == rc) {
                frag.curLba += L2V_TREE_SIZE;
                continue;
            }

            // Recurse and destroy
            root = L2V_IDX_TO_NODE(ROOT_CONTIG_GET_NODEIDX(rc));
            TreeFrag(root);

            if (frag.listSize && (L2V_FIND_FRAGS_MAX == frag.listItem[0].span)) {
                // Early out: it can't get any smaller!
                break;
            }
        }
    }

    // Extract from queue
    lba = frag.listItem[0].lba;
    span = frag.listItem[0].span;
    for (curIdx = 1; curIdx < frag.listSize; curIdx++) {
        frag.listItem[curIdx-1] = frag.listItem[curIdx];
    }
    frag.listSize--;

    // Limit
    span = WMR_MIN(span, L2V.vbas_per_sb<<1);

    // Report it
    *d_lba = lba;
    *d_span = span;
}

static void TreeFrag(lNode_t *node)
{
    UInt32 nOfs, nodeSize; // Byte offset within the node
    ContigUnpacked_t cu;
    lPtr_t thisPtr;
    UInt32 curIdx;

    // Iterate through the node,
    _L2V_NODE_ITERATE(node, nOfs, nodeSize, cu, thisPtr) {
        if (cu.isNodePtr) {
            // Recurse
            TreeFrag(L2V_IDX_TO_NODE(cu.u.nodeIdx));
        } else {
            // Prepare next index
            frag.nextIdx = frag.idx + 1;
            if (frag.nextIdx >= L2V_FIND_FRAGS_MAX)
                frag.nextIdx = 0;

            // Subtract prior span if the search is full
            if (L2V_FIND_FRAGS_MAX == frag.cnt) {
                frag.sum -= frag.item[frag.idx].span;
            }

            // Add to rolling average
            frag.item[frag.idx].span = cu.span;
            frag.item[frag.idx].lba = frag.curLba;
            frag.sum += cu.span;
            frag.cnt = WMR_MIN(frag.cnt+1, L2V_FIND_FRAGS_MAX);

            // Minify
            if (L2V_FIND_FRAGS_MAX == frag.cnt) {
                if (frag.listSize < L2V_LIST_FRAGS_MAX) {
                    frag.listItem[frag.listSize].lba = (UInt32)-1;
                    frag.listItem[frag.listSize].span = (UInt32)-1;
                    frag.listSize++;
                }

                // Insert, slipping from end
                curIdx = frag.listSize;
                while ((curIdx > 0) && (frag.sum < frag.listItem[curIdx-1].span)) {
                    curIdx--;
                    frag.listItem[curIdx+1].lba = frag.listItem[curIdx].lba;
                    frag.listItem[curIdx+1].span = frag.listItem[curIdx].span;
                }

                if (curIdx < frag.listSize) {
                    frag.listItem[curIdx].lba = frag.item[frag.nextIdx].lba;
                    frag.listItem[curIdx].span = frag.sum;
                    // Empty pending, to guarantee no overlaps
                    frag.idx = 0;
                    frag.sum = 0;
                    frag.cnt = 0;
                }
            }

            // Next
            frag.curLba += cu.span;
            frag.idx = frag.nextIdx;
        }
    } _L2V_NODE_ITERATE_END(nOfs);
}

