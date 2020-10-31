/*
 * Copyright (C) 2012-2014 Apple Inc. All rights reserved.
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

#define	rSECUREROMCTRL_ROMADDRREMAP		(*(volatile u_int32_t *)(AMCC_BASE_ADDR + 0x0904))


#define	rMCCLOCKREGION_TZ0BASEADDR		(*(volatile u_int32_t *)(AMCC_BASE_ADDR + 0x480))
#define	rMCCLOCKREGION_TZ0ENDADDR		(*(volatile u_int32_t *)(AMCC_BASE_ADDR + 0x484))
#define	rMCCLOCKREGION_TZ1BASEADDR		(*(volatile u_int32_t *)(AMCC_BASE_ADDR + 0x488))
#define	rMCCLOCKREGION_TZ1ENDADDR		(*(volatile u_int32_t *)(AMCC_BASE_ADDR + 0x48c))
#define	rMCCLOCKREGION_TZ0LOCK			(*(volatile u_int32_t *)(AMCC_BASE_ADDR + 0x490))
#define	rMCCLOCKREGION_TZ1LOCK			(*(volatile u_int32_t *)(AMCC_BASE_ADDR + 0x494))


#define ASIO_CLK_CTRL				(0x100004)
#define rASIO_CLK_CTRL				(*(volatile u_int32_t *)(SB_BASE_ADDR + ASIO_CLK_CTRL))
#define ASIO_AKF_IDLE_CTRL			(0xe00024)
#define rASIO_AKF_IDLE_CTRL			(*(volatile u_int32_t *)(SB_BASE_ADDR + ASIO_AKF_IDLE_CTRL))
#define DYN_CLK_GATING				(0x49a0000)
#define rDYN_CLK_GATING				(*(volatile u_int32_t *)(SB_BASE_ADDR + DYN_CLK_GATING))
#define SIO_ASYNC_FIFO_SB_RD_RATE_LIMIT		(0x49c0000)
#define rSIO_ASYNC_FIFO_SB_RD_RATE_LIMIT	(*(volatile u_int32_t *)(SB_BASE_ADDR + SIO_ASYNC_FIFO_SB_RD_RATE_LIMIT))
#define SIO_ASYNC_FIFO_SB_WR_RATE_LIMIT		(0x49c0004)
#define rSIO_ASYNC_FIFO_SB_WR_RATE_LIMIT	(*(volatile u_int32_t *)(SB_BASE_ADDR + SIO_ASYNC_FIFO_SB_WR_RATE_LIMIT))
#define SIO_ASYNC_FIFO_SB_WGATHER		(0x49c0010)
#define rSIO_ASYNC_FIFO_SB_WGATHER		(*(volatile u_int32_t *)(SB_BASE_ADDR + SIO_ASYNC_FIFO_SB_WGATHER))
#define SIO_DAPASYNC_FIFO_SB_RD_RATE_LIMIT	(0x49e0000)
#define rSIO_DAPASYNC_FIFO_SB_RD_RATE_LIMIT	(*(volatile u_int32_t *)(SB_BASE_ADDR + SIO_DAPASYNC_FIFO_SB_RD_RATE_LIMIT))
#define SIO_DAPASYNC_FIFO_SB_WR_RATE_LIMIT	(0x49e0004)
#define rSIO_DAPASYNC_FIFO_SB_WR_RATE_LIMIT	(*(volatile u_int32_t *)(SB_BASE_ADDR + SIO_DAPASYNC_FIFO_SB_WR_RATE_LIMIT))
#define SIO_DAPASYNC_FIFO_SB_WGATHER		(0x49e0010)
#define rSIO_DAPASYNC_FIFO_SB_WGATHER		(*(volatile u_int32_t *)(SB_BASE_ADDR + SIO_DAPASYNC_FIFO_SB_WGATHER))
#define AIU_SB_ARBCFG				(0x5010000)
#define rAIU_SB_ARBCFG				(*(volatile u_int32_t *)(SB_BASE_ADDR + AIU_SB_ARBCFG))
#define AIU_SB_CPG_CNTL				(0x5010014)
#define rAIU_SB_CPG_CNTL			(*(volatile u_int32_t *)(SB_BASE_ADDR + AIU_SB_CPG_CNTL))


#define SOCBUSMUX_ARBCFG			(0x0000)
#define rSOCBUSMUX_ARBCFG			(*(volatile u_int32_t *)(SOC_BUSMUX_BASE_ADDR + SOCBUSMUX_ARBCFG))
#if SUB_PLATFORM_T7001
#define DWRRCFG_DISP0_RT			(0x0004)
#define rDWRRCFG_DISP0_RT			(*(volatile u_int32_t *)(SOC_BUSMUX_BASE_ADDR + DWRRCFG_DISP0_RT))
#endif
#define DWRRCFG_DISPMUX_BULK			(0x0008)
#define rDWRRCFG_DISPMUX_BULK			(*(volatile u_int32_t *)(SOC_BUSMUX_BASE_ADDR + DWRRCFG_DISPMUX_BULK))
#if SUB_PLATFORM_T7001
#define DWRRCFG_IOMUX_BULK			(0x001c)
#define rDWRRCFG_IOMUX_BULK			(*(volatile u_int32_t *)(SOC_BUSMUX_BASE_ADDR + DWRRCFG_IOMUX_BULK))
#endif
#define TLIMIT_LVL0_DISP0			(0x0024)
#define rTLIMIT_LVL0_DISP0			(*(volatile u_int32_t *)(SOC_BUSMUX_BASE_ADDR + TLIMIT_LVL0_DISP0))
#define TLIMIT_LVL1_DISP0			(0x0028)
#define rTLIMIT_LVL1_DISP0			(*(volatile u_int32_t *)(SOC_BUSMUX_BASE_ADDR + TLIMIT_LVL1_DISP0))
#define TLIMIT_LVL2_DISP0			(0x002c)
#define rTLIMIT_LVL2_DISP0			(*(volatile u_int32_t *)(SOC_BUSMUX_BASE_ADDR + TLIMIT_LVL2_DISP0))
#define TLIMIT_LVL3_DISP0			(0x0030)
#define rTLIMIT_LVL3_DISP0			(*(volatile u_int32_t *)(SOC_BUSMUX_BASE_ADDR + TLIMIT_LVL3_DISP0))
#define TLIMIT_LVL0_CAMERAMUX			(0x0034)
#define rTLIMIT_LVL0_CAMERAMUX			(*(volatile u_int32_t *)(SOC_BUSMUX_BASE_ADDR + TLIMIT_LVL0_CAMERAMUX))
#define TLIMIT_LVL1_CAMERAMUX			(0x0038)
#define rTLIMIT_LVL1_CAMERAMUX			(*(volatile u_int32_t *)(SOC_BUSMUX_BASE_ADDR + TLIMIT_LVL1_CAMERAMUX))
#define TLIMIT_LVL1_MEDIAMUX			(0x0048)
#define rTLIMIT_LVL1_MEDIAMUX			(*(volatile u_int32_t *)(SOC_BUSMUX_BASE_ADDR + TLIMIT_LVL1_MEDIAMUX))
#define TLIMIT_LVL2_MEDIAMUX			(0x004c)
#define rTLIMIT_LVL2_MEDIAMUX			(*(volatile u_int32_t *)(SOC_BUSMUX_BASE_ADDR + TLIMIT_LVL2_MEDIAMUX))
#define TLIMIT_LVL0_IOMUX			(0x0054)
#define rTLIMIT_LVL0_IOMUX			(*(volatile u_int32_t *)(SOC_BUSMUX_BASE_ADDR + TLIMIT_LVL0_IOMUX))
#define TLIMIT_LVL1_IOMUX			(0x0058)
#define rTLIMIT_LVL1_IOMUX			(*(volatile u_int32_t *)(SOC_BUSMUX_BASE_ADDR + TLIMIT_LVL1_IOMUX))
#define TLIMIT_LVL2_IOMUX			(0x005c)
#define rTLIMIT_LVL2_IOMUX			(*(volatile u_int32_t *)(SOC_BUSMUX_BASE_ADDR + TLIMIT_LVL2_IOMUX))
#define TLIMIT_LVL3_IOMUX			(0x0060)
#define rTLIMIT_LVL3_IOMUX			(*(volatile u_int32_t *)(SOC_BUSMUX_BASE_ADDR + TLIMIT_LVL3_IOMUX))
#define SOCBUSMUX_CPG_CNTL			(0x0088)
#define rSOCBUSMUX_CPG_CNTL			(*(volatile u_int32_t *)(SOC_BUSMUX_BASE_ADDR + SOCBUSMUX_CPG_CNTL))


#define IOBUSMUX_ARBCFG				(0x0000)
#define rIOBUSMUX_ARBCFG			(*(volatile u_int32_t *)(IOBUSMUX_BASE_ADDR + IOBUSMUX_ARBCFG))
#if SUB_PLATFORM_T7001
#define IOBUSMUX_REGS_DWRRCFG_DISP1_BULK	(0x0010)
#define rIOBUSMUX_REGS_DWRRCFG_DISP1_BULK	(*(volatile u_int32_t *)(IOBUSMUX_BASE_ADDR + IOBUSMUX_REGS_DWRRCFG_DISP1_BULK))
#endif
#if SUB_PLATFORM_T7000
#define IOBUSMUX_CPG_CNTL			(0x0024)
#elif SUB_PLATFORM_T7001
#define IOBUSMUX_CPG_CNTL			(0x002c)
#endif
#define rIOBUSMUX_CPG_CNTL			(*(volatile u_int32_t *)(IOBUSMUX_BASE_ADDR + IOBUSMUX_CPG_CNTL))


#define SWITCH_FAB_AMAP_LOCK			(0x0000)
#define rSWITCH_FAB_AMAP_LOCK			(*(volatile u_int32_t *)(SWTCH_FAB_BASE_ADDR + SWITCH_FAB_AMAP_LOCK))
#define SWITCH_FAB_ARBCFG			(0x0008)
#define	rSWITCH_FAB_ARBCFG			(*(volatile u_int32_t *)(SWTCH_FAB_BASE_ADDR + SWITCH_FAB_ARBCFG))
#if SUB_PLATFORM_T7000
#define	SWITCH_FAB_PIOLIMIT			(0x0028)
#elif SUB_PLATFORM_T7001
#define	SWITCH_FAB_PIOLIMIT			(0x002c)
#endif
#define	rSWITCH_FAB_PIOLIMIT			(*(volatile u_int32_t *)(SWTCH_FAB_BASE_ADDR + SWITCH_FAB_PIOLIMIT))
#if SUB_PLATFORM_T7000
#define	SWITCH_FAB_CPG_CNTL			(0x0070)
#elif SUB_PLATFORM_T7001
#define	SWITCH_FAB_CPG_CNTL			(0x008c)
#endif
#define rSWITCH_FAB_CPG_CNTL			(*(volatile u_int32_t *)(SWTCH_FAB_BASE_ADDR + SWITCH_FAB_CPG_CNTL))


#define CP_DYN_CLK_GATING_CTRL			(0x000c)
#define rCP_DYN_CLK_GATING_CTRL			(*(volatile u_int32_t *)(CP_COM_BASE_ADDR + CP_DYN_CLK_GATING_CTRL))

#define CPCOM_INT_NORM_REQUEST			(0x0000)
#define rCP_COM_INT_NORM_REQUEST		(*(volatile u_int32_t *)(CP_COM_INT_BASE_ADDR + CPCOM_INT_NORM_REQUEST))
#define CPCOM_INT_NORM_MASK_SET			(0x0004)
#define rCP_COM_INT_NORM_MASK_SET		(*(volatile u_int32_t *)(CP_COM_INT_BASE_ADDR + CPCOM_INT_NORM_MASK_SET))
#define CPCOM_INT_NORM_MASK_CLR			(0x0008)
#define rCP_COM_INT_NORM_MASK_CLR		(*(volatile u_int32_t *)(CP_COM_INT_BASE_ADDR + CPCOM_INT_NORM_MASK_CLR))

#define LIO_MEMCACHE_DATASETID_OVERRIDE		(0x001000)

enum remap_select {
  REMAP_SRAM = 0,
  REMAP_SDRAM
};

extern void miu_select_remap(enum remap_select sel);
extern void miu_bypass_prep(void);
extern void miu_update_device_tree(DTNode *pmgr_node);

#endif /* ! __PLATFORM_SOC_MIU_H */
