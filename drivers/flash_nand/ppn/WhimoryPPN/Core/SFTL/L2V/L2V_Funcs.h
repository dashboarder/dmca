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

#ifndef __L2V_FUNCS_H__
#define __L2V_FUNCS_H__


#include "L2V_Types.h"


extern BOOL32 L2V_Init(UInt32 numLbas, UInt32 max_sb, UInt32 vbas_per_sb);
extern void L2V_Nuke(void);
extern void L2V_Search_Init(L2V_SearchCtx_t *c);
extern void L2V_Search(L2V_SearchCtx_t *c);
extern void L2V_Update(UInt32 lba, UInt32 size, UInt32 vba);
extern void L2V_Repack(UInt32 tree);


// Macros

#define _L2V_NODE_ITERATE_RESUME(_node, _nodeByteOfs, _nodeSize, _cu, _destPtr) \
    while ((_nodeByteOfs) <= ((_nodeSize)-sizeof(lPtr_t))) { \
        (_destPtr) = *(lPtr_t*)(&(_node)->bytes[_nodeByteOfs]); \
        if (L2V_CONTIG_FREE_SIG == (_destPtr)) { \
            break; \
        } \
        if (LPTR_IS_NODEPTR(_destPtr)) { \
            LPTR_UNPACK_NODE(_destPtr, _cu); \
        } else { \
            LPTR_UNPACK_NAND(_destPtr, _cu); \
        } \
        if ((_cu).hasSpanOF) { \
            (_nodeSize) -= sizeof(UInt16); \
            (_cu).span += *(UInt16*)&(_node)->bytes[_nodeSize] << (LPTR_IS_NODEPTR(_destPtr) ? L2V.bits.node_span : L2V.bits.nand_span); \
            l2v_assert_le(_nodeByteOfs, ((_nodeSize)-sizeof(lPtr_t))); \
        } \
        (_cu).span += 1; \
        l2v_assert_ne((_cu).span, 0);

#define _L2V_NODE_ITERATE(_node, _nodeByteOfs, _nodeSize, _cu, _destPtr) \
    (_nodeByteOfs) = 0; \
    (_nodeSize) = L2V_NODE_SIZE; \
    _L2V_NODE_ITERATE_RESUME(_node, _nodeByteOfs, _nodeSize, _cu, _destPtr)


#define _L2V_NODE_ITERATE_NEXT(_nodeByteOfs) \
        (_nodeByteOfs) += sizeof(lPtr_t);

#define _L2V_NODE_ITERATE_END_NOP }

#define _L2V_NODE_ITERATE_END(_nodeByteOfs) \
        (_nodeByteOfs) += sizeof(lPtr_t); \
    }

#define _L2V_NODE_PUSH_CONTIG_NODE(_node, _nodeByteOfs, _nodeSize, _cu) \
    { \
        l2v_assert_ne((_cu).span, 0); \
        (_cu).hasSpanOF = 1; \
        *(lPtr_t*)(&(_node)->bytes[_nodeByteOfs]) = LPTR_PACK_NODE(_cu); \
        (_nodeByteOfs) += sizeof(lPtr_t); \
        (_nodeSize) -= sizeof(UInt16); \
        *(UInt16*)&(_node)->bytes[_nodeSize] = ((_cu).span-1) >> L2V.bits.node_span; \
    }
#define _L2V_NODE_PUSH_CONTIG_NAND(_node, _nodeByteOfs, _nodeSize, _cu) \
    { \
        l2v_assert_ne((_cu).span, 0); \
        (_cu).hasSpanOF = 0; \
        if ((_cu).span > (1UL << L2V.bits.nand_span)) { \
            (_cu).hasSpanOF = 1; \
            (_nodeSize) -= sizeof(UInt16); \
            *(UInt16*)&(_node)->bytes[_nodeSize] = ((_cu).span-1) >> L2V.bits.nand_span; \
        } \
        *(lPtr_t*)(&(_node)->bytes[_nodeByteOfs]) = LPTR_PACK_NAND(_cu); \
        (_nodeByteOfs) += sizeof(lPtr_t); \
    }
#define _L2V_NODE_PUSH_CONTIG(_node, _nodeByteOfs, _nodeSize, _cu) \
    do { \
        if ((_cu).isNodePtr) { \
            _L2V_NODE_PUSH_CONTIG_NODE(_node, _nodeByteOfs, _nodeSize, _cu); \
        } else { \
            _L2V_NODE_PUSH_CONTIG_NAND(_node, _nodeByteOfs, _nodeSize, _cu); \
        } \
    } while (0)
#define _L2V_CONTIG_SIZE(_cu) (sizeof(lPtr_t) + (((_cu).isNodePtr || ((_cu).span > (1UL << L2V.bits.nand_span))) ? sizeof(UInt16) : 0))

#define _L2V_NODE_FILL(_node, _nodeByteOfs, _nodeSize) \
    do { \
        WMR_MEMSET(&(_node)->bytes[_nodeByteOfs], 0xFF, (_nodeSize) - (_nodeByteOfs)); \
    } while (0)


#define min(a, b) ((a) < (b) ? (a) : (b))




#endif // __L2V_FUNCS_H__

