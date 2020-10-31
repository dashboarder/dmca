/*
 * Copyright (C) 2011-2013 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#include <arch.h>
#include <debug.h>
#include <drivers/miu.h>
#include <platform.h>
#include <platform/memmap.h>
#include <platform/miu.h>
#include <platform/soc/chipid.h>
#include <platform/soc/miu.h>
#include <platform/soc/pmgr.h>
#include <platform/clocks.h>
#include <platform/soc/hwclocks.h>

static void miu_configure_bridge(const u_int32_t *bridge_settings);

#define STATIC_BRIDGE_SHIFT	(28)
#define STATIC_BRIDGE_MASK	((1 << STATIC_BRIDGE_SHIFT) - 1)
#define SB_WIDGETS		(0 << STATIC_BRIDGE_SHIFT)
#define SOCBUSMUX_WIDGETS	(1 << STATIC_BRIDGE_SHIFT)
#define IOBUSMUX_WIDGETS	(2 << STATIC_BRIDGE_SHIFT)
#define SWITCH_FAB_WIDGETS	(3 << STATIC_BRIDGE_SHIFT)
#define CP_WIDGETS		(4 << STATIC_BRIDGE_SHIFT)
#define LIO_WIDGETS		(5 << STATIC_BRIDGE_SHIFT)
#define ANS_WIDGETS		(6 << STATIC_BRIDGE_SHIFT)

static const u_int64_t bridge_registers[] = {
	SB_BASE_ADDR,
	SOC_BUSMUX_BASE_ADDR,
	IOBUSMUX_BASE_ADDR,
	SWTCH_FAB_BASE_ADDR,
	CP_COM_BASE_ADDR,
	LIO_AFC_AIU_BASE_ADDR,
	ANS_AFC_AIU_BASE_ADDR
};

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
static const u_int32_t bridge_settings_static[] = {

	SB_WIDGETS | ASIO_CLK_CTRL,				0x03000102,

	SB_WIDGETS | DYN_CLK_GATING,				(0x30 << 16) | (0x1 << 4) | (0x1 << 3) | (0x1 << 0),
	SB_WIDGETS | SIO_ASYNC_FIFO_SB_RD_RATE_LIMIT,		0x0,
	SB_WIDGETS | SIO_ASYNC_FIFO_SB_WR_RATE_LIMIT,		0x0,
	SB_WIDGETS | SIO_ASYNC_FIFO_SB_WGATHER,			(0x1 << 8),

	SB_WIDGETS | SIO_DAPASYNC_FIFO_SB_RD_RATE_LIMIT,	0x0,
	SB_WIDGETS | SIO_DAPASYNC_FIFO_SB_WR_RATE_LIMIT,	0x0,
	SB_WIDGETS | SIO_DAPASYNC_FIFO_SB_WGATHER,		(0x1 << 8),

	SB_WIDGETS | AIU_SB_CPG_CNTL,				(0x1 << 31) | (0x1 << 16) | (0x1 << 12) | 0x4,
	0,0,

	SOCBUSMUX_WIDGETS | DWRRCFG_DISPMUX_BULK,		(0x51 << 8) | (0x41 << 0),
	SOCBUSMUX_WIDGETS | TLIMIT_LVL0_CAMERAMUX,		(0x40 << 8) | (0xff << 0),
	SOCBUSMUX_WIDGETS | TLIMIT_LVL1_CAMERAMUX,		(0x20 << 8) | (0x80 << 0),
	SOCBUSMUX_WIDGETS | TLIMIT_LVL1_MEDIAMUX,		(0x60 << 8) | (0x20 << 0),
	SOCBUSMUX_WIDGETS | TLIMIT_LVL2_MEDIAMUX,		(0x40 << 8) | (0x10 << 0),
	SOCBUSMUX_WIDGETS | TLIMIT_LVL0_IOMUX,			0x40,
	SOCBUSMUX_WIDGETS | TLIMIT_LVL1_IOMUX,			0x40,
	SOCBUSMUX_WIDGETS | TLIMIT_LVL2_IOMUX,			0x30,
	SOCBUSMUX_WIDGETS | TLIMIT_LVL3_IOMUX,			0x20,
	SOCBUSMUX_WIDGETS | SOCBUSMUX_CPG_CNTL,			(0x1 << 31) | (0x1 << 16) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x4 << 0),
	0,0,

	IOBUSMUX_WIDGETS | IOBUSMUX_CPG_CNTL,			(0x1 << 31) | (0x1 << 16) | (0x1 << 12) | (0x1 << 10) | (0x4 << 0),
	0,0,

	SWITCH_FAB_WIDGETS | SWITCH_FAB_CPG_CNTL,		(0x1 << 31) | (0x1 << 16) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x4 << 0),
	0,0,

	CP_WIDGETS | CP_DYN_CLK_GATING_CTRL,			(0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0),
	0,0,


	LIO_WIDGETS | LIO_MEMCACHE_DATASETID_OVERRIDE,		(0xC00F000F),
	0,0,

	0,0,
};
#endif	// (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)

// Alert: The ordering effects the device tree entries under pmgr node's reg dictionary.
// If you touch this table, and DISPLAY0 index gets changed, please update miu_update_device_tree.
#if WITH_DEVICETREE && !SUPPORT_FPGA
static const u_int32_t bridge_settings_dynamic[] = {
	MEDIABUSMUX_REGS_DWRRCFG_JPEG_BULK,	(0x28 << 8) | (0x20 << 0),
	MEDIABUSMUX_REGS_DWRRCFG_MSR_BULK,	(0x35 << 8) | (0x2a << 0),
	MEDIABUSMUX_REGS_DWRRCFG_VXE_LLT,	(0x14 << 8) | (0x10 << 0),
	MEDIABUSMUX_REGS_DWRRCFG_VXE_BULK,	(0x67 << 8) | (0x52 << 0),
	MEDIABUSMUX_REGS_DWRRCFG_VXD_LLT,	(0x14 << 8) | (0x10 << 0),
	MEDIABUSMUX_REGS_DWRRCFG_VXD_BULK,	(0x35 << 8) | (0x2a << 0),
	MEDIABUSMUX_REGS_CPG_CNTL,		(0x1 << 31) | (0x1 << 16) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x4 << 0),
	0,0,

	MSR_REGS_CPG_CNTL,			(0x1 << 31) | (0x1 << 16) | (0x4 << 0),
	MSR_REGS_AW_TLIMIT,			(0x28 << 8) | (0x28 << 0),
	MSR_REGS_MEMCACHE_DATASETID_OVERRIDE,	(0xC00F000F),
	0,0,

	AJPEG_REGS_CPG_CNTL,			(0x1 << 31) | (0x1 << 16) | (0x4 << 0),
	AJPEG_REGS_MEMCACHE_DATASETID_OVERRIDE,	(0xC00F000F),	
	0,0,

	VXE_REGS_CPG_CNTL,			(0x1 << 31) | (0x1 << 16) | (0x4 << 0),
	VXE_REGS_AR_TLIMIT,			(0x10 << 8) | (0x10 << 0),
	VXE_REGS_MEMCACHE_DATASETID_OVERRIDE,	(0xC00F000F),
	VXE_REGS_MEMCACHE_HINT_OVERRIDE,	(0x1 << 31) | (0x1 << 30) | (0x1 << 16) | (0x4 << 0),
	VENC_INT_IDLE_CTRL,			(0x1 << 20) | (0x1 << 16) | (0x1 << 12),
	VENC_INT_AxCACHE_REMAPPING_REG(0, 0),	(0x01234567),
	VENC_INT_AxCACHE_REMAPPING_REG(0, 1),	(0x89abcd0f),
	VENC_INT_AxCACHE_REMAPPING_REG(1, 0),	(0x01234567),
	VENC_INT_AxCACHE_REMAPPING_REG(1, 1),	(0x89abcd0f),
	0,0,

	VXD_REGS_CPG_CNTL,			(0x1 << 31) | (0x1 << 16) | (0x4 << 0),
	VXD_REGS_MEMCACHE_HINT_OVERRIDE,	(0x1 << 31) | (0x1 << 30) | (0x1 << 16) | (0x4 << 0),
	VXD_REGS_MEMCACHE_DATASETID_OVERRIDE,	(0xC00F000F),
	VDEC_INT_IDLE_CTRL,			(0x1 << 12),
	VDEC_INT_AxCACHE_REMAPPING_REG(0, 0),	(0x01234567),
	VDEC_INT_AxCACHE_REMAPPING_REG(0, 1),	(0x89abcdef),
	VDEC_INT_AxCACHE_REMAPPING_REG(1, 0),	(0x01234567),
	VDEC_INT_AxCACHE_REMAPPING_REG(1, 1),	(0x89abcd0f),
	0,0,

	CAMERABUSMUX_REGS_ISPKF_RT,		(0x10 << 8) | (0xc << 0),
	CAMERABUSMUX_REGS_CPG_CNTL,		(0x1 << 31) | (0x4 << 0),
	0,0,

	ISP_DMA_REGS_CPG_CNTL,			(0x1 << 31)  | (0x1 << 16) | (0x4 << 0),
	ISP_DMA_REGS_BW_THRESHOLD,		(0x24 << 16) | (0x10 << 0),
	ISP_DMA_REGS_AW_LIMIT,			(0x30 << 8)  | (0x30 << 0),
	ISP_DMA_REGS_AR_LIMIT,			(0x40 << 8)  | (0x40 << 0),
	ISP_DMA_REGS_MEMCACHE_DATASETID_OVERRIDE, (0xC00F000F),

	ISP_KF_REGS_CPG_CNTL,			(0x1 << 31) | (0x1 << 16) | (0x4 << 0),
	ISP_KF_REGS_MEMCACHE_DATASETID_OVERRIDE, (0xC00F000F),
	0,0,

	DISPBUSMUX_REGS_DWRRCFG_DP1_BULK,	(0x1e << 8) | (0x18 << 0),
	DISPBUSMUX_REGS_CPG_CNTL,		(0x1 << 31) | (0x1 << 16) | (0x1 << 11) | (0x1 << 10) | (0x4 << 0),
	0,0,

	// For A0, DISP0 DYN_ENA is disabled in miu_update_device_tree.
	DISPLAYPIPE0_REGS_CPG_CNTL,			(0x1 << 31) | (0x1 << 16) | (0x4 << 0),
	DISPLAYPIPE0_REGS_MEMCACHE_DATASETID_OVERRIDE,	(0xC00F000F),	
	0,0,

	DISPLAYPIPE1_REGS_CPG_CNTL,		(0x1 << 31) | (0x1 << 16) | (0x4 << 0),
	DISPLAYPIPE1_REGS_MEMCACHE_DATASETID_OVERRIDE,	(0xC00F000F),		
	0,0,

	ANS_REGS_CPG_CNTL,			(0x1 << 31) | (0x1 << 16) | (0x4 << 0),
	ANS_REGS_MEMCACHE_DATASETID_OVERRIDE,	(0xC00F000F),
	ANS_KF_TLIMIT_RD,			(0x00000909),
	ANS_KF_TLIMIT_WR,			(0x00000909),
	ANS_ANC0_TLIMIT_RD,			(0x00000505),
	ANS_ANC0_TLIMIT_WR,			(0x00000505),
	ANS_ANC1_TLIMIT_RD,			(0x00000505),
	ANS_ANC1_TLIMIT_WR,			(0x00000505),
	0,0,

	GFX_REGS_CPG_CNTL,			(0x1 << 31) | (0x1 << 16) | (0x4 << 0),
	0,0,
	
	GFX_IMG4_AFUSER_REGS_MCCFG_0,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x4 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_1,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x4 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_2,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x4 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_3,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x1 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_4,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x1 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_5,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x4 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_6,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x4 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_7,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x4 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_8,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x4 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_9,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x4 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_10,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x4 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_11,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x4 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_12,	(0x0 << 23) | (0xD << 19) | (0x4 << 16) | (0x0 << 15) | (0xD << 11) | (0x1 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_13,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x1 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_14,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x4 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_15,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x4 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_16,	(0x0 << 23) | (0xE << 19) | (0x4 << 16) | (0x0 << 15) | (0xE << 11) | (0x2 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_17,	(0x0 << 23) | (0xE << 19) | (0x4 << 16) | (0x0 << 15) | (0xE << 11) | (0x2 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_18,	(0x0 << 23) | (0xE << 19) | (0x4 << 16) | (0x0 << 15) | (0xE << 11) | (0x2 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_19,	(0x0 << 23) | (0xE << 19) | (0x4 << 16) | (0x0 << 15) | (0xE << 11) | (0x2 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_20,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x4 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_21,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x4 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_22,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x4 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_23,	(0x0 << 23) | (0xD << 19) | (0x4 << 16) | (0x0 << 15) | (0xD << 11) | (0x1 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_24,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x4 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_25,	(0x0 << 23) | (0xF << 19) | (0x1 << 16) | (0x0 << 15) | (0xF << 11) | (0x1 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_26,	(0x0 << 23) | (0xF << 19) | (0x1 << 16) | (0x0 << 15) | (0xF << 11) | (0x1 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_27,	(0x0 << 23) | (0xF << 19) | (0x4 << 16) | (0x0 << 15) | (0xF << 11) | (0x4 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_28,	(0x0 << 23) | (0xD << 19) | (0x4 << 16) | (0x0 << 15) | (0xD << 11) | (0x1 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_29,	(0x0 << 23) | (0xF << 19) | (0x1 << 16) | (0x0 << 15) | (0xF << 11) | (0x1 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_30,	(0x0 << 23) | (0xF << 19) | (0x1 << 16) | (0x0 << 15) | (0xF << 11) | (0x1 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	GFX_IMG4_AFUSER_REGS_MCCFG_31,	(0x0 << 23) | (0xF << 19) | (0x1 << 16) | (0x0 << 15) | (0xF << 11) | (0x1 << 8) | (0x0 << 2) | (0x0 << 1) | (0x1 << 0),
	0,0,

	AUSB_REGS_RD_RATE_LIMIT,		(0x0),
	AUSB_REGS_WRALIMIT,			(0x0),
	AUSB_REGS_WGATHER,			(0x1 << 8),
	0,0,

	0,0,
};
#endif

extern void ausb_setup_widgets();

int miu_initialize_internal_ram(void)
{
#if APPLICATION_SECUREROM
	// Ensure that rPMGR_SCRATCH0-3 get cleared
	rPMGR_SCRATCH0 = 0;
	rPMGR_SCRATCH1 = 0;
	rPMGR_SCRATCH2 = 0;
	rPMGR_SCRATCH3 = 0;
#endif /* APPLICATION_SECUREROM */

	// Save the Security Epoch in the top byte of PMGR_SCRATCH0
	rPMGR_SCRATCH0 &= ~0xFF000000;
	rPMGR_SCRATCH0 |= (platform_get_security_epoch()) << 24;

	return 0;
}

int miu_init(void)
{
#if APPLICATION_IBOOT && !PRODUCT_IBEC
	// Verify that the Security Epoch in PMGR_SCRATCH0 matches
	if ((rPMGR_SCRATCH0 >> 24) != platform_get_security_epoch()) {
		panic("miu_init: Epoch Mismatch\n");
	}
#endif
#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
	miu_configure_bridge(bridge_settings_static);
#endif
	ausb_setup_widgets();

	return 0;
}

void miu_suspend(void)
{
	/* nothing required for suspend */
}

int miu_initialize_dram(bool resume)
{
#if APPLICATION_IBOOT && WITH_HW_AMC
	mcu_initialize_dram(resume);
#endif
	return 0;
}

void miu_select_remap(enum remap_select sel)
{
	switch (sel) {
		case REMAP_SRAM:
			rTZSROMCTRL_ROMADDRREMAP = (rTZSROMCTRL_ROMADDRREMAP & ~3) | (1 << 0);
			break;

		case REMAP_SDRAM:
			rTZSROMCTRL_ROMADDRREMAP = (rTZSROMCTRL_ROMADDRREMAP & ~3) | (2 << 0);
			break;
	}
}

void miu_bypass_prep(void)
{
}

static void miu_configure_bridge(const u_int32_t *bridge_settings)
{
	volatile u_int32_t *reg;
	u_int32_t cnt = 0, bridge, offset, data;

	while ((bridge_settings[cnt] != 0) || (bridge_settings[cnt + 1] != 0)) {
		while ((bridge_settings[cnt] != 0) || (bridge_settings[cnt + 1] != 0)) {
			bridge = bridge_settings[cnt] >> STATIC_BRIDGE_SHIFT;
			offset = bridge_settings[cnt] & STATIC_BRIDGE_MASK;
			data = bridge_settings[cnt + 1];
			reg = (volatile u_int32_t *)(bridge_registers[bridge] + offset);
			*reg = data;
			cnt += 2;
		}
		cnt += 2;
	}
}

#if WITH_DEVICETREE

void miu_update_device_tree(DTNode *pmgr_node)
{
	// XXX TODO: handle this better once we understand tunable story for FPGA
#if !SUPPORT_FPGA
	uint32_t propSize;
	char *propName;
	void *propData;

	// Fill in the bridge-settings property
	propName = "bridge-settings";
	if (FindProperty(pmgr_node, &propName, &propData, &propSize)) {
		if (propSize < sizeof(bridge_settings_dynamic)) {
			panic("miu_update_device_tree: bridge-settings property is too small (0x%x/0x%zx)", propSize, sizeof(bridge_settings_dynamic));
		}
		memcpy(propData, bridge_settings_dynamic, sizeof(bridge_settings_dynamic));
	}
#endif	
}

#endif
