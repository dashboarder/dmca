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
#include "L2V_Funcs.h"


void L2V_Search_Init(L2V_SearchCtx_t *c)
{
    c->next_lba = 0xffffffff;
    c->last_node = NULL;
    c->span = 0;
}


static BOOL32 HitCache(L2V_SearchCtx_t *c)
{
    ContigUnpacked_t cu;
    lPtr_t thisPtr;

    
    if ((c->treeVersion == L2V.treeVersion) && (c->lba == c->next_lba) && (NULL != c->last_node)) {
        // Sequential HIT
        _L2V_NODE_ITERATE_RESUME(c->last_node, c->next_nOfs, c->nodeSize, cu, thisPtr) {
            if (cu.isNodePtr)
                break;

            c->vba = cu.u.vba;
            c->fromStart = 0;
            c->span = cu.span;

            _L2V_NODE_ITERATE_NEXT(c->next_nOfs); // This will move nOfs along
            c->next_lba = c->lba + c->span;
            return TRUE32;

        } _L2V_NODE_ITERATE_END_NOP;
    }

    // Clear search history
    c->next_lba = 0xffffffff;
    c->last_node = NULL;

    return FALSE32;
}


void L2V_Search(L2V_SearchCtx_t *c)
{
    UInt32 targTree, targTofs;
    UInt32 thisStart, thisMax;
    UInt32 nOfs, nodeSize; // Byte offset within the node, and node effective size
    ContigUnpacked_t cu, thiscu;
    lNode_t *node = NULL;
    lPtr_t thisPtr;

    if (HitCache(c)) {
        return;
    }

    // Determine first root
    targTree = c->lba >> L2V_TREE_BITS;
    targTofs = c->lba & L2V_TREE_MASK;
    l2v_assert_lt(targTree, L2V.numRoots);

    c->level = 0;
    thisStart = 0;
    c->vba = L2V_VBA_DEALLOC;
    ROOT_CONTIG_UNPACK(L2V.Root[targTree].rootContig, cu);
    thiscu.span = L2V_TREE_SIZE;
    c->next_nOfs = 0;

    // Search from top down
    while (c->level < L2V_MAX_TREE_DEPTH) {
        // Down and next
        // Decode node pointer
        if (!cu.isNodePtr) {
            // Terminal case--pointer to physical location; calculate relative offset...
            c->vba = cu.u.vba;
            c->fromStart = targTofs - thisStart;
            if (cu.u.vba < L2V_VBA_SPECIAL) {
                c->vba += c->fromStart;
            }
            c->span = thiscu.span - c->fromStart;
            // Set up return case for sequential searching
            c->last_node = node;
            c->next_lba = c->lba + c->span;
            c->treeVersion = L2V.treeVersion;
            // c->next_nOfs is populated from code below, i.e. the previous iteration of this while loop

            // Is it getting really big?  Do some preventative maintenence...
            if (c->level >= L2V_TREE_DEPTH_REPACK) {
                L2V_Repack(targTree);
            }
            return;
        }

        // Walk through
        node = L2V_IDX_TO_NODE(cu.u.nodeIdx);

        // Iterate through the node,
        _L2V_NODE_ITERATE(node, nOfs, nodeSize, thiscu, thisPtr) {
            // Calculate endpoint
            thisMax = thisStart + thiscu.span;

            // Are we in range?
            if (targTofs < thisMax) {
                // HIT!  (Left-hand side implied by order of search.)
                cu = thiscu;
                _L2V_NODE_ITERATE_NEXT(nOfs);
                c->next_nOfs = nOfs;
                c->nodeSize = nodeSize;
                goto decodePtr;
            }

            // Move along in the node
            thisStart = thisMax;
        } _L2V_NODE_ITERATE_END(nOfs);

        WMR_PANIC("NAND index: bad tree: targTofs %d, level %d\n", targTofs, c->level);

decodePtr:
        // Resume here so we can check the while() condition
        c->level++;
    }

    l2v_assert(0); // Should never get here; otherwise, the tree is too deep (or recursive or pointing to randomness)
}

