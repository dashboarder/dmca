/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#ifndef __PLATFORM_SOC_MIU_H
#define __PLATFORM_SOC_MIU_H

#include <platform/soc/hwregbase.h>

#define rAHB_MST0			(*(volatile u_int32_t *)(AHB_BASE_ADDR + 0x004))
#define rAHB2AXIID_CON			(*(volatile u_int32_t *)(AHB_BASE_ADDR + 0x040))
#define rAHB2AXIID0			(*(volatile u_int32_t *)(AHB_BASE_ADDR + 0x044))
#define rAHB2AXIID1			(*(volatile u_int32_t *)(AHB_BASE_ADDR + 0x048))
#define rAHB2AXIID2			(*(volatile u_int32_t *)(AHB_BASE_ADDR + 0x04C))
#define rAHB2AXIID3			(*(volatile u_int32_t *)(AHB_BASE_ADDR + 0x050))
#define rAHB2AXIID4			(*(volatile u_int32_t *)(AHB_BASE_ADDR + 0x054))
#define rLEVEL1SEL			(*(volatile u_int32_t *)(AHB_BASE_ADDR + 0x090))

#define rPWRDWN_BUS			(*(volatile u_int32_t *)(BUS_BASE_ADDR + 0x008))
#define rREMAP				(*(volatile u_int32_t *)(BUS_BASE_ADDR + 0x00C))
#define rDDR1				(*(volatile u_int32_t *)(BUS_BASE_ADDR + 0x010))
#define rMFC_ARF_CON			(*(volatile u_int32_t *)(BUS_BASE_ADDR + 0x3000))
#define rMFC_ARF_BASE_ADDR		(*(volatile u_int32_t *)(BUS_BASE_ADDR + 0x3004))
#define rMFC_ARF_ADDR_MASK		(*(volatile u_int32_t *)(BUS_BASE_ADDR + 0x3008))

enum remap_select {
  REMAP_SRAM = 0,
  REMAP_SDRAM
};

extern void miu_select_remap(enum remap_select sel);

#endif /* ! __PLATFORM_SOC_MIU_H */
