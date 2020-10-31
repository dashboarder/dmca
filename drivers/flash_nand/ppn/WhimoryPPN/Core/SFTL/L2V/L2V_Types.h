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
//  isNodePtr=0: vba:L2V.bits.vba, span:L2V.bits.nand_span
//  isNodePtr=1: nodeIdx:L2V.bits.nodeIdx, span:L2V.bits.node_span
//  hasSpanOF: fetch 2 bytes from the right end of the node

typedef UInt32 lPtr_t;
#define LPTR_IS_NODEPTR(_lptr) ((_lptr) & 1)
#define LPTR_HASSPANOF(_lptr) (((_lptr) & 2) >> 1)
#define LPTR_PACK_NAND(_cu) ( \
    (((_cu).hasSpanOF & 1) << 1) | \
    (((_cu).u.vba & (((1UL<<L2V.bits.vba)-1))) << 2) | \
    ((((_cu).span-1) & ((1UL<<L2V.bits.nand_span)-1)) << (2+L2V.bits.vba)) \
    )
#define LPTR_PACK_NODE(_cu) ( \
    1 | \
    (((_cu).hasSpanOF & 1) << 1) | \
    (((_cu).u.nodeIdx & ((1UL<<L2V.bits.nodeIdx)-1)) << 2) | \
    ((((_cu).span-1) & (((1UL<<L2V.bits.node_span)-1))) << (2+L2V.bits.nodeIdx)) \
    )
#define LPTR_UNPACK_NAND(_lptr, _cu) do { \
        (_cu).isNodePtr = 0; \
        (_cu).hasSpanOF = ((_lptr) >> 1) & 1; \
        (_cu).u.vba = ((_lptr) >> 2) & ((1UL<<L2V.bits.vba)-1); \
        (_cu).span = ((_lptr) >> (2+L2V.bits.vba)) & ((1UL<<L2V.bits.nand_span)-1); \
    } while (0)
#define LPTR_UNPACK_NODE(_lptr, _cu) do { \
        (_cu).isNodePtr = 1; \
        (_cu).hasSpanOF = ((_lptr) >> 1) & 1; \
        (_cu).u.nodeIdx = ((_lptr) >> 2) & ((1UL<<L2V.bits.nodeIdx)-1); \
        (_cu).span = ((_lptr) >> (2+L2V.bits.nodeIdx)) & ((1UL<<L2V.bits.node_span)-1); \
    } while (0)
#define LPTR_GET_NODEIDX(_lptr) (((_lptr) >> 2) & ((1UL<<L2V.bits.nodeIdx)-1))
#define LPTR_UNPACK(_lptr, _cu) do { \
        if LPTR_IS_NODEPTR(_lptr) { \
            LPTR_UNPACK_NODE(_lptr, _cu); \
        } else { \
            LPTR_UNPACK_NAND(_lptr, _cu); \
        } \
    } while (0)
#define LPTR_PACK_NAND_SPANOF(_span) ((_span-1) >> L2V.bits.nand_span)
#define LPTR_PACK_NODE_SPANOF(_span) ((_span-1) >> L2V.bits.node_span)

typedef UInt16 RootContig_t;
#define ROOT_CONTIG_DEALLOC (0xFFFF)
#define ROOT_CONTIG_GET_NODEIDX(rc) (rc)
#define ROOT_CONTIG_PACK(_cu) ((_cu).isNodePtr ? (_cu).u.nodeIdx : ROOT_CONTIG_DEALLOC) // might have to type cast to UInt16
#define ROOT_CONTIG_UNPACK(_rc, _cu) do { \
        (_cu).hasSpanOF = 0; /* dont care */ \
        if (ROOT_CONTIG_DEALLOC == (_rc)) \
        { \
            (_cu).u.vba = L2V_VBA_DEALLOC; \
            (_cu).isNodePtr = 0; \
        } \
        else \
        { \
            (_cu).u.nodeIdx = (_rc); \
            (_cu).isNodePtr = 1; \
        } \
        (_cu).span = L2V_TREE_SIZE; \
    }while(0);

typedef struct {
    UInt32 isNodePtr; // Values: 0, 1 (true)... must not be any other value
    UInt32 hasSpanOF; // Values: 0, 1 (true)... ditto
    union {
        UInt32 vba;
        UInt32 nodeIdx;
    } u;
    UInt32 span;
} ContigUnpacked_t;


// Node type: union for dynamic and fixed node types (and FreePool linkage)
typedef union _lNode_t {
    union _lNode_t *nextFreePtr;

    UInt8 bytes[L2V_NODE_SIZE];
} lNode_t;
WMR_CASSERT(L2V_NODE_SIZE == sizeof(lNode_t), lNode_t_size_matches);

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

typedef struct{
    lNodeBig_t *tempNodeEntries;
    UInt8 *isEntryAvailable;
    UInt32 poolSize;
}  tempNodePool_t;

typedef struct {
    RootContig_t rootContig;
    UInt16 numNodes;
    UInt16 updatesSinceRepack; // 2 bytes because upper bounded by L2V_REPACK_PERIOD(which is 200)
} Root_t;

typedef struct {
    UInt32 node_span;
    UInt32 nodeIdx;
    UInt32 nand_span;
    UInt32 vba;
} L2V_Bits_t;

typedef struct {
    // Special vbas
    UInt32 special;
    UInt32 dealloc;
} L2V_vba_t;


typedef struct {
    // Logical space
    UInt32  numRoots;
    Root_t *Root;
    UInt32  RepackCounter;
    UInt32  treeVersion;

    // Physical space
    UInt32 max_sb;
    UInt32 vbas_per_sb;

    // Bit arrangements
    L2V_Bits_t bits;
    // Special VBAs
    L2V_vba_t vba;

    // Memory
    FTLight_NodePool_t Pool;
} L2V_t;

typedef struct {
    // Input:
    UInt32 lba;

    // Output:
    UInt32 fromStart; // blocks from start of treeSpan
    UInt32 vba;
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
extern BOOL32 _L2V_NodeBig_PushContig_Nand(lNodeBig_t *big, UInt32 vba, UInt32 span);  // True: compressed with left
#define _L2V_NodeBig_WillCompress(_big, _vba, _span) \
    ((_big.max > 0) && (!_big.cu[_big.max-1].isNodePtr) && \
        (((_vba < L2V_VBA_SPECIAL) && ((_big.cu[_big.max-1].u.vba+_big.cu[_big.max-1].span) == _vba)) \
        || ((_vba >= L2V_VBA_SPECIAL) && (_big.cu[_big.max-1].u.vba == _vba))))


#endif // __L2V_TYPES_H__

