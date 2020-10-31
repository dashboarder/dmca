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

#ifndef __L2V_CONFIG_H__
#define __L2V_CONFIG_H__

// Defines the minium page size--ONLY used for easier extent (logical range) configuration
#define L2V_MINPAGESIZE_BITS (b_4 + KiB_bits)

// Defines the node size
#define L2V_CACHELINE  64
#define L2V_NODE_SIZE L2V_CACHELINE

// Defines the memory set aside for the node pool
#define L2V_NODEPOOL_MEM (MiB>>2)

// Defines the size of a logical tree
#define L2V_TREE_BITS 15

// Repack periodicity
#define L2V_REPACK_PERIOD 200


#endif // __L2V_CONFIG_H__

