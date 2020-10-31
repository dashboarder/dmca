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

#ifndef __L2V_DEFINES_H__
#define __L2V_DEFINES_H__

// Shortcuts for the config
#define b_1 0
#define b_2 1
#define b_4 2
#define b_8 3
#define b_16 4
#define b_32 5
#define b_64 6
#define b_128 7
#define b_256 8
#define b_512 9
#define KiB_bits 10
#define MiB_bits 20
#define GiB_bits 30
#define TiB_bits 40
#define KiB (1 << KiB_bits)
#define MiB (1 << MiB_bits)
#define GiB (1 << GiB_bits)
#define TiB (1 << TiB_bits)

#include "L2V_Config.h"


// Node size
#define L2V_CONTIG_ELEM_SMALL 4
#define L2V_CONTIG_ELEM_LARGE 6
#define L2V_MIN_CONTIG_PER_NODE (L2V_NODE_SIZE / L2V_CONTIG_ELEM_LARGE)
#define L2V_MAX_CONTIG_PER_NODE (L2V_NODE_SIZE / L2V_CONTIG_ELEM_SMALL)


// Memory usage
#define L2V_NODEPOOL_COUNT (L2V_NODEPOOL_MEM / L2V_NODE_SIZE)


// Parameters
#define L2V_SUCKUP_CHILDREN_MAX 3
#define L2V_TRACK_VALID 1


// Tree size
#define L2V_TREE_SIZE (1 << L2V_TREE_BITS)
#define L2V_TREE_MASK (L2V_TREE_SIZE - 1)
#define L2V_MAX_TREE_DEPTH 32
#define L2V_MAX_TREE_DEPTH_UPD 36
#define L2V_TREE_DEPTH_REPACK 26


// Magic numbers
#define L2V_CONTIG_FREE_SIG 0xFFFFFFFF
// VBA macros, to make quick read-only accessors
#define L2V_VBA_SPECIAL (L2V.vba.special+0)
#define L2V_VBA_DEALLOC (L2V.vba.dealloc+0)


// Transforms
#define L2V_IDX_TO_NODE(idx) (&L2V.Pool.Node[idx])
#define L2V_NODE_TO_IDX(ptr) ((lNode_t*)(ptr) - &L2V.Pool.Node[0])

// Memory state
#define L2V_LowishMem (L2V.Pool.FreeCount <= (L2V_MAX_TREE_DEPTH*16))
#define L2V_LowMem (L2V.Pool.FreeCount <= (L2V_MAX_TREE_DEPTH*8))
#define L2V_CriticalMem (L2V.Pool.FreeCount <= (L2V_MAX_TREE_DEPTH+1))
#define L2V_TimeToRepack (L2V.Pool.FreeCount <= (L2V_MAX_TREE_DEPTH << 1))

// Frag search
#define L2V_FIND_FRAGS_MAX 32
#define L2V_LIST_FRAGS_MAX 64


#endif // __L2V_DEFINES_H__

