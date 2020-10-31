/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef DART_REGS_H
#define DART_REGS_H

#define DART_TT_PGSIZE			4096
#define DART_TTE_SHIFT			3
#define DART_TTE_PGENTRIES		(DART_TT_PGSIZE >> DART_TTE_SHIFT)

#define DART_TTE_VALID			(1 << 0)
#define DART_TTE_TABLE			(1 << 1)

#define DART_NUM_TTBR			4
#define DART_NUM_SID			4

#define rDART_TLB_OP(base)		(*(volatile uint32_t *)((base) + 0x00))
#define rDART_CONFIG(base)		(*(volatile uint32_t *)((base) + 0x0c))
#define rDART_DIAG_CONFIG(base)		(*(volatile uint32_t *)((base) + 0x20))
#define rDART_DIAG_BOGUS_ACCESS(base)	(*(volatile uint32_t *)((base) + 0x24))
#define rDART_BYPASS_ADDR(base)		(*(volatile uint32_t *)((base) + 0x2c))
#define rDART_FETCH_REQ_CONFIG(base)	(*(volatile uint32_t *)((base) + 0x30))
#define rDART_TTBR(base, idx)		(*(volatile uint32_t *)((base) + 0x40 + 4 * (idx)))

#define DART_TLB_OP_FLUSH		(2)
#define DART_TLB_OP_BUSY		(1 << 3)
#define DART_TLB_OP_SID_MASK0		(1 << 8)
#define DART_TLB_OP_SID_MASK1		(1 << 9)
#define DART_TLB_OP_SID_MASK2		(1 << 10)
#define DART_TLB_OP_SID_MASK3		(1 << 11)

#define DART_DIAG_CONFIG_PTE_FETCH_PROMOTE_QOS	(1 << 8)

#define DART_FETCH_REQ_CONFIG_STE(x)		(((x) & 0xf) << 0)
#define DART_FETCH_REQ_CONFIG_PTE(x)		(((x) & 0xf) << 8)
#define DART_FETCH_REQ_CONFIG_PTE_PREFETCH(x)	(((x) & 0xf) << 16)

#define DART_TTBR_VALID			(1 << 31)
#define DART_TTBR_SHIFT			(12)

/*
 *	L1 Translation table (stored in registers)
 *
 *	There are DART_NUM_TTBR entries in the L1 table
 *	DART_NUM_TTBR entries of 1GB (2^30) of address space.
 */
#define DART_TT_L1_SIZE			0x0000000040000000ULL		/* size of area covered by a tte */
#define DART_TT_L1_OFFMASK		0x000000003fffffffULL		/* offset within an L2 entry */
#define DART_TT_L1_SHIFT			30
#define DART_TT_L1_INDEX_MASK		0x0000000070000000ULL		/* mask for getting index in L1 table from virtual address */

/*
 *	L2 Translation table
 *
 *	Each translation table is 4KB
 *	512 64-bit entries of 2MB (2^21) of address space.
 */

#define DART_TT_L2_SIZE			0x0000000000200000ULL		/* size of area covered by a tte */
#define DART_TT_L2_OFFMASK		0x00000000001fffffULL		/* offset within an L2 entry */
#define DART_TT_L2_SHIFT			21				/* page descriptor shift */
#define DART_TT_L2_INDEX_MASK		0x000000003fe00000ULL		/* mask for getting index in L2 table from virtual address */

/*
 *	L3 Translation table
 *
 *	Each translation table is 4KB
 *	512 64-bit entries of 4KB (2^12) of address space.
 */

#define DART_TT_L3_SIZE			0x0000000000001000ULL		/* size of area covered by a tte */
#define DART_TT_L3_OFFMASK		0x0000000000000fffULL		/* offset within L3 PTE */
#define DART_TT_L3_SHIFT			12				/* page descriptor shift */
#define DART_TT_L3_INDEX_MASK		0x00000000001ff000ULL		/* mask for page descriptor index */

/*
 * L3 Page table entries
 *
 * The following page table entry types are possible:
 *
 *	fault page entry
 *	63			      2	 0
 *	+------------------------------+--+
 *	|    ignored		       |00|
 *	+------------------------------+--+
 *
 *
 *  63 52 51  36 35                  12 11  8 7 6   2 1 0
 * +-----+------+----------------------+-----+-+-----+-+-+
 * | ign | zero | OutputAddress[35:12] | ign |W| ign |1|V|
 * +-----+------+----------------------+-----+-+-----+-+-+
 *
 * where:
 *	'W'		Write orotect bit
 *	'V'		Valid bit
 */

#define DART_PTE_SHIFT			3
#define DART_PTE_PGENTRIES		(DART_TT_PGSIZE >> DART_PTE_SHIFT)

#define DART_PTE_EMPTY			0x0000000000000000ULL		/* unasigned - invalid entry */

#define DART_PTE_TYPE_VALID		0x0000000000000003ULL		/* valid L3 entry: includes bit #1 (counterintuitively) */

#define DART_PTE_PAGE_MASK		0x0000FFFFFFFFF000ULL		/* mask for  4KB page */
#define DART_PTE_PAGE_SHIFT		12				/* page shift for 4KB page */

#define DART_PTE_WRPROT(enable)		((enable) ? (1 << 7) : 0)	/* write protect */

#endif // DART_REGS_H
