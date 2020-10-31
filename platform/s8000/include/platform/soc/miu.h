/*
 * Copyright (C) 2012-2013 Apple Inc. All rights reserved.
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

#include <lib/devicetree.h>
#include <platform/soc/hwregbase.h>

#define	rSECUREROMCTRL_ROMADDRREMAP		(*(volatile u_int32_t *)(AOP_MINIPMGR_BASE_ADDR + 0xd0014))

#define	MCCLOCKREGION_TZ0BASEADDR(_n)		(AMCC_BASE_ADDR(_n) + 0x480)
#define	MCCLOCKREGION_TZ0ENDADDR(_n)		(AMCC_BASE_ADDR(_n) + 0x484)
#define	MCCLOCKREGION_TZ1BASEADDR(_n)		(AMCC_BASE_ADDR(_n) + 0x488)
#define	MCCLOCKREGION_TZ1ENDADDR(_n)		(AMCC_BASE_ADDR(_n) + 0x48c)
#define	MCCLOCKREGION_TZ0LOCK(_n)		(AMCC_BASE_ADDR(_n) + 0x490)
#define	MCCLOCKREGION_TZ1LOCK(_n)		(AMCC_BASE_ADDR(_n) + 0x494)

#define	rMCCLOCKREGION_TZ0BASEADDR(_n)		(*(volatile u_int32_t *)(MCCLOCKREGION_TZ0BASEADDR(_n)))
#define	rMCCLOCKREGION_TZ0ENDADDR(_n)		(*(volatile u_int32_t *)(MCCLOCKREGION_TZ0ENDADDR(_n)))
#define	rMCCLOCKREGION_TZ1BASEADDR(_n)		(*(volatile u_int32_t *)(MCCLOCKREGION_TZ1BASEADDR(_n)))
#define	rMCCLOCKREGION_TZ1ENDADDR(_n)		(*(volatile u_int32_t *)(MCCLOCKREGION_TZ1ENDADDR(_n)))
#define	rMCCLOCKREGION_TZ0LOCK(_n)		(*(volatile u_int32_t *)(MCCLOCKREGION_TZ0LOCK(_n)))
#define	rMCCLOCKREGION_TZ1LOCK(_n)		(*(volatile u_int32_t *)(MCCLOCKREGION_TZ1LOCK(_n)))

#if SUB_PLATFORM_S8000 || SUB_PLATFORM_S8003
#define CP_COM_DYN_CLK_GATING_CTRL		(0x000c)
#define rCP_COM_DYN_CLK_GATING_CTRL		(*(volatile u_int32_t *)(CP_COM_BASE_ADDR + CP_COM_DYN_CLK_GATING_CTRL))
#define CPCOM_INT_NORM_REQUEST			(0x0000)
#define rCP_COM_INT_NORM_REQUEST		(*(volatile u_int32_t *)(CP_COM_INT_BASE_ADDR + CPCOM_INT_NORM_REQUEST))
#define CPCOM_INT_NORM_MASK_SET			(0x0004)
#define rCP_COM_INT_NORM_MASK_SET		(*(volatile u_int32_t *)(CP_COM_INT_BASE_ADDR + CPCOM_INT_NORM_MASK_SET))
#define CPCOM_INT_NORM_MASK_CLR			(0x0008)
#define rCP_COM_INT_NORM_MASK_CLR		(*(volatile u_int32_t *)(CP_COM_INT_BASE_ADDR + CPCOM_INT_NORM_MASK_CLR))
#else
#define CPCOM_INT_NORM_REQUEST			(0x0000)
#define rCP0_COM_INT_NORM_REQUEST		(*(volatile u_int32_t *)(CP0_COM_INT_BASE_ADDR + CPCOM_INT_NORM_REQUEST))
#define rCP1_COM_INT_NORM_REQUEST		(*(volatile u_int32_t *)(CP1_COM_INT_BASE_ADDR + CPCOM_INT_NORM_REQUEST))
#define CPCOM_INT_NORM_MASK_SET			(0x0004)
#define rCP0_COM_INT_NORM_MASK_SET		(*(volatile u_int32_t *)(CP0_COM_INT_BASE_ADDR + CPCOM_INT_NORM_MASK_SET))
#define rCP1_COM_INT_NORM_MASK_SET		(*(volatile u_int32_t *)(CP1_COM_INT_BASE_ADDR + CPCOM_INT_NORM_MASK_SET))
#define CPCOM_INT_NORM_MASK_CLR			(0x0008)
#define rCP0_COM_INT_NORM_MASK_CLR		(*(volatile u_int32_t *)(CP0_COM_INT_BASE_ADDR + CPCOM_INT_NORM_MASK_CLR))
#define rCP1_COM_INT_NORM_MASK_CLR		(*(volatile u_int32_t *)(CP1_COM_INT_BASE_ADDR + CPCOM_INT_NORM_MASK_CLR))
#endif

// removed all the random registers used to set tunables
// we should grab these from the SPDS when we do Maui tunables

enum remap_select {
  REMAP_SRAM = 0,
  REMAP_SDRAM
};

extern void miu_select_remap(enum remap_select sel);
extern void miu_bypass_prep(void);
extern void miu_update_device_tree(DTNode *pmgr_node);

#endif /* ! __PLATFORM_SOC_MIU_H */
