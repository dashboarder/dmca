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
#include "L2V_Print.h"
#include "L2V_Funcs.h"
#include "L2V.h"
#include "L2V_Types.h"


#define PRINT_MAXPATH 0


// Prototypes

static int L2V_GetUsage(lNode_t *node, UInt32 *used, UInt32 *total, UInt32 *nodes, UInt32 totalSpan, BOOL32 isMax);


// Functions

void L2V_PrintPtr(lPtr_t lptr)
{
    ContigUnpacked_t cu;
    LPTR_UNPACK(lptr, cu);
    L2V_PrintCU(&cu);
}

void L2V_PrintCU(ContigUnpacked_t *cu)
{
    if (cu->isNodePtr)
        printf("[Node]");
    else
        printf("[NAND]");

    if (cu->hasSpanOF)
        printf("o");
    else
        printf("-");

    if (cu->isNodePtr) {
        printf("[idx:%d][span:%d]", cu->u.nodeIdx, cu->span);
    } else {
        printf("[vpn:%d][span:%d]", cu->u.n.vpn, cu->span);
    }

    printf("\n");
}


void L2V_PrintNode(lNode_t *node)
{
    UInt32 nOfs, nodeSize;
    lPtr_t thisPtr;
    ContigUnpacked_t cu;

    _L2V_NODE_ITERATE(node, nOfs, nodeSize, cu, thisPtr) {
        L2V_PrintCU(&cu);
    } _L2V_NODE_ITERATE_END(nOfs);
}


void L2V_PrintUsage(UInt32 tree)
{
    UInt32 used, total, nodes, maxdepth;

    ContigUnpacked_t cu;

    used = total = nodes = maxdepth = 0;
    printf("Max depth path: ");
    printf("digraph G {\n");
    LPTR_UNPACK(L2V.Tree[tree], cu);
    if (cu.isNodePtr) {
        maxdepth = L2V_GetUsage(L2V_IDX_TO_NODE(cu.u.nodeIdx), &used, &total, &nodes, L2V_TREE_SIZE, 1);
    }
    printf("}\n");
    printf("\nUsage tree %d: %.2f, nodes: %d, maxdepth: %d\n", tree, (float)used/(float)total, nodes, maxdepth);
    printf("Nodes calc: %d   tally: %d\n\n", nodes, L2V.TreeNodes[tree]);
    if (nodes != L2V.TreeNodes[tree]) while(1);//DP
    //l2v_assert_eq(nodes, L2V_NODEPOOL_COUNT - L2V.Pool.FreeCount); // Only valid when using 1 tree
}

static int L2V_GetUsage(lNode_t *node, UInt32 *used, UInt32 *total, UInt32 *nodes, UInt32 totalSpan, BOOL32 isMax)
{
    UInt32 nOfs, nodeSize; // Byte offset within the node
    UInt32 sumSpan;
    lPtr_t thisPtr, maxNode;
    UInt32 depth, maxDepth, maxSpan;
    ContigUnpacked_t cu;

    // Walk through
    (*nodes)++;
    maxDepth = 0;
    sumSpan = 0;

    // Iterate through the node,
    L2V_PrintNode(node);
    _L2V_NODE_ITERATE(node, nOfs, nodeSize, cu, thisPtr) {
        if (cu.isNodePtr) {
            printf("%d -> %d;\n", (int)L2V_NODE_TO_IDX(node), cu.u.nodeIdx); // DOT output
            l2v_assert_ne(cu.u.nodeIdx, 65535);
            l2v_assert_ne(L2V_IDX_TO_NODE(cu.u.nodeIdx), node);
            l2v_assert_ne(cu.span, 0);
            depth = L2V_GetUsage(L2V_IDX_TO_NODE(cu.u.nodeIdx), used, total, nodes, cu.span, 0);
            if (depth > maxDepth) {
                maxDepth = depth;
                maxNode = thisPtr;
                maxSpan = cu.span;
            }
        }
        sumSpan += cu.span;
        l2v_assert_le(cu.span, 32768);
    } _L2V_NODE_ITERATE_END(nOfs);
    *used += nOfs;
    *total += sizeof(lNode_t);
    printf("%d [label=\"us:%.2f\\nsp:%.2f\",shape=box,style=filled,color=\"1.0 %.2f 1.0\"];\n", (int)L2V_NODE_TO_IDX(node), (float)nOfs/sizeof(lNode_t), (float)totalSpan/L2V_TREE_SIZE, (float)nOfs/sizeof(lNode_t)); // DOT output

    if (sumSpan != totalSpan) {
        printf("Sum: %d, total: %d\n", sumSpan, totalSpan);
        L2V_PrintNode(node);
    }
    l2v_assert_eq(sumSpan, totalSpan);
    l2v_assert_ne(nOfs, 0);

#if PRINT_MAXPATH
    if (maxDepth && isMax) {
        printf("<-%d", maxNode.node.nodeIdx);
        L2V_GetUsage(L2V_LPTR_TO_NODE(maxNode), &nOfs, &nOfs, &nOfs, maxSpan, 1);
    }
#endif

    return maxDepth+1;
}

#endif // ENABLE_L2V_TREE

