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
#include <arch.h>
#include <debug.h>
#include <drivers/miu.h>
#include <platform.h>
#include <platform/memmap.h>
#include <platform/miu.h>
#include <platform/soc/miu.h>
#include <platform/soc/pmgr.h>
#include <platform/clocks.h>
#include <platform/soc/chipid.h>
#include <platform/soc/hwclocks.h>

static void miu_configure_bridge(const u_int32_t *bridge_settings);

#define STATIC_BRIDGE_SHIFT	(28)
#define STATIC_BRIDGE_MASK	((1 << STATIC_BRIDGE_SHIFT) - 1)
#define SB_WIDGETS		(0 << STATIC_BRIDGE_SHIFT)
#define SOCBUSMUX_WIDGETS	(1 << STATIC_BRIDGE_SHIFT)
#define IOBUSMUX_WIDGETS	(2 << STATIC_BRIDGE_SHIFT)
#define SWITCH_FAB_WIDGETS	(3 << STATIC_BRIDGE_SHIFT)
#define CP_WIDGETS		(4 << STATIC_BRIDGE_SHIFT)
#define ANS_WIDGETS		(5 << STATIC_BRIDGE_SHIFT)
#define LIO_WIDGETS		(6 << STATIC_BRIDGE_SHIFT)

static const u_int64_t bridge_registers[] = {
	SB_BASE_ADDR,
	SOC_BUSMUX_BASE_ADDR,
	IOBUSMUX_BASE_ADDR,
	SWTCH_FAB_BASE_ADDR,
	CP_COM_BASE_ADDR,
	ANS_AFC_AIU_BASE_ADDR,
	LIO_AFC_AIU_BASE_ADDR
};

#if (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)
#if SUB_PLATFORM_T7000

static const u_int32_t bridge_settings_static[] = {

	SB_WIDGETS | ASIO_CLK_CTRL,				0x03000102,

	SB_WIDGETS | DYN_CLK_GATING,				(0x30 << 16) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 0),
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
	SOCBUSMUX_WIDGETS | TLIMIT_LVL0_IOMUX,			0xa2,
	SOCBUSMUX_WIDGETS | TLIMIT_LVL1_IOMUX,			0xa2,
	SOCBUSMUX_WIDGETS | TLIMIT_LVL2_IOMUX,			0xa2,
	SOCBUSMUX_WIDGETS | TLIMIT_LVL3_IOMUX,			0xa2,
	SOCBUSMUX_WIDGETS | SOCBUSMUX_CPG_CNTL,			(0x1 << 31) | (0x1 << 16) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x4 << 0),
	0,0,

	IOBUSMUX_WIDGETS | IOBUSMUX_CPG_CNTL,			(0x1 << 31) | (0x1 << 16) | (0x1 << 13) | (0x1 << 12) | (0x1 << 10) | (0x4 << 0),
	0,0,

	SWITCH_FAB_WIDGETS | SWITCH_FAB_AMAP_LOCK,		(0x1 << 0),
	SWITCH_FAB_WIDGETS | SWITCH_FAB_ARBCFG,			(0x1 << 10) | (0x1 << 8) | (0x1 << 5) | (0x1 << 4) | (0x4 << 0),
	SWITCH_FAB_WIDGETS | SWITCH_FAB_CPG_CNTL,		(0x1 << 31) | (0x1 << 30) | (0x1 << 16) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x4 << 0),
	0,0,

	CP_WIDGETS | CP_DYN_CLK_GATING_CTRL,			(0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0),
	0,0,

	LIO_WIDGETS | LIO_MEMCACHE_DATASETID_OVERRIDE,		(0xC00F000F),
	0,0,

	0,0
};

#elif SUB_PLATFORM_T7001

static const u_int32_t bridge_settings_static[] = {

	SB_WIDGETS | ASIO_CLK_CTRL,				0x03000102,

	SB_WIDGETS | DYN_CLK_GATING,				(0x30 << 16) | (0x1 << 5) | (0x1 << 4) | (0x1 << 3) | (0x1 << 0),
	SB_WIDGETS | SIO_ASYNC_FIFO_SB_RD_RATE_LIMIT,		0x0,
	SB_WIDGETS | SIO_ASYNC_FIFO_SB_WR_RATE_LIMIT,		0x0,
	SB_WIDGETS | SIO_ASYNC_FIFO_SB_WGATHER,			(0x1 << 8),

	SB_WIDGETS | SIO_DAPASYNC_FIFO_SB_RD_RATE_LIMIT,	0x0,
	SB_WIDGETS | SIO_DAPASYNC_FIFO_SB_WR_RATE_LIMIT,	0x0,
	SB_WIDGETS | SIO_DAPASYNC_FIFO_SB_WGATHER,		(0x1 << 8),
	
	SB_WIDGETS | AIU_SB_CPG_CNTL,				(0x1 << 31) | (0x1 << 30) | (0x1 << 16) | (0x1 << 12) | 0x4,
	0,0,

	SOCBUSMUX_WIDGETS | DWRRCFG_DISP0_RT,			(0x28 << 8) | (0x20 << 0),
	SOCBUSMUX_WIDGETS | DWRRCFG_IOMUX_BULK,			(0x51 << 8) | (0x41 << 0),
	SOCBUSMUX_WIDGETS | TLIMIT_LVL0_DISP0,			(0x60 << 0),	
	SOCBUSMUX_WIDGETS | TLIMIT_LVL0_CAMERAMUX,		(0x40 << 8) | (0xff << 0),
	SOCBUSMUX_WIDGETS | TLIMIT_LVL1_CAMERAMUX,		(0x20 << 8) | (0x80 << 0),
	SOCBUSMUX_WIDGETS | TLIMIT_LVL1_MEDIAMUX,		(0x60 << 8) | (0x20 << 0),
	SOCBUSMUX_WIDGETS | TLIMIT_LVL2_MEDIAMUX,		(0x40 << 8) | (0x10 << 0),
	SOCBUSMUX_WIDGETS | TLIMIT_LVL0_IOMUX,			(0x40 << 8) | (0xa4 << 0),
	SOCBUSMUX_WIDGETS | TLIMIT_LVL1_IOMUX,			(0x40 << 8) | (0x40 << 0),
	SOCBUSMUX_WIDGETS | TLIMIT_LVL2_IOMUX,			(0x30 << 8) | (0x30 << 0),
	SOCBUSMUX_WIDGETS | TLIMIT_LVL3_IOMUX,			(0x20 << 8) | (0x20 << 0),
	SOCBUSMUX_WIDGETS | SOCBUSMUX_CPG_CNTL,			(0x1 << 31) | (0x1 << 16) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x4 << 0),
	0,0,

	IOBUSMUX_WIDGETS | IOBUSMUX_REGS_DWRRCFG_DISP1_BULK,	(0x1e << 8) | (0x18 << 0),
	IOBUSMUX_WIDGETS | IOBUSMUX_CPG_CNTL,			(0x1 << 31) | (0x1 << 16) | (0x1 << 14) | (0x1 << 13) | (0x1 << 12) | (0x1 << 10) | (0x4 << 0),
	0,0,

	SWITCH_FAB_WIDGETS | SWITCH_FAB_AMAP_LOCK,		(0x1 << 0),
	SWITCH_FAB_WIDGETS | SWITCH_FAB_CPG_CNTL,		(0x1 << 31) | (0x1 << 30) | (0x1 << 16) | (0x1 << 13) | (0x1 << 12) | (0x1 << 11) | (0x1 << 10) | (0x4 << 0),
	0,0,

	CP_WIDGETS | CP_DYN_CLK_GATING_CTRL,			(0x1 << 4) | (0x1 << 3) | (0x1 << 2) | (0x1 << 1) | (0x1 << 0),
	0,0,

	LIO_WIDGETS | LIO_MEMCACHE_DATASETID_OVERRIDE,          (0xC00F000F),	
	0,0,

	0,0,
};
#endif
#endif	// (APPLICATION_IBOOT && !PRODUCT_IBOOT && !PRODUCT_IBEC)

#if WITH_DEVICETREE
#if SUPPORT_FPGA

struct bridge_list_t {
	char		*bridge_settings;
	uint32_t	fpga_enable_mask;
};

#define BRIDGE(b, m)	{ "bridge-settings-" #b, (m) }

// ALERT: The ordering effects the device tree entries under pmgr node's reg dictionary.
//	  It also affects the bridge id used in the pmgr device-clock nodes. The
//	  bridge id must match the id on the device tree bridge-settings-<n> properties.
// NOTE:  The second parameter on the BRIDGE macro is used to dynamically determine
//	  which bridge settings properties should exist based upon the hardware
//	  blocks actually present on the device.
static const struct bridge_list_t bridge_list[] = {
	BRIDGE(0, FPGA_HAS_ALWAYS),
	BRIDGE(1, FPGA_HAS_MEDIA),
	BRIDGE(2, FPGA_HAS_MSR),
	BRIDGE(3, FPGA_HAS_JPEG),
	BRIDGE(4, FPGA_HAS_AVE),
	BRIDGE(5, FPGA_HAS_VXD),
	BRIDGE(6, FPGA_HAS_ISP),
#if SUB_PLATFORM_T7000
	BRIDGE(7, (FPGA_HAS_DISP0 | FPGA_HAS_DISP1)),
	BRIDGE(8, FPGA_HAS_DISP0),
	BRIDGE(9, FPGA_HAS_DISP1),
	BRIDGE(10, FPGA_HAS_GFX),
	BRIDGE(11, FPGA_HAS_GFX),
	BRIDGE(12, FPGA_HAS_ALWAYS),
	BRIDGE(13, FPGA_HAS_PCIE),
#elif SUB_PLATFORM_T7001
	BRIDGE(7, FPGA_HAS_DISP0),
	BRIDGE(8, FPGA_HAS_DISP1),
	BRIDGE(9, FPGA_HAS_GFX),
	BRIDGE(10, FPGA_HAS_GFX),
	BRIDGE(11, FPGA_HAS_ALWAYS),
	BRIDGE(12, FPGA_HAS_PCIE),
#endif
};
#endif // SUPPORT_FPGA
#endif // WITH_DEVICETREE

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
			rSECUREROMCTRL_ROMADDRREMAP = (rSECUREROMCTRL_ROMADDRREMAP & ~3) | (1 << 0);
			break;

		case REMAP_SDRAM:
			rSECUREROMCTRL_ROMADDRREMAP = (rSECUREROMCTRL_ROMADDRREMAP & ~3) | (2 << 0);
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
#if SUPPORT_FPGA
	DTNode		*node;
	char		*propName;
	void		*propData;
	uint32_t	propSize;
	uint32_t	fpga_blocks = chipid_get_fpga_block_instantiation();
	uint32_t	i;

	dprintf(DEBUG_INFO, "chipid_get_fpga_block_instantiation() = 0x%08X\n",
		fpga_blocks);

	if (FindNode(0, "arm-io/pmgr", &node)) {

		// For each bridge...
		for (i = 0; i < ARRAY_SIZE(bridge_list); i++) {

			// Filter out if not supported by FPGA
			if (!(fpga_blocks & bridge_list[i].fpga_enable_mask)) {

				propName = bridge_list[i].bridge_settings;

				if (FindProperty(node, &propName, &propData, &propSize)) {
					dprintf(DEBUG_CRITICAL, "Eliding %s\n", propName);
					propName[0] = '~';
				}
			}
		}
	}
#endif // SUPPORT_FPGA
}

#endif // WITH_DEVICETREE

