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

#ifndef __L2V_TYPES_H__
#define __L2V_TYPES_H__


#include "WMROAM.h"
#include "L2V_Defines.h"
#include "L2V_Assert.h"


// Pointer type: can point to NAND, or to a subtree
// Bits: 0=isNodePtr, 1=hasSpanOF.
//  isNodePtr=0: vpn:L2V_BITS_VPN, span:L2V_BITS_NAND_SPAN
//  isNodePtr=1: nodeIdx:L2V_BITS_NODEIDX, span:L2V_BITS_NODE_SPAN
//  hasSpanOF: fetch 2 bytes from the right end of the node

typedef UInt32 lPtr_t;
#define LPTR_IS_NODEPTR(_lptr) ((_lptr) & 1)
#define LPTR_HASSPANOF(_lptr) (((_lptr) & 2) >> 1)
#define LPTR_PACK_NAND(_cu) ( \
    (((_cu).hasSpanOF & 1) << 1) | \
    (((_cu).u.n.vpn & (((1 <<L2V_BITS_VPN)-1))) << 2) | \
    (((_cu).span & ((1<<L2V_BITS_NAND_SPAN)-1)) << (2+L2V_BITS_VPN)) \
    )
#define LPTR_PACK_NODE(_cu) ( \
    1 | \
    (((_cu).hasSpanOF & 1) << 1) | \
    (((_cu).u.nodeIdx & ((1 << L2V_BITS_NODEIDX)-1)) << 2) | \
    (((_cu).span & (((1 <<L2V_BITS_NODE_SPAN)-1))) << (2+L2V_BITS_NODEIDX)) \
    )
#define LPTR_UNPACK_NAND(_lptr, _cu) do { \
        (_cu).isNodePtr = 0; \
        (_cu).hasSpanOF = ((_lptr) >> 1) & 1; \
        (_cu).u.n.vpn = ((_lptr) >> 2) & ((1 << L2V_BITS_VPN)-1); \
        (_cu).span = ((_lptr) >> (2+L2V_BITS_VPN)) & ((1 << L2V_BITS_NAND_SPAN)-1); \
    } while (0)
#define LPTR_UNPACK_NODE(_lptr, _cu) do { \
        (_cu).isNodePtr = 1; \
        (_cu).hasSpanOF = ((_lptr) >> 1) & 1; \
        (_cu).u.nodeIdx = ((_lptr) >> 2) & ((1 << L2V_BITS_NODEIDX)-1); \
        (_cu).span = ((_lptr) >> (2+L2V_BITS_NODEIDX)) & ((1 << L2V_BITS_NODE_SPAN)-1); \
    } while (0)
#define LPTR_GET_NODEIDX(_lptr) (((_lptr) >> 2) & ((1 << L2V_BITS_NODEIDX)-1))
#define LPTR_UNPACK(_lptr, _cu) do { \
        if LPTR_IS_NODEPTR(_lptr) { \
            LPTR_UNPACK_NODE(_lptr, _cu); \
        } else { \
            LPTR_UNPACK_NAND(_lptr, _cu); \
        } \
    } while (0)

typedef struct {
    UInt32 isNodePtr; // Values: 0, 1 (true)... must not be any other value
    UInt32 hasSpanOF; // Values: 0, 1 (true)... ditto
    union {
        struct {
            UInt32 vpn;
        } n;
        UInt32 nodeIdx;
    } u;
    UInt32 span;
} ContigUnpacked_t;


// Node type: union for dynamic and fixed node types (and FreePool linkage)
typedef union _lNode_t {
    union _lNode_t *nextFreePtr;

    UInt8 bytes[L2V_NODE_SIZE];
} lNode_t;
CASSERT(L2V_NODE_SIZE == sizeof(lNode_t), lNode_t_size_matches);

#define L2V_LNODEBIG_MAX (L2V_MAX_CONTIG_PER_NODE+(L2V_SUCKUP_CHILDREN_MAX*2)+5)
typedef struct {
    ContigUnpacked_t cu[L2V_LNODEBIG_MAX];
    UInt32 start;
    UInt32 max;
} lNodeBig_t;


typedef struct {
    lNode_t  *Node; // Node pool allocated with malloc()
    lNode_t *FreePtr;
    UInt32 FreeCount;
} FTLight_NodePool_t;


typedef struct {
    // Logical space
    UInt32  numRoots;
    lPtr_t   *Tree;
    UInt32 *TreeNodes;
    UInt32 *UpdatesSinceRepack;
    UInt32  RepackCounter;
    UInt32  treeVersion;

    // Physical space
    UInt32 numSBs;
    UInt32 pagesPerSB;
#if L2V_TRACK_VALID!=0
    UInt32 Valid[L2V_MAX_SB];
#endif

    // Memory
    FTLight_NodePool_t Pool;
} L2V_t;

typedef struct {
    // Input:
    UInt32 lba;

    // Output:
    UInt32 fromStart; // blocks from start of treeSpan
    UInt32 vpn;
    UInt32 span;

    // Internal state:
    lNode_t *last_node;
    UInt32 next_lba;
    UInt32 next_nOfs;
    UInt32 nodeSize;
    UInt32 level;
    UInt32 treeVersion;
} L2V_SearchCtx_t;


// Helper functions

extern BOOL32 _L2V_NodeBig_PushContig(lNodeBig_t *big, ContigUnpacked_t *cu);  // True: compressed with left
extern BOOL32 _L2V_NodeBig_PushContig_Node(lNodeBig_t *big, UInt32 nodeIdx, UInt32 span); // True: compress with left (always false)
extern BOOL32 _L2V_NodeBig_PushContig_Nand(lNodeBig_t *big, UInt32 vpn, UInt32 span);  // True: compressed with left
#define _L2V_NodeBig_WillCompress(_big, _vpn, _span) \
    ((_big.max > 0) && (!_big.cu[_big.max-1].isNodePtr) && \
        (((_vpn < L2V_VPN_SPECIAL) && ((_big.cu[_big.max-1].u.n.vpn+_big.cu[_big.max-1].span) == _vpn)) \
        || ((_vpn >= L2V_VPN_SPECIAL) && (_big.cu[_big.max-1].u.n.vpn == _vpn))))


#endif // __L2V_TYPES_H__

