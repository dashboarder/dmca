/*
 * Copyright (C) 2013-2014 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef __PLATFORM_RECONFIG_H
#define __PLATFORM_RECONFIG_H

#include <sys/types.h>
#include <platform/soc/hwregbase.h>
#include <platform/memmap.h>

#define rAOP_GLOBAL(_p)						(*(volatile uint32_t *)(AOP_BASE_ADDR + (_p)))
#define rRECONFIG_RAM_BASE(base, count) 			*(volatile uint32_t *)((base) + ((count) * 4))
#define rRECONFIG_RAM_BASE_BYTE(base, count, count_bytes)	*(volatile uint32_t *)((base) + ((count) * 4) + (count_bytes))

#define ALIGNMENT_MASK_1KB			~(0x3FF)
#define RECONFIG_NUMBER_OF_REGS(regs)		((((regs) - 1) & 0xF) << 2)
#define AOP_CFG_TABLE				(AOP_RECONFIG_REGION_BASE_ADDR)
#define WRITE_OFFSETS(addr, values)		(((addr) >> 2) & 0xFF) << (((values) % 4) * 8)
#define MAX_WRITE_REGS				16

// SRAM sequences
#define AOP_DDR_S2R_AOP_PRE_BASE_ADDR		(AOP_RECONFIG_REGION_BASE_ADDR + 0x100)
#define AOP_DDR_S2R_AOP_PRE_MAX_SIZE		0x200

#define AOP_DDR_S2R_AOP_POST_BASE_ADDR		(AOP_DDR_S2R_AOP_PRE_BASE_ADDR + AOP_DDR_S2R_AOP_PRE_MAX_SIZE)
#define AOP_DDR_S2R_AOP_POST_MAX_SIZE		0x100

#define S2R_AOP_AOP_DDR_PRE_BASE_ADDR		(AOP_DDR_S2R_AOP_POST_BASE_ADDR + AOP_DDR_S2R_AOP_POST_MAX_SIZE)
#define S2R_AOP_AOP_DDR_PRE_MAX_SIZE		0x100

#define S2R_AOP_AOP_DDR_POST_BASE_ADDR		(S2R_AOP_AOP_DDR_PRE_BASE_ADDR + S2R_AOP_AOP_DDR_PRE_MAX_SIZE)
#define S2R_AOP_AOP_DDR_POST_MAX_SIZE		0x3000

#define AOP_RECONFIG_REGION_USED		(AOP_RECONFIG_REGION_BASE_ADDR - S2R_AOP_AOP_DDR_PRE_BASE_ADDR + S2R_AOP_AOP_DDR_POST_MAX_SIZE)

#if AOP_RECONFIG_REGION_USED > AOP_RECONFIG_REGION_SIZE
#error "Reconfig engine programs overflow their SRAM allocation
#endif

// DRAM sequences
#define AWAKE_AOP_DDR_PRE_BASE_ADDR		(DRAM_CONFIG_SEQ_BASE)
#define AWAKE_AOP_DDR_PRE_MAX_SIZE		0x200

#define AWAKE_AOP_DDR_POST_BASE_ADDR		(AWAKE_AOP_DDR_PRE_BASE_ADDR + AWAKE_AOP_DDR_PRE_MAX_SIZE)
#define AWAKE_AOP_DDR_POST_MAX_SIZE		0x600

#define AOP_DDR_AWAKE_PRE_BASE_ADDR		(AWAKE_AOP_DDR_POST_BASE_ADDR + AWAKE_AOP_DDR_POST_MAX_SIZE)
#define AOP_DDR_AWAKE_PRE_MAX_SIZE		0x800

#define AOP_DDR_AWAKE_POST_BASE_ADDR		(AOP_DDR_AWAKE_PRE_BASE_ADDR + AOP_DDR_AWAKE_PRE_MAX_SIZE)
#define AOP_DDR_AWAKE_POST_MAX_SIZE		0x2400

#define DRAM_CONFIG_SEQ_USED 			(AOP_DDR_AWAKE_POST_BASE_ADDR + AOP_DDR_AWAKE_POST_MAX_SIZE - DRAM_CONFIG_SEQ_BASE)

#if DRAM_CONFIG_SEQ_USED > DRAM_CONFIG_SEQ_SIZE
#error "Reconfig engine programs overflow their DRAM allocation
#endif

#endif /* __PLATFORM_RECONFIG_H */
