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
#include "L2V_Funcs.h"
#include "L2V_Mem.h"
#include "L2V_Valid.h"
#include "L2V_Repack.h"
#include "L2V_Types.h"


// #####   TYPE DEFINITIONS  -  LOCAL TO THIS SOURCE FILE   #########################

typedef struct {
    UInt32 incomingTree;
    UInt32 incomingOfs, incomingSize;
    UInt32 incomingEnd;
    UInt32 vpn;

    UInt32 thisStart, thisMax;  // Span start/end ofs while in inner loop

    ContigUnpacked_t cu;
    lPtr_t nodePtr;
    lPtr_t thisPtr;
    BOOL32 needsInsert;
    lNode_t *node;
    UInt32 nOfs;
    UInt32 nodeSize;

    BOOL32 changed; // Did we change this node?
    lNodeBig_t tmpNode;

    lNode_t *atLevel[L2V_MAX_TREE_DEPTH_UPD+5]; // Add some guard-band
    UInt32 level;
} UpdateCtx_t;


// #####   PROTOTYPES  -  LOCAL TO THIS SOURCE FILE   ###############################

static void UpdateGuts(UInt32 lba, UInt32 size, UInt32 vpn);
static void TeardownTree(UInt32 tree);
static int  DestroySpanLeft(UInt32 tree, ContigUnpacked_t *cu, lPtr_t *p, UInt32 amount);
static int  DestroySpanRight(UInt32 tree, ContigUnpacked_t *cu, lPtr_t *p, UInt32 keepAmount, UInt32 fullSpan);
static void DestroySpanMiddle(lPtr_t *p, UInt32 left, UInt32 right);
static void TreeToNand(UInt32 tree, UInt32 vpn);

// Helpers:
static void SuckOneUp(UInt32 tree, lNodeBig_t *big, lPtr_t ptr);
static void Upd_TrivialInsert();
static BOOL32 Upd_IdentityReplace();
static BOOL32 Upd_CompleteOverlap();
static BOOL32 Upd_ChopRight();
static BOOL32 Upd_ChopLeft();
static void Upd_ChopMiddleNand();
static void Upd_FillRemainder();
static int  Upd_GetNodeSize();
static int  Upd_PushNodeOut(lNodeBig_t *b, lNode_t *node, UInt32 minEl, UInt32 maxEl);  // Returns sumSpan
static void Upd_SplitAndPush();
static void RippleUp(lNode_t *oldNode, UInt32 span0, lNode_t *newNode, UInt32 span1);
static void Upd_ShareLeft();
static void Upd_ShareRight();
static int  Upd_PushLeft(lNode_t *left);
static int  Upd_PushRight(lNode_t *right);


// #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ############################

void L2V_Update(UInt32 lba, UInt32 size, UInt32 vpn)
{
    UInt32 thisSize, treeOfs;

    // Running out of L2V memory?
    while (L2V_LowMem)
    {
        // Make room by forgetting some stuff
        L2V_ForgetBiggestTree();
    }

    // Up tree version for search-update coherency
    L2V.treeVersion++;

    while (size) {
        treeOfs = lba & L2V_TREE_MASK;
        thisSize = min(L2V_TREE_SIZE - treeOfs, size);

        UpdateGuts(lba, thisSize, vpn);

        size -= thisSize;
        lba += thisSize;
        if (vpn < L2V_VPN_SPECIAL) {
            vpn += thisSize;
        }
    }
}


// #####   FUNCTION DEFINITIONS  -  LOCAL TO THIS SOURCE FILE   #####################
UpdateCtx_t u;

static UInt32 L2V_GetDepth(lNode_t *node)
{
    UInt32 nOfs, nodeSize; // Byte offset within the node
    lPtr_t thisPtr;
    UInt32 depth, maxDepth;
    ContigUnpacked_t cu;
    
    // Walk through
    maxDepth = 0;
    
    // Iterate through the node,
    _L2V_NODE_ITERATE(node, nOfs, nodeSize, cu, thisPtr) {
        if (cu.isNodePtr) {
            depth = L2V_GetDepth(L2V_IDX_TO_NODE(cu.u.nodeIdx));
            if (depth > maxDepth) {
                maxDepth = depth;
            }
        }
    } _L2V_NODE_ITERATE_END(nOfs);
    
    return maxDepth+1;
}

static void UpdateGuts(UInt32 __lba, UInt32 __size, UInt32 __vpn)
{
    UInt32 nodesBefore;

    // Input: guaranteed to not span trees
    u.incomingTree = __lba >> L2V_TREE_BITS;
    u.incomingOfs  = __lba & L2V_TREE_MASK;
    u.incomingSize = __size;
    u.incomingEnd  = u.incomingOfs + u.incomingSize;
    u.vpn          = __vpn;
    if (u.incomingTree >= L2V.numRoots) {
        WMR_PANIC("L2V update with out-of-range LBA-- lba:%d, size:%d, vpn:0x%x", __lba, __size, __vpn);
    }
    nodesBefore = L2V.TreeNodes[u.incomingTree];

    // Repack periodicity counter update
    L2V.UpdatesSinceRepack[u.incomingTree]++;
    L2V.RepackCounter++;
    L2V_PeriodicRepack();

    // First check: update to whole tree?
    if ((0 == u.incomingOfs) && (L2V_TREE_SIZE == u.incomingSize)) {
        TeardownTree(u.incomingTree);
        TreeToNand(u.incomingTree, u.vpn);
        L2V_root_depth[u.incomingTree] = 0;
        return;
    }

    // Share left/right, ripple up?
    // Tear?

    u.level = 0;
    u.thisStart = 0;
    u.thisMax = L2V_TREE_SIZE;
    u.nodePtr = L2V.Tree[u.incomingTree];

    _L2V_ValidUp(u.vpn, u.incomingSize);


    if (!LPTR_IS_NODEPTR(u.nodePtr)) {
        Upd_TrivialInsert();
        return;
    }

    // Search from top down
    while (u.level < L2V_MAX_TREE_DEPTH_UPD) {
        // Walk through
        LPTR_UNPACK_NODE(u.nodePtr, u.cu);
        u.node = L2V_IDX_TO_NODE(u.cu.u.nodeIdx);
        u.atLevel[u.level] = u.node;
        u.changed = FALSE32;
        u.needsInsert = TRUE32;
        u.tmpNode.start = 0;
        u.tmpNode.max = 0;

        // Iterate through the node,
        _L2V_NODE_ITERATE(u.node, u.nOfs, u.nodeSize, u.cu, u.thisPtr) {
            // Calculate endpoint
            u.thisMax = u.thisStart + u.cu.span;
            if (u.cu.isNodePtr) {
                l2v_assert_lt(u.cu.u.nodeIdx, L2V_NODEPOOL_COUNT);
            }

            // Identity overlap?
            if (Upd_IdentityReplace()) {
                // Exit
                goto exit;
            }

            // Complete overlap?  Free this piece...
            else if (Upd_CompleteOverlap()) {
                // ok
            }

            // Chop right off?
            else if (Upd_ChopRight()) {
                // ok
            }

            // Chop left off?
            else if (Upd_ChopLeft()) {
                // ok
            }

            // Chop middle?  (+2 total)
            else if ((u.incomingOfs > u.thisStart) && (u.incomingEnd < u.thisMax)) {
                if (!u.cu.isNodePtr) {
                    Upd_ChopMiddleNand();
                } else {
                    u.nodePtr = u.thisPtr;
                    goto nextLevel;
                }
            }

            // Too far right or left?  Just copy it out...
            Upd_FillRemainder();

            // Move along in the node
            u.thisStart = u.thisMax;
        } _L2V_NODE_ITERATE_END(u.nOfs);

        // Last guy in the tree, with CompleteOverlaps to the left?  (Since that case doesn't insert.)
        if (u.needsInsert) {
            u.changed = TRUE32;
            // +1
            _L2V_NodeBig_PushContig_Nand(&u.tmpNode, u.vpn, u.incomingSize);
            u.needsInsert = FALSE32;
        }

        if (u.changed) {
            // Will it fit?
            if ((u.tmpNode.max > L2V_MIN_CONTIG_PER_NODE) && ((u.nOfs = Upd_GetNodeSize()) > L2V_NODE_SIZE)) {
                // Try to share left/right
                Upd_ShareLeft();
                Upd_ShareRight();

                // Will it fit now?
                if (((u.tmpNode.max-u.tmpNode.start) > L2V_MIN_CONTIG_PER_NODE) && ((u.nOfs = Upd_GetNodeSize()) > L2V_NODE_SIZE)) {
                    // Darn, split it
                    Upd_SplitAndPush();
                } else {
                    Upd_PushNodeOut(&u.tmpNode, u.node, u.tmpNode.start, u.tmpNode.max);
                }
                goto exit;
            }

            if ((1 == (u.tmpNode.max - u.tmpNode.start)) && (0 == u.level)) {
                // Single node, single entry?  Squash up into root.
                _L2V_FreeNode(u.node);
                L2V.TreeNodes[u.incomingTree]--;
                TreeToNand(u.incomingTree, u.tmpNode.cu[u.tmpNode.start].u.n.vpn);
                L2V_root_depth[u.incomingTree] = 0;
                goto exit;
            }

            Upd_PushNodeOut(&u.tmpNode, u.node, 0, u.tmpNode.max);
            goto exit;
        }

        l2v_assert(0);

nextLevel:
        u.level++;
    }

    // Fell through

    // Should never get here; otherwise, the tree is too deep (or recursive or pointing to randomness)
    WMR_PANIC("NAND index update fell through: abort at level %d\n", u.level);

exit:
    // Did it get bigger than we thought it should?
    if (u.level > L2V_TREE_DEPTH_REPACK)
    {
        L2V_Repack(u.incomingTree);
        if (LPTR_IS_NODEPTR(L2V.Tree[u.incomingTree]))
        {
            L2V_root_depth[u.incomingTree] = L2V_GetDepth(L2V_IDX_TO_NODE(LPTR_GET_NODEIDX(L2V.Tree[u.incomingTree])));
        }
        else
        {
            L2V_root_depth[u.incomingTree] = 0;
        }
        return;
    }
    
    if(nodesBefore <= L2V.TreeNodes[u.incomingTree])
    {
        L2V_root_depth[u.incomingTree] += (L2V.TreeNodes[u.incomingTree] - nodesBefore);
        if(L2V_root_depth[u.incomingTree] < L2V_TREE_DEPTH_REPACK)
        {
            return;
        }
    }
    
    if (LPTR_IS_NODEPTR(L2V.Tree[u.incomingTree]))
    {
        L2V_root_depth[u.incomingTree] = L2V_GetDepth(L2V_IDX_TO_NODE(LPTR_GET_NODEIDX(L2V.Tree[u.incomingTree])));
    }
    else
    {
        L2V_root_depth[u.incomingTree] = 0;
        return;
    }
    
    if(L2V_root_depth[u.incomingTree] >= L2V_TREE_DEPTH_REPACK)
    {
        L2V_Repack(u.incomingTree);
        if (LPTR_IS_NODEPTR(L2V.Tree[u.incomingTree]))
        {
            L2V_root_depth[u.incomingTree] = L2V_GetDepth(L2V_IDX_TO_NODE(LPTR_GET_NODEIDX(L2V.Tree[u.incomingTree])));
        }
        else
        {
            L2V_root_depth[u.incomingTree] = 0;
        }
    }
}


static void Teardown(lNode_t *node, UInt32 tree)
{
    UInt32 nOfs, nodeSize; // Byte offset within the node and size
    lPtr_t thisPtr;
    ContigUnpacked_t cu;

    // Iterate through the node,
    _L2V_NODE_ITERATE(node, nOfs, nodeSize, cu, thisPtr) {
        if (cu.isNodePtr) {
            // Recurse
            Teardown(L2V_IDX_TO_NODE(cu.u.nodeIdx), tree);
        } else {
            // Reduce valid counters
            _L2V_ValidDown(cu.vpn, cu.span);
        }
    } _L2V_NODE_ITERATE_END(nOfs);

    // Free the node
    _L2V_FreeNode(node);
    L2V.TreeNodes[tree]--;
}


static void TeardownTree(UInt32 tree)
{
    ContigUnpacked_t cu;

    // Unpack the node
    LPTR_UNPACK(L2V.Tree[tree], cu);

    if (cu.isNodePtr) {
        // Tear down the tree
        Teardown(L2V_IDX_TO_NODE(cu.u.nodeIdx), tree);
    } else {
        // It's a contig (probably just token but in case tree-sb ratios change, this line is necessary)
        _L2V_ValidDown(cu.vpn, L2V_TREE_SIZE);
    }
}


static int DestroySpanLeft(UInt32 tree, ContigUnpacked_t *cu, lPtr_t *p, UInt32 amount)
{
    lNode_t *node;
    UInt32 nOfs, nodeSize; // Byte offset within the node, and size after subtracting overflows
    UInt32 thisAmount;
    Int32 children;
    lNodeBig_t tmpNode;
    BOOL32 changed = FALSE32;
    ContigUnpacked_t thiscu;
    lPtr_t thisPtr;

    l2v_assert_ne(amount, 0);
    l2v_assert_le(amount, L2V_TREE_SIZE);

    if (!cu->isNodePtr) {
        _L2V_ValidDown(cu->vpn, amount);
        if (cu->u.n.vpn < L2V_VPN_SPECIAL) {
            cu->u.n.vpn += amount;
            *p = LPTR_PACK_NAND(*cu);
        }
        return -1;
    } else {
        node = L2V_IDX_TO_NODE(cu->u.nodeIdx);
        tmpNode.max = 0;

        // Iterate through the node,
        _L2V_NODE_ITERATE(node, nOfs, nodeSize, thiscu, thisPtr) {
            thisAmount = min(amount, thiscu.span);
            if (thisAmount == thiscu.span) {
                changed = TRUE32;
                if (!thiscu.isNodePtr) {
                    _L2V_ValidDown(thiscu.vpn, thisAmount);
                } else {
                    DestroySpanLeft(tree, &thiscu, &thisPtr, thiscu.span);
                }
            } else if (thisAmount == 0) {
                // Copy it out
                _L2V_NodeBig_PushContig(&tmpNode, &thiscu);
            } else {
                changed = TRUE32;
                children = -1;

                if (!thiscu.isNodePtr) {
                    _L2V_ValidDown(thiscu.vpn, thisAmount);
                    if (thiscu.u.n.vpn < L2V_VPN_SPECIAL) {
                        thiscu.u.n.vpn += thisAmount;
                    }
                } else {
                    children = DestroySpanLeft(tree, &thiscu, &thisPtr, thisAmount);
                }

                // Copy it out
                if (children == 1) {
                    SuckOneUp(tree, &tmpNode, thisPtr);
                } else {
                    thiscu.span -= thisAmount;
                    _L2V_NodeBig_PushContig(&tmpNode, &thiscu);
                }
            }
            amount -= thisAmount;
        } _L2V_NODE_ITERATE_END(nOfs);

        if (changed) {
            // Push it back out
            Upd_PushNodeOut(&tmpNode, node, 0, tmpNode.max);
        }

        if (0 == tmpNode.max) {
            // Free this one
            _L2V_FreeNode(node);
            L2V.TreeNodes[tree]--;
            *p = L2V_CONTIG_FREE_SIG;
        }

        return tmpNode.max;
    }
}


static int DestroySpanRight(UInt32 tree, ContigUnpacked_t *cu, lPtr_t *p, UInt32 keepAmount, UInt32 fullSpan)
{
    lPtr_t thisPtr;
    lNode_t *node;
    UInt32 nOfs, nodeSize; // Byte offset within the node
    UInt32 thisAmount;
    Int32 children;
    lNodeBig_t tmpNode;
    BOOL32 changed = FALSE32;
    ContigUnpacked_t thiscu;

    l2v_assert_ne(keepAmount, 0);
    l2v_assert_lt(keepAmount, L2V_TREE_SIZE);
    l2v_assert_ne(fullSpan, 0);
    l2v_assert_le(fullSpan, L2V_TREE_SIZE);

    if (!cu->isNodePtr) {
        _L2V_ValidDown(cu->vpn, fullSpan-keepAmount);
        return -1;
    } else {
        node = L2V_IDX_TO_NODE(cu->u.nodeIdx);
        tmpNode.max = 0;

        // Iterate through the node,
        _L2V_NODE_ITERATE(node, nOfs, nodeSize, thiscu, thisPtr) {
            thisAmount = min(keepAmount, thiscu.span);
            if (thisAmount == thiscu.span) {
                // Copy it out
                _L2V_NodeBig_PushContig(&tmpNode, &thiscu);
            } else if (thisAmount == 0) {
                changed = TRUE32;
                if (!thiscu.isNodePtr) {
                    _L2V_ValidDown(thiscu.vpn, thiscu.span);
                } else {
                    DestroySpanLeft(tree, &thiscu, &thisPtr, thiscu.span);
                }
            } else if (thisAmount < thiscu.span) {
                changed = TRUE32;
                children = -1;

                if (!thiscu.isNodePtr) {
                    _L2V_ValidDown(thiscu.vpn, thiscu.span - thisAmount);
                } else {
                    children = DestroySpanRight(tree, &thiscu, &thisPtr, thisAmount, thiscu.span);
                }

                // Copy it out
                if (children == 1) {
                    SuckOneUp(tree, &tmpNode, thisPtr);
                } else {
                    thiscu.span = thisAmount;
                    _L2V_NodeBig_PushContig(&tmpNode, &thiscu);
                }
            }
            keepAmount -= thisAmount;
        } _L2V_NODE_ITERATE_END(nOfs);

        if (changed) {
            // Push it back out
            Upd_PushNodeOut(&tmpNode, node, 0, tmpNode.max);
        }

        if (0 == tmpNode.max) {
            // Free this one
            _L2V_FreeNode(node);
            L2V.TreeNodes[tree]--;
            *p = L2V_CONTIG_FREE_SIG;
        }

        return tmpNode.max;
    }
}


static void DestroySpanMiddle(lPtr_t *p, UInt32 left, UInt32 right)
{
    ContigUnpacked_t cu;

    LPTR_UNPACK_NAND(*p, cu);
    l2v_assert_eq(cu.isNodePtr, 0); // Must not be a nodePtr, since we recurse in the top-level function
    _L2V_ValidDown(cu.vpn, right-left);
}


static void TreeToNand(UInt32 tree, UInt32 vpn)
{
    ContigUnpacked_t cu;

    cu.hasSpanOF = 0;
    cu.u.n.vpn = vpn;
    cu.span = 0;
    L2V.Tree[tree] = LPTR_PACK_NAND(cu);

    _L2V_ValidUp(vpn, L2V_TREE_SIZE);
}


static void Upd_TrivialInsert()
{
    UInt32 nOfs, nodeSize;
    lNode_t *node;
    ContigUnpacked_t cu;

    LPTR_UNPACK_NAND(u.nodePtr, u.cu);

    // Break the rootPtr into a node
    node = _L2V_AllocNode();
    nodeSize = L2V_NODE_SIZE;
    L2V.TreeNodes[u.incomingTree] = 1;
    nOfs = 0;

    // ValidDown for old section
    if (u.cu.u.n.vpn < L2V_VPN_SPECIAL) {
        _L2V_ValidDown(u.cu.u.n.vpn, u.incomingSize);
    }

    // Break left?
    if (u.incomingOfs) {
        cu.u.n.vpn = u.cu.u.n.vpn;
        cu.span = u.incomingOfs;
        _L2V_NODE_PUSH_CONTIG_NAND(node, nOfs, nodeSize, cu);
        l2v_assert_le(u.incomingOfs, L2V_TREE_SIZE);
    }

    // Insert node
    cu.u.n.vpn = u.vpn;
    cu.span = u.incomingSize;
    _L2V_NODE_PUSH_CONTIG_NAND(node, nOfs, nodeSize, cu);
    l2v_assert_le(u.incomingSize, L2V_TREE_SIZE);

    // Break Right?
    cu.span = L2V_TREE_SIZE - u.incomingEnd;
    if (cu.span != 0) {
        cu.u.n.vpn = u.cu.u.n.vpn;
        // Move vpn if not special SB
        if (cu.u.n.vpn < L2V_VPN_SPECIAL) {
            cu.u.n.vpn += u.incomingEnd;
        }
        _L2V_NODE_PUSH_CONTIG_NAND(node, nOfs, nodeSize, cu);
        l2v_assert_le(cu.span, L2V_TREE_SIZE);
    }

    // Fill rest of node with "empty" signature
    _L2V_NODE_FILL(node, nOfs, nodeSize);

    // Update Tree ptr
    cu.hasSpanOF = 0;
    cu.u.nodeIdx = L2V_NODE_TO_IDX(node);
    L2V.Tree[u.incomingTree] = LPTR_PACK_NODE(cu);
    if (!LPTR_IS_NODEPTR(L2V.Tree[u.incomingTree])) {
        L2V_root_depth[u.incomingTree] = 0;
    }
}


static BOOL32 Upd_IdentityReplace()
{
    lPtr_t *p;
    UInt32 check_nOfs, check_nodeSize;
    lPtr_t check_thisPtr;
    ContigUnpacked_t check_cu, local_cu;

    if ((u.incomingOfs == u.thisStart) && (u.incomingSize == u.cu.span)) {
        // =

        // Make sure we aren't missing a compression opportunity
        if (_L2V_NodeBig_WillCompress(u.tmpNode, u.vpn, u.incomingSize)) {
            return FALSE32;
        }

        // Check spans
        _L2V_NODE_ITERATE(u.node, check_nOfs, check_nodeSize, check_cu, check_thisPtr) {
        } _L2V_NODE_ITERATE_END(check_nOfs);

        // Destroy old, recursively destroying subtrees if necessary
        DestroySpanLeft(u.incomingTree, &u.cu, &u.thisPtr, u.cu.span);

        // Check spans
        _L2V_NODE_ITERATE(u.node, check_nOfs, check_nodeSize, check_cu, check_thisPtr) {
        } _L2V_NODE_ITERATE_END(check_nOfs);

        // Overwrite in place--works because nodeptrs are always 6 bytes, and if the span is the same size, updating in place will work
        p = (lPtr_t*)(&u.node->bytes[u.nOfs]);
        local_cu.isNodePtr = 0;
        local_cu.hasSpanOF = 0;
        if (LPTR_HASSPANOF(*p))
            local_cu.hasSpanOF = 1;
        local_cu.u.n.vpn = u.vpn;
        local_cu.span = u.cu.span;
        if (local_cu.hasSpanOF) {
            *(UInt16*)(&u.node->bytes[u.nodeSize]) = u.cu.span >> L2V_BITS_NAND_SPAN;
        }
        *p = LPTR_PACK_NAND(local_cu);

        // Check spans
        _L2V_NODE_ITERATE(u.node, check_nOfs, check_nodeSize, check_cu, check_thisPtr) {
        } _L2V_NODE_ITERATE_END(check_nOfs);

        return TRUE32;
    }

    return FALSE32;
}


static BOOL32 Upd_CompleteOverlap()
{
    if ((u.incomingOfs <= u.thisStart) && (u.incomingEnd >= u.thisMax)) {
        u.changed = TRUE32;
        // Destroy old span
        DestroySpanLeft(u.incomingTree, &u.cu, &u.thisPtr, u.cu.span);

        return TRUE32;
    }

    return FALSE32;
}


static void SuckOneUp(UInt32 tree, lNodeBig_t *big, lPtr_t ptr)
{
    lNode_t *node;
    UInt32 nOfs, nodeSize, count;
    lPtr_t thisPtr;
    ContigUnpacked_t cu;

    l2v_assert_eq(LPTR_IS_NODEPTR(ptr), 1);
    node = L2V_IDX_TO_NODE(LPTR_GET_NODEIDX(ptr));
    count = 0;

    // Iterate over children
    _L2V_NODE_ITERATE(node, nOfs, nodeSize, cu, thisPtr) {
        // Push into u->tmpNode
        _L2V_NodeBig_PushContig(big, &cu);
        count++;
    } _L2V_NODE_ITERATE_END(nOfs);
    l2v_assert_eq(count, 1);

    // Free old node
    _L2V_FreeNode(node);
    L2V.TreeNodes[tree]--;
}

static void SuckUp()
{
    lNode_t *node;
    UInt32 nOfs, nodeSize;
    lPtr_t thisPtr;
    ContigUnpacked_t cu;

    l2v_assert_eq(LPTR_IS_NODEPTR(u.thisPtr), 1);
    node = L2V_IDX_TO_NODE(LPTR_GET_NODEIDX(u.thisPtr));

    // Iterate over children
    _L2V_NODE_ITERATE(node, nOfs, nodeSize, cu, thisPtr) {
        // Push into u->tmpNode
        _L2V_NodeBig_PushContig(&u.tmpNode, &cu);
    } _L2V_NODE_ITERATE_END(nOfs);

    // Free old node
    _L2V_FreeNode(node);
    L2V.TreeNodes[u.incomingTree]--;
}


static BOOL32 Upd_ChopRight()
{
    Int32 children;
    ContigUnpacked_t cu;

    if ((u.incomingOfs > u.thisStart) && (u.incomingOfs < u.thisMax) && (u.incomingEnd >= u.thisMax)) {
        u.changed = TRUE32;
        children = DestroySpanRight(u.incomingTree, &u.cu, &u.thisPtr, u.incomingOfs - u.thisStart, u.cu.span);

        // Try to reduce tree depth and maximize usage by sucking children up
        if ((children >= 1) && (children <= L2V_SUCKUP_CHILDREN_MAX)) {
            SuckUp();
        } else if (0 != children) {
            // Still have a child node we need to reference
            cu = u.cu;
            cu.span = u.incomingOfs - u.thisStart;
            _L2V_NodeBig_PushContig(&u.tmpNode, &cu);
        }

        if (u.needsInsert) {
            // +1
            _L2V_NodeBig_PushContig_Nand(&u.tmpNode, u.vpn, u.incomingSize);
            u.needsInsert = FALSE32;
        } else {
            l2v_assert(0);
        }

        return TRUE32;
    }

    return FALSE32;
}


static BOOL32 Upd_ChopLeft()
{
    Int32 children;

    if ((u.incomingOfs <= u.thisStart) && (u.incomingEnd > u.thisStart) && (u.incomingEnd < u.thisMax)) {
        u.changed = TRUE32;
        children = DestroySpanLeft(u.incomingTree, &u.cu, &u.thisPtr, u.incomingEnd - u.thisStart);
        
        if (u.needsInsert) {
            // +1
            _L2V_NodeBig_PushContig_Nand(&u.tmpNode, u.vpn, u.incomingSize);
            u.needsInsert = FALSE32;
        }

        // Try to reduce tree depth and maximize usage by sucking children up
        if ((children >= 1) && (children <= L2V_SUCKUP_CHILDREN_MAX)) {
            SuckUp();
        } else if (0 != children) {
            // Still have a child node or nand-ptr we need to reference
            u.tmpNode.cu[u.tmpNode.max] = u.cu;
            // Ofs moving taken care of by DestroySpanleft
            u.tmpNode.cu[u.tmpNode.max].span = u.thisMax - u.incomingEnd;
            l2v_assert_ne(u.thisMax - u.incomingEnd, 0);
            u.tmpNode.max++;
        }

        return TRUE32;
    }

    return FALSE32;
}


static void Upd_ChopMiddleNand()
{
    u.changed = TRUE32;
    DestroySpanMiddle(&u.thisPtr, u.thisStart-u.incomingOfs, u.incomingEnd - u.incomingOfs);

    // Break left
    _L2V_NodeBig_PushContig_Nand(&u.tmpNode, u.cu.u.n.vpn, u.incomingOfs - u.thisStart);
    
    // Insert in middle
    // +1
    _L2V_NodeBig_PushContig_Nand(&u.tmpNode, u.vpn, u.incomingSize);
    u.needsInsert = FALSE32;

    // Break right
    // +1
    if (u.cu.u.n.vpn < L2V_VPN_SPECIAL) {
        u.cu.u.n.vpn += (u.incomingEnd - u.thisStart);
    }
    _L2V_NodeBig_PushContig_Nand(&u.tmpNode, u.cu.u.n.vpn, u.thisMax - u.incomingEnd);
}


static void Upd_FillRemainder()
{
    if ((u.thisStart >= u.incomingEnd) || (u.thisMax <= u.incomingOfs)) {
        // Did none of the above trims/etc insert the new ptr?
        // This is because we had an exact eclipse(s)
        if (u.needsInsert && (u.thisStart >= u.incomingEnd)) {
            // +1
            _L2V_NodeBig_PushContig_Nand(&u.tmpNode, u.vpn, u.incomingSize);
            u.needsInsert = FALSE32;
        }

        // Copy into tmpNode
        _L2V_NodeBig_PushContig(&u.tmpNode, &u.cu);
    }
}


static int Upd_GetNodeSize()
{
    UInt32 i, nOfs;

    nOfs = 0;

    for (i = u.tmpNode.start; i < u.tmpNode.max; i++) {
        nOfs += _L2V_CONTIG_SIZE(u.tmpNode.cu[i]);
    }

    return nOfs;
}


static int Upd_PushNodeOut(lNodeBig_t *b, lNode_t *node, UInt32 minEl, UInt32 maxEl)
{
    UInt32 i, nOfs, nodeSize;
    UInt32 sumSpan;

    sumSpan = 0;

    l2v_assert_lt(minEl, L2V_LNODEBIG_MAX);
    l2v_assert_le(maxEl, L2V_LNODEBIG_MAX);

    // Copy and squeeze
    nOfs = 0;
    nodeSize = L2V_NODE_SIZE;
    for (i = minEl; i < maxEl; i++) {
        _L2V_NODE_PUSH_CONTIG(node, nOfs, nodeSize, b->cu[i]);

        sumSpan += b->cu[i].span;
        l2v_assert_le(b->cu[i].span, L2V_TREE_SIZE);
    }

    // Fill rest of node with "empty" signature
    _L2V_NODE_FILL(node, nOfs, nodeSize);

    return sumSpan;
}


static void Upd_SplitAndPush()
{
    lNode_t *newNode, *oldNode;
    UInt32 half, span0, span1;

    // Get new node
    oldNode = u.node;
    newNode = _L2V_AllocNode();
    L2V.TreeNodes[u.incomingTree]++;

    // Divide and push to old/new
    half = ((u.tmpNode.max - u.tmpNode.start)>>1) + 1 + u.tmpNode.start;
    span0 = Upd_PushNodeOut(&u.tmpNode, oldNode, u.tmpNode.start, half);
    l2v_assert_ne(span0, 0);
    span1 = Upd_PushNodeOut(&u.tmpNode, newNode, half, u.tmpNode.max);
    l2v_assert_ne(span1, 0);

    RippleUp(oldNode, span0, newNode, span1);
}


static void RippleUp(lNode_t *oldNode, UInt32 span0, lNode_t *newNode, UInt32 span1)
{
    lNode_t *parent, *parent1;
    lPtr_t thisPtr;
    UInt32 nOfs, nodeSize, span;
    UInt32 parentspan0, parentspan1, half;
    ContigUnpacked_t cu;

    u.tmpNode.start = u.tmpNode.max = 0;

    if (0 == u.level) {
        // Special case: allocate another parent node, link these in
        parent = _L2V_AllocNode();
        L2V.TreeNodes[u.incomingTree]++;
        _L2V_NodeBig_PushContig_Node(&u.tmpNode, L2V_NODE_TO_IDX(oldNode), span0);
        _L2V_NodeBig_PushContig_Node(&u.tmpNode, L2V_NODE_TO_IDX(newNode), span1);
        span = Upd_PushNodeOut(&u.tmpNode, parent, 0, u.tmpNode.max);
        l2v_assert_eq(span, L2V_TREE_SIZE);
        
        // Replace root
        cu.u.nodeIdx = L2V_NODE_TO_IDX(parent);
        cu.span = 0;
        cu.hasSpanOF = 0; // To get rid of analyzer warning--not used in root ptrs
        L2V.Tree[u.incomingTree] = LPTR_PACK_NODE(cu);
    } else {
        // Get parent
        parent = u.atLevel[u.level-1];

        // Copy into tmpNode
        _L2V_NODE_ITERATE(parent, nOfs, nodeSize, cu, thisPtr) {
            if (cu.isNodePtr && (L2V_IDX_TO_NODE(cu.u.nodeIdx) == oldNode)) {
                // Copy old node
                _L2V_NodeBig_PushContig_Node(&u.tmpNode, cu.u.nodeIdx, span0);
                l2v_assert_ne(span0, 0);

                // Copy new node
                _L2V_NodeBig_PushContig_Node(&u.tmpNode, L2V_NODE_TO_IDX(newNode), span1);
                l2v_assert_ne(span1, 0);

                if ((span0+span1) != cu.span) {
                    WMR_PANIC("NAND index: span0: %d, span1: %d, +: %d, span: %d", span0, span1, span0+span1, cu.span);
                }
            } else {
                // Not involved, copy it out
                _L2V_NodeBig_PushContig(&u.tmpNode, &cu);
                l2v_assert_ne(cu.span, 0);
            }
        } _L2V_NODE_ITERATE_END(nOfs);

        if (u.tmpNode.max > L2V_MIN_CONTIG_PER_NODE) {
            // Split
            parent1 = _L2V_AllocNode();
            L2V.TreeNodes[u.incomingTree]++;
            half = (u.tmpNode.max>>1) + 1;
            parentspan0 = Upd_PushNodeOut(&u.tmpNode, parent, 0, half);
            l2v_assert_ne(parentspan0, 0);
            parentspan1 = Upd_PushNodeOut(&u.tmpNode, parent1, half, u.tmpNode.max);
            l2v_assert_ne(parentspan1, 0);

            // Ripple up
            u.level--;
            RippleUp(parent, parentspan0, parent1, parentspan1);
        } else {
            // Push
            Upd_PushNodeOut(&u.tmpNode, parent, 0, u.tmpNode.max);
        }
    }
}


static void Upd_ShareLeft()
{
    lNode_t *parent, *left;
    lPtr_t thisPtr;
    UInt32 nOfs, nodeSize, left_nodeIdx, left_nOfs, left_span, spanUp;
    BOOL32 left_isNodePtr;
    ContigUnpacked_t cu;

    // Initialize so the compiler doesn't complain
    cu.hasSpanOF = 0;
    cu.span = 0;
    left_nOfs = 0;
    left_nodeIdx = 0;
    left_span = 0;
    left_isNodePtr = 0;

    if (0 == u.level) {
        // Can't share if we're root...
        return;
    }

    // Get parent
    parent = u.atLevel[u.level-1];
    left = NULL;

    // Find me, then previous sibling
    _L2V_NODE_ITERATE(parent, nOfs, nodeSize, cu, thisPtr) {
        if (cu.isNodePtr && (L2V_IDX_TO_NODE(cu.u.nodeIdx) == u.node)) {
            goto foundIt;
        }
        left = L2V_IDX_TO_NODE(cu.u.nodeIdx);
        left_nOfs = nOfs;
        left_nodeIdx = cu.u.nodeIdx;
        left_span = cu.span;
        left_isNodePtr = cu.isNodePtr;
    } _L2V_NODE_ITERATE_END(nOfs);

    // Couldn't find ...???
    WMR_PANIC("NAND index update failed node identity search: level: %d, parent: %p\n", u.level, (void*)parent);

foundIt:
    if ((NULL == left) || !left_isNodePtr) {
        // We were the first pointed by parent, sorry, no easy left search...
        return;
    }

    // Great, push some in
    spanUp = Upd_PushLeft(left);

    // Down my span in the parent
    cu.span -= spanUp;
    *(lPtr_t*)(&parent->bytes[nOfs]) = LPTR_PACK_NODE(cu);
    *(UInt16*)&parent->bytes[nodeSize] = cu.span >> L2V_BITS_NODE_SPAN;
    l2v_assert_ne(cu.span, 0);

    // Update left's span in parent
    left_span += spanUp;
    cu.span = left_span;
    cu.u.nodeIdx = left_nodeIdx;
    *(lPtr_t*)(&parent->bytes[left_nOfs]) = LPTR_PACK_NODE(cu);
    *(UInt16*)&parent->bytes[nodeSize+sizeof(UInt16)] = left_span >> L2V_BITS_NODE_SPAN;
    l2v_assert_ne(left_span, 0);
}


static void Upd_ShareRight()
{
    lNode_t *parent, *right;
    lPtr_t thisPtr;
    UInt32 nOfs, nodeSize, my_nOfs, my_nodeIdx, my_span, spanUp;
    BOOL32 isNext;
    ContigUnpacked_t cu;

    // Initialize so the compiler doesn't complain
    my_span = 0;
    my_nodeIdx = 0;
    my_nOfs = 0;

    if (0 == u.level) {
        // Can't share if we're root...
        return;
    }

    // Get parent
    parent = u.atLevel[u.level-1];
    right = NULL;

    // Find me, then next sibling
    isNext = FALSE32;
    _L2V_NODE_ITERATE(parent, nOfs, nodeSize, cu, thisPtr) {
        if (isNext) {
            if (cu.isNodePtr) {
                right = L2V_IDX_TO_NODE(cu.u.nodeIdx);
            }
            goto foundIt;
        }
        if (cu.isNodePtr && (L2V_IDX_TO_NODE(cu.u.nodeIdx) == u.node)) {
            isNext = TRUE32;
            my_span = cu.span;
            my_nOfs = nOfs;
            my_nodeIdx = cu.u.nodeIdx;
        }
    } _L2V_NODE_ITERATE_END(nOfs);

    // Couldn't find right because we were the last one? 
    l2v_assert_eq(isNext, 1);
    return;

foundIt:
    if (NULL == right) {
        // No node* right next to us, sorry...
        return;
    }

    // Great, push some in
    spanUp = Upd_PushRight(right);

    // Update right's span in parent
    cu.span += spanUp;
    *(lPtr_t*)(&parent->bytes[nOfs]) = LPTR_PACK_NODE(cu);
    *(UInt16*)&parent->bytes[nodeSize] = cu.span >> L2V_BITS_NODE_SPAN;
    l2v_assert_ne(cu.span, 0);

    // Down my span in the parent
    my_span -= spanUp;
    cu.u.nodeIdx = my_nodeIdx;
    cu.span = my_span;
    *(lPtr_t*)(&parent->bytes[my_nOfs]) = LPTR_PACK_NODE(cu);
    *(UInt16*)&parent->bytes[nodeSize+sizeof(UInt16)] = cu.span >> L2V_BITS_NODE_SPAN;
    l2v_assert_ne(cu.span, 0);
}


static int Upd_PushLeft(lNode_t *left)
{
    lPtr_t thisPtr;
    UInt32 nOfs, nodeSize, spanUp, myEntrySize;
    ContigUnpacked_t cu;

    spanUp = 0;

    // Has room?
    _L2V_NODE_ITERATE(left, nOfs, nodeSize, cu, thisPtr) {
    } _L2V_NODE_ITERATE_END(nOfs);

    // Calculate size
    myEntrySize = _L2V_CONTIG_SIZE(u.tmpNode.cu[u.tmpNode.start]);

    // While we have space...
    while ((nOfs < (nodeSize-myEntrySize)) && ((u.tmpNode.max - u.tmpNode.start) > 1)) {
        // Copy it out
        l2v_assert_ne(u.tmpNode.cu[u.tmpNode.start].span, 0);
        l2v_assert_le(u.tmpNode.cu[u.tmpNode.start].span, L2V_TREE_SIZE);
        _L2V_NODE_PUSH_CONTIG(left, nOfs, nodeSize, u.tmpNode.cu[u.tmpNode.start]);

        // Move along
        spanUp += u.tmpNode.cu[u.tmpNode.start].span;
        u.tmpNode.start++;

        // Recalculate size
        myEntrySize = _L2V_CONTIG_SIZE(u.tmpNode.cu[u.tmpNode.start]);
    }

    // Fill rest of node with "empty" signature
    _L2V_NODE_FILL(left, nOfs, nodeSize);

    // And ripple up
    return spanUp;
}


static int Upd_PushRight(lNode_t *right)
{
    lPtr_t thisPtr;
    UInt32 i, numToCopy, spanUp, nOfs, nodeSize, orig_nOfs, orig_nodeSize, myEntrySize;
    lNodeBig_t localNode;
    BOOL32 didSome = FALSE32;
    ContigUnpacked_t cu;

    spanUp = 0;

    // Copy originals out and find out how much free space there is
    localNode.start = localNode.max = 0;
    _L2V_NODE_ITERATE(right, orig_nOfs, orig_nodeSize, cu, thisPtr) {
        localNode.cu[localNode.max] = cu;
        l2v_assert_ne(cu.span, 0);
        localNode.max++;
    } _L2V_NODE_ITERATE_END(orig_nOfs);

    // Calculate size
    myEntrySize = _L2V_CONTIG_SIZE(u.tmpNode.cu[u.tmpNode.max-1]);

    nOfs = 0;
    numToCopy = 0;

    // While we have space...
    while (((nOfs+orig_nOfs) < (orig_nodeSize-myEntrySize)) && ((u.tmpNode.max - u.tmpNode.start - numToCopy) > 1)) {
        didSome = TRUE32;

        // Mark it as copyable
        u.tmpNode.max--;
        numToCopy++;
        nOfs += myEntrySize;

        // Recalculate size
        myEntrySize = _L2V_CONTIG_SIZE(u.tmpNode.cu[u.tmpNode.max-1]);
    }

    if (!didSome) {
        // No room, sorry...
        return 0;
    }



    // Copy new ones
    nOfs = 0;
    nodeSize = L2V_NODE_SIZE;
    i = 0;
    while (numToCopy) {
        _L2V_NODE_PUSH_CONTIG(right, nOfs, nodeSize, u.tmpNode.cu[u.tmpNode.max+i]);
        l2v_assert_le(u.tmpNode.cu[u.tmpNode.max+i].span, L2V_TREE_SIZE);
        spanUp += u.tmpNode.cu[u.tmpNode.max+i].span;
        i++;
        numToCopy--;
    }

    // Copy originals
    for (localNode.start = 0; localNode.start < localNode.max; localNode.start++) {
        _L2V_NODE_PUSH_CONTIG(right, nOfs, nodeSize, localNode.cu[localNode.start]);
        l2v_assert_ne(localNode.cu[localNode.start].span, 0);
        l2v_assert_le(localNode.cu[localNode.start].span, L2V_TREE_SIZE);
    }

    // Fill rest of node with "empty" signature
    _L2V_NODE_FILL(right, nOfs, nodeSize);

    // And ripple up
    return spanUp;
}

#endif // ENABLE_L2V_TREE

