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


// Pointers to NAND
// VPN bits were originally defined seperately as superblock bits, page offset bits;
// combining them results in more flexibility.  10+3 was supposed to represent 8192 max superblocks;
// 12 represented 4096 max pages per superblock.
#define L2V_BITS_VPN (10+3+12)
#define L2V_MAX_SB (1 << L2V_BITS_SB)
#define L2V_BITS_NAND_SPAN (32 - 2 - L2V_BITS_VPN)


// Node size
#define L2V_CONTIG_ELEM_SMALL 4
#define L2V_CONTIG_ELEM_LARGE 6
#define L2V_MIN_CONTIG_PER_NODE (L2V_NODE_SIZE / L2V_CONTIG_ELEM_LARGE)
#define L2V_MAX_CONTIG_PER_NODE (L2V_NODE_SIZE / L2V_CONTIG_ELEM_SMALL)


// Memory usage
#define L2V_NODEPOOL_COUNT (L2V_NODEPOOL_MEM / L2V_NODE_SIZE)
#define L2V_BITS_NODEIDX 16
#define L2V_BITS_NODE_SPAN (32 - 2 - L2V_BITS_NODEIDX)


// Parameters
#define L2V_SUCKUP_CHILDREN_MAX 3
#define L2V_TRACK_VALID 0


// Tree size
#define L2V_TREE_SIZE (1 << L2V_TREE_BITS)
#define L2V_TREE_MASK (L2V_TREE_SIZE - 1)
#define L2V_MAX_TREE_DEPTH 32
#define L2V_MAX_TREE_DEPTH_UPD 36
#define L2V_TREE_DEPTH_REPACK 26


// Magic numbers
#define L2V_CONTIG_FREE_SIG 0xFFFFFFFF
// These represent token values; normally, when we move along a span, we add to vpn and
// subtract from span.  If it's a token--such as "missing" or "deallocated"--we should
// subtract from span, but not add to vpn, because it would change the meaning.
#define L2V_VPN_DEALLOC     ((1<<L2V_BITS_VPN)-0xfffe)
#define L2V_VPN_MISS        ((1<<L2V_BITS_VPN)-0xfffd)
#define L2V_VPN_SPECIAL L2V_VPN_DEALLOC


// Transforms
#define L2V_IDX_TO_NODE(idx) (&L2V.Pool.Node[idx])
#define L2V_NODE_TO_IDX(ptr) ((lNode_t*)(ptr) - &L2V.Pool.Node[0])

// Memory state
#define L2V_LowMem (L2V.Pool.FreeCount <= (L2V_MAX_TREE_DEPTH*4))


#endif // __L2V_DEFINES_H__

