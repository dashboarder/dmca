/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __APPLE_AOP_DMA_H
#define __APPLE_AOP_DMA_H

#include <platform/soc/hwregbase.h>
#include <soc/t8002/a0/aop_pl080.h>

/* AOP PL080 registers */

#define rAOP_PL080_INTTCCLEAR			(*(volatile uint32_t *)(AOP_PL080_BASE_ADDR + AOP_PL080_BLK_DMAC_INTTCCLEAR_OFFSET))
#define rAOP_PL080_INTERRCLR			(*(volatile uint32_t *)(AOP_PL080_BASE_ADDR + AOP_PL080_BLK_DMAC_INTERRCLR_OFFSET))
#define rAOP_PL080_CONFIGURATION		(*(volatile uint32_t *)(AOP_PL080_BASE_ADDR + AOP_PL080_BLK_DMAC_CONFIGURATION_OFFSET))
#define rAOP_PL080_SYNC					(*(volatile uint32_t *)(AOP_PL080_BASE_ADDR + AOP_PL080_BLK_DMAC_SYNC_OFFSET))
#define rAOP_PL080_C0_SRCADDR			(*(volatile uint32_t *)(AOP_PL080_BASE_ADDR + AOP_PL080_BLK_DMAC_C0_SRCADDR_OFFSET))
#define rAOP_PL080_C0_DESTADDR			(*(volatile uint32_t *)(AOP_PL080_BASE_ADDR + AOP_PL080_BLK_DMAC_C0_DESTADDR_OFFSET))
#define rAOP_PL080_C0_LLI				(*(volatile uint32_t *)(AOP_PL080_BASE_ADDR + AOP_PL080_BLK_DMAC_C0_LLI_OFFSET))
#define rAOP_PL080_C0_CONTROL			(*(volatile uint32_t *)(AOP_PL080_BASE_ADDR + AOP_PL080_BLK_DMAC_C0_CONTROL_OFFSET))
#define rAOP_PL080_C0_CONFIGURATION		(*(volatile uint32_t *)(AOP_PL080_BASE_ADDR + AOP_PL080_BLK_DMAC_C0_CONFIGURATION_OFFSET))

#endif /* __APPLE_AOP_DMA_H */